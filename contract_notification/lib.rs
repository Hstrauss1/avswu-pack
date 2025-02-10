#![cfg_attr(not(feature = "std"), no_std, no_main)]
#[ink::contract]
mod contract_notification {

    use ink::prelude::string::String;
    use ink::prelude::string::ToString;
    use ink::prelude::{vec, vec::Vec};
    use ink::storage::Mapping;
    // #[cfg(test)]
    // use log::{debug, error};
    use scale::{Decode, Encode};

    // contract storage data structures

    /*
     * map(manufacturer_id,software_id: u64,version_id) -> notification
     */

    /// notification
    #[derive(Encode, Decode, Debug, Clone, PartialEq)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct Notification {
        notification_json: String,
    }

    /// notification map key
    #[derive(Encode, Decode, Debug, Clone, Copy)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct NotificationMapKey {
        manufacturer_id: u64,
        software_id: u64,
        version_id: u64,
        create_time: u64,
    }

    /// notification vector map key
    #[derive(Encode, Decode, Debug, Clone, Copy)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct NotificationSoftwareMapKey {
        manufacturer_id: u64,
        software_id: u64,
    }

    /// notification software vector map key
    #[derive(Encode, Decode, Debug, Clone, Copy)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct NotificationManufacturerMapKey {
        manufacturer_id: u64,
    }

    /*
     * data structures for the contract (that are implemented in the methods below)
     */
    #[ink(storage)]
    #[derive(Default)]
    pub struct ContractNotification {
        // map of all notifications
        // map(manufacturer_id, software_id, version_id) -> notification
        notification_map: Mapping<NotificationMapKey, Notification>,

        // map of all notifications-keys for a specific software
        // map(manufacturer_id, software_id) -> sorted vector<notification-map-keys>
        software_map: Mapping<NotificationSoftwareMapKey, Vec<NotificationMapKey>>,

        // map of all software_ids for a specific manufacturer, that currently has notifications
        // map(manufacturer_id) -> vector<software_id>
        manufacturer_map: Mapping<NotificationManufacturerMapKey, Vec<u64>>,
    }

    /*
     * errors
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum NotificationErrorType {
        EmptyNotificationValue,
        NotificationDoesNotExist,
        NotificationDoesNotExistAfterStartTime,
        UnableToParseManufacturerListJson,
        UnableToReadNotificationList,
    }

    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub struct NotificationError {
        error_type: NotificationErrorType,
        message: String,
    }

    /*
     * success
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum NotificationSuccessType {
        WriteNotification,
        ReadNotification,
        ReadNotificationVector,
    }

    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub struct NotificationSuccess {
        success_type: NotificationSuccessType,
        message: String,
    }

    /*
     * contract implementation
     */
    impl ContractNotification {
        /// constructor
        #[ink(constructor)]
        pub fn new() -> Self {
            // initialize data structures
            Self::default()
        }

        /// write notification
        #[ink(message)]
        pub fn write_notification(
            &mut self,
            manufacturer_id: u64,
            software_id: u64,
            version_id: u64,
            create_time: u64,
            notification_json: String,
        ) -> Result<NotificationSuccess, NotificationError> {
            // if notification is empty, error
            if notification_json.len() == 0 {
                let message = "notification json is empty".to_string();

                // return error
                return Err(NotificationError {
                    error_type: NotificationErrorType::EmptyNotificationValue,
                    message,
                });
            }

            // create notification
            let notif = Notification { notification_json };

            /*
             *insert notification in to notif map
             */
            let notif_map_key = NotificationMapKey {
                manufacturer_id,
                software_id,
                version_id,
                create_time,
            };
            self.notification_map.insert(notif_map_key, &notif);

            /*
             * insert map key into vector map
             */

            // notif vector key
            let notif_vec_key = NotificationSoftwareMapKey {
                manufacturer_id,
                software_id,
            };

            // check if entry exists, push to vector, else create new vector
            match self.software_map.get(notif_vec_key) {
                Some(mut notif_vec) => {
                    // push notif
                    notif_vec.push(notif_map_key);

                    // sort by create_time
                    notif_vec.sort_by_key(|elem| elem.create_time);

                    // update the map
                    self.software_map.insert(notif_vec_key, &notif_vec);
                }
                None => {
                    // create a new vector, and insert it into the map
                    let mut notif_vec = vec![];
                    notif_vec.push(notif_map_key);
                    self.software_map.insert(notif_vec_key, &notif_vec);
                }
            }

            /*
             * insert software_id into software_map
             */
            let sw_key = NotificationManufacturerMapKey { manufacturer_id };

            // check if entry exists, push to vector, else create new vector
            match self.manufacturer_map.get(sw_key) {
                Some(mut software_id_vec) => {
                    // if the software id does not exist, add it
                    if !software_id_vec.iter().any(|&elem| elem == software_id) {
                        software_id_vec.push(software_id);
                    }

                    // update the map
                    self.manufacturer_map.insert(sw_key, &software_id_vec);
                }
                None => {
                    // create a new vector, and insert it into the map
                    let mut software_id_vec = vec![];
                    software_id_vec.push(software_id);
                    self.manufacturer_map.insert(sw_key, &software_id_vec);
                }
            }

            // return success with message
            let mut message: String = "wrote notification for ".to_string();
            let key_message =
                Self::create_map_key_message(manufacturer_id, software_id, version_id);
            message.push_str(&key_message);
            Ok(NotificationSuccess {
                success_type: NotificationSuccessType::WriteNotification,
                message: message.to_string(),
            })
        }

        // gets/maps a vector of map keys to a vector of notifications
        pub fn get_notification_vector(
            &mut self,
            notif_vec: Vec<NotificationMapKey>,
        ) -> Result<Vec<Notification>, NotificationError> {
            let mut result: Vec<Notification> = vec![];
            for i in 0..notif_vec.len() {
                // get notification
                let notif_map_key = notif_vec[i];
                match self.notification_map.get(notif_map_key) {
                    Some(notif) => {
                        result.push(notif);
                    }
                    None => {
                        // return error
                        let mut message = "notification does not exist for ".to_string();
                        let id_message = Self::create_map_key_message(
                            notif_map_key.manufacturer_id,
                            notif_map_key.software_id,
                            notif_map_key.version_id,
                        );
                        message.push_str(id_message.as_str());
                        message.push_str(
                            " while trying to map vector of keys to vector of notifications",
                        );
                        return Err(NotificationError {
                            error_type: NotificationErrorType::NotificationDoesNotExist,
                            message,
                        });
                    }
                }
            }

            Ok(result)
        }

        // get the last notification from start time
        pub fn get_last_notification(
            &mut self,
            manufacturer_id: u64,
            software_id: u64,
            notif_vec: Vec<NotificationMapKey>,
            start_time: u64,
        ) -> Result<Notification, NotificationError> {
            // if no notifications, return err
            if notif_vec.len() == 0 {
                let mut message = "there are no notifications for ".to_string();
                let ids_message = Self::create_vector_map_key_message(manufacturer_id, software_id);
                message.push_str(&ids_message);
                return Err(NotificationError {
                    error_type: NotificationErrorType::NotificationDoesNotExist,
                    message,
                });
            }

            // get the last notification
            let last_notif_key = notif_vec[notif_vec.len() - 1];

            if last_notif_key.create_time <= start_time {
                let mut message = "there are no recent notifications for ".to_string();
                let ids_message = Self::create_vector_map_key_message(manufacturer_id, software_id);
                message.push_str(&ids_message);
                message.push_str(" with a create_time greater than start_time");
                return Err(NotificationError {
                    error_type: NotificationErrorType::NotificationDoesNotExist,
                    message,
                });
            }

            // return the last notification
            match self.notification_map.get(last_notif_key) {
                Some(notif) => Ok(notif),
                None => {
                    let mut message :String =
                        "the last notification does not exist. but, it should exist.last notification = ".to_string();
                    let ids_message = Self::create_map_key_message(
                        last_notif_key.manufacturer_id,
                        last_notif_key.software_id,
                        last_notif_key.version_id,
                    );
                    message.push_str(&ids_message);
                    return Err(NotificationError {
                        error_type: NotificationErrorType::NotificationDoesNotExist,
                        message,
                    });
                }
            }
        }

        /// read last notification after start_time
        #[ink(message)]
        pub fn read_last_notification(
            &mut self,
            manufacturer_id: u64,
            software_id: u64,
            start_time: u64,
        ) -> Result<Notification, NotificationError> {
            // map key
            let notif_vec_map_key = NotificationSoftwareMapKey {
                manufacturer_id,
                software_id,
            };

            // get notification
            match self.software_map.get(notif_vec_map_key) {
                Some(notif_vec) => {
                    self.get_last_notification(manufacturer_id, software_id, notif_vec, start_time)
                }
                None => {
                    // return error
                    let mut message = "there are no notifications for ".to_string();
                    let ids_message =
                        Self::create_vector_map_key_message(manufacturer_id, software_id);
                    message.push_str(&ids_message);
                    Err(NotificationError {
                        error_type: NotificationErrorType::NotificationDoesNotExist,
                        message,
                    })
                }
            }
        }

        /// parse a json manufacturer list (since serde and serde_json are too expensive, and do not work within smart contract)
        /// list is of the format [id1,id2,...idn]
        pub fn parse_manufacturer_list(
            &mut self,
            manufacturer_list: &String,
        ) -> Result<Vec<u64>, NotificationErrorType> {
            // parse the string, into array of u64s
            // 1. remove [] chars
            let no_brackets = manufacturer_list.replace(&['[', ']'][..], "");
            // 2. split based on ,
            let tok_list_str: Vec<&str> = no_brackets.split(",").collect();
            // 3. arr[str] -> arr[u64]
            let mut man_id_vec = vec![];
            for i in 0..tok_list_str.len() {
                let _parse_result = match tok_list_str[i].parse::<u64>() {
                    Ok(val) => man_id_vec.push(val),
                    Err(_e) => {
                        return Err(NotificationErrorType::UnableToParseManufacturerListJson)
                    }
                };
            }
            Ok(man_id_vec)
        }

        /// create_map_key_vec from manufacturer id vec
        pub fn create_map_key_vec(
            &mut self,
            manufacturer_id_vec: &Vec<u64>,
        ) -> Result<Vec<NotificationSoftwareMapKey>, NotificationErrorType> {
            // loop through man id vec, and create keys
            let mut result: Vec<NotificationSoftwareMapKey> = vec![];
            for i in 0..manufacturer_id_vec.len() {
                let manufacturer_id = manufacturer_id_vec[i];
                // lookup software ids, create map keys
                let sw_map_key = NotificationManufacturerMapKey { manufacturer_id };
                match self.manufacturer_map.get(sw_map_key) {
                    Some(sw_id_vec) => {
                        // log::info!("sw_id_vec={:?}", sw_id_vec);
                        // add all software ids to result
                        for s in 0..sw_id_vec.len() {
                            let software_id = sw_id_vec[s];
                            let notif_map_key = NotificationSoftwareMapKey {
                                manufacturer_id,
                                software_id,
                            };
                            result.push(notif_map_key);
                        }
                    }
                    None => {}
                }
            }

            Ok(result)
        }

        /// read last notification after start_time
        #[ink(message)]
        pub fn read_last_notification_list(
            &mut self,
            manufacturer_list: String,
            start_time: u64,
        ) -> Result<Vec<Notification>, NotificationError> {
            // for each manufacturer id get the last notification > start_time
            // returns a vector of notifications

            // parse manufacturer list string into vector of ids
            let manufacturer_id_vec;
            let _parse_result = match self.parse_manufacturer_list(&manufacturer_list) {
                Ok(a_vec) => {
                    manufacturer_id_vec = a_vec;
                }
                Err(e) => {
                    let message = "parsing manufacturer_list json failed".to_string();
                    return Err(NotificationError {
                        error_type: e,
                        message,
                    });
                }
            };
            // log::info!("manufacturer_id_vec={:?}", manufacturer_id_vec);

            // create vector map keys
            let notif_key_vec;
            let _create_map_result = match self.create_map_key_vec(&manufacturer_id_vec) {
                Ok(key_vec) => {
                    notif_key_vec = key_vec;
                }
                Err(e) => {
                    let message = "creating map key for notifications failed".to_string();
                    return Err(NotificationError {
                        error_type: e,
                        message,
                    });
                }
            };
            // log::info!("notif_key_vec={:?}", notif_key_vec);

            // look up last notification for each of the keys and return them
            let mut result: Vec<Notification> = vec![];
            for i in 0..notif_key_vec.len() {
                let key = notif_key_vec[i];
                match self.read_last_notification(key.manufacturer_id, key.software_id, start_time)
                {
                    // if ok, push the value and continue the loop
                    Ok(notif) => {
                        result.push(notif);
                    }
                    // if error occured when reading, return the error
                    Err(error) => return Err(error),
                }
            }
            // log::info!("result={:?}", result);

            // return the result vector of notifications
            Ok(result)
        }

        /*
         * string format tools (since no std)
         */
        fn create_map_key_message(
            manufacturer_id: u64,
            software_id: u64,
            version_id: u64,
        ) -> String {
            let mut message: String = "".to_string();
            message.push_str("(");
            message.push_str("manufacturer_id=");
            let man_id: String = manufacturer_id.to_string();
            message.push_str(&man_id);
            message.push_str(", ");
            message.push_str("software_id=");
            let sof_id: String = software_id.to_string();
            message.push_str(&sof_id);
            message.push_str(", ");
            message.push_str("version_id=");
            let ver_id: String = version_id.to_string();
            message.push_str(&ver_id);
            message.push_str(")");
            message
        }

        fn create_vector_map_key_message(manufacturer_id: u64, software_id: u64) -> String {
            let mut message: String = "".to_string();
            message.push_str("(");
            message.push_str("manufacturer_id=");
            let man_id: String = manufacturer_id.to_string();
            message.push_str(&man_id);
            message.push_str(", ");
            message.push_str("software_id=");
            let sof_id: String = software_id.to_string();
            message.push_str(&sof_id);
            message.push_str(")");
            message
        }
    }

    /// unit tests
    #[cfg(test)]
    mod tests {
        /// imports all the definitions from the outer scope so we can use them here.
        use super::*;

        use log::{debug, info};

        use std::sync::Once;
        #[allow(dead_code)]
        static INIT: Once = Once::new();

        /// enable_logging runs before all tests, if uncommented
        #[allow(dead_code)]
        pub fn enable_logging() {
            INIT.call_once(|| {
                // enable logging, since log defaults to silent
                std::env::set_var("RUST_LOG", "info");
                let _ = env_logger::try_init();
            });
        }

        /// test write 1 notification
        #[ink::test]
        fn write_notification_map_success() {
            // enable_logging();

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let version_id: u64 = 1003;
            let create_time: u64 = 5;
            let notification_json: String = "{fake-notification: \"foobar\"}".to_string();

            // write the notification
            match cn.write_notification(
                manufacturer_id,
                software_id,
                version_id,
                create_time,
                notification_json,
            ) {
                Ok(result) => {
                    let notif = cn
                        .notification_map
                        .get(NotificationMapKey {
                            manufacturer_id,
                            software_id,
                            version_id,
                            create_time,
                        })
                        .unwrap();
                    debug!("wrote the notif={:?}", notif);
                    debug!("result={:?}", result);
                    assert_eq!(
                        result.success_type,
                        NotificationSuccessType::WriteNotification
                    );
                }
                Err(_error) => {
                    assert!(false);
                }
            }
        }

        #[ink::test]
        fn write_notification_fail() {
            // enable_logging();

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let version_id: u64 = 1003;
            let create_time: u64 = 5;
            let notification_json: String = "".to_string();

            // write the notification
            match cn.write_notification(
                manufacturer_id,
                software_id,
                version_id,
                create_time,
                notification_json,
            ) {
                Ok(_result) => {
                    assert!(false);
                }
                Err(error) => {
                    debug!("error={:?}", error);
                    assert_eq!(
                        error.error_type,
                        NotificationErrorType::EmptyNotificationValue
                    );
                }
            }
        }

        // insert notif's with out of order create_time = 4,1,3,5,2
        // so that sorted list becomes [1,2,3,4,5]
        #[ink::test]
        fn write_multiple_notifications_success() {
            // enable_logging();

            let create_time_vec = vec![4, 1, 3, 7, 9];
            // debug!("create_time_vec={:?}", create_time_vec);

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let mut version_id: u64 = 1100;
            let notification_json: String = "{fake-notification: \"foobar\"}".to_string();

            // notif vector key
            let notif_vec_key = NotificationSoftwareMapKey {
                manufacturer_id,
                software_id,
            };

            // write multiple notifications out of order
            for i in 0..create_time_vec.len() {
                version_id += 1;
                let _result = cn.write_notification(
                    manufacturer_id,
                    software_id,
                    version_id,
                    create_time_vec[i],
                    notification_json.clone(),
                );
                // let notif_key_vec = cn.notification_vector_map.get(notif_vec_key).unwrap();
                // error!("at i={}, notif_key_vec={:?}", i, notif_key_vec);
            }

            // second call
            let create_time_vec2 = vec![5, 2, 6, 8];
            let mut cn2 = ContractNotification::new();
            for i in 0..create_time_vec2.len() {
                version_id += 1;
                let _result = cn2.write_notification(
                    manufacturer_id,
                    software_id,
                    version_id,
                    create_time_vec2[i],
                    notification_json.clone(),
                );
            }

            // get the notif vector
            match cn.software_map.get(notif_vec_key) {
                Some(notif_vec) => {
                    // expect notifs create_time in vector to be in sorted order
                    let prev: u64 = 0;
                    for i in 0..notif_vec.len() {
                        let notif_map_key = notif_vec[i];
                        let _notif = cn.notification_map.get(notif_map_key).unwrap();
                        debug!("notif_vec[{}].create_time={}", i, notif_map_key.create_time);
                        let curr = notif_map_key.create_time;
                        if curr < prev {
                            assert!(false);
                        }
                    }
                }
                None => {
                    assert!(false);
                }
            }

            assert!(true);
        }

        #[ink::test]
        fn write_notification_vector_success() {
            // enable_logging();

            let create_time_vec: Vec<u64> = (0..15).collect();
            debug!("create_time_vec={:?}", create_time_vec);

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let notification_json: String = "{fake-notification: \"foobar\"}".to_string();

            // write all the notifications
            for i in 0..create_time_vec.len() {
                let version_id: u64 = 1100 + create_time_vec[i];
                let _result = cn.write_notification(
                    manufacturer_id,
                    software_id,
                    version_id,
                    create_time_vec[i],
                    notification_json.clone(),
                );
            }

            // check the vector of notifications
            let notif_vec_key = NotificationSoftwareMapKey {
                manufacturer_id,
                software_id,
            };
            match cn.software_map.get(notif_vec_key) {
                Some(notif_vec) => {
                    let vec_time = notif_vec
                        .clone()
                        .into_iter()
                        .map(|elem| elem.version_id)
                        .collect::<Vec<u64>>();
                    debug!("vec_time={:?}", vec_time);
                    assert!(notif_vec.len() == create_time_vec.len());
                }
                None => {
                    assert!(false);
                }
            }
        }

        #[ink::test]
        fn read_last_notification_success() {
            // enable_logging();

            let create_time_vec: Vec<u64> = (0..5).collect();
            // debug!("create_time_vec={:?}", create_time_vec);

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let notification_json: String = "{fake-notification: \"foobar\"}".to_string();

            // write all the notifications
            for i in 0..create_time_vec.len() {
                let version_id: u64 = 1100 + create_time_vec[i];
                let _result = cn.write_notification(
                    manufacturer_id,
                    software_id,
                    version_id,
                    create_time_vec[i],
                    notification_json.clone(),
                );
            }

            // read last notification
            let start_time = 2;
            let result = cn.read_last_notification(manufacturer_id, software_id, start_time);
            match result {
                Ok(notif) => {
                    debug!("notif={:?}", notif);
                    assert!(true);
                }
                Err(error) => {
                    debug!("error={:?}", error);
                    assert!(false);
                }
            }
        }

        #[ink::test]
        fn read_last_notification_fail() {
            // enable_logging();

            let create_time_vec: Vec<u64> = (0..5).collect();
            // debug!("create_time_vec={:?}", create_time_vec);

            let mut cn = ContractNotification::new();
            let manufacturer_id: u64 = 1001;
            let software_id: u64 = 1002;
            let notification_json: String = "{fake-notification: \"foobar\"}".to_string();

            // write all the notifications
            for i in 0..create_time_vec.len() {
                let version_id: u64 = 1100 + create_time_vec[i];
                let _result = cn.write_notification(
                    manufacturer_id,
                    software_id,
                    version_id,
                    create_time_vec[i],
                    notification_json.clone(),
                );
            }

            // read last notification
            let bad_software_id = 22213;
            let result = cn.read_last_notification(manufacturer_id, bad_software_id, 0);
            match result {
                Ok(notif) => {
                    debug!("notif={:?}", notif);
                    assert!(false);
                }
                Err(error) => {
                    debug!("error={:?}", error);
                    assert!(true);
                }
            }
        }

        /// test read notification list failure
        #[ink::test]
        fn read_last_notification_list_bad_json() {
            enable_logging();

            let mut cn = ContractNotification::new();

            let bogus_keys_json: String = "[bogusjson]".to_string();
            match cn.read_last_notification_list(bogus_keys_json, 1101) {
                Ok(result) => {
                    info!("result={:?}", result);
                    assert!(false);
                }
                _error => {
                    assert!(true);
                }
            }
        }

        /// test read notification list success
        #[ink::test]
        fn read_last_notification_list_success_json() {
            enable_logging();

            let mut cn = ContractNotification::new();

            // write some fake notifications
            for m in 0..3 {
                let manufacturer_id: u64 = 100 + m;
                for s in 0..4 {
                    let software_id: u64 = 1000 + s;
                    let notification_json: String = "{fake-notification: \"foobar\"}".to_string();
                    let version_id: u64 = 1111;
                    let create_time = 2222;
                    let _result = cn.write_notification(
                        manufacturer_id,
                        software_id,
                        version_id,
                        create_time,
                        notification_json.clone(),
                    );
                }
            }

            // read a list of notifications
            let good_json: String = "[100,101,102]".to_string();
            match cn.read_last_notification_list(good_json, 1101) {
                Ok(_result) => {
                    assert!(true);
                }
                error => {
                    info!("error={:?}", error);
                    assert!(false);
                }
            }
        }
    }
}

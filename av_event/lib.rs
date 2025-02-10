#![cfg_attr(not(feature = "std"), no_std, no_main)]
#[ink::contract]
mod av_event {

    use ink::prelude::string::String;
    use ink::prelude::vec::Vec;
    use ink::storage::Mapping;
    use scale::{Decode, Encode};

    // contract storage data structures

    // sane as enum from veins /home/veins/src/omnetpp-5.7/include/omnetpp/simtime.h
    #[allow(non_camel_case_types)]
    #[derive(Debug, Copy, Clone, Eq, Ord, PartialEq, PartialOrd, Encode, Decode)]
    #[repr(i32)]
    pub enum SimTimeUnit {
        SIMTIME_S = 0i32,
        SIMTIME_MS = -3i32,
        SIMTIME_US = -6i32,
        SIMTIME_NS = -9i32,
        SIMTIME_PS = -12i32,
        SIMTIME_FS = -15i32,
        SIMTIME_AS = -18i32,
    }

    /// message event data from veins with {av id, av  action, av signature, time, sim_time_unit}
    #[derive(Encode, Decode, Debug, Clone)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct AvEventEntry {
        id: u64,
        action: String,
        signature: String,
        t: u64,
        t_sim_time_unit: i32,
    }

    /// hashmap of id -> vector { Vec<AvEventEntry> }
    #[ink(storage)]
    #[derive(Default)]
    pub struct AvEvent {
        id_event_mapping: Mapping<u64, Vec<AvEventEntry>>,
        n: u64,
    }

    /*
     * events
     */
    /// event emitted upon succesful push
    #[ink(event)]
    pub struct AvEventPushReceipt {
        // topics are indexed and searchable by other nodes
        // we can only have a maximum of 3 topics to search by
        #[ink(topic)]
        id: u64,
        #[ink(topic)]
        action: String,
        #[ink(topic)]
        t: u64,
        t_sim_time_unit: i32,
    }

    /// event emitted upon successful get
    #[ink(event)]
    pub struct AvEventGetStatus {
        #[ink(topic)]
        id: u64,
        status: Result<AvEventSuccess, AvEventError>,
    }

    /*
     * errors
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum AvEventError {
        Push,
        GetIdNotFound,
    }

    /*
     * success (so all methods are functions, that return something)
     * smart contract result data is returned by emitting events.
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum AvEventSuccess {
        Push,
        Get,
    }

    impl AvEvent {
        /// constructor
        #[ink(constructor)]
        pub fn new() -> Self {
            Self {
                id_event_mapping: Mapping::new(),
                n: 0,
            }
        }

        /// push av-event
        #[ink(message)]
        pub fn push(
            &mut self,
            id: u64,
            action: String,
            signature: String,
            t: u64,
            t_sim_time_unit: i32,
        ) -> Result<AvEventSuccess, AvEventError> {
            let avee = AvEventEntry {
                id: id,
                action: action.clone(),
                signature: signature,
                t: t,
                t_sim_time_unit: t_sim_time_unit,
            };

            // if vector exists, push the event, else create vector with the event
            if let Some(mut v) = self.id_event_mapping.get(id) {
                // push event to vector
                v.push(avee);
                self.id_event_mapping.insert(id, &v);
            } else {
                // create vector with one event element
                let mut v: Vec<AvEventEntry> = Vec::new();
                v.push(avee);
                self.id_event_mapping.insert(id, &v);
            }
            self.n += 1;

            // success, emit event receipt
            self.env().emit_event(AvEventPushReceipt {
                id: id,
                action: action,
                t: t,
                t_sim_time_unit: t_sim_time_unit,
            });

            Ok(AvEventSuccess::Push)
        }

        /// getter
        #[ink(message)]
        pub fn get(&self, id: u64) -> Result<Vec<AvEventEntry>, AvEventError> {
            // if id exists in map, return value, or error
            if let Some(v) = self.id_event_mapping.get(id) {
                // success, emit event data
                self.env().emit_event(AvEventGetStatus {
                    id: id,
                    status: Ok(AvEventSuccess::Get),
                });
                Ok(v)
            } else {
                // success, emit event data
                self.env().emit_event(AvEventGetStatus {
                    id: id,
                    status: Err(AvEventError::GetIdNotFound),
                });
                Err(AvEventError::GetIdNotFound)
            }
        }
    }

    /// unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;

        /// test default constructor
        #[ink::test]
        fn constructor_works() {
            let av_event = AvEvent::default();
            assert_eq!(av_event.n, 0);
        }

        /// test push
        #[ink::test]
        fn push_works() {
            let mut av_event = AvEvent::new();
            let result = av_event.push(
                0,
                "accident".to_string(),
                "fake-signature".to_string(),
                23,
                SimTimeUnit::SIMTIME_MS as i32,
            );
            assert_eq!(av_event.n, 1);
            assert_eq!(result, Ok(AvEventSuccess::Push));
        }

        /// test get
        #[ink::test]
        fn get_works() {
            let mut av_event = AvEvent::new();
            // push 2 events
            let id = 121;
            let _result1 = av_event.push(
                id,
                "accident".to_string(),
                "fake-signature1".to_string(),
                23,
                SimTimeUnit::SIMTIME_MS as i32,
            );
            let id = 143;
            let _result2 = av_event.push(
                id,
                "accident".to_string(),
                "fake-signature2".to_string(),
                23,
                SimTimeUnit::SIMTIME_MS as i32,
            );
            let _result3 = av_event.push(
                id,
                "accident".to_string(),
                "fake-signature3".to_string(),
                58,
                SimTimeUnit::SIMTIME_MS as i32,
            );
            assert_eq!(av_event.n, 3);

            // try to get v
            if let Ok(v) = av_event.get(143) {
                println!("\nUNIT TEST DEBUG: v = {:?}", v);
                assert!(true);
            } else {
                assert!(false);
            }
        }

        // test get fails
        #[ink::test]
        fn get_fails() {
            let mut av_event = AvEvent::new();
            // push av event
            let _result = av_event.push(
                121,
                "accident".to_string(),
                "fake-signature1".to_string(),
                23,
                SimTimeUnit::SIMTIME_MS as i32,
            );

            // get event with id that does not exist in the map, if it exists
            // the test fails
            if let Ok(_v) = av_event.get(999) {
                assert!(false);
            } else {
                assert!(true);
            }
        }
    }
}

#![cfg_attr(not(feature = "std"), no_std, no_main)]
#[ink::contract]
mod contract_commitment {

    use ink::prelude::string::String;
    use ink::prelude::string::ToString;
    use ink::prelude::{vec, vec::Vec};
    use ink::storage::Mapping;
    use scale::{Decode, Encode};

    // contract storage data structures

    // same as enum from veins /home/veins/src/omnetpp-5.7/include/omnetpp/simtime.h
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

    #[derive(Encode, Decode, Debug, Clone, PartialEq)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    /// commitments
    pub struct Commitment {
        capital_r: String,
        w: String,
        capital_w: String,
    }

    /// id pair (id1,id2)
    #[derive(Encode, Decode, Debug, Clone, Copy)]
    // #[derive(Encode, Decode, Debug, Clone, Copy)]
    #[cfg_attr(
        feature = "std",
        derive(scale_info::TypeInfo, ink::storage::traits::StorageLayout)
    )]
    pub struct CommitmentMapKey {
        shard_id: u64,
        signer_id: u64,
    }

    /// hashmap of id pair -> commitments
    #[ink(storage)]
    #[derive(Default)]
    pub struct ContractCommitment {
        commitment_map: Mapping<CommitmentMapKey, Commitment>,
        n: u64,
    }

    /*
     * errors
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum CommitmentErrorType {
        EmptyCommitmentValue,
        CommitmentDoesNotExist,
        UnableToSerializeKeyVectorJson,
    }

    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub struct CommitmentError {
        error_type: CommitmentErrorType,
        message: String,
    }

    /*
     * success
     */
    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub enum CommitmentSuccessType {
        WriteCommitment,
        ReadCommitment,
        ReadCommitmentVector,
    }

    #[derive(Debug, PartialEq, Eq, scale::Encode, scale::Decode)]
    #[cfg_attr(feature = "std", derive(scale_info::TypeInfo))]
    pub struct CommitmentSuccess {
        success_type: CommitmentSuccessType,
        message: String,
    }

    impl ContractCommitment {
        /// constructor
        #[ink(constructor)]
        pub fn new() -> Self {
            Self {
                commitment_map: Mapping::new(),
                n: 0,
            }
        }

        /// write commitment
        #[ink(message)]
        pub fn write_commitment(
            &mut self,
            shard_id: u64,
            signer_id: u64,
            capital_r: String,
            w: String,
            capital_w: String,
        ) -> Result<CommitmentSuccess, CommitmentError> {
            // if commitment is empty, return error
            if capital_r.len() == 0 || w.len() == 0 || capital_w.len() == 0 {
                let message = "One of the commitment values (R,w,W) is empty".to_string();
                return Err(CommitmentError {
                    error_type: CommitmentErrorType::EmptyCommitmentValue,
                    message,
                });
            }

            // create commitment
            let c = Commitment {
                capital_r,
                w,
                capital_w,
            };

            // insert (shard_id, signer_id,t) -> commitment
            let p = CommitmentMapKey {
                shard_id,
                signer_id,
            };
            self.commitment_map.insert(p, &c);
            self.n += 1;

            // return success
            let message = self.create_json_key(shard_id, signer_id);
            Ok(CommitmentSuccess {
                success_type: CommitmentSuccessType::WriteCommitment,
                message,
            })
        }

        // create a json string from key values
        pub fn create_json_key(&mut self, shard_id: u64, signer_id: u64) -> String {
            let mut s: String = "{".to_string();
            s.push_str("\"shard_id\": ");
            s.push_str(&shard_id.to_string());
            s.push_str(",");
            s.push_str("\"signer_id\": ");
            s.push_str(&signer_id.to_string());
            s.push_str("}");
            s
        }

        /// read commitment
        #[ink(message)]
        pub fn read_commitment(
            &mut self,
            shard_id: u64,
            signer_id: u64,
        ) -> Result<Commitment, CommitmentError> {
            let commitment_map_key = CommitmentMapKey {
                shard_id,
                signer_id,
            };
            // if (id,t) key exists in map, return value, or error
            if let Some(commit) = self.commitment_map.get(commitment_map_key) {
                // return commitment
                Ok(commit)
            } else {
                // get fails, emit read fails
                let mut message = "Commitment does not exist for the key".to_string();
                let s = self.create_json_key(shard_id, signer_id);
                message.push_str(&s);
                // return error
                return Err(CommitmentError {
                    error_type: CommitmentErrorType::CommitmentDoesNotExist,
                    message,
                });
            }
        }

        // parse key values
        fn parse_key_value_u64(field: String) -> Result<u64, CommitmentErrorType> {
            match field.find(":") {
                Some(j) => {
                    let len = field.len() - (j + 1);
                    let mut field_value_str: String = field.chars().skip(j + 1).take(len).collect();
                    field_value_str = field_value_str.trim().to_string();
                    let result: u64 = field_value_str.parse().unwrap();
                    Ok(result)
                }
                None => Err(CommitmentErrorType::UnableToSerializeKeyVectorJson),
            }
        }

        // element -> commitment map key
        fn parse_commitment_key(
            ck_fields: String,
        ) -> Result<CommitmentMapKey, CommitmentErrorType> {
            // get values
            let field_list: Vec<&str> = ck_fields.split(",").collect();
            if field_list.len() != 2 {
                return Err(CommitmentErrorType::UnableToSerializeKeyVectorJson);
            }
            if let Ok(shard_id) = Self::parse_key_value_u64(field_list[0].to_string()) {
                if let Ok(signer_id) = Self::parse_key_value_u64(field_list[1].to_string()) {
                    // push to vec
                    let com_key = CommitmentMapKey {
                        shard_id,
                        signer_id,
                    };
                    return Ok(com_key);
                }
            }

            // if all 3 fields not parsed succesfully, return error
            Err(CommitmentErrorType::UnableToSerializeKeyVectorJson)
        }

        /// parse a json key vector (since serde and serde_json are too expensive, and do not work within smart contract)
        /// key is of the format [{cmk1},{cmk2},{cmk3},...{cmkn}]
        pub fn parse_key_vector(
            &mut self,
            key_vector_json: &String,
        ) -> Result<Vec<CommitmentMapKey>, CommitmentErrorType> {
            // get tokens
            let tok_list: Vec<&str> = key_vector_json.split("}").collect();

            // loop through all tokens, except last token
            let mut key_vec: Vec<CommitmentMapKey> = vec![];
            for i in 0..tok_list.len() - 1 {
                let tok: String = tok_list[i].to_string();
                // find open {
                if let Some(start) = key_vector_json.find("{") {
                    // get commitment key json info
                    let len = tok.len() - (start + 1);
                    let ck_fields: String = tok.chars().skip(start + 1).take(len).collect();
                    if let Ok(com_key) = Self::parse_commitment_key(ck_fields) {
                        key_vec.push(com_key);
                    } else {
                        return Err(CommitmentErrorType::UnableToSerializeKeyVectorJson);
                    }
                } else {
                    return Err(CommitmentErrorType::UnableToSerializeKeyVectorJson);
                }
            }

            Ok(key_vec)
        }

        /// reads several commitments given a vector of commitments specified in json
        #[ink(message)]
        pub fn read_commitment_vector(
            &mut self,
            key_vector_json: String,
        ) -> Result<Vec<Commitment>, CommitmentError> {
            // parse the json
            let json_result: Result<Vec<CommitmentMapKey>, CommitmentErrorType> =
                self.parse_key_vector(&key_vector_json);
            if let Ok(key_vec) = json_result {
                // read each of the commitments, and put into result vector
                let mut result: Vec<Commitment> = vec![];
                for i in 0..key_vec.len() {
                    let key = key_vec[i];
                    match self.read_commitment(key.shard_id, key.signer_id) {
                        // if ok, push the value and continue the loop
                        Ok(commit) => {
                            result.push(commit);
                        }
                        // if error occured when reading, return the error
                        Err(error) => return Err(error),
                    }
                }
                // return the result vector
                Ok(result)
            } else {
                let mut message =
                    "Unable to parse to deserialize the key vector with json key_vector_json"
                        .to_string();
                message += &key_vector_json;
                Err(CommitmentError {
                    error_type: CommitmentErrorType::UnableToSerializeKeyVectorJson,
                    message,
                })
            }
        }
    }

    /// unit tests
    #[cfg(test)]
    mod tests {
        /// Imports all the definitions from the outer scope so we can use them here.
        use super::*;

        use log::{error, info};

        use std::sync::Once;
        #[allow(dead_code)]
        static INIT: Once = Once::new();

        /// enable_logging runs before all tests, if uncommented on
        #[allow(dead_code)]
        pub fn enable_logging() {
            INIT.call_once(|| {
                // enable logging, since log defaults to silent
                std::env::set_var("RUST_LOG", "info");
                let _ = env_logger::try_init();
            });
        }

        /// test default constructor
        #[ink::test]
        fn constructor_works() {
            let contract_commitment = ContractCommitment::default();
            assert_eq!(contract_commitment.n, 0);
        }

        /// test write 1 commitment
        #[ink::test]
        fn write_one_commitment_success() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let signer_id: u64 = 1;
            let capital_r: String = "R1_01234".to_string();
            let w: String = "w_123".to_string();
            let capital_w: String = "W_456".to_string();

            // write the commitment
            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r.clone(),
                w.clone(),
                capital_w.clone(),
            ) {
                info!("result={:?}", result);
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }
        }

        /// test write 2 commitments
        #[ink::test]
        fn write_two_commitments_success() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let mut signer_id: u64 = 1;
            let mut capital_r: String = "R1_01234".to_string();
            let mut w: String = "w_123".to_string();
            let mut capital_w: String = "W_456".to_string();

            // write the 1st commitment
            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r.clone(),
                w.clone(),
                capital_w.clone(),
            ) {
                info!("result={:?}", result);
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }

            // write the 2nd commitment
            signer_id = 2;
            capital_r = "R2_56789".to_string();
            w = "w_456".to_string();
            capital_w = "W_456".to_string();
            if let Ok(result) =
                contract_commitment.write_commitment(shard_id, signer_id, capital_r, w, capital_w)
            {
                info!("result={:?}", result);
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }
        }

        /// test write 2 commitments with same key, but different values
        #[ink::test]
        fn write_two_commitments_same_key_success() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let signer_id: u64 = 1;
            let capital_r: String = "R1_01234".to_string();
            let w: String = "w_123".to_string();
            let capital_w: String = "W_456".to_string();

            // write the 1st commitment
            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r.clone(),
                w.clone(),
                capital_w.clone(),
            ) {
                info!("result={:?}", result);
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }

            // write the 2nd commitment w/ different values
            let capital_r2: String = "R2_56789".to_string();
            let w2: String = "w_456".to_string();
            let capital_w2: String = "W_456".to_string();
            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r2.clone(),
                w2.clone(),
                capital_w2.clone(),
            ) {
                info!("result={:?}", result);
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }

            // if we read the result, we should get the second/last commitment values
            // read 1 commitment
            if let Ok(c) = contract_commitment.read_commitment(shard_id, signer_id) {
                info!(
                    "read_commitment(shard_id={},signer_id={}): result={:?}",
                    shard_id, signer_id, c
                );
                assert_eq!(c.capital_r, capital_r2);
            } else {
                assert!(false);
            }
        }

        /// test write empty element fails
        #[ink::test]
        fn write_commitment_fails_empty() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let signer_id: u64 = 1;
            let capital_r: String = "R1_01234".to_string();
            let w_empty: String = "".to_string();
            let capital_w: String = "W_456".to_string();

            // if it's an error, check the type
            if let Err(result) = contract_commitment
                .write_commitment(shard_id, signer_id, capital_r, w_empty, capital_w)
            {
                info!("result={:?}", result);
                assert_eq!(contract_commitment.n, 0);
                assert_eq!(result.error_type, CommitmentErrorType::EmptyCommitmentValue);
            // if it's not an error, panic
            } else {
                assert!(false);
            }
        }

        /// test read commitment success
        #[ink::test]
        fn read_commitment_success() {
            // enable_logging();

            // write 1 commitment
            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let signer_id: u64 = 1;
            let capital_r: String = "R1_01234".to_string();
            let w: String = "w_123".to_string();
            let capital_w: String = "W_456".to_string();

            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r.clone(),
                w.clone(),
                capital_w.clone(),
            ) {
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }

            // read 1 commitment
            if let Ok(c) = contract_commitment.read_commitment(shard_id, signer_id) {
                info!(
                    "read_commitment(shard_id={},signer_id={}): result={:?}",
                    shard_id, signer_id, c
                );
                assert!(true);
            } else {
                assert!(false);
            }
        }

        /// test read commitment fail
        #[ink::test]
        fn read_commitment_fail() {
            // enable_logging();

            // write 1 commitment
            let mut contract_commitment = ContractCommitment::new();
            let shard_id: u64 = 123456;
            let signer_id: u64 = 1;
            let capital_r: String = "R1_01234".to_string();
            let w: String = "w_123".to_string();
            let capital_w: String = "W_456".to_string();

            if let Ok(result) = contract_commitment.write_commitment(
                shard_id,
                signer_id,
                capital_r.clone(),
                w.clone(),
                capital_w.clone(),
            ) {
                assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
            } else {
                assert!(false);
            }

            // read 1 commitment
            let bogus_shard_id: u64 = 19999;
            if let Ok(c) = contract_commitment.read_commitment(bogus_shard_id, signer_id) {
                info!(
                    "read_commitment(shard_id={},signer_id={}): result={:?}",
                    bogus_shard_id, signer_id, c
                );
                assert!(false);
            } else {
                assert!(true);
            }
        }

        /// test read commitment success
        #[ink::test]
        fn read_commitment_vector_success() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();

            // write commitments
            let capital_r: String = "R1_01234".to_string();
            let w: String = "w_123".to_string();
            let capital_w: String = "W_456".to_string();

            let mut keys: Vec<CommitmentMapKey> = vec![];

            // write commitments, and build json for keys
            let mut keys_json: String = "".to_string();
            keys_json.push_str("[");
            let n: u8 = 5;
            for i in 0..n {
                let shard_id: u64 = (100 + i).into();
                let signer_id: u64 = (10 + i).into();

                if let Ok(result) = contract_commitment.write_commitment(
                    shard_id,
                    signer_id,
                    capital_r.clone(),
                    w.clone(),
                    capital_w.clone(),
                ) {
                    assert_eq!(result.success_type, CommitmentSuccessType::WriteCommitment);
                } else {
                    error!("Unable to write commitment");
                    assert!(false);
                }

                // construct key to read
                let commitment_map_key = CommitmentMapKey {
                    shard_id,
                    signer_id,
                };
                keys.push(commitment_map_key);

                // create json key
                let mut keys_json = contract_commitment.create_json_key(shard_id, signer_id);
                if i < n - 1 {
                    keys_json.push_str(",");
                }
            }
            keys_json.push_str("]");

            info!("keys_json={:?}", keys_json);

            if let Ok(commitment_vector_json) =
                contract_commitment.read_commitment_vector(keys_json)
            {
                info!("commitment_vector_json={:?}", commitment_vector_json);
                assert!(true);
            } else {
                error!("Unable to read commitment vector");
                assert!(false);
            }
        }

        /// test read bogus json fails
        #[ink::test]
        fn read_commitment_vector_fail_bad_json() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();

            let bogus_keys_json: String = "{bogusjson}".to_string();
            match contract_commitment.read_commitment_vector(bogus_keys_json) {
                Ok(_result) => {
                    assert!(false);
                }
                error => {
                    info!("error={:?}", error);
                    assert!(true);
                }
            }
        }

        /// test read no such commitment
        #[ink::test]
        fn read_commitment_vector_fail_no_such_commitment() {
            // enable_logging();

            let mut contract_commitment = ContractCommitment::new();
            let mut keys: Vec<CommitmentMapKey> = vec![];

            let shard_id: u64 = 100;
            let signer_id: u64 = 10;

            // construct key to read
            let commitment_map_key = CommitmentMapKey {
                shard_id,
                signer_id,
            };
            keys.push(commitment_map_key);

            let keys_json: String = "[{\"shard_id\": 100, \"signer_id\": 99}]".to_string();

            // read commitment vector with a key that does not exist
            if let Ok(_commitment_vector_json) =
                contract_commitment.read_commitment_vector(keys_json)
            {
                assert!(false);
            } else {
                info!("No such commitment");
                assert!(true);
            }
        }
    }
}

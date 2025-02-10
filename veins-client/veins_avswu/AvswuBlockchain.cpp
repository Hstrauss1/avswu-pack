#include "AvswuBlockchain.h"
#include <spdlog/spdlog.h>

namespace avswu_blockchain {

AvswuBlockchain::AvswuBlockchain(AvswuGrpcClient *clientP) : _clientP(clientP) {
  if (clientP == nullptr) {
    spdlog::critical("Unable to connect to gRPC. clientP for gRPC is null");
    exit(-1);
  }
}

AvswuBlockchain::~AvswuBlockchain() {}

// write commitment values to blockchain
// (shard_id,signer_id) -> {R,w,W}
const bool AvswuBlockchain::write_commitment(uint64_t shard_id,
                                             uint32_t signer_id, mpz_t R,
                                             uint64_t w, mpz_t W) const {

  // get the strings of these integers base 16 for storage in blockchain

  // W -> str
  string R_str(mpz_get_str(nullptr, 16, R));

  // w-> str
  string w_str = uint64_t_to_str(w);

  // W -> str
  string W_str(mpz_get_str(nullptr, 16, W));

  // write commitments to blockchain
  spdlog::debug("client sent: write_commitment(shard_id={} signer_id={}, R={}, "
                "w={}, W={})",
                shard_id, signer_id, R_str, w_str, W_str);
  string result_str;
  const bool is_success = _clientP->AvswuBlockchainWriteCommitment(
      result_str, shard_id, signer_id, R_str, w_str, W_str);

  if (!is_success) {
    spdlog::error("unable write_commitment result_str={}", result_str);
    return false;
  }

  json result;
  try {
    // parse json result
    result = json::parse(result_str);
    // spdlog::debug("client received: result={}", result.dump());
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse json result from server for "
                  "write_commitment, result_str={}",
                  result_str);
    return false;
  }

  // check if write was successful
  if (result["successType"] == "WriteCommitment") {
    return true;
  }

  return false;
}

// convert results from utf8 strings->bytes->element_t's
void json_to_commitment_vector(vector<commitment_t> &L_com, json result) {

  for (size_t i = 0; i < result.size(); i++) {
    // get R, w, and W
    string capitalR = result[i]["capitalR"];
    string w = result[i]["w"];
    string capitalW = result[i]["capitalW"];

    // create commitment
    commitment_t c;
    mpz_init(c.R);
    mpz_set_str(c.R, capitalR.c_str(), 16);
    c.w = str_to_uint64_t(w);
    mpz_init(c.W);
    mpz_set_str(c.W, capitalW.c_str(), 16);

    // push to vec
    L_com.push_back(c);
  }
}

// read vector of commitments values from blockchain
const bool AvswuBlockchain::read_commitment_vector(
    vector<commitment_t> &L_com, vector<commitment_map_key_t> key_vec) const {

  // create json key_vec json
  json j(key_vec);
  string key_vec_json = j.dump();

  // make multiple attempts to avoid read before write, race condition
  // the blockchain has to complete n writes, and the n writes can come from
  // several distributed vehicles
  uint16_t attempt = 0;
  bool is_success = false;
  string result_str;
  while (!is_success && attempt < READ_COMMITMENT_MAX_ATTEMPTS) {
    // read commitment from blockchain
    spdlog::debug("client attempt={} of {}, sent: "
                  "read_commitment_vector(key_vec_json={})",
                  attempt, READ_COMMITMENT_MAX_ATTEMPTS, key_vec_json);
    is_success =
        _clientP->AvswuBlockchainReadCommitmentVector(result_str, key_vec_json);
    // spdlog::warn("result_str={}", result_str);
    // if not success, wait this thread
    if (!is_success) {
      std::this_thread::sleep_for(READ_COMMITMENT_DELAY);
    }
    attempt += 1;
  }

  if (attempt == READ_COMMITMENT_MAX_ATTEMPTS) {
    spdlog::error("Attempted the maximum of {} reads unable to read the vector "
                  "of commitments.",
                  READ_COMMITMENT_MAX_ATTEMPTS);
    return false;
  }

  json result;
  try {
    // parse json result
    result = json::parse(result_str);
    // spdlog::debug("client received: result={}", result.dump());
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse json result from server for "
                  "read_notificiation, result_str={}",
                  result_str);
    return false;
  }

  // parse json result
  try {

    // if the result is an error, return false
    if (result.is_object()) {
      string error_type = result["Err"]["errorType"];
      string message = result["Err"]["message"];
      // if commitment does not exist, log warning (as the commitment might be
      // available later), else log error as an error
      if (error_type == "CommitmentDoesNotExist") {
        spdlog::warn("error_type={}, message={}", error_type, message);
      } else {
        spdlog::error("error_type={}, message={}", error_type, message);
      }
      return false;
    }

    if (result.is_array()) {

      // convert results from utf8 string->element_t
      json_to_commitment_vector(L_com, result);

      // check if read was successful, and returned the same # of elements that
      // we requested
      bool is_success = (result.size() == key_vec.size());

      if (is_success) {
        spdlog::debug("client successfully received: a commitment vector with "
                      "{} commitments",
                      result.size());
      } else {
        spdlog::warn("client expected to receive a vector with {} commitments, "
                     "received {} commitments",
                     key_vec.size(), result.size());
      }
      return is_success;
    }

    // if not object nor array
    spdlog::critical(
        "unexpected data type returned from smart contract. result_str={}",
        result_str);
    return false;

  } catch (char *excp) {
    spdlog::error("unable to parse the results from the smart contract. "
                  "result_str={} {}",
                  result_str, excp);
    return false;
  }
}

const bool
AvswuBlockchain::write_notification(const AvswuNotification &notif) const {
  const uint64_t manufacturer_id = notif.manufacturer_id();
  const uint64_t software_id = notif.software_id();
  const uint64_t version_id = notif.version_id();
  const uint64_t create_time = notif.create_time() * SECOND_TO_NANOSECOND;
  const string notification_json = json(notif).dump();

  // make multiple attempts to write
  uint16_t attempt = 0;
  bool is_success = false;
  string result_str;
  while (!is_success && attempt < WRITE_NOTIFICATION_MAX_ATTEMPTS) {
    // write notification to blockchain
    spdlog::debug("client attempt={} of {}, sent: "
                  "write_notification for {}",
                  attempt, WRITE_NOTIFICATION_MAX_ATTEMPTS,
                  avswu_id(manufacturer_id, software_id, version_id));
    is_success = _clientP->AvswuBlockchainWriteNotification(
        result_str, manufacturer_id, software_id, version_id, create_time,
        notification_json);

    // parse json result
    json result;
    try {
      result = json::parse(result_str);
    } catch (const std::runtime_error &e) {
      spdlog::error("unable to parse json result from server for "
                    "write_notificiation, result_str={}",
                    result_str);
    }

    // if not success, wait this thread
    if (!is_success) {
      spdlog::warn("blockchain call failed, waiting for delay, resulStr={}",
                   result_str);
      std::this_thread::sleep_for(WRITE_NOTIFICATION_DELAY);
    }

    attempt += 1;
  }

  // if reached max attempts
  if (attempt == WRITE_NOTIFICATION_MAX_ATTEMPTS) {
    spdlog::error("Attempted the maximum of {} to write the notification ",
                  WRITE_NOTIFICATION_MAX_ATTEMPTS);
    return false;
  }

  return is_success;
}

// read a vector of notifications
const bool AvswuBlockchain::read_last_notification(
    AvswuNotification &notif, const uint64_t manufacturer_id,
    const uint64_t software_id, const double start_time) const {

  // make rpc call to server
  string result_str;
  const uint64_t start_time_ns = start_time * SECOND_TO_NANOSECOND;
  const bool is_success = _clientP->AvswuBlockchainReadLastNotification(
      result_str, manufacturer_id, software_id, start_time_ns);

  if (!is_success) {
    spdlog::error("unable to read_last_notification, result_str={}",
                  result_str);
    return false;
  }

  json result;
  try {
    // parse json result
    result = json::parse(result_str);
    // spdlog::debug("client received: result={}", result.dump());
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse json result from server for "
                  "read_last_notification, result_str={}",
                  result_str);
    return false;
  }

  // check if read was successful, if notificationJson exists
  if (!result.contains("notificationJson")) {
    return false;
  }

  // set notif from json
  json j_notif_string = result["notificationJson"];
  string notif_str;
  j_notif_string.get_to(notif_str);

  json j_notif = json::parse(notif_str);

  try {
    from_json(j_notif, notif);
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to set notification from json, manufacturer_id={}, "
                  "software_id={}",
                  manufacturer_id, software_id);
    return false;
  }

  return true;
}

const bool AvswuBlockchain::read_last_notification_list(
    deque<AvswuNotification> &notification_list,
    const vector<uint64_t> manufacturer_list, const uint64_t start_time,
    const size_t max_notification) {

  // make rpc call to server
  string result_str;
  const bool is_success = _clientP->AvswuBlockchainReadLastNotificationList(
      result_str, manufacturer_list, start_time);
  if (!is_success) {
    spdlog::error("unable to read_last_notification, result_str={} for ",
                  result_str);
    return false;
  }

  json result;
  try {
    // parse json result
    result = json::parse(result_str);
    // spdlog::debug("client received: result={}", result.dump());
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse json result from server for "
                  "read_last_notification, result_str={}",
                  result_str);
    return false;
  }

  // see if error was returned
  if (result.contains("Err")) {
    if (result["Err"]["errorType"] == "NotificationDoesNotExist") {
      spdlog::warn("read_last_notification_list: notification does not exist "
                   "for start_time=" +
                   to_string(start_time));
      spdlog::warn("read_last_notification_list:, result=" + result.dump(4));
      return false;
    } else {
      spdlog::error("read_last_notification_list error, result=" +
                    result.dump(4));
      return false;
    }
  }

  // check if read was successful, if result is an array of notifications
  if (!result.is_array()) {
    spdlog::error("notification list result is not an array, result=" +
                  result.dump(4));
    return false;
  }

  // check all element in array that they are notifications
  for (json::iterator it = result.begin(); it != result.end(); ++it) {
    auto elem = *it;
    if (!elem.contains("notificationJson")) {
      spdlog::error("notification list result element is not a notification");
      return false;
    }
  }

  // add each notification to the deque notification list
  for (json::iterator it = result.begin(); it != result.end(); ++it) {
    // get a notification
    json elem = *it;
    // set notification list string
    json j_notif_list_string = elem["notificationJson"];
    string notif_str;
    j_notif_list_string.get_to(notif_str);

    // parse the json for the notification
    json j_notif_list = json::parse(notif_str);

    // set notification list up to max notifications
    AvswuNotification notif;
    try {
      from_json(j_notif_list, notif);
    } catch (const std::runtime_error &e) {
      spdlog::error("unable to set notification from json");
      return false;
    }

    // add notification to deque
    notification_list.push_back(notif);

    // TEMP DEBUG
    // spdlog::error("notification_list={}", json(notif).dump(4));
  }

  return true;
}

// to/from json
void to_json(json &j, const AvswuBlockchain &bc) {
  // j = json{{"clientP", bc._clientP}};
}

void from_json(const json &j, AvswuBlockchain &bc) {
  // j.at("clientP").get_to(bc._clientP);
}

// output
ostream &operator<<(ostream &os, AvswuBlockchain &bc) {
  json j(bc);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_blockchain

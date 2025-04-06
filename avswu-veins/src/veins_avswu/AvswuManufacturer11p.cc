
#include "AvswuManufacturer11p.h"
#include "AvswuConfig.h"

#include "AvswuVeinsUtils.h"
#include <spdlog/spdlog.h>

#include "AvswuAV11pMessage_m.h"

using namespace veins;
using namespace std;
using namespace avswu_veins_utils;
using namespace avswu_config;

Define_Module(veins::AvswuManufacturer11p);

// requires 2 stage initialization for applicaitons
int AvswuManufacturer11p::numInitStages() const { return 2; }

// use destructors for any memory cleanup
AvswuManufacturer11p::~AvswuManufacturer11p() {
  // clean up memory usage
  delete _manufacturerP;

  // if ipfs is started, stop ipfs, if it is running
  if (ipfs_start()) {
    const bool is_success = avswu_utils::stop_ipfs_node();
    if (!is_success) {
      spdlog::error("unable to stop ipfs node");
    }
    set_ipfs_start(false);
  }
}

const bool
AvswuManufacturer11p::initialize_manufacturer(const uint64_t manufacturer_id,
                                              const size_t cache_max_size) {

  // if we havent created a gRPC client, create it
  if (!get_create_grpc_client()) {
    const bool is_success = create_grpc_client();
    if (!is_success) {
      const string log_msg = "unable to create gRPC client";
      AVSWU_LOG_ERROR(log_msg);
      return false;
    }
  }

  _manufacturerP = new AvswuManufacturerNode(manufacturer_id, cache_max_size,
                                             get_grpc_clientP());

  // if null node, return
  if (!_manufacturerP) {
    AVSWU_LOG_ERROR(string("_manufacturerP=null"));
    return false;
  }

  const string log_msg = "_manufacturerP->X()=" +
                         string(mpz_get_str(nullptr, 10, _manufacturerP->X()));
  AVSWU_LOG_DEBUG(log_msg);

  return true;
}

const vector<AvswuPacketShard> AvswuManufacturer11p::create_packet_list(
    const uint64_t software_id, const uint64_t version_id,
    const vector<char> input_data, const uint32_t k, const uint32_t m) {
  vector<AvswuPacketShard> man_packet_list;
  try {
    man_packet_list = _manufacturerP->create_packet_list(
        software_id, version_id, input_data, k, m);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
  }

  spdlog::debug("packet_list.size()={}", man_packet_list.size());

  return man_packet_list;
}

const vector<string> AvswuManufacturer11p::ipfs_write_packet_list(
    const vector<AvswuPacketShard> man_packet_list) {
  vector<string> cid_hash_list;
  for (size_t i = 0; i < man_packet_list.size(); i++) {
    string cid_hash;
    const bool is_success =
        _manufacturerP->ipfs_write_packet(cid_hash, man_packet_list[i]);
    if (is_success && (cid_hash.size() > 0)) {
      spdlog::debug("cid()={}", cid_hash);
      cid_hash_list.push_back(cid_hash);
    }
  }

  return cid_hash_list;
}

const bool AvswuManufacturer11p::write_notification(
    const vector<AvswuPacketShard> man_packet_list,
    const vector<string> cid_hash_list, const uint64_t software_id,
    const uint64_t version_id, const vector<char> input_data, const uint32_t k,
    const uint32_t m, const double create_time) {

  /*
   * manufacturer creates notification, and writes notification to
   * blockchain
   */
  AvswuNotification notif;
  // set the notification
  const bool is_success_set_notif = _manufacturerP->set_notification(
      notif, man_packet_list, cid_hash_list, _manufacturerP->id(), software_id,
      version_id, input_data, k, m, create_time);
  if (!is_success_set_notif) {
    const string log_msg = "set notification failed";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  const string log_msg = "notif=" + json(notif).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // write notification to the blockchain
  const bool is_success_write_notif =
      _manufacturerP->blockchain_write_notification(notif);
  if (!is_success_write_notif) {
    const string log_msg = "write notification failed";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  return true;
}

const bool AvswuManufacturer11p::initialize_stage1() {

  // set node type
  _node_type = node_type_t::manufacturer;

  // ui icon update
  findHost()->getDisplayString().setTagArg("i", 0,
                                           "veins/avswu_icons/manufacturer");
  // color icon
  findHost()->getDisplayString().setTagArg("i", 1, "white");
  // icon size
  // findHost()->getDisplayString().setTagArg("is", 0, "vl");
  findHost()->getDisplayString().setTagArg("is", 0, "s");

  // TODO: add support for multiple manufacturers
  const uint64_t manufacturer_id = 100;

  // if we have not started ipfs yet, start ipfs
  if (!ipfs_start()) {
    bool is_success = avswu_utils::start_ipfs_node();
    if (!is_success) {
      spdlog::error("unable to start ipfs node");
    }
    set_ipfs_start(true);
  }

  // init man
  const bool is_init_success = initialize_manufacturer(
      manufacturer_id, AVSWU_MANUFACTURER_MAX_CACHE_SIZE);
  if (!is_init_success) {
    const string log_msg = "initialize manufacturer failed";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // if we have not read the config.json, read it
  if (!get_read_avswu_config()) {
    const bool is_read_success = read_avswu_config();
    if (is_read_success) {
      const string log_msg = "read config.json success";
      AVSWU_LOG_INFO(log_msg);

    } else {
      const string log_msg = "reading config.json failed";
      AVSWU_LOG_ERROR(log_msg);
      return false;
    }
  }

  // get info from .ned and .ini file, and set variable
  auto value = par("softwareUpdateSizeMB").intValue();
  this->software_update_size_mb = value;
  string log_msg =
      "software_update_size_MB=" + to_string(this->software_update_size_mb);
  AVSWU_LOG_INFO(log_msg);

  // get erasure code k, m
  value = par("erasureCodeK").intValue();
  this->erasure_code_k = value;
  log_msg = "erasure_code_k=" + to_string(this->erasure_code_k);
  AVSWU_LOG_INFO(log_msg);

  value = par("erasureCodeM").intValue();
  this->erasure_code_m = value;
  log_msg = "erasure_code_m=" + to_string(this->erasure_code_m);
  AVSWU_LOG_INFO(log_msg);

  return true;
}

const bool AvswuManufacturer11p::send_new_software_update() {
  /*
  we update the version for the softare item below, as it has a inventory with
 only 1 item in it for corresponding avs. (config_inventory_index = 12, for
 manufacturer_id = 100) from config.json since it has only 1 software in it's
 inventory
{
    "index": 2,
    "software_index_list": [3]
  },
          {
          "create_time": 0,
          "file": {
            "directory": "/home/gsolomon/avswu/veins-client/input/mb",
            "hash": {
              "type": 1,
              "value":
  "d6292685b380e338e025b3415a90fe8f9d39a46e7bdba8cb78c50a338cefca741f69e4e46411c32de1afdedfb268e579a51f81ff85e56f55b0ee7c33fe8c25c9"
            },
            "name": "test_data_0001.data"
          },
          "index": 3,
          "k": 6,
          "m": 3,
          "software_id": 1003,
          "version_id": 0
        },
 ** this is the software we are updating in the manufacturer node **
*/

  const uint64_t config_software_id = 1003;

  // read the config .json file
  if (!get_read_avswu_config()) {
    spdlog::error("config.json not read");
    return false;
  }

  // set software item based on config.json
  software_item_t sw_item;
  const bool found_sw_item = get_avswu_configP()->get_software_item(
      sw_item, _manufacturerP->id(), config_software_id);
  if (!found_sw_item) {
    spdlog::error("unable to find software item in config for "
                  "manufacturer_id={}, software_id={}, version_id={}",
                  _manufacturerP->id(), config_software_id);
    return false;
  }

  // increase the version id (since we are creating a new version)
  sw_item.version_id++;

  // get values from sw config item
  const uint64_t software_id = sw_item.software_id;
  // create a new s/w version (by increasing version id ) as the new version
  // will be stored on the blockchain via the notification
  const uint64_t version_id = sw_item.version_id;
  // get k,m erasure encoding params from simulation parameters (ignore
  // config.json k,m values)
  const uint32_t k = this->erasure_code_k;
  const uint32_t m = this->erasure_code_m;
  // set input file
  const string input_file = sw_item.file.directory + "/" + sw_item.file.name;

  /*
   * manufacturer read file, create packets with shards
   */
  // read file
  vector<char> input_data;
  try {
    const string log_msg = "input_file=" + input_file;
    AVSWU_LOG_DEBUG(log_msg);
    input_data = _manufacturerP->read_file(input_file);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
    const string log_msg = "unable to read the input_file=" + input_file;
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // create packets
  vector<AvswuPacketShard> man_packet_list =
      create_packet_list(software_id, version_id, input_data, k, m);
  if (man_packet_list.size() == 0) {
    const string log_msg = "unable to create packetes";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // write to ipfs
  // manufacturer write packets to ipfs
  vector<string> cid_hash_list = ipfs_write_packet_list(man_packet_list);
  if (cid_hash_list.size() != k + m) {
    const string log_msg = "ipfs write packetes failed";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // write notification of the new softwware update

  // create sim time greater than 0
  double create_time = max(0.1, simTime().dbl());

  const bool write_notification_success =
      write_notification(man_packet_list, cid_hash_list, software_id,
                         version_id, input_data, k, m, create_time);
  if (!write_notification_success) {
    return false;
  }

  // update the config struct info for this specific software
  // to the latest/new version of the software
  get_avswu_configP()->set_software_item(_manufacturerP->id(), sw_item);

  const string log_msg =
      "sent a new software update for " +
      avswu_id(_manufacturerP->id(), sw_item.software_id, sw_item.version_id) +
      "create_time=" + to_string(create_time);
  AVSWU_LOG_INFO(log_msg);

  // if all success, return true
  return true;
}

// set log level at runtime
void AvswuManufacturer11p::set_spdlog_level() {
  auto log_level = cComponent::getLogLevel();
  AVSWU_LOG_INFO(string("log_level=" + to_string(log_level)));

  string log_msg;

  switch (log_level) {
  case omnetpp::LOGLEVEL_OFF:
    // only show errors
    log_msg = "spdlog level=err";
    AVSWU_LOG_INFO(log_msg);
    spdlog::set_level(spdlog::level::err);
    break;
  case LOGLEVEL_DEBUG:
    log_msg = "spdlog level=debug";
    AVSWU_LOG_INFO(log_msg);
    spdlog::set_level(spdlog::level::debug);
    break;
  case LOGLEVEL_INFO:
    log_msg = "spdlog level=info";
    AVSWU_LOG_INFO(log_msg);
    spdlog::set_level(spdlog::level::info);
    break;
  default:
    log_msg = "spdlog level=off";
    AVSWU_LOG_INFO(log_msg);
    spdlog::set_level(spdlog::level::off);
  }
}

void AvswuManufacturer11p::initialize(int stage) {

  // set log level at runtime
  set_spdlog_level();

  const string log_msg =
      "AvswuManufacturer11p:initialize, stage=" + to_string(stage);
  AVSWU_LOG_DEBUG(log_msg);

  DemoBaseApplLayer::initialize(stage);

  // init structs after myId and other veins structs are set
  if (stage == 1) {
    const bool is_success_stage1 = initialize_stage1();
    if (is_success_stage1) {
      const string log_msg = "AvswuManufacturer11p:initialize_stage1() success";
      AVSWU_LOG_DEBUG(log_msg);
    } else {
      const string log_msg = "AvswuManufacturer11p:initialize_stage1() failed";
      AVSWU_LOG_ERROR(log_msg);
    }
  }
}

void AvswuManufacturer11p::finish() {}

void AvswuManufacturer11p::handleSelfMsg(cMessage *msg) {
  // string log_msg = "handleSelfMsg()";
  // AVSWU_LOG_INFO(log_msg);

  // create duplicate and schedule it to repeat
  if (AvswuAV11pMessage *wsm = dynamic_cast<AvswuAV11pMessage *>(msg)) {
    // schedule self-message
    scheduleAt(simTime() + NEW_SOFTWARE_UPDATE_PERIOD, wsm);

    // send an initial software update
    const bool sent_update = send_new_software_update();

  } else {
    DemoBaseApplLayer::handleSelfMsg(msg);
  }
}

void AvswuManufacturer11p::handlePositionUpdate(cObject *obj) {
  // string log_msg = "handlePositionUpdate() called";
  // AVSWU_LOG_ERROR(log_msg);

  // create message which will be repeated
  AvswuAV11pMessage *wsm = new AvswuAV11pMessage();

  // if null wsm, return
  if (!wsm) {
    AVSWU_LOG_ERROR(string("wsm=null"));
    return;
  }

  populateWSM(wsm);
  wsm->setSenderId(myId);
  wsm->setPayload("self-message-new-version");
  wsm->setSenderType("manufacturer");

  // scheduleAt() calls handleSelfMsg() in base class
  scheduleAt(NEW_SOFTWARE_UPDATE_START, wsm);
}

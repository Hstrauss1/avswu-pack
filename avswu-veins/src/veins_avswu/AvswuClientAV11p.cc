#include "AvswuClientAV11p.h"
#include "AvswuConfig.h"
#include "AvswuOpenSSL.h"
#include "AvswuPacketInventory.h"
#include "AvswuPacketNotificationList.h"
#include "AvswuPacketShard.h"
#include "AvswuPacketShardRequestList.h"

#include "AvswuDeliveryAV11p.h"
#include "AvswuRSU11p.h"

#include "AvswuVeinsUtils.h"
#include <spdlog/spdlog.h>

#include "AvswuAV11pMessage_m.h"

using namespace veins;
using namespace std;
using namespace avswu_veins_utils;
using namespace avswu_config;
using namespace avswu_packet_inventory;
using namespace avswu_packet_support_list;
using namespace avswu_notification_list_map;

Define_Module(veins::AvswuClientAV11p);

// requires 2 stage initialization for applicaitons
int AvswuClientAV11p::numInitStages() const { return 2; }

const bool AvswuClientAV11p::initialize_av(const uint64_t av_id,
                                           const size_t cache_max_size) {

  _avP = new AvswuAVNode(av_id, cache_max_size);
  if (!_avP) {
    const string log_msg = "unable to create av node";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  const string log_msg =
      "_avP->X()=" + string(mpz_get_str(nullptr, 10, _avP->X()));
  AVSWU_LOG_DEBUG(log_msg);

  return true;
}

const bool AvswuClientAV11p::initialize_stage1() {

  // set node type
  _node_type = node_type_t::client_av;

  // set node ui status, and update ui color/image
  _ui_status = no_shards;
  // set icon image
  findHost()->getDisplayString().setTagArg("i", 0, "veins/avswu_icons/av_red");

  // TODO: add support for multiple avs
  // manufacturer for this av
  _manufacturer_id = 100;

  /*
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
*/
  // software configuration for this av
  _inventory_index = 2;

  // set inventory pointer
  _inventoryP =
      get_avswu_configP()->inventoryP(_manufacturer_id, _inventory_index);

  const uint64_t av_id = 2000 + myId;

  // init rsu
  const bool is_init_success =
      initialize_av(av_id, AVSWU_CLIENT_AV_MAX_CACHE_SIZE);
  if (!is_init_success) {
    const string log_msg = "initialize av failed";
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

  // set uid label, if needed
  if (AVSWU_NODE_UI_LABEL_TO_ID) {
    const string ui_msg = to_string(_avP->id());
    findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());
  }

  // set depart time, so we can subtract it from sw delivery time
  _statistics.depart_time = simTime().dbl();

  // increase av count
  _client_av_count++;
  // record count
  recordScalar("client_av_count", _client_av_count);

  // get info from .ned and .ini file, and set variable
  auto value = par("softwareUpdateSizeMB").intValue();
  this->software_update_size_mb = value;
  const string log_msg =
      "software_update_size_MB=" + to_string(this->software_update_size_mb);
  AVSWU_LOG_INFO(log_msg);

  return true;
}

void AvswuClientAV11p::initialize(int stage) {
  const string log_msg = "initialize, stage=" + to_string(stage);
  AVSWU_LOG_INFO(log_msg);

  DemoBaseApplLayer::initialize(stage);

  if (stage == 0) {
    _current_subscribed_service_id = -1;
  }

  // by stage 1, veins structs are setup, and myId is set
  if (stage == 1) {
    const bool is_success_stage1 = initialize_stage1();
    if (is_success_stage1) {
      const string log_msg = "initialize_stage1() success";
      AVSWU_LOG_DEBUG(log_msg);
    } else {
      const string log_msg = "initialize_stage1() failed";
      AVSWU_LOG_ERROR(log_msg);
    }
  }
}

// records scalars (totals)
void AvswuClientAV11p::stats_record_scalar() {
  recordScalar("packet_sent", _statistics.packet_sent);
  recordScalar("packet_received", _statistics.packet_received);
  recordScalar("shard_sent", _statistics.shard_sent);
  recordScalar("shard_received", _statistics.shard_received);
  recordScalar("sw_update_received", _statistics.sw_update_received);
  recordScalar("depart_time", _statistics.depart_time);

  // speed log stats
  string log_msg = "packet_sent=" + to_string(_statistics.packet_sent);
  AVSWU_SPDLOG_INFO(log_msg);
  log_msg = "packet_received=" + to_string(_statistics.packet_received);
  AVSWU_SPDLOG_INFO(log_msg);
  log_msg = "shard_sent=" + to_string(_statistics.shard_sent);
  AVSWU_SPDLOG_INFO(log_msg);
  log_msg = "shard_received=" + to_string(_statistics.shard_received);
  AVSWU_SPDLOG_INFO(log_msg);
  log_msg = "sw_update_received=" + to_string(_statistics.sw_update_received);
  AVSWU_SPDLOG_INFO(log_msg);
}

// log histogram
void AvswuClientAV11p::log_histogram_ns(const string hist_name,
                                        omnetpp::cHistogram &hist) {

  auto min_val = static_cast<uint64_t>(hist.getMin());
  string log_msg = hist_name + " minimum=" + to_string(min_val);
  AVSWU_SPDLOG_INFO(log_msg);

  auto max_val = static_cast<uint64_t>(hist.getMax());
  log_msg = hist_name + " maximum=" + to_string(max_val);
  AVSWU_SPDLOG_INFO(log_msg);

  auto mean_val = static_cast<uint64_t>(hist.getMean());
  log_msg = hist_name + " mean=" + to_string(mean_val);
  AVSWU_SPDLOG_INFO(log_msg);
}

// records histograms (min,max,mean)
void AvswuClientAV11p::stats_record_histogram() {
  if (_statistics.shard_received > 0) {
    const string hist_name = "shard_time_to_receive";
    _statistics.shard_time_to_receive.recordAs(hist_name.c_str());
    log_histogram_ns(hist_name, _statistics.shard_time_to_receive);
  } else {
    AVSWU_LOG_WARN("no shards were received, therefore no data collected for "
                   "shard_time_to_receive");
  }

  // if sw was delivered
  if (_statistics.sw_update_received > 0) {
    // record sw time to update, from create_time
    string hist_name = "sw_time_to_update_from_create_time";
    _statistics.sw_time_to_update_from_create_time.recordAs(hist_name.c_str());
    log_histogram_ns(hist_name, _statistics.sw_time_to_update_from_create_time);
    // record sw time to update, from shard request time
    hist_name = "sw_time_to_update_from_client_shard_request_time";
    _statistics.sw_time_to_update_from_client_shard_request_time.recordAs(
        hist_name.c_str());
    log_histogram_ns(
        hist_name,
        _statistics.sw_time_to_update_from_client_shard_request_time);
  } else {
    if (node_type() == node_type_t::client_av) {
      AVSWU_LOG_WARN("no software was received by client av, therefore no data "
                     "collected for "
                     "sw_time_to_update_from_create_time and "
                     "sw_time_to_update_from_client_shard_request_time");
    }
  }

  if (_statistics.shard_received > 0) {
    const string hist_name = "shard_hop_count";
    _statistics.shard_hop_count.recordAs(hist_name.c_str());
    log_histogram_ns(hist_name, _statistics.shard_hop_count);
  } else {
    AVSWU_LOG_WARN("no shards were received, therefore no data collected for "
                   "shard_hop_count");
  }
}

// record stats
void AvswuClientAV11p::finish() {
  // record scalar stats
  stats_record_scalar();

  // write histograms
  stats_record_histogram();
}

// destructor for memory cleanup
AvswuClientAV11p::~AvswuClientAV11p() {

  // decrease av count
  _client_av_count--;

  // free av node
  delete _avP;

  // free shard request list
  delete _shard_request_listP;

  // free any cached notifications
  _notification_listP_map.clear();
}

// send inventory
// respond to support list by sending inventory to rsu
// AV creates s/w inventory packet for (av,RSU) by reading the config
const bool AvswuClientAV11p::send_inventory(const uint64_t to_id, mpz_t &to_X) {

  // if null node, return
  if (!_avP) {
    AVSWU_LOG_ERROR(string("_avP=null"));
    return false;
  }

  AvswuPacketInventory packet_inventory(_avP->id(), _avP->X(), to_id, to_X,
                                        _node_type);

  // av get it's s/w configuration (this happens upon av
  // creation/initialization in practice)
  const bool is_payload_set = packet_inventory.set_payload(
      get_avswu_configP(), _manufacturer_id, _inventory_index);
  if (!is_payload_set) {
    const string log_msg = "unable to set payload";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }
  const string log_msg = "packet_inventory={}" + json(packet_inventory).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // send inventory
  packet_inventory.statistics().set_send_time(simTime().dbl());
  const bool is_success = send_packet(packet_inventory, "av");
  // update packet sent stats
  _statistics.packet_sent++;

  return is_success;
}

void AvswuClientAV11p::onWSA(DemoServiceAdvertisment *wsa) {
  // Your application has received a service advertisement from another car or
  // RSU
  // code for handling the message goes here, see TraciDemo11p.cc for examples
  string log_msg = "onWSA() called";
  AVSWU_LOG_INFO(log_msg);

  // if null wsa, return
  if (!wsa) {
    AVSWU_LOG_ERROR(string("wsa=null"));
    return;
  }

  if (_current_subscribed_service_id == -1) {
    mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
    _current_subscribed_service_id = wsa->getPsid();
    if (currentOfferedServiceId != wsa->getPsid()) {
      stopService();
      startService(static_cast<Channel>(wsa->getTargetChannel()),
                   wsa->getPsid(), "Mirrored Traffic Service");
    }
  }
}

// if the av's manufacturer is in support list
const bool AvswuClientAV11p::is_supported(AvswuPacketSupportList &packet) {
  bool found = false;
  size_t i = 0;
  auto man_list = packet.manufacturer_list();
  while (i < man_list.size() and !found) {
    auto man_support = man_list[i];
    if (man_support.manufacturer_id == _manufacturer_id) {
      found = true;
    }
    i++;
  }
  return found;
}

// parse and handle support list
const bool AvswuClientAV11p::handle_support_list(const string payload) {

  // parse payload into support list
  AvswuPacketSupportList packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet support list, exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  // ignore support lists from RSU, only accepting shards from delivery AV for
  // data
  // ** this will help disaggregate simulation data for research paper **
  if (packet.header().from_type() == node_type_t::rsu) {
    const string log_msg = "ignoring RSU support list, only accepting shards "
                           "from delivery AVs";
    AVSWU_LOG_WARN(log_msg);
    // return true to prevent unnecessary error logs
    return true;
  }

  const string log_msg =
      "received payload=support list, not printing json since it is a "
      "long data set";
  AVSWU_LOG_DEBUG(log_msg);

  // update delivery range set for this packet
  update_delivery_range_set(packet);

  /*
    1a. if notification list in cache, do nothing b/c we should already be
    receiving shards from the RSU or delivery AV 1b. if notification list is not
    in cache, process support list
    */
  const bool has_notif_list = notification_list_cache_contains(packet);
  if (has_notif_list) {
    // do nothing
    const string log_msg = "ignoring support list packet, since we should "
                           "be receiving shards";
    AVSWU_LOG_INFO(log_msg);
    return true;
  }

  // check if the av's manufacturer is in support list
  const bool found = is_supported(packet);
  if (!found) {
    const string log_msg = "av's manufacturer is not supported by " +
                           avswu_from_id(packet.header());
    AVSWU_LOG_ERROR(log_msg);
    return true;
  }

  // send av inventory to rsu/devliery-av
  const bool is_success =
      send_inventory(packet.header().from_id(), packet.header().from_X());

  return is_success;
}

// parse and handle notification list
const bool AvswuClientAV11p::handle_notification_list(const string payload) {
  // allocate a new notification
  notification_listP_t packetP(new AvswuPacketNotificationList());
  if (!packetP) {
    const string log_msg = "unable to allocated a new notification list";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // parse payload into notification list
  try {
    *packetP = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet support list, exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // point-to-point (node-to-node): if packet not sent to this av, ignore
  // packet
  if (!is_recipient(packetP.get())) {
    const string log_msg = "wrong recipient, notification_list packet was sent "
                           "to a node with id=" +
                           to_string(packetP->header().to_id());
    AVSWU_LOG_DEBUG(log_msg);
    return true;
  }

  // set the receive time for the packet
  packetP->statistics().set_receive_time(simTime().dbl());

  string log_msg = "received payload=" + json(*packetP).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // insert packet into notification cache
  notification_list_cache_insert(packetP);

  // if shard request list, exists delete it, and create a new one
  if (_shard_request_listP) {
    delete _shard_request_listP;
  }

  // create list of shards based on notification, and which shards are not in
  // our cache
  _shard_request_listP = _avP->create_shard_request_list(
      packetP->header().from_id(), packetP->header().from_X(), _node_type,
      *packetP);

  // if create files, return
  if (!_shard_request_listP) {
    const string log_msg = "unable to create shard request list";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // set from type
  _shard_request_listP->header().set_from_type(_node_type);

  // set client shard request time (the time that the client requested the
  // software for statistics)
  _shard_request_listP->statistics().set_client_shard_request_time(
      simTime().dbl());

  log_msg = "sending shard_request_list packet=" +
            json(*_shard_request_listP).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // send shard request list to rsu
  _shard_request_listP->statistics().set_send_time(simTime().dbl());
  const bool is_success = send_packet(*_shard_request_listP, "av");
  // update packet sent stats
  _statistics.packet_sent++;

  log_msg = "sent a shard_request_list packet to id=" +
            to_string(_shard_request_listP->header().to_id());
  AVSWU_LOG_INFO(log_msg);

  return is_success;
}

// write (decoded) data to disk
const bool AvswuClientAV11p::write_file(const vector<char> &decoded_output_data,
                                        const uint64_t manufacturer_id,
                                        const uint64_t software_id,
                                        const uint64_t version_id) {

  software_item_t *sw_itemP =
      get_avswu_configP()->software_itemP(manufacturer_id, software_id);
  if (!sw_itemP) {
    spdlog::error("unable to find software item for {}",
                  avswu_id(manufacturer_id, software_id));
    return false;
  }

  string input_file = sw_itemP->file.name;
  const uint64_t av_id = _avP->id();

  try {
    string output_file_path = avswu_utils::output_file_path(
        input_file, av_id, manufacturer_id, software_id, version_id);
    spdlog::info("writing to output_file_path={}", output_file_path);
    _avP->write_file(output_file_path, decoded_output_data);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
    return false;
  }

  return true;
}

// compare input data with output data
const bool AvswuClientAV11p::compare_input_output(
    const vector<char> &decoded_output_data, const uint64_t manufacturer_id,
    const uint64_t software_id, const uint64_t version_id) {

  // get input file hash from config
  software_item_t *sw_itemP =
      get_avswu_configP()->software_itemP(manufacturer_id, software_id);
  if (!sw_itemP) {
    spdlog::error("unable to find software item for {}",
                  avswu_id(manufacturer_id, software_id));
    return false;
  }

  // get input data hash
  string input_file_hash = sw_itemP->file.hash.value;

  // compute output data hash

  // convert data to string
  string data_str =
      string(decoded_output_data.begin(), decoded_output_data.end());

  // hash the data string
  string output_data_hash;
  const bool is_success = avswu_open_ssl::hash_by_type(
      output_data_hash, data_str, AVSWU_DATA_HASH_ALGORTITHM);
  if (!is_success) {
    spdlog::error("sha hash generation failed");
  }

  // compare the input hash to hash(output)
  bool input_output_match = output_data_hash == input_file_hash;

  return input_output_match;
}

// process, write, and compare data versus input data
const bool AvswuClientAV11p::process_decoded_data(
    vector<char> &decoded_output_data, const double earliest_shard_request_time,
    const uint64_t manufacturer_id, const uint64_t software_id,
    const uint64_t version_id, const double time_delivered,
    const double create_time) {
  // remove all shard requests for software version since the sw version
  // has been decoded succesfully. removing the request prevents repeating m
  // decodes when the rsu sends all k+m shards.
  const bool removed_software_request =
      _shard_request_listP->remove(manufacturer_id, software_id, version_id);
  if (!removed_software_request) {
    const string log_msg =
        "unable to remove request for decoded software update for " +
        avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // write decoded software update to disk
  if (WRITE_TO_DISK) {
    const bool write_success = write_file(decoded_output_data, manufacturer_id,
                                          software_id, version_id);
    if (!write_success) {
      const string log_msg =
          "unable to write decoded software update to disk for " +
          avswu_id(manufacturer_id, software_id, version_id);
      AVSWU_LOG_ERROR(log_msg);
      return false;
    }
  }

  // compare input_file hash to hash of input data, if successful update
  // statistics
  const bool compare_success = compare_input_output(
      decoded_output_data, manufacturer_id, software_id, version_id);
  if (compare_success) {
    /*
     * update statistics
     */
    // update sw_update_received
    _statistics.sw_update_received++;

    // update time to deliver a software update. using the last shard
    // create_time is ok, since all shards were created at (nearly) the same
    // time.
    // the earliest time a s/w update could have been received (max of when it
    // was created, and when the vehicle departed and joined the simulation)
    const double earliest_time = max(create_time, _statistics.depart_time);
    _statistics.sw_time_to_update_from_create_time.collect(time_delivered -
                                                           create_time);
    // update time to delivery software update since request shards time
    _statistics.sw_time_to_update_from_client_shard_request_time.collect(
        time_delivered - earliest_shard_request_time);

    AVSWU_LOG_INFO(
        "*******************************************************************");
    const string log_msg = "SUCCESS software has been updated to " +
                           avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_INFO(log_msg);
    AVSWU_LOG_INFO(
        "*******************************************************************");
  } else {
    const string log_msg = "input and output files are not the same, and do "
                           "not have the same hash for " +
                           avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
  }

  // update ui label
  if (AVSWU_NODE_UI_LABEL_DECODE_SUCCESS) {
    findHost()->getDisplayString().setTagArg("t", 0, "decode success");
  }

  // set icon image
  if (AVSWU_NODE_UI_COLOR) {
    findHost()->getDisplayString().setTagArg("i", 0,
                                             "veins/avswu_icons/av_green");
  }

  // set ui sw update success status
  _ui_status = sw_update_success;

  return true;
}

// handle all possible decode results
const bool AvswuClientAV11p::handle_decode_result(
    const decode_status_t status, vector<char> &decoded_output_data,
    const double earliest_shard_request_time, const uint64_t manufacturer_id,
    const uint64_t software_id, const uint64_t version_id,
    const double time_delivered, const double create_time) {

  bool is_success;
  string log_msg;

  // handle all decode types
  string desc = _avP->decode_status_description(status);
  switch (status.code) {
  case NO_SHARDS:
    // this case should never happen, since we just inserted a shard
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  case NOT_ENOUGH_SHARDS:
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_INFO(log_msg);
    is_success = true;
    break;
  case INVALID_PARAMS:
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  case DECODE_SUCCESS:
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_INFO(log_msg);
    is_success = process_decoded_data(
        decoded_output_data, earliest_shard_request_time, manufacturer_id,
        software_id, version_id, time_delivered, create_time);
    break;
  case DECODE_FAIL:
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  default:
    log_msg =
        desc + " for " + avswu_id(manufacturer_id, software_id, version_id);
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  }

  return is_success;
}

// parse and handle shard
const bool AvswuClientAV11p::handle_shard(const string payload) {

  // parse payload into shard
  AvswuPacketShard packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet support list, exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // point-to-point (node-to-node): if packet not sent to this av, ignore
  // packet
  if (!is_recipient(&packet)) {
    const string log_msg =
        "wrong recipient, shard packet was sent to a node with id=" +
        to_string(packet.header().to_id());
    AVSWU_LOG_DEBUG(log_msg);
    return true;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  string log_msg =
      "received payload=shard, not printing json since it is a long "
      "data set";
  AVSWU_LOG_DEBUG(log_msg);

  // set time delivered to current time for stats
  const double time_delivered = simTime().dbl();

  // insert shard in cache
  const bool insert_success = _avP->insert(packet);

  // if shard not requested by my request list, stop here
  if (!_shard_request_listP) {
    const string log_msg = "av has no request list, but it has cached a shard "
                           "that it did not request for "
                           "distribution to other avs";
    AVSWU_LOG_INFO(log_msg);
    return true;
  }

  // remove the shard received from the shard request lists
  const uint64_t manufacturer_id = packet.shard().manufacturer_id();
  const uint64_t software_id = packet.shard().software_id();
  const uint64_t version_id = packet.shard().version_id();
  const uint64_t shard_id = packet.shard().shard_id();

  const bool removed_shard_request = _shard_request_listP->remove(
      manufacturer_id, software_id, version_id, shard_id);

  // describe who sent the shard
  log_msg = "av received a shard that it requested from " +
            avswu_from_id(packet.header());
  AVSWU_LOG_INFO(log_msg);

  // update stats since at this point the shard has been accepted by the vehicle

  // update the histogram for tracking hop_count per shard
  _statistics.shard_hop_count.collect(packet.statistics().hop_count());

  // update time to receive a requested shard
  const double create_time = packet.header().create_time();
  _statistics.shard_time_to_receive.collect(time_delivered - create_time);

  // update shard received stats
  _statistics.shard_received++;

  // handle case when shard received, when > k shards
  if (!removed_shard_request) {
    string log_msg;
    switch (node_type()) {
    case node_type_t::delivery_av:
      log_msg = "av has cached a shard for delivery to other avs";
      break;
    case node_type_t::client_av:
      log_msg = "av has cached an additional shard, > k shards, for decoding";
      break;
    default:
      log_msg = "av has cached an additional shard, unknown purpose.";
    }
    AVSWU_LOG_INFO(log_msg);
    return true;
  }

  // if the av is a client av, we should try to decode the result
  // else (delivery), av does not decode
  bool is_success = false;
  if (node_type() == node_type_t::client_av) {
    // attempt to decode and get status
    vector<char> decoded_output_data;
    double earliest_shard_request_time;
    const decode_status_t status =
        _avP->decode(decoded_output_data, earliest_shard_request_time,
                     manufacturer_id, software_id, version_id);

    // handle decode result
    is_success = handle_decode_result(
        status, decoded_output_data, earliest_shard_request_time,
        manufacturer_id, software_id, version_id, time_delivered, create_time);
  } else {
    // do not decode, this av is a non-client av, and it does not decode
    is_success = true;
  }

  return is_success;
}

// parse and handle the payload
const bool AvswuClientAV11p::handle_payload(const string payload) {

  // parse payload
  json j_payload;
  try {
    j_payload = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg = "message is not json. "
                           "unable to parse payload, exception e=" +
                           string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  payload_type_t type;
  try {
    int type_int = j_payload.at("header").at("payload_type");
    type = static_cast<payload_type_t>(type_int);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "message is not a "
        "packet.  unable to get payload type , exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // update packet received stats
  _statistics.packet_received++;

  // switch based on packet type received
  string log_msg;
  bool is_success = false;
  switch (type) {
  case payload_type_t::none:
    log_msg = "payload type not set";
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  case payload_type_t::support_list:
    log_msg = "payload type is support list";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_support_list(payload);
    break;
  case payload_type_t::inventory:
    log_msg = "payload type is inventory. ignoring.";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  case payload_type_t::notification_list:
    log_msg = "payload type is notification list";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_notification_list(payload);
    break;
  case payload_type_t::shard_request_list:
    log_msg = "payload type is shard request list. ignoring.";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  case payload_type_t::shard:
    log_msg = "payload type is shard";
    AVSWU_LOG_INFO(log_msg);
    log_msg = "received payload=shard, not printing json since it is a long "
              "data set";
    AVSWU_LOG_DEBUG(log_msg);
    is_success = handle_shard(payload);
    break;
  case payload_type_t::delivery_request:
    log_msg = "payload type is delivery_request. ignoring.";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  default:
    log_msg = "unknown payload type";
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
  }

  // update ui
  if (is_success) {
    ui_update(payload);
  }

  return is_success;
}

/*
The onWSM method of the application defines how it reacts to received data: by
calling changeRoute to avoid the road it has received information about -- and
(if sentMessage is still false) by scheduling a reminder to re-send this
message in a little bit over 2 seconds and setting the sentMessage flag.

from: https://veins.car2x.org/tutorial/

*/
void AvswuClientAV11p::onWSM(BaseFrame1609_4 *frame) {
  // Your application has received a data message from another car or RSU
  // code for handling the message goes here, see TraciDemo11p.cc for examples
  string log_msg = "onWSM() called";
  AVSWU_LOG_DEBUG(log_msg);

  AvswuAV11pMessage *wsm = check_and_cast<AvswuAV11pMessage *>(frame);

  // if null wsm, return
  if (!wsm) {
    AVSWU_LOG_ERROR(string("wsm=null"));
    return;
  }

  // if null node, return
  if (!_avP) {
    AVSWU_LOG_ERROR(string("_avP=null"));
    return;
  }

  // read data from wsm
  uint64_t senderId = wsm->getSenderId();
  string payload = wsm->getPayload();

  log_msg = "onWSM() receiving_av_id=" + to_string(myId) +
            +": senderId=" + to_string(senderId);
  AVSWU_LOG_DEBUG(log_msg);

  // parse and handle the payload
  const bool is_success = handle_payload(payload);
  if (!is_success) {
    log_msg = "unable to process the message/payload";
    AVSWU_LOG_ERROR(log_msg);
  }
}

void AvswuClientAV11p::handlePositionUpdate(cObject *obj) {
  // if null node, return
  if (!_avP) {
    AVSWU_LOG_ERROR(string("_avP=null"));
    return;
  }

  // remove old delivery rsu/av nodes from set
  remove_out_of_range_delivery_nodes();

  // if no delivery (av or rsu) nodes in range, clear cache, ui label update
  if (_delivery_range_map.size() == 0) {
    // clear notification cache
    _notification_listP_map.clear();

    // update ui
    if (AVSWU_NODE_UI_LABEL_OUT_OF_RANGE) {
      auto ui_msg = to_string(_avP->id()) + ":out of range";
      findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());
    }
  }
}

// check if this av, is the recipient (to in packet header)
const bool AvswuClientAV11p::is_recipient(AvswuPacket *packetP) const {
  // if not packet, return false
  if (!packetP) {
    return false;
  }
  // check if id's match
  const bool same = compare_id(_avP->id(), _avP->X(), packetP->header().to_id(),
                               packetP->header().to_X());
  return same;
}

// cache a notification list into cache
void AvswuClientAV11p::notification_list_cache_insert(
    notification_listP_t packetP) {
  if (!packetP) {
    return;
  }

  // create a new key
  auto nl_key = NotificationListMapKey(packetP->header().from_id(),
                                       packetP->header().from_X());
  // insert
  _notification_listP_map[nl_key] = packetP;
}

// delete notification list from cache
const bool AvswuClientAV11p::notification_list_cache_delete(const uint64_t id,
                                                            const mpz_t &X) {

  // create a new key
  mpz_t X_copy;
  mpz_init_set(X_copy, X);
  auto nl_key = NotificationListMapKey(id, X_copy);

  // delete
  auto removed = _notification_listP_map.erase(nl_key);

  // free temp element
  mpz_clear(X_copy);

  return (removed > 0);
}

// is the notification for this rsu/av already in cache?
const bool AvswuClientAV11p::notification_list_cache_contains(
    AvswuPacketSupportList &packet) {

  for (auto it = _notification_listP_map.begin();
       it != _notification_listP_map.end(); it++) {
    auto key = it->first;
    auto value = it->second;
    // is this notification list have the same from, as the packet received?
    if (compare_id(value->header().from_id(), value->header().from_X(),
                   packet.header().from_id(), packet.header().from_X())) {
      return true;
    }
  }

  return false;
}

// update delivery range set for this packet
void AvswuClientAV11p::update_delivery_range_set(
    AvswuPacketSupportList &packet) {

  // update last shard request list time

  // create a new key
  auto rs_key =
      RangeMapKey(packet.header().from_id(), packet.header().from_X());

  // update the element, or insert new element if it does not exist
  _delivery_range_map[rs_key] = simTime();

  // TEMP DEBUG START
  // for (auto it = _delivery_range_map.begin(); it !=
  // _delivery_range_map.end();
  //      it++) {
  //   auto key = it->first;
  //   auto value = it->second;
  //   auto id = key.id();
  //   const string log_msg = "in range of delivery av/rsu id=" +
  //   to_string(id); AVSWU_LOG_ERROR(log_msg);
  // }
  // TEMP DEBUG END
}

// remove old delivery rsu/av nodes from set
const size_t AvswuClientAV11p::remove_out_of_range_delivery_nodes() {

  // loop thru map, if we have not received a recent support list packet, we
  // remove any of it's notifications and delete it from the (in range) map
  size_t removed = 0;
  auto it = _delivery_range_map.begin();
  while (it != _delivery_range_map.end()) {
    auto key = it->first;
    auto value = it->second;
    if (simTime() >=
        value + max(RSU_SUPPORT_LIST_PERIOD, DELIVERY_SUPPORT_LIST_PERIOD) +
            1) {
      // point erase_it to element that needs to be deleted
      auto erase_it = it;
      // advance it to next element before deleting
      it++;
      // delete the element
      _delivery_range_map.erase(erase_it);
      // delete it's notification list from cache (if it exists)
      notification_list_cache_delete(key.id(), key.X());
      removed++;
    } else {
      it++;
    }
  }
  return removed;
}

// change ui for node based on packet received
/*
label for nodes are [to_id:summary:packet:from_id] with each can be turned
off/on using defines below

summary indicates, "" = no shards, "S" = shard received, "U" = software update
received
*/
void AvswuClientAV11p::ui_update(const string payload) {

  // parse payload into packet
  AvswuPacket packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet support list, exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return;
  }

  auto type = packet.header().payload_type();
  auto from_id = packet.header().from_id();
  string packet_type;

  bool update_image = false;

  switch (type) {
  case support_list:
    packet_type = "support_list";
    break;
  case inventory:
    packet_type = "inventory";
    break;
  case notification_list:
    packet_type = "notification_list";
    break;
  case shard_request_list:
    packet_type = "shard_request_list";
    break;
  case shard:
    packet_type = "shard";
    // if status was no_shards, advance status to has shards
    if (_ui_status == no_shards) {
      _ui_status = has_shards;
      update_image = true;
    }
    break;
  case delivery_request:
    packet_type = "delivery_request";
    break;
  default:
    // no color change
    packet_type = "UNKNOWN";
    AVSWU_LOG_ERROR("unknown packet type for ui message")
  }

  // figure out the summary
  string summary;
  if (AVSWU_NODE_UI_LABEL_SUMMARY) {
    switch (_ui_status) {
    case no_shards:
      // show nothing if no shards collected
      summary = "";
      break;
    case has_shards:
      summary = "S";
      break;
    case sw_update_success:
      summary = "U";
      break;
    default:
      summary = "";
      break;
    }

    // update ui label
    string ui_msg = "";

    if (AVSWU_NODE_UI_LABEL_TO_ID) {
      ui_msg += to_string(_avP->id());
    }

    if (AVSWU_NODE_UI_LABEL_SUMMARY) {
      ui_msg += ":" + summary;
    }

    if (AVSWU_NODE_UI_LABEL_PACKET) {
      ui_msg += ":" + packet_type;
    }

    if (AVSWU_NODE_UI_LABEL_FROM_ID) {
      ui_msg += ":" + to_string(from_id);
    }

    if (AVSWU_NODE_UI_LABEL_TO_ID || AVSWU_NODE_UI_LABEL_SUMMARY ||
        AVSWU_NODE_UI_LABEL_PACKET || AVSWU_NODE_UI_LABEL_FROM_ID) {
      findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());
    }

    /*
     * update image
     */
    if (AVSWU_NODE_UI_COLOR && update_image) {
      // update node color/image
      switch (_ui_status) {
      case no_shards:
        // do nothing, we set image color at initialization
        break;
      case has_shards:
        // if have shards, update image
        findHost()->getDisplayString().setTagArg("i", 0,
                                                 "veins/avswu_icons/av_yellow");
        break;
      case sw_update_success:
        // do nothing, it is already changed
        break;
      default:
        AVSWU_LOG_ERROR("unknown ui_status")
      }
    }
  }
}


#include "AvswuRSU11p.h"
#include "AvswuConfig.h"
#include "AvswuPacketDeliveryRequest.h"
#include "AvswuPacketInventory.h"
#include "AvswuPacketShardRequestList.h"
#include "AvswuPacketSupportList.h"

#include "AvswuVeinsUtils.h"
#include <spdlog/spdlog.h>

#include "AvswuAV11pMessage_m.h"

using namespace veins;
using namespace std;
using namespace avswu_veins_utils;
using namespace avswu_config;
using namespace avswu_packet_support_list;
using namespace avswu_packet_shard_request_list;
using namespace avswu_packet_delivery_request;

// schedule msg to be sent every 5 seconds
const uint32_t support_list_delay = 5;

Define_Module(veins::AvswuRSU11p);

// requires 2 stage initialization for applicaitons
int AvswuRSU11p::numInitStages() const { return 3; }

const bool AvswuRSU11p::initialize_rsu(const uint64_t rsu_id,
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

  _rsuP = new AvswuRSUNode(rsu_id, cache_max_size, get_grpc_clientP());
  if (!_rsuP) {
    const string log_msg = "unable to create rsu node";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  const string log_msg =
      "_rsuP->X()=" + string(mpz_get_str(nullptr, 10, _rsuP->X()));
  AVSWU_LOG_DEBUG(log_msg);

  return true;
}

void AvswuRSU11p::initialize_supported_manufacturer_list() {

  // read config
  auto configP = get_avswu_configP();
  auto man_list = configP->manufacturer_list();

  // init to 0 since broadcast to all
  uint64_t to_id = 0;
  mpz_t to_X;
  mpz_init_set_ui(to_X, 0);

  // initialize the supported manufactuers packet
  _supported_manufacturer.header().set(_rsuP->id(), _rsuP->X(), to_id, to_X,
                                       payload_type_t::support_list,
                                       node_type_t::rsu);

  // make the rsu support all manufacturers

  // TODO: make all rsu's support all manufacturers in config
  // multi-stage rsu stage1 => create rsu, and write id->X to a global hashmap,
  // stage 2 => read populate support list with X for mans, and start to send
  // out repeating support packages
  for (size_t i = 0; i < man_list.size(); i++) {
    auto man = man_list[i];
    manufacturer_id_t man_id;
    man_id.manufacturer_id = man.manufacturer_id;
    // TODO: TEMPORARY set X to id value, UNTIL WE GET OR GENERATE A THE REAL X
    mpz_init_set_ui(man_id.manufacturer_X, man_id.manufacturer_id);
    _supported_manufacturer.push_back(man_id);
  }
}

const bool AvswuRSU11p::initialize_stage1() {

  // set node type
  _node_type = node_type_t::rsu;

  // ui icon update
  findHost()->getDisplayString().setTagArg("i", 0, "veins/avswu_icons/rsu");
  // color icon
  findHost()->getDisplayString().setTagArg("i", 1, "white");
  // icon size
  // findHost()->getDisplayString().setTagArg("is", 0, "vl");
  findHost()->getDisplayString().setTagArg("is", 0, "s");

  // TODO: add support for multiple rsu
  const uint64_t rsu_id = 1000 + myId;

  // init rsu
  const bool is_init_success = initialize_rsu(rsu_id, AVSWU_RSU_MAX_CACHE_SIZE);
  if (!is_init_success) {
    const string log_msg = "initialize rsu failed";
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

  // populate the supported manufacturer list
  initialize_supported_manufacturer_list();

  // schedule first support list
  const uint64_t current_time = simTime().dbl();
  const bool is_success = schedule_packet(
      _supported_manufacturer, "av", current_time + RSU_SUPPORT_LIST_START);

  // get info from .ned and .ini file, and set variable
  auto value = par("softwareUpdateSizeMB").intValue();
  this->software_update_size_mb = value;
  const string log_msg =
      "software_update_size_MB=" + to_string(this->software_update_size_mb);
  AVSWU_LOG_INFO(log_msg);

  return is_success;
}

void AvswuRSU11p::initialize(int stage) {
  const string log_msg = "AvswuRSU11p:initialize, stage=" + to_string(stage);
  AVSWU_LOG_DEBUG(log_msg);

  DemoBaseApplLayer::initialize(stage);

  if (stage == 1) {
    const bool is_success_stage1 = initialize_stage1();
    if (is_success_stage1) {
      const string log_msg = "AvswuRSU11p:initialize_stage1() success";
      AVSWU_LOG_DEBUG(log_msg);
    } else {
      const string log_msg = "AvswuRSU11p:initialize_stage1() failed";
      AVSWU_LOG_ERROR(log_msg);
    }
  }

  if (stage == 2) {
    const string log_msg = "stage 2 called";
    AVSWU_LOG_DEBUG(log_msg);
  }
}

// destructor for memory cleanup
AvswuRSU11p::~AvswuRSU11p() { delete _rsuP; }

void AvswuRSU11p::onWSA(DemoServiceAdvertisment *wsa) {
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

  // if this RSU receives a WSA for service 42, it will tune to the chan
  if (wsa->getPsid() == 42) {
    mac->changeServiceChannel(static_cast<Channel>(wsa->getTargetChannel()));
  }
}

// parse and handle support list
const bool AvswuRSU11p::handle_inventory(const string payload) {

  // parse payload into inventory
  AvswuPacketInventory packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg = "unable to parse payload into a packet support list";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

  // point-to-point (node-to-node) communication: if packet not sent to this
  // rsu, ignore packet
  if (!is_recipient(&packet)) {
    const string log_msg = "wrong recipient, inventory packet was sent "
                           "to a node with id=" +
                           to_string(packet.header().to_id());
    AVSWU_LOG_WARN(log_msg);
    return true;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  // update ui (after point-to-point)
  auto packet_from_id = packet.header().from_id();
  auto ui_msg =
      to_string(_rsuP->id()) + ":inventory:" + to_string(packet_from_id);
  findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());

  // read notification_list from blockchain, based on s/w inventory from av
  AvswuPacketNotificationList notification_list =
      _rsuP->blockchain_read_notification_list(
          packet.header().from_id(), packet.header().from_X(), packet);

  // set from type
  notification_list.header().set_from_type(node_type());

  if (notification_list.notif_deque().size() == 0) {
    const string log_msg = "unable to find any new notifications";
    AVSWU_LOG_INFO(log_msg);
    return true;
  }

  // send notification list to av
  notification_list.statistics().set_send_time(simTime().dbl());
  const bool is_sent = send_packet(notification_list, "rsu");
  if (!is_sent) {
    const string log_msg = "unable to send notification list";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // rsu read packets of shards from ipfs based on those requested from
  // notification, and saves them into it's cache
  vector<AvswuPacketShard> shard_list;
  bool is_success_prime_cache =
      _rsuP->ipfs_read_shard_list(shard_list, packet.header().from_id(),
                                  packet.header().from_X(), notification_list);
  if (is_success_prime_cache) {
    const string log_msg = "successsfully primed the cache";
    AVSWU_LOG_INFO(log_msg);
  } else {
    const string log_msg = "priming the cache failed";
    AVSWU_LOG_ERROR(log_msg);
  }

  return is_success_prime_cache;
}

// parse and handle delivery request
const bool AvswuRSU11p::handle_delivery_request(const string payload) {

  // parse payload into delivery request
  AvswuPacketDeliveryRequest packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet delivery request";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

  // point-to-point (node-to-node) communication: if packet not sent to this
  // rsu, ignore packet
  if (!is_recipient(&packet)) {
    const string log_msg = "wrong recipient, inventory packet was sent "
                           "to a node with id=" +
                           to_string(packet.header().to_id());
    AVSWU_LOG_WARN(log_msg);
    return true;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  // update ui (after point-to-point)
  auto packet_from_id = packet.header().from_id();
  auto ui_msg =
      to_string(_rsuP->id()) + ":delivery_request:" + to_string(packet_from_id);
  findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());

  // create vector of manufacturer ids
  vector<uint64_t> manufacturer_list;
  auto supported_list = _supported_manufacturer.manufacturer_list();
  for (auto it = supported_list.begin(); it != supported_list.end(); it++) {
    manufacturer_list.push_back(it->manufacturer_id);
  }

  // read notification_list from blockchain, based on delivery_request from av
  AvswuPacketNotificationList notification_list =
      _rsuP->blockchain_read_last_notification_list(packet.header().from_id(),
                                                    packet.header().from_X(),
                                                    manufacturer_list, packet);

  // set from type
  notification_list.header().set_from_type(_node_type);

  if (notification_list.notif_deque().size() == 0) {
    const string log_msg = "unable to find any new notifications";
    AVSWU_LOG_INFO(log_msg);
    return true;
  }

  // send notification list to av
  notification_list.statistics().set_send_time(simTime().dbl());
  const bool is_sent = send_packet(notification_list, "rsu");
  if (!is_sent) {
    const string log_msg = "unable to send notification list";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // rsu read packets of shards from ipfs based on those requested from
  // notification, and saves them into it's cache
  vector<AvswuPacketShard> shard_list;
  bool is_success_prime_cache =
      _rsuP->ipfs_read_shard_list(shard_list, packet.header().from_id(),
                                  packet.header().from_X(), notification_list);
  if (is_success_prime_cache) {
    const string log_msg = "successsfully primed the cache";
    AVSWU_LOG_INFO(log_msg);
  } else {
    const string log_msg = "priming the cache failed";
    AVSWU_LOG_ERROR(log_msg);
  }

  return is_success_prime_cache;
}

// handle shard request list
const bool AvswuRSU11p::handle_shard_request_list(const string payload) {

  // parse payload into shard request list
  AvswuPacketShardRequestList packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to parse payload into a packet shard request list";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

  // point-to-point (node-to-node): if packet not sent to this rsu, ignore
  // packet
  if (!is_recipient(&packet)) {
    const string log_msg =
        "wrong recipient, shard_request_list packet was sent "
        "to a node with id=" +
        to_string(packet.header().to_id());
    AVSWU_LOG_WARN(log_msg);
    return true;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  // update ui (after point-to-point)
  auto packet_from_id = packet.header().from_id();
  auto ui_msg = to_string(_rsuP->id()) +
                ":shard request list:" + to_string(packet_from_id);
  findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());

  // rsu fetches shard packets from it's cache based on shard_request_list
  size_t sent_count = 0;
  vector<AvswuPacketShard> request_packet_list = _rsuP->get_request_packet_list(
      packet.header().from_id(), packet.header().from_X(), packet);

  // randomly pick the packet to send to increase distribution of packets so
  // same packets are not sent repeatedly. create vector of indices 0 to n-1,
  // then shuffle the indices
  vector<size_t> shuffle_indices(request_packet_list.size());
  std::iota(std::begin(shuffle_indices), std::end(shuffle_indices), 0);
  std::random_shuffle(shuffle_indices.begin(), shuffle_indices.end());

  double delay = 0.0f;
  for (size_t shuffle_i = 0; shuffle_i < shuffle_indices.size(); shuffle_i++) {
    size_t i = shuffle_indices[shuffle_i];
    auto packet = request_packet_list[i];
    // increment sent_count in header of packet
    packet.statistics().set_hop_count(packet.statistics().hop_count() + 1);
    // set from type
    packet.header().set_from_type(_node_type);

    // send the shard w/ additional delay for each shard sent
    // 802.11p sends at 27Mbps, so we delay by packet size / 27Mbps
    const size_t bytes = packet.size() * software_update_size_mb;
    double seconds =
        static_cast<double>(bytes) / AVSWU_802_11P_BYTES_PER_SECOND;
    delay += seconds;
    const string msg = fmt::format(
        "sending a shard packet of size bytes={} with delay of={:.8f}", bytes,
        delay);
    AVSWU_LOG_INFO(msg);

    packet.statistics().set_send_time(simTime().dbl());
    const bool is_sent = send_packet_delayed(packet, "rsu", delay);

    if (is_sent) {
      sent_count++;
    }
    // spdlog::debug("request_packet_list[{}]={}", i, json(packet).dump());
  }

  const string log_msg = "shard sent_count=" + to_string(sent_count);
  AVSWU_LOG_INFO(log_msg);

  const bool is_success = sent_count > 0;
  return is_success;
}

// parse and handle the payload
const bool AvswuRSU11p::handle_payload(const string payload) {

  // parse payload
  json j_payload;
  try {
    j_payload = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg = "message is not json. unable to parse payload";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

  payload_type_t type;
  try {
    int type_int = j_payload.at("header").at("payload_type");
    type = static_cast<payload_type_t>(type_int);
  } catch (const std::runtime_error &e) {
    const string log_msg = "message is not a "
                           "packet.  unable to get payload type";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

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
    log_msg = "payload type is support list. ignoring";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  case payload_type_t::inventory:
    log_msg = "payload type is inventory";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_inventory(payload);
    break;
  case payload_type_t::notification_list:
    log_msg = "payload type is notification list. ignoring";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  case payload_type_t::shard_request_list:
    log_msg = "payload type is shard request list";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_shard_request_list(payload);
    break;
  case payload_type_t::shard:
    log_msg = "payload type is shard. ignoring";
    AVSWU_LOG_INFO(log_msg);
    // ignore, do nothing
    is_success = true;
    break;
  case payload_type_t::delivery_request:
    log_msg = "payload type is delivery_request";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_delivery_request(payload);
    break;
  default:
    log_msg = "unknown payload type";
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
  }

  log_msg = "received payload=" + json::parse(payload).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  return is_success;
}

void AvswuRSU11p::onWSM(BaseFrame1609_4 *frame) {
  string log_msg = "onWSM() called";
  AVSWU_LOG_DEBUG(log_msg);

  AvswuAV11pMessage *wsm = check_and_cast<AvswuAV11pMessage *>(frame);

  // if null wsm, return
  if (!wsm) {
    AVSWU_LOG_ERROR(string("wsm=null"));
    return;
  }

  // if null node, return
  if (!_rsuP) {
    AVSWU_LOG_ERROR(string("_rsuP=null"));
    return;
  }

  // read data from wsm
  uint64_t senderId = wsm->getSenderId();
  string payload = wsm->getPayload();
  string senderType = wsm->getSenderType();

  log_msg = "onWSM() receiving_rsu_id=" + to_string(myId) +
            +": senderId=" + to_string(senderId) + ", senderType=" + senderType;
  AVSWU_LOG_INFO(log_msg);

  // parse and handle the payload
  const bool is_success = handle_payload(payload);
  if (!is_success) {
    log_msg = "unable to process the message/payload";
    AVSWU_LOG_ERROR(log_msg);
  }
}

void AvswuRSU11p::handleSelfMsg(cMessage *msg) {
  // string log_msg = "handleSelfMsg()";
  // AVSWU_LOG_INFO(log_msg);

  // create duplicate and schedule the message to repeat it
  if (AvswuAV11pMessage *wsm = dynamic_cast<AvswuAV11pMessage *>(msg)) {
    // schedule duplicate message
    auto wsm_dup = wsm->dup();
    scheduleAt(simTime() + RSU_SUPPORT_LIST_PERIOD, wsm_dup);
    // send current messge immediately
    sendDown(wsm);
  } else {
    DemoBaseApplLayer::handleSelfMsg(msg);
  }
}

// check if this rsu, is the recipient (to in packet header)
const bool AvswuRSU11p::is_recipient(AvswuPacket *packetP) const {

  // if not packet, return false
  if (!packetP) {
    return false;
  }

  // check if id's match
  const bool same = avswu_veins_utils::compare_id(_rsuP->id(), _rsuP->X(),
                                                  packetP->header().to_id(),
                                                  packetP->header().to_X());
  return same;
}

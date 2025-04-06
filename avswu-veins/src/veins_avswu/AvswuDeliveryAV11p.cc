#include "AvswuDeliveryAV11p.h"
#include "AvswuConfig.h"

#include "AvswuPacketDeliveryRequest.h"
#include "AvswuVeinsUtils.h"
#include <spdlog/spdlog.h>

#include "AvswuPacketInventory.h"

using namespace veins;
using namespace std;
using namespace avswu_veins_utils;
using namespace avswu_config;
using namespace avswu_packet_support_list;
using namespace avswu_notification_list_map;
using namespace avswu_packet_delivery_request;
using namespace avswu_packet_inventory;

Define_Module(veins::AvswuDeliveryAV11p);

// initialize an empty support list
// devivery av starts by supporting no manufacturers
// support list is added by contact with RSU and received
// a notification list
void AvswuDeliveryAV11p::initialize_supported_manufacturer_list() {

  // read config
  auto configP = get_avswu_configP();
  auto man_list = configP->manufacturer_list();

  // init to 0 since broadcast to all
  uint64_t to_id = 0;
  mpz_t to_X;
  mpz_init_set_ui(to_X, 0);

  // initialize the supported manufactuers packet
  _supported_manufacturer.header().set(_avP->id(), _avP->X(), to_id, to_X,
                                       payload_type_t::support_list,
                                       node_type_t::delivery_av);
}

const bool AvswuDeliveryAV11p::initialize_stage1() {

  // set node type
  _node_type = node_type_t::delivery_av;

  // set node ui status, and update ui color/image
  _ui_status = no_shards;
  // set icon image
  findHost()->getDisplayString().setTagArg("i", 0,
                                           "veins/avswu_icons/av_black");

  // TODO: add support for multiple manufacturers
  // manufacturer for this av
  _manufacturer_id = 100;

  /*
{
"index": 8,
"software_index_list": [
    8
]
},
*/
  // software configuration for this av
  _inventory_index = 8;

  // set inventory pointer
  _inventoryP =
      get_avswu_configP()->inventoryP(_manufacturer_id, _inventory_index);

  const uint64_t av_id = 3000 + myId;

  // init rsu
  const bool is_init_success =
      initialize_av(av_id, AVSWU_DELIVERY_AV_MAX_CACHE_SIZE);
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

  // initiazlie an empty supported manufacturer list
  initialize_supported_manufacturer_list();

  // schedule first support list
  const double current_time = simTime().dbl();
  const bool is_success =
      schedule_packet(_supported_manufacturer, "av",
                      current_time + DELIVERY_SUPPORT_LIST_START);

  // set uid label, if needed
  if (AVSWU_NODE_UI_LABEL_TO_ID) {
    const string ui_msg = to_string(_avP->id());
    findHost()->getDisplayString().setTagArg("t", 0, ui_msg.c_str());
  }

  // set depart time, so we can subtract it from sw delivery time
  _statistics.depart_time = simTime().dbl();

  // increase av count
  _delivery_av_count++;
  // record count
  recordScalar("delivery_av_count", _delivery_av_count);

  // get info from .ned and .ini file, and set variable
  auto value = par("softwareUpdateSizeMB").intValue();
  this->software_update_size_mb = value;
  const string log_msg =
      "software_update_size_MB=" + to_string(this->software_update_size_mb);
  AVSWU_LOG_INFO(log_msg);

  return is_success;
}

void AvswuDeliveryAV11p::initialize(int stage) {
  const string log_msg = "initialize, stage=" + to_string(stage);
  AVSWU_LOG_INFO(log_msg);

  DemoBaseApplLayer::initialize(stage);

  if (stage == 0) {
    _current_subscribed_service_id = -1;
  }

  // by stage 1, the veins structs are setup, and myId is set
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

// destructor
AvswuDeliveryAV11p::~AvswuDeliveryAV11p() {

  // decrease av count
  _delivery_av_count--;

  // clear notification list
  _last_notification_list.clear();
}

// send delivery request
// respond to support list by sending a delivery request to rsu or another
// delivery av
const bool AvswuDeliveryAV11p::send_delivery_request(const uint64_t to_id,
                                                     mpz_t &to_X) {

  // if null node, return
  if (!_avP) {
    AVSWU_LOG_ERROR(string("_avP=null"));
    return false;
  }

  // create a delivery request to get some notifications
  const size_t max_notification = DELIVERY_MAX_NOTIFCATION;
  // any update created within the last minute is considred to be new
  const double current = simTime().dbl();
  const double start_time = std::max((double)0, (double)(current - 60));
  AvswuPacketDeliveryRequest packet_delivery_request(
      _avP->id(), _avP->X(), to_id, to_X, start_time, max_notification,
      _node_type);

  const string log_msg =
      "packet_delivery_request=" + json(packet_delivery_request).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // send inventory
  packet_delivery_request.statistics().set_send_time(simTime().dbl());
  const bool is_success = send_packet(packet_delivery_request, "av");
  // update packet sent stats
  _statistics.packet_sent++;

  return is_success;
}

// parse and handle support list
const bool AvswuDeliveryAV11p::handle_support_list(const string payload) {

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

  const string log_msg = "received payload=" + json(packet).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // update delivery range set for this packet
  update_delivery_range_set(packet);

  /*
    1a. if notification list in cache, do nothing => we have already updated
    or we/are in the process of updating
    1b. if notification list is not in
    cache, process support list
    */
  const bool has_notif_list = notification_list_cache_contains(packet);
  if (has_notif_list) {
    // do nothing
    const string log_msg =
        "ignoring support list packet, since notification list is cached";
    AVSWU_LOG_DEBUG(log_msg);
    return true;
  }

  // send delivery request list to rsu or another delivery-av
  const bool is_success = send_delivery_request(packet.header().from_id(),
                                                packet.header().from_X());

  return is_success;
}

// updates support list based on notification payload
const bool AvswuDeliveryAV11p::update_support_list() {

  // 1. traverse notification list, add manufactuer into a set to get unique ids
  // and remove any dups
  set<uint64_t> id_set;
  for (auto it = _last_notification_list.begin();
       it != _last_notification_list.end(); it++) {
    auto notif = *it;
    id_set.insert(notif.manufacturer_id());
  }

  // 2. then, put all the unique ids into to support list
  for (auto it = id_set.begin(); it != id_set.end(); it++) {
    manufacturer_id_t man_id;
    man_id.manufacturer_id = *it;
    // TODO: TEMPORARY set X to id value, UNTIL WE GET OR GENERATE A THE REAL X
    mpz_init_set_ui(man_id.manufacturer_X, man_id.manufacturer_id);

    // add manufacturer ids to currently supported manufacturrers
    _supported_manufacturer.push_back(man_id);
  }

  return true;
}

// update last notification list based on the notification list received
const bool AvswuDeliveryAV11p::update_last_notification_list(
    notification_listP_t packetP) {

  if (packetP == nullptr) {
    return false;
  }

  auto notif_deque = packetP->notif_deque();
  for (auto it = notif_deque.begin(); it != notif_deque.end(); it++) {
    auto notif = *it;
    // if notif is in list, check if this notif is more recent
    bool found = false;
    size_t i = 0;
    while (i < _last_notification_list.size() && !found) {
      auto last_notif = _last_notification_list[i];
      // if notif is in the last (for any version)
      if ((notif.manufacturer_id() == last_notif.manufacturer_id()) &&
          (notif.software_id() == last_notif.software_id())) {
        // found the software
        found = true;
        // if the new notif is more recent, update it and make this notif the
        // last notification
        if ((notif.version_id() >= last_notif.version_id()) &&
            (notif.create_time() > last_notif.create_time())) {
          _last_notification_list[i] = notif;
        }
      }
      i++;
    }

    // if this notif is not in the list, add it
    if (!found) {
      _last_notification_list.push_back(notif);
    }
  }

  return true;
}

// parse and handle notification list
const bool AvswuDeliveryAV11p::handle_notification_list(const string payload) {

  // call the base class handle_notification_list function, to send shard
  // request back to RSU or delivery-AV to cache the shards for delivery
  // to other avs later
  // note: the notification cache is also populated in this base method
  bool is_success = AvswuClientAV11p::handle_notification_list(payload);
  if (!is_success) {
    return false;
  }

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

  const string log_msg = "received payload=" + json::parse(payload).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // update notification set using new any notification in packet
  is_success = update_last_notification_list(packetP);
  if (!is_success) {
    const string log_msg = "unable to update notification set";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // update the support list based on notification dav has in its notification
  // set
  is_success = update_support_list();
  if (!is_success) {
    const string log_msg = "unable to update support list";
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  return true;
}

const bool AvswuDeliveryAV11p::add_notification(
    AvswuPacketNotificationList &notification_list,
    const software_t &software) {

  // search the last_notification_list to find out if a notification for the
  // software exists
  bool found = false;
  size_t i = 0;
  while (i < _last_notification_list.size() && !found) {
    auto last_notif = _last_notification_list[i];
    // if a notification exist, add it to notif list
    if ((last_notif.manufacturer_id() == software.manufacturer_id) &&
        (last_notif.software_id() == software.software_id)) {
      notification_list.push_back(last_notif);
      found = true;
    }
    i++;
  }
  return found;
}

// update notification list based on delivery request packet
const size_t AvswuDeliveryAV11p::update_notification_list(
    AvswuPacketNotificationList &notification_list,
    const AvswuPacketDeliveryRequest &packet) {

  // send the other delivery-av up to max_notifications starting from start_time
  // from the cached notifications

  // 1. put all notifications >= start_time from _last_notification_list into
  // vector
  vector<AvswuNotification *> notifP_list;
  auto start_time = packet.start_time();
  for (size_t i = 0; i < _last_notification_list.size(); i++) {
    AvswuNotification *last_notifP = &_last_notification_list[i];

    if (last_notifP->create_time() >= start_time) {
      notifP_list.push_back(last_notifP);
    }
  }

  // 2. randomly pick max_notifications from vector
  auto max_notification = packet.max_notification();
  auto orig_size = notifP_list.size();
  auto n = min(max_notification, orig_size);
  size_t count = 0;
  for (size_t i = 0; i < n; i++) {
    auto choice_i = rand() % notifP_list.size();
    auto notif_choice = notifP_list[choice_i];
    // add the notification to be sent back to av
    notification_list.push_back(*notif_choice);
    count++;
    // remove the element from list
    notifP_list.erase(notifP_list.begin() + choice_i);
  }

  return n;
}

// update notification list based on inventory packet
const size_t AvswuDeliveryAV11p::update_notification_list(
    AvswuPacketNotificationList &notification_list,
    const AvswuPacketInventory &packet) {

  // check if any of the cachced notifications (in notification cache)
  // are new updates for the avs inventory if so, send the notification list
  // back to av.
  // for each software in inventory, add latest notification
  size_t count = 0;
  for (size_t i = 0; i < packet.software_list().size(); i++) {
    auto software_item = packet.software_list()[i];
    // if software has a corresponding notification in the
    // last_notification_list, add the notification to be sent back to av
    const bool found = add_notification(notification_list, software_item);
    if (found) {
      count++;
    }
  }

  return count;
}

// parse and handle support list
const bool AvswuDeliveryAV11p::handle_inventory(const string payload) {

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

  // update a new notification list based on inventory packet
  AvswuPacketNotificationList notification_list(
      _avP->id(), _avP->X(), packet.header().from_id(),
      packet.header().from_X(), node_type());
  const size_t num_new_notifications =
      update_notification_list(notification_list, packet);

  // if there are new notifications, send back to av
  if (num_new_notifications > 0) {

    // send notification list to av
    notification_list.statistics().set_send_time(simTime().dbl());
    const bool is_sent = send_packet(notification_list, "av");
    if (!is_sent) {
      const string log_msg = "unable to send notification list";
      AVSWU_LOG_ERROR(log_msg);
      return false;
    }
  }

  return true;
}

// parse and handle delivery request
const bool AvswuDeliveryAV11p::handle_delivery_request(const string payload) {

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

  // create vector of manufacturer ids
  vector<uint64_t> manufacturer_list;
  auto supported_list = _supported_manufacturer.manufacturer_list();
  for (auto it = supported_list.begin(); it != supported_list.end(); it++) {
    manufacturer_list.push_back(it->manufacturer_id);
  }

  // update a new notification list based on inventory packet
  AvswuPacketNotificationList notification_list(
      _avP->id(), _avP->X(), packet.header().from_id(),
      packet.header().from_X(), node_type());
  const size_t num_new_notifications =
      update_notification_list(notification_list, packet);

  // if there are new notifications, send back to av
  if (num_new_notifications > 0) {

    // send notification list to av
    notification_list.statistics().set_send_time(simTime().dbl());
    const bool is_sent = send_packet(notification_list, "av");
    if (!is_sent) {
      const string log_msg = "unable to send notification list";
      AVSWU_LOG_ERROR(log_msg);
      return false;
    }
  }

  return true;
}

// handle shard request list
const bool AvswuDeliveryAV11p::handle_shard_request_list(const string payload) {

  // parse payload into shard request list
  AvswuPacketShardRequestList packet;
  try {
    packet = json::parse(payload);
  } catch (const std::runtime_error &e) {
    const string log_msg = "unable to parse payload into a packet support list";
    AVSWU_LOG_ERROR(log_msg);
    spdlog::error("exception e={}", e.what());
    return false;
  }

  // point-to-point (node-to-node): if packet not sent to this av, ignore
  // packet
  if (!is_recipient(&packet)) {
    const string log_msg =
        "wrong recipient, shard_request_list packet was sent "
        "to a node with id=" +
        to_string(packet.header().to_id());
    AVSWU_LOG_DEBUG(log_msg);
    return true;
  }

  // set the receive time for the packet
  packet.statistics().set_receive_time(simTime().dbl());

  // extra debug message
  auto to_id = packet.header().to_id();
  auto from_id = packet.header().from_id();
  string tmp = fmt::format("shard request list received from_id={}, to_id={}",
                           from_id, to_id);
  AVSWU_LOG_INFO(tmp);

  string log_msg = "received payload=" + json::parse(payload).dump(4);
  AVSWU_LOG_DEBUG(log_msg);

  // av fetches shard packets from its cache based on shard_request_list
  size_t sent_count = 0;
  vector<AvswuPacketShard> request_packet_list = _avP->get_request_packet_list(
      packet.header().from_id(), packet.header().from_X(), _node_type, packet);

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

    // increment sent_count in statistics of packet
    packet.statistics().set_hop_count(packet.statistics().hop_count() + 1);
    // set from type
    packet.header().set_from_type(_node_type);

    // send the shard w/ additional delay for each shard sent
    // 802.11p sends at 27Mbps, so we delay by packet size / 27Mbps
    // increase delay by software_size to mimic time for sending a larger s/w
    // update (software size is 1MiB so we multiply by software_update_size_mb)
    const size_t bytes = packet.size() * software_update_size_mb;
    double seconds =
        static_cast<double>(bytes) / AVSWU_802_11P_BYTES_PER_SECOND;
    delay += seconds;
    const string msg = fmt::format(
        "sending a shard packet of size bytes={} with delay of={:.8f}", bytes,
        delay);
    AVSWU_LOG_INFO(msg);

    packet.statistics().set_send_time(simTime().dbl());
    const bool is_sent = send_packet_delayed(packet, "av", delay);
    if (is_sent) {
      sent_count++;
    }
    // spdlog::debug("request_packet_list[{}]={}", i, json(packet).dump());
  }

  log_msg = "shard sent_count=" + to_string(sent_count);
  AVSWU_LOG_DEBUG(log_msg);

  // update shard sent stats
  _statistics.shard_sent += sent_count;

  // update packet sent stats
  _statistics.packet_sent += sent_count;

  bool is_success = false;
  // if we had packets in our cache to send
  if (request_packet_list.size() > 0) {
    if (sent_count > 0) {
      // had shard packets in cache, and sent some
      is_success = true;
    } else {
      // had shard packets in cache, but sent none
      is_success = false;
    }
  } else {
    // if we did not have any shards that we could send, success is true
    is_success = true;
  }

  return is_success;
}

// parse and handle the payload
const bool AvswuDeliveryAV11p::handle_payload(const string payload) {

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
    log_msg = "payload type is inventory";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_inventory(payload);
    break;
  case payload_type_t::notification_list:
    log_msg = "payload type is notification list";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_notification_list(payload);
    break;
  case payload_type_t::shard_request_list:
    log_msg = "payload type is shard request list";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_shard_request_list(payload);
    break;
  case payload_type_t::shard:
    log_msg = "payload type is shard";
    AVSWU_LOG_INFO(log_msg);
    is_success = handle_shard(payload);
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
void AvswuDeliveryAV11p::onWSM(BaseFrame1609_4 *frame) {
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

// handle a self-payload packet, and re-broadcast if needed
const bool
AvswuDeliveryAV11p::handle_self_support_list(const string payload,
                                             AvswuAV11pMessage *wsm) {

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

  // create a new wsm to be scheduled
  auto wsm_dup = wsm->dup();

  // if support lists are the same (no new updated support list), we re-send
  // the support list we simply compare json strings to compare packets
  if (json(packet).dump() == json(_supported_manufacturer).dump()) {
    // schedule the next support list to be sent, which is a duplicate message
    scheduleAt(simTime() + DELIVERY_SUPPORT_LIST_PERIOD, wsm_dup);

    // if manufactuer list > 0, send support list immediately
    // we do not send empty support lists
    if (packet.manufacturer_list().size() > 0) {
      sendDown(wsm);
    } else {
      // dispose of the wsm
      delete wsm;
    }
  } else {

    // update payloads w/ the new support_list
    const string new_support_list = json(_supported_manufacturer).dump();
    wsm->setPayload(new_support_list.c_str());
    wsm_dup->setPayload(new_support_list.c_str());

    // schedule the next support list to be sent, which is a duplicate message
    scheduleAt(simTime() + DELIVERY_SUPPORT_LIST_PERIOD, wsm_dup);

    // if manufactuer list > 0, send support list immediately
    // we do not send empty support lists
    if (packet.manufacturer_list().size() > 0) {
      sendDown(wsm);
    } else {
      // dispose of the wsm
      delete wsm;
    }
  }

  return true;
}

// handle self-payload wsm message, determine if wsm is a packet
const bool AvswuDeliveryAV11p::handle_self_payload(AvswuAV11pMessage *wsm) {

  // read payload from wsm
  const string self_payload = wsm->getPayload();

  // parse payload
  json j_payload;
  try {
    j_payload = json::parse(self_payload);
  } catch (const std::runtime_error &e) {
    const string log_msg = "self-message is not json. "
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
        "self-message is not a "
        "packet.  unable to get payload type , exception e=" +
        string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }

  // switch based on packet type received
  string log_msg;
  bool is_success = false;
  switch (type) {
  case payload_type_t::none:
    log_msg = "self-payload type not set";
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
    break;
  case payload_type_t::support_list:
    log_msg = "self-payload type is support list";
    AVSWU_LOG_DEBUG(log_msg);
    is_success = handle_self_support_list(self_payload, wsm);
    log_msg = "received payload=support list, not printing json since it is a "
              "long data set";
    AVSWU_LOG_DEBUG(log_msg);
    break;
  default:
    log_msg = "self-payload currently not supported, and should not have been "
              "received";
    AVSWU_LOG_ERROR(log_msg);
    is_success = false;
  }

  return is_success;
}

// schedule repetitive broadcast of support-list via self-message
void AvswuDeliveryAV11p::handleSelfMsg(cMessage *msg) {
  // string log_msg = "handleSelfMsg()";
  // AVSWU_LOG_INFO(log_msg);

  // if null node, return
  if (!_avP) {
    AVSWU_LOG_ERROR(string("_avP=null"));
    return;
  }

  // create duplicate and schedule the message to repeat it
  if (AvswuAV11pMessage *wsm = dynamic_cast<AvswuAV11pMessage *>(msg)) {

    // parse and handle the payload
    const bool is_success = handle_self_payload(wsm);
    if (!is_success) {
      const string log_msg = "unable to process the self-message/self-payload";
      AVSWU_LOG_ERROR(log_msg);
    }

  } else {
    AVSWU_LOG_WARN(string("self-message wsm=null"));
    DemoBaseApplLayer::handleSelfMsg(msg);
  }
}

// change ui for node based on packet received
/*
label for nodes are [to_id:summary:packet:from_id] with each can be turned
off/on using defines below

summary indicates, "" = no shards, "S" = shard received, "U" = software update
received
*/
void AvswuDeliveryAV11p::ui_update(const string payload) {

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
                                                 "veins/avswu_icons/av_white");
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
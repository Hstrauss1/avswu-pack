#include "AvswuPacketNotificationList.h"

namespace avswu_packet_notification_list {

AvswuPacketNotificationList::AvswuPacketNotificationList(
    const uint64_t from_id, mpz_t &from_X, const uint64_t to_id, mpz_t &to_X,
    node_type_t from_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::notification_list,
              from_type);

  // 1. build payload, later thru push_back
  // 2. create hash later after payload is built
  // 3. later, sign the pdu
}

AvswuPacketNotificationList::AvswuPacketNotificationList() {}

AvswuPacketNotificationList::~AvswuPacketNotificationList() {}

// create json for the payload
const json AvswuPacketNotificationList::payload_to_json() const {
  // create payload json
  json j_notif_list = json::array();
  for (size_t i = 0; i < _payload.notif_deque.size(); i++) {
    j_notif_list.push_back(_payload.notif_deque[i]);
  }
  json j_payload = {{"notif_list", j_notif_list}};

  return j_payload;
}

// create hash from payload
bool AvswuPacketNotificationList::create_hash() {

  // create json for the payload
  const json j_payload = payload_to_json();

  // create hash
  const bool is_success = AvswuPacket::create_hash(j_payload.dump());
  return is_success;
}

// to/from json
void to_json(json &j, const AvswuPacketNotificationList &p) {

  // create json for the payload
  const json j_payload = p.payload_to_json();

  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketNotificationList &p) {

  j.at("header").get_to(p._header);

  // handle payload
  json j_payload = j.at("payload");
  json j_notif_list = j_payload.at("notif_list");
  // clear item list, set, then push new items
  p._payload.notif_deque.clear();
  for (size_t i = 0; i < j_notif_list.size(); i++) {
    auto j_item = j_notif_list[i];
    AvswuNotification notif;
    j_item.get_to(notif);
    p._payload.notif_deque.push_back(notif);
  }

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketNotificationList &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_notification_list

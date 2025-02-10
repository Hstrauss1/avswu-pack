#include "AvswuPacketDeliveryRequest.h"

namespace avswu_packet_delivery_request {

AvswuPacketDeliveryRequest::AvswuPacketDeliveryRequest(
    const uint64_t from_id, mpz_t &from_X, const uint64_t to_id, mpz_t &to_X,
    const double start_time, const uint64_t max_notification,
    node_type_t from_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::delivery_request,
              from_type);

  // 1. build payload
  _payload.start_time = start_time;
  _payload.max_notification = max_notification;

  // 2. create hash later after payload is built
  // 3. later, sign the pdu
}

AvswuPacketDeliveryRequest::AvswuPacketDeliveryRequest() {}

AvswuPacketDeliveryRequest::~AvswuPacketDeliveryRequest() {}

// create json for the payload
const json AvswuPacketDeliveryRequest::payload_to_json() const {
  // create payload json
  json j_payload = {
      {"start_time", start_time()},
      {"max_notification", max_notification()},
  };

  return j_payload;
}

// to/from json
void to_json(json &j, const AvswuPacketDeliveryRequest &p) {

  // create json for the payload
  const json j_payload = p.payload_to_json();

  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketDeliveryRequest &p) {

  j.at("header").get_to(p._header);

  // handle payload
  json j_payload = j.at("payload");
  j_payload.at("start_time").get_to(p._payload.start_time);
  j_payload.at("max_notification").get_to(p._payload.max_notification);

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketDeliveryRequest &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_delivery_request

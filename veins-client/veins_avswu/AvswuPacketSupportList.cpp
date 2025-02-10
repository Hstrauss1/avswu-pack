#include "AvswuPacketSupportList.h"
#include "veins_avswu/AvswuConfig.h"

#include <iostream>

namespace avswu_packet_support_list {

AvswuPacketSupportList::AvswuPacketSupportList(const uint64_t from_id,
                                               mpz_t &from_X,
                                               const uint64_t to_id,
                                               mpz_t &to_X,
                                               node_type_t from_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::support_list,
              from_type);

  // 1. build payload, later thru push_back
  // 2. create hash later after payload is built
  // 3. later, sign the pdu
}

AvswuPacketSupportList::AvswuPacketSupportList() {}

AvswuPacketSupportList::~AvswuPacketSupportList() {}

// create json for the payload
const json AvswuPacketSupportList::payload_to_json() const {
  // create payload json
  json j_manufacturer_list = json::array();
  for (size_t i = 0; i < _payload._manufacturer_list.size(); i++) {
    auto man_item = _payload._manufacturer_list[i];

    // convert gmp elem to string
    string manufacturer_X(mpz_to_string(man_item.manufacturer_X));

    json j_man_item = {{"manufacturer_id", man_item.manufacturer_id},
                       {"manufacturer_X", manufacturer_X}};
    j_manufacturer_list.push_back(j_man_item);
  }
  json j_payload = {{"manufacturer_list", j_manufacturer_list}};

  return j_payload;
}

// set payload from json
void AvswuPacketSupportList::payload_from_json(const json &j) {
  json j_payload = j.at("payload");
  json j_manufacturer_list = j_payload.at("manufacturer_list");
  // clear item list, set, then push new items
  _payload._manufacturer_list.clear();
  for (size_t i = 0; i < j_manufacturer_list.size(); i++) {
    auto j_man_item = j_manufacturer_list[i];
    manufacturer_id_t item;
    j_man_item.at("manufacturer_id").get_to(item.manufacturer_id);

    mpz_init(item.manufacturer_X);
    set_mpz_str_from_json(j_man_item, "manufacturer_X", &item.manufacturer_X);

    _payload._manufacturer_list.push_back(item);
  }
}

// to/from json
void to_json(json &j, const AvswuPacketSupportList &p) {

  // create json for the payload
  const json j_payload = p.payload_to_json();

  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketSupportList &p) {

  j.at("header").get_to(p._header);

  // handle payload
  p.payload_from_json(j);

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketSupportList &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_support_list

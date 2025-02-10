#include "AvswuPacketInventory.h"
#include "AvswuConfig.h"

#include <iostream>

namespace avswu_packet_inventory {

AvswuPacketInventory::AvswuPacketInventory(const uint64_t from_id,
                                           mpz_t &from_X, const uint64_t to_id,
                                           mpz_t &to_X, node_type_t from_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::inventory,
              from_type);

  // set other_av to false
  _payload.other_av = false;

  // 1. build payload, later thru push_back
  // 2. create hash later after payload is built
  // 3. later, sign the pdu
}

AvswuPacketInventory::AvswuPacketInventory() { _payload.other_av = false; }

AvswuPacketInventory::~AvswuPacketInventory() {}

// create json for the payload
const json AvswuPacketInventory::payload_to_json() const {
  // create payload json

  json j_software_list = json::array();
  for (size_t i = 0; i < _payload.software_list.size(); i++) {
    auto software = _payload.software_list[i];
    json j_software = {{"manufacturer_id", software.manufacturer_id},
                       {"software_id", software.software_id},
                       {"version_id", software.version_id},
                       {"create_time", software.create_time}};
    j_software_list.push_back(j_software);
  }

  // add other_av and software_list
  json j_payload = {{"other_av", _payload.other_av},
                    {"software_list", j_software_list}};

  return j_payload;
}

// set payload from json
void AvswuPacketInventory::payload_from_json(const json &j) {
  json j_payload = j.at("payload");

  // set other_av
  j_payload.at("other_av").get_to(_payload.other_av);

  // set software_list
  json j_software_list = j_payload.at("software_list");
  // clear item list, set, then push new items
  _payload.software_list.clear();
  for (size_t i = 0; i < j_software_list.size(); i++) {
    auto j_software = j_software_list[i];
    software_t item;
    j_software.at("manufacturer_id").get_to(item.manufacturer_id);
    j_software.at("software_id").get_to(item.software_id);
    j_software.at("version_id").get_to(item.version_id);
    j_software.at("create_time").get_to(item.create_time);
    _payload.software_list.push_back(item);
  }
}

// set payload based on configuration
const bool AvswuPacketInventory::set_payload(AvswuConfig *configP,
                                             const uint64_t manufacturer_id,
                                             const size_t inventory_index) {

  bool found = false;
  size_t m = 0;

  while (m < configP->manufacturer_list().size() && !found) {
    const auto man = configP->manufacturer_list()[m];
    if (man.manufacturer_id == manufacturer_id) {

      // clear the software list
      _payload.software_list.clear();

      // check if inventory_index is valid
      if (inventory_index >= man.inventory_list.size()) {
        spdlog::error("inventory_index={} is invalid for manufacturer_id={} in "
                      "config.json",
                      inventory_index, manufacturer_id);
        return false;
      }
      // get the inventory
      auto inv = man.inventory_list[inventory_index];

      // set the payload software list
      software_t sw;
      sw.manufacturer_id = manufacturer_id;
      for (size_t i = 0; i < inv.software_index_list.size(); i++) {
        size_t sw_item_index = inv.software_index_list[i];
        auto sw_item = man.software_list[sw_item_index];

        // set the sw item
        sw.software_id = sw_item.software_id;
        sw.version_id = sw_item.version_id;
        sw.create_time = sw_item.create_time;
        // add the software item
        _payload.software_list.push_back(sw);
      }

      found = true;
    }
    m++;
  }

  return found;
}

// create hash from payload
bool AvswuPacketInventory::create_hash() {
  // create json for the payload
  const json j_payload = payload_to_json();

  // create hash
  const bool is_success = AvswuPacket::create_hash(j_payload.dump());
  return is_success;
}

// to/from json
void to_json(json &j, const AvswuPacketInventory &p) {
  // create json for the payload
  const json j_payload = p.payload_to_json();
  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketInventory &p) {
  j.at("header").get_to(p._header);
  // handle payload
  p.payload_from_json(j);

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketInventory &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_inventory

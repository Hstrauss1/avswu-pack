#include "AvswuPacketShardRequestList.h"

namespace avswu_packet_shard_request_list {

AvswuPacketShardRequestList::AvswuPacketShardRequestList(
    const uint64_t from_id, mpz_t &from_X, const uint64_t to_id, mpz_t &to_X,
    node_type_t from_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::shard_request_list,
              from_type);

  // 1. build payload, later thru push_back
  // 2. create hash later after payload is built
  // 3. later, sign the pdu
}

AvswuPacketShardRequestList::AvswuPacketShardRequestList() {}

AvswuPacketShardRequestList::~AvswuPacketShardRequestList() {}

// create json for the payload
const json AvswuPacketShardRequestList::payload_to_json() const {
  // create payload json
  json j_shard_request_list = json::array();
  for (size_t i = 0; i < _payload.shard_request_list.size(); i++) {
    auto shard_request = _payload.shard_request_list[i];
    json j_shard_request = {{"manufacturer_id", shard_request.manufacturer_id},
                            {"software_id", shard_request.software_id},
                            {"version_id", shard_request.version_id},
                            {"shard_id", shard_request.shard_id},
                            {"payload_hash", shard_request.shard_hash}};
    j_shard_request_list.push_back(j_shard_request);
  }
  json j_payload = {{"shard_request_list", j_shard_request_list}};

  return j_payload;
}

// remove item from request list
const bool AvswuPacketShardRequestList::remove(const uint64_t manufacturer_id,
                                               const uint64_t software_id,
                                               const uint64_t version_id,
                                               const uint64_t shard_id) {

  bool removed = false;
  size_t i = 0;
  while (i < _payload.shard_request_list.size() && !removed) {
    auto req = _payload.shard_request_list[i];
    if ((req.manufacturer_id == manufacturer_id) &&
        (req.software_id == software_id) && (req.version_id == version_id) &&
        (req.shard_id == shard_id)) {
      // erase the ith element
      _payload.shard_request_list.erase(_payload.shard_request_list.begin() +
                                        i);
      removed = true;
    } else {
      // only increment to next element, if we did not delete an element
      i++;
    }
  }
  return removed;
}

// remove all shard for a specific software update version from request list
const bool AvswuPacketShardRequestList::remove(const uint64_t manufacturer_id,
                                               const uint64_t software_id,
                                               const uint64_t version_id) {
  size_t removed_count = 0;
  size_t i = 0;
  while (i < _payload.shard_request_list.size()) {
    auto req = _payload.shard_request_list[i];
    if ((req.manufacturer_id == manufacturer_id) &&
        (req.software_id == software_id) && (req.version_id == version_id)) {
      // erase the ith element
      _payload.shard_request_list.erase(_payload.shard_request_list.begin() +
                                        i);
      removed_count++;
    } else {
      // only increment to next element, if we did not delete an element
      i++;
    }
  }
  const bool removed_some = removed_count > 0;
  return removed_some;
}

// create hash from payload
bool AvswuPacketShardRequestList::create_hash() {

  // create json for the payload
  const json j_payload = payload_to_json();

  // create hash
  const bool is_success = AvswuPacket::create_hash(j_payload.dump());
  return is_success;
}

// to/from json
void to_json(json &j, const AvswuPacketShardRequestList &p) {

  // create json for the payload
  const json j_payload = p.payload_to_json();

  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketShardRequestList &p) {

  j.at("header").get_to(p._header);

  // handle payload
  json j_payload = j.at("payload");
  json j_shard_request_list = j_payload.at("shard_request_list");
  // clear item list, set, then push new items
  p._payload.shard_request_list.clear();
  for (size_t i = 0; i < j_shard_request_list.size(); i++) {
    auto j_item = j_shard_request_list[i];
    shard_request_t request;
    j_item.at("manufacturer_id").get_to(request.manufacturer_id);
    j_item.at("software_id").get_to(request.software_id);
    j_item.at("version_id").get_to(request.version_id);
    j_item.at("shard_id").get_to(request.shard_id);
    j_item.at("payload_hash").get_to(request.shard_hash);
    p._payload.shard_request_list.push_back(request);
  }

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketShardRequestList &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_shard_request_list

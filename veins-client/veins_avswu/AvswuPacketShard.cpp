#include "AvswuPacketShard.h"

namespace avswu_packet_shard {

AvswuPacketShard::AvswuPacketShard(
    const uint64_t from_id, mpz_t &from_X, const uint64_t to_id, mpz_t &to_X,
    const uint64_t manufacturer_id, const uint64_t software_id,
    const uint64_t version_id, const uint64_t shard_id,
    const uint64_t erasure_id, const uint32_t k, const uint32_t m,
    const uint32_t w, const char *dataP, const size_t data_size,
    const size_t original_file_size) {

  // set shard
  set(from_id, from_X, to_id, to_X, manufacturer_id, software_id, version_id,
      shard_id, erasure_id, k, m, w, dataP, data_size, original_file_size);
}

AvswuPacketShard::AvswuPacketShard(const uint64_t from_id, mpz_t &from_X,
                                   const uint64_t to_id, mpz_t &to_X,
                                   const AvswuShard &shard) {
  // set shard

  // parse the values for setting
  auto manufacturer_id = shard.manufacturer_id();
  auto software_id = shard.software_id();
  auto version_id = shard.version_id();
  auto shard_id = shard.shard_id();
  auto erasure_id = shard.erasure_id();
  auto k = shard.k();
  auto m = shard.m();
  auto w = shard.w();
  vector<char> shard_data = shard.shard_data();
  auto dataP = shard_data.data();
  auto data_size = shard.shard_data().size();
  auto original_file_size = shard.original_file_size();

  set(from_id, from_X, to_id, to_X, manufacturer_id, software_id, version_id,
      shard_id, erasure_id, k, m, w, dataP, data_size, original_file_size);
}

void AvswuPacketShard::set(const uint64_t from_id, mpz_t &from_X,
                           const uint64_t to_id, mpz_t &to_X,
                           const uint64_t manufacturer_id,
                           const uint64_t software_id,
                           const uint64_t version_id, const uint64_t shard_id,
                           const uint64_t erasure_id, const uint32_t k,
                           const uint32_t m, const uint32_t w,
                           const char *dataP, const size_t data_size,
                           const size_t original_file_size) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::shard,
              node_type_t::unknown);

  // build payload
  const vector<char> shard_data(dataP, dataP + data_size);
  _payload.shard.set(manufacturer_id, software_id, version_id, shard_id,
                     erasure_id, k, m, w, original_file_size, shard_data);

  // get hash of pdu
  create_hash();

  // sign the pdu
}

AvswuPacketShard::AvswuPacketShard() {}

AvswuPacketShard::~AvswuPacketShard() {}

// size in bytes
const size_t AvswuPacketShard::size() const {
  size_t n = 0;
  n += _header.size();
  n += _payload.shard.buffer_size();
  n += _statistics.size();
  n += _footer.size();
  return n;
}

// create hash from payload
bool AvswuPacketShard::create_hash() {
  json j_payload = {{"shard", _payload.shard}};

  // create hash
  const bool is_success = AvswuPacket::create_hash(j_payload.dump());
  return is_success;
}

// create json for the payload
const json AvswuPacketShard::payload_to_json() const {
  // create payload json
  json j_payload = {{"shard", _payload.shard}};
  return j_payload;
}

// to/from json
void to_json(json &j, const AvswuPacketShard &p) {

  // create json for the payload
  const json j_payload = p.payload_to_json();

  j = json{{"header", p._header},
           {"payload", j_payload},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacketShard &p) {

  j.at("header").get_to(p._header);

  // handle payload
  json j_payload = j.at("payload");
  j_payload.at("shard").get_to(p._payload.shard);

  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacketShard &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_shard

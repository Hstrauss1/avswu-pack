#include "AvswuPacket.h"
#include "AvswuOpenSSL.h"

namespace avswu_packet {

AvswuPacket::AvswuPacket(const uint64_t from_id, mpz_t &from_X,
                         const uint64_t to_id, mpz_t &to_X) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type_t::none,
              node_type_t::unknown);
}

AvswuPacket::AvswuPacket() {}

AvswuPacket::~AvswuPacket() {}

// creates hash(header|payloa)d based on payload string
bool AvswuPacket::create_hash(const string payload) {

  // hash the payload
  bool is_payload_success =
      avswu_open_ssl::hash_by_type(_footer.get_payload_hash(), payload,
                                   avswu_open_ssl::AVSWU_DATA_HASH_ALGORTITHM);
  if (!is_payload_success) {
    return false;
  }

  // concatenate header and payload as string, header+payload
  const string header = json(_header).dump();
  const string header_payload = header + payload;
  bool is_header_payload_success = avswu_open_ssl::hash_sha512(
      _footer.get_header_payload_hash(), header_payload);

  return is_header_payload_success;
}

// set header and footer, but not the payload itself
const bool AvswuPacket::set(const uint64_t from_id, mpz_t &from_X,
                            const uint64_t to_id, mpz_t &to_X,
                            const string payload_str,
                            payload_type_t payload_type,
                            node_type_t node_type) {

  // set header
  _header.set(from_id, from_X, to_id, to_X, payload_type, node_type);

  // set footer hash
  const bool hash_success = create_hash(payload_str);
  return hash_success;
}

// to/from json
void to_json(json &j, const AvswuPacket &p) {

  j = json{{"header", p._header},
           {"statistics", p._statistics},
           {"footer", p._footer}};
}

void from_json(const json &j, AvswuPacket &p) {

  j.at("header").get_to(p._header);
  j.at("statistics").get_to(p._statistics);
  j.at("footer").get_to(p._footer);
}

// output
ostream &operator<<(ostream &os, AvswuPacket &p) {
  json j(p);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet

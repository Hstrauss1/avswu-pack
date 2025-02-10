#include "AvswuPacketHeader.h"

namespace avswu_packet_header {

AvswuPacketHeader::AvswuPacketHeader(const uint64_t from_id, mpz_t &from_X,
                                     const uint64_t to_id, mpz_t &to_X,
                                     payload_type_t payload_type,
                                     node_type_t from_type) {
  set(from_id, from_X, to_id, to_X, payload_type, from_type);
}

// empty header
AvswuPacketHeader::AvswuPacketHeader() {
  // init header
  const uint64_t temp_from_id = 0;
  mpz_t temp_from_X;
  mpz_init(temp_from_X);

  const uint64_t temp_to_id = 0;
  mpz_t temp_to_X;
  mpz_init(temp_to_X);

  // set create time
  _create_time = 0;

  set(temp_from_id, temp_from_X, temp_to_id, temp_to_X, payload_type_t::none,
      node_type_t::unknown);
}

void AvswuPacketHeader::set(const uint64_t from_id, mpz_t &from_X,
                            const uint64_t to_id, mpz_t &to_X,
                            payload_type_t payload_type,
                            node_type_t from_type) {
  _from_id = from_id;
  mpz_init(_from_X);
  mpz_set(_from_X, from_X);

  _to_id = to_id;
  mpz_init(_to_X);
  mpz_set(_to_X, to_X);

  _payload_type = payload_type;
  _from_type = from_type;
}

AvswuPacketHeader::~AvswuPacketHeader() {}

// size in bytes
const size_t AvswuPacketHeader::size() const {
  size_t n = 0;
  n += sizeof(_create_time);
  n += sizeof(_from_id);
  n += mpz_size(_from_X);
  n += sizeof(_to_id);
  n += mpz_size(_to_X);
  n += sizeof(_payload_type);
  n += sizeof(_from_type);
  return n;
}

// to/from json
void to_json(json &j, const AvswuPacketHeader &h) {

  // convert gmp elem to string
  string from_X(mpz_get_str(nullptr, 16, h._from_X));
  string to_X(mpz_get_str(nullptr, 16, h._to_X));

  j = json{
      {"from_id", h.from_id()},
      {"from_X", from_X},
      {"to_id", h.to_id()},
      {"to_X", to_X},
      {"payload_type", h.payload_type()},
      {"from_type", h.from_type()},
      {"create_time", h.create_time()},
  };
}

const string AvswuPacketHeader::from_type_to_string() const {
  string result;
  switch (_from_type) {
  case node_type_t::unknown:
    result = "unknown";
    break;
  case node_type_t::unknown_av:
    result = "unknown_av";
    break;
  case node_type_t::manufacturer:
    result = "manufacturer";
    break;
  case node_type_t::rsu:
    result = "rsu";
    break;
  case node_type_t::client_av:
    result = "client_av";
    break;
  case node_type_t::delivery_av:
    result = "delivery_av";
    break;
  default:
    spdlog::error("error, no status code");
    break;
  }
  return result;
}

void from_json(const json &j, AvswuPacketHeader &h) {

  j.at("from_id").get_to(h._from_id);
  mpz_init(h._from_X);
  set_mpz_str_from_json(j, "from_X", &h._from_X);

  j.at("to_id").get_to(h._to_id);
  mpz_init(h._to_X);
  set_mpz_str_from_json(j, "to_X", &h._to_X);

  j.at("payload_type").get_to(h._payload_type);
  j.at("from_type").get_to(h._from_type);

  j.at("create_time").get_to(h._create_time);
}

// output
ostream &operator<<(ostream &os, const AvswuPacketHeader &h) {
  json j(h);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_header

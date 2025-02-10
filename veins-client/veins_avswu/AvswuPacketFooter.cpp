#include "AvswuPacketFooter.h"

namespace avswu_packet_footer {

AvswuPacketFooter::AvswuPacketFooter(const string header_payload_hash, mpz_t &R,
                                     mpz_t &s) {
  // set hasn and signature
  set_header_payload_hash(header_payload_hash);
  set_hash_signature(R, s);
}

// empty packet
AvswuPacketFooter::AvswuPacketFooter() {

  // init footer

  set_header_payload_hash("");

  mpz_t temp_R;
  mpz_init(temp_R);
  mpz_t temp_s;
  mpz_init(temp_s);
  set_hash_signature(temp_R, temp_s);
}

AvswuPacketFooter::~AvswuPacketFooter() {}

void AvswuPacketFooter::set_hash_signature(mpz_t &R, mpz_t &s) {
  mpz_init(_hash_signature.R);
  mpz_set(_hash_signature.R, R);
  mpz_init(_hash_signature.s);
  mpz_set(_hash_signature.s, s);
}

// size in bytes
const size_t AvswuPacketFooter::size() const {
  size_t n = 0;
  n += sizeof(_payload_hash);
  n += sizeof(_header_payload_hash);
  n += mpz_size(_hash_signature.R);
  n += mpz_size(_hash_signature.s);
  return n;
}

// to/from json
void to_json(json &j, const AvswuPacketFooter &f) {

  // convert gmp elems to strings
  string hash_signature_R(mpz_get_str(nullptr, 16, f.hash_signature().R));
  string hash_signature_s(mpz_get_str(nullptr, 16, f.hash_signature().s));

  j = json{{"payload_hash", f.payload_hash()},
           {"header_payload_hash", f.header_payload_hash()},
           {"hash_signature_R", hash_signature_R},
           {"hash_signature_s", hash_signature_s}};
}

void from_json(const json &j, AvswuPacketFooter &p) {

  // set hash
  j.at("payload_hash").get_to(p._payload_hash);
  j.at("header_payload_hash").get_to(p._header_payload_hash);

  // set signature
  mpz_init(p._hash_signature.R);
  set_mpz_str_from_json(j, "hash_signature_R", &p._hash_signature.R);

  mpz_init(p._hash_signature.s);
  set_mpz_str_from_json(j, "hash_signature_s", &p._hash_signature.s);
}

// output
ostream &operator<<(ostream &os, AvswuPacketFooter &f) {
  json j(f);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_footer

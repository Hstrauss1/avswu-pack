#pragma once

#include "AvswuUtils.h"
#include "NonConMusigNode.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_utils;
using namespace non_con_musig_node;
using json = nlohmann::json;

namespace avswu_packet_footer {

class AvswuPacketFooter {
private:
  string _payload_hash;
  string _header_payload_hash;
  signature_t _hash_signature;

public:
  // constructors/destructor
  AvswuPacketFooter(const string header_payload_hash, mpz_t &R, mpz_t &s);
  AvswuPacketFooter();
  ~AvswuPacketFooter();

  // hash, and signature const getters
  inline const signature_t &hash_signature() const { return _hash_signature; }
  inline const string &payload_hash() const { return _payload_hash; }
  inline const string &header_payload_hash() const {
    return _header_payload_hash;
  }

  // hash, and signature mutable getters (needed by packet classes to
  // efficiently set hash and signature)
  inline signature_t &get_hash_signature() { return _hash_signature; }
  inline string &get_payload_hash() { return _payload_hash; }
  inline string &get_header_payload_hash() { return _header_payload_hash; }

  // setters
  inline void set_header_payload_hash(const string hash) {
    _header_payload_hash = hash;
  }
  inline void set_payload_hash(const string hash) { _payload_hash = hash; }
  void set_hash_signature(mpz_t &R, mpz_t &s);

  // size in bytes
  const size_t size() const;

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketFooter &f);
  friend void to_json(json &j, const AvswuPacketFooter &f);
  friend void from_json(const json &j, AvswuPacketFooter &f);
};

} // namespace avswu_packet_footer

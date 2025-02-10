#pragma once

#include "AvswuPacketFooter.h"
#include "AvswuPacketHeader.h"
#include "AvswuPacketStatistics.h"
#include "AvswuUtils.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_packet_header;
using namespace avswu_packet_statistics;
using namespace avswu_packet_footer;
using namespace avswu_utils;
using json = nlohmann::json;

namespace avswu_packet {

// base class for all packets
class AvswuPacket {
protected:
  AvswuPacketHeader _header;
  // payload is defined in parent classes
  AvswuPacketStatistics _statistics;
  AvswuPacketFooter _footer;

public:
  // constructors/destructor
  AvswuPacket(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
              mpz_t &to_X);
  AvswuPacket();
  ~AvswuPacket();

  // header getters
  inline AvswuPacketHeader &header() { return _header; }
  inline AvswuPacketStatistics &statistics() { return _statistics; }
  inline AvswuPacketFooter &footer() { return _footer; }

  // creates hash(header|payload) based on payload string
  bool create_hash(const string payload);

  // set header and footer, but not the payload itself
  const bool set(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
                 mpz_t &to_X, const string payload_str,
                 payload_type_t payload_type, node_type_t node_type);

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacket &p);
  friend void to_json(json &j, const AvswuPacket &p);
  friend void from_json(const json &j, AvswuPacket &p);
};

} // namespace avswu_packet

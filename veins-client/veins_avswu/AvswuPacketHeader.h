#pragma once

#include "AvswuUtils.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_utils;
using json = nlohmann::json;

namespace avswu_packet_header {

// enumerated types of payloads
typedef enum payload_type_t {
  none = -1,
  // list of manufacturers supported by a RSU
  support_list,
  // client av inventory sent to rsu or delivery av
  inventory,
  // list of notifications
  notification_list,
  // list of requested shards
  shard_request_list,
  // a shard
  shard,
  // delivery av request sent to rsu
  delivery_request,
} payload_type_t;

// enumerated types for what type of node sent the package
typedef enum node_type_t {
  // unknown node
  unknown = -2,
  // node is an av, but type of av is unknown
  unknown_av = -1,
  // known types
  manufacturer,
  rsu,
  client_av,
  delivery_av
} node_type_t;

class AvswuPacketHeader {
private:
  // simulation time this packet was created
  double _create_time;

  // from/to id
  uint64_t _from_id;
  mpz_t _from_X;
  uint64_t _to_id;
  mpz_t _to_X;

  // payload and node type identification
  payload_type_t _payload_type;
  node_type_t _from_type;

public:
  // constructors/destructor
  AvswuPacketHeader(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
                    mpz_t &to_X, payload_type_t payload_type,
                    node_type_t from_type);
  AvswuPacketHeader();
  ~AvswuPacketHeader();

  // header getters
  inline const uint64_t from_id() const { return _from_id; }
  inline mpz_t &from_X() { return _from_X; }
  inline const uint64_t to_id() const { return _to_id; }
  inline mpz_t &to_X() { return _to_X; }
  inline const payload_type_t payload_type() const { return _payload_type; }
  inline const node_type_t from_type() const { return _from_type; }

  // for logging
  const string from_type_to_string() const;

  // from_type
  inline void set_from_type(node_type_t from_type) { _from_type = from_type; }

  //  stats getters
  inline const double create_time() const { return _create_time; }

  // setters
  void set(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
           mpz_t &to_X, payload_type_t payload_type, node_type_t from_type);
  void set_create_time(const double t) { _create_time = t; }

  // size in bytes
  const size_t size() const;

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketHeader &h);
  friend void to_json(json &j, const AvswuPacketHeader &h);
  friend void from_json(const json &j, AvswuPacketHeader &h);
};

} // namespace avswu_packet_header

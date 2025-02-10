#pragma once

#include "AvswuUtils.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_utils;
using json = nlohmann::json;

namespace avswu_packet_statistics {

class AvswuPacketStatistics {
private:
  // number of time this packet was sent (number of hops)
  size_t _hop_count;
  // time a packet was sent, and when it was recieved
  double _send_time;
  double _receive_time;
  // simulation time this packet shard was requested by a client av
  double _client_shard_request_time;

public:
  // constructors/destructor
  AvswuPacketStatistics();
  ~AvswuPacketStatistics();

  //  stats getters/setters
  inline const size_t hop_count() const { return _hop_count; }
  void set_hop_count(const size_t n) { _hop_count = n; }
  inline const size_t client_shard_request_time() const {
    return _client_shard_request_time;
  }
  void set_client_shard_request_time(const double t) {
    _client_shard_request_time = t;
  }
  inline const size_t send_time() const { return _send_time; }
  void set_send_time(const double t) { _send_time = t; }
  inline const size_t receive_time() const { return _receive_time; }
  void set_receive_time(const double t) { _receive_time = t; }

  // size in bytes
  inline const size_t size() const { return sizeof(AvswuPacketStatistics); }

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketStatistics &h);
  friend void to_json(json &j, const AvswuPacketStatistics &h);
  friend void from_json(const json &j, AvswuPacketStatistics &h);
};

} // namespace avswu_packet_statistics

#include "AvswuPacketStatistics.h"

namespace avswu_packet_statistics {

AvswuPacketStatistics::AvswuPacketStatistics() {

  // initialize count to 0
  _hop_count = 0;

  // initialize time stats to 0
  _send_time = 0.0f;
  _receive_time = 0.0f;
  _client_shard_request_time = 0.0f;
}

AvswuPacketStatistics::~AvswuPacketStatistics() {}

// to/from json
void to_json(json &j, const AvswuPacketStatistics &h) {

  /*
    double _send_time;
  double _receive_time;
  // simulation time this packet shard was requested
  double _shard_request_time;
  */

  j = json{
      {"sent_count", h.hop_count()},
      {"send_time", h.send_time()},
      {"receive_time", h.receive_time()},
      {"client_shard_request_time", h.client_shard_request_time()},
  };
}

void from_json(const json &j, AvswuPacketStatistics &h) {
  j.at("sent_count").get_to(h._hop_count);
  j.at("send_time").get_to(h._send_time);
  j.at("receive_time").get_to(h._receive_time);
  j.at("client_shard_request_time").get_to(h._client_shard_request_time);
}

// output
ostream &operator<<(ostream &os, const AvswuPacketStatistics &h) {
  json j(h);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_packet_statistics

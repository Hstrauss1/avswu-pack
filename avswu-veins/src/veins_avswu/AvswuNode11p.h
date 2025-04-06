#pragma once

#include <nlohmann/json_fwd.hpp>
#include <string>

#include "veins/modules/application/ieee80211p/DemoBaseApplLayer.h"

#include "AvswuPacketHeader.h"

// 802.11p is 27Mbps
#define AVSWU_802_11P_BYTES_PER_SECOND                                         \
  (static_cast<double>(27U * 1024 * 1024) / static_cast<double>(8U));

using namespace std;
using json = nlohmann::json;

using namespace avswu_packet_header;

namespace veins {

class VEINS_API AvswuNode11p : public DemoBaseApplLayer {

protected:
  // node type
  node_type_t _node_type = node_type_t::unknown;
  // send a packet (send packet now, without time advance/delay for normal usage
  // outside of tight-for loops)
  inline const bool send_packet(const json &packet_json,
                                const string node_type) {
    return send_packet_delayed(packet_json, node_type, 0.0f);
  }
  // send a packet for usage within for-loops to add delay
  const bool send_packet_delayed(const json &packet_json,
                                 const string node_type, double delay);
  // schedule a packet
  const bool schedule_packet(const json &packet_json, const string node_type,
                             const double schedule_time);

  inline const node_type_t node_type() const { return _node_type; }

  // size of the software update
  size_t software_update_size_mb = 0;

  // increase data size
  void increase_data_size(vector<char> &data, size_t const scale);
};

} // namespace veins

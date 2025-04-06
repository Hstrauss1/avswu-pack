#pragma once

#include <omnetpp/chistogram.h>

using namespace omnetpp;

namespace avswu_av_stat {

// stats
typedef struct stat_t {
  // time to receive this shard -- at decode, current_time - create_time
  cHistogram shard_time_to_receive;
  // sw time to update (during decode), current_time - software create_time
  cHistogram sw_time_to_update_from_create_time;
  // sw time to update (during decode), current_time - min(client shard request
  // time)
  cHistogram sw_time_to_update_from_client_shard_request_time;
  // shard number of avs/hops from rsu to target av for a shard, hop_count
  // is tracked per shard in the statistics of the shard
  cHistogram shard_hop_count;
  // scalar totals/counts
  uint64_t packet_sent = 0;
  uint64_t packet_received = 0;
  uint64_t shard_sent = 0;
  uint64_t shard_received = 0;
  uint64_t sw_update_received = 0;
  // start time
  double depart_time;
} stat_t;

} // namespace avswu_av_stat

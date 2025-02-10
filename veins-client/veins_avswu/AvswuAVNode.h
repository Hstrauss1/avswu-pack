#pragma once

#include "AvswuNode.h"
#include "AvswuPacketNotificationList.h"
#include "AvswuPacketShard.h"
#include "AvswuPacketShardRequestList.h"
#include "NonConMusigSigner.h"
#include <gmp.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace avswu_packet_shard;
using namespace avswu_packet_notification_list;
using namespace avswu_packet_shard_request_list;
using namespace avswu_node;

namespace avswu_av_node {

enum decode_code_t {
  DECODE_SUCCESS = 1,
  NO_SHARDS,
  INVALID_PARAMS,
  NOT_ENOUGH_SHARDS,
  DECODE_FAIL
};

typedef struct {
  decode_code_t code;
  string message;
} decode_status_t;

class AvswuAVNode : public AvswuNode {
private:
  // nc musig
  NonConMusigSigner _signer;

  // check if we can decode a list of shards
  const decode_status_t
  can_decode(const vector<packet_shardP_t> packet_shardP_list) const;
  // set erasures array based the set of shard erasure_id's that are missing
  void identify_erasures(int erasures[],
                         vector<packet_shardP_t> packet_shardP_list,
                         const uint32_t k, const uint32_t m);

public:
  AvswuAVNode(const uint32_t id, const size_t cache_max_size);
  ~AvswuAVNode();

  // getter
  inline mpz_t &X() { return _signer.X(); }

  // decode shards
  const decode_status_t decode(vector<char> &decoded_output_data,
                               double &earliest_shard_request_time,
                               const uint64_t manufacturer_id,
                               const uint64_t software_id,
                               const uint64_t version_id);

  // log based on decode status
  void spdlog_decode_status(decode_status_t status);
  string decode_status_description(decode_status_t status);

  // write (decoded) data to disk
  void write_file(const string output_file, const vector<char> data) const;

  // packet methods
  bool insert(const AvswuPacketShard &packet_shard);

  // set shard request list for AV or RSU
  AvswuPacketShardRequestList
      // create list of shards based on notification, and which shards are not
      // in our cache
      *
      create_shard_request_list(const uint64_t to_id, mpz_t &to_X,
                                node_type_t node_type,
                                AvswuPacketNotificationList &notification_list);

  // gets shards from cache based on request list, and returns packets
  // method must exists in both AvswuAVNode and AvswuRSUNode due
  // to class hierarchy design, and for veins sim
  vector<AvswuPacketShard>
  get_request_packet_list(const uint64_t to_id, mpz_t &to_X,
                          node_type_t node_type,
                          AvswuPacketShardRequestList &shard_request_list);
};

} // namespace avswu_av_node
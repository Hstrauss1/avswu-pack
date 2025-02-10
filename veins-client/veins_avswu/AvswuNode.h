#pragma once

#include "AvswuShardCache.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigNode.h"
#include <gmp.h>

using namespace std;
using namespace avswu_shard_cache;
using namespace non_con_musig_node;
using namespace non_con_musig_aggregator;
using namespace avswu_packet_shard;

namespace avswu_node {

class AvswuNode {
private:
  uint64_t _id;

protected:
  AvswuShardCache _cache;

  // mallocs erasure blocks
  char **malloc_erasure_blocks(const uint32_t n, const size_t block_size);

public:
  AvswuNode(const uint64_t id, const size_t cache_max_size);
  ~AvswuNode();

  // getters
  inline const uint64_t id() { return _id; }
  const inline AvswuShardCache &cache() const { return _cache; }

  // sign a message, signature <- sign(message,aggregator)
  // this method is in this class so the Manufacturer, RSU, and
  // AV which ineherit from AvswuNode, can use method without code duplication
  bool sign(signature_t &signature, const string message,
            const NonConMusigAggregator &aggregator);

  // gets a vector of shard pointers from cache given a shard id request list
  vector<packet_shardP_t>
  get_shardP_list(const vector<shard_request_t> &shard_request_list) const;

  // read a file
  const vector<char> read_file(const string input_file) const;
};

} // namespace avswu_node
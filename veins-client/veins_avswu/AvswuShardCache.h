#pragma once

#include <deque>
#include <iostream>
#include <nlohmann/json.hpp>

#include "AvswuPacketShard.h"
#include "AvswuShardCacheMap.h"
#include "AvswuShardCacheSetMap.h"

using namespace std;
using json = nlohmann::json;
using namespace avswu_shard;
using namespace avswu_shard_cache_map;
using namespace avswu_shard_cache_set_map;

namespace avswu_shard_cache {

class AvswuShardCache {

private:
  // cache current and maximum size
  size_t _size;
  size_t _max_size;

  // cache stats
  uint32_t _hit;
  uint32_t _miss;

  // fifo queue of all shards currently in the cache
  deque<packet_shardP_t> _shard_deque;
  // map of all shards in cache (manufactuer_id,software_id,shard_id) -> shard
  shard_map_t _shard_map;
  // map of sets of shards in cache
  // (manufactuer_id,software_id) -> set of shards
  shard_set_map_t _shard_set_map;

  // free space
  bool free_space(const size_t need);

public:
  // constructor/destructor
  AvswuShardCache(size_t max_size);
  ~AvswuShardCache();

  inline const size_t size() const { return _size; }
  inline const size_t max_size() const { return _max_size; }
  inline const size_t hit() const { return _hit; }
  inline const size_t miss() const { return _miss; }

  // insert a shard in to the cache
  void insert(const AvswuPacketShard &packet_shard);
  // contains
  const bool contains(const ShardMapKey &sm_key) const;
  const bool contains(const uint64_t manufacturer_id,
                      const uint64_t software_id, const uint64_t version_id,
                      const uint64_t shard_id) const;

  // getters
  // get all shards
  const vector<packet_shardP_t> get() const;
  // get a specific shard
  packet_shardP_t get(const uint64_t manufacturer_id,
                      const uint64_t software_id, const uint64_t version_id,
                      const uint64_t shard_id) const;
  // get all shards for manufacturer id, and software id
  vector<packet_shardP_t> get(const uint64_t manufacturer_id,
                              const uint64_t software_id,
                              const uint64_t version_id) const;
  // count # of shards
  const size_t count(const uint64_t manufacturer_id, const uint64_t software_id,
                     const uint64_t version_id) const;

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuShardCache &c);
  friend void to_json(json &j, const AvswuShardCache &c);
};

} // namespace avswu_shard_cache
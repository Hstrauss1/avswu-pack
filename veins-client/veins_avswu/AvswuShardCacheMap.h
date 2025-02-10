#pragma once

#include <nlohmann/json.hpp>

#include "AvswuPacketShard.h"

using namespace std;
using json = nlohmann::json;
using namespace avswu_packet_shard;

namespace avswu_shard_cache_map {

/*
 * map that is (manufacturer id, software id, shard id) -> shard pointer
 */
// key for unordered shard map
class ShardMapKey {
private:
  uint64_t _manufacturer_id;
  uint64_t _software_id;
  uint64_t _version_id;
  uint64_t _shard_id;

public:
  // constructor/destructor
  ShardMapKey(const uint64_t manufacturer_id, const uint64_t software_id,
              const uint64_t version_id, const uint64_t shard_id);
  ~ShardMapKey();
  // getters
  inline const uint64_t manufacturer_id() const { return _manufacturer_id; }
  inline const uint64_t software_id() const { return _software_id; }
  inline const uint64_t version_id() const { return _version_id; }
  inline const uint64_t shard_id() const { return _shard_id; }
  // comparison, needed for unordered map
  bool operator==(const ShardMapKey sm_key) const {
    return (_manufacturer_id == sm_key.manufacturer_id()) &&
           (_software_id == sm_key.software_id()) &&
           (_version_id == sm_key.version_id()) &&
           (_shard_id == sm_key.shard_id());
  }
};

// custom hash for unordered shard map key
class ShardMapHash {
public:
  // fast bit-wise hash function
  size_t operator()(const ShardMapKey &smk) const {
    size_t hash = ((smk.manufacturer_id() | smk.version_id()) &
                   (smk.software_id() | smk.shard_id())) %
                  SIZE_MAX;
    return hash;
  }
};

// map type
typedef unordered_map<ShardMapKey, packet_shardP_t, ShardMapHash> shard_map_t;

} // namespace avswu_shard_cache_map
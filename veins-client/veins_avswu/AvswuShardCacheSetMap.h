#pragma once

#include <nlohmann/json.hpp>

#include <unordered_set>

#include "AvswuPacketShard.h"

using namespace std;
using json = nlohmann::json;
using namespace avswu_packet_shard;

namespace avswu_shard_cache_set_map {

/*
 * map that is (manufacturer id, software id, shard id) -> shard pointer
 */
// key for unordered shard map
class ShardSetMapKey {
private:
  uint64_t _manufacturer_id;
  uint64_t _software_id;
  uint64_t _version_id;

public:
  // constructor/destructor
  ShardSetMapKey(const uint64_t manufacturer_id, const uint64_t software_id,
                 const uint64_t version_id);
  ~ShardSetMapKey();
  // getters
  inline const uint64_t manufacturer_id() const { return _manufacturer_id; }
  inline const uint64_t software_id() const { return _software_id; }
  inline const uint64_t version_id() const { return _version_id; }
  // comparison, needed for unordered map
  bool operator==(const ShardSetMapKey ssm_key) const {
    return (_manufacturer_id == ssm_key.manufacturer_id()) &&
           (_version_id == ssm_key.version_id()) &&
           (_software_id == ssm_key.software_id());
  }
};

// custom hash for unordered shard map key
class ShardMapHash {
public:
  // fast bit-wise hash function
  size_t operator()(const ShardSetMapKey &smk) const {
    size_t hash =
        (smk.manufacturer_id() & smk.software_id() & smk.version_id()) %
        SIZE_MAX;
    return hash;
  }
};

// map type
// typedef unordered_map<ShardSetMapKey, shardP_t, ShardMapHash>
// shard_set_map_t;
typedef unordered_map<ShardSetMapKey, unordered_set<packet_shardP_t>,
                      ShardMapHash>
    shard_set_map_t;

} // namespace avswu_shard_cache_set_map
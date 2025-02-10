#include "AvswuShardCacheMap.h"
#include <spdlog/spdlog.h>

namespace avswu_shard_cache_map {

// constructor
ShardMapKey::ShardMapKey(const uint64_t manufacturer_id,
                         const uint64_t software_id, const uint64_t version_id,
                         const uint64_t shard_id)
    : _manufacturer_id(manufacturer_id), _software_id(software_id),
      _version_id(version_id), _shard_id(shard_id) {}
// destructor
ShardMapKey::~ShardMapKey() {}

} // namespace avswu_shard_cache_map
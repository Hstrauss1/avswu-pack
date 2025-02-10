#include "AvswuShardCacheSetMap.h"
#include <spdlog/spdlog.h>

namespace avswu_shard_cache_set_map {

// constructor
ShardSetMapKey::ShardSetMapKey(const uint64_t manufacturer_id,
                               const uint64_t software_id,
                               const uint64_t version_id)
    : _manufacturer_id(manufacturer_id), _software_id(software_id),
      _version_id(version_id) {}
// destructor
ShardSetMapKey::~ShardSetMapKey() {}

} // namespace avswu_shard_cache_set_map
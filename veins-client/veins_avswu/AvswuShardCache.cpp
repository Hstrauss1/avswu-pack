#include "AvswuShardCache.h"
#include <spdlog/spdlog.h>

namespace avswu_shard_cache {

// constructor
AvswuShardCache::AvswuShardCache(size_t max_size)
    : _size(0), _max_size(max_size), _hit(0), _miss(0) {}

// destructor
AvswuShardCache::~AvswuShardCache() {}

// free space by removing some old shards (FIFO queue)
bool AvswuShardCache::free_space(const size_t need) {

  if (need > _max_size) {
    return false;
  }

  // while the need > space available, free space
  while (need > (_max_size - _size)) {
    // get the oldest shard
    auto shardP = _shard_deque.front();

    // get shard id info
    const uint64_t manufacturer_id = shardP->shard().manufacturer_id();
    const uint64_t software_id = shardP->shard().software_id();
    const uint64_t version_id = shardP->shard().version_id();
    const uint64_t shard_id = shardP->shard().shard_id();
    const uint64_t buffer_size = shardP->shard().buffer_size();

    // delete from shard map
    ShardMapKey sm_key(manufacturer_id, software_id, version_id, shard_id);
    _shard_map.erase(sm_key);

    // delete from set map
    ShardSetMapKey ssm_key(manufacturer_id, software_id, version_id);
    _shard_set_map[ssm_key].erase(shardP);

    // remove from fifo queue
    _shard_deque.pop_front();

    // update the cache size
    _size -= buffer_size;

    // report removed shard
    spdlog::info("need={} bytes, removed shard=({},{},{}) size={}", need,
                 manufacturer_id, software_id, shard_id, buffer_size);
  }

  return true;
}

const bool AvswuShardCache::contains(const ShardMapKey &sm_key) const {
  const bool has_shard = _shard_map.find(sm_key) != _shard_map.end();
  return has_shard;
}

const bool AvswuShardCache::contains(const uint64_t manufacturer_id,
                                     const uint64_t software_id,
                                     const uint64_t version_id,
                                     const uint64_t shard_id) const {
  ShardMapKey sm_key(manufacturer_id, software_id, version_id, shard_id);
  const bool has_shard = contains(sm_key);
  return has_shard;
}

// insert a shard in to the cache
void AvswuShardCache::insert(const AvswuPacketShard &packet_shard) {

  // if shard is already in cache, return
  ShardMapKey sm_key(packet_shard.shard().manufacturer_id(),
                     packet_shard.shard().software_id(),
                     packet_shard.shard().version_id(),
                     packet_shard.shard().shard_id());
  if (contains(sm_key)) {
    // spdlog::debug("cache hit");
    _hit += 1;
    return;
  }
  // spdlog::debug("cache miss");
  _miss += 1;

  // create smart pointer to a new shard
  const packet_shardP_t packet_shardP(new AvswuPacketShard(packet_shard));

  if (packet_shardP == nullptr) {
    const string msg = "out of memory. unable create a new shard";
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // add to queue
  _shard_deque.push_back(packet_shardP);

  // add to shard map
  _shard_map[sm_key] = packet_shardP;

  // free space by removing some old shards (FIFO queue)
  free_space(packet_shard.size());

  // add the new shard set map
  const ShardSetMapKey ssm_key(packet_shard.shard().manufacturer_id(),
                               packet_shard.shard().software_id(),
                               packet_shard.shard().version_id());
  _shard_set_map[ssm_key].insert(packet_shardP);

  // update the cache size
  _size += packet_shard.size();
}

// get vector of shards
const vector<packet_shardP_t> AvswuShardCache::get() const {
  vector<packet_shardP_t> result;
  for (size_t i = 0; i < _shard_deque.size(); i++) {
    result.push_back(_shard_deque[i]);
  }
  return result;
}

// get a specific shard
packet_shardP_t AvswuShardCache::get(const uint64_t manufacturer_id,
                                     const uint64_t software_id,
                                     const uint64_t version_id,
                                     const uint64_t shard_id) const {
  const ShardMapKey sm_key(manufacturer_id, software_id, version_id, shard_id);
  // if the shard is in the map, return it
  if (_shard_map.count(sm_key) == 0) {
    const string msg = "trying to get a shard that does not exist in the cache";
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  packet_shardP_t shardP = _shard_map.at(sm_key);
  return shardP;
}

// get all shards for manufacturer id, and software id
vector<packet_shardP_t> AvswuShardCache::get(const uint64_t manufacturer_id,
                                             const uint64_t software_id,
                                             const uint64_t version_id) const {
  vector<packet_shardP_t> result;

  // get the set
  ShardSetMapKey ssm_key(manufacturer_id, software_id, version_id);
  // if set is not in the map, return
  if (_shard_set_map.count(ssm_key) == 0) {
    return result;
  }

  // construct the set of shards
  auto shard_set = _shard_set_map.at(ssm_key);
  for (auto it = shard_set.begin(); it != shard_set.end(); it++) {
    packet_shardP_t shardP = *it;
    result.push_back(shardP);
  }

  return result;
}

const size_t AvswuShardCache::count(const uint64_t manufacturer_id,
                                    const uint64_t software_id,
                                    const uint64_t version_id) const {

  // get the set
  ShardSetMapKey ssm_key(manufacturer_id, software_id, version_id);
  // if set is in the map, return size of set
  if (_shard_set_map.count(ssm_key) == 1) {
    auto shard_set = _shard_set_map.at(ssm_key);
    return shard_set.size();
  }

  return 0;
}

// to json
void to_json(json &j, const AvswuShardCache &c) {
  json arr = json::array();
  for (size_t i = 0; i < c._shard_deque.size(); i++) {
    json j_shard = json(*c._shard_deque[i]);
    arr.push_back(j_shard);
  }
  j = json{{"size", 0}, {"max_size", c.max_size()}, {"shard_queue", arr}};
}

// output
ostream &operator<<(ostream &os, const AvswuShardCache &c) {
  json j(c);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_shard_cache
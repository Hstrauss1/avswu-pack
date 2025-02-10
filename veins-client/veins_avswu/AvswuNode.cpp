#include "AvswuNode.h"
#include "AvswuOpenSSL.h"
#include <fstream>
#include <gmp.h>
#include <spdlog/spdlog.h>

using namespace std;
using namespace avswu_utils;

namespace avswu_node {

// constructors
AvswuNode::AvswuNode(const uint64_t id, const size_t cache_max_size)
    : _id(id), _cache(cache_max_size) {}
// destructor
AvswuNode::~AvswuNode() {}

// mallocs erasure blocks
char **AvswuNode::malloc_erasure_blocks(const uint32_t n,
                                        const size_t block_size) {
  char **data = (char **)malloc(sizeof(char *) * n);
  for (size_t i = 0; i < n; i++) {
    data[i] = (char *)malloc(sizeof(char) * block_size);
    if (data[i] == NULL) {
      const string msg = "unable to malloc block data";
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }
  }
  return data;
}

// sign a message, signature <- sign(message,aggregator)
bool AvswuNode::sign(signature_t &signature, const string message,
                     const NonConMusigAggregator &aggregator) {

  bool is_success = true;

  mpz_set_ui(signature.R, 1);
  mpz_set_ui(signature.s, 2);

  return is_success;
}

// gets shards from cache based on a shard request list
vector<packet_shardP_t> AvswuNode::get_shardP_list(
    const vector<shard_request_t> &shard_request_list) const {

  vector<packet_shardP_t> shardP_list;

  // for each shard requested
  for (size_t i = 0; i < shard_request_list.size(); i++) {
    auto request = shard_request_list[i];

    // get info for the requested shard
    auto manufacturer_id = request.manufacturer_id;
    auto software_id = request.software_id;
    auto version_id = request.version_id;
    auto shard_id = request.shard_id;
    auto shard_hash = request.shard_hash;

    // if requested shard is in cache, add to shard_list
    ShardMapKey sm_key(manufacturer_id, software_id, version_id, shard_id);
    if (_cache.contains(sm_key)) {
      // read shard from cache,
      // cache");
      auto packet_shardP =
          _cache.get(manufacturer_id, software_id, version_id, shard_id);

      // check payload hash
      string cache_payload_hash;
      const string cache_payload_shard_str =
          packet_shardP->payload_to_json().dump();
      bool is_success = avswu_open_ssl::hash_by_type(
          cache_payload_hash, cache_payload_shard_str,
          avswu_open_ssl::AVSWU_DATA_HASH_ALGORTITHM);
      if (is_success) {
        // make sure hash's match
        if (cache_payload_hash != shard_hash) {
          const string error_msg = fmt::format(
              "unable to verify payload hash of shard requested "
              "and the hash of the "
              "shard in cache\n\tcache_payload_hash={}\n\tshard_hash={}",
              cache_payload_hash, shard_hash);
          spdlog::error(error_msg);
          // TODO: TEMP DEBUG
          const string tmp =
              fmt::format("cache_payload_shard_str={}",
                          json(packet_shardP->payload_to_json().dump(4)));
          spdlog::debug(tmp);
        } else {
          // add to list
          shardP_list.push_back(packet_shardP);
        }
      } else {
        spdlog::error("unable to create payload_hash of shard payload");
      }
    } else {
      spdlog::warn(
          "unable to find shard from request list in cache for {}",
          avswu_id(manufacturer_id, software_id, version_id, shard_id));
    }
  }
  return shardP_list;
}

// read file
const vector<char> AvswuNode::read_file(const string input_file) const {
  // open file
  ifstream input(input_file, ifstream::binary);
  if (!input.is_open()) {
    const string msg = "unable to open input_file=" + input_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // copy data info buffer
  vector<char> data_buf(istreambuf_iterator<char>(input), {});

  vector<char> data;
  for (size_t i = 0; i < data_buf.size(); i++) {
    data.push_back(data_buf[i]);
  }

  // close file
  input.close();

  return data;
}

} // namespace avswu_node
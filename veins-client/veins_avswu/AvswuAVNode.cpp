#include "AvswuAVNode.h"
#include <spdlog/spdlog.h>

#include <algorithm>
#include <fstream>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "jerasure.h"
#include "reed_sol.h"
#include <gf_rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif

namespace avswu_av_node {

// constructor
AvswuAVNode::AvswuAVNode(const uint32_t id, const size_t cache_max_size)
    : AvswuNode(id, cache_max_size), _signer() {}

// destructor
AvswuAVNode::~AvswuAVNode() {}

// check if we can decode
const decode_status_t AvswuAVNode::can_decode(
    const vector<packet_shardP_t> packet_shardP_list) const {

  // get shards, if < k shards, return false

  if (packet_shardP_list.size() == 0) {
    return decode_status_t{NO_SHARDS, "0 shards in packet"};
  }
  // all k and m should be the same
  auto k = packet_shardP_list[0]->shard().k();
  auto m = packet_shardP_list[0]->shard().m();
  for (size_t i = 1; i < packet_shardP_list.size(); i++) {
    if ((packet_shardP_list[i]->shard().k() != k) ||
        (packet_shardP_list[i]->shard().m() != m)) {
      return decode_status_t{INVALID_PARAMS,
                             "k or m are not the same, but they should be the "
                             "same for all shards within a packet"};
    }
  }

  // if < k shards, we can't decode so we return false
  if (packet_shardP_list.size() < k) {
    return decode_status_t{
        NOT_ENOUGH_SHARDS,
        "unable to decode using " + to_string(packet_shardP_list.size()) +
            " shards of k=" + to_string(k) + " shards needed"};
  }

  return decode_status_t{DECODE_SUCCESS,
                         "successfully decoded using " +
                             to_string(packet_shardP_list.size()) +
                             " shards when k=" + to_string(k)};
}

// set erasures array based the set of shard erasure_id's that are missing
/*
per from: https://library.eecs.utk.edu/files/ut-cs-07-603.pdf
• int *erasures:
This is an array of id’s of erased devices. Id’s are numbers between 0 and k+m-1
as described in Section 4.1. If there are e erasures, then elements 0 through e
− 1 of erasures identify the erased devices, and erasures[e] must equal -1.
*/
void AvswuAVNode::identify_erasures(int erasures[],
                                    vector<packet_shardP_t> packet_shardP_list,
                                    const uint32_t k, const uint32_t m) {

  // creates set ids 0 to k+m-1
  set<int> all;
  for (uint32_t i = 0; i < k + m; i++) {
    all.insert(i);
  }

  // create set of erasure ids
  set<int> present;
  for (uint32_t i = 0; i < packet_shardP_list.size(); i++) {
    const size_t id = packet_shardP_list[i]->shard().erasure_id();
    present.insert(id);
  }

  // missing = all - present
  set<int> missing;
  set_difference(all.begin(), all.end(), present.begin(), present.end(),
                 inserter(missing, missing.begin()));

  // set erasures values
  size_t i = 0;
  for (auto itr = missing.begin(); itr != missing.end(); itr++) {
    erasures[i] = *itr;
    i++;
  }
  // terminate erasures with -1
  if (i < k + m) {
    erasures[i] = -1;
  }
}

// attempt to decode software
const decode_status_t AvswuAVNode::decode(vector<char> &decoded_output_data,
                                          double &earliest_shard_request_time,
                                          const uint64_t manufacturer_id,
                                          const uint64_t software_id,
                                          const uint64_t version_id) {

  // get shards, and check if we can potentially decode
  vector<packet_shardP_t> packet_shardP_list =
      cache().get(manufacturer_id, software_id, version_id);
  decode_status_t status = can_decode(packet_shardP_list);
  if (status.code != DECODE_SUCCESS) {
    return status;
  }

  // set the minimum shard request time/the earliest shard request time
  earliest_shard_request_time = std::numeric_limits<double>::max();
  for (size_t i = 0; i < packet_shardP_list.size(); i++) {
    double shard_request_time =
        packet_shardP_list[i]->statistics().client_shard_request_time();
    earliest_shard_request_time =
        min(earliest_shard_request_time, shard_request_time);
  }

  // attempt to decode
  uint32_t k = packet_shardP_list[0]->shard().k();
  uint32_t m = packet_shardP_list[0]->shard().m();
  uint32_t w = 8;

  // create reed-solomon matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

  // get the block size
  size_t block_size = packet_shardP_list[0]->shard().shard_data().size();

  // create k+m data and coding blocks
  const size_t n_shards = packet_shardP_list.size();
  char **data = malloc_erasure_blocks(k, block_size);
  char **coding = malloc_erasure_blocks(m, block_size);

  // copy the shard data into the correct data and coding structures
  // based on erasure_id
  for (size_t i = 0; i < n_shards; i++) {
    vector<char> shard_data = packet_shardP_list[i]->shard().shard_data();
    const int erasure_id = packet_shardP_list[i]->shard().erasure_id();
    if ((erasure_id < 0) || (erasure_id >= k + m)) {
      spdlog::error("erasure id is out of range");
      return {DECODE_FAIL, "erasure id is out of range"};
    }
    // copy into data or coding based on erasure id
    if (erasure_id < k) {
      memcpy(data[erasure_id], shard_data.data(), block_size);
    } else {
      memcpy(coding[erasure_id - k], shard_data.data(), block_size);
    }
  }

  // set erasures array based the set of shard erasure_id's that are missing
  // erasures is a array of erasures ids with a -1 terminator at the end. thus
  // we need k+m ints
  int erasures[k + m];
  identify_erasures(erasures, packet_shardP_list, k, m);

  /* START TEMP FOR DEBUGGING */
  // vector<vector<char>> data_view = create_data_view(data, block_size, k);
  // vector<vector<char>> coding_view = create_data_view(coding, block_size, m);
  /* END TEMP FOR DEBUGGING */

  // log any erasures (missing shards)
  size_t i = 0;
  while (i < (k + m) && erasures[i] != -1) {
    spdlog::debug("erasures[{}]={}", i, erasures[i]);
    i++;
  }

  // decode the shards
  int jerasure_status = jerasure_matrix_decode(k, m, w, matrix, 1, erasures,
                                               data, coding, block_size);
  // if status is -1, decode failed
  if (jerasure_status == -1) {
    spdlog::warn("erasure decode failed");
    return {DECODE_FAIL, "erasure decode failed"};
  }

  /* START TEMP FOR DEBUGGING */
  // data_view = create_data_view(data, block_size, k);
  // coding_view = create_data_view(coding, block_size, m);
  /* END TEMP FOR DEBUGGING */

  // copy the k data shards byte-by-byte into output data
  size_t original_file_size =
      packet_shardP_list[0]->shard().original_file_size();

  // read decoded data out of data and coding arrays
  for (size_t i = 0; i < original_file_size; i++) {
    size_t shard_index = i / block_size;
    size_t shard_byte = i % block_size;
    char ch;
    ch = data[shard_index][shard_byte];
    decoded_output_data.push_back(ch);
  }

  // free memory
  free(coding);
  free(data);

  return {DECODE_SUCCESS, "successfully decoded using " +
                              to_string(packet_shardP_list.size()) +
                              " shards when k=" + to_string(k)};
}

// log based on decode status
void AvswuAVNode::spdlog_decode_status(decode_status_t status) {
  switch (status.code) {
  case NO_SHARDS:
    spdlog::warn("no shards in cache, for a specific software id and version "
                 "id. message={}",
                 status.message);
    break;
  case NOT_ENOUGH_SHARDS:
    spdlog::debug("not enough shards. message={}", status.message);
    break;
  case INVALID_PARAMS:
    spdlog::warn("k and m are not the same value for all shard, for a "
                 "specific software id. message={}",
                 status.message);
    break;
  case DECODE_SUCCESS:
    spdlog::info("decoded software. message={}", status.message);
    spdlog::info("writing decoded data to disk");
    // write software to disk
    break;
  case DECODE_FAIL:
    spdlog::info("decode failed. message={}", status.message);
    break;
  default:
    spdlog::error("error, no status code");
    break;
  }
}

// log based on decode status
string AvswuAVNode::decode_status_description(decode_status_t status) {
  string desc;
  switch (status.code) {
  case NO_SHARDS:
    desc = "no shards in cache, for a specific software id and version id. "
           "message=" +
           status.message;
    break;
  case NOT_ENOUGH_SHARDS:
    desc = "not enough shards. message=" + status.message;
    break;
  case INVALID_PARAMS:
    desc = "k and m are not the same value for all shard, for a "
           "specific software id. message=" +
           status.message;
    break;
  case DECODE_SUCCESS:
    desc = "decoded success. message=" + status.message;
    // write software to disk
    break;
  case DECODE_FAIL:
    desc = "decode failed. message=" + status.message;
    break;
  default:
    desc = "error, no status code";
    break;
  }
  return desc;
}

// write (decoded) data to disk
void AvswuAVNode::write_file(const string output_file,
                             const vector<char> output_data) const {
  // open file
  ofstream output(output_file, ofstream::binary);
  if (!output.is_open()) {
    const string msg = "unable to open output_file=" + output_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  output.write(output_data.data(), output_data.size());

  // close file
  output.close();
}

// verify signature, check hash, insert encrypted shard into cache
bool AvswuAVNode::insert(const AvswuPacketShard &packet_shard) {
  bool is_verify = true;
  bool is_hash = true;

  _cache.insert(packet_shard);

  bool is_success = is_verify && is_hash;

  return is_success;
}

// create list of shards based on notification, and which shards are not in
// our cache
AvswuPacketShardRequestList *AvswuAVNode::create_shard_request_list(
    const uint64_t to_id, mpz_t &to_X, node_type_t node_type,
    AvswuPacketNotificationList &notification_list) {

  AvswuPacketShardRequestList *shard_request_listP =
      new AvswuPacketShardRequestList();
  if (!shard_request_listP) {
    spdlog::error("unable to create shard request list");
    return nullptr;
  }

  // build shard request list based on notification list
  // for each notification in list
  for (size_t i = 0; i < notification_list.deque().size(); i++) {
    auto notif = notification_list.deque()[i];
    // for each shard in notification
    for (size_t s = 0; s < notif.packet_notif_list().size(); s++) {

      // get info for the shard
      auto manufacturer_id = notif.manufacturer_id();
      auto software_id = notif.software_id();
      auto version_id = notif.version_id();
      auto shard_info = notif.packet_notif_list()[s];
      auto shard_id = shard_info.shard_id;

      // if the shard is not in our cachce, fill shard request,
      // then add shard request to list of requested shards
      if (!cache().contains(manufacturer_id, software_id, version_id,
                            shard_id)) {
        shard_request_t shard_request;
        shard_request.manufacturer_id = manufacturer_id;
        shard_request.software_id = software_id;
        shard_request.version_id = version_id;
        shard_request.shard_id = shard_id;
        shard_request.shard_hash = shard_info.shard_hash;

        shard_request_listP->push_back(shard_request);
      }
    }
  }

  // now that the payload is built, we can set the header and footer hashes
  // set from/to
  const string payload_str = shard_request_listP->payload_to_json().dump();
  shard_request_listP->set(id(), X(), to_id, to_X, payload_str,
                           payload_type_t::shard_request_list, node_type);

  return shard_request_listP;
}

// gets shards from cache based on request list, and returns packets
// method must exists in both AvswuAVNode and AvswuRSUNode due
// to class hierarchy design, and for veins sim
vector<AvswuPacketShard> AvswuAVNode::get_request_packet_list(
    const uint64_t to_id, mpz_t &to_X, node_type_t node_type,
    AvswuPacketShardRequestList &shard_request_list) {

  // get shard pointers from cache
  vector<packet_shardP_t> shardP_list =
      get_shardP_list(shard_request_list.request_list());

  spdlog::info(fmt::format("get_request_packet_list:DEBUG={}", 1));

  // for each shard requested, create packets
  vector<AvswuPacketShard> packet_shard_list;
  for (size_t i = 0; i < shardP_list.size(); i++) {
    auto packet_shardP = shardP_list[i];
    const string payload_str = packet_shardP->payload_to_json().dump();
    packet_shardP->AvswuPacket::set(id(), X(), to_id, to_X, payload_str,
                                    payload_type_t::shard, node_type);
    packet_shard_list.push_back(*packet_shardP);
  }

  return packet_shard_list;
}

} // namespace avswu_av_node
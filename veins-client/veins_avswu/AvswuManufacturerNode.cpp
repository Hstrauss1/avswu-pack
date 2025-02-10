#include "AvswuManufacturerNode.h"
#include "AvswuOpenSSL.h"
#include <spdlog/spdlog.h>

#include "libipfs-linux.h"

#include <filesystem>
#include <fstream>
#include <string>

#ifdef __cplusplus
extern "C" {
#endif

#include "jerasure.h"
#include "liberation.h"
#include "reed_sol.h"
#include "timing.h"
#include <assert.h>
#include <errno.h>
#include <gf_rand.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#ifdef __cplusplus
}
#endif

namespace avswu_manufacturer_node {

// constructor
AvswuManufacturerNode::AvswuManufacturerNode(const uint32_t id,
                                             const size_t cache_max_size,
                                             AvswuGrpcClient *clientP)
    : AvswuNode(id, cache_max_size), _signer(), _aggregator(clientP) {}

// destructor
AvswuManufacturerNode::~AvswuManufacturerNode() {}

// erasure encode the file into k+m packets
const vector<AvswuPacketShard> AvswuManufacturerNode::create_packet_list(
    const uint64_t software_id, const uint64_t version_id,
    const vector<char> input_data, const uint32_t k, const uint32_t m) {

  vector<AvswuPacketShard> result;

  // encode data
  vector<AvswuPacketShard> packet_list;
  try {
    packet_list = encode(software_id, version_id, input_data, k, m);
  } catch (const std::runtime_error &e) {
    const string msg = "unable to encode input data";
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // for all packets, create pdu hash and signature for all packets
  for (size_t i = 0; i < packet_list.size(); i++) {
    // create hash for packet
    auto packetP = &packet_list[i];
    const bool is_hash_success = packetP->create_hash();
    if (!is_hash_success) {
      const string msg = "unable to create hash for packet";
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }

    // create signature of the hash of the pdu
    // the signer (manufacturer, RSU, or AV) signs the hash
    const bool is_signature_success =
        sign(packetP->footer().get_hash_signature(),
             packetP->footer().header_payload_hash(), _aggregator);

    if (!is_signature_success) {
      const string msg = "unable to create signature for packet";
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }

    // spdlog::debug("packet_list[{}]={}", i, json(packet_list[i]).dump());
  }

  return packet_list;
}

const vector<AvswuPacketShard> AvswuManufacturerNode::encode(
    const uint64_t software_id, const uint64_t version_id,
    const vector<char> input_data, const uint32_t k, const uint32_t m) {

  uint32_t w = 8;

  // calculate block size
  const size_t input_size = input_data.size();
  const size_t block_size = static_cast<size_t>(
      ceil(((double)input_size) / ((double)(k * w * sizeof(long)))) * w *
      sizeof(long));
  spdlog::debug("block_size={}", block_size);

  // create reed-solomon matrix
  int *matrix = reed_sol_vandermonde_coding_matrix(k, m, w);

  // create data and coding blocks
  char **data = malloc_erasure_blocks(k, block_size);
  char **coding = malloc_erasure_blocks(m, block_size);

  // copy file block data from input_data into data
  for (size_t i = 0; i < k - 1; i++) {
    memcpy(data[i], input_data.data() + (i * block_size), block_size);
  }

  // copy last of bytes
  const size_t i = k - 1;
  memcpy(data[i], input_data.data() + (i * block_size),
         input_size % block_size);

  // pad remaining bytes with 0's
  const size_t pad_size = block_size - (input_size % block_size);
  char *pad_start = data[i] + (input_size % block_size);
  memset(pad_start, '0', pad_size);

  // encode
  jerasure_matrix_encode(k, m, w, matrix, data, coding, block_size);

  // copy data blocks into packets
  vector<AvswuPacketShard> result;
  for (uint32_t i = 0; i < k; i++) {
    const uint64_t shard_id = current_time();
    const uint32_t erasure_id = i;
    AvswuPacketShard p(id(), X(), id(), X(), id(), software_id, version_id,
                       shard_id, erasure_id, k, m, w, data[i], block_size,
                       input_size);
    result.push_back(p);
  }

  // copy coding blocks into packets
  for (uint32_t i = 0; i < m; i++) {
    const uint64_t shard_id = current_time();
    const uint32_t erasure_id = i + k;
    AvswuPacketShard p(id(), X(), id(), X(), id(), software_id, version_id,
                       shard_id, erasure_id, k, m, w, coding[i], block_size,
                       input_size);
    result.push_back(p);
  }

  // free memory
  free(coding);
  free(data);

  return result;
};

// write a packet to ipfs, and return cid hash
const bool
AvswuManufacturerNode::ipfs_write_packet(string &server_result,
                                         const AvswuPacketShard &packet) const {

  // convert packet into json string
  string packet_json = json(packet).dump();

  // write packet to /tmp ramdisk
  auto t = current_time();
  const string tmp_file = "/tmp/avswu-tmp-" + to_string(t) + "-" + "shard-id-" +
                          to_string(packet.shard().shard_id()) + ".json";
  // spdlog::error("tmp_file={}", tmp_file);
  try {
    // open file
    ofstream output(tmp_file, ofstream::binary);
    if (!output.is_open()) {
      const string msg = "unable to open tmp_file=" + tmp_file;
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }

    // write json
    output.write(packet_json.c_str(), packet_json.size());

    // close file
    output.close();
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to write tmp_file={}", tmp_file);
    return false;
  }

  // write tmp_file to ipfs
  string ipfs_add_result;
  try {
    ipfs_add_result = IpfsAdd((char *)tmp_file.c_str());
  } catch (const std::runtime_error &err) {
    spdlog::error("unable to add packet json to ipfs, err={}", tmp_file,
                  err.what());
    return false;
  }

  // get cid from result
  string cid_hash;
  try {
    json j_result = json::parse(ipfs_add_result);
    if (j_result["Status"] != "ok") {
      spdlog::error("unable to add packet to ipfs, ipfs_add_result={}",
                    ipfs_add_result);
      return false;
    }
    cid_hash = j_result["Message"];
  } catch (const std::runtime_error &err) {
    spdlog::error("unable to add packet json to ipfs, err={}", tmp_file,
                  err.what());
    return false;
  }

  // send cid hash back
  server_result = cid_hash;

  // remove tmp file
  try {
    if (!std::filesystem::remove(tmp_file)) {
      spdlog::error("unable to remove tmp_file={}", tmp_file);
    }
  } catch (const std::filesystem::filesystem_error &err) {
    spdlog::error("unable to remove tmp_file={}, err={}", tmp_file, err.what());
  }

  return true;
}

// create notification
const bool AvswuManufacturerNode::set_notification(
    AvswuNotification &notif, vector<AvswuPacketShard> packet_list,
    vector<string> cid_hash_list, const uint64_t manufacturer_id,
    const uint64_t software_id, const uint64_t version_id,
    const vector<char> input_data, const uint32_t k, const uint32_t m,
    const double create_time) const {

  // check lengths are non-zero, and they match
  if (packet_list.size() == 0) {
    const string msg = "empty packet list";
    spdlog::error(msg);
    return false;
  }

  if (packet_list.size() != cid_hash_list.size()) {
    const string msg = "packet and cid list do not match";
    spdlog::error(msg);
    return false;
  }

  // set id
  notif.set(manufacturer_id, software_id, version_id, create_time);

  // clear/empty the notification packet list
  notif.clear();

  // create packet notification list, list of cid and pdu hashs for each packet
  for (size_t i = 0; i < packet_list.size(); i++) {
    const string cid_hash = cid_hash_list[i];

    // compute hash of payload

    // hash the string
    string shard_hash;
    string shard_str = json(packet_list[i].payload_to_json().dump());
    bool is_success = avswu_open_ssl::hash_by_type(
        shard_hash, shard_str, avswu_open_ssl::AVSWU_DATA_HASH_ALGORTITHM);
    if (!is_success) {
      spdlog::error("unable to create payload hash");
      return false;
    }

    const uint64_t shard_id = packet_list[i].shard().shard_id();
    notif.push_back(shard_id, cid_hash, shard_hash);
  }

  return true;
}

// write notification to blockchain
const bool AvswuManufacturerNode::blockchain_write_notification(
    const AvswuNotification &notif) const {
  // get blockchain and write notification
  auto blockchain = _aggregator.blockchain();
  const bool is_success = blockchain.write_notification(notif);
  return is_success;
}

} // namespace avswu_manufacturer_node
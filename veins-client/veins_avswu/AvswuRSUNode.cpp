#include "AvswuRSUNode.h"
#include <spdlog/spdlog.h>

#include "libipfs-linux.h"

#include <fstream>
#include <iostream>

namespace avswu_rsu_node {

// constructor
AvswuRSUNode::AvswuRSUNode(const uint32_t id, const size_t cache_max_size,
                           AvswuGrpcClient *clientP)
    : AvswuNode(id, cache_max_size), _signer(), _aggregator(clientP) {}

// destructor
AvswuRSUNode::~AvswuRSUNode() {}

// reads packet from ipfs
const bool AvswuRSUNode::ipfs_read_packet(AvswuPacketShard &packet,
                                          const string cid_hash) {

  // read json for packet from ipfs, write to /tmp ramdisk
  auto t = current_time();
  const string right_hash = cid_hash.substr(cid_hash.size() - 12, 12);
  const string tmp_file =
      "/tmp/avswu-tmp-" + to_string(t) + "-" + right_hash + ".json";
  // spdlog::error("tmp_file={}", tmp_file);
  try {
    // read from ipfs
    const string result =
        IpfsGet((char *)cid_hash.c_str(), (char *)tmp_file.c_str());
    json j_result = json::parse(result);
    if (j_result["Status"] != "ok") {
      spdlog::error("unable to read packet json from ipfs, result={}", result);
      return false;
    }
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to read packet json from ipfs");
    return false;
  }

  // read packet json from /tmp and parse into packet
  // open file
  ifstream input(tmp_file);
  if (!input.is_open()) {
    const string msg = "unable to open tmp_file=" + tmp_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // try to parse json into the packet
  try {
    json j_packet = json::parse(input);
    from_json(j_packet, packet);
  } catch (char *excp) {
    spdlog::error("unable to parse the json returned from ipfs,  excp={}",
                  excp);
    return false;
  }

  // close file
  input.close();

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

// RSU polls and reads blockchain to create notification_list for a specific
// software
const bool AvswuRSUNode::blockchain_read_last_notification(
    AvswuNotification &notif, const uint64_t manufacturer_id,
    const uint64_t software_id, const double start_time) const {

  // get blockchain
  auto blockchain = _aggregator.blockchain();

  // attempt to read notification max_attempt times
  bool is_success = false;
  uint32_t attempt = 0;
  while (!is_success && attempt < RSU_POLL_MAX_ATTEMPT) {
    // try to read
    is_success = blockchain.read_last_notification(notif, manufacturer_id,
                                                   software_id, start_time);
    if (!is_success) {
      // wait for notifications
      spdlog::warn("attempt {}/{} unable to find a new notification for "
                   "start_time={}, will retry in {} seconds...",
                   attempt + 1, RSU_POLL_MAX_ATTEMPT, start_time,
                   RSU_POLL_DELAY_SECONDS);
      usleep(RSU_POLL_DELAY_SECONDS * 1000000);
    }
    attempt += 1;
  }

  return is_success;
}

// RSU creates notification_list for each software from inventory
const AvswuPacketNotificationList
AvswuRSUNode::blockchain_read_notification_list(
    const uint64_t to_id, mpz_t &to_X,
    const AvswuPacketInventory &packet_inventory) {

  // create packet notification list from this RSU to a specific AV
  AvswuPacketNotificationList notification_list(id(), X(), to_id, to_X,
                                                node_type_t::rsu);

  // for each software in inventory, poll for a new notifications
  for (size_t i = 0; i < packet_inventory.software_list().size(); i++) {
    software_t sw = packet_inventory.software_list()[i];
    AvswuNotification notif;
    bool is_success = blockchain_read_last_notification(
        notif, sw.manufacturer_id, sw.software_id, sw.create_time);
    if (is_success) {
      notification_list.push_back(notif);
      spdlog::info("found a new notification for manufactuer_id={}, "
                   "software_id={}, create_time={}",
                   sw.manufacturer_id, sw.software_id, sw.create_time);
    } else {
      spdlog::error("unable to find a new notification for manufactuer_id={}, "
                    "software_id={}, create_time={}",
                    sw.manufacturer_id, sw.software_id, sw.create_time);
    }
  }

  return notification_list;
}

// RSU polls and reads blockchain to create notification_list based on
// delivery av request
const AvswuPacketNotificationList
AvswuRSUNode::blockchain_read_last_notification_list(
    const uint64_t to_id, mpz_t &to_X, const vector<uint64_t> manufacturer_list,
    const AvswuPacketDeliveryRequest &packet_delivery_request) {

  // create packet notification list from this RSU to a specific AV
  AvswuPacketNotificationList notification_list(id(), X(), to_id, to_X,
                                                node_type_t::rsu);

  // get blockchain
  auto blockchain = _aggregator.blockchain();

  const auto start_time = packet_delivery_request.start_time();
  const auto max_notification = packet_delivery_request.max_notification();

  // attempt to read notification max_attempt times
  bool is_success = false;
  uint32_t attempt = 0;
  while (!is_success && attempt < RSU_POLL_MAX_ATTEMPT) {
    is_success = blockchain.read_last_notification_list(
        notification_list.payload().notif_deque, manufacturer_list, start_time,
        max_notification);
    if (!is_success) {
      // wait for notifications
      spdlog::warn("attempt {}/{} unable to find a new notification for "
                   "start_time={}, will retry in {} seconds...",
                   attempt + 1, RSU_POLL_MAX_ATTEMPT, start_time,
                   RSU_POLL_DELAY_SECONDS);
      usleep(RSU_POLL_DELAY_SECONDS * 1000000);
    }
    attempt += 1;
  }

  return notification_list;
}

// rsu read packets of shards from ipfs based on notification list, saves
// shard to cache
const bool AvswuRSUNode::ipfs_read_shard_list(
    vector<AvswuPacketShard> &shard_list, const uint64_t to_id, mpz_t &to_X,
    const AvswuPacketNotificationList &notification_list) {

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

      // spdlog::debug("attempting to read manufacturer_id={}, "
      //               "software_id={}, version_id={}, shard_id={}",
      //               manufacturer_id, software_id, version_id, shard_id);

      // if shard is in cache, add to shard_list
      ShardMapKey sm_key(manufacturer_id, software_id, version_id, shard_id);
      if (_cache.contains(sm_key)) {
        // read shard from cache, create packet w/ shard, update to/from, add to
        // list
        // spdlog::debug("cache hit, found shard in cache");
        auto packet_shardP =
            _cache.get(manufacturer_id, software_id, version_id, shard_id);
        const string payload_str = packet_shardP->payload_to_json().dump();
        packet_shardP->AvswuPacket::set(id(), X(), to_id, to_X, payload_str,
                                        payload_type_t::shard,
                                        node_type_t::rsu);
        shard_list.push_back(*packet_shardP);
      } else {
        // if shard is not in cache, read shard/packet from ipfs, update packet
        // to/from, add to list, write shard to cache,
        // spdlog::debug("cache miss, reading shard from ipfs");
        // read a packet from ipfs
        auto packet_shardP = new AvswuPacketShard();
        bool is_success_read =
            ipfs_read_packet(*packet_shardP, shard_info.cid_hash);
        if (!is_success_read) {
          spdlog::error("unable to read shard with cid_hash={} from ipfs",
                        shard_info.cid_hash);
          return false;
        }

        // set the packets new from/to, add to list
        const string payload_str = packet_shardP->payload_to_json().dump();
        packet_shardP->AvswuPacket::set(id(), X(), to_id, to_X, payload_str,
                                        payload_type_t::shard,
                                        node_type_t::rsu);
        shard_list.push_back(*packet_shardP);

        // update cache
        _cache.insert(*packet_shardP);
      }
    }
  }

  return true;
}

// gets shards from cache based on request list, and returns packets
// method must exists in both AvswuAVNode and AvswuRSUNode due
// to class hierarchy design, and for veins sim
vector<AvswuPacketShard> AvswuRSUNode::get_request_packet_list(
    const uint64_t to_id, mpz_t &to_X,
    AvswuPacketShardRequestList &shard_request_list) {

  // get shard pointers from cache
  vector<packet_shardP_t> shardP_list =
      get_shardP_list(shard_request_list.request_list());

  // for each shard requested, create packets
  vector<AvswuPacketShard> shard_list;
  for (size_t i = 0; i < shardP_list.size(); i++) {
    auto packet_shardP = shardP_list[i];
    const string payload_str = packet_shardP->payload_to_json().dump();
    packet_shardP->AvswuPacket::set(id(), X(), to_id, to_X, payload_str,
                                    payload_type_t::shard, node_type_t::rsu);
    shard_list.push_back(*packet_shardP);
  }

  return shard_list;
}

} // namespace avswu_rsu_node
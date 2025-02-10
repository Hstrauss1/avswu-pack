#pragma once

#include "AvswuNode.h"
#include "AvswuNotification.h"
#include "AvswuPacketDeliveryRequest.h"
#include "AvswuPacketInventory.h"
#include "AvswuPacketNotificationList.h"
#include "AvswuPacketShard.h"
#include "AvswuPacketShardRequestList.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigSigner.h"
#include <gmp.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace non_con_musig_aggregator;
using namespace avswu_packet_shard;
using namespace avswu_packet_shard_request_list;
using namespace avswu_packet_notification_list;
using namespace avswu_packet_inventory;
using namespace avswu_node;
using namespace avswu_notification;
using namespace avswu_packet_delivery_request;

// rsu polling parameters
#define RSU_POLL_MAX_ATTEMPT 5
#define RSU_POLL_DELAY_SECONDS 0.1

namespace avswu_rsu_node {

class AvswuRSUNode : public AvswuNode {
private:
  // nc musig
  NonConMusigSigner _signer;
  NonConMusigAggregator _aggregator;

  // RSU polls and reads blockchain to create notification_list for a specific
  // software
  const bool blockchain_read_last_notification(AvswuNotification &notif,
                                               const uint64_t manufacturer_id,
                                               const uint64_t software_id,
                                               const double start_time) const;

  // reads packet from ipfs
  const bool ipfs_read_packet(AvswuPacketShard &packet, const string cid_hash);

  // insert shard within a packet to cache
  inline void cache_insert(const AvswuPacketShard &packet_shard) {
    _cache.insert(packet_shard);
  };

public:
  AvswuRSUNode(const uint32_t id, const size_t cache_max_size,
               AvswuGrpcClient *clientP);
  ~AvswuRSUNode();

  // for public/private key pair, we use the _signer's X (not _aggregator's X)
  inline mpz_t &X() { return _signer.X(); }

  // RSU creates notification_list for each software from inventory
  const AvswuPacketNotificationList blockchain_read_notification_list(
      const uint64_t to_id, mpz_t &to_X,
      const AvswuPacketInventory &packet_inventory);

  // RSU polls and reads blockchain to create notification_list based on
  // delivery av request
  const AvswuPacketNotificationList blockchain_read_last_notification_list(
      const uint64_t to_id, mpz_t &to_X,
      const vector<uint64_t> manufacturer_list,
      const AvswuPacketDeliveryRequest &packet_delivery_request);

  // rsu read packets of shards from ipfs based on notification list, saves
  // shard to cache
  const bool ipfs_read_shard_list(
      vector<AvswuPacketShard> &shard_list, const uint64_t to_id, mpz_t &to_X,
      const AvswuPacketNotificationList &packet_notification_list);

  // gets shards from cache based on request list, and returns packets
  // method must exists in both AvswuAVNode and AvswuRSUNode due
  // to class hierarchy design, and for veins sim
  vector<AvswuPacketShard>
  get_request_packet_list(const uint64_t to_id, mpz_t &to_X,
                          AvswuPacketShardRequestList &shard_request_list);
};

} // namespace avswu_rsu_node
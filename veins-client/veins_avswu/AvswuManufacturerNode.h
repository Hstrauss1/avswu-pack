#pragma once

#include "AvswuNode.h"
#include "AvswuNotification.h"
#include "AvswuPacketShard.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigSigner.h"
#include <gmp.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace non_con_musig_aggregator;
using namespace avswu_packet_shard;
using namespace avswu_node;
using namespace avswu_notification;

namespace avswu_manufacturer_node {

class AvswuManufacturerNode : public AvswuNode {
private:
  // nc musig
  NonConMusigSigner _signer;
  NonConMusigAggregator _aggregator;

  const vector<AvswuPacketShard> encode(const uint64_t software_id,
                                        const uint64_t version_id,
                                        const vector<char> input_data,
                                        const uint32_t k, const uint32_t m);

public:
  AvswuManufacturerNode(const uint32_t id, const size_t cache_max_size,
                        AvswuGrpcClient *clientP);
  ~AvswuManufacturerNode();

  // for public/private key pair, we use the _signer's X (not _aggregator's X)
  inline mpz_t &X() { return _signer.X(); }

  // erasure encode the file into k+m packets
  const vector<AvswuPacketShard>
  create_packet_list(const uint64_t software_id, const uint64_t version_id,
                     const vector<char> input_data, const uint32_t k,
                     const uint32_t m);

  // write a packet to ipfs, and return cid
  const bool ipfs_write_packet(string &serverResult,
                               const AvswuPacketShard &packet) const;

  // create notification
  const bool set_notification(AvswuNotification &notif,
                              const vector<AvswuPacketShard> packet_list,
                              const vector<string> cid_hash_list,
                              const uint64_t manufacturer_id,
                              const uint64_t software_id,
                              const uint64_t version_id,
                              const vector<char> input_data, const uint32_t k,
                              const uint32_t m, const double create_time) const;

  const bool
  blockchain_write_notification(const AvswuNotification &notif) const;
};

} // namespace avswu_manufacturer_node
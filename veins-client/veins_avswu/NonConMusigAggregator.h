#pragma once

#include "AvswuBlockchain.h"
#include "NonConMusigNode.h"
#include <gmp.h>

using namespace std;
using namespace avswu_utils;
using namespace non_con_musig_node;
using namespace avswu_blockchain;

namespace non_con_musig_aggregator {

class NonConMusigAggregator : public NonConMusigNode {
private:
  AvswuBlockchain _blockchain;
  uint64_t _w;
  mpz_t _W;

  void calc_w_and_W();

public:
  NonConMusigAggregator(AvswuGrpcClient *clientP);
  ~NonConMusigAggregator();

  // getters for public values
  inline uint64_t &w() { return _w; }
  inline mpz_t &W() { return _W; }

  // clientP from blockchain (b/c it's needed for ipfs RPC calls, et cetera)
  const inline AvswuGrpcClient *clientP() const {
    return _blockchain.clientP();
  }

  // blockchain (b/c it's needed by manufacturer to write/read notifications)
  const inline AvswuBlockchain &blockchain() const { return _blockchain; }

  // blockchain methods
  bool blockchain_write_commitment(uint64_t shard_id, uint64_t signer_id,
                                   mpz_t R, bool write_bc);
  bool blockchain_read_commitment_vector(vector<commitment_t> &L_com,
                                         vector<commitment_map_key_t> key_vec);
};

} // namespace non_con_musig_aggregator
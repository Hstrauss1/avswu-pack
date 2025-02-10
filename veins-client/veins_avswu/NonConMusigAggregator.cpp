#include "NonConMusigAggregator.h"
#include "AvswuOpenSSL.h"
#include <spdlog/spdlog.h>

using namespace non_con_musig_node;

namespace non_con_musig_aggregator {

// constructor
NonConMusigAggregator::NonConMusigAggregator(AvswuGrpcClient *clientP)
    : NonConMusigNode(), _blockchain(clientP) {
  // spdlog::debug("blockchain={:s}", json(_blockchain).dump());

  // init w and W
  _w = 0;
  mpz_init(_W);
}

// destructor
NonConMusigAggregator::~NonConMusigAggregator() {}

// write commitment
bool NonConMusigAggregator::blockchain_write_commitment(uint64_t shard_id,
                                                        uint64_t signer_id,
                                                        mpz_t R,
                                                        bool write_bc) {

  bool is_success = false;

  // 1. calculate w and W
  calc_w_and_W();

  // 2. write { R, w, W } to blockchain
  if (write_bc) {
    // write the key (signer_id,signer_id) -> commitment value (R,w,W)
    is_success = _blockchain.write_commitment(shard_id, signer_id, R, _w, _W);
  } else {
    is_success = true;
  }

  return is_success;
}

// read commitment vector
bool NonConMusigAggregator::blockchain_read_commitment_vector(
    vector<commitment_t> &L_com, vector<commitment_map_key_t> key_vec) {

  bool is_success = is_success =
      _blockchain.read_commitment_vector(L_com, key_vec);

  return is_success;
}

// calculate w and W, w=H(t) and computes W=g^w
void NonConMusigAggregator::calc_w_and_W() {

  // 1. get the current time
  uint64_t t = current_time();

  // 2. create a hash based on t to use for pbc
  string str_t = std::to_string(t);
  string hash;
  const bool is_success = avswu_open_ssl::hash_by_type(
      hash, str_t, avswu_open_ssl::AVSWU_SIGNATURE_HASH_ALGORTITHM);
  if (!is_success) {
    spdlog::error("sha hash generation failed");
  }

  // 3. calculate w=H(t)
  std::memcpy(&_w, hash.c_str(), sizeof(uint64_t));
  _w = _w % PARAM_P;

  // 4. calculate W=g^w
  mpz_powm_ui(_W, _par.g, _w, _par.p);
}

} // namespace non_con_musig_aggregator
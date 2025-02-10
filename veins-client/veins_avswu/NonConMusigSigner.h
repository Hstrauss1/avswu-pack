#pragma once

#include "NonConMusigAggregator.h"
#include "NonConMusigNode.h"
#include <vector>

using namespace std;
using namespace avswu_utils;
using namespace non_con_musig_node;
using namespace non_con_musig_aggregator;

namespace non_con_musig_signer {

class NonConMusigSigner : public NonConMusigNode {
private:
  // non-concurrent musig parameters
  uint64_t _r;
  mpz_t _R;
  mpz_t _aggregate_R;
  mpz_t _s_partial;
  mpz_t _s_joint;

  // private methods

  // private non-con musig methods
  void initialize();
  void hash2(mpz_t &result, mpz_t X, mpz_t aggregate_R, const string message);
  bool verify_signature_R(vector<commitment_t> L_com, const size_t i_end,
                          mpz_t signature_R);
  bool verify_signature_s(vector<mpz_t *> L_X_ptr, vector<commitment_t> L_com,
                          string message, const size_t i_end, mpz_t signature_R,
                          mpz_t signature_s);
  bool verify_signature_s2(vector<mpz_t *> L_X_ptr, vector<commitment_t> L_com,
                           string message, mpz_t signature_R,
                           mpz_t signature_s);

public:
  NonConMusigSigner();
  ~NonConMusigSigner();

  // getters for public values
  inline mpz_t &R() { return _R; }
  inline mpz_t &aggregate_R() { return _aggregate_R; }
  inline mpz_t &s_partial() { return _s_partial; }
  inline mpz_t &s_joint() { return _s_joint; }

  // non-con-musig methods
  void calc_R();
  bool blockchain_write_commitment(const uint64_t shard_id,
                                   const uint64_t signer_id,
                                   NonConMusigAggregator &rsu_aggregator);
  bool blockchain_write_commitment(const uint64_t shard_id,
                                   const uint64_t signer_id,
                                   NonConMusigAggregator &rsu_aggregator,
                                   bool write_bc);

  // various ways to call aggregation algorithm
  void calc_aggregate_R(mpz_t &result, vector<commitment_t> L_com,
                        const size_t i_end);
  inline void calc_aggregate_R(vector<commitment_t> L_com) {
    calc_aggregate_R(_aggregate_R, L_com, L_com.size());
  }
  inline void calc_aggregate_R(vector<commitment_t> L_com, const size_t i_end) {
    calc_aggregate_R(_aggregate_R, L_com, i_end);
  }

  // partial-signature and joint-signature
  void calc_s_partial(const string message, uint64_t w);
  void calc_s_joint(mpz_t s_joint_previous);
  void set_s_joint_to_s_partial();

  // verification
  bool verify(vector<mpz_t *> L_X_ptr, vector<commitment_t> L_com,
              string message, const size_t i_end, mpz_t signature_R,
              mpz_t signature_s);
};

} // namespace non_con_musig_signer
#include "NonConMusigSigner.h"
#include "AvswuOpenSSL.h"
#include <spdlog/spdlog.h>

using namespace non_con_musig_node;
using namespace non_con_musig_aggregator;
using namespace avswu_open_ssl;

namespace non_con_musig_signer {

// constructors
NonConMusigSigner::NonConMusigSigner() { initialize(); }

// destructor
NonConMusigSigner::~NonConMusigSigner() {}

// initialize
void NonConMusigSigner::initialize() {
  _r = 0;
  mpz_init(_R);
  mpz_init(_aggregate_R);
  mpz_init(_s_partial);
  mpz_init(_s_joint);
}

// calculate R = g^r
void NonConMusigSigner::calc_R() {
  // 1. pick a random r value
  _r = rand() % PARAM_P;
  // 2. calculate R = g^r
  mpz_powm_ui(_R, _par.g, _r, _par.p);
}

// send R to aggregator
bool NonConMusigSigner::blockchain_write_commitment(
    const uint64_t shard_id, const uint64_t signer_id,
    NonConMusigAggregator &rsu_aggregator, bool write_bc) {

  // send R to aggregator
  bool is_success = rsu_aggregator.blockchain_write_commitment(
      shard_id, signer_id, _R, write_bc);

  if (!is_success) {
    spdlog::error("Unable to send R to aggregator, rsu receiving failed");
  }

  return is_success;
}

// send R to aggregator and write to blockchain
bool NonConMusigSigner::blockchain_write_commitment(
    const uint64_t shard_id, const uint64_t signer_id,
    NonConMusigAggregator &rsu_aggregator) {
  return blockchain_write_commitment(shard_id, signer_id, rsu_aggregator, true);
}

// calculate aggregated commitment
void NonConMusigSigner::calc_aggregate_R(mpz_t &result,
                                         vector<commitment_t> L_com,
                                         const size_t i_end) {
  // check args
  if (L_com.size() == 0) {
    spdlog::critical("commitment vector is empty");
    return;
  }
  if (i_end + 1 > L_com.size()) {
    spdlog::critical("commitment vector does not have enough elements to "
                     "calculate aggregate R");
    return;
  }

  // calc for i = 0, Ragg = W0*R0
  const size_t i = 0;
  mpz_mul(result, L_com[i].W, L_com[i].R);
  mpz_mod(result, result, _par.p);

  // calculate remaining products, j=1 ... j=i
  mpz_t tempWR;
  mpz_init(tempWR);
  for (size_t i = 1; i < i_end + 1; i++) {
    // tempWR = W*R
    mpz_mul(tempWR, L_com[i].W, L_com[i].R);
    // aggregate_R = aggregate_R * tempWR
    mpz_mul(result, result, tempWR);
    mpz_mod(result, result, _par.p);
  }

  // free temp elements
  mpz_clear(tempWR);
}

// H2(X, R12, m) = H(X || R12 || m), 3 input hash2 function
void NonConMusigSigner::hash2(mpz_t &result, mpz_t X, mpz_t aggregate_R,
                              const string message) {

  // concat X, aggregate_R, and message
  string X_str = mpz_get_str(nullptr, 16, X);
  string aggregate_R_str = mpz_get_str(nullptr, 16, aggregate_R);
  string X_R_message = X_str + aggregate_R_str + message;

  // hash m
  string hash;
  const bool is_success = avswu_open_ssl::hash_by_type(
      hash, X_R_message, avswu_open_ssl::AVSWU_SIGNATURE_HASH_ALGORTITHM);
  if (!is_success) {
    spdlog::error("sha hash generation failed");
  }

  // convert hash into a gmp int
  mpz_set_str(result, hash.c_str(), 16);
  mpz_mod(result, result, _par.p);
}

// calculate the partial signature
// s = s=w+r+c*sk (mod p) where c = H(X, R_aggregate, m)
// s.t. H2(X, R12, m) = H(X)*H(aggregate_R)*H(m)
void NonConMusigSigner::calc_s_partial(const string message, uint64_t w) {

  // we calculate s = s=w+r+c*sk
  // 1. s = c, where c = H2(pk,aggregate_R,message)
  mpz_t c;
  mpz_init(c);
  hash2(c, _key_pair.X, _aggregate_R, message);
  // 2. s = c*sk
  mpz_mul_ui(_s_partial, c, _key_pair.x);
  // 3. s += r
  mpz_add_ui(_s_partial, _s_partial, _r);
  // 4. s += w
  mpz_add_ui(_s_partial, _s_partial, w);
}

// s_joint_i = s_partial + s_joint_previous
void NonConMusigSigner::calc_s_joint(mpz_t s_joint_previous) {
  mpz_add(_s_joint, _s_partial, s_joint_previous);
}

// s_joint = s_partial
void NonConMusigSigner::set_s_joint_to_s_partial() {
  mpz_set(_s_joint, _s_partial);
};

// verify signature_R == product of (W_i*R_i)
bool NonConMusigSigner::verify_signature_R(vector<commitment_t> L_com,
                                           const size_t i_end,
                                           mpz_t signature_R) {
  // calculate aggregate R
  mpz_t tmp_aggregate_R;
  mpz_init(tmp_aggregate_R);
  calc_aggregate_R(tmp_aggregate_R, L_com, i_end);

  // compare R elements
  bool verify_R = (mpz_cmp(signature_R, tmp_aggregate_R) == 0);

  mpz_clear(tmp_aggregate_R);

  return verify_R;
}

// since we have verified signature_R == product of (W_i*R_i), now
// we verify g^signature_s == signature_R* product of (X_i^c_i)
bool NonConMusigSigner::verify_signature_s(vector<mpz_t *> L_X_ptr,
                                           vector<commitment_t> L_com,
                                           string message, const size_t i_end,
                                           mpz_t signature_R,
                                           mpz_t signature_s) {
  /*
   * calculate left side of question = g^s
   */
  mpz_t tmp_left;
  mpz_init(tmp_left);
  // tmp_left = static_cast<uint64_t>(pow(_par.g, signature_s));
  mpz_powm(tmp_left, _par.g, signature_s, _par.p);

  /*
   * calculate right side of question = R * product (X_i & c_i)
   *    where c_i = H2(X_i, R_aggregate_i, i, m)
   */

  // calculate product of (X_i^c_i)

  // create temp elements
  mpz_t tmp_c_i;
  mpz_init(tmp_c_i);
  mpz_t tmp_R_agg_i;
  mpz_init(tmp_R_agg_i);
  mpz_t tmp_product;
  mpz_init(tmp_product);
  mpz_t tmp_X_i_pow_c_i;
  mpz_init(tmp_X_i_pow_c_i);

  // calc product for for i = 0
  const size_t i = 0;
  // calculate X_i^c_i, where c_i = H2(X_i,R_i-1,i,m)
  calc_aggregate_R(tmp_R_agg_i, L_com, i);
  hash2(tmp_c_i, *(L_X_ptr[i]), tmp_R_agg_i, message);
  // calculate product(X_i^c_i)
  mpz_powm(tmp_product, *(L_X_ptr[i]), tmp_c_i, _par.p);

  // calc product for i = 1 to n
  for (size_t i = 1; i < i_end + 1; i++) {
    // calculate X_i^c_i, where c_i = H2(X_i,R_i-1,i,m)
    calc_aggregate_R(tmp_R_agg_i, L_com, i);
    hash2(tmp_c_i, *(L_X_ptr[i]), tmp_R_agg_i, message);
    mpz_powm(tmp_X_i_pow_c_i, *(L_X_ptr[i]), tmp_c_i, _par.p);

    // calculate product *= X_i^c_i
    mpz_mul(tmp_product, tmp_product, tmp_X_i_pow_c_i);
  }

  // calculate R * product(X_i^c_i)
  mpz_t tmp_right;
  mpz_init(tmp_right);
  mpz_mul(tmp_right, signature_R, tmp_product);

  // compare g^s == R * product of (X_i^c_i)
  mpz_mod(tmp_left, tmp_left, _par.p);
  mpz_mod(tmp_right, tmp_right, _par.p);

  bool verify_s = (mpz_cmp(tmp_left, tmp_right) == 0);

  // clear temp elements
  mpz_clear(tmp_right);
  mpz_clear(tmp_left);
  mpz_clear(tmp_c_i);
  mpz_clear(tmp_R_agg_i);
  mpz_clear(tmp_product);
  mpz_clear(tmp_X_i_pow_c_i);

  return verify_s;
}

// verify (signature_R, signature_s)
bool NonConMusigSigner::verify(vector<mpz_t *> L_X_ptr,
                               vector<commitment_t> L_com, string message,
                               const size_t i_end, mpz_t signature_R,
                               mpz_t signature_s) {

  // 1. verify signature_R == aggregateR
  bool verify_R = verify_signature_R(L_com, i_end, signature_R);

  if (verify_R) {
    spdlog::info("signature_R is verified");
  } else {
    spdlog::error("signature_R is NOT verified");
  }

  // if verify_signature_R failed, return false. this constitutes a lazy-and
  // operation operation)
  if (!verify_R) {
    return false;
  }

  bool verify_s = verify_signature_s(L_X_ptr, L_com, message, i_end,
                                     signature_R, signature_s);

  if (verify_s) {
    spdlog::info("signature_s is verified");
  } else {
    spdlog::error("signature_s is NOT verified");
  }

  return verify_s;
}

} // namespace non_con_musig_signer
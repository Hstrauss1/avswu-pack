/*
 * non-concurrent musig for n signers
 */
#include "AvswuGrpcClient.h"
#include "AvswuUtilsVector.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigSigner.h"

#include <spdlog/spdlog.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace non_con_musig_aggregator;
using namespace avswu_grpc_client;
using namespace avswu_utils_vector;
using json = nlohmann::json;

void non_con_musig_test(AvswuGrpcClient *clientP) {

  string message = "the quick brown fox";

  const uint32_t n = 10;
  // av_signer needs to be an array (not a vector) due to pbc elements
  NonConMusigSigner av_signer[n];

  for (size_t i = 0; i < n; i += 1) {
    spdlog::debug("av_signer[{}].X()={}", i, mpz_to_string(av_signer[i].X()));
  }

  // rsu
  NonConMusigAggregator rsu_aggregator(clientP);

  // id for a sw shard (get a new shard id based on time, so old values are not
  // read from blockchain)
  // commitment vector is based on map key of (id,shard_id)
  uint64_t shard_id = 13 * (current_time() % PARAM_P);

  // create key vectors
  vector<commitment_map_key_t> key_vec;
  // vector of commitments
  vector<commitment_t> L_com;

  for (size_t i = 0; i < n; i += 1) {

    /*
     * sign round 1
     */
    spdlog::debug("**********************************************");
    spdlog::debug("sign round 1 for av_signer i={}", i);
    spdlog::debug("**********************************************");

    // signer_i calculates R=g^r
    av_signer[i].calc_R();
    spdlog::debug("av_signer[{}].R()={}", i, mpz_to_string(av_signer[i].R()));

    // signer sends R to smart contract, smart contract calculates {w,W}, and
    // writes commitments {R, w, W} to blockchain
    bool is_success =
        av_signer[i].blockchain_write_commitment(shard_id, i, rsu_aggregator);
    if (!is_success) {
      spdlog::error("Unable to sucessfully send shard for signer[{}] shard={} "
                    "to rsu_aggregator",
                    i, shard_id);
    }

    /*
     * sign round 2
     */
    spdlog::debug("**********************************************");
    spdlog::debug("sign round 2 for av_signer i={}", i);
    spdlog::debug("**********************************************");

    // rsu aggregator reads commitment vector from smart contract

    // create keys, push to key_vec
    commitment_map_key_t key;
    set_commitment_map_key(key, shard_id, i);
    key_vec.push_back(key);

    // read the vector
    clear_commitment_vector(L_com);
    const bool read_com_success =
        rsu_aggregator.blockchain_read_commitment_vector(L_com, key_vec);
    if (!read_com_success) {
      spdlog::error("unable to read commitment vector");
      exit(-1);
    }

    // av_i calculates the aggregated commitment R_i-1,i
    av_signer[i].calc_aggregate_R(L_com, i);
    spdlog::debug("av_signer[{}].aggregate_R()={}", i,
                  mpz_to_string(av_signer[i].aggregate_R()));

    // av_i calculates partial signature s_i
    av_signer[i].calc_s_partial(message, L_com[i].w);
    spdlog::debug("av_signer[{}].s_partial()={}", i,
                  mpz_to_string(av_signer[i].s_partial()));

    // calculate s_joint, if i==0, there is no previous, so s_joint_0 =
    // s_partial_0
    if (i == 0) {
      av_signer[i].set_s_joint_to_s_partial();
    } else {
      av_signer[i].calc_s_joint(av_signer[i - 1].s_joint());
    }
    spdlog::debug("av_signer[{}].s_joint()={}", i,
                  mpz_to_string(av_signer[i].s_joint()));
  }

  /*
   * verify
   */
  spdlog::debug("**********************************************");
  spdlog::debug("verify");
  spdlog::debug("**********************************************");

  // set verifier index, the last av is the verifier
  const size_t i = n - 1;
  vector<mpz_t *> L_X_ptr;

  // set input vectors for verify method
  set_L_X(L_X_ptr, av_signer, n);

  spdlog_vector("L_com", L_com);
  spdlog_vector("L_X_ptr", L_X_ptr);

  if (av_signer[i].verify(L_X_ptr, L_com, message, i,
                          av_signer[i].aggregate_R(), av_signer[i].s_joint())) {
    spdlog::info("signature is verified");
  } else {
    spdlog::error("signature is NOT verified");
  }
}

int main(int argc, char **argv) {

  // verbose logging
  spdlog::set_level(spdlog::level::trace);

  AvswuGrpcClient client(argc, argv);

  // run ncmusig test
  non_con_musig_test(&client);

  return 0;
}
#include "AvswuGrpcClient.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigSigner.h"

#include <spdlog/spdlog.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace non_con_musig_aggregator;
using namespace avswu_grpc_client;
using json = nlohmann::json;

void non_con_musig_test(AvswuGrpcClient *clientP) {

  string message = "the quick brown fox";

  NonConMusigSigner av_signer;
  // rsu
  NonConMusigAggregator rsu_aggregator(clientP);

  // id for a sw shard (get a new shard id based on time, so old values are not
  // read from blockchain)
  // commitment vector is based on map key of (id,shard_id)
  uint64_t shard_id = 13 * (current_time() % PARAM_P);

  /*
   * sign round 1
   */
  uint64_t id = 1;
  spdlog::debug("**********************************************");
  spdlog::debug("sign round 1 for av_signer id={}", id);
  spdlog::debug("**********************************************");

  // signer_i calculates R=g^r
  av_signer.calc_R();
  spdlog::debug("av_signer.R()={}", mpz_to_string(av_signer.R()));

  // signer sends R to smart contract, smart contract calculates {w2,W2}, and
  // writes commitments {R, w, W} to blockchain
  bool is_success =
      av_signer.blockchain_write_commitment(shard_id, id, rsu_aggregator, true);
  if (!is_success) {
    spdlog::error("Unable to sucessfully send shard for signer.id={} shard={} "
                  "to rsu_aggregator",
                  id, shard_id);
  }

  /*
   * sign round 2
   */
  spdlog::debug("**********************************************");
  spdlog::debug("sign round 2 for av_signer id={}", id);
  spdlog::debug("**********************************************");

  // push key to vec
  commitment_map_key_t key;
  set_commitment_map_key(key, shard_id, id);
  vector<commitment_map_key_t> key_vec;
  key_vec.push_back(key);

  vector<commitment_t> L_com;
  clear_commitment_vector(L_com);

  rsu_aggregator.read_commitment_vector(L_com, key_vec);
  spdlog_vector("L_com", L_com);

  // check if what written same as what was read
  if (mpz_cmp(av_signer.R(), L_com[0].R) == 0) {
    spdlog::info("R is same");
  } else {
    spdlog::error("R is NOT same");
  }
  if (rsu_aggregator.w(), L_com[0].w) {
    spdlog::info("w is same");
  } else {
    spdlog::error("w is NOT same");
  }
  if (mpz_cmp(rsu_aggregator.W(), L_com[0].W) == 0) {
    spdlog::info("W is same");
  } else {
    spdlog::error("W is NOT same");
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
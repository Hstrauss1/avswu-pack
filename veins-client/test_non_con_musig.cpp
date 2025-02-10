/*
 * non-concurrent musig for n signers
 */
#include "AvswuGrpcClient.h"
#include "AvswuUtilsVector.h"
#include "NonConMusigAggregator.h"
#include "NonConMusigSigner.h"

#include <getopt.h>
#include <spdlog/spdlog.h>

using namespace std;
using namespace non_con_musig_signer;
using namespace non_con_musig_aggregator;
using namespace avswu_grpc_client;
using namespace avswu_utils_vector;
using json = nlohmann::json;

void non_con_musig_test(AvswuGrpcClient *clientP, size_t n) {

  // seed the random number generator
  srand(time(NULL));

  // time start
  uint64_t time_start = current_time();

  string message = "the quick brown fox";

  // av signers, generate param, and keys for all avs is done in constructor
  NonConMusigSigner signer[n];

  for (size_t i = 0; i < n; i += 1) {
    spdlog::debug("signer[{}].X()={}", i, mpz_to_string(signer[i].X()));
  }

  // rsu, aggregator
  NonConMusigAggregator aggregator(clientP);

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
    spdlog::debug("sign round 1 for signer i={}", i);
    spdlog::debug("**********************************************");

    // signer_i calculates R=g^r
    signer[i].calc_R();
    spdlog::debug("signer[{}].R()={}", i, mpz_to_string(signer[i].R()));

    // signer sends R to smart contract, smart contract calculates {w,W}, and
    // writes commitments {R, w, W} to blockchain
    bool is_success =
        signer[i].blockchain_write_commitment(shard_id, i, aggregator, false);
    if (!is_success) {
      spdlog::error("Unable to sucessfully send shard for signer[{}] shard={} "
                    "to aggregator",
                    i, shard_id);
    }

    /*
     * build the L_com vector by hand since we
     * do not read/write it to/from the blockchain in our ncmusig tests
     */
    commitment_t c;
    mpz_init(c.R);
    mpz_set(c.R, signer[i].R());
    c.w = aggregator.w();
    mpz_init(c.W);
    mpz_set(c.W, aggregator.W());

    L_com.push_back(c);

    /*
     * sign round 2
     */
    spdlog::debug("**********************************************");
    spdlog::debug("sign round 2 for signer i={}", i);
    spdlog::debug("**********************************************");

    // av_i calculates the aggregated commitment R_i-1,i
    signer[i].calc_aggregate_R(L_com, i);
    spdlog::debug("signer[{}].aggregate_R()={}", i,
                  mpz_to_string(signer[i].aggregate_R()));

    // av_i calculates partial signature s_i
    signer[i].calc_s_partial(message, L_com[i].w);
    spdlog::debug("signer[{}].s_partial()={}", i,
                  mpz_to_string(signer[i].s_partial()));

    // calculate s_joint, if i==0, there is no previous, so s_joint_0 =
    // s_partial_0
    if (i == 0) {
      signer[i].set_s_joint_to_s_partial();
    } else {
      signer[i].calc_s_joint(signer[i - 1].s_joint());
    }
    spdlog::debug("signer[{}].s_joint()={}", i,
                  mpz_to_string(signer[i].s_joint()));
  }
  spdlog::info("completed round 1 and round 2 for n={} signers", n);

  spdlog_vector("L_com", L_com);

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
  set_L_X(L_X_ptr, signer, n);

  // spdlog_vector("L_com", L_com);
  // spdlog_vector("L_X_ptr", L_X_ptr);

  if (signer[i].verify(L_X_ptr, L_com, message, i, signer[i].aggregate_R(),
                       signer[i].s_joint())) {
    spdlog::info("signature is verified for n={} signers", n);
  } else {
    spdlog::error("signature is NOT verified for n={} signers", n);
  }

  uint64_t time_end = current_time();
  auto cpu_time = time_end - time_start;
  spdlog::info("n={} cpu_time={} ns", n, cpu_time);
  // output csv data
  cout << n << "," << cpu_time << endl;
}

uint32_t parse_args(int argc, char **argv) {
  // parse command line arguments using gnu's getopt

  // n has a parameter "n:" and v does not have a parameter, => "n:v"
  const char *short_options = "n:v";
  option long_options[] = {{"number", required_argument, NULL, 'n'},
                           {"verbose", no_argument, NULL, 'v'},
                           {NULL, 0u, NULL, 0u}};
  int option_index = 0;

  // set defaults
  uint32_t n = 10;

  int opt;
  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'n':
      n = stoi(optarg);
      spdlog::debug("n={}", n);
      break;
    case 'v':
      spdlog::set_level(spdlog::level::trace);
      spdlog::debug("verbose logging is on");
      break;
    case '?':
      spdlog::error("command line error. one or more flags not recognized: {}",
                    char(opt));
      break;
    default:
      spdlog::warn("usage: prog --number number_of_signers --verbose");
      exit(-1);
    }
  }

  // process non-option arguments
  // for (size_t i = optind; i < argc; i++) {
  //   spdlog::debug("Non-option argument argv[{}]={}", i, argv[i]);
  // }

  return n;
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::info);

  uint32_t n = parse_args(argc, argv);

  // argc and argv are handle by getopt, pass argc=1 only program name, and argv
  AvswuGrpcClient client(1u, argv);

  // run ncmusig test
  non_con_musig_test(&client, n);

  return 0;
}
#include "AvswuVeinsUtils.h"

#include <spdlog/spdlog.h>

using namespace std;
using namespace avswu_config;
using namespace avswu_grpc_client;

namespace avswu_veins_utils {

/*
 * one global config (from config.json) shared across all nodes
 */
AvswuConfig _avswu_config;
// has config.json been read
bool _read_avswu_config = false;
// read config.json
const bool read_avswu_config() {
  const string config_file =
      string(AVSWU_BASEDIR) + "/veins-client/inventory/config.json";
  try {
    _avswu_config.read_file(config_file);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
    return false;
  }

  _read_avswu_config = true;

  return true;
}

/*
 * one global gRPC client connection
 */
// the client pointer
AvswuGrpcClient *_grpc_clientP;

// create gRPC client
bool _create_grpc_client = false;
// create the client, if we have not yet
const bool create_grpc_client() {
  try {
    // argc and argv, pass argc=1 only program name, in argv
    const char *prog_name = "veins_simulation";
    _grpc_clientP = new AvswuGrpcClient(1u, (char **)&prog_name);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
    return false;
  }

  // if not created, return false
  if (!_grpc_clientP) {
    return false;
  }

  _create_grpc_client = true;

  return true;
}

// getters/setters for global variables in .cc file
const bool get_read_avswu_config() { return _read_avswu_config; }

void set_read_avswu_config(const bool val) { _read_avswu_config = val; }
AvswuConfig *get_avswu_configP() { return &_avswu_config; }
const bool get_create_grpc_client() { return _create_grpc_client; }

AvswuGrpcClient *get_grpc_clientP() { return _grpc_clientP; }

// compare ids,return true if a_id==b_id a_X==b_X
const bool compare_id(uint64_t a_id, mpz_t &a_X, uint64_t b_id, mpz_t &b_X) {
  // check if id's match
  if (a_id != b_id) {
    return false;
  }
  // check if public keys match
  if (mpz_cmp(a_X, b_X) != 0) {
    return false;
  }

  return true;
}

/*
 * one ipfs connection
 */
bool _ipfs_start = false;
void set_ipfs_start(const bool val) { _ipfs_start = val; }
const bool ipfs_start() { return _ipfs_start; }

} // namespace avswu_veins_utils
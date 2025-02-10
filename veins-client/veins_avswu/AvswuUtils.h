#pragma once

#include <gmp.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>
#include <unistd.h>

using namespace std;

/*
 * configuration variables
 */

/*
send grpc calls to avswu veins-server, using docker run mapped port for grpc

server_ip:server_port
<server_ip> is from server. use windows ipconfig
  Locate a section labeled "Ethernet adapter Ethernet" Find the "IPv4 Address"
<server_port> is from docker -publish port mapping. also listed in docker
desktop.
*/

/* ubuntu-laptop ipv4 address
look for "inet" address in output of ifconfig
> ifconfig
*/

// set ip address of grpc server -- if in instant-veins virtual box, or avswu1
// container

/*
 * configs for instant-veins virtual box
 */
// _HOSTNAME_INSTANT_VEINS defined by -D compile variable in
// CMakeLists.txt/Makefile
#ifdef _HOSTNAME_INSTANT_VEINS
#define AVSWU_BASEDIR "/home/gsolomon/avswu"
/*
 * ubuntu-laptop ip address
 */
#define AVSWU_GRPC_SERVER_IP "192.168.50.112"
#define AVSWU_GRPC_SERVER_PORT "50051"
/*
 * docker container, ipv4 from windows ipconfig (see detailed comment above)
 * 58851 is the published docker port for gRPC from docker run command
 */
// #define AVSWU_GRPC_SERVER_IP "192.168.56.1"
// #define AVSWU_GRPC_SERVER_PORT "58851"
#else
/*
 * configs for avswu container
 */
#define AVSWU_BASEDIR "/home/gsolomon/avswu"
// ipv4 loopback to gRPC port w/in this container
#define AVSWU_GRPC_SERVER_IP "127.0.0.1"
#define AVSWU_GRPC_SERVER_PORT "50051"
#endif

/*
 * public params
 */

// #define PARAM_G 3
// #define PARAM_G 37
#define PARAM_G 89

// #define PARAM_P 13
// #define PARAM_P 5003
#define PARAM_P 7409543

// hash algorithms
#define AVSWU_DATA_HASH_ALGORTITHM sha512
#define AVSWU_SIGNATURE_HASH_ALGORTITHM sha512

// encoding params
#define AVSWU_ENCODE_K 6
#define AVSWU_ENCODE_M 3
// #define AVSWU_ENCODE_K 30
// #define AVSWU_ENCODE_M 15

// node cache sizes
/*
NOTE** these sies must be large enough to hold
several software updates
*/
#define MiB 1024 * 1024
#define AVSWU_CLIENT_AV_MAX_CACHE_SIZE 32 * MiB
#define AVSWU_DELIVERY_AV_MAX_CACHE_SIZE 128 * MiB
#define AVSWU_RSU_MAX_CACHE_SIZE 256 * MiB
#define AVSWU_MANUFACTURER_MAX_CACHE_SIZE 512 * MiB

// for converting linux time seconds
#define SECOND_TO_NANOSECOND 1000000000

using namespace std;
using json = nlohmann::json;

/* utils structs */
namespace avswu_utils {

// initial params
typedef struct {
  mpz_t g;
  mpz_t p;
} param_t;

// key pair ( x = sk secret key, X = pk public key,)
typedef struct {
  mpz_t X;
  uint64_t x;
} key_pair_t;

// commitment
typedef struct {
  mpz_t R;
  uint64_t w;
  mpz_t W;
} commitment_t;

// commitment map key
typedef struct {
  uint64_t shard_id;
  uint64_t signer_id;
} commitment_map_key_t;

// request structure for shard cache
typedef struct {
  uint64_t manufacturer_id;
  uint64_t software_id;
  uint64_t version_id;
  uint64_t shard_id;
  string shard_hash;
} shard_request_t;

// commitment methods
void clear_commitment_vector(vector<commitment_t> &v);
void set_commitment_map_key(commitment_map_key_t &key, uint64_t shard_id,
                            uint64_t signer_id);

// spdlog and printing utilities
void spdlog_vector(string prefix, vector<uint64_t> v);
void spdlog_vector(string prefix, vector<mpz_t *> vP);
void spdlog_vector(string prefix, vector<commitment_t> v);
void spdlog_vector(string prefix, vector<uint64_t> v);

// to/from json
void to_json(json &j, const commitment_map_key_t &c);
void from_json(const json &j, commitment_map_key_t &cmk);

// current time in nanoseconds
uint64_t current_time();

// string <-> uint64_t
uint64_t str_to_uint64_t(string s);
string uint64_t_to_str(uint64_t i);

// json -> mpz_t
void set_mpz_str_from_json(const json &j, string field, mpz_t *elemP);

// debug data,coding ptrs -> vector of vectors of chars
vector<vector<char>> create_data_view(char **data, const size_t block_size,
                                      const int32_t n);

// generate file path for output files
const string output_file_path(string input_file_path, const uint64_t av_id,
                              const uint64_t manufacturer_id,
                              const uint64_t software_id,
                              const uint64_t version_id);

// compare two files
const bool compare_file(string a_file, string b_file);

// mpz printing tools
inline const string mpz_to_string(mpz_t &x) {
  return mpz_get_str(nullptr, 16, x);
}

// id printing tools (for logs)
inline const string avswu_id(const uint64_t manufacturer_id) {
  string msg = "(manufacturer_id=" + to_string(manufacturer_id) + ")";
  return msg;
}

inline const string avswu_id(const uint64_t manufacturer_id,
                             const uint64_t software_id) {
  string msg = "(manufacturer_id=" + to_string(manufacturer_id) +
               ", software_id=" + to_string(software_id) + ")";
  return msg;
}

inline const string avswu_id(const uint64_t manufacturer_id,
                             const uint64_t software_id,
                             const uint64_t version_id) {
  string msg = "(manufacturer_id=" + to_string(manufacturer_id) +
               ", software_id=" + to_string(software_id) +
               ", version_id=" + to_string(version_id) + ")";
  return msg;
}

inline const string avswu_id(const uint64_t manufacturer_id,
                             const uint64_t software_id,
                             const uint64_t version_id,
                             const uint64_t shard_id) {
  string msg = "(manufacturer_id=" + to_string(manufacturer_id) +
               ", software_id=" + to_string(software_id) +
               ", version_id=" + to_string(version_id) +
               ", shard_id=" + to_string(shard_id) + ")";
  return msg;
}

// zero pad a number
const string zero_pad(const int num, const size_t width);

// start ipfs and parse libipfs result string
const bool start_ipfs_node();
const bool stop_ipfs_node();

} // namespace avswu_utils

#include "AvswuUtils.h"
#include <chrono>
#include <stdlib.h>

#include "libipfs-linux.h"
#include <filesystem>

using namespace std;

namespace avswu_utils {

void clear_commitment_vector(vector<commitment_t> &v) {
  for (size_t i = 0; i < v.size(); i++) {
    mpz_init(v[i].R);
    mpz_set_ui(v[i].R, 0);
    v[i].w = 0;
    mpz_init(v[i].W);
    mpz_set_ui(v[i].W, 0);
  }
  v.clear();
}

void set_commitment_map_key(commitment_map_key_t &key, uint64_t shard_id,
                            uint64_t signer_id) {
  key.shard_id = shard_id;
  key.signer_id = signer_id;
}

// truncate left and right
string left_right_truncate(string s, uint32_t left, uint32_t right) {
  string s_left = s.substr(0, left);
  string s_right = s.substr(s.length() - right, right);
  return s_left + "..." + s_right;
}

// logs vectors of elements, and other types
void spdlog_vector(string prefix, vector<mpz_t> v) {
  for (size_t i = 0; i < v.size(); i++) {
    spdlog::debug("{}[{}]={}", prefix, i, mpz_to_string(v[i]));
  }
}

void spdlog_vector(string prefix, vector<mpz_t *> vP) {
  spdlog::debug("{}.size()={}", prefix, vP.size());
  for (size_t i = 0; i < vP.size(); i++) {
    spdlog::debug("{}[{}]={}", prefix, i, mpz_to_string(*(vP[i])));
  }
}

void spdlog_vector(string prefix, vector<commitment_t> v) {
  spdlog::debug("{}.size()={}", prefix, v.size());
  for (size_t i = 0; i < v.size(); i++) {
    spdlog::debug("{}[{}].R={}", prefix, i, mpz_to_string(v[i].R));
    spdlog::debug("{}[{}].w={}", prefix, i, v[i].w);
    spdlog::debug("{}[{}].W={}", prefix, i, mpz_to_string(v[i].W));
  }
}

// to/from json
void to_json(json &j, const commitment_map_key_t &c) {
  j = json{{"shard_id", c.shard_id}, {"signer_id", c.signer_id}};
}

void from_json(const json &j, commitment_map_key_t &cmk) {
  j.at("shard_id").get_to(cmk.shard_id);
  j.at("signer_id").get_to(cmk.signer_id);
}

// current time in nanoseconds
uint64_t current_time() {
  auto now = std::chrono::high_resolution_clock::now();
  auto now_ns = chrono::time_point_cast<std::chrono::nanoseconds>(now);
  auto now_ns_since_epoch = now_ns.time_since_epoch().count();
  return now_ns_since_epoch;
}

// str -> uint64_t
uint64_t str_to_uint64_t(string str) {
  char *end = nullptr;
  const char *c_str = str.c_str();
  errno = 0;
  uint64_t result = strtoull(c_str, &end, 16);
  if (result == 0 && end == c_str) {
    spdlog::error("Unable to convert string to uint64_t");
  } else if (result == ULLONG_MAX && errno) {
    spdlog::error("Value of str larger than uint64_t");
  } else if (*end) {
    spdlog::warn("String has leftover debugrmation in it that is not a number");
  }

  return result;
}

// uint64_t -> str
string uint64_t_to_str(uint64_t i) {
  stringstream stream;
  stream << hex << i;
  string str(stream.str());
  return str;
}

// json -> mpz_t
void set_mpz_str_from_json(const json &j, string field, mpz_t *elemP) {
  string s;
  j.at(field).get_to(s);
  mpz_set_str(*elemP, s.c_str(), 16);
}

// debug data,coding ptrs -> vector of vectors of chars
vector<vector<char>> create_data_view(char **data, const size_t block_size,
                                      const int32_t n) {
  vector<vector<char>> data_view(n);
  for (size_t i = 0; i < n; i++) {
    data_view[i].resize(block_size);
    memcpy(data_view[i].data(), data[i], block_size);
  }
  return data_view;
}

// path for output
const string output_file_path(const string input_file_path,
                              const uint64_t av_id,
                              const uint64_t manufacturer_id,
                              const uint64_t software_id,
                              const uint64_t version_id) {

  /*
   * example:
   * input_file = test_data_0001.data";
   *
   * output_file = string(AVSWU_BASEDIR) +
   * "/veins-client/output/av210_man100_sof110_ver121_test_data_0001.data";
   */
  const string input_file = filesystem::path(input_file_path).filename();
  const string base_path = string(AVSWU_BASEDIR) + "/veins-client/output/";

  const string av = "av_" + to_string(av_id);
  const string man = "man_" + to_string(manufacturer_id);
  const string sof = "sof_" + to_string(software_id);
  const string ver = "ver_" + to_string(version_id);
  const string prefix = av + "_" + man + "_" + sof + "_" + ver;
  const string result = base_path + prefix + "_" + input_file;

  return result;
}

// compare two files
const bool compare_file(string a_file, string b_file) {
  const string cmd = "/usr/bin/cmp " + a_file + " " + b_file;
  const int status_code = system(cmd.c_str());
  if (status_code == -1) {
    spdlog::error("unable to run /usr/bin/cmp");
  }
  return (status_code == 0);
}

// zero pad a number
const string zero_pad(const int num, const size_t width) {
  std::ostringstream ss;
  ss << std::setw(width) << std::setfill('0') << num;
  return ss.str();
}

/*
 * ipfs utils
 */

// start ipfs and parse libipfs result string
const bool start_ipfs_node() {
  // start ipfs
  spdlog::info("starting IPFS node");
  string ipfs_result;
  try {
    ipfs_result = IPFSStartNode((char *)"", 0);
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to start ipfs");
    return false;
  }

  // check for errors
  try {
    json j_result = json::parse(ipfs_result);
    if (j_result["Status"] != "ok") {
      spdlog::error("ipfs error, ipfs_result={}", ipfs_result);
      return false;
    }
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse ipfs_result={}", ipfs_result);
    return false;
  }

  return true;
}

const bool stop_ipfs_node() {
  // stop ipfs
  spdlog::info("stopping IPFS node");
  string ipfs_result;
  try {
    ipfs_result = IPFSStopNode();
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to stop ipfs");
    return false;
  }

  // check for errors
  try {
    json j_result = json::parse(ipfs_result);
    if (j_result["Status"] != "ok") {
      spdlog::error("ipfs error, ipfs_result={}", ipfs_result);
      return false;
    }
  } catch (const std::runtime_error &e) {
    spdlog::error("unable to parse ipfs_result={}", ipfs_result);
    return false;
  }

  return true;
}

} // namespace avswu_utils

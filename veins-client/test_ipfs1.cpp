#include "libipfs-linux.h"
#include <getopt.h>
#include <spdlog/spdlog.h>

#include "AvswuUtils.h"

#include <unistd.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

int main() {
  bool is_success = avswu_utils::start_ipfs_node();
  if (!is_success) {
    spdlog::error("unable to start ipfs node");
  }

  spdlog::info("getting peer ID");
  spdlog::info(GetPeerID());

  spdlog::info("adding file to IPFS");
  const string result =
      IpfsAdd((char *)"/home/gsolomon/avswu/veins-client/input/kb/"
                      "test_data_0003.data");
  spdlog::info("result={}", result);

  // get cid from result
  json j_result = json::parse(result);
  const string cid = j_result["Message"];
  spdlog::info("cid={}", cid);

  // remove file on disk, before downloading
  const string output_file =
      "/home/gsolomon/avswu/veins-client/output/ipfs-download.data";
  std::filesystem::remove(output_file);

  spdlog::info("downloading file from IPFS");
  spdlog::info(IpfsGet((char *)cid.c_str(), (char *)output_file.c_str()));

  // stop ipfs
  is_success = avswu_utils::stop_ipfs_node();
  if (!is_success) {
    spdlog::error("unable to stop ipfs node");
  }
}
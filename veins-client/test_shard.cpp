/*
 * shard protocol test
 */
#include "AvswuAVNode.h"
#include "AvswuConfig.h"
#include "AvswuGrpcClient.h"
#include "AvswuManufacturerNode.h"
#include "AvswuNotification.h"
#include "AvswuPacketInventory.h"
#include "AvswuPacketNotificationList.h"
#include "AvswuPacketShardRequestList.h"
#include "AvswuRSUNode.h"
#include "AvswuUtils.h"

#include "libipfs-linux.h"
#include <getopt.h>
#include <spdlog/spdlog.h>

#include <unistd.h>

using namespace std;
using namespace avswu_grpc_client;
using namespace avswu_manufacturer_node;
using namespace avswu_notification;
using namespace avswu_rsu_node;
using namespace avswu_av_node;
using namespace avswu_packet_inventory;
using namespace avswu_packet_shard_request_list;
using namespace avswu_packet_notification_list;
using namespace avswu_config;

using json = nlohmann::json;

void parse_args(int argc, char **argv) {
  // parse command line arguments using gnu's getopt

  string path = argv[0];
  string prog = path.substr(path.find_last_of("/\\") + 1);

  // f,n,c has a parameter, v does not have a parameter, => "f:n:v"
  const char *short_options = "v";
  option long_options[] = {{"verbose", no_argument, NULL, 'v'},
                           {NULL, 0u, NULL, 0u}};
  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'v':
      spdlog::set_level(spdlog::level::trace);
      break;
    case '?':
      spdlog::error("command line error. one or more flags not recognized: {}",
                    char(opt));
      break;
    default:
      spdlog::warn("usage: {} prog --verbose", prog);
      exit(-1);
    }
  }
}

void shard_test(AvswuGrpcClient *clientP, const string config_file) {

  // start ipfs
  bool is_success = avswu_utils::start_ipfs_node();
  if (!is_success) {
    spdlog::error("unable to start ipfs node");
  }

  // seed the random number generator
  srand(time(NULL));

  /*
     we use config_inventory_index = 2, for manufacturer_id = 100
    from config.json since it has only 1 software in it's inventory
        {
          "index": 2,
          "software_index_list": [3]
        },
  */

  /*
   * create some ids, public keys, and cache size
   */

  // manufacturer reads configuration
  AvswuConfig config;
  try {
    config.read_file(config_file);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
  }

  // manufacturer id
  const uint64_t manufacturer_id = 100;

  // get initial software_item_t config info (for manufacturer to shard and
  // send)
  const uint64_t config_software_id = 1003;
  software_item_t sw_item;
  const bool found_sw_item =
      config.get_software_item(sw_item, manufacturer_id, config_software_id);
  if (!found_sw_item) {
    spdlog::error("unable to find software item in {}",
                  avswu_id(manufacturer_id, config_software_id));
  }

  // values from sw config info
  const uint64_t software_id = sw_item.software_id;
  // create a new s/w version (by increasing version id ) as the new version
  // will be stored on the blockchain via the notification
  const uint64_t version_id = sw_item.version_id + 1;

  // set input file
  const string dir = sw_item.file.directory;
  const string name = sw_item.file.name;
  const string input_file = dir + "/" + name;

  // rsu id
  const uint64_t rsu_id = 1000;

  // av id and it's software config
  const uint64_t av_id = 2000;
  const size_t config_inventory_index = 2;

  // create manufacturer
  size_t cache_max_size = 1024;
  AvswuManufacturerNode man(manufacturer_id, cache_max_size, clientP);
  spdlog::debug("man.X={}", mpz_to_string(man.X()));

  /*
   * manufacturer read file, create packets with shards
   */
  // read file
  vector<char> input_data;
  try {
    spdlog::info("input_file={}", input_file);
    input_data = man.read_file(input_file);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
  }

  // create packets, and writes packets to ipfs
  vector<AvswuPacketShard> man_packet_list;
  try {
    man_packet_list = man.create_packet_list(
        software_id, version_id, input_data, AVSWU_ENCODE_K, AVSWU_ENCODE_M);
  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
  }

  spdlog::debug("packet_list.size()={}", man_packet_list.size());

  // manufacturer write packets to ipfs
  vector<string> cid_hash_list;
  for (size_t i = 0; i < man_packet_list.size(); i++) {
    string cid_hash;
    const bool is_success = man.ipfs_write_packet(cid_hash, man_packet_list[i]);
    if (is_success && cid_hash.size() > 0) {
      spdlog::debug("cid()={}", cid_hash);
      cid_hash_list.push_back(cid_hash);
    }
  }

  /*
   * manufacturer creates notifications for 1 of it's software (with current
   * time create_time. so we have all new software and writes notification to
   * blockchain for any/all inventories
   */

  AvswuNotification notif;
  const double create_time = 1.0;
  const bool is_success_set_notif = man.set_notification(
      notif, man_packet_list, cid_hash_list, manufacturer_id, software_id,
      version_id, input_data, AVSWU_ENCODE_K, AVSWU_ENCODE_M, create_time);
  if (is_success_set_notif) {
    spdlog::debug("notif={}", json(notif).dump(4));

    // write notification to the blockchain
    const bool is_success_write_notif =
        man.blockchain_write_notification(notif);

    if (!is_success_write_notif) {
      spdlog::error("write notification failed");
      exit(-1);
    }
  }

  // create an av
  AvswuAVNode av(av_id, cache_max_size);

  // create RSU
  AvswuRSUNode rsu(rsu_id, cache_max_size, clientP);

  /*
   * AV creates s/w inventory packet for (av,RSU) by reading the config
   */

  AvswuPacketInventory packet_inventory(av.id(), av.X(), rsu.id(), rsu.X(),
                                        node_type_t::client_av);

  // av get it's s/w configuration (this happens upon av
  // creation/initialization in practice)
  const bool is_payload_set = packet_inventory.set_payload(
      &config, manufacturer_id, config_inventory_index);
  if (!is_payload_set) {
    spdlog::error("unable to set payload");
  }
  spdlog::debug("inventory={}", json(packet_inventory).dump(4));

  /*
   * if RSU in range of AV, AV sends s/w inventory packet to RSU
   */
  // based on veins, av.send(inventory);

  /*
   * RSU receives s/w inventory packet from AV
   */
  // based on veins, rsu.receive(inventory);

  /*
   * RSU reads notification_list from blockahin, based on s/w inventory from
   * AV
   */
  AvswuPacketNotificationList notification_list =
      rsu.blockchain_read_notification_list(av.id(), av.X(), packet_inventory);

  /*
   * if AV in range of RSU, RSU sends notification_list to AV
   */
  // based on veins, rsu.send(notification_list);

  /*
   * AV receives notification_list
   */
  // based on veins, av.receive(notification_list);

  /*
   * rsu read packets of shards from ipfs based on those requested from
   * notification, and saves them into it's cache
   */
  vector<AvswuPacketShard> shard_list;
  bool is_success_prime_cache =
      rsu.ipfs_read_shard_list(shard_list, av.id(), av.X(), notification_list);
  if (is_success_prime_cache) {
    spdlog::info("successsfully primed the cache");
  } else {
    spdlog::error("priming the cache failed");
  }

  /*
   * AV creates shard request list based on notification_list
   */
  AvswuPacketShardRequestList *shard_request_listP =
      av.create_shard_request_list(rsu.id(), rsu.X(), node_type_t::rsu,
                                   notification_list);
  if (!shard_request_listP) {
    spdlog::error("fatal error");
    return;
  }
  // spdlog::debug("shard_request_list={}", json(shard_request_list).dump(4));

  /*
   * if RSU in range of AV, AV sends shard request list to RSU (or AV)
   */
  // based on veins, av.send(shard_request_list);

  /*
   * if RSU in range of AV, RSU receives shard request list from AV
   */
  // based on veins, rsu.receive(shard_request_list);

  /*
   * RSU fetches shard packets from it's cache based on shard_request_list
   */

  vector<AvswuPacketShard> request_packet_list =
      rsu.get_request_packet_list(av.id(), av.X(), *shard_request_listP);
  for (size_t i = 0; i < request_packet_list.size(); i++) {
    auto packet = request_packet_list[i];
    // spdlog::debug("request_packet_list[{}]={}", i, json(packet).dump());
  }

  /*
   * while AV in range, RSU sends a requested shards to AV (one at a time)
   */
  uint32_t distance = 0;
  size_t i = 0;
  while (distance < 10 && i < request_packet_list.size()) {
    auto shard = request_packet_list[i];
    // rsu.send(shard);
    distance -= 1;
    i++;
  }

  /*
   * AV recieves shard, inserts shard in cache, then updates its shard_request
   * list
   */
  i = 0;
  while (i < request_packet_list.size()) {
    auto shard = request_packet_list[i];
    // av.receive(shard);
    av.insert(shard);
    // av.update(shard_request_list,shard);
    i++;
  }

  /*
   * AV attempts to decode each software in notification_list
   */
  string output_file_path;
  for (size_t i = 0; i < notification_list.deque().size(); i++) {
    auto notif = notification_list.deque()[i];
    auto manufacturer_id = notif.manufacturer_id();
    auto software_id = notif.software_id();
    auto version_id = notif.version_id();

    // attempt to decode the shards
    vector<char> decoded_output_data;
    double earliest_shard_request_time;
    const decode_status_t status =
        av.decode(decoded_output_data, earliest_shard_request_time,
                  manufacturer_id, software_id, version_id);

    // print message based on status
    av.spdlog_decode_status(status);

    // if successful write to disk
    if (status.code == DECODE_SUCCESS) {
      spdlog::info(
          "decode success, manufacturer_id={}, software_id={}, version_id={}",
          manufacturer_id, software_id, version_id);
      // write software to disk
      try {
        output_file_path = avswu_utils::output_file_path(
            input_file, av_id, manufacturer_id, software_id, version_id);
        spdlog::info("writing to output_file_path={}", output_file_path);
        av.write_file(output_file_path, decoded_output_data);
      } catch (const std::runtime_error &e) {
        spdlog::error("exception e={}", e.what());
      }
    }
  }

  // compare the output with input
  const bool input_output_match =
      avswu_utils::compare_file(input_file, output_file_path);
  if (input_output_match) {
    spdlog::info("input and output files are the same");
  } else {
    spdlog::error("input and output files are not the same");
  }

  // stop ipfs
  is_success = avswu_utils::stop_ipfs_node();
  if (!is_success) {
    spdlog::error("unable to stop ipfs node");
  }
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::info);

  // parse args
  // parse file args
  parse_args(argc, argv);

  // argc and argv are handle by getopt, pass argc=1 only program name, and argv
  AvswuGrpcClient client(1u, argv);

  // shard input file
  string config_file =
      string(AVSWU_BASEDIR) + "/veins-client/inventory/config.json";
  shard_test(&client, config_file);

  return 0;
}
#include <gtest/gtest.h>

#include "AvswuAVNode.h"
#include "AvswuGrpcClient.h"
#include "AvswuManufacturerNode.h"
#include "AvswuNotification.h"
#include "AvswuRSUNode.h"
#include <spdlog/spdlog.h>

#include <unistd.h>

using namespace avswu_notification;
using namespace std;
using namespace avswu_grpc_client;
using namespace avswu_manufacturer_node;
using namespace avswu_notification;
using namespace avswu_rsu_node;
using namespace avswu_av_node;

// global test environment w/ setup and tear down
class NotificationTestEnviron : public ::testing::Environment {

  // global setup
  void SetUp() override {
    const bool is_success = start_ipfs_node();
    if (!is_success) {
      spdlog::error("unable to start ipfs node");
    }
  }

  // global tear down
  void TearDown() override {
    const bool is_success = stop_ipfs_node();
    if (!is_success) {
      spdlog::error("unable to stop ipfs node");
    }
  }
};

class NotificationTest : public ::testing::Test {
  // fixture for setup and teardown
protected:
  // variables

  // gRPC
  AvswuGrpcClient *_clientP;

  // ids
  const uint64_t _manufacturer_id = 100;
  const uint64_t _software_id = 1003;
  const uint64_t _version_id = 0;
  const uint64_t _rsu_id = 1000;

  const uint64_t _av_id = 100;

  // manufacturer
  const size_t _cache_max_size = 1024;
  const uint32_t _k = 6;
  const uint32_t _m = 3;
  AvswuManufacturerNode *_manP;
  vector<AvswuPacketShard> _man_packet_list;

  double _create_time = 1.0;

  vector<packet_notif_t> _packet_notif_list;
  vector<string> _cid_hash_list;

  vector<char> _input_data;

  const string _config_file =
      string(AVSWU_BASEDIR) + "/veins-client/inventory/config.json";
  const size_t _config_inventory_index = 6;
  AvswuConfig _config;

  void SetUp() override {
    // set debug level
    spdlog::set_level(spdlog::level::debug);

    // seed the random number generator
    srand(time(NULL));

    // set gRPC client
    char *argv[1];
    string prog = "google_test";
    argv[0] = (char *)prog.c_str();
    _clientP = new AvswuGrpcClient(1u, argv);

    // create man
    _manP =
        new AvswuManufacturerNode(_manufacturer_id, _cache_max_size, _clientP);

    try {
      string input_file =
          string(AVSWU_BASEDIR) + "/veins-client/input/kb/test_data_0010.data";
      _input_data = _manP->read_file(input_file);
    } catch (const std::runtime_error &e) {
      spdlog::error("exception e={}", e.what());
    }

    // reads config
    try {
      _config.read_file(_config_file);
    } catch (const std::runtime_error &e) {
      spdlog::error("exception e={}", e.what());
    }
  }

  void TearDown() override {}

  // create a new s/w version
  const bool CreateNotification(AvswuNotification &notif,
                                const uint64_t software_id,
                                const uint64_t version_id) {

    // man packet list
    _man_packet_list =
        _manP->create_packet_list(software_id, version_id, _input_data, _k, _m);

    // create packet notif list and cid hash list
    _packet_notif_list.clear();
    _cid_hash_list.clear();
    const size_t n = _man_packet_list.size();
    for (size_t i = 0; i < n; i++) {
      packet_notif_t pn;
      string result_str;
      const bool is_success =
          _manP->ipfs_write_packet(result_str, _man_packet_list[i]);
      // spdlog::error("result_str={}", result_str);
      if (is_success) {
        const string cid_hash = result_str;
        auto man_packet = _man_packet_list[i];
        if (cid_hash.size() > 0) {
          spdlog::debug("cid()={}", cid_hash);
          _cid_hash_list.push_back(cid_hash);
          pn.shard_hash = man_packet.footer().payload_hash();
          _packet_notif_list.push_back(pn);
        }
      } else {
        spdlog::error("ipfs_write_packet failed, result_str={}", result_str);
        return false;
      }
    }

    const bool result = _manP->set_notification(
        notif, _man_packet_list, _cid_hash_list, _manufacturer_id, software_id,
        version_id, _input_data, _k, _m, _create_time);
    return result;
  };
};

TEST_F(NotificationTest, Create) {
  AvswuNotification notif(_manufacturer_id, _software_id, _version_id,
                          _packet_notif_list, _create_time);
  EXPECT_NE(&notif, nullptr);
}

TEST_F(NotificationTest, ToJson) {
  // sh->json str (from_json)
  AvswuNotification notif1(_manufacturer_id, _software_id, _version_id,
                           _packet_notif_list, _create_time);
  json j1(notif1);
  string notif_json_str = j1.dump(4);
  spdlog::debug("notif_json_str1={}", notif_json_str);

  EXPECT_TRUE(notif_json_str.size() > 0);
}

TEST_F(NotificationTest, FromJson) {
  // sh->json str (from_json)
  AvswuNotification notif1(_manufacturer_id, _software_id, _version_id,
                           _packet_notif_list, _create_time);
  json j1(notif1);
  string notif_json_str1 = j1.dump(4);
  spdlog::debug("notif_json_str1={}", notif_json_str1);

  // sh <- json str (to_json)
  AvswuNotification sh2(j1);
  json j2(sh2);
  string notif_json_str2 = j2.dump(4);
  spdlog::debug("notif_json_str2={}", notif_json_str2);

  EXPECT_TRUE(notif_json_str1 == notif_json_str2);
}

TEST_F(NotificationTest, WriteOneNotification) {

  // create notification
  AvswuNotification notif;

  const bool is_success_set_notif =
      CreateNotification(notif, _software_id, _version_id);
  EXPECT_TRUE(is_success_set_notif);

  if (is_success_set_notif) {
    // write notification to the blockchain
    spdlog::debug("writing notif={}", json(notif).dump(4));
    const bool is_success_write_notif =
        _manP->blockchain_write_notification(notif);
    EXPECT_TRUE(is_success_write_notif);
  }
}

TEST_F(NotificationTest, ReadNotification) {

  // create notifications
  const size_t n = 4;
  AvswuNotification write_notif[n];

  for (size_t i = 0; i < n; i++) {
    // create notification with new version id
    const bool is_success_set_notif =
        CreateNotification(write_notif[i], _software_id, _version_id + i + 1);
    EXPECT_TRUE(is_success_set_notif);

    if (is_success_set_notif) {
      // write notification to the blockchain
      const bool is_success_write_notif =
          _manP->blockchain_write_notification(write_notif[i]);
      EXPECT_TRUE(is_success_write_notif);
    }
    spdlog::debug("write_notif[{}].create_time={}", i,
                  write_notif[i].create_time());
  }

  // create RSU
  AvswuRSUNode rsu(_rsu_id, _cache_max_size, _clientP);

  // create an av
  AvswuAVNode av(_av_id, _cache_max_size);

  AvswuPacketInventory packet_inventory(av.id(), av.X(), rsu.id(), rsu.X(),
                                        node_type_t::unknown);
  try {
    const bool is_payload_set = packet_inventory.set_payload(
        &_config, _manufacturer_id, _config_inventory_index);
    if (!is_payload_set) {
      spdlog::error("unable to set payload");
      EXPECT_TRUE(false);
    }
    spdlog::debug("inventory={}", json(packet_inventory).dump(4));
  } catch (const std::runtime_error &e) {
    EXPECT_TRUE(false);
  }

  // read notification

  AvswuPacketNotificationList notification_list =
      rsu.blockchain_read_notification_list(av.id(), av.X(), packet_inventory);

  /*
   * rsu read packets of shards from ipfs based on those requested from
   * notification, and saves them into it's cache
   */
  vector<AvswuPacketShard> shard_list;
  bool is_success_prime_cache =
      rsu.ipfs_read_shard_list(shard_list, av.id(), av.X(), notification_list);

  EXPECT_TRUE(is_success_prime_cache);
}

TEST_F(NotificationTest, WriteMultipleNotification) {

  // create notification
  AvswuNotification notif;

  for (size_t i = 0; i < 50; i++) {
    const bool is_success_set_notif =
        CreateNotification(notif, _software_id, _version_id + i);
    EXPECT_TRUE(is_success_set_notif);
    if (is_success_set_notif) {
      // write notification to the blockchain
      spdlog::debug("writing notif={}", json(notif).dump(4));
      const bool is_success_write_notif =
          _manP->blockchain_write_notification(notif);
      EXPECT_TRUE(is_success_write_notif);
    }
  }
}

int main(int argc, char *argv[]) {
  testing::InitGoogleTest();
  testing::AddGlobalTestEnvironment(new NotificationTestEnviron());
  return RUN_ALL_TESTS();
}
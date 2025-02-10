#include <gtest/gtest.h>

#include "AvswuPacketNotificationList.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_notification_list;

// fixture for setup and teardown
class PacketNotificationListTest : public testing::Test {
protected:
  // variables
  uint64_t _from_id = 100;
  mpz_t _from_X;
  uint64_t _to_id = 200;
  mpz_t _to_X;

  void SetUp() override {

    mpz_init(_from_X);
    mpz_init(_to_X);

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}

  // add items to a packet
  void CreateNotifList(AvswuPacketNotificationList &packet, const size_t n) {
    for (size_t i = 0; i < n; i++) {
      AvswuNotification notif;
      notif.set(10 + i, 100 + i, 1000 + i);
      // add notif info per shard
      for (size_t j = 0; j < 3; j++) {
        notif.push_back(100 + j, "fake-ipfs-cid-hash", "fake-sha3-hash");
      }
      packet.push_back(notif);
    }
  }
};

TEST_F(PacketNotificationListTest, Create) {
  AvswuPacketNotificationList p(_from_id, _from_X, _to_id, _to_X,
                                node_type_t::unknown);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketNotificationListTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketNotificationList packet1(_from_id, _from_X, _to_id, _to_X,
                                      node_type_t::unknown);
  CreateNotifList(packet1, 3);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketNotificationListTest, FromJson) {
  // packet1->json str (from_json)
  AvswuPacketNotificationList packet1(_from_id, _from_X, _to_id, _to_X,
                                      node_type_t::unknown);
  CreateNotifList(packet1, 2);

  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketNotificationList packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}
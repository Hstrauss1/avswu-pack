#include <gtest/gtest.h>

#include "AvswuPacketSupportList.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_support_list;

// fixture for setup and teardown
class PacketSupportListTest : public testing::Test {
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
  void CreateItemList(AvswuPacketSupportList &packet, const size_t n) {
    for (size_t i = 0; i < n; i++) {
      manufacturer_id_t item;
      item.manufacturer_id = 10 + i;
      uint64_t X = 100 + i;
      mpz_init_set_ui(item.manufacturer_X, X);
      packet.push_back(item);
    }
  }
};

TEST_F(PacketSupportListTest, Create) {
  AvswuPacketSupportList p(_from_id, _from_X, _to_id, _to_X);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketSupportListTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketSupportList packet1(_from_id, _from_X, _to_id, _to_X);
  CreateItemList(packet1, 3);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketSupportListTest, FromJson) {
  // packet1->json str (from_json)
  AvswuPacketSupportList packet1(_from_id, _from_X, _to_id, _to_X);
  CreateItemList(packet1, 2);

  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketSupportList packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}
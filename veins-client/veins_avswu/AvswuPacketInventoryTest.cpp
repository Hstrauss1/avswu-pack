#include <gtest/gtest.h>

#include "AvswuPacketInventory.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_inventory;

// fixture for setup and teardown
class PacketInventoryTest : public testing::Test {
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
  void CreateItemList(AvswuPacketInventory &packet, const size_t n) {
    for (size_t i = 0; i < n; i++) {
      software_t item;
      item.manufacturer_id = 10 + i;
      item.software_id = 100 + i;
      item.version_id = 1000 + i;
      item.create_time = current_time() / SECOND_TO_NANOSECOND;
      packet.push_back(item);
    }
  }
};

TEST_F(PacketInventoryTest, Create) {
  AvswuPacketInventory p(_from_id, _from_X, _to_id, _to_X,
                         node_type_t::unknown);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketInventoryTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketInventory packet1(_from_id, _from_X, _to_id, _to_X,
                               node_type_t::unknown);
  CreateItemList(packet1, 3);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketInventoryTest, FromJson) {
  // packet1->json str (from_json)
  AvswuPacketInventory packet1(_from_id, _from_X, _to_id, _to_X,
                               node_type_t::unknown);
  CreateItemList(packet1, 2);

  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketInventory packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}
#include <gtest/gtest.h>

#include "AvswuPacketDeliveryRequest.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_delivery_request;

// fixture for setup and teardown
class PacketDeliveryRequesTest : public testing::Test {
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
};

TEST_F(PacketDeliveryRequesTest, Create) {
  const double create_time =
      static_cast<double>(current_time()) / SECOND_TO_NANOSECOND;
  AvswuPacketDeliveryRequest p(_from_id, _from_X, _to_id, _to_X, create_time,
                               10, node_type_t::unknown);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketDeliveryRequesTest, ToJson) {
  // packet->json str (from_json)
  const double create_time =
      static_cast<double>(current_time()) / SECOND_TO_NANOSECOND;
  AvswuPacketDeliveryRequest packet1(_from_id, _from_X, _to_id, _to_X,
                                     create_time, 10, node_type_t::unknown);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketDeliveryRequesTest, FromJson) {
  // packet1->json str (from_json)
  const double create_time =
      static_cast<double>(current_time()) / SECOND_TO_NANOSECOND;
  AvswuPacketDeliveryRequest packet1(_from_id, _from_X, _to_id, _to_X,
                                     create_time, 10, node_type_t::unknown);

  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketDeliveryRequest packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}

#include <gtest/gtest.h>

#include "AvswuPacketHeader.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_header;

// fixture for setup and teardown
class AvswuPacketHeaderTest : public testing::Test {
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

TEST_F(AvswuPacketHeaderTest, Create) {
  AvswuPacketHeader packet_header(_from_id, _from_X, _to_id, _to_X,
                                  payload_type_t::none, node_type_t::unknown);
  EXPECT_NE(&packet_header, nullptr);
}

TEST_F(AvswuPacketHeaderTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketHeader packet_header1(_from_id, _from_X, _to_id, _to_X,
                                   payload_type_t::none, node_type_t::unknown);
  json j1(packet_header1);
  string packet_header1_json_str = j1.dump(4);
  spdlog::debug("packet_header1_json_str={}", packet_header1_json_str);

  EXPECT_TRUE(packet_header1_json_str.size() > 0);
}

TEST_F(AvswuPacketHeaderTest, FromJson) {
  // packet_header1->json str (from_json)
  AvswuPacketHeader packet_header1(_from_id, _from_X, _to_id, _to_X,
                                   payload_type_t::none, node_type_t::unknown);
  json j1(packet_header1);
  string packet_header1_json_str = j1.dump(4);
  spdlog::debug("packet_header1_json_str={}", packet_header1_json_str);

  // packet_header2 <- json str (to_json)
  AvswuPacketHeader packet_header2(j1);
  json j2(packet_header2);
  string packet_header2_json_str = j2.dump(4);
  spdlog::debug("packet_header2_json_str={}", packet_header2_json_str);

  EXPECT_TRUE(packet_header2_json_str == packet_header2_json_str);
}
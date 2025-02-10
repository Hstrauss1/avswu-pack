#include <gtest/gtest.h>

#include "AvswuPacketFooter.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_footer;

// fixture for setup and teardown
class AvswuPacketFooterTest : public testing::Test {
protected:
  // variables
  const string _header_payload_hash = "fake-hash";
  signature_t _signature;

  void SetUp() override {

    mpz_init_set_ui(_signature.R, 123456);
    mpz_init_set_ui(_signature.s, 654321);

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(AvswuPacketFooterTest, Create) {
  AvswuPacketFooter packet_footer(_header_payload_hash, _signature.R,
                                  _signature.s);
  EXPECT_NE(&packet_footer, nullptr);
}

TEST_F(AvswuPacketFooterTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketFooter packet_footer1(_header_payload_hash, _signature.R,
                                   _signature.s);
  json j1(packet_footer1);
  string packet_footer1_json_str = j1.dump(4);
  spdlog::debug("packet_footer1_json_str={}", packet_footer1_json_str);

  EXPECT_TRUE(packet_footer1_json_str.size() > 0);
}

TEST_F(AvswuPacketFooterTest, FromJson) {
  // packet_footer1->json str (from_json)
  AvswuPacketFooter packet_footer1(_header_payload_hash, _signature.R,
                                   _signature.s);
  json j1(packet_footer1);
  string packet_footer1_json_str = j1.dump(4);
  spdlog::debug("packet_footer1_json_str={}", packet_footer1_json_str);

  // packet_footer2 <- json str (to_json)
  AvswuPacketFooter packet_footer2(j1);
  json j2(packet_footer2);
  string packet_footer2_json_str = j2.dump(4);
  spdlog::debug("packet_footer2_json_str={}", packet_footer2_json_str);

  EXPECT_TRUE(packet_footer2_json_str == packet_footer2_json_str);
}
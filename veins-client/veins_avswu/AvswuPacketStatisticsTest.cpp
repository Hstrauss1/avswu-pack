#include <gtest/gtest.h>

#include "AvswuPacketStatistics.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_statistics;

// fixture for setup and teardown
class AvswuPacketStatisticsTest : public testing::Test {
protected:
  void SetUp() override {

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(AvswuPacketStatisticsTest, Create) {
  AvswuPacketStatistics packet_stats;
  EXPECT_NE(&packet_stats, nullptr);
}

TEST_F(AvswuPacketStatisticsTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketStatistics packet_stats1;
  json j1(packet_stats1);
  string packet_stats1_json_str = j1.dump(4);
  spdlog::debug("packet_stats1_json_str={}", packet_stats1_json_str);

  EXPECT_TRUE(packet_stats1_json_str.size() > 0);
}

TEST_F(AvswuPacketStatisticsTest, FromJson) {
  // packet_stats1->json str (from_json)
  AvswuPacketStatistics packet_stats1;
  json j1(packet_stats1);
  string packet_stats1_json_str = j1.dump(4);
  spdlog::debug("packet_stats1_json_str={}", packet_stats1_json_str);

  // packet_stats2 <- json str (to_json)
  AvswuPacketStatistics packet_stats2(j1);
  json j2(packet_stats2);
  string packet_stats2_json_str = j2.dump(4);
  spdlog::debug("packet_stats2_json_str={}", packet_stats2_json_str);

  EXPECT_TRUE(packet_stats2_json_str == packet_stats2_json_str);
}
#include <gtest/gtest.h>

#include "AvswuPacketShard.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_shard;

// fixture for setup and teardown
class PacketShardTest : public testing::Test {
protected:
  // variables
  uint64_t _from_id = 100;
  mpz_t _from_X;
  uint64_t _to_id = 200;
  mpz_t _to_X;
  uint64_t _manufacturer_id = 1;
  uint64_t _software_id = 2;
  uint64_t _version_id = 3;
  uint64_t _shard_id = 4;
  int _erasure_id = 4;
  uint32_t _k = 6;
  uint32_t _m = 3;
  uint32_t _w = 8;
  size_t _original_file_size = 10;
  const vector<char> _shard_data = {'a', 'b', 'c'};

  void SetUp() override {

    mpz_init(_from_X);
    mpz_init(_to_X);

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(PacketShardTest, Create) {
  AvswuPacketShard p(_from_id, _from_X, _to_id, _to_X, _manufacturer_id,
                     _software_id, _version_id, _shard_id, _erasure_id, _k, _m,
                     _w, _shard_data.data(), _shard_data.size(),
                     _original_file_size);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketShardTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketShard packet1(_from_id, _from_X, _to_id, _to_X, _manufacturer_id,
                           _version_id, _software_id, _shard_id, _erasure_id,
                           _k, _m, _w, _shard_data.data(), _shard_data.size(),
                           _original_file_size);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketShardTest, FromJson) {
  // packet1->json str (from_json)
  AvswuPacketShard packet1(_from_id, _from_X, _to_id, _to_X, _manufacturer_id,
                           _software_id, _version_id, _shard_id, _erasure_id,
                           _k, _m, _w, _shard_data.data(), _shard_data.size(),
                           _original_file_size);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketShard packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}

// increase buffer size
TEST_F(PacketShardTest, IncreaseBufferSize) {
  // packet1->json str (from_json)
  AvswuPacketShard packet1(_from_id, _from_X, _to_id, _to_X, _manufacturer_id,
                           _software_id, _version_id, _shard_id, _erasure_id,
                           _k, _m, _w, _shard_data.data(), _shard_data.size(),
                           _original_file_size);

  size_t orig_size = packet1.shard().buffer_size();
  size_t scale = 3;
  spdlog::debug("scale={}", scale);
  bool const success = packet1.increase_buffer_size(scale);

  bool const match = (orig_size * scale == packet1.shard().buffer_size());
  EXPECT_TRUE(success && match);
}

// decrease buffer size
TEST_F(PacketShardTest, DecreaseBufferSize) {
  // packet1->json str (from_json)
  AvswuPacketShard packet1(_from_id, _from_X, _to_id, _to_X, _manufacturer_id,
                           _software_id, _version_id, _shard_id, _erasure_id,
                           _k, _m, _w, _shard_data.data(), _shard_data.size(),
                           _original_file_size);

  size_t orig_size = packet1.shard().buffer_size();
  size_t scale = orig_size;
  spdlog::debug("scale={}", scale);
  bool const success_dec = packet1.decrease_buffer_size(scale);

  bool const match = (packet1.shard().buffer_size() == 1U);
  EXPECT_TRUE(success_dec && match);
}
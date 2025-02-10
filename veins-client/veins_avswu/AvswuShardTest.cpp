#include <gtest/gtest.h>

#include "AvswuShard.h"
#include <spdlog/spdlog.h>

using namespace avswu_shard;

// fixture for setup and teardown
class ShardTest : public testing::Test {
protected:
  // variables
  uint64_t _manufacturer_id = 1;
  uint64_t _software_id = 2;
  uint64_t _version_id = 2;
  uint64_t _shard_id = 4;
  int _erasure_id = 5;
  uint32_t _k = 6;
  uint32_t _m = 3;
  uint32_t _w = 8;
  size_t _original_file_size = 10;
  const vector<char> _shard_data = {'a', 'b', 'c'};

  void SetUp() override {
    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(ShardTest, Create) {
  AvswuShard sh(_manufacturer_id, _software_id, _version_id, _shard_id,
                _erasure_id, _k, _m, _w, _original_file_size, _shard_data);
  EXPECT_NE(&sh, nullptr);
}

TEST_F(ShardTest, ToJson) {
  // sh->json str (from_json)
  AvswuShard sh1(_manufacturer_id, _software_id, _version_id, _shard_id,
                 _erasure_id, _k, _m, _w, _original_file_size, _shard_data);
  json j1(sh1);
  string sh_json_str = j1.dump(4);
  spdlog::debug("sh_json_str1={}", sh_json_str);

  EXPECT_TRUE(sh_json_str.size() > 0);
}

TEST_F(ShardTest, FromJson) {
  // sh->json str (from_json)
  AvswuShard sh1(_manufacturer_id, _software_id, _version_id, _shard_id,
                 _erasure_id, _k, _m, _w, _original_file_size, _shard_data);
  json j1(sh1);
  string sh_json_str1 = j1.dump(4);
  spdlog::debug("sh_json_str1={}", sh_json_str1);

  // sh <- json str (to_json)
  AvswuShard sh2(j1);
  json j2(sh2);
  string sh_json_str2 = j2.dump(4);
  spdlog::debug("sh_json_str2={}", sh_json_str2);

  EXPECT_TRUE(sh_json_str1 == sh_json_str2);
}
#include <gtest/gtest.h>

#include "AvswuConfig.h"
#include <spdlog/spdlog.h>

using namespace avswu_config;

// fixture for setup and teardown
class ConfigTest : public testing::Test {
protected:
  // variables
  const uint32_t _cache_min = 1024;
  const uint32_t _cache_max = 10240;
  const uint32_t _n_manufacturer = 5;
  const uint32_t _n_software = 4;
  const uint32_t _n_inventory = 10;

  // select data
  uint32_t _input_start = 1;
  uint32_t _input_end = 8;
  uint32_t _input_step = 1;
  string _input_subdir = "kb_small";

  void SetUp() override {
    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }
  void TearDown() override {}
};

TEST_F(ConfigTest, Create) {
  AvswuConfig config(_n_manufacturer, _n_software, _n_inventory, _input_start,
                     _input_end, _input_step, _input_subdir);

  /*
  const uint32_t n_manufacturer,
      const uint32_t n_software, const uint32_t n_inventory,
      uint32_t input_start, uint32_t input_end,
      uint32_t input_step, string input_subdir
      */
  EXPECT_NE(&config, nullptr);
}

TEST_F(ConfigTest, ToJson) {
  // config->json str (from_json)
  AvswuConfig config1(_n_manufacturer, _n_software, _n_inventory, _input_start,
                      _input_end, _input_step, _input_subdir);

  json j1(config1);
  string config1_json_str = j1.dump(4);
  spdlog::debug("config1_json_str={}", config1_json_str);

  EXPECT_TRUE(config1_json_str.size() > 0);
}

TEST_F(ConfigTest, FromJson) {
  // config1->json str (to_json)
  AvswuConfig config1(_n_manufacturer, _n_software, _n_inventory, _input_start,
                      _input_end, _input_step, _input_subdir);

  // get json for config1
  json j1(config1);
  string s1 = j1.dump(4);

  // create config2
  AvswuConfig config2(_n_manufacturer + 10, _n_software + 10, _n_inventory,
                      _input_start, _input_end, _input_step, _input_subdir);
  // update config2 with j1
  from_json(j1, config2);
  // get j2
  json j2(config2);

  // compare j1 to j2
  spdlog::debug("j1.dump()={}", j1.dump(4));
  spdlog::debug("j2.dump()={}", j2.dump(4));
  EXPECT_TRUE(j1.dump() == j2.dump());
}

TEST_F(ConfigTest, WriteReadFile) {
  AvswuConfig config1(_n_manufacturer, _n_software, _n_inventory, _input_start,
                      _input_end, _input_step, _input_subdir);

  string temp_file = "__temp.json";

  config1.write_file(temp_file);

  // get json for config1
  json j1(config1);
  string s1 = j1.dump(4);

  // create config2
  AvswuConfig config2(_n_manufacturer + 10, _n_software + 10, _n_inventory,
                      _input_start, _input_end, _input_step, _input_subdir);
  // update config2 from file
  config2.read_file(temp_file);
  // get j2
  json j2(config2);

  // remove the temp file
  try {
    filesystem::remove(temp_file);
  } catch (const std::filesystem::filesystem_error &err) {
    spdlog::error("filesystem error: err.what()={}", err.what());
  }

  // compare j1 to j2
  spdlog::debug("j1.dump()={}", j1.dump(4));
  spdlog::debug("j2.dump()={}", j2.dump(4));
  EXPECT_TRUE(j1.dump() == j2.dump());
}
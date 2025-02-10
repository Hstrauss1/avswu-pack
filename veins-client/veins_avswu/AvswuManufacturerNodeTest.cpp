#include <gtest/gtest.h>

#include "AvswuManufacturerNode.h"
#include <spdlog/spdlog.h>

using namespace avswu_manufacturer_node;

// fixture for setup and teardown
class ShardTest : public testing::Test {
protected:
  // variables
  AvswuGrpcClient *_clientP;

  void SetUp() override {
    // create grpc client
    string prog = "test_prog";
    char *argv[1];
    argv[0] = (char *)prog.c_str();
    _clientP = new AvswuGrpcClient(1u, argv);

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override { delete _clientP; }
};

TEST_F(ShardTest, Create) {
  size_t cache_max_size = 1024;
  AvswuManufacturerNode man(100, cache_max_size, _clientP);
  EXPECT_NE(&man, nullptr);
}

TEST(AvswuManufacturerNode, PublicKey) {
  string prog = "prog_name";
  char *argv[1];
  argv[0] = (char *)prog.c_str();
  AvswuGrpcClient client(1u, argv);
  size_t cache_max_size = 1024;
  AvswuManufacturerNode man(100, cache_max_size, &client);
  string X(mpz_to_string(man.X()));
  spdlog::debug("man.X={}", X);
  EXPECT_TRUE(X.size() > 0);
}
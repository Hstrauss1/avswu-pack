#include <gtest/gtest.h>

#include "AvswuOpenSSL.h"
#include <spdlog/spdlog.h>

using namespace avswu_open_ssl;

// hash generation tools for testing w/ linux openssl
// printf "quick brown fox" | openssl dgst -blake2b512

// fixture for setup and teardown
class OpenSSLTest : public testing::Test {
protected:
  // variables
  const string _msg = "quick brown fox";

  void SetUp() override {
    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}
};

TEST_F(OpenSSLTest, Sha256) {

  const string expected_hash =
      "8700be3b2fe64bd5f36be0b194f838c3aa475cbee660601f5acf19c99498d264";
  string hash;
  const bool is_success = hash_sha256(hash, _msg);

  EXPECT_TRUE(is_success);

  spdlog::debug("_msg={}", _msg);
  spdlog::debug("expected_hash={}", expected_hash);
  spdlog::debug("hash={}", hash);

  EXPECT_STREQ(hash.c_str(), expected_hash.c_str());
}

TEST_F(OpenSSLTest, Sha_512) {
  const string expected_hash =
      "6010c16c434b3af089c1bbf7891d0fd3987ee5cdaf9a6aebeb474bf3fd0b99aced57e9e3"
      "4be7744fd2c87ff25bce96ce54e13fd220d823b4a4f6994fdafdb947";
  string hash;
  const bool is_success = hash_sha512(hash, _msg);

  EXPECT_TRUE(is_success);

  spdlog::debug("_msg={}", _msg);
  spdlog::debug("expected_hash={}", expected_hash);
  spdlog::debug("hash={}", hash);

  EXPECT_STREQ(hash.c_str(), expected_hash.c_str());
}

TEST_F(OpenSSLTest, Blake2b_512) {
  const string expected_hash =
      "176a04d1dc47df7dae53078942e9828f80943978f5d11d9c6a2cd6d5c46ff5398b092abd"
      "7514c1d9596089de1f8b281e2ba8c4eb656011fd59284b8735abb295";
  string hash;
  const bool is_success = hash_blake2b512(hash, _msg);

  EXPECT_TRUE(is_success);

  spdlog::debug("_msg={}", _msg);
  spdlog::debug("expected_hash={}", expected_hash);
  spdlog::debug("hash={}", hash);

  EXPECT_STREQ(hash.c_str(), expected_hash.c_str());
}
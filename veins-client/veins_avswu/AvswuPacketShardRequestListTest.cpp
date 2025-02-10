#include <gtest/gtest.h>

#include "AvswuPacketShardRequestList.h"
#include <spdlog/spdlog.h>

using namespace avswu_packet_shard_request_list;

// fixture for setup and teardown
class PacketRequestListTest : public testing::Test {
protected:
  // variables
  uint64_t _from_id = 100;
  mpz_t _from_X;
  uint64_t _to_id = 200;
  mpz_t _to_X;

  // shard-based test variables
  const uint64_t _manufacturer_id = 100;
  const uint64_t _software_id = 1003;
  const uint64_t _version_id = 0;
  const size_t _cache_max_size = 1024;
  const uint32_t _k = 6;
  const uint32_t _m = 3;
  const vector<char> _shard_data = {'a', 'b', 'c'};

  void SetUp() override {

    mpz_init(_from_X);
    mpz_init(_to_X);

    // set debug level
    spdlog::set_level(spdlog::level::debug);
  }

  void TearDown() override {}

  void SetRequest(shard_request_t &request, const uint64_t manufacturer_id,
                  const uint64_t software_id, const uint64_t version_id,
                  const uint64_t shard_id) {
    request.manufacturer_id = manufacturer_id;
    request.software_id = software_id;
    request.version_id = version_id;
    request.shard_id = shard_id;
    request.shard_hash = "fake-shard-hash";
  }

  // add items to a packet
  void CreateSomeRequests(AvswuPacketShardRequestList &packet, const size_t n) {

    // add n items for each id type
    for (size_t m = 0; m < n; m++) {
      const uint64_t manufacturer_id = 10 + m;
      for (size_t s = 0; s < n; s++) {
        const uint64_t software_id = 100 + s;
        for (size_t v = 0; v < n; v++) {
          const uint64_t version_id = 1000 + v;
          for (size_t sh = 0; sh < n; sh++) {
            const uint64_t shard_id = 5000 + sh;
            shard_request_t request;
            SetRequest(request, manufacturer_id, software_id, version_id,
                       shard_id);
            packet.push_back(request);
          }
        }
      }
    }
  }
};

TEST_F(PacketRequestListTest, Create) {
  AvswuPacketShardRequestList p(_from_id, _from_X, _to_id, _to_X,
                                node_type_t::unknown);
  EXPECT_NE(&p, nullptr);
}

TEST_F(PacketRequestListTest, ToJson) {
  // packet->json str (from_json)
  AvswuPacketShardRequestList packet1(_from_id, _from_X, _to_id, _to_X,
                                      node_type_t::unknown);
  CreateSomeRequests(packet1, 3);
  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  EXPECT_TRUE(packet1_json_str.size() > 0);
}

TEST_F(PacketRequestListTest, FromJson) {
  // packet1->json str (from_json)
  AvswuPacketShardRequestList packet1(_from_id, _from_X, _to_id, _to_X,
                                      node_type_t::unknown);
  CreateSomeRequests(packet1, 2);

  json j1(packet1);
  string packet1_json_str = j1.dump(4);
  spdlog::debug("packet1_json_str={}", packet1_json_str);

  // packet2 <- json str (to_json)
  AvswuPacketShardRequestList packet2(j1);
  json j2(packet2);
  string packet2_json_str = j2.dump(4);
  spdlog::debug("packet2_json_str={}", packet2_json_str);

  EXPECT_TRUE(packet2_json_str == packet2_json_str);
}

TEST_F(PacketRequestListTest, RemoveASpecificShard) {
  // packet1->json str (from_json)
  AvswuPacketShardRequestList srql(_from_id, _from_X, _to_id, _to_X,
                                   node_type_t::unknown);
  CreateSomeRequests(srql, 4);

  // remove a specific request
  const uint64_t manufacturer_id = 12;
  const uint64_t software_id = 102;
  const uint64_t version_id = 1002;
  const uint64_t shard_id = 5002;

  auto size_before = srql.request_list().size();
  // remove request 3
  bool removed =
      srql.remove(manufacturer_id, software_id, version_id, shard_id);
  auto size_after = srql.request_list().size();

  EXPECT_TRUE(removed);
  EXPECT_EQ(size_before, size_after + 1);
}

TEST_F(PacketRequestListTest, RemoveAllShardsForASoftwareVersion) {
  // packet1->json str (from_json)
  AvswuPacketShardRequestList srql(_from_id, _from_X, _to_id, _to_X,
                                   node_type_t::unknown);
  CreateSomeRequests(srql, 10);

  // remove all request for a specfic version
  const uint64_t manufacturer_id = 14;
  const uint64_t software_id = 102;
  const uint64_t version_id = 1002;

  auto size_before = srql.request_list().size();
  // remove request 3
  bool removed = srql.remove(manufacturer_id, software_id, version_id);
  auto size_after = srql.request_list().size();

  EXPECT_TRUE(removed);
  EXPECT_TRUE(size_after < size_before);
}
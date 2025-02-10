#pragma once

#include "AvswuPacket.h"
#include "AvswuPacketFooter.h"
#include "AvswuPacketHeader.h"
#include "AvswuShard.h"
#include "AvswuUtils.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_shard;
using namespace avswu_packet;
using namespace avswu_packet_header;
using namespace avswu_packet_footer;
using namespace avswu_utils;
using json = nlohmann::json;

namespace avswu_packet_shard {

// payload
typedef struct {
  AvswuShard shard;
} payload_t;

// packet = [header:payload:footer]
// header and footer are defined in AvswuPacket base class
class AvswuPacketShard : public AvswuPacket {
private:
  payload_t _payload;

public:
  // constructors/destructor
  AvswuPacketShard(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
                   mpz_t &to_X, const uint64_t manufacturer_id,
                   const uint64_t software_id, const uint64_t version_id,
                   const uint64_t shard_id, const uint64_t erasure_id,
                   const uint32_t k, const uint32_t m, const uint32_t w,
                   const char *dataP, const size_t data_size,
                   const size_t original_file_size);
  AvswuPacketShard(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
                   mpz_t &to_X, const AvswuShard &shard);
  AvswuPacketShard();
  ~AvswuPacketShard();

  // set a shard
  void set(const uint64_t from_id, mpz_t &from_X, const uint64_t to_id,
           mpz_t &to_X, const uint64_t manufacturer_id,
           const uint64_t software_id, const uint64_t version_id,
           const uint64_t shard_id, const uint64_t erasure_id, const uint32_t k,
           const uint32_t m, const uint32_t w, const char *dataP,
           const size_t data_size, const size_t original_file_size);

  // header getters
  inline AvswuPacketHeader &header() { return _header; }
  inline AvswuPacketFooter &footer() { return _footer; }

  // getters for payload components
  inline const AvswuShard &shard() const { return _payload.shard; }

  // size in bytes
  const size_t size() const;

  // increase buffer size
  inline bool increase_buffer_size(size_t const scale) {
    return _payload.shard.increase_buffer_size(scale);
  };
  // decrease buffer size
  inline bool decrease_buffer_size(size_t const scale) {
    return _payload.shard.decrease_buffer_size(scale);
  };

  // create packet hash
  bool create_hash();

  // create json for the payload
  const json payload_to_json() const;

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketShard &p);
  friend void to_json(json &j, const AvswuPacketShard &p);
  friend void from_json(const json &j, AvswuPacketShard &p);
};

// smart-pointer to a shard
typedef shared_ptr<AvswuPacketShard> packet_shardP_t;

} // namespace avswu_packet_shard

#pragma once

#include "AvswuNotification.h"
#include "AvswuPacket.h"
#include "AvswuPacketFooter.h"
#include "AvswuPacketHeader.h"
#include "AvswuUtils.h"

#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_packet;
using namespace avswu_packet_header;
using namespace avswu_packet_footer;
using namespace avswu_utils;
using namespace avswu_notification;
using json = nlohmann::json;

namespace avswu_packet_shard_request_list {

// list of notifications sent by RSU to AVs
typedef struct {
  vector<shard_request_t> shard_request_list;
} payload_t;

// packet = [header:payload:footer]
class AvswuPacketShardRequestList : public AvswuPacket {
private:
  payload_t _payload;

public:
  // constructors/destructor
  AvswuPacketShardRequestList(const uint64_t from_id, mpz_t &from_X,
                              const uint64_t to_id, mpz_t &to_X,
                              node_type_t from_type);
  AvswuPacketShardRequestList();
  ~AvswuPacketShardRequestList();

  // getters for payload components
  inline const vector<shard_request_t> &request_list() const {
    return _payload.shard_request_list;
  }

  // add an item to inventory
  inline void push_back(shard_request_t &item) {
    _payload.shard_request_list.push_back(item);
  }

  // remove item from request list
  const bool remove(const uint64_t manufacturer_id, const uint64_t software_id,
                    const uint64_t version_id, const uint64_t shard_id);

  // remove all shard for a specific software update version from request list
  const bool remove(const uint64_t manufacturer_id, const uint64_t software_id,
                    const uint64_t version_id);

  // create packet hash
  bool create_hash();

  // create json for the payload
  const json payload_to_json() const;

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketShardRequestList &p);
  friend void to_json(json &j, const AvswuPacketShardRequestList &p);
  friend void from_json(const json &j, AvswuPacketShardRequestList &p);
};

} // namespace avswu_packet_shard_request_list

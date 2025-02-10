#pragma once

#include "AvswuGrpcClient.h"
#include "AvswuNotification.h"
#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_grpc_client;
using namespace avswu_notification;
using json = nlohmann::json;

// constants
#define READ_COMMITMENT_MAX_ATTEMPTS 10
#define READ_COMMITMENT_DELAY 1ms
#define WRITE_NOTIFICATION_MAX_ATTEMPTS 10
#define WRITE_NOTIFICATION_DELAY 1ms

namespace avswu_blockchain {

class AvswuBlockchain {

private:
  AvswuGrpcClient *_clientP;

public:
  // constructors/destructor
  AvswuBlockchain(AvswuGrpcClient *clientP);
  ~AvswuBlockchain();

  // funcs
  // av1.R(), av2.R(), agg.w(), agg.W(), t
  const bool write_commitment(uint64_t shard_id, uint32_t signer_id, mpz_t R,
                              uint64_t w, mpz_t W) const;
  // read commitment vector
  const bool read_commitment_vector(vector<commitment_t> &L_com,
                                    vector<commitment_map_key_t> key_vec) const;

  const bool write_notification(const AvswuNotification &notif) const;
  const bool read_last_notification(AvswuNotification &notif,
                                    const uint64_t manufacturer_id,
                                    const uint64_t software_id,
                                    const double start_time) const;
  const bool
  read_last_notification_list(deque<AvswuNotification> &notification_list,
                              const vector<uint64_t> manufacturer_list,
                              const uint64_t start_time,
                              const size_t max_notification);

  // clientP
  const inline AvswuGrpcClient *clientP() const { return _clientP; }

  // i/o and json
  friend ostream &operator<<(ostream &os, AvswuBlockchain &td);
  friend void to_json(json &j, const AvswuBlockchain &p);
  friend void from_json(const json &j, AvswuBlockchain &p);
};

} // namespace avswu_blockchain

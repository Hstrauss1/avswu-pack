#pragma once

#include "AvswuUtils.h"
#include "veins_avswu.grpc.pb.h"
#include "veins_avswu.pb.h"
#include <grpcpp/grpcpp.h>

using avswu::Avswu;
using avswu::AvswuBlockchainWriteCommitmentReply;
using avswu::AvswuBlockchainWriteCommitmentRequest;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace avswu_utils;

namespace avswu_grpc_client {

class AvswuGrpcClient {
public:
  AvswuGrpcClient(int argc, char **argv);
  ~AvswuGrpcClient();

  // blockchain read/write commitment
  const bool
  AvswuBlockchainWriteCommitment(string &serverResult, const uint64_t shard_id,
                                 const uint64_t signer_id, const string R_str,
                                 const string w_str, const string W_str);
  const bool AvswuBlockchainReadCommitmentVector(string &serverResult,
                                                 const string key_vec_json);

  // blockchain read/write notification
  const bool AvswuBlockchainWriteNotification(string &serverResult,
                                              const uint64_t manufacturer_id,
                                              const uint64_t software_id,
                                              const uint64_t version_id,
                                              const uint64_t create_time_ns,
                                              const string notification_json);

  const bool AvswuBlockchainReadLastNotification(string &serverResult,
                                                 const uint64_t manufacturer_id,
                                                 const uint64_t software_id,
                                                 const uint64_t start_time_ns);

  const bool AvswuBlockchainReadLastNotificationList(
      string &serverResult, const vector<uint64_t> manufacturer_list,
      const uint64_t start_time);

private:
  unique_ptr<Avswu::Stub> stub_;
};
} // namespace avswu_grpc_client
#include "AvswuGrpcClient.h"
#include "AvswuUtils.h"
#include <spdlog/spdlog.h>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

const string endpoint =
    string(AVSWU_GRPC_SERVER_IP) + ":" + string(AVSWU_GRPC_SERVER_PORT);
ABSL_FLAG(string, target, endpoint, "Server address");

namespace avswu_grpc_client {

using namespace avswu;

// create the connection to the server
AvswuGrpcClient::AvswuGrpcClient(int argc, char **argv) : stub_(nullptr) {
  // standard gRPC set up
  absl::ParseCommandLine(argc, argv);
  spdlog::info("connecting to target={}", absl::GetFlag(FLAGS_target));
  try {
    auto channel = grpc::CreateChannel(absl::GetFlag(FLAGS_target),
                                       grpc::InsecureChannelCredentials());
    stub_ = Avswu::NewStub(channel);
  } catch (char *excp) {
    spdlog::error("Unable to connect to grpc server. {}", excp);
    exit(-1);
  }
}

AvswuGrpcClient::~AvswuGrpcClient() {}

// assembles client's payload, sends it and presents the response back from
// the server
const bool AvswuGrpcClient::AvswuBlockchainWriteCommitment(
    string &serverResult, const uint64_t shard_id, const uint64_t signer_id,
    const string R_str, const string w_str, const string W_str) {
  // build the request
  AvswuBlockchainWriteCommitmentRequest request;

  // args
  request.set_shard_id(shard_id);
  request.set_signer_id(signer_id);
  request.set_capital_r(R_str);
  request.set_w(w_str);
  request.set_capital_w(W_str);

  // reply
  AvswuBlockchainWriteCommitmentReply reply;
  ClientContext context;

  // make the RPC call
  Status result =
      stub_->AvswuBlockchainWriteCommitment(&context, request, &reply);

  // handle the return result
  if (result.ok()) {
    // check if read was successful (read returns a vector, or an error)
    // spdlog::debug("reply.result()={}", reply.result());
    try {
      json j_result = json::parse(reply.result());
      // if result is an error
      if (j_result.contains("Err")) {
        serverResult = reply.result();
        return false;
      } else {
        // it is a success
        serverResult = reply.result();
        return true;
      }
    } catch (const json::type_error &e) {
      serverResult = e.what();
      return false;
    }
  } else {
    spdlog::error(
        "gRPC call to server failed. error_code={} error_message()={}",
        result.error_code(), result.error_message());
    json errorJson = {{"error_code", result.error_code()},
                      {"error_message", result.error_message()}};
    serverResult = errorJson.dump();
    return false;
  }
}

// read commitment vector package data, and send to server
const bool AvswuGrpcClient::AvswuBlockchainReadCommitmentVector(
    string &serverResult, const string key_vec_json) {
  // build the request
  AvswuBlockchainReadCommitmentVectorRequest request;

  // args
  request.set_key_vec_json(key_vec_json);

  // reply
  AvswuBlockchainReadCommitmentVectorReply reply;
  ClientContext context;

  // make the RPC call
  Status result =
      stub_->AvswuBlockchainReadCommitmentVector(&context, request, &reply);

  // handle the return result
  if (result.ok()) {
    // check if read was successful (read returns a vector, or an error)
    // spdlog::warn("reply.result()={}", reply.result());
    try {
      json j_result = json::parse(reply.result());
      // if result is an error
      if (j_result.contains("Err")) {
        serverResult = reply.result();
        return false;
      } else {
        // it is a success
        serverResult = reply.result();
        return true;
      }
    } catch (const json::type_error &e) {
      serverResult = e.what();
      return false;
    }
  } else {
    spdlog::error(
        "gRPC call to server failed. error_code={} error_message()={}",
        result.error_code(), result.error_message());
    json errorJson = {{"error_code", result.error_code()},
                      {"error_message", result.error_message()}};
    serverResult = errorJson.dump();
    return false;
  }
}

const bool AvswuGrpcClient::AvswuBlockchainWriteNotification(
    string &serverResult, const uint64_t manufacturer_id,
    const uint64_t software_id, const uint64_t version_id,
    const uint64_t create_time_ns, const string notification_json) {
  // build the request
  AvswuBlockchainWriteNotificationRequest request;

  // args
  request.set_manufacturer_id(manufacturer_id);
  request.set_software_id(software_id);
  request.set_version_id(version_id);
  request.set_create_time(create_time_ns);
  request.set_notification_json(notification_json);

  // reply
  AvswuBlockchainWriteNotificationReply reply;
  ClientContext context;

  // make the RPC call
  Status result =
      stub_->AvswuBlockchainWriteNotification(&context, request, &reply);

  // handle the return result
  if (result.ok()) {
    // check if write was successful
    try {
      json j_result = json::parse(reply.result());
      if (j_result.contains("successType")) {
        string successType = j_result.at("successType");
        if (successType == "WriteNotification") {
          serverResult = reply.result();
          return true;
        }
      }
      // return the error
      serverResult = reply.result();
      return false;
    } catch (const json::type_error &e) {
      serverResult = e.what();
      return false;
    }
  } else {
    spdlog::error(
        "gRPC call to server failed. error_code={} error_message()={}",
        result.error_code(), result.error_message());
    json errorJson = {{"error_code", result.error_code()},
                      {"error_message", result.error_message()}};
    serverResult = errorJson.dump();
    return false;
  }
}

const bool AvswuGrpcClient::AvswuBlockchainReadLastNotification(
    string &serverResult, const uint64_t manufacturer_id,
    const uint64_t software_id, const uint64_t start_time_ns) {
  // build the request
  AvswuBlockchainReadLastNotificationRequest request;

  // args
  request.set_manufacturer_id(manufacturer_id);
  request.set_software_id(software_id);
  request.set_start_time(start_time_ns);

  // reply
  AvswuBlockchainReadLastNotificationReply reply;
  ClientContext context;

  // make the RPC call
  Status result =
      stub_->AvswuBlockchainReadLastNotification(&context, request, &reply);

  // handle the return result
  if (result.ok()) {
    // return the result
    serverResult = reply.result();
    return true;
  } else {
    spdlog::error(
        "gRPC call to server failed. error_code={} error_message()={}",
        result.error_code(), result.error_message());
    json errorJson = {{"error_code", result.error_code()},
                      {"error_message", result.error_message()}};
    serverResult = errorJson.dump();
    return false;
  }
}

const bool AvswuGrpcClient::AvswuBlockchainReadLastNotificationList(
    string &serverResult, const vector<uint64_t> manufacturer_list,
    const uint64_t start_time) {

  // build the request
  AvswuBlockchainReadLastNotificationListRequest request;

  // get json for manufacturer list
  json j_manufacturer_list = json(manufacturer_list);
  const string manufacturer_list_json = j_manufacturer_list.dump();
  spdlog::debug("manufacturer_list_json={}", manufacturer_list_json);

  // args
  request.set_manufacturer_list_json(manufacturer_list_json);
  request.set_start_time(start_time);

  // reply
  AvswuBlockchainReadLastNotificationListReply reply;
  ClientContext context;

  // make the RPC call
  Status result =
      stub_->AvswuBlockchainReadLastNotificationList(&context, request, &reply);

  // handle the return result
  if (result.ok()) {
    // return the result
    serverResult = reply.result();
    return true;
  } else {
    spdlog::error(
        "gRPC call to server failed. error_code={} error_message()={}",
        result.error_code(), result.error_message());
    json errorJson = {{"error_code", result.error_code()},
                      {"error_message", result.error_message()}};
    serverResult = errorJson.dump();
    return false;
  }
}

} // namespace avswu_grpc_client
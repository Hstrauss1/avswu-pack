#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "veins_avswu.pb.h"

#include <grpcpp/grpcpp.h>

#include "veins_avswu.grpc.pb.h"

// send grpc calls to avswu server, using docker run mapped port for grpcs calls
ABSL_FLAG(std::string, target, "10.5.0.2:58851", "Server address");
// polkadot-test server, using docker run mapped port for grpcs calls
// ABSL_FLAG(std::string, target, "10.5.0.2:58951", "Server address");

using avswu::Avswu;
using avswu::AvswuPushEventReply;
using avswu::AvswuPushEventRequest;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class AvswuClient {
public:
  AvswuClient(std::shared_ptr<Channel> channel)
      : stub_(Avswu::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server
  /*
 std::int32_t &id, std::string &action, std::string &signature, std::uint64_t
 &t, std::int32_t &t_sim_time_unit
  */
  std::string AvswuPushEvent(const std::int32_t &id, const std::string &action,
                             const std::string &signature,
                             const std::uint64_t &t,
                             const std::int32_t &t_sim_time_unit) {
    // build the request
    AvswuPushEventRequest request;
    request.set_id(id);
    request.set_action(action);
    request.set_signature(signature);
    request.set_t(t);
    request.set_t_sim_time_unit(t_sim_time_unit);

    // get the reply
    AvswuPushEventReply reply;
    ClientContext context;

    // make the RPC call
    Status result = stub_->AvswuPushEvent(&context, request, &reply);
    if (result.ok()) {
      return reply.result();
    } else {
      std::cout << result.error_code() << ": " << result.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

private:
  std::unique_ptr<Avswu::Stub> stub_;
};

int main(int argc, char **argv) {
  // standard gRPC set up
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);
  AvswuClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  /* avswu event
   * avswuEvent { id: uint32, action: string, signature: string, t: uint64,
   * t_sim_time_unit: int32_t } -> result
   */

  std::int32_t id(23);
  std::string action("test-action");
  std::string signature("test-signature");
  std::uint64_t t(1002);
  std::int32_t t_sim_time_unit(-3);
  std::cout << "client sent: "
            << "{ id=" << id << ", action=" << action
            << ", signature=" << signature << ", t=" << t
            << ", t_sim_time_unit=" << t_sim_time_unit << "}" << std::endl;
  std::string result =
      client.AvswuPushEvent(id, action, signature, t, t_sim_time_unit);
  std::cout << "client received: " << result << std::endl;

  return 0;
}
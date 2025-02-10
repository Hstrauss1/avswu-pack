#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "veins_avswu.pb.h"

#include <grpcpp/grpcpp.h>

#include "veins_avswu.grpc.pb.h"

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// send grpc calls to avswu server, using docker run mapped port for grpcs calls
ABSL_FLAG(std::string, target, "10.5.0.2:58851", "Server address");
// polkadot-test server, using docker run mapped port for grpcs calls
// ABSL_FLAG(std::string, target, "10.5.0.2:58951", "Server address");

using avswu::Avswu;
using avswu::AvswuGetEventListReply;
using avswu::AvswuGetEventListRequest;
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
  std::string AvswuGetEventList(const std::int32_t &id) {
    // build the request
    AvswuGetEventListRequest request;
    request.set_id(id);

    // get the reply
    AvswuGetEventListReply reply;
    ClientContext context;

    // make the RPC call
    Status result = stub_->AvswuGetEventList(&context, request, &reply);
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
  std::cout << "client sent: "
            << "{ id=" << id << "}" << std::endl;
  std::string resultJson = client.AvswuGetEventList(id);
  std::cout << "client received: " << resultJson << std::endl;

  // parse the json string into a struct
  json ex1 = json::parse(resultJson);
  std::cout << std::setw(4) << ex1 << std::endl;

  return 0;
}
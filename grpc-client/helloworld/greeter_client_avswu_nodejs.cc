/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

// send grpc calls to avswu server, using docker run mapped port for grpcs calls
// ABSL_FLAG(std::string, target, "10.5.0.2:58851", "Server address");
// polkadot-test server, using docker run mapped port for grpcs calls
ABSL_FLAG(std::string, target, "10.5.0.2:58951", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::ClientReader;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient {
public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string SayHello(const std::string &world) {
    // Data we are sending to the server.
    HelloRequest request;
    request.set_name(world);

    // Container for the data we expect from the server.
    HelloReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_->SayHello(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  std::string SayHelloAgain(const std::string &universe) {
    // Follows the same pattern as SayHello.
    HelloRequest request;
    request.set_name(universe);
    HelloReply reply;
    ClientContext context;

    // Here we can use the stub's newly available method we just added.
    Status status = stub_->SayHelloAgain(&context, request, &reply);
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

  // get stream reply from server
  void SayHelloStreamReply(const std::string &city) {
    // Follows the same pattern as SayHello.
    HelloRequest request;
    request.set_name(city);
    HelloReply reply;
    ClientContext context;

    // create the reader
    std::unique_ptr<ClientReader<HelloReply>> reader(
        stub_->SayHelloStreamReply(&context, request));

    // read results until the streaming server stops sending messages
    while (reader->Read(&reply)) {
      std::cout << reply.message() << std::endl;
    }
    Status status = reader->Finish();
    if (status.ok()) {
      std::cout << "SayHelloStreamReply rpc succeeded." << std::endl;
    } else {
      std::cout << "SayHelloStreamReply rpc failed." << std::endl;
    }
  }

private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char **argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual
  // RPCs are created. This channel models a connection to an endpoint
  // specified by the argument "--target=" which is the only expected
  // argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // method 1
  std::string world("world");
  std::cout << "Greeter client sent: " << world << std::endl;
  std::string reply = greeter.SayHello(world);
  std::cout << "Greeter client received: " << reply << std::endl;

  // method 2
  std::string universe("universe");
  std::cout << "Greeter client sent: " << universe << std::endl;
  reply = greeter.SayHelloAgain(universe);
  std::cout << "Greeter client received: " << reply << std::endl;

  return 0;
}
cmake_minimum_required(VERSION 3.8)

project(VeinsEvent C CXX)

include(../cmake/common.cmake)

# add -fPIC for all compiles
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fPIC")

# use clang++
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-17")

# Proto file
get_filename_component(veins_avswu_proto "./veins_avswu.proto" ABSOLUTE)
get_filename_component(veins_avswu_proto_path "${veins_avswu_proto}" PATH)

# Generated sources
set(veins_avswu_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/veins_avswu.pb.cc")
set(veins_avswu_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/veins_avswu.pb.h")
set(veins_avswu_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/veins_avswu.grpc.pb.cc")
set(veins_avswu_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/veins_avswu.grpc.pb.h")
add_custom_command(
      OUTPUT "${veins_avswu_proto_srcs}" "${veins_avswu_proto_hdrs}" "${veins_avswu_grpc_srcs}" "${veins_avswu_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${veins_avswu_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${veins_avswu_proto}"
      DEPENDS "${veins_avswu_proto}")

# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# veins_avswu_grpc_proto
add_library(veins_avswu_grpc_proto
  ${veins_avswu_grpc_srcs}
  ${veins_avswu_grpc_hdrs}
  ${veins_avswu_proto_srcs}
  ${veins_avswu_proto_hdrs})
target_link_libraries(veins_avswu_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF})

# Targets
foreach(_exe
  veins_client_push veins_client_get)
  add_executable(${_exe} "${_exe}.cc")
  target_link_libraries(${_exe}
    veins_avswu_grpc_proto
    absl::flags
    absl::flags_parse
    ${_REFLECTION}
    ${_GRPC_GRPCPP}
    ${_PROTOBUF_LIBPROTOBUF})
endforeach()

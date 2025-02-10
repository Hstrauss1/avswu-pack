###############################################################
# project configs
###############################################################
project(update-client)

# verbosity on
# set(CMAKE_VERBOSE_MAKEFILE ON)

# specifify cmake version
cmake_minimum_required(VERSION 3.18.4)

# create compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# erasure flags
set(ERASURE_FLAGS "-pedantic-errors -Wall -Wextra -Werror -Wno-long-long")

# libipfs flags
set(IPFS_FLAGS "-pthread -Wl,--no-as-needed -ldl -lresolv")

# grpc cmake settings
include(./common.cmake)
# reset to use clang++
set(CMAKE_CXX_COMPILER "/usr/bin/clang++-17")

# use hostname to determine container versus vm virtualbox Makefile settings
cmake_host_system_information(RESULT _hostname QUERY HOSTNAME)
# message("_hostname=" ${_hostname})
if (${_hostname} STREQUAL "instant-veins")
  message("creating Makefile for instant-veins debian virtualbox environment")
  set(_instant_veins 1)
else()
  message("creating Makefile for avswu ubuntu container environment")
  set(_instant_veins 0)
endif()

# compile OPTIMIZED
message("compiling with optimization flags")
set(OPTIMIZATION_FLAGS "-O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OPTIMIZATION_FLAGS}")

# # compile for DEBUG
# message("compiling with debug flags")
# # debug flags, turn off for performance runs
# set(DEBUG_FLAGS "-g -fno-limit-debug-info")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DEBUG_FLAGS}"

# add -fPIC for all compiles
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -Wall")

# set project basedir
if (${_instant_veins} EQUAL 1)
  # set project basedir
  set(_basedir "/home/veins/avswu")
else()
  # set project basedir
  set(_basedir "/home/gsolomon/avswu")
endif()

# project directories
set(_project_dir "${_basedir}/veins-client")
set(AVSWU_CPP_DIR "${_project_dir}/veins_avswu")

# allows changing ip address for gRPC server in cpp file via #ifdef
if (${_instant_veins} EQUAL 1)
  add_definitions(-D_HOSTNAME_INSTANT_VEINS=${_hostname})
endif()

# output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${_project_dir}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${_project_dir}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${_project_dir}/bin)

#################################################################
# executables
#################################################################
set(_target_exe_list
  test_non_con_musig
  test_non_con_musig_client
  # test_com_read_write
  # test_encode
  # test_decode
  test_shard
  # test_shard_cache
  # encoder
  # decoder
  create_config
  create_input_test_data
  test_ipfs1
)

#################################################################
# unit tests
#################################################################
set(_target_exe_unit_test_list
  # HelloTest
  # AvswuManufacturerNodeTest
  # AvswuShardTest
  # AvswuOpenSSLTest
  # AvswuNotificationTest
  # AvswuPacketTest
  AvswuPacketHeaderTest
  AvswuPacketStatisticsTest
  # AvswuPacketFooterTest
  AvswuPacketShardTest
  # AvswuPacketInventoryTest
  # AvswuPacketNotificationListTest
  # AvswuPacketShardRequestListTest
  # AvswuPacketDeliveryRequestTest
  # AvswuPacketSupportListTest
  AvswuConfigTest
)

#################################################################
# avswu library files
#################################################################
set(_lib_file_basename_list 
AvswuShard
AvswuShardCache
AvswuShardCacheMap
AvswuShardCacheSetMap
AvswuNotification
AvswuBlockchain
AvswuManufacturerNode
AvswuRSUNode
AvswuAVNode
AvswuNode
AvswuGrpcClient
AvswuUtils
AvswuUtilsVector
AvswuOpenSSL
NonConMusigNode
NonConMusigSigner
NonConMusigAggregator
AvswuPacket
AvswuPacketHeader
AvswuPacketStatistics
AvswuPacketFooter
AvswuPacketShard
AvswuPacketInventory
AvswuPacketNotificationList
AvswuPacketShardRequestList
AvswuPacketDeliveryRequest
AvswuPacketSupportList
AvswuConfig
)

#################################################################
# generate and compile gRPC files
#################################################################

# proto file
get_filename_component(veins_avswu_proto "${_basedir}/veins-server/veins_avswu.proto" ABSOLUTE)
get_filename_component(veins_avswu_proto_path "${veins_avswu_proto}" PATH)

# generated sources
set(veins_avswu_proto_srcs "${AVSWU_CPP_DIR}/veins_avswu.pb.cc")
set(veins_avswu_proto_hdrs "${AVSWU_CPP_DIR}/veins_avswu.pb.h")
set(veins_avswu_grpc_srcs "${AVSWU_CPP_DIR}/veins_avswu.grpc.pb.cc")
set(veins_avswu_grpc_hdrs "${AVSWU_CPP_DIR}/veins_avswu.grpc.pb.h")
add_custom_command(
      OUTPUT "${veins_avswu_proto_srcs}" "${veins_avswu_proto_hdrs}" "${veins_avswu_grpc_srcs}" "${veins_avswu_grpc_hdrs}"
      COMMAND ${_PROTOBUF_PROTOC}
      ARGS --grpc_out "${AVSWU_CPP_DIR}"
        --cpp_out "${AVSWU_CPP_DIR}"
        -I "${veins_avswu_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${veins_avswu_proto}"
      DEPENDS "${veins_avswu_proto}")

# include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# veins_avswu_grpc_proto
add_library(veins_avswu_grpc_proto
  ${veins_avswu_grpc_srcs}
  ${veins_avswu_grpc_hdrs}
  ${veins_avswu_proto_srcs}
  ${veins_avswu_proto_hdrs}
)
target_link_libraries(veins_avswu_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  absl::flags
  absl::flags_parse
)

###############################################################
# google test framework
###############################################################
enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)

###############################################################
# find libraries
###############################################################
find_package(nlohmann_json 3.2 REQUIRED)
find_package(spdlog REQUIRED)
# use open ssl 3.3.0
include_directories(/usr/local/openssl-3.3.0/include)
find_package(OpenSSL 3.3.0 EXACT REQUIRED PATHS /usr/local/openssl-3.3.0)

###############################################################
# create avswu library so Avswu*.cpp files dont have to be re-compilied
###############################################################


# build library
set(_lib_file_cpp_list "")
foreach(_lib_file ${_lib_file_basename_list})
  set(_lib_file_list ${_lib_file_list} "${AVSWU_CPP_DIR}/${_lib_file}.cpp")
endforeach()

set(_target "veins_avswu")
add_library(${_target} STATIC ${_lib_file_list})

target_include_directories(${_target} PUBLIC ${NON_CON_MUSIG_CPP_DIR})
target_include_directories(${_target} PUBLIC /usr/local/Jerasure/include)

# jerasure
target_link_libraries(${_target} PRIVATE /usr/local/gf-complete/src/.libs/libgf_complete.a)
target_link_libraries(${_target} PRIVATE /usr/local/Jerasure/src/libtiming.a)
target_link_libraries(${_target} PRIVATE /usr/local/lib/libJerasure.so)

# libipfs library
target_include_directories(${_target} PUBLIC /usr/local/libipfs/bin)
target_link_libraries(${_target} PRIVATE /usr/local/libipfs/bin/libipfs-linux.a)

target_link_libraries(${_target} PRIVATE
  veins_avswu_grpc_proto
  ${_REFLECTION}
  ${_GRPC_GRPCPP}
  ${_PROTOBUF_LIBPROTOBUF}
  absl::flags
  absl::flags_parse
)

###############################################################
# build targets
###############################################################


# targets
set(_all_target_list ${_target_exe_list} ${_target_exe_unit_test_list})
foreach(_target ${_all_target_list})

  # set target file extension
  if (${_target} STREQUAL "encoder" OR ${_target} STREQUAL "decoder")
    set(_target_ext "c")
  # default
  else()
    set(_target_ext "cpp")
  endif()

  # if unit tests, add to google test
  STRING(FIND ${_target} "Test" _position)
  if (${_position} EQUAL -1)
    # non-test executables
    set(_target_file ${_target}.${_target_ext})
    message("executable _target_file=${_target_file}")
    add_executable(${_target} ${_target_file})
  else()
    # test executables
    set(_target_file "veins_avswu/${_target}.${_target_ext}")
    message("executable _target_file=${_target_file}")
    add_executable(${_target} ${_target_file})
    target_link_libraries(${_target} PUBLIC GTest::gtest_main)
    gtest_add_tests(${_target} "" AUTO) 
  endif()

  # includes
  target_include_directories(${_target} PUBLIC /usr/local/include)
  target_include_directories(${_target} PUBLIC ${AVSWU_CPP_DIR})
  target_include_directories(${_target} PUBLIC ${NON_CON_MUSIG_CPP_DIR})
  target_include_directories(${_target} PUBLIC /usr/local/Jerasure/include)

  # libraries
  target_link_libraries(${_target} PRIVATE nlohmann_json::nlohmann_json)
  target_link_libraries(${_target} PRIVATE spdlog::spdlog)

  target_link_libraries(${_target} PRIVATE veins_avswu_grpc_proto)
  target_link_libraries(${_target} PRIVATE absl::flags)
  target_link_libraries(${_target} PRIVATE absl::flags_parse)
  target_link_libraries(${_target} PRIVATE ${_GRPC_GRPCPP})
  target_link_libraries(${_target} PRIVATE ${_PROTOBUF_LIBPROTOBUF})

  target_link_libraries(${_target} PRIVATE gmp)
  target_link_libraries(${_target} PRIVATE OpenSSL::SSL)

  target_link_libraries(${_target} PRIVATE veins_avswu)

  # jerasure and gf-complete
  target_link_libraries(${_target} PRIVATE /usr/local/gf-complete/src/.libs/libgf_complete.a)
  target_link_libraries(${_target} PRIVATE /usr/local/Jerasure/src/libtiming.a)
  target_link_libraries(${_target} PRIVATE /usr/local/lib/libJerasure.so)

  # ssl 3.3.0 dynamic libraries
  target_link_libraries(${_target} PRIVATE /usr/local/openssl-3.3.0/libcrypto.so.3)

  # libipfs library
  target_include_directories(${_target} PUBLIC /usr/local/libipfs/bin)
  target_link_libraries(${_target} PRIVATE /usr/local/libipfs/bin/libipfs-linux.a)

  # link directories
  target_link_directories(${_target} PUBLIC /usr/local/lib)

endforeach()


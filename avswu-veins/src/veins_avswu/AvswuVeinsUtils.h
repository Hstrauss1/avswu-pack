#pragma once

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>
#include <string>

#include "AvswuConfig.h"
#include "AvswuGrpcClient.h"
#include "AvswuPacketHeader.h"
#include "omnetpp/csimulation.h"

/*
label for nodes are [to_id:summary:packet:from_id] with each can be turned
off/on using defines below

summary indicates, "" = no shards, "S" = shard received, "U" = software update
received
*/
// update node ui info, or not
#define AVSWU_NODE_UI_LABEL_TO_ID false
#define AVSWU_NODE_UI_LABEL_SUMMARY false
#define AVSWU_NODE_UI_LABEL_PACKET false
#define AVSWU_NODE_UI_LABEL_FROM_ID false
#define AVSWU_NODE_UI_LABEL_OUT_OF_RANGE false
#define AVSWU_NODE_UI_LABEL_DECODE_SUCCESS false

// turn on node color transitions
#define AVSWU_NODE_UI_COLOR true

using namespace std;
using json = nlohmann::json;

using namespace avswu_packet_header;

// only C++ spdlog, prepends [avswu] so it can be grepped
#define AVSWU_SPDLOG_INFO(msg)                                                 \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    spdlog::info(_log_msg);                                                    \
  }

#define AVSWU_SPDLOG_WARN(msg)                                                 \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    spdlog::warn(_log_msg);                                                    \
  }

#define AVSWU_SPDLOG_ERROR(msg)                                                \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    spdlog::error(_log_msg);                                                   \
  }

#define AVSWU_SPDLOG_DEBUG(msg)                                                \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    spdlog::debug(_log_msg);                                                   \
  }

// log utils for EV prepends [avswu] so it can be grepped
#define AVSWU_EVLOG_INFO(msg)                                                  \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_INFO << _log_msg << endl;                                               \
  }

#define AVSWU_EVLOG_ERROR(msg)                                                 \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_ERROR << _log_msg << endl;                                              \
  }

#define AVSWU_EVLOG_WARN(msg)                                                  \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_WARN << _log_msg << endl;                                               \
  }

#define AVSWU_EVLOG_DEBUG(msg)                                                 \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_DEBUG << _log_msg << endl;                                              \
  }

// log utils logs to both veins EV and C++ spdlog, prepends [avswu] so it
// can be grepped
#define AVSWU_LOG_INFO(msg)                                                    \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_INFO << _log_msg << endl;                                               \
    spdlog::info(_log_msg);                                                    \
  }

#define AVSWU_LOG_WARN(msg)                                                    \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_WARN << _log_msg << endl;                                               \
    spdlog::warn(_log_msg);                                                    \
  }

#define AVSWU_LOG_ERROR(msg)                                                   \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_ERROR << _log_msg << endl;                                              \
    spdlog::error(_log_msg);                                                   \
  }

#define AVSWU_LOG_DEBUG(msg)                                                   \
  {                                                                            \
    const string _log_msg =                                                    \
        "[avswu " + log_sim_time() + "] " + log_prefix() + msg;                \
    EV_DEBUG << _log_msg << endl;                                              \
    spdlog::debug(_log_msg);                                                   \
  }

using namespace avswu_config;
using namespace avswu_grpc_client;

namespace avswu_veins_utils {

// log prefix for debugging
inline const string log_prefix() { return "AvswuVeinsUtils: "; };

// config funcs
const bool read_avswu_config();
// gRPC funcs
const bool create_grpc_client();

// getters/setters for global variables in .cc file
AvswuConfig *get_avswu_configP();
const bool get_read_avswu_config();
void set_read_avswu_config(const bool val);
const bool get_create_grpc_client();
AvswuGrpcClient *get_grpc_clientP();

// compare ids,return true if a_id==b_id a_X==b_X
const bool compare_id(uint64_t a_id, mpz_t &a_X, uint64_t b_id, mpz_t &b_X);

// format time
inline const string log_sim_time() {
  const auto t = omnetpp::simTime().dbl();
  const string s = fmt::format("t={:.2f}", t);
  return s;
}

// log from type
inline const string avswu_from_id(const AvswuPacketHeader &header) {
  const string log_from_id = "from_id=" + to_string(header.from_id());
  const string log_from_type = "from_type=" + header.from_type_to_string();
  const string log_msg = "(" + log_from_id + ", " + log_from_type + ")";
  return log_msg;
}

// ipfs
void set_ipfs_start(const bool val);
const bool ipfs_start();

} // namespace avswu_veins_utils
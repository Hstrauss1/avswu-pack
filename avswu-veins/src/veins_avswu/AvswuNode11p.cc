#include "AvswuNode11p.h"

#include "AvswuVeinsUtils.h"
#include <spdlog/spdlog.h>

#include "AvswuAV11pMessage_m.h"

using namespace veins;
using namespace std;
using namespace avswu_veins_utils;

Define_Module(veins::AvswuNode11p);

// send a packet
const bool AvswuNode11p::send_packet_delayed(const json &packet_json,
                                             const string node_type,
                                             const double delay) {
  try {
    const string packet = packet_json.dump();
    AvswuAV11pMessage *wsm = new AvswuAV11pMessage();
    populateWSM(wsm);
    wsm->setSenderId(myId);
    wsm->setPayload(packet.c_str());
    wsm->setSenderType(node_type.c_str());
    sendDelayedDown(wsm, delay);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to send packet, exception e=" + string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }
  return true;
}

// schedule a packet
const bool AvswuNode11p::schedule_packet(const json &packet_json,
                                         const string node_type,
                                         const double schedule_time) {
  try {
    const string packet = packet_json.dump();
    AvswuAV11pMessage *wsm = new AvswuAV11pMessage();
    populateWSM(wsm);
    wsm->setSenderId(myId);
    wsm->setPayload(packet.c_str());
    wsm->setSenderType(node_type.c_str());
    // scheduleAt() calls handleSelfMsg() in base class
    scheduleAt(schedule_time, wsm);
  } catch (const std::runtime_error &e) {
    const string log_msg =
        "unable to send packet, exception e=" + string(e.what());
    AVSWU_LOG_ERROR(log_msg);
    return false;
  }
  return true;
}

// increase data size
void AvswuNode11p::increase_data_size(vector<char> &data, size_t const scale) {
  size_t const orig_data_size = data.size();
  size_t const new_size = orig_data_size * scale;
  data.resize(new_size);

  for (size_t i = orig_data_size; i < new_size; i++) {
    // beef pattern is our poison
    if (i % 4 == 0) {
      data[i] = 0xb;
    }
    if (i % 4 == 1) {
      data[i] = 0xe;
    }
    if (i % 4 == 2) {
      data[i] = 0xe;
    }
    if (i % 4 == 3) {
      data[i] = 0xf;
    }
  }
}

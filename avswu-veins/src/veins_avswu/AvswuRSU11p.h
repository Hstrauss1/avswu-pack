#pragma once

#include "AvswuNode11p.h"
#include "AvswuPacketSupportList.h"
#include "AvswuRSUNode.h"

using namespace avswu_rsu_node;
using namespace avswu_packet_support_list;

// support list message period and start time
#define RSU_SUPPORT_LIST_START 30
#define RSU_SUPPORT_LIST_PERIOD 5

namespace veins {

class VEINS_API AvswuRSU11p : public AvswuNode11p {
private:
  // variables
  AvswuRSUNode *_rsuP = nullptr;
  // manufacturers that this RSU supports
  AvswuPacketSupportList _supported_manufacturer;

  // init methods
  const bool initialize_stage1();
  const bool initialize_rsu(const uint64_t rsu_id, const size_t cache_max_size);

  // create support list packet string
  const string create_support_list_packet();
  // parse and handle the payload
  const bool handle_payload(const string payload);
  // handle inventory from client-av
  const bool handle_inventory(const string payload);
  // handle delivery request from delivery-av
  const bool handle_delivery_request(const string payload);
  // handle shard request list
  const bool handle_shard_request_list(const string payload);

  // initialize supported manufacturer list
  void initialize_supported_manufacturer_list();

  // check if this rsu, is the recipient (to in packet header)
  const bool is_recipient(AvswuPacket *packetP) const;

protected:
  void onWSM(BaseFrame1609_4 *wsm) override;
  void onWSA(DemoServiceAdvertisment *wsa) override;

  // initiates repetitive broadcast of support-list via self-message
  void handleSelfMsg(cMessage *msg) override;

public:
  // initialization
  int numInitStages() const override;
  void initialize(int stage) override;
  // destructor for memory cleanup
  ~AvswuRSU11p();

  // log prefix for debugging
  inline const string log_prefix() {
    if (_rsuP) {
      return "rsu_id=" + to_string(_rsuP->id()) + ", ";
    } else {
      return "_rsuP=null, ";
    }
  }
};

} // namespace veins

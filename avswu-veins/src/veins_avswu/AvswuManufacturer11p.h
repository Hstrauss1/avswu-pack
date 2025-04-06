#pragma once

#include "AvswuConfig.h"
#include "AvswuManufacturerNode.h"
#include "AvswuNode11p.h"

using namespace avswu_manufacturer_node;
using namespace avswu_config;

// every n seconds send a new software update
#define NEW_SOFTWARE_UPDATE_START 0
#define NEW_SOFTWARE_UPDATE_PERIOD 5000

namespace veins {

class VEINS_API AvswuManufacturer11p : public AvswuNode11p {
private:
  // variables
  AvswuManufacturerNode *_manufacturerP = nullptr;

  // last time software update was sent
  double _next_software_update = NEW_SOFTWARE_UPDATE_PERIOD;

  // methods
  // set log level at runtime
  void set_spdlog_level();

  const bool initialize_stage1();
  const bool initialize_manufacturer(const uint64_t manufacturer_id,
                                     const size_t cache_max_size);

  // create packets
  const vector<AvswuPacketShard>
  create_packet_list(const uint64_t software_id, const uint64_t version_id,
                     const vector<char> input_data, const uint32_t k,
                     const uint32_t m);
  // write packets to ipfs
  const vector<string> ipfs_write_packet_list(const vector<AvswuPacketShard>);
  // write notification to blockchain
  const bool write_notification(const vector<AvswuPacketShard> man_packet_list,
                                const vector<string> cid_hash_list,
                                const uint64_t software_id,
                                const uint64_t version_id,
                                const vector<char> input_data, const uint32_t k,
                                const uint32_t m, const double create_time);

  const bool send_new_software_update();

  // initiates repetitive self message for new versions of software
  void handleSelfMsg(cMessage *msg) override;
  void handlePositionUpdate(cObject *obj) override;

  // size of the software update
  uint32_t erasure_code_k;
  uint32_t erasure_code_m;

public:
  ~AvswuManufacturer11p();

  void initialize(int stage) override;
  void finish() override;
  // we only need 1 initialize stage
  int numInitStages() const override;

  // log prefix for debugging
  inline const string log_prefix() {
    if (_manufacturerP) {
      return "manufacturer_id=" + to_string(_manufacturerP->id()) + ", ";
    } else {
      return "manufacturer, ";
    }
  }
};

} // namespace veins

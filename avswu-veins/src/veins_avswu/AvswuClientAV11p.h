#pragma once

#include "AvswuAVNode.h"
#include "AvswuAVNotificationListMap.h"
#include "AvswuAVRangeMap.h"
#include "AvswuAVStat.h"
#include "AvswuConfig.h"
#include "AvswuNode11p.h"
#include "AvswuPacketSupportList.h"

// write decoded s/w update to output directory
#define WRITE_TO_DISK false

using namespace avswu_av_node;
using namespace avswu_config;
using namespace avswu_packet_support_list;
using namespace avswu_av_stat;
using namespace avswu_range_map;
using namespace avswu_notification_list_map;

namespace veins {
// total # of client avs in simulation
static size_t _client_av_count = 0;

// ui node coloring status
typedef enum ui_status_t {
  no_shards = 0,
  has_shards,
  sw_update_success
} ui_status_t;

class VEINS_API AvswuClientAV11p : public AvswuNode11p {
protected:
  /*
   * av core attributes
   */
  // av's manufacturer_id
  uint64_t _manufacturer_id;
  // av's s/w inventory configuration
  size_t _inventory_index;
  // pointer to current inventory for this av
  inventory_t *_inventoryP = nullptr;
  // av node w/ non-conmusig, cache, etc
  AvswuAVNode *_avP = nullptr;
  // list of current software requests based on the latest notifications
  AvswuPacketShardRequestList *_shard_request_listP = nullptr;
  // cache of notifications received
  notification_listP_map_t _notification_listP_map;

  // update delivery range set for this packet
  void update_delivery_range_set(AvswuPacketSupportList &packet);

  // if notification cache contains the packet or not
  const bool notification_list_cache_contains(AvswuPacketSupportList &packet);

  /*
   *statistics
   */
  stat_t _statistics;

  /*
   * communication protocol methods
   */
  // parse and handle notification list
  const bool handle_notification_list(const string payload);
  // parse and handle shard
  const bool handle_shard(const string payload);

  // check if this av, is the recipient (to in packet header)
  const bool is_recipient(AvswuPacket *packetP) const;

private:
  /*
   * delivery av/rsu range
   */
  // rsu range or delivery-av in range
  range_map_t _delivery_range_map;
  // remove old delivery rsu/av nodes from set
  const size_t remove_out_of_range_delivery_nodes();

  /*
   *statistics
   */
  // records scalars (totals)
  void stats_record_scalar();
  // records histograms (min,max,mean)
  void stats_record_histogram();

  const bool initialize_stage1();

  /*
   * notification list cache
   */
  // cache a notification list into cache
  void notification_list_cache_insert(notification_listP_t packetP);
  const bool notification_list_cache_delete(const uint64_t id, const mpz_t &X);

  /*
   * communication protocol methods
   */
  // parse and handle the payload
  const bool handle_payload(const string payload);
  // parse and handle support list
  const bool handle_support_list(const string payload);
  // send inventory
  const bool send_inventory(const uint64_t to_id, mpz_t &to_X);
  // if the av's manufacturer is in support list
  const bool is_supported(AvswuPacketSupportList &packet);

  // handle all possible decode results
  const bool handle_decode_result(
      const decode_status_t status, vector<char> &decoded_output_data,
      const double earliest_shard_request_time, const uint64_t manufacturer_id,
      const uint64_t software_id, const uint64_t version_id,
      const double time_delivered, const double create_time);

  // write (decoded) data to disk
  const bool write_file(const vector<char> &decoded_output_data,
                        const uint64_t manufacturer_id,
                        const uint64_t software_id, const uint64_t version_id);
  // compare input data with output data
  const bool compare_input_output(const vector<char> &decoded_output_data,
                                  const uint64_t manufacturer_id,
                                  const uint64_t software_id,
                                  const uint64_t version_id);
  // process, write, and compare data versus input data
  const bool process_decoded_data(vector<char> &decoded_output_data,
                                  const double earliest_shard_request_time,
                                  const uint64_t manufacturer_id,
                                  const uint64_t software_id,
                                  const uint64_t version_id,
                                  const double time_delivered,
                                  const double create_time);

  /*
   * ui color and labeling
   */
  ui_status_t _ui_status;
  void ui_update(const string payload);

protected:
  int _current_subscribed_service_id;

  const bool initialize_av(const uint64_t av_id, const size_t cache_max_size);

  virtual void onWSM(BaseFrame1609_4 *wsm) override;
  virtual void onWSA(DemoServiceAdvertisment *wsa) override;
  virtual void handlePositionUpdate(cObject *obj) override;

public:
  // initialization
  virtual int numInitStages() const override;
  virtual void initialize(int stage) override;
  // record stats
  virtual void finish() override;
  // destructor for memory cleanup
  ~AvswuClientAV11p();

  // log prefix for debugging
  virtual inline const string log_prefix() {
    if (_avP) {
      return "client_av_id=" + to_string(_avP->id()) + ", ";
    } else {
      return "client av, ";
    }
  };

  // log histogram
  virtual void log_histogram_ns(const string log_prefix, cHistogram &hist);
};

} // namespace veins

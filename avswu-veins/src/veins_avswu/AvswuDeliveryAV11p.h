#pragma once

#include "AvswuAVNode.h"
#include "AvswuClientAV11p.h"
#include "AvswuPacketDeliveryRequest.h"
#include "AvswuPacketInventory.h"

#include "AvswuAV11pMessage_m.h"

// constants for delivery request
#define DELIVERY_MAX_NOTIFCATION 3

// support list message period and start time
#define DELIVERY_SUPPORT_LIST_START 30
#define DELIVERY_SUPPORT_LIST_PERIOD 10

using namespace avswu_packet_inventory;
using namespace avswu_packet_delivery_request;
using namespace avswu_packet_notification_list;

namespace veins {
// total # of client avs in simulation
static size_t _delivery_av_count = 0;

class VEINS_API AvswuDeliveryAV11p : public AvswuClientAV11p {

private:
  // manufacturers that this delivery-av supports (based on those manufactuer's
  // received from an RSU)
  AvswuPacketSupportList _supported_manufacturer;
  // list of the last notifications for delivery to other delivery-avs and
  // client-avs (using a vector, since sets require too many constriants on the
  // type held)
  vector<AvswuNotification> _last_notification_list;

  const bool initialize_stage1();

  // initialize supported manufacturer list
  void initialize_supported_manufacturer_list();
  void schedule_first_support_list();
  // update the support list based on notification dav has in its last
  // notification list
  const bool update_support_list();
  // update last notification list based on inventory packet
  const size_t
  update_notification_list(AvswuPacketNotificationList &notification_list,
                           const AvswuPacketInventory &packet);
  // update last notification list based on delivery request packet
  const size_t
  update_notification_list(AvswuPacketNotificationList &notification_list,
                           const AvswuPacketDeliveryRequest &packet);
  // update our cached notification list based on the notification list received
  const bool update_last_notification_list(notification_listP_t packetP);
  // if it exists, add latest notification
  const bool add_notification(AvswuPacketNotificationList &notification_list,
                              const software_t &software);

  /*
   * communication protocol methods
   */
  // parse and handle the payload
  const bool handle_payload(const string payload);
  // parse and handle support list
  const bool handle_support_list(const string payload);
  // send delivery_request
  const bool send_delivery_request(const uint64_t to_id, mpz_t &to_X);
  // parse and handle notification list
  const bool handle_notification_list(const string payload);
  // parse and handle inventory
  const bool handle_inventory(const string payload);
  // parse and handle delivery request
  const bool handle_delivery_request(const string payload);
  // parse and handle shard request list
  const bool handle_shard_request_list(const string payload);

  // self-messages
  // handle a self-payload packet, and re-broadcast if needed
  const bool handle_self_support_list(const string payload,
                                      AvswuAV11pMessage *wsm);
  // handle self-payload wsm message, determine if wsm is a packet
  const bool handle_self_payload(AvswuAV11pMessage *wsm);

  /*
   * ui color and labeling
   */
  ui_status_t _ui_status;
  void ui_update(const string payload);

protected:
  void onWSM(BaseFrame1609_4 *wsm) override;

  // initiates repetitive broadcast of support-list via self-message, iff
  // delivery-av has manufacturer's it supports
  void handleSelfMsg(cMessage *msg) override;

public:
  void initialize(int stage) override;
  ~AvswuDeliveryAV11p();

  // log prefix for debugging
  inline const string log_prefix() override {
    if (_avP) {
      return "delivery_av_id=" + to_string(_avP->id()) + ", ";
    } else {
      return "delivery av, ";
    }
  };
};

} // namespace veins

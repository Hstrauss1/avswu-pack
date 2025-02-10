#pragma once

#include "AvswuNotification.h"
#include "AvswuPacket.h"
#include "AvswuPacketFooter.h"
#include "AvswuPacketHeader.h"
#include "AvswuUtils.h"
#include <deque>

#include <deque>
#include <gmp.h>
#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_packet;
using namespace avswu_packet_header;
using namespace avswu_packet_footer;
using namespace avswu_utils;
using namespace avswu_notification;
using json = nlohmann::json;

namespace avswu_packet_notification_list {

// payload

// list of notifications sent by RSU to AVs
typedef struct {
  deque<AvswuNotification> notif_deque;
} payload_t;

// packet = [header:payload:footer]
class AvswuPacketNotificationList : public AvswuPacket {
private:
  payload_t _payload;

  // create json for the payload
  const json payload_to_json() const;

public:
  // constructors/destructor
  AvswuPacketNotificationList(const uint64_t from_id, mpz_t &from_X,
                              const uint64_t to_id, mpz_t &to_X,
                              node_type_t from_type);
  AvswuPacketNotificationList();
  ~AvswuPacketNotificationList();

  // getters for payload components
  inline const deque<AvswuNotification> &deque() const {
    return _payload.notif_deque;
  }

  inline const std::deque<AvswuNotification> &notif_deque() const {
    return _payload.notif_deque;
  }

  inline payload_t &payload() { return _payload; }

  // add an item to inventory
  inline void push_back(AvswuNotification &item) {
    _payload.notif_deque.push_back(item);
  }

  // add an item to inventory
  inline void pop_front() { _payload.notif_deque.pop_front(); }

  // create packet hash
  bool create_hash();

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketNotificationList &p);
  friend void to_json(json &j, const AvswuPacketNotificationList &p);
  friend void from_json(const json &j, AvswuPacketNotificationList &p);
};

} // namespace avswu_packet_notification_list

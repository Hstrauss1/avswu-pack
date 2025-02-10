#pragma once

#include "AvswuNotification.h"
#include "AvswuPacket.h"
#include "AvswuPacketFooter.h"
#include "AvswuPacketHeader.h"
#include "AvswuUtils.h"

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

namespace avswu_packet_delivery_request {

// list of notifications sent by RSU to AVs
typedef struct {
  double start_time;
  size_t max_notification;
} payload_t;

// packet = [header:payload:footer]
class AvswuPacketDeliveryRequest : public AvswuPacket {
private:
  payload_t _payload;

  // create json for the payload
  const json payload_to_json() const;

public:
  // constructors/destructor
  AvswuPacketDeliveryRequest(const uint64_t from_id, mpz_t &from_X,
                             const uint64_t to_id, mpz_t &to_X,
                             const double start_time,
                             const uint64_t max_notification,
                             node_type_t from_type);
  AvswuPacketDeliveryRequest();
  ~AvswuPacketDeliveryRequest();

  // getters for payload components
  inline const double start_time() const { return _payload.start_time; }
  inline const uint64_t max_notification() const {
    return _payload.max_notification;
  }

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketDeliveryRequest &p);
  friend void to_json(json &j, const AvswuPacketDeliveryRequest &p);
  friend void from_json(const json &j, AvswuPacketDeliveryRequest &p);
};

} // namespace avswu_packet_delivery_request

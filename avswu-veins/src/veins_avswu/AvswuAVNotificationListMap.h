#pragma once

#include "AvswuPacketNotificationList.h"
#include <cstdint>
#include <gmp.h>
#include <unordered_map>

using namespace std;
using namespace avswu_packet_notification_list;

namespace avswu_notification_list_map {

/*
 * map of (id,X) -> notification_list_map
 */
// key for unordered notification list map
class NotificationListMapKey {
private:
  uint64_t _id;
  mpz_t _X;

public:
  // constructor/destructor
  inline NotificationListMapKey(const uint64_t id, mpz_t &X) {
    _id = id;
    mpz_init(_X);
  }
  inline ~NotificationListMapKey() {}
  // getters
  inline const uint64_t id() const { return _id; }
  inline const mpz_t &X() const { return _X; }
  // comparison, needed for unordered map
  bool operator==(const NotificationListMapKey rs_key) const {
    return (_id == rs_key.id()) && (mpz_cmp(_X, rs_key.X()) == 0);
  }
};

// custom hash for unordered notification list map key
class NotificationListMapHash {
public:
  // fast bit-wise hash function
  size_t operator()(const NotificationListMapKey &rs_key) const {
    size_t hash = (rs_key.id() & mpz_get_ui(rs_key.X())) % SIZE_MAX;
    return hash;
  }
};

// smart-pointer to a notification list
typedef shared_ptr<AvswuPacketNotificationList> notification_listP_t;

// map of (id,X) ->last_notification list_request_list time
typedef unordered_map<NotificationListMapKey, notification_listP_t,
                      NotificationListMapHash>
    notification_listP_map_t;

} // namespace avswu_notification_list_map
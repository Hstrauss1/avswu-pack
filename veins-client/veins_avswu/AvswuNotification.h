#pragma once

#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

using namespace std;
using json = nlohmann::json;

namespace avswu_notification {

typedef struct {
  uint64_t shard_id;
  string cid_hash;
  string shard_hash;
} packet_notif_t;

class AvswuNotification {
private:
  // ids to specify softare
  uint64_t _manufacturer_id;
  uint64_t _software_id;
  uint64_t _version_id;
  // simulation time the notification was created
  double _create_time;
  // list of cid and pdu hashs for each packet
  vector<packet_notif_t> _packet_notif_list;

  // copy
  void copy(AvswuNotification &a, const AvswuNotification &b) const;

public:
  // constructor/destructor
  AvswuNotification(const uint64_t manufacturer_id, const uint64_t software_id,
                    const uint64_t version_id,
                    const vector<packet_notif_t> packet_notif_list,
                    double create_time);
  AvswuNotification(const AvswuNotification &notif);

  AvswuNotification();
  ~AvswuNotification();

  // getters
  inline const double create_time() const { return _create_time; }
  inline const uint64_t manufacturer_id() const { return _manufacturer_id; }
  inline const uint64_t software_id() const { return _software_id; }
  inline const uint64_t version_id() const { return _version_id; }
  inline const vector<packet_notif_t> packet_notif_list() const {
    return _packet_notif_list;
  }

  // set and push to configuration a notification
  void set(const uint64_t manufacturer_id, const uint64_t software_id,
           const uint64_t version_id, const double create_time);
  void push_back(const uint64_t shard_id, const string cid_hash,
                 const string shard_hash);

  inline void clear() { _packet_notif_list.clear(); }

  // assignment operator
  AvswuNotification &operator=(const AvswuNotification &notif);

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuNotification &notif);
  friend void to_json(json &j, const AvswuNotification &notif);
  friend void from_json(const json &j, AvswuNotification &notif);
};

} // namespace avswu_notification
#include "AvswuNotification.h"
#include "AvswuUtils.h"
#include <spdlog/spdlog.h>

namespace avswu_notification {

// constructor
AvswuNotification::AvswuNotification(
    const uint64_t manufacturer_id, const uint64_t software_id,
    const uint64_t version_id, const vector<packet_notif_t> packet_notif_list,
    double create_time)
    :

      _manufacturer_id(manufacturer_id), _software_id(software_id),
      _version_id(version_id), _create_time(create_time),
      _packet_notif_list(packet_notif_list) {}

AvswuNotification::AvswuNotification(const AvswuNotification &notif) {
  copy(*this, notif);
}

AvswuNotification::AvswuNotification() {
  _create_time = 0;
  _manufacturer_id = 0;
  _software_id = 0;
  _version_id = 0;
}

// destructor
AvswuNotification::~AvswuNotification() {
  // deallocate data
  _packet_notif_list.clear();
}

// assignment operator
AvswuNotification &
AvswuNotification::operator=(const AvswuNotification &notif) {
  // perform the copy
  copy(*this, notif);
  return *this;
}

// copy
void AvswuNotification::copy(AvswuNotification &a,
                             const AvswuNotification &b) const {
  // copy creation time
  a._create_time = b.create_time();
  // set ids
  a.set(b.manufacturer_id(), b.software_id(), b.version_id(), b.create_time());
  // copy all packet info
  a._packet_notif_list.clear();
  for (size_t i = 0; i < b._packet_notif_list.size(); i++) {
    packet_notif_t notif_info;
    notif_info.shard_id = b._packet_notif_list[i].shard_id;
    notif_info.cid_hash = b._packet_notif_list[i].cid_hash;
    notif_info.shard_hash = b._packet_notif_list[i].shard_hash;
    a._packet_notif_list.push_back(notif_info);
  }
}

// set
void AvswuNotification::set(const uint64_t manufacturer_id,
                            const uint64_t software_id,
                            const uint64_t version_id,
                            const double create_time) {
  _manufacturer_id = manufacturer_id;
  _software_id = software_id;
  _version_id = version_id;
  _create_time = create_time;
}

// push back a packet notification
void AvswuNotification::push_back(const uint64_t shard_id,
                                  const string cid_hash,
                                  const string shard_hash) {
  packet_notif_t p;
  p.shard_id = shard_id;
  p.cid_hash = cid_hash;
  p.shard_hash = shard_hash;
  _packet_notif_list.push_back(p);
}

// to/from json
void to_json(json &j, const AvswuNotification &notif) {

  j = json{{"create_time", notif.create_time()},
           {"manufacturer_id", notif.manufacturer_id()},
           {"software_id", notif.software_id()},
           {"version_id", notif.version_id()}};
  // add packet notification list to json
  j["packet_notif_list"] = json::array();
  for (size_t i = 0; i < notif.packet_notif_list().size(); i++) {
    const auto pn = notif.packet_notif_list()[i];
    json json_pn = json{{"shard_id", pn.shard_id},
                        {"cid_hash", pn.cid_hash},
                        {"shard_hash", pn.shard_hash}};
    j["packet_notif_list"].push_back(json_pn);
  }
}

void from_json(const json &j, AvswuNotification &notif) {
  j.at("create_time").get_to(notif._create_time);
  j.at("manufacturer_id").get_to(notif._manufacturer_id);
  j.at("software_id").get_to(notif._software_id);
  j.at("version_id").get_to(notif._version_id);

  // get packet notification list from json
  notif._packet_notif_list.clear();
  size_t n = j.at("packet_notif_list").size();
  packet_notif_t pn;
  json json_pn_list = j.at("packet_notif_list");
  for (size_t i = 0; i < n; i++) {
    json_pn_list[i].at("shard_id").get_to(pn.shard_id);
    json_pn_list[i].at("cid_hash").get_to(pn.cid_hash);
    json_pn_list[i].at("shard_hash").get_to(pn.shard_hash);
    notif._packet_notif_list.push_back(pn);
  }
}

// output
ostream &operator<<(ostream &os, const AvswuNotification &s) {
  json j(s);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_notification
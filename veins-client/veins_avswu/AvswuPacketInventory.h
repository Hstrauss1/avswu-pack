#pragma once

#include "AvswuConfig.h"
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
using namespace avswu_config;
using json = nlohmann::json;

namespace avswu_packet_inventory {

// payload

// payload
typedef struct {
  uint64_t manufacturer_id;
  uint64_t software_id;
  uint64_t version_id;
  double create_time;
} software_t;

// payload
typedef struct {
  // if this software list is for other av's (or this av)
  // if other is true, we are requesting any s/w that is *not* in
  // software_list
  bool other_av;
  // list of software items used by AV
  vector<software_t> software_list;
} payload_t;

// packet = [header:payload:footer]
class AvswuPacketInventory : public AvswuPacket {
private:
  payload_t _payload;

  // create json for the payload
  const json payload_to_json() const;
  // set payload from json
  void payload_from_json(const json &j_payload);

public:
  // constructors/destructor
  AvswuPacketInventory(const uint64_t from_id, mpz_t &from_X,
                       const uint64_t to_id, mpz_t &to_X,
                       node_type_t from_type);
  AvswuPacketInventory();
  ~AvswuPacketInventory();

  // getters for payload components
  inline const vector<software_t> &software_list() const {
    return _payload.software_list;
  }

  // add an item to inventory
  inline void push_back(software_t &item) {
    _payload.software_list.push_back(item);
  }

  // set inventory based on configuration
  const bool set_payload(AvswuConfig *configP, const uint64_t manufacturer_id,
                         const size_t inventory_index);

  // set/get other_av
  inline void set_other_av(const bool val) { _payload.other_av = val; }
  inline const bool other_av() { return _payload.other_av; }

  // create packet hash
  bool create_hash();

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketInventory &p);
  friend void to_json(json &j, const AvswuPacketInventory &p);
  friend void from_json(const json &j, AvswuPacketInventory &p);
};

} // namespace avswu_packet_inventory

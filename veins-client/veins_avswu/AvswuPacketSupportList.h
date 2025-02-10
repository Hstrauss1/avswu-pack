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
using json = nlohmann::json;

namespace avswu_packet_support_list {

// payload

// payload
typedef struct {
  uint64_t manufacturer_id;
  mpz_t manufacturer_X;
} manufacturer_id_t;

// list of software items used by AV
typedef struct {
  vector<manufacturer_id_t> _manufacturer_list;
} payload_t;

// packet = [header:payload:footer]
class AvswuPacketSupportList : public AvswuPacket {
private:
  payload_t _payload;

  // create json for the payload
  const json payload_to_json() const;
  // set payload from json
  void payload_from_json(const json &j_payload);

public:
  // constructors/destructor
  AvswuPacketSupportList(const uint64_t from_id, mpz_t &from_X,
                         const uint64_t to_id, mpz_t &to_X,
                         node_type_t from_type);
  AvswuPacketSupportList();
  ~AvswuPacketSupportList();

  // getters for payload components
  inline const vector<manufacturer_id_t> &manufacturer_list() const {
    return _payload._manufacturer_list;
  }

  // add an item to inventory
  inline void push_back(manufacturer_id_t &item) {
    _payload._manufacturer_list.push_back(item);
  }

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuPacketSupportList &p);
  friend void to_json(json &j, const AvswuPacketSupportList &p);
  friend void from_json(const json &j, AvswuPacketSupportList &p);
};

} // namespace avswu_packet_support_list

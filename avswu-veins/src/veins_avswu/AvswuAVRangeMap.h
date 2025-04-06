#pragma once

#include <cstdint>
#include <gmp.h>
#include <omnetpp/simtime_t.h>
#include <unordered_map>

using namespace std;
using namespace omnetpp;

namespace avswu_range_map {

/*
 * map of (id,X) -> last_shard_request_list time
 */
// key for unordered shard request list time map
class RangeMapKey {
private:
  uint64_t _id;
  mpz_t _X;

public:
  // constructor/destructor
  inline RangeMapKey(const uint64_t id, mpz_t &X) {
    _id = id;
    mpz_init(_X);
  }
  inline ~RangeMapKey() {}
  // getters
  inline const uint64_t id() const { return _id; }
  inline const mpz_t &X() const { return _X; }
  // comparison, needed for unordered map
  bool operator==(const RangeMapKey rs_key) const {
    return (_id == rs_key.id()) && (mpz_cmp(_X, rs_key.X()) == 0);
  }
};

// custom hash for unordered time map key
class RangeMapHash {
public:
  // fast bit-wise hash function
  size_t operator()(const RangeMapKey &rs_key) const {
    size_t hash = (rs_key.id() & mpz_get_ui(rs_key.X())) % SIZE_MAX;
    return hash;
  }
};

// map of (id,X) ->last_shard_request_list time
typedef unordered_map<RangeMapKey, simtime_t, RangeMapHash> range_map_t;

} // namespace avswu_range_map
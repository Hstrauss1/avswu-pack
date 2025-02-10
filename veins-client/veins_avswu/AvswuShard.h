#pragma once

#include "spdlog/fmt/bin_to_hex.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

using namespace std;
using json = nlohmann::json;

namespace avswu_shard {

class AvswuShard {
private:
  uint64_t _manufacturer_id;
  uint64_t _software_id;
  uint64_t _version_id;
  uint64_t _shard_id;
  // erasure encode parameters
  int _erasure_id;
  uint32_t _k;
  uint32_t _m;
  uint32_t _w;
  size_t _buffer_size;
  size_t _original_file_size;
  // erasure-encoded cpabe-encrypted shard of data
  vector<char> _shard_data;

  void copy(AvswuShard &a, const AvswuShard &b) const;

public:
  // constructor/destructor
  AvswuShard(const uint64_t manufacturer_id, const uint64_t software_id,
             const uint64_t version_id, const uint64_t shard_id,
             const int erasure_id, const uint32_t k, const uint32_t m,
             const uint32_t w, const size_t original_file_size,
             const vector<char> shard_data);
  AvswuShard(const AvswuShard &shard);

  AvswuShard();
  ~AvswuShard();

  // getters
  inline const uint64_t manufacturer_id() const { return _manufacturer_id; }
  inline const uint64_t software_id() const { return _software_id; }
  inline const uint64_t version_id() const { return _version_id; }
  inline const uint64_t shard_id() const { return _shard_id; }
  inline const int erasure_id() const { return _erasure_id; }
  inline const uint32_t k() const { return _k; }
  inline const uint32_t m() const { return _m; }
  inline const uint32_t w() const { return _w; }
  inline const size_t buffer_size() const { return _buffer_size; }
  inline const size_t original_file_size() const { return _original_file_size; }
  inline vector<char> shard_data() const { return _shard_data; }

  // setters
  void set(const uint64_t manufacturer_id, const uint64_t software_id,
           const uint64_t version_id, const uint64_t shard_id, int _erasure_id,
           const uint32_t k, const uint32_t m, const uint32_t w,
           const size_t original_file_size, const vector<char> shard_data);

  // copy
  AvswuShard &operator=(const AvswuShard &shard);

  // size in bytes
  const size_t size() const;

  // increase/decrease buffer size
  bool increase_buffer_size(size_t const scale);
  bool decrease_buffer_size(size_t const scale);

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuShard &s);
  friend void to_json(json &j, const AvswuShard &s);
  friend void from_json(const json &j, AvswuShard &s);
};

} // namespace avswu_shard
#include "AvswuShard.h"
#include <spdlog/spdlog.h>

namespace avswu_shard {

// constructor
AvswuShard::AvswuShard(const uint64_t manufacturer_id,
                       const uint64_t software_id, const uint64_t version_id,
                       const uint64_t shard_id, const int erasure_id,
                       const uint32_t k, const uint32_t m, const uint32_t w,
                       const size_t original_file_size,
                       const vector<char> shard_data)
    : _manufacturer_id(manufacturer_id), _software_id(software_id),
      _version_id(version_id), _shard_id(shard_id), _erasure_id(erasure_id),
      _k(k), _m(m), _w(w), _original_file_size(original_file_size) {
  _buffer_size = shard_data.size();
  _shard_data = shard_data;
}

AvswuShard::AvswuShard(const AvswuShard &shard) { copy(*this, shard); }

AvswuShard::AvswuShard() {
  _manufacturer_id = 0;
  _software_id = 0;
  _version_id = 0;
  _shard_id = 0;

  _erasure_id = 0;
  _k = 0;
  _m = 0;
  _w = 0;

  _buffer_size = 0;
  _original_file_size = 0;
}

// destructor
AvswuShard::~AvswuShard() {
  // deallocate data
  _shard_data.clear();
}

// copy operator
AvswuShard &AvswuShard::operator=(const AvswuShard &shard) {
  // perform the copy
  copy(*this, shard);
  return *this;
}

void AvswuShard::copy(AvswuShard &a, const AvswuShard &b) const {
  a.set(b.manufacturer_id(), b.software_id(), b.version_id(), b.shard_id(),
        b.erasure_id(), b.k(), b.m(), b.w(), b.original_file_size(),
        b.shard_data());
}

// set shard values
void AvswuShard::set(const uint64_t manufacturer_id, const uint64_t software_id,
                     const uint64_t version_id, const uint64_t shard_id,
                     const int erasure_id, const uint32_t k, const uint32_t m,
                     const uint32_t w, const size_t original_file_size,
                     const vector<char> shard_data) {
  _manufacturer_id = manufacturer_id;
  _software_id = software_id;
  _version_id = version_id;
  _shard_id = shard_id;
  _erasure_id = erasure_id;
  _k = k;
  _m = m;
  _w = w;
  _buffer_size = shard_data.size();
  _original_file_size = original_file_size;
  _shard_data = shard_data;
}

/*
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
  */
// size in bytes
const size_t AvswuShard::size() const {
  size_t n = 0;
  n += sizeof(_manufacturer_id);
  n += sizeof(_software_id);
  n += sizeof(_version_id);
  n += sizeof(_shard_id);
  n += sizeof(_erasure_id);
  n += sizeof(_k);
  n += sizeof(_m);
  n += sizeof(_w);
  n += sizeof(_buffer_size);
  n += sizeof(_original_file_size);
  n += _shard_data.size();

  return n;
}

// increase buffer size
bool AvswuShard::increase_buffer_size(size_t const scale) {
  // scale shard_data
  size_t const orig_shard_size = _shard_data.size();
  size_t const new_size = orig_shard_size * scale;
  _shard_data.resize(new_size);

  // set shard data with random values with a poison pattern
  for (size_t i = orig_shard_size; i < new_size; i++) {
    // beef pattern is our poison
    if (i % 4 == 0) {
      _shard_data[i] = 0xb;
    }
    if (i % 4 == 1) {
      _shard_data[i] = 0xe;
    }
    if (i % 4 == 2) {
      _shard_data[i] = 0xe;
    }
    if (i % 4 == 3) {
      _shard_data[i] = 0xf;
    }
  }

  // update buffer size
  _buffer_size = new_size;

  return true;
}

// increase buffer size
bool AvswuShard::decrease_buffer_size(size_t const scale) {
  // scale shard_data
  size_t const orig_shard_size = _shard_data.size() / scale;
  _shard_data.resize(orig_shard_size);

  // update buffer size
  _buffer_size = orig_shard_size;

  return true;
}

// to/from json
void to_json(json &j, const AvswuShard &s) {

  j = json{{"manufacturer_id", s.manufacturer_id()},
           {"software_id", s.software_id()},
           {"version_id", s.version_id()},
           {"shard_id", s.shard_id()},
           {"erasure_id", s.erasure_id()},
           {"k", s.k()},
           {"m", s.m()},
           {"w", s.w()},
           {"buffer_size", s.buffer_size()},
           {"original_file_size", s.original_file_size()},
           {"shard_data", s.shard_data()}};
}

void from_json(const json &j, AvswuShard &s) {

  j.at("manufacturer_id").get_to(s._manufacturer_id);
  j.at("software_id").get_to(s._software_id);
  j.at("version_id").get_to(s._version_id);
  j.at("shard_id").get_to(s._shard_id);
  j.at("erasure_id").get_to(s._erasure_id);
  j.at("k").get_to(s._k);
  j.at("m").get_to(s._m);
  j.at("w").get_to(s._w);
  j.at("buffer_size").get_to(s._buffer_size);
  j.at("original_file_size").get_to(s._original_file_size);

  // json binary shard data -> vector char
  auto shard_data = j.at("shard_data");
  vector<unsigned char> bytes = shard_data;
  s._shard_data.clear();
  for (size_t i = 0; i < bytes.size(); i++) {
    auto ch = bytes[i];
    s._shard_data.push_back(ch);
  }
}

// output
ostream &operator<<(ostream &os, const AvswuShard &s) {
  json j(s);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_shard
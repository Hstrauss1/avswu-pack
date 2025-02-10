#pragma once

#include "AvswuOpenSSL.h"
#include "AvswuUtils.h"

#include <iostream>
#include <nlohmann/json.hpp>

using namespace std;
using namespace avswu_utils;
using namespace avswu_open_ssl;
using json = nlohmann::json;

namespace avswu_config {

typedef struct {
  hash_type_t type;
  string value;
} hash_t;

typedef struct {
  string directory;
  string name;
  hash_t hash;
} software_file_t;

// software list item in json file
typedef struct {
  size_t index;
  uint64_t software_id;
  uint64_t version_id;
  double create_time;
  software_file_t file;
} software_item_t;

// inventory item
typedef struct {
  size_t index;
  vector<size_t> software_index_list;
} inventory_t;

// manufacturer list item in json file
typedef struct {
  size_t index;
  uint64_t manufacturer_id;
  vector<software_item_t> software_list;
  vector<inventory_t> inventory_list;
} manufacturer_item_t;

class AvswuConfig {
private:
  vector<manufacturer_item_t> _manufacturer_list;
  // hash file
  const string hash_file(const string input_file, const hash_type_t type);
  // input file path
  const string input_file_name(const size_t i, uint32_t input_start,
                               uint32_t input_end, uint32_t input_step);

public:
  // constructors/destructor
  AvswuConfig(const uint32_t n_manufacturer, const uint32_t n_software,
              const uint32_t n_inventory, uint32_t input_start,
              uint32_t input_end, uint32_t input_step, string input_subdir);
  AvswuConfig();
  ~AvswuConfig();

  // getters
  // manfacturer list
  const vector<manufacturer_item_t> manufacturer_list() {
    return _manufacturer_list;
  }
  // gets a software item
  software_item_t *software_itemP(const uint64_t manufacturer_id,
                                  const uint64_t software_id);
  // gets a inventory item
  inventory_t *inventoryP(const uint64_t manufacturer_id,
                          const size_t inventory_index);

  // get software item, based on config structure's specific software item
  const bool get_software_item(software_item_t &sw_item,
                               const uint64_t manufacturer_id,
                               const uint64_t software_id) const;
  // sets software item
  const bool set_software_item(const uint64_t manufacturer_id,
                               const software_item_t &sw_item);

  // read a config json file
  void write_file(const string output_file);
  void read_file(const string input_file);

  // i/o and json
  friend ostream &operator<<(ostream &os, const AvswuConfig &config);
  friend void to_json(json &j, const AvswuConfig &config);
  friend void from_json(const json &j, AvswuConfig &config);
};

} // namespace avswu_config

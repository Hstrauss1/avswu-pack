#include "AvswuConfig.h"
#include "AvswuOpenSSL.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <nlohmann/json_fwd.hpp>

using namespace avswu_open_ssl;

namespace avswu_config {

const string AvswuConfig::hash_file(const string input_file,
                                    const hash_type_t type) {

  // read file
  vector<char> input_data;

  spdlog::info("input_file={}", input_file);

  string hash_sha256;
  try {
    // read the file
    // open file
    ifstream input(input_file, ifstream::binary);
    if (!input.is_open()) {
      const string msg = "unable to open input_file=" + input_file;
      spdlog::error(msg);
      throw std::runtime_error(msg);
    }

    // copy data info buffer
    vector<char> data(istreambuf_iterator<char>(input), {});

    // close file
    input.close();

    // convert data to string
    string data_str = string(data.begin(), data.end());

    // hash the data string
    const bool is_success = avswu_open_ssl::hash_by_type(
        hash_sha256, data_str, AVSWU_DATA_HASH_ALGORTITHM);
    if (!is_success) {
      spdlog::error("hash generation failed");
    }

  } catch (const std::runtime_error &e) {
    spdlog::error("exception e={}", e.what());
  }

  return hash_sha256;
}

const string AvswuConfig::input_file_name(const size_t i, uint32_t input_start,
                                          uint32_t input_end,
                                          uint32_t input_step) {
  const size_t sw_file_num = input_start + i * input_step;
  string pad_num = fmt::format("{:04}", sw_file_num);
  const string name = "test_data_" + pad_num + ".data";
  return name;
}

AvswuConfig::AvswuConfig(const uint32_t n_manufacturer,
                         const uint32_t n_software, const uint32_t n_inventory,
                         uint32_t input_start, uint32_t input_end,
                         uint32_t input_step, string input_subdir) {

  // seed the random number generator, so that it's repeatable
  srand(123456);

  const uint64_t create_time = 0;

  const size_t n_input = (input_end - input_start + 1) / input_step;

  const string input_dir =
      string(AVSWU_BASEDIR) + "/veins-client/input/" + input_subdir;

  // calculate all the hashes so we only calculate them once
  hash_type_t hash_type = avswu_open_ssl::AVSWU_DATA_HASH_ALGORTITHM;
  vector<string> hash_list;
  for (size_t i = 0; i < n_input; i++) {
    const string name = input_file_name(i, input_start, input_end, input_step);
    const string input_file = input_dir + "/" + name;
    spdlog::info("calculating hash {}/{} for {}", i, n_input, input_file);
    const string hash = hash_file(input_file, hash_type);
    hash_list.push_back(hash);
  }

  // create manufacturer and set values
  for (uint32_t m = 0; m < n_manufacturer; m++) {

    // create man
    manufacturer_item_t man;
    const uint64_t manufacturer_id = 100 + m;
    man.manufacturer_id = manufacturer_id;
    man.index = m;

    // create all s/w
    for (uint32_t s = 0; s < n_software; s++) {
      software_item_t sw_item;
      sw_item.index = s;
      sw_item.software_id = 1000 + s;
      sw_item.version_id = 0;
      sw_item.create_time = create_time;
      // randomly pick 1the test data input files
      const size_t sw_rand_index = (rand() % n_input);
      sw_item.file.directory = input_dir;
      const string name =
          input_file_name(sw_rand_index, input_start, input_end, input_step);
      sw_item.file.name = name;

      // compute hash
      sw_item.file.hash.type = hash_type;
      sw_item.file.hash.value = hash_list[sw_rand_index];

      man.software_list.push_back(sw_item);
    }

    // create inventory list that AVs can pick from upon their creation
    for (size_t i = 0; i < n_inventory; i++) {
      inventory_t inv;
      inv.index = i;
      // get a set of indices for the software in this inventory
      auto n_sw_selected = max((int)1, (int)(rand() % (n_software + 1)));
      // create vector n_sw_selected random indices

      // create vector of indices
      for (size_t i = 0; i < n_software; i++) {
        inv.software_index_list.push_back(i);
      }
      // shuffle the indices
      random_shuffle(inv.software_index_list.begin(),
                     inv.software_index_list.end());

      // select the first n_sw_selected indices from the shuffled vector
      inv.software_index_list.resize(n_sw_selected);

      // sort the n_sw_selected indices
      sort(inv.software_index_list.begin(), inv.software_index_list.end());

      man.inventory_list.push_back(inv);
    }

    _manufacturer_list.push_back(man);
  }
}

AvswuConfig::AvswuConfig() {}

AvswuConfig::~AvswuConfig() {}

// gets a software item
software_item_t *AvswuConfig::software_itemP(const uint64_t manufacturer_id,
                                             const uint64_t software_id) {
  software_item_t *sw_itemP = nullptr;
  bool sw_found = false;
  size_t m = 0;
  while (m < manufacturer_list().size() && !sw_found) {
    const auto man = manufacturer_list()[m];
    if (man.manufacturer_id == manufacturer_id) {
      const auto sw_list = man.software_list;
      size_t s = 0;
      while (s < sw_list.size() && !sw_found) {
        auto sw = sw_list[s];
        if (sw.software_id == software_id) {
          sw_found = true;
          sw_itemP = &_manufacturer_list[m].software_list[s];
        }
        s++;
      }
    }
    m++;
  }

  return sw_itemP;
}

// gets a inventory item
inventory_t *AvswuConfig::inventoryP(const uint64_t manufacturer_id,
                                     const size_t inventory_index) {
  inventory_t *inv_itemP = nullptr;
  bool inv_found = false;
  size_t m = 0;
  while (m < manufacturer_list().size() && !inv_found) {
    const auto man = manufacturer_list()[m];
    if (man.manufacturer_id == manufacturer_id) {
      if (inventory_index < man.inventory_list.size()) {
        inv_found = true;
        inv_itemP = &_manufacturer_list[m].inventory_list[inventory_index];
      }
    }
    m++;
  }

  return inv_itemP;
}

// searches config structure for a specific softare item
const bool AvswuConfig::get_software_item(software_item_t &sw_item,
                                          const uint64_t manufacturer_id,
                                          const uint64_t software_id) const {

  // loop thru man list,  loop thru software list
  uint32_t m = 0;
  bool found = false;
  while (m < _manufacturer_list.size() && !found) {
    const auto man = _manufacturer_list[m];
    if (man.manufacturer_id == manufacturer_id) {
      const auto sw_list = _manufacturer_list[m].software_list;
      uint32_t s = 0;
      while (s < sw_list.size() && !found) {
        const auto item = sw_list[s];
        if (item.software_id == software_id) {
          sw_item = item;
          found = true;
        }
        s++;
      }
    }
    m++;
  }

  return found;
}

const bool AvswuConfig::set_software_item(const uint64_t manufacturer_id,
                                          const software_item_t &sw_item) {

  // find manufacturer
  uint32_t m = 0;
  while (m < _manufacturer_list.size()) {
    const auto man = _manufacturer_list[m];
    if (man.manufacturer_id == manufacturer_id) {
      const auto sw_list = _manufacturer_list[m].software_list;
      if (sw_item.index < sw_list.size()) {
        // update the item
        _manufacturer_list[m].software_list[sw_item.index] = sw_item;
        return true;
      } else {
        return false;
      }
    }
    m++;
  }

  return false;
}

void AvswuConfig::write_file(const string output_file) {
  // open file
  ofstream output(output_file, ofstream::binary);
  if (!output.is_open()) {
    const string msg = "unable to open output_file=" + output_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // write json
  string output_data = json(*this).dump(4);
  output.write(output_data.data(), output_data.size());

  // close file
  output.close();
}

// read file
void AvswuConfig::read_file(const string input_file) {
  // open file
  ifstream input(input_file);
  if (!input.is_open()) {
    const string msg = "unable to open input_file=" + input_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // parse file
  json j_config;
  try {
    j_config = json::parse(input);
  } catch (char *excp) {
    const string msg =
        "unable to parse the inventory payload json file input_file=" +
        input_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // set config  from json
  from_json(j_config, (*this));

  // close file
  input.close();
}

// to/from json
void to_json(json &j, const AvswuConfig &config) {

  // manufacturer list -> json
  json j_manufacturer_list = json::array();
  for (size_t m = 0; m < config._manufacturer_list.size(); m++) {
    // software list -> json
    json j_software_list = json::array();
    auto man = config._manufacturer_list[m];
    for (size_t s = 0; s < man.software_list.size(); s++) {
      auto sw = man.software_list[s];
      json j_software_item = {
          {"index", sw.index},
          {"software_id", sw.software_id},
          {"version_id", sw.version_id},
          {"create_time", sw.create_time},
          {"file",
           {
               {"directory", sw.file.directory},
               {"name", sw.file.name},
               {"hash",
                {
                    {"type", sw.file.hash.type},
                    {"value", sw.file.hash.value},
                }},

           }},
      };
      j_software_list.push_back(j_software_item);
    }

    // inventory list -> json
    json j_inventory_list = json::array();
    for (size_t i = 0; i < man.inventory_list.size(); i++) {
      auto inv = man.inventory_list[i];
      json j_inv = {{"index", inv.index},
                    {"software_index_list", inv.software_index_list}};
      j_inventory_list.push_back(j_inv);
    }

    // set man json
    json j_man = {
        {"index", config._manufacturer_list[m].index},
        {"manufacturer_id", config._manufacturer_list[m].manufacturer_id},
        {"software_list", j_software_list},
        {"inventory_list", j_inventory_list}};

    // add man to list
    j_manufacturer_list.push_back(j_man);
  }
  j = {{"manufacturer_list", j_manufacturer_list}};
}

void from_json(const json &j, AvswuConfig &config) {

  // clear the struct
  config._manufacturer_list.clear();

  // get manufacturer list
  for (size_t m = 0; m < j.at("manufacturer_list").size(); m++) {

    // fill an item
    manufacturer_item_t man_item;
    json j_man = j.at("manufacturer_list")[m];
    j_man.at("index").get_to(man_item.index);
    j_man.at("manufacturer_id").get_to(man_item.manufacturer_id);

    // get software list
    for (size_t s = 0; s < j_man.at("software_list").size(); s++) {
      // add a new item
      software_item_t sw_item;

      // fill the item
      json j_sw = j_man.at("software_list")[s];
      j_sw.at("index").get_to(sw_item.index);
      j_sw.at("software_id").get_to(sw_item.software_id);
      j_sw.at("version_id").get_to(sw_item.version_id);
      j_sw.at("create_time").get_to(sw_item.create_time);

      // file json structure
      json j_file = j_sw.at("file");
      j_file.at("directory").get_to(sw_item.file.directory);
      j_file.at("name").get_to(sw_item.file.name);

      // hash json structure
      json j_hash = j_file.at("hash");
      j_hash.at("type").get_to(sw_item.file.hash.type);
      j_hash.at("value").get_to(sw_item.file.hash.value);

      // add a new sw item
      man_item.software_list.push_back(sw_item);
    }

    // get inventory list
    for (size_t i = 0; i < j_man.at("inventory_list").size(); i++) {
      // fill the item
      inventory_t inv_item;
      json j_inv = j_man.at("inventory_list")[i];
      j_inv.at("index").get_to(inv_item.index);
      j_inv.at("software_index_list").get_to(inv_item.software_index_list);

      // add the item
      man_item.inventory_list.push_back(inv_item);
    }

    config._manufacturer_list.push_back(man_item);
  }
}

// output
ostream &operator<<(ostream &os, AvswuConfig &config) {
  json j(config);
  os << j.dump(4) << endl;
  return os;
}

} // namespace avswu_config

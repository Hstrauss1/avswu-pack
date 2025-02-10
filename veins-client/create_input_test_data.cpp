/*
 * create invetory json files
 */

#include "AvswuUtils.h"
#include <filesystem>
#include <fstream>

#include <getopt.h>
#include <spdlog/spdlog.h>

#include <unistd.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;

using json = nlohmann::json;

void parse_args(int argc, char **argv) {
  // parse command line arguments using gnu's getopt

  // n has a parameter "n:" and v does not have a parameter, => "n:v"
  const char *short_options = "k:m:v";
  option long_options[] = {{"verbose", no_argument, NULL, 'v'},
                           {NULL, 0u, NULL, 0u}};
  int option_index = 0;

  int opt;
  string cache_min_str, cache_max_str, c_args, delimiter;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'v':
      spdlog::set_level(spdlog::level::trace);
      spdlog::debug("verbose logging is on");
      break;
    case '?':
      spdlog::error("command line error. one or more flags not recognized: {}",
                    char(opt));
      spdlog::info("usage: " + string(argv[0]) + "--verbose");
      break;
    default:
      exit(-1);
    }
  }
}

void write_file(const uint32_t n, const size_t block_size,
                const string subdir) {

  const string output_dir =
      string(AVSWU_BASEDIR) + "/veins-client/input/" + subdir;

  // create dir
  std::filesystem::create_directories(output_dir);

  // open file
  const string output_file =
      output_dir + "/" + "test_data_" + avswu_utils::zero_pad(n, 4) + ".data";
  spdlog::info("output_file={}", output_file);

  ofstream output(output_file, ofstream::binary);
  if (!output.is_open()) {
    const string msg = "unable to open output_file=" + output_file;
    spdlog::error(msg);
    throw std::runtime_error(msg);
  }

  // write i blocks of block_size bytes
  vector<char> output_data;
  for (size_t block = 0; block < n; block += 1) {
    // add a block of bytes with value block
    const char value = static_cast<char>(block);
    for (size_t b = 0; b < block_size; b++) {
      output_data.push_back(value);
    }
  }

  output.write(output_data.data(), output_data.size());

  // close file
  output.close();
}

// create test data
void create_input_test_data() {
  size_t size_kb = 1024;
  size_t size_mb = 1024 * size_kb;

  // 1-1024 KiB data files
  uint32_t start = 1;
  uint32_t end = 1024;
  uint32_t step = 1;
  size_t size_bytes = size_kb;
  string subdir = "kb";
  for (uint32_t i = start; i <= end; i += step) {
    write_file(i, size_bytes, subdir);
  }

  // 1 Mib -> 128 Mib data files
  start = 1;
  end = 128;
  step = 1;
  size_bytes = size_mb;
  subdir = "mb";
  for (uint32_t i = start; i <= end; i += step) {
    write_file(i, size_bytes, subdir);
  }

  // 100 Mib -> 1600 Mib
  // 32 KiB of data files 0->32
  // start = 100;
  // end = 1600;
  // step = 100;
  // size_bytes = size_mb;
  // subdir = "mb_large";
  // for (uint32_t i = start; i <= end; i += step) {
  //   write_file(i, size_bytes, subdir);
  // }
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::info);

  // parse args
  parse_args(argc, argv);

  // create input files
  create_input_test_data();

  return 0;
}
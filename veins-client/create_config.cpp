/*
 * create invetory json files
 */
#include "AvswuConfig.h"

#include <getopt.h>
#include <spdlog/spdlog.h>

#include <unistd.h>

#include <nlohmann/json.hpp>
using json = nlohmann::json;

using namespace std;
using namespace avswu_config;

using json = nlohmann::json;

void parse_args(uint32_t &n_manufacturer, uint32_t &n_software_max, int argc,
                char **argv) {
  // parse command line arguments using gnu's getopt

  // n has a parameter "n:" and v does not have a parameter, => "n:v"
  const char *short_options = "m:s:v";
  option long_options[] = {{"n_manufacturer", required_argument, NULL, 'm'},
                           {"n_software_max", required_argument, NULL, 's'},
                           {"verbose", no_argument, NULL, 'v'},
                           {NULL, 0u, NULL, 0u}};
  int option_index = 0;

  int opt;
  string cache_min_str, cache_max_str, c_args, delimiter;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'm':
      n_manufacturer = stoi(optarg);
      spdlog::debug("n_manufacturer={}", n_manufacturer);
      break;
    case 's':
      n_software_max = stoi(optarg);
      spdlog::debug("n_software_max={}", n_software_max);
      break;
    case 'v':
      spdlog::set_level(spdlog::level::trace);
      spdlog::debug("verbose logging is on");
      break;
    case '?':
      spdlog::error("command line error. one or more flags not recognized: {}",
                    char(opt));
      spdlog::info("usage: prog --n_manufacturer n --n_software_max n "
                   "--n_inventory n --verbose");
      break;
    default:
      exit(-1);
    }
  }
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::info);

  // parse args
  string output_dir = string(AVSWU_BASEDIR) + "/veins-client/inventory";
  // set defaults
  uint32_t n_manufacturer = 4;
  uint32_t n_software = 16;
  uint32_t n_inventory = 10;

  parse_args(n_manufacturer, n_software, argc, argv);

  // select data
  // limiting to 1 MiB to support gRPC limit of 4MiB
  // we reproduce generate larger file size durign simulation by scaling the
  // 1MiB of data
  uint32_t input_start = 1;
  uint32_t input_end = 1;
  uint32_t input_step = 1;
  string input_subdir = "mb";

  spdlog::info("n_manufacturer={}", n_manufacturer);
  spdlog::info("n_software_max={}", n_software);
  spdlog::info("output_dir={}", output_dir);

  // create config
  AvswuConfig config(n_manufacturer, n_software, n_inventory, input_start,
                     input_end, input_step, input_subdir);

  // write the config
  const string output_file = output_dir + "/" + "config.json";
  config.write_file(output_file);
  spdlog::info("write output_file={}", output_file);

  return 0;
}
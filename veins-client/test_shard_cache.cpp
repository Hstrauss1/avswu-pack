/*
 * non-concurrent musig for n signers
 */
#include "AvswuGrpcClient.h"
#include "AvswuShardCache.h"

#include <getopt.h>
#include <spdlog/spdlog.h>

using namespace std;
using namespace avswu_grpc_client;
using json = nlohmann::json;

using namespace avswu_shard_cache;

void parse_args(string &file_path, uint32_t &n, uint32_t &cache_size, int argc,
                char **argv) {
  // parse command line arguments using gnu's getopt

  string path = argv[0];
  string prog = path.substr(path.find_last_of("/\\") + 1);

  // f,n,c has a parameter, v does not have a parameter, => "f:n:v"
  const char *short_options = "f:n:c:v";
  option long_options[] = {{"file", required_argument, NULL, 'f'},
                           {"number", required_argument, NULL, 'n'},
                           {"cache_max_size", required_argument, NULL, 'c'},
                           {"verbose", no_argument, NULL, 'v'},
                           {NULL, 0u, NULL, 0u}};
  int option_index = 0;
  int opt;

  while ((opt = getopt_long(argc, argv, short_options, long_options,
                            &option_index)) != -1) {
    switch (opt) {
    case 'f':
      file_path = string(optarg);
      break;
    case 'n':
      n = stoi(optarg);
      spdlog::debug("n={}", n);
      break;
    case 'c':
      cache_size = stoi(optarg);
      break;
    case 'v':
      spdlog::set_level(spdlog::level::trace);
      break;
    case '?':
      spdlog::error("command line error. one or more flags not recognized: {}",
                    char(opt));
      break;
    default:
      spdlog::warn("usage: {} --file path_to_file --number number_of_shards "
                   "--cache_size "
                   "size_of_cache --verbose",
                   prog);
      exit(-1);
    }
  }
}

// make psuedo-random with repeating manufactuer & software values. so, the
void set_random(AvswuPacketShard &packet_shard, size_t i, size_t max,
                size_t original_file_size) {

  // cache map has collisions
  uint64_t m_id = i % max;
  uint64_t sw_id = rand() % max;
  uint64_t ver_id = rand() % max;
  uint64_t sh_id = rand() % max;

  // random array of data to represent the shard
  vector<char> shard_data;
  shard_data.resize(1 + rand() % 20);
  // insert random char from a-z into data vector
  auto rand_a_to_z = [](const int &x) -> char { return ('a' + (rand() % 26)); };
  transform(shard_data.begin(), shard_data.end(), shard_data.begin(),
            rand_a_to_z);

  // create ids
  uint64_t from_id = rand() % 100;
  mpz_t from_X;
  mpz_init(from_X);
  mpz_set_ui(from_X, from_id);
  uint64_t to_id = from_id + 200;
  mpz_t to_X;
  mpz_init(to_X);
  mpz_set_ui(to_X, to_id);

  // set the shard values
  packet_shard.set(from_id, from_X, to_id, to_X, m_id, sw_id, ver_id, sh_id, i,
                   8, 6, 8, shard_data.data(), shard_data.size(),
                   original_file_size);
}

int main(int argc, char **argv) {

  spdlog::set_level(spdlog::level::info);

  // parse file args
  string file_path =
      string(AVSWU_BASEDIR) + "/veins-client/test_data/test_data_10_shard.data";
  uint32_t n = 50;
  uint32_t cache_max_size = 30720;

  parse_args(file_path, n, cache_max_size, argc, argv);

  spdlog::debug("file_path={}", file_path);
  spdlog::debug("n={}", n);
  spdlog::debug("cache_size={}", cache_max_size);

  // argc and argv are handle by getopt, pass argc=1 only program name, and
  // argv
  AvswuGrpcClient client(1u, argv);

  // create n shards
  spdlog::info("creating n={} shards", n);
  vector<AvswuPacketShard> packet_shard_list;
  srand(time(NULL));
  for (size_t i = 0; i < n; i++) {
    AvswuPacketShard s;
    set_random(s, i, 5, 10240);
    packet_shard_list.push_back(s);
  }

  // add shards to the cache
  AvswuShardCache cache(cache_max_size);

  for (size_t i = 0; i < n; i++) {
    try {
      // insert
      spdlog::info("insert shard: i={} shard=({},{},{}) size={}", i,
                   packet_shard_list[i].shard().manufacturer_id(),
                   packet_shard_list[i].shard().software_id(),
                   packet_shard_list[i].shard().shard_id(),
                   packet_shard_list[i].shard().buffer_size());
      cache.insert(packet_shard_list[i]);

      // calculate stats
      uint32_t perc =
          (uint32_t)((float)cache.size() / (float)cache.max_size() * 100);
      spdlog::info("cache space {}% used, size={} of {} bytes ", perc,
                   cache.size(), cache.max_size());
    } catch (const std::runtime_error &e) {
      spdlog::error("exception e={}", e.what());
    }
  }

  // spdlog::debug("cache={}", json(cache).dump(4));

  float hit_perc = ((float)cache.hit() / (cache.hit() + cache.miss())) * 100;
  spdlog::info("hit ={:.2f}%, hit={} miss={}", hit_perc, cache.hit(),
               cache.miss());

  // // test get vector
  // vector<AvswuShard> vec = cache.get();
  // for (size_t i = 0; i < vec.size(); i++) {
  //   // spdlog::debug("vec[{}]={}", i, json(vec[i]).dump(4));
  // }

  // // test get a single shard
  // const uint64_t manufacturer_id = vec[2].manufacturer_id();
  // const uint64_t software_id = vec[2].software_id();
  // const uint64_t shard_id = vec[2].shard_id();
  // AvswuShard shard = cache.get(manufacturer_id, software_id, shard_id);
  // // spdlog::debug("shard={}", json(shard).dump(4));

  // // get all shards from a specific manufacturer
  // vector<AvswuShard> software_shards = cache.get(manufacturer_id,
  // software_id); spdlog::debug("get(manufacturer_id={}, software_id={}) found
  // {} shards",
  //               manufacturer_id, software_id, software_shards.size());
  // for (size_t i = 0; i < software_shards.size(); i++) {
  //   // spdlog::debug("software_shards[{}]={}", i,
  //   // json(software_shards[i]).dump(4));
  // }

  return 0;
}
#pragma once

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>

using namespace std;

namespace avswu_open_ssl {

// enumerated types of hashes
typedef enum hash_type_t : int32_t {
  filesize = -1,
  sha256,
  sha512,
  blake2s256,
  blake2b512
} hash_type_t;

// hash by type
const bool hash_by_type(string &hashed, const string &unhashed,
                        int32_t hash_type);

// for debuging to reduce compute time
const bool hash_filesize(string &hashed, const string &unhashed);

// real hash algs
const bool hash_sha256(string &hashed, const string &unhashed);
const bool hash_sha512(string &hashed, const string &unhashed);
const bool hash_blake2s256(string &hashed, const string &unhashed);
const bool hash_blake2b512(string &hashed, const string &unhashed);
const bool hash_algorithm(string &hashed, const string &unhashed,
                          const EVP_MD *algorithm, const int digest_length);
} // namespace avswu_open_ssl

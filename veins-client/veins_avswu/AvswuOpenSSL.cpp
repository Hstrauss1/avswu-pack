#include "AvswuOpenSSL.h"
#include <iomanip>
#include <iostream>
#include <spdlog/spdlog.h>

#include <openssl/evp.h>
#include <openssl/sha.h>
#include <string>

using namespace std;

namespace avswu_open_ssl {

const bool hash_by_type(string &hashed, const string &unhashed,
                        int32_t hash_type) {
  bool is_success;
  switch (hash_type) {
  case filesize:
    is_success = hash_filesize(hashed, unhashed);
    break;
  case sha256:
    is_success = hash_sha256(hashed, unhashed);
    break;
  case sha512:
    is_success = hash_sha512(hashed, unhashed);
    break;
  case blake2s256:
    is_success = hash_blake2s256(hashed, unhashed);
    break;
  case blake2b512:
    is_success = hash_blake2b512(hashed, unhashed);
    break;
  default:
    spdlog::error("unknown hash type, hash_type ={}", hash_type);
    is_success = false;
    break;
  }
  return is_success;
}

const bool hash_filesize(string &hashed, const string &unhashed) {
  hashed = std::to_string(unhashed.size());
  return true;
}

const bool hash_blake2s256(string &hashed, const string &unhashed) {
  //
  // VEINS WILL NOT BUILD WITH THIS
  //
  // auto *algorithm = EVP_blake2s256();
  // const int digest_length = SHA256_DIGEST_LENGTH;
  // const bool is_success =
  //     hash_algorithm(hashed, unhashed, algorithm, digest_length);

  // return is_success;

  return false;
}

const bool hash_blake2b512(string &hashed, const string &unhashed) {
  //
  // VEINS WILL NOT BUILD WITH THIS
  //
  // auto *algorithm = EVP_blake2b512();
  // const int digest_length = SHA512_DIGEST_LENGTH;
  // const bool is_success =
  //     hash_algorithm(hashed, unhashed, algorithm, digest_length);

  // return is_success;

  return false;
}

const bool hash_sha256(string &hashed, const string &unhashed) {
  auto *algorithm = EVP_sha256();
  const int digest_length = SHA256_DIGEST_LENGTH;
  const bool is_success =
      hash_algorithm(hashed, unhashed, algorithm, digest_length);

  return is_success;
}

const bool hash_sha512(string &hashed, const string &unhashed) {
  auto *algorithm = EVP_sha512();
  const int digest_length = SHA512_DIGEST_LENGTH;
  const bool is_success =
      hash_algorithm(hashed, unhashed, algorithm, digest_length);

  return is_success;
}

// adapted from
// https://stackoverflow.com/questions/51144505/generate-sha-3-hash-in-c-using-openssl-library
const bool hash_algorithm(string &hashed, const string &unhashed,
                          const EVP_MD *algorithm, const int digest_length) {
  bool success = false;

  EVP_MD_CTX *context = EVP_MD_CTX_new();

  if (context != NULL) {
    if (EVP_DigestInit_ex(context, algorithm, NULL)) {
      if (EVP_DigestUpdate(context, unhashed.c_str(), unhashed.length())) {
        unsigned char *hash = new unsigned char[digest_length];
        unsigned int lengthOfHash = 0;

        if (EVP_DigestFinal_ex(context, hash, &lengthOfHash)) {
          std::stringstream ss;
          for (unsigned int i = 0; i < lengthOfHash; ++i) {
            ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
          }

          hashed = ss.str();
          success = true;
        }
        free(hash);
      }
    }
    EVP_MD_CTX_free(context);
  }

  return success;
}

} // namespace avswu_open_ssl

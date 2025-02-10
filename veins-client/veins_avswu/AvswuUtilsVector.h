#pragma once

#include "NonConMusigSigner.h"
#include <gmp.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

using namespace std;
using namespace non_con_musig_signer;
using json = nlohmann::json;

namespace avswu_utils_vector {

// vector methods for verify
void set_L_X(vector<mpz_t *> &L_pk, NonConMusigSigner signer[], const size_t n);

} // namespace avswu_utils_vector

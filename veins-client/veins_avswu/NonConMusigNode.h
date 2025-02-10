#pragma once

#include "AvswuUtils.h"
#include <gmp.h>

using namespace std;
using namespace avswu_utils;

namespace non_con_musig_node {

// non-con-musig signature = (R,s)
typedef struct {
  // aggregated signature R
  mpz_t R;
  // joint signature s
  mpz_t s;
} signature_t;

class NonConMusigNode {

protected:
  // non-concurrent musig parameters
  param_t _par;
  key_pair_t _key_pair;

public:
  NonConMusigNode();
  ~NonConMusigNode();

  // getters/setters for public values
  inline mpz_t &g() { return _par.g; }
  inline mpz_t &X() { return _key_pair.X; }

  // non-con-musig methods
  void param_gen();
  void key_gen();
};

} // namespace non_con_musig_node
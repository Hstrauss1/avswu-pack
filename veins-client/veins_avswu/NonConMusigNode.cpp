#include "NonConMusigNode.h"
#include <gmp.h>
#include <spdlog/spdlog.h>
#include <stdlib.h> /* srand, rand */
#include <time.h>   /* time */

using namespace std;
using namespace avswu_utils;

namespace non_con_musig_node {

// constructors
NonConMusigNode::NonConMusigNode() {
  // create params, generate keys
  param_gen();
  key_gen();
}

// destructor
NonConMusigNode::~NonConMusigNode() {}

// param gen(lambda) -> par = (G,p,g)
void NonConMusigNode::param_gen() {
  mpz_init(_par.g);
  mpz_set_ui(_par.g, PARAM_G);
  mpz_init(_par.p);
  mpz_set_ui(_par.p, PARAM_P);
}

// keyGen(par) -> (pk,sk)
void NonConMusigNode::key_gen() {
  // calculate private key
  _key_pair.x = rand() % PARAM_P;
  // calculate public key
  mpz_init(_key_pair.X);
  mpz_powm_ui(_key_pair.X, _par.g, _key_pair.x, _par.p);

  spdlog::debug("key_gen:_key_pair.x={},_key_pair.X={}", _key_pair.x,
                mpz_to_string(_key_pair.X));
}

} // namespace non_con_musig_node
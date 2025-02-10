#include "NonConMusigSigner.h"
#include <vector>

using namespace std;
using namespace non_con_musig_signer;

namespace avswu_utils_vector {

// vector methods for verify
void set_L_X(vector<mpz_t *> &L_X_ptr, NonConMusigSigner signer[],
             const size_t n) {
  for (size_t i = 0; i < n; i++) {
    L_X_ptr.push_back(&signer[i].X());
  }
}

} // namespace avswu_utils_vector

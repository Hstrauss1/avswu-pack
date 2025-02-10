#include "pbc.h"
#include <stdio.h>

/*
    we wrap any pbc functions that are static inline, since bindgen does not
   export these
    https://users.rust-lang.org/t/bindgen-not-generating-bindings-for-functions/63441
 */

// create a new element
element_t *pbc_wrapper_new_element();

// from pbc_pairing.h
void pbc_wrapper_element_init_G1(element_t e, pairing_t pairing);
// from pbc_field.h
void pbc_wrapper_element_set0(element_t e);

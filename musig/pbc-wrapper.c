#include "pbc-wrapper.h"
#include <stdio.h>

element_t *pbc_wrapper_new_element() {
  element_t *el_ptr;
  el_ptr = malloc(sizeof(element_t));
  return el_ptr;
}

void pbc_wrapper_element_init_G1(element_t e, pairing_t pairing) {
  element_init_G1(e, pairing);
}

void pbc_wrapper_element_set0(element_t e) { element_set0(e); }
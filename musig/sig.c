#include "musig.h"
#include "pbc.h"
#include <stdio.h>

extern element_t g;
extern pairing_t pairing;
extern element_t pklist[signNumber];

int main(void) {

  setup();
  struct Key users[signNumber];
  element_t ri[signNumber], ti[signNumber], Ri[signNumber];
  unsigned char *message = (unsigned char *)"hello musig";
  for (int i = 0; i < signNumber; i++) {
    users[i] = keyGen();
    element_init_G1(pklist[i], pairing);
    element_set(pklist[i], users[i].pk);
  }

  element_t ai[signNumber];
  for (int i = 0; i < signNumber; i++) {
    element_init_Zr(ai[i], pairing);
    calAi(ai[i], pklist[i]);
  }

  element_t apk;
  element_init_G1(apk, pairing);
  aggregateKey(apk, ai);
  for (int i = 0; i < signNumber; i++) {
    element_init_Zr(ri[i], pairing);
    element_init_G1(Ri[i], pairing);
    element_init_Zr(ti[i], pairing);
    struct commitR tmp = generateRi();

    element_set(ri[i], tmp.ri);
    element_set(Ri[i], tmp.Ri);
    element_set(ti[i], tmp.ti);
  }

  element_t R, c, msg;
  element_init_G1(R, pairing);
  calR(R, Ri);

  element_init_Zr(c, pairing);
  element_init_Zr(msg, pairing);
  element_from_bytes(msg, message);
  calC(c, apk, R, msg);

  element_t signi[signNumber];
  element_t sigma;
  element_init_Zr(sigma, pairing);
  for (int i = 0; i < signNumber; i++) {
    element_init_Zr(signi[i], pairing);
    singleSign(signi[i], ri[i], c, ai[i], users[i].sk);
  }
  sign(sigma, signi);
  element_printf("multi-sig(%B,%B)\n", sigma, R);
  verify(sigma, R, apk, c);
  return 0;
}
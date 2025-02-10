#include "musig.h"
#include <pbc.h>
#include <stdio.h>

element_t g;
pairing_t pairing;
element_t pklist[signNumber];

uint8_t helloWorld(uint8_t n) {
  for (size_t i = 0; i < n; i++) {
    printf("hello world\n");
  }
  return n;
}

void setup() {

  char param[1024];
  // full path is needed for rust to find a.param
  char *aParamFile = "./param/a.param";
  FILE *fp = fopen(aParamFile, "r");
  if (fp == NULL) {
    printf("FATAL ERROR: Unable to open a.param file\n");
    exit(-1);
  }
  size_t count = fread(param, 1, 1024, fp);
  if (!count)
    pbc_die("读取参数失败");
  fclose(fp);
  pairing_init_set_buf(pairing, param, count);
  element_init_G1(g, pairing);
  element_random(g);
}

struct Key keyGen() {
  struct Key tmp;
  element_init_Zr(tmp.sk, pairing);
  element_random(tmp.sk);
  element_init_G1(tmp.pk, pairing);
  element_pow_zn(tmp.pk, g, tmp.sk);
  return tmp;
}

// ai = H(pk_i,{pk_1,...,pk_i})
void calAi(element_t ai, element_t pk) {
  element_t temp;
  element_init_Zr(temp, pairing);
  element_add(temp, temp, pk);
  for (int i = 0; i < signNumber; i++) {
    element_add(temp, temp, pklist[i]);
  }
  element_from_hash(ai, temp, element_length_in_bytes(temp));
}

struct commitR generateRi() {
  struct commitR tmp;
  element_init_Zr(tmp.ri, pairing);
  element_random(tmp.ri);
  element_init_G1(tmp.Ri, pairing);
  element_pow_zn(tmp.Ri, g, tmp.ri);
  element_init_Zr(tmp.ti, pairing);

  element_from_hash(tmp.ti, tmp.Ri, element_length_in_bytes(tmp.Ri));
  return tmp;
}

void calR(element_t R, element_t RiPar[signNumber]) {
  element_t tmp;
  element_init_G1(tmp, pairing);
  for (size_t i = 0; i < signNumber; i++) {
    element_mul(tmp, tmp, RiPar[i]);
  }
  element_set(R, tmp);
  element_clear(tmp);
}

void calC(element_t c, element_t apk, element_t R, element_t message) {
  element_t tmp;
  element_init_Zr(tmp, pairing);
  element_add(tmp, tmp, apk);
  element_add(tmp, tmp, R);
  element_add(tmp, tmp, message);
  element_from_hash(c, tmp, element_length_in_bytes(tmp));
  element_clear(tmp);
}

void aggregateKey(element_t apk, element_t aiPar[signNumber]) {
  element_t temp, t;
  element_init_G1(temp, pairing);
  element_init_G1(t, pairing);
  for (int i = 0; i < signNumber; i++) {
    element_pow_zn(t, pklist[i], aiPar[i]);
    element_mul(temp, temp, t);
  }
  element_set(apk, temp);
  element_clear(temp);
  element_clear(t);
}

void singleSign(element_t si, element_t ri, element_t c, element_t ai,
                element_t xi) {
  element_t tmp, axc;
  element_init_Zr(tmp, pairing);
  element_init_Zr(axc, pairing);
  element_mul(tmp, ai, xi);
  element_mul(axc, tmp, c);
  element_add(si, axc, ri);
  // element_set(si,tmp);
  element_clear(tmp);
  element_clear(axc);
}

void sign(element_t s, element_t si[signNumber]) {
  for (int i = 0; i < signNumber; i++) {
    element_add(s, s, si[i]);
  }
}

int verify(element_t sigma, element_t R, element_t apk, element_t c) {
  element_t left, xc, right;
  element_init_G1(left, pairing);
  element_init_G1(xc, pairing);
  element_init_G1(right, pairing);

  element_pow_zn(left, g, sigma);
  element_pow_zn(xc, apk, c);
  element_mul(right, R, xc);
  return element_cmp(left, right);
}

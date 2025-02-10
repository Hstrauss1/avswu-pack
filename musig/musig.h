#include <pbc.h>

#define signNumber 5

struct Key {
  element_t pk;
  element_t sk;
};

struct commitR {
  element_t ri;
  element_t Ri;
  element_t ti;
};

// musig funcs
void setup();
struct Key keyGen();
void calAi(element_t ai, element_t pk);
struct commitR generateRi();
void calR(element_t R, element_t RiPar[signNumber]);
void calC(element_t c, element_t apk, element_t R, element_t message);
void aggregateKey(element_t apk, element_t aiPar[signNumber]);
void singleSign(element_t si, element_t ri, element_t c, element_t ai,
                element_t xi);
void sign(element_t s, element_t si[signNumber]);
int verify(element_t sigma, element_t R, element_t apk, element_t c);

// test functions
uint8_t helloWorld(uint8_t n);

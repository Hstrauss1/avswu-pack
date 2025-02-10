/*
 * header file for interface into bethencourt cp-abe
 */

namespace cpabe {

    struct setupKeysType {
      GByteArray *publicKeyByteArrayPtr;
      GByteArray *masterKeyByteArrayPtr;
    };

    struct encryptResultType {
      int fileLen;
      GByteArray* cipherBufByteArray;
      GByteArray* aesBufByteArray;
    };

    struct decryptResultType {
      int fileLen;
      GByteArray* fileBufByteArray;
    };

    extern setupKeysType *cpabeSetup();
    extern GByteArray *cpabeKeyGen(GByteArray *publicKeyByteArray, GByteArray *masterKeyByteArray, char *attributes);
    extern encryptResultType *cpabeEncrypt(GByteArray *dataGByteArray, GByteArray *publicKeyGByteArray, char *policy);
    extern decryptResultType *cpabeDecrypt(unsigned int fileLen, GByteArray *cipherGByteArray, 
        GByteArray *aesGByteArray, GByteArray *publicKeyGByteArray, GByteArray *privateKeyGByteArray);

};

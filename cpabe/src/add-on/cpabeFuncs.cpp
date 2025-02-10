/*
 * v8 C++ interface into bethencourt's libraries 
 */
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <string>

#include <string.h>
#include <unistd.h>
#include <glib.h>
#include <pbc.h>
#include <pbc_random.h>

#include "bswabe.h"
extern "C" {
#include "common.h"
}
extern "C" {
#include "policy_lang.h"
}

#include "nodeAddOnUtils.hpp"
#include "cpabeFuncs.hpp"

using namespace std;

namespace cpabe {

	// performs cpabe setup a public key and master key
	setupKeysType *cpabeSetup() {

		bswabe_pub_t* publicKey;
		bswabe_msk_t* masterKey;

		bswabe_setup(&publicKey, &masterKey);

		setupKeysType *keysPtr = new setupKeysType;

		// serialize keys into byte arrays
		keysPtr->publicKeyByteArrayPtr = bswabe_pub_serialize(publicKey);
		keysPtr->masterKeyByteArrayPtr = bswabe_msk_serialize(masterKey);

		return keysPtr;

	}

    // compare strings
	gint comp_string( gconstpointer a, gconstpointer b) {
		return strcmp((char *) a, (char *) b);
	}

	// replce word, from geeks for geeks
	char* replaceWord(const char* s, const char* oldW, 
					const char* newW) { 
		char* result; 
		int i, cnt = 0; 
		int newWlen = strlen(newW); 
		int oldWlen = strlen(oldW); 
	
		// Counting the number of times old word 
		// occur in the string 
		for (i = 0; s[i] != '\0'; i++) { 
			if (strstr(&s[i], oldW) == &s[i]) { 
				cnt++; 
	
				// Jumping to index after the old word. 
				i += oldWlen - 1; 
			}
		}
	
		// Making new string of enough length 
		result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1); 
	
		i = 0; 
		while (*s) { 
			// compare the substring with the result 
			if (strstr(s, oldW) == s) { 
				strcpy(&result[i], newW); 
				i += newWlen; 
				s += oldWlen; 
			} 
			else
				result[i++] = *s++; 
		} 
	
		result[i] = '\0'; 
		return result; 
	}

	// improved more robust comma-separated parsing of cpabe attributes
	void improvedParse(GSList **alistPtr, char *attributes) {

		

		// add attributes to alist.  (use , to sepearate attributes)

		// first token
   		char *token = strtok(attributes, ",");

		// remove spaces from token
		token = replaceWord(token," ","");

		// if = operator, add space around = for parse_attribute
		token = replaceWord(token,"="," = ");

		// printf( "token = <%s>\n", token );

		// add attribute to alist
		if (token != NULL) {
			cout << "STATUS: parse_attribute token = " << token << endl;
			parse_attribute(alistPtr, token);
			cout << "STATUS: done" << endl;
		}

		// loop through the string to extract attributes
		while( token != NULL) {

			token = strtok(NULL, ",");

			if (token == NULL) {
				break;
			}
			
			// printf( "token = <%s>\n", token );

			// remove spaces from token
			token = replaceWord(token," ","");

			// if = operator, add space around = for parse_attribute
			token = replaceWord(token,"="," = ");

			// printf( "token = <%s>\n", token );

			// add attribute to alist
			if (token != NULL) {
				parse_attribute(alistPtr, token);
			}

		}

		

	}

	//  performs cpabe keygen with public key, master key, & attributes to generates the private key
	GByteArray *cpabeKeyGen(GByteArray *publicKeyByteArray, GByteArray *masterKeyByteArray, char *attributes) {

		bswabe_pub_t* publicKey;
		bswabe_msk_t* masterKey;
		bswabe_prv_t* privateKey;

		// cout << "publicKeyByteArray->len = " << publicKeyByteArray->len << endl;
		// cout << "masterKeyByteArray->len = " << masterKeyByteArray->len << endl;

		// deserialize public, the 1 tells it to free the array after deserializes it, 0 to not free it
		publicKey = bswabe_pub_unserialize(publicKeyByteArray, 0);
		masterKey = bswabe_msk_unserialize(publicKey, masterKeyByteArray, 0);

		// parse the attributes
		char** attrs = 0;
		int i;
		GSList* alist;
		GSList* ap;
		int n;

		alist = 0;

		// parse comma separated attributes and add them to alist
		improvedParse(&alist, attributes);


		alist = g_slist_sort(alist, comp_string);
		n = g_slist_length(alist);

		// printf("attributes = %s\n", attributes);

		// printf("n = %d\n", n);

		attrs = (char **) malloc((n + 1) * sizeof(char*));

		i = 0;
		for( ap = alist; ap; ap = ap->next )
			attrs[i++] = (char *) ap->data;
		attrs[i] = 0;

		// for (i=0; i<n; i++) {
		// 	printf("attrs[%d] = %s\n", i, attrs[i]);
		// }

		// call cpabe keygen routine
		privateKey = bswabe_keygen(publicKey, masterKey, attrs);

		// serialize private byte arrays
		GByteArray *privateKeyByteArray;
		privateKeyByteArray = bswabe_prv_serialize(privateKey);

		// cout << "privateKeyByteArray->len = " << privateKeyByteArray->len << endl;
		// cout << "privateKeyByteArray->data        = " << privateKeyByteArray->data << endl;

		return privateKeyByteArray;

	}


	// performs cpabe encrypt with public key, master key, & attributes to generates the private key
	encryptResultType *cpabeEncrypt(GByteArray *dataGByteArray, GByteArray *publicKeyGByteArray, char *policy) {

		bswabe_pub_t* pub;
		bswabe_cph_t* cph;
		int file_len;
		GByteArray* plt;
		GByteArray* cph_buf;
		GByteArray* aes_buf;
		element_t m;

		// the plain text to be encrypted
		plt = dataGByteArray;

		// deserialize public, the 1 tells it to free the array after deserializes it, 0 to not free it
		pub = bswabe_pub_unserialize(publicKeyGByteArray, 0);

		// parse the policy
		char *policyParsed = parse_policy_lang(policy);

		// printf("policyParsed = %s\n", policyParsed);

		if( !(cph = bswabe_enc(pub, m, policyParsed)) )
				die((char *) "%s", bswabe_error());

		cph_buf = bswabe_cph_serialize(cph);
		

		file_len = plt->len;
		aes_buf = aes_128_cbc_encrypt(plt, m);
		g_byte_array_free(plt, 1);
		element_clear(m);



		encryptResultType *resultPtr = new encryptResultType;
		resultPtr->fileLen = file_len;
		resultPtr->cipherBufByteArray = cph_buf;
		resultPtr->aesBufByteArray = aes_buf;

		// memory clean up
		free(policyParsed);
		bswabe_pub_free(pub);

		return resultPtr;

	}

	// performs cpabe decrypt and returns plaintext byte array
	decryptResultType *cpabeDecrypt(
		unsigned int fileLen,
		GByteArray *cipherGByteArray, GByteArray *aesGByteArray,
		GByteArray *publicKeyGByteArray, GByteArray *privateKeyGByteArray
	) {

		bswabe_pub_t* pub;
		bswabe_prv_t* prv;
		int file_len;
		GByteArray* aes_buf;
		GByteArray* plt;
		GByteArray* cph_buf;
		bswabe_cph_t* cph;
		element_t m;

		// parse arguments into decrypt algoritrhm
		file_len = fileLen;
		cph_buf = cipherGByteArray;
		aes_buf = aesGByteArray;

		// decrypt result struct
		decryptResultType *resultPtr = new decryptResultType;
		resultPtr->fileLen = 0;
		resultPtr->fileBufByteArray = g_byte_array_new();


		pub = bswabe_pub_unserialize(publicKeyGByteArray, 0);

		// cout << "privateKeyGByteArray = " << privateKeyGByteArray << endl;
		// cout << "privateKeyGByteArray->len = " << privateKeyGByteArray->len << endl;
		// cout << "privateKeyGByteArray->data = " << privateKeyGByteArray->data << endl;

		prv = bswabe_prv_unserialize(pub, privateKeyGByteArray, 0);

		cph = bswabe_cph_unserialize(pub, cph_buf, 0);

		// if unable to decript due to attrivutes not satisfying the policy, return empty byte array
		if( !bswabe_dec(pub, prv, cph, m) ) {
			// die((char *) "%s", bswabe_error());
			printf("\n-----------------------------------------------\n");
			printf("cpabe ERROR: %s", bswabe_error());
			printf("-----------------------------------------------\n\n");

			// return error code of -1
			resultPtr->fileLen = -1;

			bswabe_cph_free(cph);

			return resultPtr;

		}

		bswabe_cph_free(cph);

		plt = aes_128_cbc_decrypt(aes_buf, m);

		g_byte_array_set_size(plt, file_len);

		// set result structure
		resultPtr->fileLen = file_len;
		resultPtr->fileBufByteArray = plt;

		return resultPtr;

	}

};
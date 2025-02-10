#include <nan.h>

#include <stdio.h>
#include <iostream>
#include <string>

#include <stdlib.h>
#include <string>

#include <unistd.h>

#include <glib.h>
#include <pbc.h>
#include <pbc_random.h>

#include "bswabe.h"
#include "common.h"
#include "policy_lang.h"

#include "nodeAddOnUtils.hpp"
#include "cpabeFuncs.hpp"

using namespace std;
using namespace Nan;
using namespace v8;

//
// CPABESetup() => { masterKey, publicKey }
//
NAN_METHOD(CPABESetup) {

    // check length of input to function
    if ( info.Length() != 0 ) {

        cerr << "ERROR: cpabeSetup() must be called with no arguments." << endl;
        return;

    }

    // cout << "STATUS: cpabeSetup() executing in C++ and C" << endl;

    // call cpabe functions
    cpabe::setupKeysType *setupKeysPtr = cpabe::cpabeSetup();

    // cout << "STATUS: C++ cpabeSetup() setupKeysPtr = " << std::hex << setupKeysPtr << endl;

    // create the return object
    Local<Object> retObj = Nan::New<Object>();
    
    // return master and public key data
    v8::Isolate* isolate = info.GetIsolate();
    Util_NanSetReturnObject(isolate, retObj, "masterKey", setupKeysPtr->masterKeyByteArrayPtr);
    Util_NanSetReturnObject(isolate, retObj, "publicKey", setupKeysPtr->publicKeyByteArrayPtr);

    // returns the object
    info.GetReturnValue().Set(retObj);

    // memory clean up
    g_byte_array_free(setupKeysPtr->publicKeyByteArrayPtr, 1);
    g_byte_array_free(setupKeysPtr->masterKeyByteArrayPtr, 1);


}



//
// CAPBEKeyGen({publicKey, masterKey, attributes}) => { privbateKey }
// attributes are of the format "a1 a2 'a3 = num'" where num is always a non-negative whole number
// non-simple attributes (like 'a3 = 23534') produce larger keys
//
NAN_METHOD(CPABEKeyGen) {
    
    // check length of input to function
    if ( info.Length() != 5 ) {

        cerr << "ERROR: cpabeKeyGen() must be called with 5 arguments which are "  <<
            "publicKey, publicKeyLength, masterKey, masterKeyLength, attributes" << endl;
        return;

    }

    // cout << "STATUS: cpabeKeyGen() executing in C++ and C" << endl;

    //
    // parse arguments
    //

    // get property names to access the input object fields

    //
    // parse buffer input args and convert them into GByteArrays
    //
    GByteArray *publicKeyGByteArray = Util_NanGetGByteArray(info, 0, 1);
    GByteArray *masterKeyGByteArray = Util_NanGetGByteArray(info, 2, 3);

    //
    // attribute string parse argument
    //

    v8::Local<v8::String> attributes = info[4]->ToString(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::String>());
    v8::Isolate* isolate = info.GetIsolate();
    char* attributesCStr = (char *) Util_v8StrToCStr(isolate, attributes);
 
    // cout << "attributesCStr = " << attributesCStr << endl;


    //
    // call cpabe functions
    //

    GByteArray *privateKeyByteArray = cpabe::cpabeKeyGen(publicKeyGByteArray, masterKeyGByteArray, attributesCStr);

    //
    // return object
    //

    // create the return object
    Local<Object> retObj = Nan::New<Object>();

    // return private key data
    Util_NanSetReturnObject(isolate, retObj, "privateKey", privateKeyByteArray);
    
    // returns the object
    info.GetReturnValue().Set(retObj);

    // memory clean up
    g_byte_array_free(privateKeyByteArray, 1);


}

//
// CPABEEncrypt
// {data, public keey, polict } => encrypted data
//
NAN_METHOD(CPABEEncrypt) {

    // check length of input to function
    if ( info.Length() != 5 ) {

        cerr << "ERROR: cpabeEncrypt() must be called with 5 arguments which are "  <<
            "data, dataLength, publicKey, publicKeyLength, policy" << endl;
        return;

    }

    // cout << "STATUS: cpabeEncrypt() executing in C++ and C" << endl;

    //
    // parse buffer input args and convert them into GByteArrays
    //
    GByteArray *dataGByteArray = Util_NanGetGByteArray(info, 0, 1);
    GByteArray *publicKeyGByteArray = Util_NanGetGByteArray(info, 2, 3);

    //
    // policy string parse argument
    //

    v8::Local<v8::String> policy = info[4]->ToString(Nan::GetCurrentContext()).FromMaybe(v8::Local<v8::String>());
    v8::Isolate* isolate = info.GetIsolate();
    char* policyCStr = Util_v8StrToCStr(isolate, policy);
 

    cpabe::encryptResultType *result = cpabe::cpabeEncrypt(dataGByteArray, publicKeyGByteArray, policyCStr);

    //
    // return object
    //

    // create the return object
    Local<Object> retObj = Nan::New<Object>();

    // set the properties of the return object

    //
    // create result structure
    //

    // create the property/field name
    Local<String> fileLenProp = Nan::New<String>("fileLen").ToLocalChecked();
    // set the property in the return object
    Nan::Set(retObj, fileLenProp, Nan::New<Number>(result->fileLen));

    // return cipher buffer and aes buffer
    Util_NanSetReturnObject(isolate, retObj, "cipherBufByteArray", result->cipherBufByteArray);
    Util_NanSetReturnObject(isolate, retObj, "aesBufByteArray", result->aesBufByteArray);
    
    // returns the object
    info.GetReturnValue().Set(retObj);

    // memory clean up
    g_byte_array_free(result->cipherBufByteArray, 1);
    g_byte_array_free(result->aesBufByteArray, 1);


}

//
// CPABEDecrypt
// {data, public keey, polict } => encrypted data
//
NAN_METHOD(CPABEDecrypt) {

    // check length of input to function
    if ( info.Length() != 9 ) {

        cerr << "ERROR: cpabeDecrypt() must be called with 9 arguments which are "  <<
            "fileLen, cipherInt8Array, cipherInt8Array.length, aesInt8Array, aesInt8Array.length, publicKeyInt8Array, " << 
            "publicKeyInt8Array.length, privateKeyInt8Array, privateKeyInt8Array.length" << endl;
        return;

    }


    // cout << "STATUS: cpabeDecrypt() executing in C++ and C" << endl;

    //
    // file length string parse argument
    //
    // get the context
    v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();
    unsigned int fileLen = info[0]->NumberValue(context).FromJust();

    //
    // parse buffer input args and convert them into GByteArrays
    //
    GByteArray *cipherGByteArray = Util_NanGetGByteArray(info, 1, 2);
    GByteArray *aesGByteArray = Util_NanGetGByteArray(info, 3, 4);
    GByteArray *publicKeyGByteArray = Util_NanGetGByteArray(info, 5, 6);
    GByteArray *privateKeyGByteArray = Util_NanGetGByteArray(info, 7, 8);

    // call cpabe decrypt


    // cout << "privateKeyGByteArray = " << privateKeyGByteArray << endl;
    // cout << "privateKeyGByteArray->len = " << privateKeyGByteArray->len << endl;
    // cout << "privateKeyGByteArray->data = " << privateKeyGByteArray->data << endl;

    // cout << "publicKeyGByteArray = " << publicKeyGByteArray << endl;
    // cout << "publicKeyGByteArray->len = " << publicKeyGByteArray->len << endl;
    // cout << "publicKeyGByteArray->data = " << publicKeyGByteArray->data << endl;

    cpabe::decryptResultType *result
        = cpabe::cpabeDecrypt(fileLen, cipherGByteArray, aesGByteArray, publicKeyGByteArray, privateKeyGByteArray);


    //
    // return object
    //

    // create the return object
    Local<Object> retObj = Nan::New<Object>();

    // set the properties of the return object

    //
    // create result structure
    //

    // create the property/field name
    Local<String> fileLenProp = Nan::New<String>("fileLen").ToLocalChecked();
    // set the property in the return object
    Nan::Set(retObj, fileLenProp, Nan::New<Number>(result->fileLen));

    // return file buffer
    v8::Isolate* isolate = info.GetIsolate();
    Util_NanSetReturnObject(isolate, retObj, "fileBufByteArray", result->fileBufByteArray);
    
    // returns the object
    info.GetReturnValue().Set(retObj);

    // memory clean up
    g_byte_array_free(result->fileBufByteArray, 1);


}


NAN_MODULE_INIT(Init) {

    Nan::Set(target, New<String>("cpabeSetup").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(CPABESetup)).ToLocalChecked());

    Nan::Set(target, New<String>("cpabeKeyGen").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(CPABEKeyGen)).ToLocalChecked());

    Nan::Set(target, New<String>("cpabeEncrypt").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(CPABEEncrypt)).ToLocalChecked());

    Nan::Set(target, New<String>("cpabeDecrypt").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(CPABEDecrypt)).ToLocalChecked());

}

NODE_MODULE(NODE_GYP_MODULE_NAME, Init)
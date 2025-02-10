//
// Utility functions to ease parsing of arguments and data converfsions
//
#include <iterator>
#include <nan.h>

#include <glib.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "nodeAddOnUtils.hpp"

using namespace std;
using namespace Nan;
using namespace v8;

// convert v8::String::Utf8Value -> c++ string
std::string Util_v8StrToCppStr(v8::Isolate *isolate,
                               v8::Local<v8::String> v8Str) {

  v8::String::Utf8Value temp(isolate, v8Str);

  std::string cppStr(*temp);

  return cppStr;
}

// convert c++ string to v8 string
v8::Local<v8::String> Util_cppStrTov8(v8::Isolate *isolate,
                                      std::string cppStr) {
  v8::Local<v8::String> result =
      String::NewFromUtf8(isolate, cppStr.c_str(), NewStringType::kNormal)
          .ToLocalChecked();
  return result;
}

// convert v8 string to c string
char *Util_v8StrToCStr(v8::Isolate *isolate, v8::Local<v8::String> v8Str) {

  v8::String::Utf8Value temp(isolate, v8Str);

  std::string cppStr(*temp);

  char *cStr = new char[cppStr.length() + 1];
  std::strcpy(cStr, cppStr.c_str());

  return cStr;
}

// is the attribute a valid attribute
bool Util_isAttribute(v8::Isolate *isolate, v8::Local<v8::Object> inputObj,
                      std::string attribute) {

  Local<String> attributeLocal = Nan::New<String>(attribute).ToLocalChecked();
  v8::Local<v8::Context> context = isolate->GetCurrentContext();
  v8::Maybe<bool> mb = inputObj->HasOwnProperty(context, attributeLocal);
  bool isAttribute = mb.FromJust();

  return isAttribute;
}

// parse js object into c++ data types
std::string Util_AttributeToString(v8::Isolate *isolate,
                                   v8::Local<v8::Object> inputObj,
                                   std::string attribute) {

  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  Local<String> attributeLocal = Nan::New<String>(attribute).ToLocalChecked();
  v8::MaybeLocal<v8::String> valMaybe =
      Nan::Get(inputObj, attributeLocal).ToLocalChecked()->ToString(context);
  v8::Local<v8::String> valLocal = valMaybe.ToLocalChecked();
  std::string val = Util_v8StrToCppStr(isolate, valLocal);

  return val;
}

double Util_AttributeToDouble(v8::Isolate *isolate,
                              v8::Local<v8::Object> inputObj,
                              std::string attribute) {

  v8::Local<v8::Context> context = isolate->GetCurrentContext();

  Local<String> attributeLocal = Nan::New<String>(attribute).ToLocalChecked();
  v8::Maybe<double> valMaybe =
      Nan::Get(inputObj, attributeLocal).ToLocalChecked()->NumberValue(context);
  double val = valMaybe.FromJust();

  return val;
}

// adds property and gbytearray data to a return object
void Util_NanSetReturnObject(v8::Isolate *isolate,
                             v8::Local<v8::Object> &returnObject,
                             const char *propertyName, GByteArray *byteArray) {

  // create the property/field name
  Local<String> prop = Nan::New<String>(propertyName).ToLocalChecked();

  // create an array buffer for the bytes in a v8 browser compatible way
  Local<ArrayBuffer> arrayBuf = v8::ArrayBuffer::New(isolate, byteArray->len);

  // copy the data out of the data byte structure into v8 array buffer
  memcpy(arrayBuf->GetContents().Data(), byteArray->data, byteArray->len);

  // set the property in the return object
  Nan::Set(returnObject, prop, arrayBuf);
}

// adds property and vector of chars into a return object
void Util_NanSetReturnBuffer(v8::Isolate *isolate,
                             v8::Local<v8::Object> &returnObject,
                             const char *propertyName,
                             std::vector<char> buffer) {

  // create the property/field name
  Local<String> prop = Nan::New<String>(propertyName).ToLocalChecked();

  // create an array buffer for the bytes in a v8 browser compatible way
  Local<ArrayBuffer> arrayBuf = v8::ArrayBuffer::New(isolate, buffer.size());

  // copy the data out of the data byte structure into v8 array buffer
  memcpy(arrayBuf->GetContents().Data(), buffer.data(), buffer.size());

  // set the property in the return object
  Nan::Set(returnObject, prop, arrayBuf);
}

// gets argument passed as a byte array (byte array, length)
GByteArray *
Util_NanGetGByteArray(const Nan::FunctionCallbackInfo<v8::Value> &info,
                      unsigned int argDataIndex, unsigned int argLenIndex) {

  // get context
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

  // get v8 arguments
  v8::Local<v8::Object> arg0 =
      info[argDataIndex]->ToObject(context).ToLocalChecked();

  // convert v8 object into char byte buffer
  unsigned char *bufferChar = (unsigned char *)node::Buffer::Data(arg0);

  // get integer length of buffer
  unsigned int bufferLength =
      info[argLenIndex]->NumberValue(context).FromJust();

  // create new gbyte array with size
  GByteArray *result;
  result = g_byte_array_sized_new(bufferLength);

  // copy data into char buffer into gbytearray, and set gbytearray length
  memcpy(result->data, bufferChar, bufferLength);
  result->len = bufferLength;

  return result;
}

std::vector<char>
Util_NanGetByteBuffer(const Nan::FunctionCallbackInfo<v8::Value> &info,
                      unsigned int argDataIndex, unsigned int argLenIndex) {

  // get context
  v8::Local<v8::Context> context = info.GetIsolate()->GetCurrentContext();

  // get v8 arguments
  v8::Local<v8::Object> arg0 =
      info[argDataIndex]->ToObject(context).ToLocalChecked();

  // convert v8 object into char byte buffer
  unsigned char *bufferChar = (unsigned char *)node::Buffer::Data(arg0);

  // get integer length of buffer
  unsigned int bufferLength =
      info[argLenIndex]->NumberValue(context).FromJust();

  // data to c++ string
  std::vector<char> buffer(bufferChar, bufferChar + bufferLength);

  return buffer;
}

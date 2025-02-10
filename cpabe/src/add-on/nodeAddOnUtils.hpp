//
// Utility functions to ease parsing of arguments and data converfsions
//
#include <nan.h>

#include <stdio.h>
#include <iostream>
#include <string>
#include <stdlib.h>

#include <glib.h>

#include <v8.h>

// string conversions
std::string Util_v8StrToCppStr(v8::Isolate* isolate, v8::Local<v8::String> v8Str);
v8::Local<v8::String> Util_cppStrTov8(v8::Isolate* isolate, std::string cppStr);
char *Util_v8StrToCStr(v8::Isolate* isolate, v8::Local<v8::String> v8Str);

// js object -> c++ attributes
bool Util_isAttribute(v8::Isolate* isolate, v8::Local<v8::Object> inputObj, std::string attribute);
std::string Util_AttributeToString(v8::Isolate* isolate, v8::Local<v8::Object> inputObj, std::string attribute);
double Util_AttributeToDouble(v8::Isolate* isolate, v8::Local<v8::Object> inputObj, std::string attribute);

// buffer tools
void Util_NanSetReturnObject(v8::Isolate* isolate, v8::Local<v8::Object> &returnObject, const char* propertyName, GByteArray *byteArray);
void Util_NanSetReturnBuffer(v8::Isolate* isolate, v8::Local<v8::Object> &returnObject, const char* propertyName, std::vector<char> buffer);

GByteArray *Util_NanGetGByteArray(const Nan::FunctionCallbackInfo<v8::Value>& info, unsigned int argDataIndex, unsigned int argLenIndex);
std::vector<char> Util_NanGetByteBuffer(const Nan::FunctionCallbackInfo<v8::Value>& info, unsigned int argDataIndex, unsigned int argLenIndex);

#pragma once
#include "types.h"
#include "variables.h"

//const values
extern const Value VALUE_EMPTY;
extern const Value VALUE_TRUE;
extern const Value VALUE_FALSE;
extern const Value VALUE_VOID;
extern const Value VALUE_UNSET;

//frees any pointer that may be held in a value object
void freeValue(Value in);
//change any integers to doubles
Value tryCastValue(Value in, ValueType type);
//duplicate a value, duplicating malloc()ed data. It is caller's responsibility to free the new pointer
Value dupVal(Value in);
//Check whether a value has malloc()ed data, and add it to the environment if so
void addValToEnv(Environment *env, Value in);
//get a string for a value type
const char* getValueTypeString(ValueType v);
//add two values
Value addValues(Value left, Value right);
Value subValues(Value left, Value right);
Value multValues(Value left, Value right);
Value divValues(Value left, Value right);
Value modValues(Value left, Value right);

//conditional helpers
bool isTruthy(Value val);
//evaluate whether a given expression between two values is true
bool evalExpression(Value left, Value right, TokenType comparator);
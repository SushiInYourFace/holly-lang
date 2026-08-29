#pragma once
#include "types.h"

//const values
extern const Value VALUE_EMPTY;
extern const Value VALUE_TRUE;
extern const Value VALUE_FALSE;




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
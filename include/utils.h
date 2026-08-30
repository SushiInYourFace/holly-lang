#pragma once

#include <time.h>

#include "parsing/parsing.h"
#include "types.h"

//macros

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

double getElapsedUs(struct timespec start, struct timespec end);

//return a string corresponding to a token type
const char* getTokenString(TokenType t);
//return a string corresponding to a token category
const char* getTokenCatString(TokenCategory t);

//return whether a given string is a valid variable name following the x_foo form
bool isValidVarName(char* name);
//return whether a given string is a valid function name following the x:foo form
bool isValidFunName(char *name);
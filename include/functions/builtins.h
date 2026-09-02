#pragma once

#include "parsing/parsing.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

//add builtin functions to the function hash
void initBuiltins(FunctionEntry **hash);
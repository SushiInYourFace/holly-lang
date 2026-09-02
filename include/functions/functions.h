#pragma once

#include "parsing/parsing.h"
#include "types.h"

extern const ParamNames EMPTY_PARAMS;

//functions

//get a function return type by parsing name
ValueType getFunReturnFromName(char* name);
//add a function to a functionEntry hashmap
void addFunctionToHash(char* fun_name, ParamNames params, TreeNode *fun_block, FunctionEntry **hash, ValueType return_type); 
//completely clean a function hash
void cleanFunctionHash(FunctionEntry **hash);
//find a function with a given name
FunctionEntry *findFunction(char* fun_name, FunctionEntry *hash);
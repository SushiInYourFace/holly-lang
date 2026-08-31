#pragma once

#include "types.h"
#include "uthash/uthash.h"
#include <stdbool.h>

//forward declare type prefixes

//hashmap layout for variables
typedef struct {
    char* var_name;
    bool final;
    Value value;
    UT_hash_handle hh;
} VarEntry;

//forward declare
typedef struct Environment Environment;

struct Environment{
    Environment *parent;
    VarEntry *var_table;
    bool should_return;
    bool should_break;
    Value return_val;
};

typedef struct EnvSketch EnvSketch;

typedef struct {
    char c;
    ValueType type;
} TypePrefix;

extern const TypePrefix type_prefixes[];
extern const size_t type_prefixes_len;

//add a variable to the var hash
void addVarToHash(Environment *env, char* var_name, Value var_val, bool final);
//add a variable to the env WITHOUT setting a value
void addUnsetVarToHash(Environment *env, char* var_name);
//change a non-final var into a final one
void finalizeVar(Environment *env, char* var_name);
//update the value of a var
void updateVarValue(Environment *env, char* var_name, Value new_val);
//Return a var's value as an int
int64_t getIntVarValue(Environment *env, char* var_name);
//return a var's value as a value struct
Value getVarValue(Environment *env, char* var_name);
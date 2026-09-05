#pragma once

#include "types.h"
#include "uthash/uthash.h"
#include <stdbool.h>

#define ENV_STRING_ARR_LEN (16) //how much space gets allocated for string storage when a new environment is created

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
    struct {
        char** arr;
        size_t len;
        size_t max;
    } strings;
};

//return for a var lookup. Gives the var entry itself as well as the environment which owns it
typedef struct VarLookup {
    VarEntry *var;
    Environment *owner;
}VarLookup;

typedef struct {
    char c;
    ValueType type;
} TypePrefix;

extern const TypePrefix type_prefixes[];
extern const size_t type_prefixes_len;

//set up an environment
void initEnv(Environment *env, Environment *parent);
//clean an environment, removing all variables and string storage
void cleanEnv(Environment *env);
//add a variable to the var hash
void addVarToHash(Environment *env, char* var_name, Value var_val, bool final);
//add a variable to the env WITHOUT setting a value
void addUnsetVarToHash(Environment *env, char* var_name);
//add a new array to the env, leaving values unset
void addArrayVarToHash(Environment *env, char* var_name, size_t members);
//change a non-final var into a final one
void finalizeVar(Environment *env, char* var_name);
//update the value of a var
void updateVarValue(Environment *env, char* var_name, Value new_val);
//update the value of an array member
void updateVarValueAtIndex(Environment *env, char* var_name, Value new_val, size_t index);
//Return a var's value as an int
int64_t getIntVarValue(Environment *env, char* var_name);
//return a var's value as a value struct
Value getVarValue(Environment *env, char* var_name);
//given an array variable, get the value at the provided index
Value getVarArrayValueAtPos(Environment *env, char* var_name, size_t index);
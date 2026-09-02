#include "errors.h"
#include "variables.h"
#include "logging.h"
#include "types.h"
#include "utils.h"
#include "values.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

const VarLookup VAR_NOT_FOUND = {.owner = NULL, .var = NULL};
 
const TypePrefix type_prefixes[] = {
    {'i',   VAL_INT},
    {'f',   VAL_DOUBLE}, //Holly floats are handled as doubles internally
    {'b',   VAL_BOOL},
    {'s', VAL_STRING},
    {'v',   VAL_VOID}
};

const size_t type_prefixes_len = ARRAY_LEN(type_prefixes);

static ValueType getVarTypeFromName(char* var_name) {
    //re-check string validity. Just to be safe
    if(!isValidVarName(var_name)) {
        raiseError(ERR_INVALID_VARNAME);
    }
    ValueType ret = VAL_INVALID;
    char prefix = var_name[0];
    for(size_t i = 0; i < ARRAY_LEN(type_prefixes); i++) { //loop through type prefixes
        if(type_prefixes[i].c == prefix) {
            ret = type_prefixes[i].type;
            break;
        }
    }
    if(ret == VAL_INVALID) raiseError(ERR_INVALID_PREFIX);
    return ret;
}

static VarLookup findVarEntry(Environment *env, char* var_name) {
    VarEntry *ret = NULL;
    HASH_FIND_STR(env->var_table, var_name, ret);
    if(!ret) {
        //check parents
        if(!env->parent) return VAR_NOT_FOUND; //if not found by top level, it's not there
        logTrace("Searching for var %s in parent env", var_name);
        return findVarEntry(env->parent, var_name); //recurse
    }
    return (VarLookup){.owner = env, .var = ret};
}

//add a string pointer to the environment storage, ensuring that it will be properly freed when the environment is cleaned
static void addStrToEnv(Environment *env, char* string) {
    if(env->strings.len >= env->strings.max) { //grow if needed
        size_t new_max = (env->strings.max * 2); //double capacity
        char** new_arr = realloc(env->strings.arr, new_max * sizeof(char*));
        if(!new_arr) raiseError(ERR_MEM_ALLOC);
        env->strings.arr = new_arr;
        env->strings.max = new_max;
    }
    env->strings.arr[env->strings.len] = string;
    env->strings.len += 1; 
}

void addValToEnv(Environment *env, Value in) {
    if(in.type == VAL_STRING) {
        addStrToEnv(env, in.val_str);
    }
}

void initEnv(Environment *env, Environment *parent) {
    env->parent = parent;
    env->return_val = VALUE_VOID;
    env->should_return = false;
    env->should_break = false;
    env->var_table = NULL; //this is set up elsewhere
    //init strings
    env->strings.arr = malloc(ENV_STRING_ARR_LEN * sizeof(char*));
    env->strings.len = 0;
    env->strings.max = ENV_STRING_ARR_LEN;
}

void cleanEnv(Environment *env) {
    VarEntry *cur, *tmp; //need these for iter
    uint16_t count = 0;
    HASH_ITER(hh, env->var_table, cur, tmp) {
        count++;
        HASH_DEL(env->var_table, cur);
        free(cur);  
    }
    logNode("Freed %d var items", count);
    for(size_t i = 0; i < env->strings.len; i++) { //free stored strings
        free(env->strings.arr[i]);
    }
    logData("Freed %zu stored strings", env->strings.len);
    free(env->strings.arr);
}


void addVarToHash(Environment *env, char* var_name, Value var_val, bool final) {
    //check if var exists
    VarEntry *var_hash = findVarEntry(env, var_name).var;
    if(var_hash) {
        raiseError(ERR_ALREADY_ASSIGNED_VAR);
    }
    //ensure our var is correctly typed
    if(getVarTypeFromName(var_name) != var_val.type) {
        raiseError(ERR_MISMATCH_PREFIX);
    }
    //create new copy if string
    Value val_copy = var_val;
    if(var_val.type == VAL_STRING) {
        val_copy.val_str = var_val.val_str; //this is owned by whomever
    }
    //make actual var
    var_hash = malloc(sizeof(VarEntry)); //make our new hash
    var_hash->var_name = var_name; //var name is owned by tree
    var_hash->value = val_copy;
    var_hash->final = final;
    HASH_ADD_KEYPTR(hh, env->var_table, var_hash->var_name, strlen(var_hash->var_name), var_hash); //add the variable to the hash
}

void addUnsetVarToHash(Environment *env, char* var_name) {
    VarEntry *var_hash_item = findVarEntry(env, var_name).var;
    if(var_hash_item) raiseError(ERR_ALREADY_ASSIGNED_VAR);
    //create var item
    var_hash_item = malloc(sizeof(VarEntry));
    var_hash_item->var_name = var_name;
    var_hash_item->value = VALUE_UNSET;
    var_hash_item->final = false;
    HASH_ADD_KEYPTR(hh, env->var_table, var_hash_item->var_name, strlen(var_hash_item->var_name), var_hash_item);
}

void finalizeVar(Environment *env, char* var_name) {
    VarEntry *entry = findVarEntry(env, var_name).var;
    if(!entry) raiseError(ERR_UNASSIGNED_VAR);
    if(entry->value.type == VAL_UNSET) raiseError(ERR_UNSET_FINAL);
    if(entry->final) sendWarning(WARN_VAR_ALREADY_FINAL); //warn if var is final already
    entry->final = true;
}

void updateVarValue(Environment *env, char* var_name, Value new_val) {
    VarLookup lookup = findVarEntry(env, var_name);
    VarEntry *entry = lookup.var;
    if(!entry) raiseError(ERR_UNASSIGNED_VAR);
    if(entry->final) raiseError(ERR_MODIFY_FINAL_VAR);
    //expected type can be inferred from current type if set, otherwise have to check string
    ValueType exp_type = (entry->value.type == VAL_UNSET) ? getVarTypeFromName(entry->var_name) : entry->value.type;
    if(exp_type != new_val.type) raiseError(ERR_MISMATCH_PREFIX);
    //handle malloc()ed string data
    Value new_val_copy = new_val;
    if(new_val.type == VAL_STRING && env != lookup.owner) { 
        //if setting a string above the current environment, need to add the string to that environment so it doesn't get freed
        //after the lifetime of the inner block
        new_val_copy = dupVal(new_val);
        addValToEnv(lookup.owner, new_val_copy);
    }
    entry->value = new_val_copy;
}


Value getVarValue(Environment *env, char* var_name) {
    VarEntry *tmp = findVarEntry(env, var_name).var;
    if(!tmp) raiseError(ERR_UNASSIGNED_VAR);
    Value ret = tmp->value;
    return ret;
}

int64_t getIntVarValue(Environment *env, char* var_name) {
    VarEntry *tmp = findVarEntry(env, var_name).var;
    if(!tmp) raiseError(ERR_UNASSIGNED_VAR);
    if(tmp->value.type != VAL_INT) raiseErrorWithCtx(ERR_EXP_INT, CTX_1TKTYPE, tmp->value.type);
    return tmp->value.val_int;
}

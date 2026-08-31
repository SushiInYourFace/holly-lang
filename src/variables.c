#include "errors.h"
#include "variables.h"
#include "logging.h"
#include "types.h"
#include "utils.h"
#include "values.h"
#include <stddef.h>
#include <string.h>
 
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

static VarEntry* findVarEntry(Environment *env, char* var_name) {
    VarEntry *ret = NULL;
    HASH_FIND_STR(env->var_table, var_name, ret);
    if(!ret) {
        //check parents
        if(!env->parent) return NULL; //if not found by top level, it's not there
        logTrace("Searching for var %s in parent env", var_name);
        return findVarEntry(env->parent, var_name); //recurse
    }
    return ret;
}

void addVarToHash(Environment *env, char* var_name, Value var_val, bool final) {
    //check if var exists
    VarEntry *var_hash = findVarEntry(env, var_name);
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
        val_copy.val_str = strdup(var_val.val_str); //this is no longer owned by tree
    }
    //make actual var
    var_hash = malloc(sizeof(VarEntry)); //make our new hash
    var_hash->var_name = var_name; //var name is owned by tree
    var_hash->value = val_copy;
    var_hash->final = final;
    HASH_ADD_KEYPTR(hh, env->var_table, var_hash->var_name, strlen(var_hash->var_name), var_hash); //add the variable to the hash
}

void addUnsetVarToHash(Environment *env, char* var_name) {
    VarEntry *var_hash_item = findVarEntry(env, var_name);
    if(var_hash_item) raiseError(ERR_ALREADY_ASSIGNED_VAR);
    //create var item
    var_hash_item = malloc(sizeof(VarEntry));
    var_hash_item->var_name = var_name;
    var_hash_item->value = VALUE_UNSET;
    var_hash_item->final = false;
    HASH_ADD_KEYPTR(hh, env->var_table, var_hash_item->var_name, strlen(var_hash_item->var_name), var_hash_item);
}

void finalizeVar(Environment *env, char* var_name) {
    VarEntry *entry = findVarEntry(env, var_name);
    if(!entry) raiseError(ERR_UNASSIGNED_VAR);
    if(entry->final) sendWarning(WARN_VAR_ALREADY_FINAL); //warn if var is final already
    entry->final = true;
}

void updateVarValue(Environment *env, char* var_name, Value new_val) {
    VarEntry *entry = findVarEntry(env, var_name);
    if(!entry) raiseError(ERR_UNASSIGNED_VAR);
    if(entry->final) raiseError(ERR_MODIFY_FINAL_VAR);
    //expected type can be inferred from current type if set, otherwise have to check string
    ValueType exp_type = (entry->value.type == VAL_UNSET) ? getVarTypeFromName(entry->var_name) : entry->value.type;
    if(exp_type != new_val.type) raiseError(ERR_MISMATCH_PREFIX);
    //handle malloc()ed string data
    Value new_val_copy = new_val;
    if(new_val.type == VAL_STRING) {
        if(entry->value.type != VAL_UNSET) free(entry->value.val_str); //if value was previously unset, don't have to free old string
        new_val_copy.val_str = strdup(new_val.val_str);
    }
    entry->value = new_val_copy;
}


Value getVarValue(Environment *env, char* var_name) {
    VarEntry *tmp = findVarEntry(env, var_name);
    if(!tmp) raiseError(ERR_UNASSIGNED_VAR);
    Value ret = tmp->value;
    return ret;
}

int64_t getIntVarValue(Environment *env, char* var_name) {
    VarEntry *tmp = findVarEntry(env, var_name);
    if(!tmp) raiseError(ERR_UNASSIGNED_VAR);
    if(tmp->value.type != VAL_INT) raiseErrorWithCtx(ERR_EXP_INT, CTX_1TKTYPE, tmp->value.type);
    return tmp->value.val_int;
}

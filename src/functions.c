#include "errors.h"
#include "logging.h"
#include "utils.h"
#include "variables.h"


ValueType getFunReturnFromName(char* name) {
    if(!isValidFunName(name)) raiseError(ERR_INVALID_FUN_NAME);
    char c = name[0]; 
    for(size_t i = 0; i < type_prefixes_len; i++) {
        if(type_prefixes[i].c == c) {
            return type_prefixes[i].type;
        }
    }
    return VAL_INVALID;
}

void addFunctionToHash(char* fun_name, ParamNames params, TreeNode *fun_block, FunctionEntry **hash, ValueType return_type) {
    FunctionEntry *entry = NULL;
    HASH_FIND_STR(*hash, fun_name, entry);
    if(entry) raiseError(ERR_DUP_FUNCTION); //error if function already exists
    entry = malloc(sizeof(FunctionEntry));
    entry->fun_block = fun_block;
    entry->fun_name = fun_name; //string passed as param will be destroyed when the tokens are cleaned
    entry->return_type = return_type;
    entry->params = params;
    HASH_ADD_KEYPTR(hh, *hash, entry->fun_name, strlen(entry->fun_name), entry);
}

FunctionEntry *findFunction(char* fun_name, FunctionEntry *hash) {
    FunctionEntry *ret = NULL;
    HASH_FIND_STR(hash, fun_name, ret);
    return ret;

}

void cleanFunctionHash(FunctionEntry **hash) {
    FunctionEntry *cur, *tmp;
    HASH_ITER(hh, *hash, cur, tmp) {
        HASH_DEL(*hash, cur);
        free(cur->fun_name); //function hash owns their OWN copy of the name. This is DIFFERENT from variables
        freeNode(cur->fun_block);
        //free param names
        for(size_t i = 0; i < cur->params.count; i++) {
            logTrace("Freeing param %s", cur->params.list[i]);
            free(cur->params.list[i]); 
        }
        logTrace("Cleared function with %zu params", cur->params.count);
        free(cur);
    }
}
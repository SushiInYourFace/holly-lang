#include "errors.h"
#include "parsing/parsing.h"
#include "types.h"
#include "functions/builtins.h"
#include "functions/functions.h"
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

void addBuiltinToHash(char* fun_name, FunctionEntry **hash, ValueType return_type, BuiltinFun fun) {
    FunctionEntry *entry = NULL;
    HASH_FIND_STR(*hash, fun_name, entry);
    if(entry) raiseError(ERR_DUP_FUNCTION); //error if function already exists
    entry = malloc(sizeof(FunctionEntry));
    entry->fun_name = fun_name;
    entry->param_names = EMPTY_PARAMS;
    entry->is_builtin = true;
    entry->return_type = return_type;
    entry->builtin = fun;
    HASH_ADD_KEYPTR(hh, *hash, entry->fun_name, strlen(entry->fun_name), entry);
}

static void checkValues(ParamValues vals, size_t exp_count, ...) {
    if(vals.count != exp_count) raiseError(ERR_INVALID_NUM_PARAMS);
    va_list args;
    va_start(args, exp_count);
    for(size_t i = 0; i < exp_count; i++) {
        ValueType cur_type = va_arg(args, ValueType);
        if(vals.list[i].type != cur_type) {
            raiseErrorWithCtx(ERR_INVALID_VALTYPE, CTX_2VAL, cur_type, vals.list[i].type);
        }
    }
    va_end(args);
}


Value builtin_stringlen(ParamValues vals) {
    //expect one string
    checkValues(vals, 1, VAL_STRING);
    return (Value) {.type = VAL_INT, .val_int = strlen(vals.list[0].val_str)};
}

Value builtin_max_int(ParamValues vals) {
    checkValues(vals, 2, VAL_INT, VAL_INT);
    return (Value) {.type = VAL_INT, .val_int = MAX(vals.list[0].val_int, vals.list[1].val_int)};
}

Value builtin_min_int(ParamValues vals) {
    checkValues(vals, 2, VAL_INT, VAL_INT);
    return (Value) {.type = VAL_INT, .val_int = MIN(vals.list[0].val_int, vals.list[1].val_int)};
}

Value builtin_max_float(ParamValues vals) {
    checkValues(vals, 2, VAL_DOUBLE, VAL_DOUBLE);
    return (Value) {.type = VAL_DOUBLE, .val_float = fmax(vals.list[0].val_float, vals.list[1].val_float)};
}

Value builtin_min_float(ParamValues vals) {
    checkValues(vals, 2, VAL_DOUBLE, VAL_DOUBLE);
    return (Value) {.type = VAL_DOUBLE, .val_float = fmin(vals.list[0].val_float, vals.list[1].val_float)};
}


void initBuiltins(FunctionEntry **hash) {
    addBuiltinToHash("i:stringlen", hash, VAL_INT, builtin_stringlen);
    addBuiltinToHash("i:min", hash, VAL_INT, builtin_min_int);
    addBuiltinToHash("i:max", hash, VAL_INT, builtin_max_int);
    addBuiltinToHash("f:max", hash, VAL_DOUBLE, builtin_max_float);
    addBuiltinToHash("f:min", hash, VAL_DOUBLE, builtin_min_float);  
}

#include "errors.h"
#include "parsing/parsing.h"
#include "types.h"
#include "functions/builtins.h"
#include "functions/functions.h"
#include "values.h"
#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>

static char* strToCase(char* in, bool upper) {
    size_t str_len = strlen(in);
    char* out = malloc(str_len + 1);
    if(!out) raiseError(ERR_MEM_ALLOC);
    for(size_t i = 0; i < str_len; i++) {
        out[i] = (upper) ? toupper(in[i]) : tolower(in[i]);
    }
    out[str_len] = '\0';
    return out;
}

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

static ParamValues checkValues(ParamValues vals, size_t exp_count, ...) {
    if(vals.count != exp_count) raiseError(ERR_INVALID_NUM_PARAMS);
    ParamValues ret = vals; //will store any casts
    va_list args;
    va_start(args, exp_count);
    for(size_t i = 0; i < exp_count; i++) {
        ValueType cur_type = va_arg(args, ValueType);
        if(vals.list[i].type != cur_type) {
            ret.list[i] = tryCastValue(vals.list[i], cur_type); //attempt to cast
            if(ret.list[i].type != cur_type) {//if cast didn't work
                raiseErrorWithCtx(ERR_INVALID_VALTYPE, CTX_2VAL, cur_type, vals.list[i].type);
            }
            sendWarning(WARN_CAST_VAR_TYPE);
            continue;
        }
        ret.list[i] = vals.list[i];
    }
    va_end(args);
    return ret;
}


static Value builtin_stringlen(ParamValues vals) {
    //expect one string
    vals = checkValues(vals, 1, VAL_STRING);
    return (Value) {.type = VAL_INT, .val_int = strlen(vals.list[0].val_str)};
}

static Value builtin_max_int(ParamValues vals) {
    vals = checkValues(vals, 2, VAL_INT, VAL_INT);
    return (Value) {.type = VAL_INT, .val_int = MAX(vals.list[0].val_int, vals.list[1].val_int)};
}

static Value builtin_min_int(ParamValues vals) {
    vals = checkValues(vals, 2, VAL_INT, VAL_INT);
    return (Value) {.type = VAL_INT, .val_int = MIN(vals.list[0].val_int, vals.list[1].val_int)};
}

static Value builtin_max_float(ParamValues vals) {
    vals = checkValues(vals, 2, VAL_DOUBLE, VAL_DOUBLE);
    return (Value) {.type = VAL_DOUBLE, .val_float = fmax(vals.list[0].val_float, vals.list[1].val_float)};
}

static Value builtin_min_float(ParamValues vals) {
    vals = checkValues(vals, 2, VAL_DOUBLE, VAL_DOUBLE);
    return (Value) {.type = VAL_DOUBLE, .val_float = fmin(vals.list[0].val_float, vals.list[1].val_float)};
}

static Value builtin_itod(ParamValues vals) {
    vals = checkValues(vals, 1, VAL_INT);
    return (Value) {.type = VAL_DOUBLE, .val_float = vals.list[0].val_int};
}

static Value builtin_pow_float(ParamValues vals) {
    vals = checkValues(vals, 2, VAL_DOUBLE, VAL_DOUBLE);
    return (Value) {.type = VAL_DOUBLE, .val_float = pow(vals.list[0].val_float, vals.list[1].val_float)};
}

static Value builtin_floor_int(ParamValues vals) {
    vals = checkValues(vals, 1, VAL_DOUBLE);
    return (Value) {.type = VAL_INT, .val_int = floor(vals.list[0].val_float)};
}

static Value builtin_ceil_int(ParamValues vals) {
    vals = checkValues(vals, 1, VAL_DOUBLE);
    return (Value) {.type = VAL_INT, .val_int = ceil(vals.list[0].val_float)};
}

static Value builtin_to_lower(ParamValues vals) {
    vals = checkValues(vals, 1, VAL_STRING);
    return (Value) {.type = VAL_STRING, .val_str = strToCase(vals.list[0].val_str, false)};
}

static Value builtin_to_upper(ParamValues vals) {
    vals = checkValues(vals, 1, VAL_STRING);
    return (Value) {.type = VAL_STRING, .val_str = strToCase(vals.list[0].val_str, true)};
}

void initBuiltins(FunctionEntry **hash) {
    addBuiltinToHash("i:stringlen", hash, VAL_INT, builtin_stringlen);
    addBuiltinToHash("i:min", hash, VAL_INT, builtin_min_int);
    addBuiltinToHash("i:max", hash, VAL_INT, builtin_max_int);
    addBuiltinToHash("f:max", hash, VAL_DOUBLE, builtin_max_float);
    addBuiltinToHash("f:min", hash, VAL_DOUBLE, builtin_min_float);  
    addBuiltinToHash("f:itod", hash, VAL_DOUBLE, builtin_itod);
    addBuiltinToHash("f:pow", hash, VAL_DOUBLE, builtin_pow_float);
    addBuiltinToHash("i:floor", hash, VAL_INT, builtin_floor_int);
    addBuiltinToHash("i:ceil", hash, VAL_INT, builtin_ceil_int);
    addBuiltinToHash("s:tolower", hash, VAL_STRING, builtin_to_lower);
    addBuiltinToHash("s:toupper", hash, VAL_STRING, builtin_to_upper);
}

#include "values.h"
#include "errors.h"
#include "types.h"
#include <_string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


const Value VALUE_EMPTY = {.type = VAL_INT, .val_int = 0};
const Value VALUE_VOID = {.type = VAL_VOID};
const Value VALUE_TRUE = {.type = VAL_BOOL, .val_bool = true};
const Value VALUE_FALSE = {.type = VAL_BOOL, .val_bool = false};
const Value VALUE_UNSET = {.type = VAL_UNSET};

static inline double valAsDouble(Value v) { //reminder that all float values are stored internally as doubles
    if(v.type == VAL_BOOL) raiseError(ERR_TRY_CAST_BOOL); //no doing math on bools!!
    if(v.type == VAL_STRING) raiseError(ERR_MATH_ON_STR); //no doing math on bools!!
    return (v.type == VAL_DOUBLE) ? v.val_float : (double) v.val_int;
}

const char* getValueTypeString(ValueType v) {
    switch (v) {
        case VAL_DOUBLE:    return "VAL_DOUBLE";
        case VAL_INT:       return "VAL_INT";
        case VAL_BOOL:      return "VAL_BOOL";
        case VAL_STRING:    return "VAL_STRING";
        case VAL_VOID:      return "VAL_VOID";
        case VAL_INVALID:   return "VAL_INVALID";
        case VAL_UNSET:     return "VAL_UNSET";
    }
}

static void catchInvalidMath(ValueType left, ValueType right) {
    if(left == VAL_BOOL || right == VAL_BOOL) raiseError(ERR_MATH_ON_BOOL);
    if(left == VAL_STRING || right == VAL_STRING) raiseError(ERR_MATH_ON_STR);
    if(left == VAL_VOID || right == VAL_VOID) raiseError(ERR_OP_ON_VOID);
    if(left == VAL_UNSET || right == VAL_UNSET) raiseError(ERR_REF_UNSET);
}

void freeValue(Value in) {
    if(in.type == VAL_STRING) {
        free(in.val_str);
    }
}

//change any integers to doubles
Value tryCastValue(Value in, ValueType type) {
    if(type != VAL_DOUBLE) return in; //only doubles can be cast to
    if(in.type != VAL_INT) return in; //only ints can be cast from
    return (Value) {.type = VAL_DOUBLE, .val_float = in.val_int};
}

Value dupVal(Value in) {
    Value ret = in;
    if(in.type == VAL_STRING) {
        ret.val_str = strdup(in.val_str);
    }
    return ret;
}

Value addValues(Value left, Value right) {
    //string concat
    if(left.type == VAL_STRING && right.type == VAL_STRING) {
        char* out = malloc(strlen(left.val_str) + strlen(right.val_str) + 1); //new char buffer
        strcpy(out, left.val_str);
        strcat(out, right.val_str);
        return (Value){.type = VAL_STRING, .val_str = out};
    }
    catchInvalidMath(left.type, right.type);
    if(left.type != right.type) {
        sendWarning(WARN_MIXING_VAR_TYPES);
        double d_left = valAsDouble(left);
        double d_right = valAsDouble(right);
        return (Value){.type = VAL_DOUBLE, .val_float = (d_left + d_right)};
    };
    if(left.type == VAL_INT) return (Value){.type = VAL_INT, .val_int = (left.val_int + right.val_int)};
    else return (Value){.type = VAL_DOUBLE, .val_float = (left.val_float + right.val_float)};
}

Value subValues(Value left, Value right) {
    catchInvalidMath(left.type, right.type);
    if(left.type != right.type) {
        sendWarning(WARN_MIXING_VAR_TYPES);
        double d_left = valAsDouble(left);
        double d_right = valAsDouble(right);
        return (Value){.type = VAL_DOUBLE, .val_float = (d_left - d_right)};
    };
    if(left.type == VAL_INT) return (Value){.type = VAL_INT, .val_int = (left.val_int - right.val_int)};
    else return (Value){.type = VAL_DOUBLE, .val_float = (left.val_float - right.val_float)};
}

Value multValues(Value left, Value right) {
    catchInvalidMath(left.type, right.type);
    if(left.type != right.type) {
        sendWarning(WARN_MIXING_VAR_TYPES);
        double d_left = valAsDouble(left);
        double d_right = valAsDouble(right);
        return (Value){.type = VAL_DOUBLE, .val_float = (d_left * d_right)};
    };
    if(left.type == VAL_INT) return (Value){.type = VAL_INT, .val_int = (left.val_int * right.val_int)};
    else return (Value){.type = VAL_DOUBLE, .val_float = (left.val_float * right.val_float)};
}

Value divValues(Value left, Value right) {
    catchInvalidMath(left.type, right.type);   
    if(left.type != right.type) {
        sendWarning(WARN_MIXING_VAR_TYPES);
        double d_left = valAsDouble(left);
        double d_right = valAsDouble(right);
        return (Value){.type = VAL_DOUBLE, .val_float = (d_left / d_right)};
    };
    if(left.type == VAL_INT) return (Value){.type = VAL_INT, .val_int = (left.val_int / right.val_int)};
    else return (Value){.type = VAL_DOUBLE, .val_float = (left.val_float / right.val_float)};
}

Value modValues(Value left, Value right) {
    catchInvalidMath(left.type, right.type); 
    if(left.type != right.type) {
        sendWarning(WARN_MIXING_VAR_TYPES);
        double d_left = valAsDouble(left);
        double d_right = valAsDouble(right);
        return (Value){.type = VAL_DOUBLE, .val_float = (d_left / d_right)};
    };
    if(left.type == VAL_INT) return (Value){.type = VAL_INT, .val_int = (left.val_int % right.val_int)};
    else return (Value){.type = VAL_DOUBLE, .val_float = fmod(left.val_float, right.val_float)};
}

bool isTruthy(Value val) {
    switch(val.type) {
        case(VAL_BOOL):
            return val.val_bool;
        case(VAL_INT):
            return (val.val_int); //if it's truthy in C, it's truthy in Holly
        case(VAL_DOUBLE):
            return (val.val_float);
        case(VAL_STRING):
            return (val.val_str); //if a pointer has been set, it is truthy
        case(VAL_VOID):
            return false;
        case(VAL_UNSET):
            raiseError(ERR_REF_UNSET);
            return false; //unreachable
        case(VAL_INVALID):
            return (false); //fallback
    }
}

#define DO_COMPARE(left, right, op, result_dest)                                    \
    switch((left).type) {                                                           \
        case(VAL_BOOL): (result_dest) = ((left).val_bool op (right).val_bool);      \
            break;                                                                  \
        case(VAL_INT): (result_dest) = ((left).val_int op (right).val_int);         \
            break;                                                                  \
        case(VAL_DOUBLE): (result_dest) = ((left).val_float op (right).val_float);  \
            break;                                                                  \
        case(VAL_STRING): raiseError(ERR_MATH_ON_STR);                              \
        case(VAL_UNSET):   raiseError(ERR_REF_UNSET);                               \
        case(VAL_VOID):     raiseError(ERR_OP_ON_VOID);                             \
        case(VAL_INVALID): raiseError(ERR_INVALID_NUM); exit(1);                    \
    }

bool evalExpression(Value left, Value right, TokenType comparator) {
    bool desired = (comparator == TK_IS_EQ) ? true : false;
    bool actual;
    if(left.type != right.type) {
        if(left.type == VAL_BOOL || right.type == VAL_BOOL) { //if only one token is a bool
            raiseError(ERR_TRY_CAST_BOOL);
        }
        sendWarning(WARN_MIXING_VAR_TYPES);
        double left_d = valAsDouble(left);
        double right_d = valAsDouble(right);
        switch(comparator) {
            case(TK_IS_EQ): return left_d == right_d;
            case(TK_NOT_EQ): return left_d != right_d;
            case(TK_GT): return left_d > right_d;
            case(TK_LT): return left_d < right_d;
            case(TK_GT_EQ): return left_d >= right_d;
            case(TK_LT_EQ): return left_d <= right_d;
            default: return false;
        }
        actual = (valAsDouble(left) == valAsDouble(right));
        return desired == actual;
    } //else data types are the same
    bool return_val;
    switch(comparator) {
        case(TK_IS_EQ): 
            DO_COMPARE(left, right, ==, return_val);
            return return_val;
        case(TK_NOT_EQ): 
            DO_COMPARE(left, right, !=, return_val);
            return return_val;
        case(TK_GT):
            DO_COMPARE(left, right, >, return_val);
            return return_val;
        case(TK_LT):
            DO_COMPARE(left, right, <, return_val);
            return return_val;
        case(TK_GT_EQ):
            DO_COMPARE(left, right, >=, return_val);
            return return_val;
        case(TK_LT_EQ):
            DO_COMPARE(left, right, <=, return_val);
            return return_val;
        default: return false;
    }
}


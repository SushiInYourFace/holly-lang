#pragma once


//error types
typedef enum {
    ERR_NO_START,
    ERR_NO_DONE,
    ERR_FILE_OPEN,
    ERR_DONE_BEFORE_START,
    ERR_UNEXPECTED_TOK,
    ERR_INVALID_NUM,
    ERR_EXP_PRIMARY,
    ERR_EXP_COMPARATOR,
    ERR_UNKNOWN_PRIMARY,
    ERR_EXP_IDENT,
    ERR_EXP_VARASSIGN,
    ERR_EXP_INT,
    ERR_EXP_DOUBLE,
    ERR_EXP_BOOL,
    ERR_UNKNOWN_BINARY_OP,
    ERR_UNKNOWN_NODE_TYPE,
    ERR_MEM_ALLOC,
    ERR_ALREADY_ASSIGNED_VAR,
    ERR_UNASSIGNED_VAR,
    ERR_MODIFY_FINAL_VAR,
    ERR_MISMATCH_NUM_TYPES,
    ERR_EXP_VARNAME,
    ERR_INVALID_VARNAME,
    ERR_INVALID_PREFIX,
    ERR_MISMATCH_PREFIX,
    ERR_UNCLOSED_STR,
    ERR_INVALID_CHAR,
    ERR_TRY_CAST_BOOL,
    ERR_MATH_ON_BOOL,
    ERR_UNCLOSED_BLOCK,
    ERR_DUP_FUNCTION,
    ERR_UNDEF_FUN,
    ERR_INVALID_FUN_NAME,
    ERR_MISMATCH_FUN_RETURN,
    ERR_INVALID_NUM_PARAMS,
    ERR_NO_IN_FILE,
    ERR_MATH_ON_STR,
    ERR_MATH_ON_CHAR,
    ERR_BREAK_NO_LOOP,
    ERR_OP_ON_VOID,
    ERR_MATH_ON_ARRAY,
    ERR_REF_UNSET,
    ERR_UNSET_FINAL,
    ERR_INVALID_VALTYPE,
    ERR_TOO_MANY_PARAMS,
    ERR_USER_RAISED,
    ERR_OTHER
} Error;

typedef enum {
    WARN_VAR_ALREADY_FINAL,
    WARN_MIXING_VAR_TYPES,
    WARN_CHANGING_VAR_TYPES,
    WARN_CAST_VAR_TYPE,
    WARN_SAW_INVALID_VAL,
    WARN_MAX_LOOP_ITERS,
    WARN_NEG_NUM_LOOP
} Warning;

typedef enum {
    CTX_1TKTYPE,
    CTX_2TKTYPE,
    CTX_1TKCAT_1TKTYPE,
    CTX_2SIZE,
    CTX_2VAL
} ErrCtx;

//raise an error with the given code
void raiseError(Error e);
/**
 * @brief Raise an error with more context
 * 
 * If the additional param form contains 2 of the same type, the message will always take the form of "expected x, got y"
 * @param e The error type 
 * @param p The form of the additional params
 * @param ... The additional params
 */
void raiseErrorWithCtx(Error e, ErrCtx p, ...);
//send a warning, but don't exit from program
void sendWarning(Warning w);
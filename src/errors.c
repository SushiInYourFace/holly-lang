#include "errors.h"
#include "logging.h"
#include "utils.h"
#include "values.h"
#include <stdio.h>
#include <stdarg.h>

static char* getErrString(Error e) {
    switch(e) {
        case ERR_NO_DONE:               return "No 'done' statement in source file!";
        case ERR_NO_START:              return "No 'start' statement in source file!";
        case ERR_DONE_BEFORE_START:     return "'Done' statement must come after 'start' statement!";
        case ERR_UNEXPECTED_TOK:        return "Encountered an unexpected token!";
        case ERR_EXP_PRIMARY:           return "Parser expected a 'primary' data type, but something else was provided!";
        case ERR_EXP_VARNAME:           return "Parser expected a 'varname' data type, but something else was provided!";
        case ERR_UNKNOWN_BINARY_OP:     return "Eval encountered an unknown binary operator!";
        case ERR_UNKNOWN_NODE_TYPE:     return "Eval encountered an unknown node type!";
        case ERR_UNKNOWN_PRIMARY:       return "Parsing encountered an unknown primary type!";
        case ERR_MEM_ALLOC:             return "Memory allocation failed!";
        case ERR_EXP_IDENT:             return "Expected an identifier!";
        case ERR_EXP_VARASSIGN:         return "Expected a variable assignment token!";
        case ERR_EXP_INT:               return "Eval expected an int, got something else";
        case ERR_EXP_DOUBLE:            return "Eval expected a double, got something else";
        case ERR_EXP_BOOL:              return "Eval expected a bool, got something else";
        case ERR_ALREADY_ASSIGNED_VAR:  return "Attempted to re-declare an existing variable!";
        case ERR_UNASSIGNED_VAR:        return "Attempted to reference an unassigned variable!";
        case ERR_MODIFY_FINAL_VAR:      return "Attempted to modify a 'final' var!";
        case ERR_INVALID_NUM:           return "Attempted to parse an invalid number! ";
        case ERR_MISMATCH_NUM_TYPES:    return "Attempted to use mismatched data types in a math operation!";
        case ERR_INVALID_VARNAME:       return "Attempted to parse a var type from an invalid varname!";
        case ERR_INVALID_PREFIX:        return "Attempted to use a var name with an invalid prefix!";
        case ERR_MISMATCH_PREFIX:       return "Attempted to assign a value to a variable with a mismatched prefix!";
        case ERR_UNCLOSED_STR:          return "Attempted to parse an improperly closed string!";
        case ERR_FILE_OPEN:             return "Could not open file!";
        case ERR_TRY_CAST_BOOL:         return "Tried to cast a bool to another data type!";
        case ERR_MATH_ON_BOOL:          return "Tried to do math using a bool!";
        case ERR_UNCLOSED_BLOCK:        return "Failed to close a block!";
        case ERR_EXP_COMPARATOR:        return "Parser expected a comparator token!";
        case ERR_DUP_FUNCTION:          return "Attempted to create an already-existing function!";
        case ERR_UNDEF_FUN:             return "Attempted to call an undefined function!";
        case ERR_INVALID_FUN_NAME:      return "Attempted to parse an invalid string as a function name!";
        case ERR_MISMATCH_FUN_RETURN:   return "A function returned a value other than declared!";
        case ERR_INVALID_NUM_PARAMS:    return "Function was given an invalid number of params!";
        case ERR_NO_IN_FILE:            return "No input file was provided!";
        case ERR_MATH_ON_STR:           return "Tried to do math on string";
        case ERR_OTHER:                 return "Unknown error!";
    }
}

static void displayErrorMessage(Error e) {
    fprintf(stderr, "ERROR: %s", getErrString(e));
}


void raiseError(Error e) {
    displayErrorMessage(e);  //display error msg
    fprintf(stderr, "\n");
    exit(1);
}

void raiseErrorWithCtx(Error e, ErrCtx p, ...) {
    //display error message
    displayErrorMessage(e);
    fprintf(stderr, ": "); //formatting
    va_list args;
    va_start(args, p); //init variadic params
    switch(p) {
        case CTX_1TKTYPE:
            fprintf(stderr, "received token type %s\n", getTokenString(va_arg(args, TokenType)));
            break;
        case CTX_2TKTYPE:
            fprintf(stderr, 
                "Expected token %s, got %s\n", 
                getTokenString(va_arg(args, TokenType)), 
                getTokenString(va_arg(args, TokenType)));
            break;
        case CTX_2VAL:
            fprintf(stderr, 
                "Expected type %s, got %s\n", 
                getValueTypeString(va_arg(args, ValueType)), 
                getValueTypeString(va_arg(args, ValueType)));
            break;
        case CTX_1TKCAT_1TKTYPE:
            fprintf(
                stderr, 
                "Expected token in category %s, got token %s\n",
                getTokenCatString(va_arg(args, TokenCategory)),
                getTokenString(va_arg(args, TokenType)));
            break;
        case CTX_2SIZE:
            fprintf(
                stderr, 
                "Expected %zu items, got %zu\n",
                va_arg(args, size_t),
                va_arg(args, size_t));
            break;           
    }
    exit(1);
}

static char* getWarnString(Warning w) {
    switch (w) {
        case WARN_VAR_ALREADY_FINAL:    return "'Finalizing' var which was already set to 'final'";
        case WARN_MIXING_VAR_TYPES:     return "Mixing vars of type `float` and `int` in a math operation";
        case WARN_CHANGING_VAR_TYPES:   return "Setting a var to a different type than originally assigned";
        case WARN_SAW_INVALID_VAL:      return "Saw an invalid value in a non-critical context";
        case WARN_CAST_VAR_TYPE:        return "Casting the type of a var";
        case WARN_MAX_LOOP_ITERS:       return "Broke out of a loop early, is there an infinite loop?";
    }
}

void sendWarning(Warning w) {
    logWarn("%s", getWarnString(w));
}
#include <stdint.h>
#include <stdio.h>
#include "parsing/parsing.h"
#include "types.h"
#include <string.h>
#include <time.h>

//timing helper
double getElapsedUs(struct timespec start, struct timespec end) {
    int64_t secs = end.tv_sec - start.tv_sec;
    int64_t nanosecs = end.tv_nsec - start.tv_nsec;
    double us = (secs * 1000000.0) + (nanosecs / 1000.0);
    return us;
}

const char* getTokenString(TokenType t) {
    switch(t) {
        case TK_START:          return "TK_START";
        case TK_DONE:           return "TK_DONE";
        case TK_FUN:            return "TK_FUN";
        case TK_ENDFUN:         return "TK_ENDFUN";
        case TK_SEMICOLON:      return "TK_SEMICOLON";
        case TK_EQ:             return "TK_EQ";
        case TK_PLUS:           return "TK_PLUS";
        case TK_ASSIGN:         return "TK_ASSIGN";
        case TK_MINUS:          return "TK_MINUS";
        case TK_MULT:           return "TK_MULT";
        case TK_PLUS_INPLACE:   return "TK_PLUS_INPLACE";
        case TK_MINUS_INPLACE:  return "TK_MINUS_INPLACE";
        case TK_MULT_INPLACE:   return "TK_MULT_INPLACE";
        case TK_DIV_INPLACE:    return "TK_DIV_INPLACE";
        case TK_INTEGER:        return "TK_INTEGER";
        case TK_LPAREN:         return "TK_LPAREN";
        case TK_RPAREN:         return "TK_RPAREN";
        case TK_IDENT:          return "TK_IDENT";
        case TK_DOUBLE:         return "TK_DOUBLE";
        case TK_STRING:         return "TK_STRING";
        case TK_CHAR:           return "TK_CHAR";
        case TK_EOF:            return "TK_EOF";
        case TK_UNKNOWN:        return "TK_UNKNOWN";
        case NUM_TOKENS:        return "NUM_TOKENS";
        case TK_FINAL:          return "TK_FINAL";
        case TK_VARNAME:        return "TK_VARNAME";
        case TK_DISPLAY:        return "TK_DISPLAY";
        case TK_FINALIZE:       return "TK_FINALIZE";
        case TK_DIV:            return "TK_DIV";
        case TK_BOOL:           return "TK_BOOL";
        case TK_IF:             return "TK_IF";
        case TK_ELSE:           return "TK_ELSE";
        case TK_WHILE:          return "TK_WHILE";
        case TK_ENDWHILE:       return "TK_ENDWHILE";
        case TK_ENDIF:          return "TK_ENDIF";
        case TK_COLON:          return "TK_COLON";
        case TK_IS_EQ:          return "TK_IS_EQ";
        case TK_NOT_EQ:         return "TK_NOT_EQ";
        case TK_GT:             return "TK_GT";
        case TK_LT:             return "TK_LT";
        case TK_GT_EQ:          return "TK_GT_EQ";
        case TK_LT_EQ:          return "TK_LT_EQ";
        case TK_FUN_NAME:       return "TK_FUN_NAME";
        case TK_EMPTY:          return "TK_EMPTY";
        case TK_RETURN:         return "TK_RETURN";
        case TK_COMMA:          return "TK_COMMA";
        case TK_MODULO:         return "TK_MODULO";
        case TK_BREAK:          return "TK_BREAK";
        case TK_LOOP:           return "TK_LOOP";
        case TK_ENDLOOP:        return "TK_ENDLOOP";
    }
}

const char* getTokenCatString(TokenCategory t) {
    switch(t) {
        case CAT_PRIMARY:           return "CAT_PRIMARY";
        case CAT_VAR_DECLARE:       return "CAT_VAR_DECLARE";
        case CAT_ANY:               return "CAT_ANY";
        case CAT_BINARY_ACTION:     return "CAT_BINARY_ACTION";
        case CAT_START_FILE:        return "CAT_START_FILE";
        case CAT_ADDITIVE:          return "CAT_ADDITIVE";
        case CAT_COMPARATORS:       return "CAT_COMPARATORS";
        case CAT_VAR_REASSIGN:      return "CAT_VAR_REASSIGN";
        case CAT_MULTIPLICATIVE:    return "CAT_MULTIPLICATIVE";   
        case CAT_START_STATEMENT:   return "CAT_START_STATEMENT";
    }
}

bool isValidVarName(char* name) {
    if(strlen(name) < 3) return false;
    if(name[0] != '_' && name[1] == '_' && name[2] != '_') return true;
    return false;
}   

bool isValidFunName(char* name) {
    if(strlen(name) < 3) return false;
    if(name[0] != ':' && name[1] == ':' && name[2] != ':') return true;
    return false;
}   
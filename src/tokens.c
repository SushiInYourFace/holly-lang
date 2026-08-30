#include "cursor.h"
#include "errors.h"
#include "types.h"
#include "utils.h"
#include <_ctype.h>
#include <_stdio.h>
#include <_string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//macro to help with null string tokens
#define NULL_STRING_TK(t) ((Token){.type=(t), .string=NULL})

static const KeywordEntry keywords[] = {
    {"start",       TK_START},
    {"done",        TK_DONE},
    {"assign",      TK_ASSIGN},
    {"fun",         TK_FUN},
    {"endfun",      TK_ENDFUN},
    {"final",       TK_FINAL},
    {"finalize",    TK_FINALIZE},
    {"display",     TK_DISPLAY},
    {"while",       TK_WHILE},
    {"endwhile",    TK_ENDWHILE},
    {"true",        TK_BOOL},
    {"false",       TK_BOOL},
    {"if",         TK_IF},
    {"else",         TK_ELSE},
    {"endif",      TK_ENDIF},
    {"return",      TK_RETURN},
    {"break",       TK_BREAK}
};

//any token that stores a number rather than a pointer to a string
static const TokenType non_string_tokens[] = {
    TK_INTEGER, TK_DOUBLE, TK_BOOL
};
#define NUM_NONSTRING_TOKENS (sizeof(non_string_tokens) /sizeof(non_string_tokens[0]))

static char* rangeToString(Cursor *cur, Range range) {
    size_t len = range.end - range.start;
    return strndup(&cur->data[range.start], len);
}

static int64_t rangeToInt(Cursor *cur, Range range) {
    char* num_string = rangeToString(cur, range);
    int64_t val = strtol(num_string, NULL, 0);
    free(num_string); //no longer need after conversion
    return val;
}

static double rangeToDouble(Cursor *cur, Range range) {
    char* num_string = rangeToString(cur, range);
    double val = strtod(num_string, NULL);
    free(num_string);
    return val;
}

static Token tokenFromRange(Cursor *cur, Range range) {
    char *in = rangeToString(cur, range);
    //check against keywords
    for(size_t i = 0; i < ARRAY_LEN(keywords); i++) {
        if(strcmp(in, keywords[i].word) == 0) {
            //if bool keyword, check whether true or false
            if(keywords[i].type == TK_BOOL) {
                bool tk_val = (strcmp(in, "true") == 0);
                free(in);
                return(Token){TK_BOOL, .boolean=tk_val};
            }
            free(in); //if it's a keyword, name is irrelevant
            return (Token){keywords[i].type, .string=NULL};
        }
    }
    //check var pattern
    bool has_var_pattern = isValidVarName(in);
    bool has_fun_pattern = isValidFunName(in);
    //check against var and fun name
    TokenType ret_type = (has_var_pattern) ? TK_VARNAME : (has_fun_pattern ? TK_FUN_NAME : TK_IDENT);
    //change ownership of the string being compared against to the token being returned
    return (Token){.type=ret_type, .string=in};
}

Token nextTokenData(Cursor *cur) {
    advPastWhitespaceAndComments(cur);
    uint32_t start = cur->pos;
    char c = advance(cur);
    switch(c) {
        //1 char tokens first
        case ';': return NULL_STRING_TK(TK_SEMICOLON);
        case '(': return NULL_STRING_TK(TK_LPAREN);
        case ')': return NULL_STRING_TK(TK_RPAREN);
        case ':': return NULL_STRING_TK(TK_COLON);
        case ',': return NULL_STRING_TK(TK_COMMA);
        case '%': return NULL_STRING_TK(TK_MODULO);
        case '\0': return NULL_STRING_TK(TK_EOF);
        //cases that must be 2 chars
        case '!':
            if(see(cur) == '=') {
                advance(cur);
                return NULL_STRING_TK(TK_NOT_EQ);
            }
            return NULL_STRING_TK(TK_UNKNOWN); //! isn't valid in any other way yet
        //cases that may or may not be 1 char tokens
        case '<':
            if(see(cur) == '=') { //<=
                advance(cur);
                return NULL_STRING_TK(TK_LT_EQ);
            }
            return NULL_STRING_TK(TK_LT);
        case '>':
            if(see(cur) == '=') { //>=
                advance(cur);
                return NULL_STRING_TK(TK_GT_EQ);
            }
            return NULL_STRING_TK(TK_GT);
        case '+':
            if(see(cur) == '=') { //case +=
                advance(cur); //advance past the equals
                return NULL_STRING_TK(TK_PLUS_INPLACE);
            }
            return NULL_STRING_TK(TK_PLUS);
        case '*':
            if(see(cur) == '=') {
                advance(cur);
                return NULL_STRING_TK(TK_MULT_INPLACE);
            }
            return NULL_STRING_TK(TK_MULT);
        case '/':
            if(see(cur) == '=') {
                advance(cur);
                return NULL_STRING_TK(TK_DIV_INPLACE);
            }
            return NULL_STRING_TK(TK_DIV);
            
        case '-': 
            if(see(cur) == '=') { //-=
                advance(cur);
                return (Token){ TK_MINUS_INPLACE, .string=NULL};
            } else if (isnumber(see(cur))) { //handle a negative number
                //TODO: maybe stick this in a static function?
                bool is_float = advPastNum(cur);
                if(is_float) {
                    double num = rangeToDouble(cur, (Range){start, cur->pos});
                    return (Token){TK_DOUBLE, .decimal=num};
                } else {
                    int64_t num = rangeToInt(cur, (Range){start, cur->pos});
                    return (Token){TK_INTEGER, .integer=num};
                }
            }
            return NULL_STRING_TK(TK_MINUS);
            case '=':
            if(see(cur) == '=') {
                advance(cur);
                return (Token){TK_IS_EQ, .string=NULL};
            }
            return (Token){ TK_EQ, .string=NULL};
        //longer token types
        case '"':
            //string
            advPastStr(cur);
            char* str = rangeToString(cur, (Range){start, cur->pos});
            return (Token){ TK_STRING, .string=str};
        default:
            if(isalpha(c)) { //if the char is a letter, it's the start of a keyword
                advPastIdent(cur);
                return tokenFromRange(cur, (Range){start, cur->pos});
            } else if(isnumber(c)) {
                bool is_float = advPastNum(cur);
                if(is_float) {
                    double num = rangeToDouble(cur, (Range){start, cur->pos});
                    return (Token){TK_DOUBLE, .decimal=num};
                } else {
                    int64_t num = rangeToInt(cur, (Range){start, cur->pos});
                    return (Token){TK_INTEGER, .integer=num};
                }
            }
            return (Token){ TK_UNKNOWN, .string=NULL};
    }
}

bool tokenHasInt(Token *tok) {
    //check if it's a token that stores a number
    for(size_t i = 0; i < NUM_NONSTRING_TOKENS; i++) {
        if(tok->type == non_string_tokens[i]) {
            return true;
        }
    }
    return false;
}

bool tokenHasStr(Token *tok) {
    if(tokenHasInt(tok)) return false;
    if(tok->string) return true;
    return false;
}

//token list stuff
void initTokenList(TokenList *list) {
    list->count = 0;
    list->max = 32;
    list->items = malloc(32 * sizeof(Token)); //32 items is a decent starting point
}

void addTokenToList(TokenList *list, Token *item) {
    if(list->count >= list->max) {
        size_t new_max = list->max * 2;
        Token *new_items = realloc(list->items, new_max * sizeof(Token));
        if(!new_items) {
            raiseError(ERR_MEM_ALLOC);
        }
        list->items = new_items;
        list->max = new_max;
    }
    list->items[list->count] = *item;
    list->count++;
}

void freeTokenListItems(TokenList *list) {
    for(size_t i = 0; i < list->count; i++) {
        if(tokenHasStr(&list->items[i])) { //if more number items are added, this line should be updated
            free(list->items[i].string); //frees a lot of NULLs, who cares
        }
    }
    free(list->items);
}


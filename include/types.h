#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//helper macros
#define SEC_TO_US 1000000.0
#define NS_TO_US 1000.0



//basic helper types
//the start and end range of a given run
typedef struct {
    uint32_t start;
    uint32_t end;
} Range;


typedef enum {
    VAL_DOUBLE,
    VAL_INT,
    VAL_BOOL,
    VAL_STRING,
    VAL_INVALID,
    VAL_VOID
} ValueType;

typedef struct {
    ValueType type;
    union {
        int64_t val_int;
        double val_float;
        bool val_bool;
        char* val_str;
    };
} Value;

//tokens
typedef enum {
    //keywords
    TK_START,
    TK_DONE,
    TK_ASSIGN,
    TK_FINAL,
    TK_FUN,
    TK_ENDFUN,
    TK_IF,
    TK_ELSE,
    TK_ENDIF,
    TK_WHILE,
    TK_ENDWHILE,
    TK_FINALIZE,
    TK_DISPLAY,
    TK_RETURN,
    TK_BREAK,
    //items
    TK_PLUS,
    TK_MINUS,
    TK_MULT,
    TK_DIV,
    TK_MODULO,
    TK_PLUS_INPLACE,
    TK_MINUS_INPLACE,
    TK_MULT_INPLACE,
    TK_DIV_INPLACE,
    TK_SEMICOLON,
    TK_EQ,
    TK_IS_EQ,
    TK_NOT_EQ,
    TK_GT,
    TK_LT,
    TK_GT_EQ,
    TK_LT_EQ,
    TK_LPAREN,
    TK_RPAREN,
    TK_COLON,
    TK_COMMA,
    TK_EOF,
    //strings and such
    TK_INTEGER,
    TK_DOUBLE,
    TK_BOOL,
    TK_IDENT,
    TK_VARNAME, //distinct! It's an ident that follows the varname pattern of x_foo
    TK_FUN_NAME, 
    TK_STRING,
    //unknown
    TK_EMPTY,
    TK_UNKNOWN,
    NUM_TOKENS
} TokenType;




//token type + data. May contain allocated data
typedef struct {
    TokenType type;
    union {
        int64_t integer;
        double decimal;
        bool boolean;
        char* string;
    };
} Token;

typedef struct {
    size_t count;
    size_t max;
    Token *items;
} TokenList;


//Token lookup table
typedef struct {
    const char *word;
    TokenType type;
} KeywordEntry;



//parsing types

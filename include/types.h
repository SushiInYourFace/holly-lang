#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//helper macros
#define SEC_TO_US 1000000.0
#define NS_TO_US 1000.0


//forward declares
typedef struct Value Value;


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
    VAL_CHAR,
    VAL_ARRAY,
    VAL_INVALID,
    VAL_VOID,
    VAL_UNSET
} ValueType;

typedef struct {
    size_t len;
    ValueType member_type;
    Value* arr;
} ValueArray;

struct Value{
    ValueType type;
    union {
        int64_t val_int;
        double val_float;
        bool val_bool;
        char* val_str;
        char val_char;
        ValueArray val_arr;
    };
};

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
    TK_LOOP,
    TK_ENDLOOP,
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
    TK_ARRAY_INDEX,
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
    TK_CHAR,
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
        char _char;
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

#pragma once

#include "types.h"
#include "uthash/uthash.h"
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>

#define MAX_FUN_PARAMS (8)

//categories of tokens that can be used in certain contexts
typedef enum {
    CAT_START_FILE,
    CAT_START_STATEMENT,
    CAT_COMPARATORS,
    CAT_PRIMARY,
    CAT_BINARY_ACTION,
    CAT_VAR_DECLARE,
    CAT_VAR_REASSIGN,
    CAT_ADDITIVE,
    CAT_MULTIPLICATIVE,
    CAT_ANY
} TokenCategory;

typedef struct {
    const TokenType *toks;
    size_t count;
} CategoryEntry;

//types of blocks to parse
typedef enum {
    BLOCK_MAIN,
    BLOCK_COND_NOT_LAST,
    BLOCK_COND_LAST,
    BLOCK_FUNCTION,
    BLOCK_WHILE
}BlockType;

//tree stuff

typedef enum {
    NODE_VALUE,
    NODE_BINARY,
    NODE_BLOCK,
    NODE_VAR_ASSIGN,
    NODE_VAR_REFERENCE,
    NODE_VAR_REASSIGN,
    NODE_VAR_FINALIZE,
    NODE_DISPLAY,
    NODE_CONDITIONAL,
    NODE_CONDITION,
    NODE_FUN_CALL,
    NODE_RETURN,
    NODE_WHILE_LOOP,
    NODE_BREAK
} NodeType;

typedef struct TreeNode TreeNode; //forward dec for recursion
typedef struct ParamValueNodes ParamValueNodes;

struct ParamValueNodes {
    TreeNode *list[MAX_FUN_PARAMS];
    size_t count;
};

//Param value nodes after being parsed
typedef struct ParamValues {
    Value list[MAX_FUN_PARAMS];
    size_t count;
} ParamValues;

struct TreeNode {
    NodeType type;
    union {
        Value value;
        struct {
            TreeNode *val;
        } display;
        struct {
            TreeNode *left;
            TreeNode *right;
            TokenType action;
        } binary;
        struct {
            BlockType type;
            TreeNode **children;
            size_t max;
            size_t len;
        } block;
        struct {
            char* name;
            bool final;
            bool set;
            TreeNode* val;
        } var_assign;
        struct {
            char* name;
        } var_reference;
        struct {
            char* name;
            TreeNode* val;
        } var_reassign;
        struct {
            char* name;
        } finalize;
        struct {
            TreeNode* condition;
            TreeNode* body;
            TreeNode* else_body;
        } conditional;
        struct {
            TreeNode* left;
            TokenType comparator;
            TreeNode* right;
        } condition;
        struct {
            TreeNode* condition;
            TreeNode* body;
        } while_loop;
        struct {
            char* fun_name;
            ParamValueNodes params;
        } fun_call;
        struct {
            TreeNode* val;
        } node_return;
    };
};

typedef struct {
    TokenList *list; //can use the same token list form, as long as we free it later
    size_t pos;
} Parser;

//parse a list of tokens, returns a pointer to the tree
TreeNode *parseMain(TokenList *list);

typedef struct {
    char *list[MAX_FUN_PARAMS];
    size_t count;
} ParamNames;

typedef Value (*BuiltinFun)(ParamValues);

typedef struct {
    char* fun_name;
    bool is_builtin; //if the function is a builtin, it needs to be evaluated differently
    ValueType return_type;
    union{
        TreeNode *fun_block;
        BuiltinFun builtin;
    };
    ParamNames param_names;
    UT_hash_handle hh;
} FunctionEntry;


void parseFunctions(TokenList *list, FunctionEntry **fun_hash);

//tree functions

//free a tree node recursively
void freeNode(TreeNode *node);
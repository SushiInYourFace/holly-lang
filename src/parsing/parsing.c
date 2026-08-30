#include "parsing/parsing.h"
#include "logging.h"
#include "types.h"
#include "utils.h"
#include "errors.h"
#include "functions.h"
#include "values.h"

static const TokenType start_statement_tokens[] =   
{TK_DONE,TK_SEMICOLON,TK_INTEGER,TK_ASSIGN,TK_FINAL,TK_FINALIZE,TK_DOUBLE,TK_VARNAME,TK_DISPLAY, 
TK_IF,TK_FUN_NAME,TK_RETURN, TK_WHILE, TK_BREAK};
static const TokenType var_declare_tokens[] =        {TK_ASSIGN, TK_FINAL};
static const TokenType var_reassign_tokens[] =      {TK_EQ, TK_PLUS_INPLACE, 
                                                    TK_MINUS_INPLACE, TK_MULT_INPLACE, TK_DIV_INPLACE};
static const TokenType primary_tokens[] =           {TK_INTEGER, TK_VARNAME, TK_DOUBLE, TK_BOOL,
                                                    TK_FUN_NAME, TK_STRING};
static const TokenType add_sub_tokens[] =           {TK_PLUS,TK_MINUS};
static const TokenType mult_div_tokens[] =          {TK_MULT, TK_DIV, TK_MODULO};
static const TokenType comparator_tokens[] =         {TK_IS_EQ, TK_NOT_EQ, TK_GT, TK_GT_EQ, 
                                                    TK_LT, TK_LT_EQ}; 

static const CategoryEntry categories[] = {
    [CAT_START_STATEMENT] =     {start_statement_tokens,    ARRAY_LEN(start_statement_tokens)},
    [CAT_PRIMARY] =             {primary_tokens,            ARRAY_LEN(primary_tokens)},
    [CAT_ADDITIVE] =            {add_sub_tokens,            ARRAY_LEN(add_sub_tokens)},      
    [CAT_MULTIPLICATIVE] =      {mult_div_tokens,           ARRAY_LEN(mult_div_tokens)},
    [CAT_COMPARATORS] =         {comparator_tokens,      ARRAY_LEN(comparator_tokens)},
    [CAT_VAR_DECLARE] =         {var_declare_tokens,        ARRAY_LEN(var_declare_tokens)},
    [CAT_VAR_REASSIGN] =        {var_reassign_tokens,       ARRAY_LEN(var_reassign_tokens)}
};

static const ParamNames params_empty = (ParamNames){.count=0,.list={0}};

static uint32_t num_nodes = 0;

//forward declares
static TreeNode *parseBlock(Parser *p, BlockType b_type);
static TreeNode *parseConditional(Parser *p);
static TreeNode *parseWhileLoop(Parser *p);
static TreeNode *parseFunCall(Parser *p);

//returns whether a token is in a given list
static bool tokenInList(TokenType tok, const TokenType *list, size_t len) {
    for(size_t i = 0; i < len; i++) {
        if(tok == list[i]) return true;
    }
    return false;
}

static bool tokenInCategory(TokenType tok, TokenCategory cat) {
    const CategoryEntry *entry = &categories[cat];
    return tokenInList(tok, entry->toks, entry->count);
}


//find where start point is
static int findEntry(TokenList *list) {
    for(size_t i = 0; i < list->count; i++) {
        if(list->items[i].type == TK_START) {
            return i;
        }
    }
    return -1;
}

static int findExit(TokenList *list) {
    for(size_t i = 0; i < list->count; i++) {
        if(list->items[i].type == TK_DONE) {
            return i;
        }
    }
    return -1;
}

//parser helpers
static Token *p_see(Parser *p) {
    return &p->list->items[p->pos];
}

static Token *p_advance(Parser *p) {
    Token *ret = &p->list->items[p->pos];
    if(p->pos < p->list->count) p->pos++;
    return ret;
}

//look ahead to the next token
static Token *p_lookahead(Parser *p) {
    Token *ret = &p->list->items[p->pos + 1];
    return ret;
}

static void p_rewind(Parser *p) {
    p->pos--;
}

//check the next token. Consume if valid, error otherwise
static Token *expectToken(Parser *p, TokenType t) {
    //don't use token in category here because we're checking specific token
    if(p->list->items[p->pos].type != t) {
        //TODO: can this just be p_see?
        raiseErrorWithCtx(ERR_UNEXPECTED_TOK, CTX_2TKTYPE, t, p->list->items[p->pos].type);
    }
    return p_advance(p);
}


//advance to next function. Return true if there is a function to parse, false otherwise
static bool p_findfun(Parser *p) {
    while(p_see(p)->type != TK_EOF) {
        if(p_see(p)->type == TK_FUN) {
            logDebug("Found new function");
            return true;
        } //stop advancing, return the start pos for next fun
        p_advance(p);
    }
    logDebug("Done parsing functions");
    return false;
}

static ParamNames p_getParams(Parser *p) {
    expectToken(p, TK_LPAREN);
    ParamNames ret = params_empty;
    Token *cur;
    while(p_see(p)->type != TK_RPAREN) {
        cur = expectToken(p, TK_VARNAME);
        ret.list[ret.count] = strdup(cur->string);
        ret.count++;
        if(p_see(p)->type != TK_RPAREN) {
            expectToken(p, TK_COMMA);
        }
    }
    p_advance(p); //past r paren
    logDebug("Parsed %d function params", ret.count);
    return ret;
}

//look ahead for else statement without changing parser location
static BlockType p_checkForElse(Parser *p) {
    Parser tmp = *p;
    while(p_see(&tmp)->type != TK_DONE && p_see(&tmp)->type != TK_EOF) {
        Token *next = p_advance(&tmp);
        if(next->type == TK_ENDIF) return BLOCK_COND_LAST;
        if(next->type == TK_ELSE) return BLOCK_COND_NOT_LAST;
    }
    raiseError(ERR_UNCLOSED_BLOCK);
    return BLOCK_COND_LAST;
}

//make tree items
static TreeNode *makeValueNode(Value val) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_VALUE;
    node->value = val;
    return node;
}

static TreeNode *makeBinaryNode(TreeNode *left, TokenType action, TreeNode *right) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_BINARY;
    node->binary.left = left;
    node->binary.right = right;
    node->binary.action = action;
    logNode("New binary node: %s", getTokenString(action));
    return node;
}

//make a block node that can hold other nodes
static TreeNode *makeBlockNode() {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_BLOCK;
    node->block.len = 0;
    //setup array of items
    node->block.children = malloc(8 * sizeof(TreeNode*));
    node->block.max = 8;
    logNode("New block node");
    return node;
}

//make a node that assigns a variable to an expression
static TreeNode *makeVarAssignNode(char* var_name, TreeNode *value_exp, bool is_final) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_VAR_ASSIGN;
    node->var_assign.name = var_name;
    node->var_assign.val = value_exp;
    node->var_assign.final = is_final;
    logNode("New var assign node");
    return node;
}

static TreeNode *makeVarReferenceNode(char* var_name) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_VAR_REFERENCE;
    node->var_reference.name = var_name;
    logNode("New var reference node");
    return node;
}

static TreeNode *makeVarReassignNode(char* var_name, TreeNode *value_exp) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_VAR_REASSIGN;
    node->var_reassign.name = var_name;
    node->var_reassign.val = value_exp;
    logNode("New var reassign node");
    return node;
}

//declare a non-final variable to be final
static TreeNode *makeFinalizeNode(char* var_name) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_VAR_FINALIZE;
    node->finalize.name = var_name;
    return node;
}

//call for an expression to be displayed to stdout
static TreeNode *makeDisplayNode(TreeNode *value_exp) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_DISPLAY;
    node->display.val = value_exp;
    return node;
}

//block of code to be executed if a condition is met
static TreeNode *makeConditionalNode(TreeNode *condition, TreeNode *body, TreeNode *else_body) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_CONDITIONAL;
    node->conditional.condition = condition;
    node->conditional.body = body;
    node->conditional.else_body = else_body;
    return node;
}

static TreeNode *makeWhileLoopNode(TreeNode *condition, TreeNode *body) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_WHILE_LOOP;
    node->while_loop.condition = condition;
    node->while_loop.body = body;
    return  node;
}

//a comparison or expression that evaluates to either true or false
static TreeNode *makeConditionNode(TreeNode *left, TokenType comparator, TreeNode *right) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_CONDITION;
    node->condition.left = left;
    node->condition.right = right;
    node->condition.comparator = comparator;
    return node;
}

//a node that calls a function
static TreeNode *makeFunCallNode(char *fun_name, ParamValues params) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_FUN_CALL;
    node->fun_call.fun_name = fun_name;
    node->fun_call.params = params;
    return node;
}

//a node that tells a block to return
static TreeNode *makeReturnNode(TreeNode *return_val) {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_RETURN;
    node->node_return.val = return_val;
    return node;
}

//node that tells a loop to break
static TreeNode *makeBreakNode() {
    num_nodes++;
    TreeNode *node = malloc(sizeof(TreeNode));
    node->type = NODE_BREAK;
    return node;
}

//parse functions

//parse basic data
static TreeNode *parsePrimary(Parser *p) {
    Token *t = p_see(p); 
    if(!tokenInCategory(t->type, CAT_PRIMARY)) { //need a primary token to parse correctly
        raiseErrorWithCtx(ERR_EXP_PRIMARY, CTX_1TKTYPE, t->type);
    }
    p_advance(p);
    //primary nodes are either values or idents
    switch(t->type) {
        case TK_INTEGER: 
            Value i_val = {.type=VAL_INT, .val_int = t->integer};
            return makeValueNode(i_val);
        case TK_DOUBLE:
            Value d_val = {.type=VAL_DOUBLE, .val_float = t->decimal};
            return makeValueNode(d_val);
        case TK_STRING:
            Value s_val = {.type=VAL_STRING, .val_str = strdup(t->string)}; //freed with the tree
            return makeValueNode(s_val);
        case TK_VARNAME:
            char* var_name = strdup(t->string); //detach string from token list
            return makeVarReferenceNode(var_name);
        case TK_BOOL:
            Value b_val = {.type=VAL_BOOL, .val_bool = t->boolean};
            return makeValueNode(b_val);
        case TK_FUN_NAME:
            //if it's a function, expect the ()
            p_rewind(p);
            return parseFunCall(p);
        default: raiseErrorWithCtx(ERR_UNKNOWN_PRIMARY, CTX_1TKTYPE, t->type);
    }
    exit(1); //gets debugger to stop yelling
}

//split this out because * / should be evaluated first. 
static TreeNode *parseMultiplicative(Parser *p) {
    //The output here is hopefully that x + y * z becomes x + (y * z)
    TreeNode *left = parsePrimary(p); //left will still always be primary

    while(tokenInCategory(p_see(p)->type, CAT_MULTIPLICATIVE)) {
        //if we land on a binary multiplicative, we go one level higher here
        TokenType act = p_advance(p)->type;
        TreeNode *right = parsePrimary(p); //since mult/div have highest prio, we can jsut go left to right
        left = makeBinaryNode(left, act, right);
    } 
    return left;
}

static TreeNode *parseExpression(Parser *p) {
    TreeNode *left = parseMultiplicative(p); //handle any mult/div on the left side

    while(tokenInCategory(p_see(p)->type,CAT_ADDITIVE)) { 
        //any time we land on a binary additive, we go one level higher in tree
        TokenType act = p_advance(p)->type; //take next token, which will be + or -
        TreeNode *right = parseMultiplicative(p); //parse any mult on the right of our + or -
        left = makeBinaryNode(left, act, right);
    }
    return left;
}

static TreeNode *parseVarAssign(Parser *p) {
    Token *dec_type = p_advance(p);
    if(!tokenInCategory(dec_type->type, CAT_VAR_DECLARE)) raiseError(ERR_EXP_VARASSIGN); 
    //tok needs to be a var dec
    bool is_final = (dec_type->type == TK_FINAL); //dec final if final
    //next token needs to be a keyword
    Token *name_tk = p_advance(p);
    if(name_tk->type != TK_VARNAME) raiseError(ERR_EXP_VARNAME); 
    char *varname = strdup(name_tk->string); //detatch from the token list
    //bool values need the next token parsed as a primary
    TreeNode *expression;
    if(p_see(p)->type == TK_BOOL) {
        expression = parsePrimary(p);
    } else {
        expression = parseExpression(p); //get the expression the var will be set to
    }
    return makeVarAssignNode(varname, expression, is_final);
}

//TODO: slim down
static TreeNode *parseVarReassign(Parser *p) {
    //get var we are reassigning
    Token *name_tk = p_advance(p);
    char *var_name = strdup(name_tk->string); //detach name from token list
    Token *op_tk = p_advance(p); //either a straight =, or an inplace op
    TreeNode *expression = parseExpression(p); //right side of the statement, new value for var
    TreeNode *var_reference, *bin_node = NULL;
    if(op_tk->type == TK_EQ) { //no inplace operation
        logNode("New = reassign node");
        return makeVarReassignNode((var_name), expression);
    }
    //inplace operations
    TokenType act = TK_EMPTY;
    switch(op_tk->type) {
        case TK_PLUS_INPLACE:   act = TK_PLUS; break;
        case TK_MINUS_INPLACE:  act = TK_MINUS; break;
        case TK_MULT_INPLACE:   act = TK_MULT; break;
        case TK_DIV_INPLACE:    act = TK_DIV; break;
        default: raiseErrorWithCtx(ERR_UNEXPECTED_TOK, CTX_1TKCAT_1TKTYPE, CAT_VAR_REASSIGN, op_tk->type); break;
    }
    var_reference = makeVarReferenceNode(strdup(var_name)); //each node gets own string
    bin_node = makeBinaryNode(var_reference, act, expression);
    logNode("New reassign node with operator %s", getTokenString(act));
    return makeVarReassignNode(var_name, bin_node);
}

static TreeNode *parseFinalize(Parser *p) {
    expectToken(p, TK_FINALIZE);
    Token *var = p_advance(p);
    if(var->type != TK_VARNAME) raiseError(ERR_EXP_VARNAME);
    char* var_str = strdup(var->string);
    logNode("New Finalize node");
    return makeFinalizeNode(var_str);

}

static TreeNode *parseDisplay(Parser *p) {
    expectToken(p, TK_DISPLAY);
    TreeNode *val = parseExpression(p);
    return makeDisplayNode(val);
}

static TreeNode *parseFunCall(Parser *p) {
    Token *fun_name_tk = expectToken(p, TK_FUN_NAME);
    expectToken(p, TK_LPAREN);
    ParamValues params = {.count = 0, .list = {NULL}};
    //get given params
    while(p_see(p)->type != TK_RPAREN) {
        params.list[params.count] = parseExpression(p);
        params.count++;
        if(p_see(p)->type == TK_RPAREN) {
            break;
        }
        expectToken(p, TK_COMMA);

    }
    expectToken(p, TK_RPAREN);
    char* fun_str = strdup(fun_name_tk->string); //detach string, will be owned by the created node
    return makeFunCallNode(fun_str, params);
}


static TreeNode *parseReturn(Parser *p) {
    expectToken(p, TK_RETURN);
    //may or may not have value attached
    TreeNode *return_val;
    if(p_see(p)->type == TK_SEMICOLON) {
        return_val = makeValueNode(VALUE_VOID);
    } else {
        return_val = parseExpression(p); //figure out what is actually being returned
    }
    return makeReturnNode(return_val);
}

static TreeNode *parseBreak(Parser *p) {
    expectToken(p, TK_BREAK);
    return makeBreakNode();
}

static TreeNode *parseStatement(Parser *p) {
    //if else rather than switch here, since some categories overlap. This makes everything easier
    TokenType tok = p_see(p)->type;
    TreeNode *ret = NULL;

    if(tokenInCategory(tok, CAT_PRIMARY)) { //a statement that starts with a primary
        if(tok == TK_VARNAME && tokenInCategory(p_lookahead(p)->type, CAT_VAR_REASSIGN)) {
            // check for reassigns
            ret = parseVarReassign(p);
        } else ret = parseExpression(p); //if it's not a reassign, it's a normal expression
    } else if (tok == TK_FINALIZE) { //finalize token
        ret = parseFinalize(p);
    } else if (tokenInCategory(tok, CAT_VAR_DECLARE)){ //assigning a new variable
        ret = parseVarAssign(p); //this will take care of the variable name and expression
    } else if (tok == TK_DISPLAY) {
        ret = parseDisplay(p);
    } else if (tok == TK_IF) {
        ret = parseConditional(p);
    } else if (tok == TK_FUN_NAME) { //currently, idents should ONLY be function calls
        ret = parseFunCall(p);
    } else if (tok == TK_RETURN){
        ret = parseReturn(p);
    } else if (tok == TK_WHILE) {
        ret = parseWhileLoop(p);
    } else if (tok == TK_BREAK) {
        ret = parseBreak(p);
    } else if (tok == TK_SEMICOLON) {
        logWarn("Parsed empty condition");
        p_advance(p);
        return NULL; //return empty if it's an empty statement
    } else {
        raiseError(ERR_UNEXPECTED_TOK);
    }
    //statements end in semicolons
    expectToken(p, TK_SEMICOLON);
    return ret;
}

static TreeNode *parseCondition(Parser *p) {
    expectToken(p, TK_LPAREN); //open the () for the evaluation
    TreeNode *left = parseExpression(p); //our statement will always have a left side
    if(p_see(p)->type == TK_RPAREN) { //if there's nothing being compared
        p_advance(p);
        return makeConditionNode(left, TK_EMPTY, NULL);
    }
    TokenType comparator = p_advance(p)->type;
    if(!tokenInCategory(comparator, CAT_COMPARATORS)) raiseError(ERR_EXP_COMPARATOR);
    TreeNode *right = parseExpression(p);
    expectToken(p, TK_RPAREN);
    return makeConditionNode(left, comparator, right);


}

static TreeNode *parseConditional(Parser *p) {
    expectToken(p, TK_IF); //declaration
    TreeNode *condition = parseCondition(p); //this should be a statement that evaluates to a bool
    expectToken(p, TK_COLON);
    BlockType b = p_checkForElse(p); //check if there is an else, parse accordingly
    TreeNode *body = parseBlock(p, b);
    TreeNode *else_body = NULL;
    if(b == BLOCK_COND_NOT_LAST) { //parse else if needed
        expectToken(p, TK_COLON); //parse block has consumed the TK_ELSE already
        else_body = parseBlock(p, BLOCK_COND_LAST);
    }
    return makeConditionalNode(condition, body, else_body);
}

static TreeNode *parseWhileLoop(Parser *p) {
    expectToken(p, TK_WHILE);
    TreeNode *condition = parseCondition(p);
    expectToken(p, TK_COLON);
    TreeNode *body = parseBlock(p, BLOCK_WHILE);
    return makeWhileLoopNode(condition, body);
}



//parse a full block of tokens
static TreeNode *parseBlock(Parser *p, BlockType b_type) {
    TreeNode *node = makeBlockNode();
    node->block.type = b_type;
    //get done keyword
    TokenType done_tk;
    switch(b_type) {
        case BLOCK_COND_NOT_LAST:   done_tk = TK_ELSE;      break;
        case BLOCK_COND_LAST:       done_tk = TK_ENDIF;     break;
        case BLOCK_MAIN:            done_tk = TK_DONE;      break;
        case BLOCK_FUNCTION:        done_tk = TK_ENDFUN;    break;
        case BLOCK_WHILE:           done_tk = TK_ENDWHILE;  break;
    }
    while(true) {
       if(p_see(p)->type == done_tk) break; //go until we get a done token
       if(p_see(p)->type == TK_DONE) raiseError(ERR_UNCLOSED_BLOCK); //if we are in a non-main block and see DONE
        //make sure we're parsing a valid statement
        if(!tokenInCategory(p_see(p)->type, CAT_START_STATEMENT)) {
            raiseErrorWithCtx(ERR_UNEXPECTED_TOK, CTX_1TKCAT_1TKTYPE, CAT_START_STATEMENT, p_see(p)->type);
        }
        TreeNode *statement = parseStatement(p);
        if(!statement) continue; //no need to add an empty node
        //add statement to block
        if(node->block.len >= node->block.max) {
            //grow block if needed
            size_t new_max = node->block.max * 2;
            TreeNode **new_list = realloc(node->block.children, new_max * sizeof(TreeNode*));
            if(!new_list) raiseError(ERR_MEM_ALLOC);
            node->block.children = new_list;
            node->block.max = new_max;
        }
        node->block.children[node->block.len] = statement;
        node->block.len++; //inc our number of children
    }
    p_advance(p); //advance past our done statement
    return node;
}

void parseFunctions(TokenList *list, FunctionEntry **fun_hash) {
    Parser p = {list, 0};
    while(p_findfun(&p)) {
        //advance to the next function, skip past the fun statement
        expectToken(&p, TK_FUN);
        Token *fun_name = expectToken(&p, TK_FUN_NAME);
        logVerbose("Encountered function with name %s", fun_name->string);
        ParamNames params = p_getParams(&p);
        TreeNode *fun_block = parseBlock(&p, BLOCK_FUNCTION);
        char* fun_name_str = strdup(fun_name->string);
        ValueType return_type = getFunReturnFromName(fun_name_str);
        if(return_type == VAL_INVALID) raiseError(ERR_INVALID_PREFIX);
        addFunctionToHash(fun_name_str, params, fun_block, fun_hash, return_type);

    }
}

TreeNode *parseMain(TokenList *list) {
    //make sure we have a valid start and done point, and that start comes before done
    const int start_point = findEntry(list);
    if(start_point == -1) raiseError(ERR_NO_START);
    const int done_point = findExit(list);
    if(done_point == -1) raiseError(ERR_NO_DONE);
    if(done_point <= start_point) raiseError(ERR_DONE_BEFORE_START);
    //after setup check, wrap our list in a parser
    Parser p = {list, start_point};
    //advance past our start; line
    expectToken(&p, TK_START);
    expectToken(&p, TK_SEMICOLON);

    TreeNode *tree = parseBlock(&p, BLOCK_MAIN);
    logVerbose("Created %d nodes", num_nodes);
    return tree;
}


//free tree nodes
void freeNode(TreeNode *node) {
    if(node == NULL) return; //don't need to free something empty

    switch(node->type) {
        case NODE_VALUE: 
            freeValue(node->value);
            break;
        case NODE_BINARY:
            freeNode(node->binary.left); //free left subtree
            freeNode(node->binary.right); //free right subtree
            break;
        case NODE_BLOCK:
            //free each node in the block
            for(size_t i = 0; i < node->block.len; i++) {
                freeNode(node->block.children[i]);
            }
            free(node->block.children); //free the array storing the children
            break;
        case NODE_VAR_ASSIGN:
            //free the char string
            free(node->var_assign.name);
            //free the value assignment
            freeNode(node->var_assign.val);
            break;
        case NODE_VAR_REFERENCE:
            free(node->var_reference.name);
            break;
        case NODE_VAR_REASSIGN: 
            free(node->var_reassign.name);
            freeNode(node->var_reassign.val);
            break;
        case NODE_VAR_FINALIZE:
            free(node->finalize.name);
            break;
        case NODE_DISPLAY:
            freeNode(node->display.val);
            break;
        case NODE_CONDITIONAL:
            freeNode(node->conditional.condition); //free condition
            freeNode(node->conditional.body); //free body
            freeNode(node->conditional.else_body); //free execution of the else statement
            break;
        case NODE_CONDITION:
            freeNode(node->condition.left);
            freeNode(node->condition.right);
            break;
        case NODE_WHILE_LOOP:
            freeNode(node->while_loop.condition);
            freeNode(node->while_loop.body);
            break;
        case NODE_FUN_CALL:
            free(node->fun_call.fun_name); //name of called function
            for(size_t i = 0; i < node->fun_call.params.count; i++) {
                freeNode(node->fun_call.params.list[i]); //param value
            }
            break;
        case NODE_RETURN:
            freeNode(node->node_return.val);
            break;
        case NODE_BREAK:
            break;
    }
    free(node);
}
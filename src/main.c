#include <stdio.h>
#include <unistd.h>

#include "errors.h"
#include "eval.h"
#include "logging.h"
#include "tokens.h"
#include "utils.h"
#include "functions.h"
#include "values.h"

int main(int argc, char** argv) {
    //options
    bool delay = false;
    int opt;
    char *in_file = NULL;
    while((opt = getopt(argc, argv, "hdf:")) != -1) {
        switch (opt) {
            case 'h': displayHelpText(); exit(0);
            case 'd': delay = true; break;
            case 'f': in_file = optarg; break;
        
        }
    }
    if(!in_file) raiseError(ERR_NO_IN_FILE);
    logInfo("Using source file %s", in_file);
    //timing
    struct timespec start, done_loadfile, done_token_gen, done_parsing, done_eval;
    clock_gettime(CLOCK_MONOTONIC, &start);
    Cursor cur;
    loadFile(in_file, &cur);
    clock_gettime(CLOCK_MONOTONIC, &done_loadfile);

    Token tok = {TK_UNKNOWN, .string=NULL};
    cur.pos = 0; //rewind for second run
    TokenList list;
    initTokenList(&list);
    while(tok.type != TK_EOF) { //get tokens
        tok = nextTokenData(&cur);
        addTokenToList(&list, &tok);
    }
    //we have all tokens, can get rid of our initial data
    free(cur.data);
    clock_gettime(CLOCK_MONOTONIC, &done_token_gen);
    //loop through tokens
    for(size_t i = 0; i < list.count; i++) {
        Token *c_tok = &list.items[i];
        if(tokenHasStr(c_tok)) {
            logToken("New Token %s: %s", getTokenString(c_tok->type), c_tok->string);
        } else if(c_tok->type == TK_INTEGER) {
            logToken("New Token %s: %lld", getTokenString(c_tok->type), c_tok->integer);
        } else if(c_tok->type == TK_DOUBLE) {
            logToken("New Token %s: %lf", getTokenString(c_tok->type), c_tok->decimal);           
        }
        else logToken("New Token %s", getTokenString(c_tok->type));
    }
    logVerbose("%zu total tokens", list.count);
    TreeNode *main_tree = parseMain(&list);
    FunctionEntry *fun_hash = NULL;
    parseFunctions(&list, &fun_hash);
    freeTokenListItems(&list);
    clock_gettime(CLOCK_MONOTONIC, &done_parsing);

    Value ret_val = eval(main_tree, NULL, fun_hash);
    clock_gettime(CLOCK_MONOTONIC, &done_eval);
    freeNode(main_tree);
    cleanFunctionHash(&fun_hash);
    char ret_val_str[VAL_STR_LEN];
    valueToString(ret_val, ret_val_str, VAL_STR_LEN);
    logInfo("Tree returned value of %s", ret_val_str);
    printf("Program finished with return %s\n", ret_val_str);
    freeValue(ret_val);
    logInfo("Program execution took %.4f us", getElapsedUs(start, done_eval));
    if(delay) {
        sleep(2);
    }
    return 0;
}
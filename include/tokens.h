#pragma once


#include "cursor.h"
#include "types.h"
#include <stdbool.h>

//Gets the next token type and range, advancing past it. 
Token nextTokenData(Cursor *cur);

//inits a TokenList object to hold the starting amount of 32 tokens
void initTokenList(TokenList *list);

//adds a new token to the list, growing capacity if need be
void addTokenToList(TokenList *list, Token *item);

//free all allocated memory in a token list
void freeTokenListItems(TokenList *list);

//check whether a given token contains an int
bool tokenHasInt(Token *tok);

//check whether a given token has a malloc-d string
bool tokenHasStr(Token *tok);

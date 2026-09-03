#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "cursor.h"
#include "errors.h"
#include "logging.h"

void loadFile(const char *path, Cursor *cur) {
    FILE *f = fopen(path, "rb");
    if(!f) {
        raiseError(ERR_FILE_OPEN);
    }

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    logVerbose("Loaded file of size %zu", size);
    rewind(f);

    char *buf = malloc(size + 1);
    fread(buf, 1, size, f);
    buf[size] = '\0';

    fclose(f);
    cur->data = buf;
    cur->len = size + 1;
    cur->pos = 0;
}

char advance(Cursor *cur) {
    assert(cur->pos < cur->len);
    char ret = cur->data[cur->pos];
    cur->pos++;
    return ret;
}

char see(Cursor *cur) {
    assert(cur->pos < cur->len);
    return cur->data[cur->pos];
}

char lookahead(Cursor *cur) {
    assert(cur->pos < cur->len);
    return cur->data[cur->pos + 1];
}

void advPastWhitespace(Cursor *cur) {
    while(see(cur) == ' ' || see(cur) == '\n') {
        advance(cur);
    }
}

void advPastStr(Cursor *cur) {
    //We already advanced past the first ", so current pos is start of run
    char c = advance(cur); //advance past next char, so in the case of "a", c = 'a', and cursor is over top of "
    while(c != '"' && c != '\0') {
        c = advance(cur);
    }
    //ensure the string was closed before we went out of bounds
    if(c == '\0') {
        raiseError(ERR_UNCLOSED_STR);
    }
}

char advPastCharDec(Cursor *cur) {
    char ret = advance(cur);
    if(advance(cur) != '\'') raiseError(ERR_INVALID_CHAR);
    return ret;
}

void advPastAlpha(Cursor *cur) {
    while(isalpha(see(cur))) {
        advance(cur); //advance until we are no longer over a letter
    }
}

void advPastIdent(Cursor *cur) {
    //only the second char is allowed to be a colon
    if(see(cur) == ':') advance(cur);
    while(isalpha(see(cur)) || see(cur) == '_') {
        advance(cur);
    }
}

void advPastInt(Cursor *cur) {
    while(isnumber(see(cur))) {
        advance(cur);
    }
}

bool advPastNum(Cursor *cur) {
    bool isFloat = false; //start by assuming number is not a float
    //account for negatives
    if(see(cur) == '-') { //advance past neg sign
        advance(cur);
    }
    advPastInt(cur); //advance past any int run
    if(see(cur) == '.') {
        //if we have a decimal place
        advance(cur); //advance past decimal
        isFloat = true; 
        advPastInt(cur); //adv past decimal portion of number
        if(see(cur) == '.') raiseError(ERR_INVALID_NUM); //guard against 3.5.5
    }
    return isFloat;

}

void advToNewline(Cursor *cur) {
    while(see(cur) != '\n' && see(cur) != '\0') {
        advance(cur);
    }
}

void advPastWhitespaceAndComments(Cursor *cur) {
    while(true) {
        advPastWhitespace(cur); //this also goes past newlines
        if(see(cur) == '~') {//if first non whitespace character we see is this, start a comment
            advToNewline(cur); //newlines break comments
        } else break;
    }
}

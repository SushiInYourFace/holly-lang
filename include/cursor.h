#pragma once


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//cursor object
typedef struct cursor {
    char* data;
    size_t len;
    uint32_t pos;
} Cursor;

//funcs

//load a file into the cursor
void loadFile(const char *path, Cursor *cur);

//read one letter and advance
char advance(Cursor *cur);

//read one letter without advancing
char see(Cursor *cur);

//read one letter in fronot of cursor
char lookahead(Cursor *cur);

//advances the cursor to the next non-space item
void advPastWhitespace(Cursor *cur);

//advances past the next double quote, returning the pos of the start and end of the quote
void advPastStr(Cursor *cur);

//advances until the next non alpha character
void advPastAlpha(Cursor *cur);

//advances until the next character that isn't alpha or an underscore
void advPastIdent(Cursor *cur);

//advances until the next non-numeric character
void advPastInt(Cursor *cur);

//advances past a number which may or may not have a decimal point. Returns true if the seen number is a float
bool advPastNum(Cursor *cur);

//advances until it hits a newline or EOF, stopping ON that char
void advToNewline(Cursor *cur);

//advances until it hits a character that isn't whitespace, newline, or part of a comment
void advPastWhitespaceAndComments(Cursor *cur);
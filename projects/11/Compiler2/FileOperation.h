#include<stdio.h>
#include<stdlib.h>
#include<stdarg.h>
#include<stdbool.h>

void error(char *fmt, ...);
bool isJackfile(char *filename);
bool isEOF(FILE *fp);
bool isEndOfLine(FILE *fp);
bool isSpace(FILE *fp);
bool isLineComment(FILE *fp);
bool isStartOfBlockComment(FILE *fp);
bool isEndOfBlockComment(FILE *fp);
void skipEndOfLine(FILE *fp);
void skipSpace(FILE *fp);
void skipLineComment(FILE *fp);
void skipBlockComment(FILE *fp);
bool isSkipToken(FILE *fp);
void skip(FILE *fp);
bool isSymbol(FILE *fp);
bool isInteger(FILE *fp);
bool isString(FILE *fp);
bool getSymbol(FILE *fp, char *token);
bool getStringConst(FILE *fp, char *token);
bool getIntConst(FILE *fp, char *token);
bool getKeywordOrIdentifier(FILE *fp, char *token);
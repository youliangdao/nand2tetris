#include "JackTokenizer.h"
#include "FileOperation.h"

JackTokenizer *tokenizerInit(FILE *fp_jack){
    static JackTokenizer this;
    this.fp_jack = fp_jack;
    fseek(this.fp_jack, 0, SEEK_SET);
    skip(this.fp_jack);
    strcpy(this.token, "");

    return &this;
}

bool hasMoreTokens(JackTokenizer *this){
    return !isEOF(this->fp_jack);
}

void advance(JackTokenizer *this){
    if(getSymbol(this->fp_jack, this->token)) this->tokenkind = TK_SYMBOL;
    else if(getIntConst(this->fp_jack, this->token)) this->tokenkind = TK_INTCONST;
    else if(getStringConst(this->fp_jack, this->token)) this->tokenkind = TK_STRCONST;
    else{
        getKeywordOrIdentifier(this->fp_jack, this->token);
        decideKeywordOrIdentifier(this);
    }
    skip(this->fp_jack);
}

TokenKind tokenType(JackTokenizer *this){
    return this->tokenkind;
}

KeywordKind keyWord(JackTokenizer *this){
    return this->keywordkind;
}

void symbol(JackTokenizer *this, char *symbol){
    if(this->tokenkind == TK_SYMBOL) strcpy(symbol, this->token);
}

void identifier(JackTokenizer *this, char *identifier){
    if(this->tokenkind == TK_IDENTIFIER) strcpy(identifier, this->token);
}

void intVal(JackTokenizer *this, int *intVal){
    if(this->tokenkind == TK_INTCONST) *intVal = atoi(this->token);
}

void stringVal(JackTokenizer *this, char *stringVal){
    if(this->tokenkind == TK_STRCONST) strcpy(stringVal, this->token);
}
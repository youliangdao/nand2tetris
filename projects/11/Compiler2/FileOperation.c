#include "FileOperation.h"
#include "JackTokenizer.h"

void error(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

bool isJackfile(char *filename){
    size_t len1 = strlen(filename);
    char *suffix = ".jack";
    size_t len2 = strlen(suffix);
    if(len1 <= len2) return false;
    if(strcmp(filename+len1-len2, suffix) != 0) return false;

    return true;
}

bool isEOF(FILE *fp){
    int c = fgetc(fp);
    ungetc(c, fp);
    if(c == EOF) return true;
    else return false;
}

bool isEndOfLine(FILE *fp){
    int c = fgetc(fp);
    ungetc(c, fp);
    if((char)c == '\n' || (char)c == '\r') return true;
    else return false;
}

bool isSpace(FILE *fp){
    int c = fgetc(fp);
    ungetc(c, fp);
    if((char)c == ' ' || (char)c == '\t') return true;
    else return false;
}

bool isLineComment(FILE *fp){
    int c1 = fgetc(fp);
    if((char)c1 != '/'){
        ungetc(c1, fp);
        return false;
    }

    int c2 = fgetc(fp);
    ungetc(c2, fp);
    ungetc(c1, fp);

    if((char)c2 == '/') return true;
    else return false;
}

bool isStartOfBlockComment(FILE *fp){
    int c1 = fgetc(fp);
    if((char)c1 != '/'){
        ungetc(c1, fp);
        return false;
    }

    int c2 = fgetc(fp);
    ungetc(c2, fp);
    ungetc(c1, fp);

    if((char)c2 == '*') return true;
    else return false;
}

bool isEndOfBlockComment(FILE *fp){
    int c1 = fgetc(fp);
    if((char)c1 != '*'){
        ungetc(c1, fp);
        return false;
    }

    int c2 = fgetc(fp);
    ungetc(c2, fp);
    ungetc(c1, fp);
    
    if((char)c2 == '/') return true;
    else return false;
}

void skipEndOfLine(FILE *fp){
    while(isEndOfLine(fp)) fgetc(fp);
}

void skipSpace(FILE *fp){
    while(isSpace(fp)) fgetc(fp);
}

void skipLineComment(FILE *fp){
    if(isLineComment(fp)){
        do fgetc(fp); while(!(isEndOfLine(fp) || isEOF(fp)));
    }
}

void skipBlockComment(FILE *fp){
    if(isStartOfBlockComment(fp)){
        do fgetc(fp); while(!(isEndOfBlockComment(fp)));
        fgetc(fp); fgetc(fp);
    }
}

bool isSkipToken(FILE *fp){
    return (isSpace(fp) || 
            isEOF(fp) || 
            isEndOfLine(fp) || 
            isStartOfBlockComment(fp) || 
            isEndOfBlockComment(fp));
}

void skip(FILE *fp){
    do{
        skipEndOfLine(fp);
        skipSpace(fp);
        skipLineComment(fp);
        skipBlockComment(fp);
    }while(!isEOF(fp) && isSkipToken(fp));
}

bool isSymbol(FILE *fp){
    char symbols[] = {
        '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-',
        '*', '/', '&', '|', '<', '>', '=', '~'
    };
    int c = fgetc(fp);
    ungetc(c, fp);
    for(size_t i=0;i<sizeof(symbols);++i){
        if((char)c == symbols[i]) return true;
    }
    return false;
}

bool isInteger(FILE *fp){
    int c = fgetc(fp);
    ungetc(c, fp);
    if(isdigit(c)) return true;
    else return false;
}

bool isString(FILE *fp){
    int c = fgetc(fp);
    ungetc(c, fp);
    if((char)c == '"') return true;
    else return false;
}

bool getSymbol(FILE *fp, char *token){
    if(!isSymbol(fp)) return false;

    int c = fgetc(fp);
    token[0] = (char)c;
    token[1] = '\0';
    return true;
}

bool getStringConst(FILE *fp, char *token){
    if(!isString(fp)) return false;

    int c = fgetc(fp);
    int i = 0;
    do{
        c = fgetc(fp);
        token[i] = (char)c;
        ++i;
    }while(!isString(fp));
    token[i] = '\0';
    c = fgetc(fp);
    return true;
}

bool getIntConst(FILE *fp, char *token){
    if(!isInteger(fp)) return false;

    int i = 0;
    do{
        int c = fgetc(fp);
        token[i] = (char)c;
        ++i;
    }while(isInteger(fp));
    token[i] = '\0';
    return true;
}

bool getKeywordOrIdentifier(FILE *fp, char *token){
    int i = 0;
    while(!(isSkipToken(fp) || isSymbol(fp))){
        int c = fgetc(fp);
        token[i] = (char)c;
        ++i;
    } 
    token[i] = '\0';
    return true;
}

void decideKeywordOrIdentifier(JackTokenizer *this){
    bool flag = false;
    for(size_t i=0;i<sizeof(keywords)/sizeof(keywords[0]);++i){
        if(strcmp(this->token, keywords[i].str) == 0){
            this->tokenkind = TK_KEYWORD;
            this->keywordkind = keywords[i].kind;
            flag = true;
            break;
        }
    }

    if(!flag) this->tokenkind = TK_IDENTIFIER;
}
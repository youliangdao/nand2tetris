#include "CompilationEngine.h"
#include "VMWriter.h"

CompilationEngine *compilationEngineInit(FILE *fp_jack, FILE *fp_xml){
    static CompilationEngine comp;
    comp.tokenizer = tokenizerInit(fp_jack);
    comp.fp_xml = fp_xml;
    return &comp;
}

void advanceWrite(CompilationEngine *comp){
    advance(comp->tokenizer);
    writeToken(comp->fp_xml, comp->tokenizer);
}

bool isSymbolToken(CompilationEngine *comp, char *token){
    if(tokenType(comp->tokenizer) != TK_SYMBOL) return false;

    char tok[20];
    symbol(comp->tokenizer, tok);

    if(strcmp(token, tok) == 0) return true;
    else return false;
}

bool isKeyword(CompilationEngine *comp, KeywordKind kind){
    if(tokenType(comp->tokenizer) != TK_KEYWORD) return false;
    
    if(keyWord(comp->tokenizer) == kind) return true;
    else return false;
}

void compileClass(CompilationEngine *comp){
    advance(comp->tokenizer);

    advance(comp->tokenizer);
    char className[50];
    identifier(comp->tokenizer, className);

    advance(comp->tokenizer);
    advance(comp->tokenizer);

    while(!isSymbolToken(comp, "}")){
        if(isKeyword(comp, KW_STATIC) || isKeyword(comp, KW_FIELD)) compileClassVarDec(comp);
        else if(isKeyword(comp, KW_CONSTRUCTOR) || isKeyword(comp, KW_FUNCTION) || isKeyword(comp, KW_METHOD)){
            compileSubroutineDec(comp, className);
        }
        else break;
    }
}

void compileClassVarDec(CompilationEngine *comp){
    SymbolKind symkind;
    if(isKeyword(comp, KW_STATIC)) symkind = SYMBOL_STATIC;
    else if(isKeyword(comp, KW_FIELD)) symkind = SYMBOL_FIELD;
    else error("Error in \"compileClassVarDec: undefined symbol kind.");

    advance(comp->tokenizer);
    TokenKind tokind = tokenType(comp->tokenizer);
    char type[50];

    if(tokind == TK_KEYWORD){
        if(keyWord(comp->tokenizer) == KW_INT) strcpy(type, "int");
        else if(keyWord(comp->tokenizer) == KW_CHAR) strcpy(type, "char");
        else if(keyWord(comp->tokenizer) == KW_BOOLEAN) strcpy(type, "boolean");
        else error("Error in \"compileClassVarDec: invalid variable type.");
    }
    else{
        identifier(comp->tokenizer, type);
    }

    advance(comp->tokenizer);
    char varName[50];
    identifier(comp->tokenizer, varName);
    compile_varDec(varName, type, symkind, SCOPE_CLASS);

    advance(comp->tokenizer);

    while(!isSymbolToken(comp, ";")){
        advance(comp->tokenizer);
        char varName[50];
        identifier(comp->tokenizer, varName);
        compile_varDec(varName, type, symkind, SCOPE_CLASS);

        advance(comp->tokenizer);
    }
    advance(comp->tokenizer);
}

void compileSubroutineDec(CompilationEngine *comp, char *className){
    startSubroutine();

    KeywordKind kwKind;
    if(isKeyword(comp, KW_CONSTRUCTOR)) kwKind = KW_CONSTRUCTOR;
    else if(isKeyword(comp, KW_FUNCTION)) kwKind = KW_FUNCTION;
    else if(isKeyword(comp, KW_METHOD)) kwKind = KW_METHOD;
    else error("Error in \"compileSubroutineDec\": invaild function type.");

    advance(comp->tokenizer);
    char type[50];
    
    if(isKeyword(comp, KW_VOID)) strcpy(type, "void");

    else{
        TokenKind tokind = tokenType(comp->tokenizer);
        if(tokind == TK_KEYWORD){
        if(keyWord(comp->tokenizer) == KW_INT) strcpy(type, "int");
        else if(keyWord(comp->tokenizer) == KW_CHAR) strcpy(type, "char");
        else if(keyWord(comp->tokenizer) == KW_BOOLEAN) strcpy(type, "boolean");
        else error("Error in \"compileSubroutineDec\": invalid variable type.");
        }
        else{
            identifier(comp->tokenizer, type);
        }
    }
    
    advance(comp->tokenizer);
    char subroutineName[50];
    identifier(comp->tokenizer, subroutineName);

    advance(comp->tokenizer);

    if(kwKind == KW_METHOD){
        int cnt = varCount(SCOPE_SUBROUTINE, SYMBOL_ARG);
        define("this", type, cnt, SYMBOL_ARG, SCOPE_SUBROUTINE);
    }

    advance(comp->tokenizer);
    compileParameterList(comp);

    advance(comp->tokenizer);
    compileSubroutineBody(comp, className, subroutineName, kwKind);
}

int compileSubroutineBody(CompilationEngine *comp, char *className, char *subroutineName, KeywordKind kwKind){
    advance(comp->tokenizer);
    int nLocals = 0;

    while(isKeyword(comp, KW_VAR)){
        int varNum = compileVarDec(comp);
        nLocals += varNum;
    }
    char VMfunctionName[100];
    strcpy(VMfunctionName, className);
    strcat(VMfunctionName, ".");
    strcat(VMfunctionName, subroutineName);
    writeFunction(VMfunctionName, nLocals, comp->fp_xml);

    if(kwKind == KW_CONSTRUCTOR){
        writePush(SEG_CONST, varCount(SCOPE_CLASS, SYMBOL_FIELD), comp->fp_xml);

        writeCall("Memory.alloc", 1, comp->fp_xml);
        writePop(SEG_POINTER, 0, comp->fp_xml);
    }
    else if(kwKind == KW_METHOD){
        writePush(SEG_ARG, 0, comp->fp_xml);
        writePop(SEG_POINTER, 0, comp->fp_xml);
    }
    else if(kwKind == KW_FUNCTION){
    }
    else error("Error in \"compileSubroutineBody\": invalid function type.");
    compileStatements(comp, className);

    advance(comp->tokenizer);
    return nLocals;
}

void compileParameterList(CompilationEngine *comp){
    if(!isSymbolToken(comp, ")")){
        char type[50];
        identifier(comp->tokenizer, type);

        advance(comp->tokenizer);
        char varName[50];
        identifier(comp->tokenizer, varName);
        compile_varDec(varName, type, SYMBOL_ARG, SCOPE_SUBROUTINE);

        advance(comp->tokenizer);

        while(isSymbolToken(comp, ",")){
            advance(comp->tokenizer);
            char type[50];
            identifier(comp->tokenizer, type);

            advance(comp->tokenizer);
            char varName[50];
            identifier(comp->tokenizer, varName);
            compile_varDec(varName, type, SYMBOL_ARG, SCOPE_SUBROUTINE);

            advance(comp->tokenizer);
        }
    }
}

int compileVarDec(CompilationEngine *comp){
    int nArgs = 0;
    advance(comp->tokenizer);
    TokenKind tokind = tokenType(comp->tokenizer);
    char type[50];

    if(tokind == TK_KEYWORD){
        if(keyWord(comp->tokenizer) == KW_INT) strcpy(type, "int");
        else if(keyWord(comp->tokenizer) == KW_CHAR) strcpy(type, "char");
        else if(keyWord(comp->tokenizer) == KW_BOOLEAN) strcpy(type, "boolean");
        else error("Error in \"compileClassVarDec: invalid variable type.");
    }
    else{
        identifier(comp->tokenizer, type);
    }

    advance(comp->tokenizer);
    char varName[50];
    identifier(comp->tokenizer, varName);
    compile_varDec(varName, type, SYMBOL_VAR, SCOPE_SUBROUTINE);

    ++nArgs;

    advance(comp->tokenizer);

    while(!isSymbolToken(comp, ";")){
        advance(comp->tokenizer);
        char varName[50];
        identifier(comp->tokenizer, varName);
        compile_varDec(varName, type, SYMBOL_VAR, SCOPE_SUBROUTINE);
        ++nArgs;

        advance(comp->tokenizer);
    }
    advance(comp->tokenizer);

    return nArgs;
}

void compileStatements(CompilationEngine *comp, char *className){
    while(isKeyword(comp, KW_LET) || isKeyword(comp, KW_IF) || isKeyword(comp, KW_WHILE) || 
        isKeyword(comp, KW_DO) || isKeyword(comp, KW_RETURN)){
        
        if(isKeyword(comp, KW_LET)) compileLet(comp, className);
        else if(isKeyword(comp, KW_IF)) compileIf(comp, className);
        else if(isKeyword(comp, KW_WHILE)) compileWhile(comp, className);
        else if(isKeyword(comp, KW_DO)) compileDo(comp, className);
        else compileReturn(comp, className);
    }
}

void compileSubroutineCall(CompilationEngine *comp, char *className, char *varName, bool fromTerm){
    if(!fromTerm) advance(comp->tokenizer);
    if(isSymbolToken(comp, "(")){
        char subroutineName[50];
        strcpy(subroutineName, varName);
        advance(comp->tokenizer);
        writePush(SEG_POINTER, 0, comp->fp_xml);
        int nArgs = compileExpressionList(comp, className);
        
        char VMfunctionName[100];
        strcpy(VMfunctionName, className);
        strcat(VMfunctionName, ".");
        strcat(VMfunctionName, subroutineName);
        writeCall(VMfunctionName, nArgs + 1, comp->fp_xml);

        advance(comp->tokenizer);
    }
    else if(isSymbolToken(comp, ".")){
        SymbolKind kind = kindOf(varName);
        if(kind == SYMBOL_NONE){

            char className[50];
            strcpy(className, varName);
            advance(comp->tokenizer);
            char subroutineName[50];
            identifier(comp->tokenizer, subroutineName);
            advance(comp->tokenizer);

            advance(comp->tokenizer);
            int nArgs = compileExpressionList(comp, className);
            char VMfunctionName[100];

            strcpy(VMfunctionName, className);
            strcat(VMfunctionName, ".");
            strcat(VMfunctionName, subroutineName);
            writeCall(VMfunctionName, nArgs, comp->fp_xml);
            advance(comp->tokenizer);
        }
        else{
            advance(comp->tokenizer);
            char subroutineName[50];
            identifier(comp->tokenizer, subroutineName);

            advance(comp->tokenizer);
            if(kind == SYMBOL_STATIC){
                writePush(SEG_STATIC, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_FIELD){
                writePush(SEG_THIS, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_ARG){
                writePush(SEG_ARG, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_VAR){
                writePush(SEG_LOCAL, indexOf(varName), comp->fp_xml);
            }

            advance(comp->tokenizer);
            int nArgs = compileExpressionList(comp, className);

            char className[50];
            typeOf(varName, className);
            char VMfunctionName[100];
            strcpy(VMfunctionName, className);
            strcat(VMfunctionName, ".");
            strcat(VMfunctionName, subroutineName);
            writeCall(VMfunctionName, nArgs + 1, comp->fp_xml);

            advance(comp->tokenizer);
        }
    }
    else error("Error in \"compileSubroutineCall\": invalid sub-routine call.");
}

void compileDo(CompilationEngine *comp, char *className){
    advance(comp->tokenizer);

    char varName[50];
    identifier(comp->tokenizer, varName);
    compileSubroutineCall(comp, className, varName, false);

    writePop(SEG_TEMP, 0, comp->fp_xml);

    advance(comp->tokenizer);
}

void compileLet(CompilationEngine *comp, char *className){
    advance(comp->tokenizer);

    char varName[50];
    identifier(comp->tokenizer, varName);

    advance(comp->tokenizer);

    if(isSymbolToken(comp, "[")){

        SymbolKind kind = kindOf(varName);
        if(kind == SYMBOL_STATIC){
            writePush(SEG_STATIC, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_FIELD){
            writePush(SEG_THIS, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_ARG){
            writePush(SEG_ARG, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_VAR){
            writePush(SEG_LOCAL, indexOf(varName), comp->fp_xml);
        }
        
        advance(comp->tokenizer);
        compileExpression(comp, className);
        advance(comp->tokenizer);
        advance(comp->tokenizer);
        
        writeArithmetic(COM_ADD, comp->fp_xml);
        compileExpression(comp, className);
        writePop(SEG_TEMP, 2, comp->fp_xml);
        writePop(SEG_POINTER, 1, comp->fp_xml);
        writePush(SEG_TEMP, 2, comp->fp_xml);        
        writePop(SEG_THAT, 0, comp->fp_xml);

        advance(comp->tokenizer);
    }
    else{
        advance(comp->tokenizer);

        SymbolKind kind = kindOf(varName);

        compileExpression(comp, className);

        if(kind == SYMBOL_STATIC){
            writePop(SEG_STATIC, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_FIELD){
            writePop(SEG_THIS, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_ARG){
            writePop(SEG_ARG, indexOf(varName), comp->fp_xml);
        }
        else if(kind == SYMBOL_VAR){
            writePop(SEG_LOCAL, indexOf(varName), comp->fp_xml);
        }

        advance(comp->tokenizer);
    }
}

void compileWhile(CompilationEngine *comp, char *className){
    char label1[20];
    char label2[20];
    getLabel(label1);
    getLabel(label2);

    writeLabel(label1, comp->fp_xml);
    advance(comp->tokenizer);
    
    advance(comp->tokenizer);
    compileExpression(comp, className);

    writeArithmetic(COM_NOT, comp->fp_xml);
    writeIf(label2, comp->fp_xml);
    advance(comp->tokenizer);

    advance(comp->tokenizer);
    compileStatements(comp, className);

    writeGoto(label1, comp->fp_xml);
    writeLabel(label2, comp->fp_xml);
    
    advance(comp->tokenizer);
}

void compileReturn(CompilationEngine *comp, char *className){

    advance(comp->tokenizer);

    if(!isSymbolToken(comp, ";")){
        compileExpression(comp, className);
    }
    else{
        writePush(SEG_CONST, 0, comp->fp_xml);
    }

    writeReturn(comp->fp_xml);
    advance(comp->tokenizer);
}

void compileIf(CompilationEngine *comp, char *className){
    advance(comp->tokenizer);

    advance(comp->tokenizer);
    compileExpression(comp, className);

    char label1[20];
    char label2[20];
    getLabel(label1);
    getLabel(label2);
    writeArithmetic(COM_NOT, comp->fp_xml);
    writeIf(label1, comp->fp_xml);
    advance(comp->tokenizer);

    advance(comp->tokenizer);
    compileStatements(comp, className);

    advance(comp->tokenizer);
    writeGoto(label2, comp->fp_xml);
    writeLabel(label1, comp->fp_xml);
    if(isKeyword(comp, KW_ELSE)){
        advance(comp->tokenizer);
        
        advance(comp->tokenizer);
        compileStatements(comp, className);

        advance(comp->tokenizer);
    }

    writeLabel(label2, comp->fp_xml);
}

void compileExpression(CompilationEngine *comp, char *className){
    compileTerm(comp, className);


    while(isSymbolToken(comp, "+") || isSymbolToken(comp, "-") || isSymbolToken(comp, "*") || 
        isSymbolToken(comp, "/") || isSymbolToken(comp, "&") || isSymbolToken(comp, "|") || 
        isSymbolToken(comp, "<") || isSymbolToken(comp, ">") || isSymbolToken(comp, "=")){

        char sym[5];
        symbol(comp->tokenizer, sym);
        advance(comp->tokenizer);
        compileTerm(comp, className);
        if(strcmp(sym, "+") == 0) writeArithmetic(COM_ADD, comp->fp_xml);
        else if(strcmp(sym, "-") == 0) writeArithmetic(COM_SUB, comp->fp_xml);
        else if(strcmp(sym, "*") == 0) writeCall("Math.multiply", 2, comp->fp_xml);
        else if(strcmp(sym, "/") == 0) writeCall("Math.divide", 2, comp->fp_xml);
        else if(strcmp(sym, "&") == 0) writeArithmetic(COM_AND, comp->fp_xml);
        else if(strcmp(sym, "|") == 0) writeArithmetic(COM_OR, comp->fp_xml);
        else if(strcmp(sym, "<") == 0) writeArithmetic(COM_LT, comp->fp_xml);
        else if(strcmp(sym, ">") == 0) writeArithmetic(COM_GT, comp->fp_xml);
        else if(strcmp(sym, "=") == 0) writeArithmetic(COM_EQ, comp->fp_xml);
    }
}

void compileTerm(CompilationEngine *comp, char *className){

    if(tokenType(comp->tokenizer) == TK_INTCONST || tokenType(comp->tokenizer) == TK_STRCONST ||
        tokenType(comp->tokenizer) == TK_KEYWORD){
        if(tokenType(comp->tokenizer) == TK_INTCONST){
            int intval;
            intVal(comp->tokenizer, &intval);
            writePush(SEG_CONST, intval, comp->fp_xml);
        }
        else if(tokenType(comp->tokenizer) == TK_STRCONST){
            char str[100];
            stringVal(comp->tokenizer, str);
            int len = strlen(str);
            writePush(SEG_CONST, len, comp->fp_xml);
            writeCall("String.new", 1, comp->fp_xml);
            for(int i=0;i<len;++i){
                writePush(SEG_CONST, (int)str[i], comp->fp_xml);
                writeCall("String.appendChar", 2, comp->fp_xml);
            }
        }
        else{
            if(keyWord(comp->tokenizer) == KW_TRUE){
                writePush(SEG_CONST, 1, comp->fp_xml);
                writeArithmetic(COM_NEG, comp->fp_xml);
            }
            else if(keyWord(comp->tokenizer) == KW_FALSE || keyWord(comp->tokenizer) == KW_NULL){
                writePush(SEG_CONST, 0, comp->fp_xml);
            }
            else if(keyWord(comp->tokenizer) == KW_THIS){
                writePush(SEG_POINTER, 0, comp->fp_xml);
            }
        }
        advance(comp->tokenizer);
    }
    else if(isSymbolToken(comp, "(")){

        advance(comp->tokenizer);
        compileExpression(comp, className);

        advance(comp->tokenizer);
    }
    else if(isSymbolToken(comp, "-") || isSymbolToken(comp, "~")){
        if(isSymbolToken(comp, "-")){
            advance(comp->tokenizer);
            compileTerm(comp, className);
            writeArithmetic(COM_NEG, comp->fp_xml);
        }
        else{
            advance(comp->tokenizer);
            compileTerm(comp, className);
            writeArithmetic(COM_NOT, comp->fp_xml);
        }
    }
    else{
        char varName[50];
        identifier(comp->tokenizer, varName);
        char token[50];
        advance(comp->tokenizer);

        if(isSymbolToken(comp, "[")){
            advance(comp->tokenizer);

            SymbolKind kind = kindOf(varName);
            if(kind == SYMBOL_STATIC){
                writePush(SEG_STATIC, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_FIELD){
                writePush(SEG_THIS, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_ARG){
                writePush(SEG_ARG, indexOf(varName), comp->fp_xml);
            }
            else if(kind == SYMBOL_VAR){
                writePush(SEG_LOCAL, indexOf(varName), comp->fp_xml);
            }

            compileExpression(comp, className);

            writeArithmetic(COM_ADD, comp->fp_xml);
            writePop(SEG_POINTER, 1, comp->fp_xml);
            writePush(SEG_THAT, 0, comp->fp_xml);

            advance(comp->tokenizer);
        }
        else if(isSymbolToken(comp, "(") || isSymbolToken(comp, ".")){
            compileSubroutineCall(comp, className, varName, true);
        }
        else{
            compileWrite_varName(varName, comp);
        }
    }
}

int compileExpressionList(CompilationEngine *comp, char *className){
    int nArgs = 0;
    if(!isSymbolToken(comp, ")")){
        compileExpression(comp, className);
        ++nArgs;

        while(isSymbolToken(comp, ",")){
            
            advance(comp->tokenizer);
            compileExpression(comp, className);
            ++nArgs;
        }
    }

    return nArgs;
}

void compileWrite_varName(char *varName, CompilationEngine *comp){
    printf("%s\n", varName);
    SymbolKind kind = kindOf(varName);
    if(kind == SYMBOL_STATIC){
        writePush(SEG_STATIC, indexOf(varName), comp->fp_xml);
    }
    else if(kind == SYMBOL_FIELD){
        writePush(SEG_THIS, indexOf(varName), comp->fp_xml);
    }
    else if(kind == SYMBOL_ARG){
        writePush(SEG_ARG, indexOf(varName), comp->fp_xml);
    }
    else if(kind == SYMBOL_VAR){
        writePush(SEG_LOCAL, indexOf(varName), comp->fp_xml);
    }
    else error("Error in \"compileWrite_varName\": undefined symbol kind.");
}

void compile_varDec(char *varName, char *type, SymbolKind symkind, ScopeKind scpkind){
    int cnt = varCount(scpkind, symkind);
    define(varName, type, cnt, symkind, scpkind);
}
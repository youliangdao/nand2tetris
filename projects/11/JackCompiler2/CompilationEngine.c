#include "CompilationEngine.h"
#include "JackTokenizer.h"
#include "VMWriter.h"
#include "string.h"
#include <stdarg.h>

void writeToken(FILE *fp, JackTokenizer tokenizer);
void writeKeyword(FILE *fp, JackTokenizer tokenizer);
void writeSymbol(FILE *fp, JackTokenizer tokenizer);
void writeIdentifier(FILE *fp, JackTokenizer tokenizer);
void writeIntegerConstant(FILE *fp, JackTokenizer tokenizer);
void writeStringConstant(FILE *fp, JackTokenizer tokenizer);
void advanceAndWriteToken(CompilationEngine thisObject);
bool isKeywordToken(CompilationEngine thisObject, JackTokenizer_Keyword keyword);
bool isSymbolToken(CompilationEngine thisObject, char *symbol);
bool inSymbolListToken(CompilationEngine thisObject, ...);

typedef struct compilation_engine * CompilationEngine;
struct compilation_engine
{
    JackTokenizer tokenizer;
    FILE* fpXml;
};

CompilationEngine CompilationEngine_init(FILE *fpJack, FILE *fpXml)
{
    static struct compilation_engine thisObject;

    thisObject.tokenizer = JackTokenizer_init(fpJack);
    thisObject.fpXml = fpXml;

    return &thisObject;
}

// 'class' className '{' classVarDec* subroutineDec* '}'
void CompilationEngine_compileClass(CompilationEngine thisObject)
{
    JackTokenizer_advance(thisObject->tokenizer);   // 'class'
    JackTokenizer_advance(thisObject->tokenizer);   // className
    char className[50];
    JackTokenizer_identifier(thisObject->tokenizer, className);

    JackTokenizer_advance(thisObject->tokenizer);   // '{'

    JackTokenizer_advance(thisObject->tokenizer);   // '}' or not
    while (! isSymbolToken(thisObject, "}")) {
        if (
            isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_STATIC) ||
            isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_FIELD)
        ) {  // classVarDec
            CompilationEngine_compileClassVarDec(thisObject);
        } else if (
            isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_CONSTRUCTION) ||
            isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_FUNCTION) ||
            isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_METHOD)
        ) {  // subroutineDec
            CompilationEngine_compileSubroutine(thisObject, className);
        } else {
            break;
        }
    }
}

// ('static' | 'field') type varName (',' varName)* ';'
void CompilationEngine_compileClassVarDec(CompilationEngine thisObject)
{
    SymbolKind symkind;

    // ('static' | 'field')
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_STATIC))
    {
        symkind = SYMBOL_STATIC;
    }
    else
    {
        symkind = SYMBOL_FIELD;
    }

    JackTokenizer_advance(thisObject->tokenizer);   // type
    JackTokenizer_TokenType tokind = JackTokenizer_tokenType(thisObject->tokenizer);
    char type[50];

    if (tokind == JACK_TOKENIZER_TOKEN_TYPE_KEYWORD)
    {
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_INT) strcpy(type, "int");
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_CHAR) strcpy(type, "char");
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_BOOLEAN) strcpy(type, "boolean");
    }
    else
    {
        JackTokenizer_identifier(thisObject->tokenizer, type);
    }

    do {
        JackTokenizer_advance(thisObject->tokenizer);   // varName
        char varName[50];
        JackTokenizer_identifier(thisObject->tokenizer, varName);
        compile_varDec(varName, type, symkind, SCOPE_CLASS);
        JackTokenizer_advance(thisObject->tokenizer);   // ',' or ';'
    } while (! isSymbolToken(thisObject, ";"));

    JackTokenizer_advance(thisObject->tokenizer);
}

// ('constructor' | 'function' | 'method') ('void' | type) subroutineName '(' parameterList ')' subroutineBody
// subroutineBody: '{' varDec* statements '}'
void CompilationEngine_compileSubroutine(CompilationEngine thisObject, char *className)
{
    startSubroutine();

    JackTokenizer_Keyword kwKind;   // ('constructor' | 'function' | 'method')
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_CONSTRUCTION)) kwKind = JACK_TOKENIZER_KEYWORD_CONSTRUCTION;
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_FUNCTION)) kwKind = JACK_TOKENIZER_KEYWORD_FUNCTION;
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_METHOD)) kwKind = JACK_TOKENIZER_KEYWORD_METHOD;

    char type[50];
    JackTokenizer_advance(thisObject->tokenizer);    // ('void' | type)
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_VOID))
    {
        strcpy(type, "void");
    } 
    else
    {
        JackTokenizer_TokenType tokind = JackTokenizer_tokenType(thisObject->tokenizer);
        if (tokind == JACK_TOKENIZER_TOKEN_TYPE_KEYWORD)
        {
            if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_INT) strcpy(type, "int");
            if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_CHAR) strcpy(type, "char");
            if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_BOOLEAN) strcpy(type, "boolean");
        }
        else
        {
            JackTokenizer_identifier(thisObject->tokenizer, type);
        }
        
    }

    JackTokenizer_advance(thisObject->tokenizer);   // subroutineName
    char subroutineName[50];
    JackTokenizer_identifier(thisObject->tokenizer, subroutineName);

    JackTokenizer_advance(thisObject->tokenizer);   // '('

    if (kwKind == JACK_TOKENIZER_KEYWORD_METHOD)
    {
        int cnt = varCount(SCOPE_SUBROUTINE, SYMBOL_ARG);
        define("this", type, cnt, SYMBOL_ARG, SCOPE_SUBROUTINE);
    }
    
    JackTokenizer_advance(thisObject->tokenizer);
    CompilationEngine_compileParameterList(thisObject);
    // ')'

    JackTokenizer_advance(thisObject->tokenizer);   // '{'

    JackTokenizer_advance(thisObject->tokenizer);   // 'var' or statements or '}'
    int nLocals = 0;

    // varDec
    while (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_VAR)) {
        int varNum = CompilationEngine_compileVarDec(thisObject);
        nLocals += varNum;
    }

    char VMfunctionName[100];
    strcpy(VMfunctionName, className);
    strcat(VMfunctionName, ".");
    strcat(VMfunctionName, subroutineName);
    writeFunction(VMfunctionName, nLocals, thisObject->fpXml);
    if (kwKind == JACK_TOKENIZER_KEYWORD_CONSTRUCTION)
    {
        writePush(SEG_CONST, varCount(SCOPE_CLASS, SYMBOL_FIELD), thisObject->fpXml);

        writeCall("Memory.alloc", 1, thisObject->fpXml);
        writePop(SEG_POINTER, 0, thisObject->fpXml);
    }
    else if (kwKind == JACK_TOKENIZER_KEYWORD_METHOD)
    {
        writePush(SEG_ARG, 0, thisObject->fpXml);
        writePop(SEG_POINTER, 0, thisObject->fpXml);
    }
    else if (kwKind == JACK_TOKENIZER_KEYWORD_FUNCTION)
    {
    }

    // statements
    if (! isSymbolToken(thisObject, "}")) {   // statements or '}'
        CompilationEngine_compileStatements(thisObject, className);
    }
    // '}'
    JackTokenizer_advance(thisObject->tokenizer);
}

// ((type varName) (',' type varName)*)?
void CompilationEngine_compileParameterList(CompilationEngine thisObject)
{
    if (!isSymbolToken(thisObject, ")")) {  // type
        char type[50];
        JackTokenizer_identifier(thisObject->tokenizer, type);    // type

        JackTokenizer_advance(thisObject->tokenizer);   // varName
        char varName[50];
        JackTokenizer_identifier(thisObject->tokenizer, varName);
        compile_varDec(varName, type, SYMBOL_ARG, SCOPE_SUBROUTINE);

        JackTokenizer_advance(thisObject->tokenizer);   // ',' or not
        while (isSymbolToken(thisObject, ",")) {
            // ','

            JackTokenizer_advance(thisObject->tokenizer);   // type
            char type[50];
            JackTokenizer_identifier(thisObject->tokenizer, type);                   

            JackTokenizer_advance(thisObject->tokenizer);   // varName
            char varName[50];
            JackTokenizer_identifier(thisObject->tokenizer, varName);
            compile_varDec(varName, type, SYMBOL_ARG, SCOPE_SUBROUTINE);

            JackTokenizer_advance(thisObject->tokenizer);   // ',' or not
        }
    }

}

// 'var' type varName (',' varName)* ';'
int CompilationEngine_compileVarDec(CompilationEngine thisObject)
{
    int nArgs = 0;
    // 'var'

    JackTokenizer_advance(thisObject->tokenizer);   // type
    JackTokenizer_TokenType tokind = JackTokenizer_tokenType(thisObject->tokenizer);
    char type[50];
    if (tokind == JACK_TOKENIZER_TOKEN_TYPE_KEYWORD)
    {
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_INT) strcpy(type, "int");
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_CHAR) strcpy(type, "char");
        if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_BOOLEAN) strcpy(type, "boolean");
    }
    else
    {
        JackTokenizer_identifier(thisObject->tokenizer, type);
    }

    do {
        JackTokenizer_advance(thisObject->tokenizer);   //varName
        char varName[50];
        JackTokenizer_identifier(thisObject->tokenizer, varName);
        compile_varDec(varName, type, SYMBOL_VAR, SCOPE_SUBROUTINE);
        ++nArgs;
        JackTokenizer_advance(thisObject->tokenizer);   // ',' or ';'
    } while (! isSymbolToken(thisObject, ";"));

    JackTokenizer_advance(thisObject->tokenizer);
    return nArgs;
}

// statement*
// statement: letStatement | ifStatement | whileStatement | doStatement | returnStatement
void CompilationEngine_compileStatements(CompilationEngine thisObject, char *className)
{
    do {
        if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_LET)) {
            CompilationEngine_compileLet(thisObject, className);
        } else if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_IF)) {
            CompilationEngine_compileIf(thisObject, className);
        } else if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_WHILE)) {
            CompilationEngine_compileWhile(thisObject, className);
        } else if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_DO)) {
            CompilationEngine_compileDo(thisObject, className);
        } else if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_RETURN)) {
            CompilationEngine_compileReturn(thisObject, className);
        } else {
            break;
        }
    } while (true);

}

// subroutineCall: subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
void compileSubroutineCall(CompilationEngine thisObject, char *className, char *varName, bool fromTerm){
    if(!fromTerm) JackTokenizer_advance(thisObject->tokenizer); //// '(' or '.'

    //subroutineName '(' expressionList ')'
    if(isSymbolToken(thisObject, "(")){
        char subroutineName[50];
        strcpy(subroutineName, varName);
        JackTokenizer_advance(thisObject->tokenizer);   // expressionList
        writePush(SEG_POINTER, 0, thisObject->fpXml);
        int nArgs = CompilationEngine_compileExpressionList(thisObject, className);
        
        // ')'
        char VMfunctionName[100];
        strcpy(VMfunctionName, className);
        strcat(VMfunctionName, ".");
        strcat(VMfunctionName, subroutineName);
        writeCall(VMfunctionName, nArgs + 1, thisObject->fpXml);

        JackTokenizer_advance(thisObject->tokenizer);
    }
    else if(isSymbolToken(thisObject, ".")){    //(className | varName) '.' subroutineName '(' expressionList ')'
        SymbolKind kind = kindOf(varName);
        if(kind == SYMBOL_NONE){    //className '.' subroutineName '(' expressionList ')'

            char className[50];
            strcpy(className, varName);
            JackTokenizer_advance(thisObject->tokenizer);   // subroutineName
            char subroutineName[50];
            JackTokenizer_identifier(thisObject->tokenizer, subroutineName);

            JackTokenizer_advance(thisObject->tokenizer);   // '('

            JackTokenizer_advance(thisObject->tokenizer);   // expressionList
            int nArgs = CompilationEngine_compileExpressionList(thisObject, className);

            // ')'
            char VMfunctionName[100];

            strcpy(VMfunctionName, className);
            strcat(VMfunctionName, ".");
            strcat(VMfunctionName, subroutineName);
            writeCall(VMfunctionName, nArgs, thisObject->fpXml);
            JackTokenizer_advance(thisObject->tokenizer);
        }
        else{   //varName '.' subroutineName '(' expressionList ')'
            JackTokenizer_advance(thisObject->tokenizer);   // subroutineName
            char subroutineName[50];
            JackTokenizer_identifier(thisObject->tokenizer, subroutineName);

            JackTokenizer_advance(thisObject->tokenizer);   // '('
            if(kind == SYMBOL_STATIC){
                writePush(SEG_STATIC, indexOf(varName), thisObject->fpXml);
            }
            else if(kind == SYMBOL_FIELD){
                writePush(SEG_THIS, indexOf(varName), thisObject->fpXml);
            }
            else if(kind == SYMBOL_ARG){
                writePush(SEG_ARG, indexOf(varName), thisObject->fpXml);
            }
            else if(kind == SYMBOL_VAR){
                writePush(SEG_LOCAL, indexOf(varName), thisObject->fpXml);
            }

            JackTokenizer_advance(thisObject->tokenizer);   // expressionList
            int nArgs = CompilationEngine_compileExpressionList(thisObject, className); 

            // )
            char className[50];
            typeOf(varName, className);
            char VMfunctionName[100];
            strcpy(VMfunctionName, className);
            strcat(VMfunctionName, ".");
            strcat(VMfunctionName, subroutineName);
            writeCall(VMfunctionName, nArgs + 1, thisObject->fpXml);

            JackTokenizer_advance(thisObject->tokenizer);
        }
    }
}

// 'do' subroutineCall ';'
void CompilationEngine_compileDo(CompilationEngine thisObject, char *className)
{
    // 'do'
    char varName[50];
    JackTokenizer_advance(thisObject->tokenizer);
    JackTokenizer_identifier(thisObject->tokenizer, varName);

     // subroutineName | (className | varName)
    compileSubroutineCall(thisObject, className, varName, false);

    // ';'
    writePop(SEG_TEMP, 0, thisObject->fpXml);

    JackTokenizer_advance(thisObject->tokenizer);
}

// 'let' varName ('[' expression ']')? '=' expression ';'
void CompilationEngine_compileLet(CompilationEngine thisObject, char *className)
{
     // 'let'
    JackTokenizer_advance(thisObject->tokenizer);   // varName
    char varName[50];
    JackTokenizer_identifier(thisObject->tokenizer, varName);

    JackTokenizer_advance(thisObject->tokenizer);   // '[' or '='
    if (isSymbolToken(thisObject, "[")) {
        SymbolKind kind = kindOf(varName);
        if (kind == SYMBOL_STATIC)
        {
            writePush(SEG_STATIC, indexOf(varName), thisObject->fpXml); 
        } 
        else if (kind == SYMBOL_FIELD)
        {
            writePush(SEG_THIS, indexOf(varName), thisObject->fpXml);
        }
        else if (kind == SYMBOL_ARG)
        {
            writePush(SEG_ARG, indexOf(varName), thisObject->fpXml);
        }
        else if (kind == SYMBOL_VAR)
        {
            writePush(SEG_LOCAL, indexOf(varName), thisObject->fpXml);
        }
        
        JackTokenizer_advance(thisObject->tokenizer);
        // expression
        CompilationEngine_compileExpression(thisObject, className);

        writeArithmetic(COM_ADD, thisObject->fpXml);
        writePop(SEG_TEMP, 2, thisObject->fpXml);

        // ']'
        JackTokenizer_advance(thisObject->tokenizer);   // '='
        JackTokenizer_advance(thisObject->tokenizer);   // expression
        CompilationEngine_compileExpression(thisObject, className);    

        writePush(SEG_TEMP, 2, thisObject->fpXml);
        writePop(SEG_POINTER, 1, thisObject->fpXml);
        writePop(SEG_THAT, 0, thisObject->fpXml);

        // ';'
        JackTokenizer_advance(thisObject->tokenizer);
    }
    else
    {
        JackTokenizer_advance(thisObject->tokenizer);   //expression

        SymbolKind kind = kindOf(varName);
        CompilationEngine_compileExpression(thisObject, className); // ';'

        if(kind == SYMBOL_STATIC){
            writePop(SEG_STATIC, indexOf(varName), thisObject->fpXml);
        }
        else if(kind == SYMBOL_FIELD){
            writePop(SEG_THIS, indexOf(varName), thisObject->fpXml);
        }
        else if(kind == SYMBOL_ARG){
            writePop(SEG_ARG, indexOf(varName), thisObject->fpXml);
        }
        else if(kind == SYMBOL_VAR){
            writePop(SEG_LOCAL, indexOf(varName), thisObject->fpXml);
        }
        JackTokenizer_advance(thisObject->tokenizer);
    }


}

// 'while' '(' expression ')' '{' statements '}'
void CompilationEngine_compileWhile(CompilationEngine thisObject, char *className)
{
    char label1[20];
    char label2[20];
    getLabel(label1);
    getLabel(label2);

    // 'while'
    writeLabel(label1, thisObject->fpXml);

    JackTokenizer_advance(thisObject->tokenizer);   // '('

    JackTokenizer_advance(thisObject->tokenizer);
    CompilationEngine_compileExpression(thisObject, className);

    // ')'
    writeArithmetic(COM_NOT, thisObject->fpXml);
    writeIf(label2, thisObject->fpXml);     //与えられた条件式が偽の時
    JackTokenizer_advance(thisObject->tokenizer);   // '{'

    JackTokenizer_advance(thisObject->tokenizer);   // statements
    CompilationEngine_compileStatements(thisObject, className);

    // '}'
    writeGoto(label1, thisObject->fpXml);
    writeLabel(label2, thisObject->fpXml);

    JackTokenizer_advance(thisObject->tokenizer);
}

// 'return' expression? ';'
void CompilationEngine_compileReturn(CompilationEngine thisObject, char *className)
{
    // 'return'
    JackTokenizer_advance(thisObject->tokenizer);   // expression or ';'
    if (! isSymbolToken(thisObject, ";")) {
        CompilationEngine_compileExpression(thisObject, className);
    }
    else
    {
        writePush(SEG_CONST, 0, thisObject->fpXml);
    }
    writeReturn(thisObject->fpXml);
    // ';'
    JackTokenizer_advance(thisObject->tokenizer);
}

// 'if' '(' expression ')' '{' statements '}' ('else' '{' statements '}')?
void CompilationEngine_compileIf(CompilationEngine thisObject, char *className)
{

    // 'if'
    JackTokenizer_advance(thisObject->tokenizer);   // '('

    JackTokenizer_advance(thisObject->tokenizer);   // expression
    CompilationEngine_compileExpression(thisObject, className);

    // ')'
    char label1[20];
    char label2[20];
    getLabel(label1);
    getLabel(label2);
    writeArithmetic(COM_NOT, thisObject->fpXml);
    writeIf(label1, thisObject->fpXml); //与えられた条件式が偽の時

    JackTokenizer_advance(thisObject->tokenizer);   // '{'

    JackTokenizer_advance(thisObject->tokenizer);
    CompilationEngine_compileStatements(thisObject, className);

    // '}'
    JackTokenizer_advance(thisObject->tokenizer);   // 'else' or not
    writeGoto(label2, thisObject->fpXml);
    writeLabel(label1, thisObject->fpXml);
    if (isKeywordToken(thisObject, JACK_TOKENIZER_KEYWORD_ELSE)) {
        // 'else'
        JackTokenizer_advance(thisObject->tokenizer);   // '{'

        JackTokenizer_advance(thisObject->tokenizer);
        CompilationEngine_compileStatements(thisObject, className);

        // '}'
        JackTokenizer_advance(thisObject->tokenizer);
    }

    writeLabel(label2, thisObject->fpXml);
}

// term (op term)*
// op: '+' | '-' | '*' | '/' | '&' | '|' | '<' | '>' | '='
void CompilationEngine_compileExpression(CompilationEngine thisObject, char *className)
{
    CompilationEngine_compileTerm(thisObject, className);

    while (inSymbolListToken(thisObject, "+", "-", "*",  "/", "&", "|", "<", ">", "=", NULL)) {
        // op
        char sym[5];
        JackTokenizer_symbol(thisObject->tokenizer, sym);
        JackTokenizer_advance(thisObject->tokenizer);   // term
        CompilationEngine_compileTerm(thisObject, className);
        if(strcmp(sym, "+") == 0) writeArithmetic(COM_ADD, thisObject->fpXml);
        else if(strcmp(sym, "-") == 0) writeArithmetic(COM_SUB, thisObject->fpXml);
        else if(strcmp(sym, "*") == 0) writeCall("Math.multiply", 2, thisObject->fpXml);
        else if(strcmp(sym, "/") == 0) writeCall("Math.divide", 2, thisObject->fpXml);
        else if(strcmp(sym, "&") == 0) writeArithmetic(COM_AND, thisObject->fpXml);
        else if(strcmp(sym, "|") == 0) writeArithmetic(COM_OR, thisObject->fpXml);
        else if(strcmp(sym, "<") == 0) writeArithmetic(COM_LT, thisObject->fpXml);
        else if(strcmp(sym, ">") == 0) writeArithmetic(COM_GT, thisObject->fpXml);
        else if(strcmp(sym, "=") == 0) writeArithmetic(COM_EQ, thisObject->fpXml);        
    }

}

// integerConstant | stringConstant | keywordConstant | varName | varName '[' expression ']' | subroutineCall | '(' expression ')' | unaryOp term
// subroutineCall: subroutineName '(' expressionList ')' | (className | varName) '.' subroutineName '(' expressionList ')'
// unaryOp: '-' | '~'
void CompilationEngine_compileTerm(CompilationEngine thisObject, char *className)
{
    if (
        JackTokenizer_tokenType(thisObject->tokenizer) == JACK_TOKENIZER_TOKEN_TYPE_INT_CONST ||
        JackTokenizer_tokenType(thisObject->tokenizer) == JACK_TOKENIZER_TOKEN_TYPE_STRING_CONST ||
        JackTokenizer_tokenType(thisObject->tokenizer) == JACK_TOKENIZER_TOKEN_TYPE_KEYWORD
    ) { // integerConstant | stringConstant | keywordConstant
        if (JackTokenizer_tokenType(thisObject->tokenizer) == JACK_TOKENIZER_TOKEN_TYPE_INT_CONST)
        {
            int intval;
            JackTokenizer_intVal(thisObject->tokenizer, &intval);
            writePush(SEG_CONST, intval, thisObject->fpXml);
        }
        else if (JackTokenizer_tokenType(thisObject->tokenizer) == JACK_TOKENIZER_TOKEN_TYPE_STRING_CONST)
        {
            char str[100];
            JackTokenizer_stringVal(thisObject->tokenizer, str);
            int len = strlen(str);
            writePush(SEG_CONST, len, thisObject->fpXml);
            writeCall("String.new", 1, thisObject->fpXml);
            for (int i = 0; i < len; i++)
            {
                writePush(SEG_CONST, (int)str[i], thisObject->fpXml);
                writeCall("String.appendChar", 2, thisObject->fpXml);
            }
        }
        else
        {
            if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_TRUE)
            {
                writePush(SEG_CONST, 1, thisObject->fpXml);
                writeArithmetic(COM_NEG, thisObject->fpXml);
            }
            else if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_FALSE || JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_NULL)
            {
                writePush(SEG_CONST, 0, thisObject->fpXml);
            }
            else if (JackTokenizer_keyword(thisObject->tokenizer) == JACK_TOKENIZER_KEYWORD_THIS)
            {
                writePush(SEG_POINTER, 0, thisObject->fpXml);   //セットされたオブジェクトのアドレス値
            }
            
        }
        JackTokenizer_advance(thisObject->tokenizer);
    } else if (isSymbolToken(thisObject, "(")) {    // '(' expression ')'
        // '('
        JackTokenizer_advance(thisObject->tokenizer);   // expression
        CompilationEngine_compileExpression(thisObject, className); 
    
        // ')'
        JackTokenizer_advance(thisObject->tokenizer);
    } else if (inSymbolListToken(thisObject, "-", "~", NULL)) { // unaryOp term
        // unaryOp
        if (isSymbolToken(thisObject, "-"))
        {
            JackTokenizer_advance(thisObject->tokenizer);
            CompilationEngine_compileTerm(thisObject, className);
            writeArithmetic(COM_NEG, thisObject->fpXml);    // 符号反転
        }
        else
        {
            JackTokenizer_advance(thisObject->tokenizer);
            CompilationEngine_compileTerm(thisObject, className);   
            writeArithmetic(COM_NOT, thisObject->fpXml);    // ブーリアン否定      
        }
        
    } else {    // varName | varName '[' expression ']' | subroutineCall
        // varName | subroutineName | className
        char varName[50];
        JackTokenizer_identifier(thisObject->tokenizer, varName);
        char token[50];

        JackTokenizer_advance(thisObject->tokenizer);   // '[' or '(' or '.' or not
        if (isSymbolToken(thisObject, "[")) {
            // '['
            JackTokenizer_advance(thisObject->tokenizer);   // expression
            
            SymbolKind kind = kindOf(varName);
            if (kind == SYMBOL_STATIC)
            {
                writePush(SEG_STATIC, indexOf(varName), thisObject->fpXml);
            }
            else if (kind == SYMBOL_FIELD)
            {
                writePush(SEG_THIS, indexOf(varName), thisObject->fpXml);
            }
            else if (kind == SYMBOL_ARG)
            {
                writePush(SEG_ARG, indexOf(varName), thisObject->fpXml);
            }
            else if (kind == SYMBOL_VAR)
            {
                writePush(SEG_LOCAL, indexOf(varName), thisObject->fpXml);
            }
    
            CompilationEngine_compileExpression(thisObject, className);

             // ']'
            writeArithmetic(COM_ADD, thisObject->fpXml);
            writePop(SEG_POINTER, 1, thisObject->fpXml);
            writePush(SEG_THAT, 0, thisObject->fpXml);
           
            JackTokenizer_advance(thisObject->tokenizer);
        } else if (inSymbolListToken(thisObject, "(", ".", NULL)) {
            // '(' or '.'
            compileSubroutineCall(thisObject, className, varName, true);
        }
        else {
            // varName
            compileWrite_varName(varName, thisObject);
        }
        
    }

}

// (expression (',' expression)*)?
int CompilationEngine_compileExpressionList(CompilationEngine thisObject, char *className)
{
    int nArgs = 0;
    if (! isSymbolToken(thisObject, ")")) { // expression is not ')'
        CompilationEngine_compileExpression(thisObject, className);
        ++nArgs;

        while (isSymbolToken(thisObject, ",")) {
            // ','
            JackTokenizer_advance(thisObject->tokenizer);   // expression
            CompilationEngine_compileExpression(thisObject, className);
            ++nArgs;
        }
    }

    return nArgs;
}

void writeToken(FILE *fp, JackTokenizer tokenizer)
{
    switch (JackTokenizer_tokenType(tokenizer))
    {
    case JACK_TOKENIZER_TOKEN_TYPE_KEYWORD:
        writeKeyword(fp, tokenizer);
        break;
    case JACK_TOKENIZER_TOKEN_TYPE_SYMBOL:
        writeSymbol(fp, tokenizer);
        break;
    case JACK_TOKENIZER_TOKEN_TYPE_IDENTIFIER:
        writeIdentifier(fp, tokenizer);
        break;
    case JACK_TOKENIZER_TOKEN_TYPE_INT_CONST:
        writeIntegerConstant(fp, tokenizer);
        break;
    case JACK_TOKENIZER_TOKEN_TYPE_STRING_CONST:
        writeStringConstant(fp, tokenizer);
        break; 
    default:
        break;
    }
}

void writeKeyword(FILE *fp, JackTokenizer tokenizer)
{
    struct keyword {
        JackTokenizer_Keyword id;
        char *string;
    };
    struct keyword keywords[] = {
        { JACK_TOKENIZER_KEYWORD_CLASS,        "class" },
        { JACK_TOKENIZER_KEYWORD_METHOD,       "method" },
        { JACK_TOKENIZER_KEYWORD_FUNCTION,     "function" },
        { JACK_TOKENIZER_KEYWORD_CONSTRUCTION, "constructor" },
        { JACK_TOKENIZER_KEYWORD_INT,          "int" },
        { JACK_TOKENIZER_KEYWORD_BOOLEAN,      "boolean" },
        { JACK_TOKENIZER_KEYWORD_CHAR,         "char" },
        { JACK_TOKENIZER_KEYWORD_VOID,         "void" },
        { JACK_TOKENIZER_KEYWORD_VAR,          "var" },
        { JACK_TOKENIZER_KEYWORD_STATIC,       "static" },
        { JACK_TOKENIZER_KEYWORD_FIELD,        "field" },
        { JACK_TOKENIZER_KEYWORD_LET,          "let" },
        { JACK_TOKENIZER_KEYWORD_DO,           "do" },
        { JACK_TOKENIZER_KEYWORD_IF,           "if" },
        { JACK_TOKENIZER_KEYWORD_ELSE,         "else" },
        { JACK_TOKENIZER_KEYWORD_WHILE,        "while" },
        { JACK_TOKENIZER_KEYWORD_RETURN,       "return" },
        { JACK_TOKENIZER_KEYWORD_TRUE,         "true" },
        { JACK_TOKENIZER_KEYWORD_FALSE,        "false" },
        { JACK_TOKENIZER_KEYWORD_NULL,         "null" },
        { JACK_TOKENIZER_KEYWORD_THIS,         "this" },
    };
    JackTokenizer_Keyword id = JackTokenizer_keyword(tokenizer);

    fprintf(fp, "<keyword> ");
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (id == keywords[i].id) {
            fprintf(fp, "%s", keywords[i].string);
            break;
        }
    }
    fprintf(fp, " </keyword>\n");
}

void writeSymbol(FILE *fp, JackTokenizer tokenizer)
{
    char token[JACK_TOKEN_SIZE];
    JackTokenizer_symbol(tokenizer, token);

    fprintf(fp, "<symbol> ");
    if (strcmp(token, "<") == 0) {
        fprintf(fp, "&lt;");
    } else if (strcmp(token, ">") == 0) {
        fprintf(fp, "&gt;");
    } else if (strcmp(token, "&") == 0) {
        fprintf(fp, "&amp;");
    } else {
        fprintf(fp, "%s", token);
    }
    fprintf(fp, " </symbol>\n");
}

void writeIdentifier(FILE *fp, JackTokenizer tokenizer)
{
    char token[JACK_TOKEN_SIZE];
    JackTokenizer_identifier(tokenizer, token);
    fprintf(fp, "<identifier> %s </identifier>\n", token);
}

void writeIntegerConstant(FILE *fp, JackTokenizer tokenizer)
{
    int intVal;
    JackTokenizer_intVal(tokenizer, &intVal);
    fprintf(fp, "<integerConstant> %d </integerConstant>\n", intVal);
}

void writeStringConstant(FILE *fp, JackTokenizer tokenizer)
{
    char token[JACK_TOKEN_SIZE];
    JackTokenizer_stringVal(tokenizer, token);
    fprintf(fp, "<stringConstant> %s </stringConstant>\n", token);
}

void advanceAndWriteToken(CompilationEngine thisObject)
{
    JackTokenizer_advance(thisObject->tokenizer);
    writeToken(thisObject->fpXml ,thisObject->tokenizer);
}

bool isKeywordToken(CompilationEngine thisObject, JackTokenizer_Keyword keyword)
{
    if (JackTokenizer_tokenType(thisObject->tokenizer) != JACK_TOKENIZER_TOKEN_TYPE_KEYWORD) {
        return false;
    }

    if (JackTokenizer_keyword(thisObject->tokenizer) != keyword) {
        return false;
    }

    return true;
}

bool isSymbolToken(CompilationEngine thisObject, char *symbol)
{
    char token[JACK_TOKEN_SIZE];

    if (JackTokenizer_tokenType(thisObject->tokenizer) != JACK_TOKENIZER_TOKEN_TYPE_SYMBOL) {
        return false;
    }

    JackTokenizer_symbol(thisObject->tokenizer, token);
    if (strcmp(token, symbol) != 0) {
        return false;
    }

    return true;
}

bool inSymbolListToken(CompilationEngine thisObject, ...)
{
    char token[JACK_TOKEN_SIZE];
    char* symbol;
    bool found = false;

    if (JackTokenizer_tokenType(thisObject->tokenizer) != JACK_TOKENIZER_TOKEN_TYPE_SYMBOL) {
        return found;
    }

    JackTokenizer_symbol(thisObject->tokenizer, token);

    va_list args;
    va_start(args, thisObject);

    while ((symbol = va_arg(args, char*)) != NULL) {
        if (strcmp(token, symbol) == 0) {
            found = true;
            break;
        }
    }

    va_end(args);

    return found;
}

void compileWrite_varName(char *varName, CompilationEngine thisObject){
    SymbolKind kind = kindOf(varName);
    if(kind == SYMBOL_STATIC){
        writePush(SEG_STATIC, indexOf(varName), thisObject->fpXml);
    }
    else if(kind == SYMBOL_FIELD){
        writePush(SEG_THIS, indexOf(varName), thisObject->fpXml);
    }
    else if(kind == SYMBOL_ARG){
        writePush(SEG_ARG, indexOf(varName), thisObject->fpXml);
    }
    else if(kind == SYMBOL_VAR){
        writePush(SEG_LOCAL, indexOf(varName), thisObject->fpXml);
    }
    else if (strcmp(varName, "this") == 0)
    {
        writePush(SEG_POINTER, 0, thisObject->fpXml);
    }
    
}

void compile_varDec(char *varName, char *type, SymbolKind symkind, ScopeKind scpkind){
    int cnt = varCount(scpkind, symkind);
    define(varName, type, cnt, symkind, scpkind);
}
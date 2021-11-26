#include "JackTokenizer.h"
#include "SymbolHashTable.h"

typedef struct CompilationEngine CompilationEngine;

struct CompilationEngine{
    JackTokenizer *tokenizer;
    FILE *fp_xml;
};

CompilationEngine *compilationEngineInit(FILE *fp_jack, FILE *fp_xml);
void advanceWrite(CompilationEngine *comp);
bool isSymbolToken(CompilationEngine *comp, char *token);
bool isKeyword(CompilationEngine *comp, KeywordKind kind);
void compileClass(CompilationEngine *comp);
void compileClassVarDec(CompilationEngine *comp);
void compileSubroutineDec(CompilationEngine *comp, char *className);
int compileSubroutineBody(CompilationEngine *comp, char *className, char *subroutineName, KeywordKind kwKind);
void compileParameterList(CompilationEngine *comp);
int compileVarDec(CompilationEngine *comp);
void compileSubroutineCall(CompilationEngine *comp, char *className, char *varName, bool fromTerm);
void compileStatements(CompilationEngine *comp, char *className);
void compileDo(CompilationEngine *comp, char *className);
void compileLet(CompilationEngine *comp, char *className);
void compileWhile(CompilationEngine *comp, char *className);
void compileReturn(CompilationEngine *comp, char *className);
void compileIf(CompilationEngine *comp, char *className);
void compileExpression(CompilationEngine *comp, char *className);
void compileTerm(CompilationEngine *comp, char *className);
int compileExpressionList(CompilationEngine *comp, char *className);
void compileWrite_varName(char *varName, CompilationEngine *comp);
void compile_varDec(char *varName, char *type, SymbolKind symkind, ScopeKind scpkind);
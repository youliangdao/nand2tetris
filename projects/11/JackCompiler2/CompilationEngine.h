#ifndef _COMPILATION_ENGINE_H_INCLUDE_
#define _COMPILATION_ENGINE_H_INCLUDE_

#include <stdio.h>
#include <stdbool.h>
#include "SymbolHashTable.h"

typedef struct compilation_engine * CompilationEngine;

CompilationEngine CompilationEngine_init(FILE *fpJack, FILE *fpXml);
void CompilationEngine_compileClass(CompilationEngine thisObject);
void CompilationEngine_compileClassVarDec(CompilationEngine thisObject);
void CompilationEngine_compileSubroutine(CompilationEngine thisObject, char *className);
void CompilationEngine_compileParameterList(CompilationEngine thisObject);
int CompilationEngine_compileVarDec(CompilationEngine thisObject);
void CompilationEngine_compileStatements(CompilationEngine thisObject, char *className);
void CompilationEngine_compileDo(CompilationEngine thisObject, char *className);
void CompilationEngine_compileLet(CompilationEngine thisObject, char *className);
void CompilationEngine_compileWhile(CompilationEngine thisObject, char *className);
void CompilationEngine_compileReturn(CompilationEngine thisObject, char *className);
void CompilationEngine_compileIf(CompilationEngine thisObject, char *className);
void CompilationEngine_compileExpression(CompilationEngine thisObject, char *className);
void CompilationEngine_compileTerm(CompilationEngine thisObject, char *className);
int CompilationEngine_compileExpressionList(CompilationEngine thisObject, char *className);
void compile_varDec(char *varName, char *type, SymbolKind symkind, ScopeKind scpkind);
void compileWrite_varName(char *varName, CompilationEngine thisObject);
void compileSubroutineCall(CompilationEngine thisObject, char *className, char *varName, bool fromTerm);

#endif
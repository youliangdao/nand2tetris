#ifndef _COMPILATION_ENGINE_H_INCLUDE_
#define _COMPILATION_ENGINE_H_INCLUDE_

#include <stdio.h>
#include "SymbolHashTable.h"

typedef struct compilation_engine * CompilationEngine;

CompilationEngine CompilationEngine_init(FILE *fpJack, FILE *fpXml);
void CompilationEngine_compileClass(CompilationEngine thisObject);
void CompilationEngine_compileClassVarDec(CompilationEngine thisObject);
void CompilationEngine_compileSubroutine(CompilationEngine thisObject);
void CompilationEngine_compileParameterList(CompilationEngine thisObject);
int CompilationEngine_compileVarDec(CompilationEngine thisObject);
void CompilationEngine_compileStatements(CompilationEngine thisObject);
void CompilationEngine_compileDo(CompilationEngine thisObject);
void CompilationEngine_compileLet(CompilationEngine thisObject);
void CompilationEngine_compileWhile(CompilationEngine thisObject);
void CompilationEngine_compileReturn(CompilationEngine thisObject);
void CompilationEngine_compileIf(CompilationEngine thisObject);
void CompilationEngine_compileExpression(CompilationEngine thisObject);
void CompilationEngine_compileTerm(CompilationEngine thisObject);
void CompilationEngine_compileExpressionList(CompilationEngine thisObject);
void compile_varDec(char *varName, char *type, SymbolKind symkind, ScopeKind scpkind);

#endif
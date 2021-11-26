#include<stdlib.h>
#include<stdbool.h>

#define HASHTABLE_SIZE 10000

typedef enum{
    SYMBOL_STATIC,
    SYMBOL_FIELD,
    SYMBOL_ARG,
    SYMBOL_VAR,
    SYMBOL_NONE,
} SymbolKind;

typedef enum{
    SCOPE_CLASS,
    SCOPE_SUBROUTINE,
} ScopeKind;

typedef struct HashTable HashTable;

struct HashTable{
    char *identifier;
    SymbolKind kind;
    char *type;
    int index;
    HashTable *next;
};

HashTable *classHashTable[HASHTABLE_SIZE];
HashTable **cHT_ptr = classHashTable;
HashTable *subroutineHashTable[HASHTABLE_SIZE];
HashTable **sHT_ptr = subroutineHashTable;

unsigned getHashVal(char *identifier);
HashTable *find(char *identifier, ScopeKind kind);
HashTable *define(char *identifier, char *type, int index, SymbolKind symkind, ScopeKind scpkind);
int varCount(ScopeKind scpkind, SymbolKind symkind);
SymbolKind kindOf(char *identifier);
bool typeOf(char *identifier, char *type);
int indexOf(char *identifier);
void initClassHashTable();
void initSubroutineHashTable();
void constructHashTable();
void startSubroutine();
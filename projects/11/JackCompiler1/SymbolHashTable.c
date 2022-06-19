

#include "SymbolHashTable.h"

unsigned getHashVal(char *identifier){
    unsigned val = 0;
    for(; *identifier != '\0'; ++identifier)  val = *identifier + 31 * val;
    return val % HASHTABLE_SIZE; 
}

HashTable *find(char *identifier, ScopeKind kind){
    HashTable *p;
    unsigned hashVal = getHashVal(identifier);
    if(kind == SCOPE_CLASS){
        p = classHashTable[hashVal];
    }
    else{
        p = subroutineHashTable[hashVal];
    }
    for(; p != NULL; p = p->next){
        if(strcmp(identifier, p->identifier) == 0) return p;
    }
    return NULL;
}

HashTable *define(char *identifier, char *type, int index, SymbolKind symkind, ScopeKind scpkind){
    HashTable *p = find(identifier, scpkind);
    unsigned hashVal;
    if((p == NULL)){
        p = (HashTable *)malloc(sizeof(HashTable));

        p->identifier = strdup(identifier);
        p->type = strdup(type);
        p->index = index;
        p->kind = symkind;

        hashVal = getHashVal(identifier);
        if(scpkind == SCOPE_CLASS){
            p->next = classHashTable[hashVal];
            classHashTable[hashVal] = p;
        }
        else{
            p->next = subroutineHashTable[hashVal];
            subroutineHashTable[hashVal] = p;
        }
    }

    return p;
}

int varCount(ScopeKind scpkind, SymbolKind symkind){
    int rep = 0;
    HashTable **ptr;
    if(scpkind == SCOPE_CLASS) ptr = classHashTable;
    else ptr = subroutineHashTable;

    for(int i=0;i<HASHTABLE_SIZE;++i){
        if(ptr[i] != NULL && ptr[i]->kind == symkind) ++rep;
    }

    return rep;
}

SymbolKind kindOf(char *identifier){
    HashTable *p = find(identifier, SCOPE_CLASS);

    if(p != NULL) return p->kind;
    else{
        p = find(identifier, SCOPE_SUBROUTINE);
        if(p != NULL) return p->kind;
        else return SYMBOL_NONE;
    }
}

bool typeOf(char *identifier, char *type){
    HashTable *p = find(identifier, SCOPE_CLASS);
    if(p != NULL){
        strcpy(type, p->type);
        return true;
    }
    else{
        p = find(identifier, SCOPE_SUBROUTINE);
        if(p != NULL){
            strcpy(type, p->type);
            return true;
        }
        else return false;
    }
}

int indexOf(char *identifier){
    HashTable *p = find(identifier, SCOPE_CLASS);
    if(p != NULL) return p->index;
    else{
        p = find(identifier, SCOPE_SUBROUTINE);
        if(p != NULL){
            return p->index;
        }
    }
}

void initClassHashTable(){
    for(int i=0;i<HASHTABLE_SIZE;++i){
        if(classHashTable[i] != NULL){
            classHashTable[i]->kind = SYMBOL_NONE;
            classHashTable[i]->index = 0;
            if(classHashTable[i]->identifier != NULL) free(classHashTable[i]->identifier);
            if(classHashTable[i]->type != NULL) free(classHashTable[i]->type);
            classHashTable[i] = NULL;
        }
    }
}

void initSubroutineHashTable(){
    for(int i=0;i<HASHTABLE_SIZE;++i){
        if(subroutineHashTable[i] != NULL){
            subroutineHashTable[i]->kind = SYMBOL_NONE;
            subroutineHashTable[i]->index = 0;
            if(subroutineHashTable[i]->identifier != NULL) free(subroutineHashTable[i]->identifier);
            if(subroutineHashTable[i]->type != NULL) free(subroutineHashTable[i]->type);
            subroutineHashTable[i] = NULL;
        }
    }
}

void constructHashTable(){
    initClassHashTable();
    initSubroutineHashTable();
}

void startSubroutine(){
    initSubroutineHashTable();
}
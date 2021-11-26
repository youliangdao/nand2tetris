#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "hashtable.h"


/**
	Creates the HashTable structure.
	@param ht: the hastable pointer
	@param dim: the wanted dimension
*/
void init_hash_table(struct HashTable* ht, size_t dim){
	ht->size = dim;
	ht->buckets = (struct ListNode**)malloc(sizeof(struct ListNode) * dim);
	memset(ht->buckets, 0, dim*sizeof(struct ListNode*));

	for(int i = 0; i < 5; i++){
		ht->count[i] = 0;
	}
}


/**
	Creates the Symbol structure which holds the actual information.
	@param name: the symbol name
	@param type: the symbol type
	@param kind: the symbol kind
	@param index: the symbol index
	@return the Symbol structure
*/
Symbol* create_symbol(char* name, char* type, enum kind_t kind, uint16_t index){
	Symbol* temp = (Symbol*)malloc(sizeof(Symbol));
	temp->name = (char*)malloc(strlen(name)+1);
	strcpy(temp->name, name);
	temp->type = (char*)malloc(strlen(type)+1);
	strcpy(temp->type, type);
	temp->kind = kind;
	temp->index = index;
	return temp;
}


/**
	Simple hashing function for the name. (can be improved)
	@param key: the label name
	@param size: size of the table
	@return the hash
*/
uint16_t hash(char* key, size_t size){
	uint16_t h = 0;
	size_t len = strlen(key);
	for(size_t i=0; i<len; i++){
		h += key[i];
	}

	return h % size;
}


/**
	Creates a list node.
	@param name: the symbol name
	@param type: the symbol type
	@param kind: the symbol kind
	@param index: the symbol index
	@return the ListNode structure
*/
struct ListNode* create_node(char* name, char* type, enum kind_t kind, uint16_t index){
	Symbol *sym = create_symbol(name, type, kind, index);
	struct ListNode *node = (struct ListNode*)malloc(sizeof(struct ListNode));
	node->info = sym;
	node->next = NULL;
	return node;
}


/**
	Inserts a new node into the list.
	@param name: the symbol name
	@param type: the symbol type
	@param kind: the symbol kind
	@param index: the symbol index
	@return the new pointer to the list
*/
struct ListNode* insert_node(struct ListNode* list, char* name, char* type, enum kind_t kind, uint16_t index){
	struct ListNode *node = create_node(name, type, kind, index);

	struct ListNode *temp = list;

	if(!list){
		node->next = node;
	}
	else{
		node->next = list;
		while(temp->next != list){
			temp = temp->next;
		}
		temp->next = node;
	}

	return node;
}

/**
	Returns the number of variables of a certain kind from the hash table.
	@param ht: the hashtable structure
	@param name: the kind
	@return number of variables
*/
uint16_t get_var_count(struct HashTable ht, enum kind_t kind){
	return ht.count[kind];
}

/**
	Inserts a new element into the HashTable struct. 
	It uses chaining for collision avoidance (using a circular linked list).
	@param ht: the hashtable structure
	@param name: the label name
	@param addr: the label address
*/
void insert_HT(struct HashTable *ht, char* name, char* type, enum kind_t kind){
	uint16_t position = hash(name, ht->size);
	struct ListNode *list = ht->buckets[position];
	uint16_t ind = get_var_count(*ht, kind);
	(*ht).count[kind] += 1;

	list = insert_node(list, name, type, kind, ind);

	ht->buckets[position] = list; 
}


/*
	Prints all elements in a list.
*/
void print_list(struct ListNode *ls){
	struct ListNode *temp = ls;
	do{
		printf("Name: %s, type: %s, kind: %d, index: %d\n", temp->info->name, temp->info->type, temp->info->kind, temp->info->index);
		temp = temp->next;
	}while(temp != ls);
}


/*
	Prints all elements in a table.
*/
void print_HT(struct HashTable ht){
	if(ht.size > 0){
		for(uint16_t i = 0; i < ht.size; i++){
			if(ht.buckets[i]){
				print_list(ht.buckets[i]);
				printf("\n");
			}
		}
	}
}



/**
	Checks if an item is present in the table.
	@param ht: the hashtable structure
	@param name: the label name
	@return true if it is present, else false
*/
bool is_item(struct HashTable ht, char* name){
	uint16_t position = hash(name, ht.size);
	struct ListNode *list = ht.buckets[position];

	if(!list){
		return false;
	}
	else{
		struct ListNode *temp = list;
		do{
			if(strcmp(temp->info->name, name) == 0){
				return true;
			}
			temp = temp->next;
		}while(temp != list);

	}
	return false;
}


/**
	Returns the kind of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return kind of the symbol, NONE if it does not exist
*/
enum kind_t get_kind(struct HashTable ht, char* name){
	enum kind_t kind = NONE_K;
	uint16_t position = hash(name, ht.size);
	struct ListNode *list = ht.buckets[position];

	if(!list){
		return NONE_K;
	}
	else{
		struct ListNode *temp = list;
		do{
			if(strcmp(temp->info->name, name) == 0){
				kind = temp->info->kind;
				break;
			}
			temp = temp->next;
		}while(temp != list);

	}

	return kind;
}

/**
	Returns the index of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return index of the symbol, -1 if it does not exist
*/
int32_t get_index(struct HashTable ht, char* name){
	int32_t index = -1;
	uint16_t position = hash(name, ht.size);
	struct ListNode *list = ht.buckets[position];

	if(!list){
		return -1;
	}
	else{
		struct ListNode *temp = list;
		do{
			if(strcmp(temp->info->name, name) == 0){
				index = temp->info->index;
				break;
			}
			temp = temp->next;
		}while(temp != list);

	}

	return index;
}


/**
	Returns the type of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return type of the symbol, NULL if it does not exist
*/
char* get_type(struct HashTable ht, char* name){
	char *type = NULL;
	uint16_t position = hash(name, ht.size);
	struct ListNode *list = ht.buckets[position];

	if(!list){
		return type;
	}
	else{
		struct ListNode *temp = list;
		do{
			if(strcmp(temp->info->name, name) == 0){
				type = temp->info->type;
				break;
			}
			temp = temp->next;
		}while(temp != list);

	}

	return type;
}

void clear_symbol(struct ListNode *ln){
	if(ln->info->name != NULL){
		free(ln->info->name);
		free(ln->info->type);
		free(ln->info);
	}
}


/**
	Clears all entries from a hashtable;
	@param ht: the hashtable structure
*/
void clear_ht(struct HashTable *ht){

	if(ht->size > 0){
		for(int i = 0; i < ht->size; i++){
			if(ht->buckets[i]){
				struct ListNode *temp = ht->buckets[i];
				struct ListNode *temp2 = temp;
				do{
					clear_symbol(temp);
					temp = temp->next;
				}while(temp != ht->buckets[i]);
				free(temp2);
				ht->buckets[i] = NULL;
			}
		}
	}

	for(int i = 0; i < 5; i++){
		ht->count[i] = 0;
	}
}
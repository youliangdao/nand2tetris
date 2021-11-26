/*
	The header file for the hash table implementation.
*/

#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>


enum kind_t {
	NONE_K,
	STATIC_K,
	FIELD_K,
	ARG_K,
	VAR_K
};



struct HashTable{
	struct ListNode** buckets;
	size_t size;
	uint16_t count[5];
};

typedef struct Symbol{
	char* name;
	char* type;
	enum kind_t kind;
	uint16_t index;
}Symbol;

struct ListNode{
	Symbol* info;
	struct ListNode* next;
};


/**
	Creates the HashTable structure.
	@param ht: the hastable pointer
	@param dim: the wanted dimension
*/
void init_hash_table(struct HashTable* ht, size_t dim);

/**
	Inserts a new element into the HashTable struct. 
	It uses chaining for collision avoidance (using a circular linked list).
	@param ht: the hashtable structure
	@param name: the label name
	@param addr: the label address
*/
void insert_HT(struct HashTable *ht, char* name, char* type, enum kind_t kind);

/*
	Prints all elements in a list.
*/
void print_HT(struct HashTable ht);

/**
	Checks if an item is present in the table.
	@param ht: the hashtable structure
	@param name: the label name
	@return true if it is present, else false
*/
bool is_item(struct HashTable ht, char* name);


/**
	Returns the kind of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return kind of the symbol, NONE if it does not exist
*/
enum kind_t get_kind(struct HashTable ht, char* name);

/**
	Returns the index of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return index of the symbol, -1 if it does not exist
*/
int32_t get_index(struct HashTable ht, char* name);

/**
	Returns the type of the variable from the Hash table.
	@param ht: the hashtable structure
	@param name: the symbol name
	@return type of the symbol, NULL if it does not exist
*/
char* get_type(struct HashTable ht, char* name);

/**
	Returns the number of variables of a certain kind from the hash table.
	@param ht: the hashtable structure
	@param name: the kind
	@return number of variables
*/
uint16_t get_var_count(struct HashTable ht, enum kind_t kind);

/**
	Clears all entries from a hashtable;
	@param ht: the hashtable structure
*/
void clear_ht(struct HashTable *ht);
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include "tokenizer.h"

/**
	Text processing functions, used for tokenizing the input file.
*/

char keywords[21][12] = {
						"class",
						"constructor",
						"function", 
						"method",
						"field",
						"static",
						"var",
						"int",
						"char",
						"boolean",
						"void",
						"true",
						"false",
						"null",
						"this",
						"let",
						"do",
						"if",
						"else",
						"while",
						"return"
					};

char symbols[20] = "{}()[].,;+-*/&|<>=~";


/**
	Checks if a string is a keyword.
	@param word: string to check
	@return true if it is a keyword, else false
*/
bool is_keyword(char* word){
	bool res = false;

	for(int i = 0; i < 21; i++){
		if(strcmp(word, keywords[i]) == 0){
			res = true;
			break;
		}
	}
	return res;
}


/**
	Checks if a string is a symbol.
	@param word: string to check
	@return true if it is a symbol, else false
*/
bool is_symbol(char c){
	bool res = false;

	for(int i = 0; i < 21; i++){
		if(c == symbols[i]){
			res = true;
			break;
		}
	}
	return res;
}


/**
	Adds a string to a given array of strings.
	@param word: string to add
	@param tokens: array of strings
	@param counter: current number of elements in the array
*/
void add_to_list(char* word, char ***tokens, uint32_t counter){
	char *to_add = (char*)malloc(strlen(word) + 1);
	strcpy(to_add, word);

	*tokens = (char**)realloc(*tokens, sizeof(char*) * (counter+1));
	if(*tokens == NULL){
		printf("Error in realloc.\n");
		exit(1);
	}
	(*tokens)[counter] = to_add;
}

/**
	Takes a line and splits it into Jack language tokens.
	@param line: line to split
	@param tokens: array of tokens
	@param start: true if it is the beginning of a file, else false
	@return total number of tokens
*/
uint32_t tokenize(char *line, char ***tokens, bool start){
	static bool comment = false;
	static uint32_t counter = 0;
	bool quote = false;
	uint8_t pos = 0;
	uint32_t len = strlen(line);
	char word[64] = {0};

	if(start){
		counter = 0;
		comment = false;
	}

	for(int i = 0; i < len; i++){
		if(line[i] == '/' && line[i+1] == '/'){
			break;
		}
		if(line[i] == '/' && line[i+1] == '*'){
			comment = true;
			continue;
		}
		if(line[i] == '*' && line[i+1] == '/'){
			comment = false;
			i++;
			continue;
		}

		if(!comment){
			if(!quote){
				if(isspace(line[i]) || is_symbol(line[i])){
					if(pos > 0){
						add_to_list(word, tokens, counter);
						counter++;

						memset(word, 0, 64);
						pos = 0;
					}
					if(is_symbol(line[i])){
						word[0] = line[i];
						add_to_list(word, tokens, counter);
						counter++;

						memset(word, 0, 64);
					}
				}
				else if(isalnum(line[i]) || line[i] == '"' || line[i] == '_'){
					word[pos] = line[i];
					pos++;

					if(line[i] == '"'){
						quote = true;
					}
				}
			}
			else{
				word[pos] = line[i];
				pos++;
				if(line[i] == '"'){
					quote = false;
				}
			}
		}

	}

	return counter;
}

/**
	Decides what type a token is.
	@param token: token to check
	@return the type
*/
enum token_t token_type(char *token){
	if(is_keyword(token)){
		return KEYWORD;
	}
	else if(is_symbol(token[0])){
		return SYMBOL;
	}
	else if(isdigit(token[0])){
		size_t len = strlen(token);
		for(int i = 1; i < len; i++){
			if(!isdigit(token[i])){
				printf("invalid variable name %s\n", token);
				exit(1);
			}
		}
		return INT_CONST;
	}
	else if(token[0] == '"'){
		return STRING_CONST;
	}
	else{
		return IDENTIFIER;
	}
}

/**
	Decides what type a keyword is.
	@param token: token to check
	@return the type
*/
enum keyword_t keyword_type(char *token){
	if(strcmp(token, "class") == 0){
		return CLASS;
	}
	else if(strcmp(token, "method") == 0){
		return METHOD;
	}
	else if(strcmp(token, "function") == 0){
		return FUNCTION;
	}
	else if(strcmp(token, "constructor") == 0){
		return CONSTRUCTOR;
	}
	else if(strcmp(token, "int") == 0){
		return INT;
	}
	else if(strcmp(token, "boolean") == 0){
		return BOOLEAN;
	}
	else if(strcmp(token, "char") == 0){
		return CHAR;
	}
	else if(strcmp(token, "void") == 0){
		return VOID;
	}
	else if(strcmp(token, "var") == 0){
		return VAR;
	}
	else if(strcmp(token, "static") == 0){
		return STATIC;
	}
	else if(strcmp(token, "field") == 0){
		return FIELD;
	}
	else if(strcmp(token, "let") == 0){
		return LET;
	}
	else if(strcmp(token, "do") == 0){
		return DO;
	}
	else if(strcmp(token, "if") == 0){
		return IF;
	}
	else if(strcmp(token, "else") == 0){
		return ELSE;
	}
	else if(strcmp(token, "while") == 0){
		return WHILE;
	}
	else if(strcmp(token, "return") == 0){
		return RETURN;
	}
	else if(strcmp(token, "true") == 0){
		return TRUE;
	}
	else if(strcmp(token, "false") == 0){
		return FALSE;
	}
	else if(strcmp(token, "null") == 0){
		return NULL_T;
	}
	else if(strcmp(token, "this") == 0){
		return THIS;
	}

	return 0;
}

/**
	Decides if a token is an operator.
	@param token: token to check
	@return true if it's an operator, else false
*/
bool is_op(char *token){
	char ops[10] = "+-*/&|<>=";

	for(int i = 0; i < 9; i++){
		if(ops[i] == token[0]){
			return true;
		}
	}
	return false;
}

/**
	Decides if a token is a unary operator.
	@param token: token to check
	@return true if it's an operator, else false
*/
bool is_unary_op(char *token){
	char ops[3] = "-~";

	for(int i = 0; i < 3; i++){
		if(ops[i] == token[0]){
			return true;
		}
	}
	return false;
}

/**
	Decides if a token is a statement.
	@param token: token to check
	@return true if it's a statement, else false
*/
bool is_statement(char *token){
	if(keyword_type(token) == LET){
		return true;
	}
	else if(keyword_type(token) == IF){
		return true;
	}
	else if(keyword_type(token) == WHILE){
		return true;
	}
	else if(keyword_type(token) == DO){
		return true;
	}
	else if(keyword_type(token) == RETURN){
		return true;
	}

	return false;
}
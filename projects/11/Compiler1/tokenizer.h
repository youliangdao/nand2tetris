#pragma once

#include <stdint.h>
#include <stdbool.h>

/**
	Text processing functions, used for tokenizing the input file.
*/

enum token_t {
	KEYWORD,
	SYMBOL,
	IDENTIFIER,
	INT_CONST,
	STRING_CONST
};

enum keyword_t {
	CLASS,
	METHOD,
	FUNCTION,
	CONSTRUCTOR,
	INT,
	BOOLEAN,
	CHAR,
	VOID,
	VAR,
	STATIC,
	FIELD,
	LET,
	DO,
	IF,
	ELSE,
	WHILE,
	RETURN,
	TRUE,
	FALSE,
	NULL_T,
	THIS
};

/**
	Takes a line and splits it into Jack language tokens.
	@param line: line to split
	@param tokens: array of tokens
	@param start: true if it is the beginning of a file, else false
	@return total number of tokens
*/
uint32_t tokenize(char *line, char ***tokens, bool start);

/**
	Decides what type a token is.
	@param token: token to check
	@return the type
*/
enum token_t token_type(char *token);

/**
	Decides what type a keyword is.
	@param token: token to check
	@return the type
*/
enum keyword_t keyword_type(char *token);

/**
	Decides if a token is an operator.
	@param token: token to check
	@return true if it's an operator, else false
*/
bool is_op(char *token);

/**
	Decides if a token is a unary operator.
	@param token: token to check
	@return true if it's an operator, else false
*/
bool is_unary_op(char *token);

/**
	Decides if a token is a statement.
	@param token: token to check
	@return true if it's a statement, else false
*/
bool is_statement(char *token);
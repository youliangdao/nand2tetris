#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "global.h"
#include "tokenizer.h"
#include "compilation_engine.h"
#include "vmwriter.h"
#include "hashtable.h"

char *class_name = NULL;
char *subroutine_name = NULL;
uint16_t label_index = 0;
char kind_strings[5][9] = {"none", "static", "this", "argument", "local"};

/**
	Beginning of the recursive descent parsing algorithm. All other functions are called starting here.
	It uses the token_index global variable to keep the position in the array.
	@param tokens: the array of tokens
	@param fp: file pointer to write to
*/
void compile_class(char **tokens, FILE* fp){
	if(token_type(tokens[token_index]) == KEYWORD && keyword_type(tokens[token_index]) == CLASS){
		token_index++;

		// change class name
		class_name = strdup(tokens[token_index]);
		
		token_index++;

		// read '{'
		token_index++;

		while(keyword_type(tokens[token_index]) == STATIC || keyword_type(tokens[token_index]) == FIELD){
			compile_class_var_dec(tokens, fp);
		}

		while(keyword_type(tokens[token_index]) == CONSTRUCTOR || keyword_type(tokens[token_index]) == FUNCTION || 
				keyword_type(tokens[token_index]) == METHOD){
			compile_subroutine_dec(tokens, fp);
		}

		// delete class names
		free(class_name);
		class_name = NULL;

		// clear class scope table
		clear_ht(&class_table);
	}
}

void compile_class_var_dec(char **tokens, FILE* fp){
	enum kind_t var_kind = NONE_K;
	char *var_type = NULL;
	char *var_name = NULL;

	if(keyword_type(tokens[token_index]) == STATIC){
		// compile static
		var_kind = STATIC_K;
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == FIELD){
		// compile field
		var_kind = FIELD_K;
		token_index++;
	}

	// compile type
	var_type = strdup(tokens[token_index]);
	token_index++;

	// read variable name
	var_name = strdup(tokens[token_index]);
	token_index++;

	insert_HT(&class_table, var_name, var_type, var_kind);

	free(var_name);
	var_name = NULL;

	while(tokens[token_index][0] == ','){
		token_index++;

		// new var name
		var_name = strdup(tokens[token_index]);
		insert_HT(&class_table, var_name, var_type, var_kind);
		token_index++;

		free(var_name);
		var_name = NULL;
	}

	// read ';'
	token_index++;
	free(var_type);
}

void compile_subroutine_dec(char **tokens, FILE* fp){
	enum keyword_t subroutine_type;
	char *full_sub_name = NULL;

	if(keyword_type(tokens[token_index]) == CONSTRUCTOR){
		subroutine_type = CONSTRUCTOR;
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == FUNCTION){
		subroutine_type = FUNCTION;
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == METHOD){
		subroutine_type = METHOD;
		token_index++;
	}

	if(token_type(tokens[token_index]) == KEYWORD){
		// compile void
		token_index++;
	}
	else{
		token_index++;
	}

	// subroutine name
	subroutine_name = strdup(tokens[token_index]);
	token_index++;

	// '('
	token_index++;

	if(subroutine_type == METHOD){
		// for the 'this' parameter
		insert_HT(&subroutine_table, "this", class_name, ARG_K);
	}

	compile_parameter_list(tokens, fp);

	// ')'
	token_index++;

	// read '{'
	token_index++;

	while(keyword_type(tokens[token_index]) == VAR){
		compile_var_dec(tokens, fp);
	}

	full_sub_name = (char*)calloc(strlen(class_name) + strlen(subroutine_name) + 2, sizeof(char));
	strcat(full_sub_name, class_name);
	strcat(full_sub_name, ".");
	strcat(full_sub_name, subroutine_name);

	write_function(full_sub_name, get_var_count(subroutine_table, VAR_K), fp);

	if(subroutine_type == METHOD){
		write_push("argument", 0, fp);
		write_pop("pointer", 0, fp);
	}
	else if(subroutine_type == CONSTRUCTOR){
		write_push("constant", get_var_count(class_table, FIELD_K), fp);
		write_call("Memory.alloc", 1, fp);
		write_pop("pointer", 0, fp);
	}

	compile_subroutine_body(tokens, fp);

	// clear subroutine scope table
	clear_ht(&subroutine_table);

	free(full_sub_name);
}

void compile_parameter_list(char **tokens, FILE* fp){
	char *var_type = NULL;
	char *var_name = NULL;

	if(strcmp(tokens[token_index], ")") != 0){

		do{
			// variable type
			var_type = strdup(tokens[token_index]);
			token_index++;

			// compile var name
			var_name = strdup(tokens[token_index]);
			token_index++;

			insert_HT(&subroutine_table, var_name, var_type, ARG_K);


			if(tokens[token_index][0] == ','){
				token_index++;
			}
		}while(tokens[token_index - 1][0] == ',');

	}

	free(var_name);
	free(var_type);

}

void compile_subroutine_body(char **tokens, FILE* fp){

	compile_statements(tokens, fp);

	// read '}'
	token_index++;
}

void compile_var_dec(char **tokens, FILE* fp){
	char *var_type = NULL;
	char *var_name = NULL;

	// read VAR
	token_index++;

	// compile type;
	var_type = strdup(tokens[token_index]);
	token_index++;

	// compile var name
	var_name = strdup(tokens[token_index]);
	token_index++;

	insert_HT(&subroutine_table, var_name, var_type, VAR_K);

	while(tokens[token_index][0] == ','){
		// read ','
		token_index++;

		// new var name
		var_name = strdup(tokens[token_index]);
		insert_HT(&subroutine_table, var_name, var_type, VAR_K);
		token_index++;
	}

	// read ';'
	token_index++;

	free(var_name);
	free(var_type);
}

void compile_statements(char **tokens, FILE* fp){
	// if it's not the end of a statement block
	while(is_statement(tokens[token_index])){
		if(keyword_type(tokens[token_index]) == LET){
			compile_let(tokens, fp);
		}
		else if(keyword_type(tokens[token_index]) == IF){
			compile_if(tokens, fp);
		}
		else if(keyword_type(tokens[token_index]) == WHILE){
			compile_while(tokens, fp);
		}
		else if(keyword_type(tokens[token_index]) == DO){
			compile_do(tokens, fp);
		}
		else if(keyword_type(tokens[token_index]) == RETURN){
			compile_return(tokens, fp);
		}
	}
}

void compile_let(char **tokens, FILE* fp){
	char *var_name = NULL;
	// read LET
	token_index++;

	// read var name
	var_name = strdup(tokens[token_index]);
	token_index++;

	// if it is an array
	if(tokens[token_index][0] == '['){
		token_index++;

		compile_expression(tokens, fp);
		// find variable
		if(is_item(subroutine_table, var_name)){
			write_push(kind_strings[get_kind(subroutine_table, var_name)], get_index(subroutine_table, var_name), fp);
		}
		else if(is_item(class_table, var_name)){
			write_push(kind_strings[get_kind(class_table, var_name)], get_index(class_table, var_name), fp);
		}
		write_arithmetic("add", fp);

		// ']'
		token_index++;

		// read '='
		token_index++;

		compile_expression(tokens, fp);

		write_pop("temp", 0, fp);
		write_pop("pointer", 1, fp);
		write_push("temp", 0, fp);
		write_pop("that", 0, fp);

	}
	else{
		// read '='
		token_index++;

		compile_expression(tokens, fp);

		if(is_item(subroutine_table, var_name)){
			write_pop(kind_strings[get_kind(subroutine_table, var_name)], get_index(subroutine_table, var_name), fp);
		}
		else if(is_item(class_table, var_name)){
			write_pop(kind_strings[get_kind(class_table, var_name)], get_index(class_table, var_name), fp);
		}
	}

	// read ';'
	token_index++;

	free(var_name);

}

void compile_if(char **tokens, FILE* fp){
	char label1[11];
	char label2[11];

	snprintf(label1, 11, "L1_%d", label_index);
	snprintf(label2, 11, "L2_%d", label_index);
	label_index++;


	// read IF
	token_index++;

	// read '('
	token_index++;

	compile_expression(tokens, fp);
	write_arithmetic("not", fp);
	write_if(label1, fp);

	// read ')'
	token_index++;

	// read '{'
	token_index++;

	compile_statements(tokens, fp);

	write_goto(label2, fp);

	// read '}'
	token_index++;

	write_label(label1, fp);
	// if else
	if(keyword_type(tokens[token_index]) == ELSE){
		// read 'else'
		token_index++;

		// read '{'
		token_index++;

		compile_statements(tokens, fp);

		// read '}'
		token_index++;
	}

	write_label(label2, fp);
}

void compile_while(char **tokens, FILE* fp){
	char label1[11];
	char label2[11];

	snprintf(label1, 11, "L1_%d", label_index);
	snprintf(label2, 11, "L2_%d", label_index);
	label_index++;

	// read WHILE
	token_index++;

	// read '('
	token_index++;
	write_label(label1, fp);
	compile_expression(tokens, fp);
	write_arithmetic("not", fp);

	// read ')'
	token_index++;

	// read '{'
	token_index++;
	write_if(label2, fp);
	compile_statements(tokens, fp);

	write_goto(label1, fp);
	write_label(label2, fp);

	// read '}'
	token_index++;

}

void compile_do(char **tokens, FILE* fp){
	// read DO
	token_index++;

	compile_subroutine_call(tokens, fp);

	// read ';'
	token_index++;
	write_pop("temp", 0, fp);
}

void compile_return(char **tokens, FILE* fp){
	// read RETURN
	token_index++;

	if(tokens[token_index][0] != ';'){
		compile_expression(tokens, fp);
	}
	else{
		write_push("constant", 0, fp);
	}
	write_return(fp);

	// read ';'
	token_index++;

}

void compile_expression(char **tokens, FILE* fp){
	char op;

	compile_term(tokens, fp);

	// compile operations
	while(is_op(tokens[token_index])){
		op = tokens[token_index][0];
		token_index++;

		compile_term(tokens, fp);

		if(op == '+'){
			write_arithmetic("add", fp);
		}
		else if(op == '-'){
			write_arithmetic("sub", fp);
		}
		else if(op == '*'){
			write_call("Math.multiply", 2, fp);
		}
		else if(op == '/'){
			write_call("Math.divide", 2, fp);
		}
		else if(op == '&'){
			write_arithmetic("and", fp);
		}
		else if(op == '|'){
			write_arithmetic("or", fp);
		}
		else if(op == '<'){
			write_arithmetic("lt", fp);
		}
		else if(op == '>'){
			write_arithmetic("gt", fp);
		}
		else if(op == '='){
			write_arithmetic("eq", fp);
		}
	}
}

void compile_term(char **tokens, FILE* fp){

	if(token_type(tokens[token_index]) == INT_CONST){
		uint16_t val = (uint16_t) atoi(tokens[token_index]);
		write_push("constant", val, fp);
		token_index++;
	}
	else if(token_type(tokens[token_index]) == STRING_CONST){
		char *temp = (char*)malloc(strlen(tokens[token_index]) - 1);
		memcpy(temp, tokens[token_index] + 1, strlen(tokens[token_index]) - 2);
		temp[strlen(tokens[token_index]) - 2] = 0;
		size_t l = strlen(temp);
		write_push("constant", l, fp);
		write_call("String.new", 1, fp);
		for(int i = 0; i < l; i++){
			write_push("constant", temp[i], fp);
			write_call("String.appendChar", 2, fp);
		}

		free(temp);
		token_index++;
	}
	else if(token_type(tokens[token_index]) == KEYWORD){
		// true, false, null, this
		if(keyword_type(tokens[token_index]) == TRUE){
			write_push("constant", 0, fp);
			write_arithmetic("not", fp);
		}
		else if(keyword_type(tokens[token_index]) == FALSE){
			write_push("constant", 0, fp);
		}
		else if(keyword_type(tokens[token_index]) == NULL_T){
			write_push("constant", 0, fp);
		}
		else if(keyword_type(tokens[token_index]) == THIS){
			// complete here
			write_push("pointer", 0, fp);
		}
		token_index++;
	}
	else if(token_type(tokens[token_index]) == IDENTIFIER){
		if(tokens[token_index + 1][0] == '['){
			char *var_name = NULL;
			// variable
			var_name = strdup(tokens[token_index]);
			token_index++;

			// '['
			token_index++;

			compile_expression(tokens, fp);
			// ']'
			token_index++;


			if(is_item(subroutine_table, var_name)){
				write_push(kind_strings[get_kind(subroutine_table, var_name)], get_index(subroutine_table, var_name), fp);
			}
			else if(is_item(class_table, var_name)){
				write_push(kind_strings[get_kind(class_table, var_name)], get_index(class_table, var_name), fp);
			}
			write_arithmetic("add", fp);

			write_pop("pointer", 1, fp);
			write_push("that", 0, fp);

		}
		else if(tokens[token_index + 1][0] == '(' || tokens[token_index + 1][0] == '.'){
			compile_subroutine_call(tokens, fp);
		}
		else{
			if(is_item(subroutine_table, tokens[token_index])){
				write_push(kind_strings[get_kind(subroutine_table, tokens[token_index])], get_index(subroutine_table, tokens[token_index]), fp);
			}
			else if(is_item(class_table, tokens[token_index])){
				write_push(kind_strings[get_kind(class_table, tokens[token_index])], get_index(class_table, tokens[token_index]), fp);
			}

			token_index++;
		}
	}
	else if(token_type(tokens[token_index]) == SYMBOL){
		char op;
		// read unaryOp
		if(is_unary_op(tokens[token_index])){
			op = tokens[token_index][0];
			token_index++;

			compile_term(tokens, fp);

			if(op == '-'){
				write_arithmetic("neg", fp);
			}
			else if(op == '~'){
				write_arithmetic("not", fp);
			}
		}
		else{
			// if it is expression in ()
			token_index++;

			compile_expression(tokens, fp);

			token_index++;
		}
	}
}

void compile_subroutine_call(char **tokens, FILE* fp){
	char *var_name = NULL;
	char *c_name = NULL;
	char *f_name = NULL;
	char *full_name = NULL;
	uint16_t no_params = 0;

	if(tokens[token_index + 1][0] == '.'){
		// compile class/var name
		var_name = strdup(tokens[token_index]);
		token_index++;

		if(is_item(subroutine_table, var_name)){
			write_push(kind_strings[get_kind(subroutine_table, var_name)], get_index(subroutine_table, var_name), fp);
			no_params++;
			c_name = strdup(get_type(subroutine_table, var_name));
			free(var_name);
			var_name = c_name;
		}
		else if(is_item(class_table, var_name)){
			write_push(kind_strings[get_kind(class_table, var_name)], get_index(class_table, var_name), fp);
			no_params++;
			c_name = strdup(get_type(class_table, var_name));
			free(var_name);
			var_name = c_name;
		}

		// read '.'
		token_index++;
	}
	else{
		no_params++;
		var_name = class_name;
		write_push("pointer", 0, fp);
	}
	// compile subroutine name
	f_name = strdup(tokens[token_index]);
	token_index++;

	full_name = (char*)calloc(strlen(var_name) + strlen(f_name) + 2, sizeof(char));
	strcat(full_name, var_name);
	strcat(full_name, ".");
	strcat(full_name, f_name);


	// '('
	token_index++;

	no_params += compile_expression_list(tokens, fp);

	write_call(full_name, no_params, fp);

	// ')'
	token_index++;

	if(var_name != class_name){
		free(var_name);
	}
	// if(c_name != NULL){
	// 	free(c_name);
	// }
	free(f_name);
	free(full_name);

}

uint16_t compile_expression_list(char **tokens, FILE* fp){
	uint16_t no_params = 0;

	if(tokens[token_index][0] != ')'){
		no_params++;
		compile_expression(tokens, fp);

		while(tokens[token_index][0] == ','){
			token_index++;
			no_params++;
			compile_expression(tokens, fp);
		}
	}

	return no_params;
}
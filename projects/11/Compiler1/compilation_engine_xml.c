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


/**
	Beginning of the recursive descent parsing algorithm. All other functions are called starting here.
	It uses the token_index global variable to keep the position in the array.
	@param tokens: the array of tokens
	@param fp: file pointer to write to
*/
void compile_class(char **tokens, FILE* fp){
	if(token_type(tokens[token_index]) == KEYWORD && keyword_type(tokens[token_index]) == CLASS){
		fprintf(fp, "<class>\n");
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;

		// read identifier
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;

		// read '{'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;

		while(keyword_type(tokens[token_index]) == STATIC || keyword_type(tokens[token_index]) == FIELD){
			compile_class_var_dec(tokens, fp);
		}

		while(keyword_type(tokens[token_index]) == CONSTRUCTOR || keyword_type(tokens[token_index]) == FUNCTION || 
				keyword_type(tokens[token_index]) == METHOD){
			compile_subroutine_dec(tokens, fp);
		}

		// read '}'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);

		fprintf(fp, "</class>\n");
	}
}

void compile_class_var_dec(char **tokens, FILE* fp){
	fprintf(fp, "<classVarDec>\n");
	if(keyword_type(tokens[token_index]) == STATIC){
		// compile static
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == FIELD){
		// compile field
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}

	// compile type; should be extended
	if(token_type(tokens[token_index]) == KEYWORD){
		// primitives
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else{
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;
	}

	// read variable name
	fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
	token_index++;

	while(tokens[token_index][0] == ','){
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;

		// new var name
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;
	}

	// read ';'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	fprintf(fp, "</classVarDec>\n");
}

void compile_subroutine_dec(char **tokens, FILE* fp){
	fprintf(fp, "<subroutineDec>\n");

	if(keyword_type(tokens[token_index]) == CONSTRUCTOR){
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == FUNCTION){
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else if(keyword_type(tokens[token_index]) == METHOD){
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}

	if(token_type(tokens[token_index]) == KEYWORD){
		// compile void
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else{
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;
	}

	// subroutine name
	fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
	token_index++;

	// '('
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_parameter_list(tokens, fp);

	// ')'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_subroutine_body(tokens, fp);

	fprintf(fp, "</subroutineDec>\n");
}

void compile_parameter_list(char **tokens, FILE* fp){
	fprintf(fp, "<parameterList>\n");
	if(strcmp(tokens[token_index], ")") != 0){

		do{
			// compile type; should be extended
			if(token_type(tokens[token_index]) == KEYWORD){
				// primitives
				fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
			}
			else{
				fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
			}
			token_index++;

			// compile var name
			fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
			token_index++;

			if(tokens[token_index][0] == ','){
				fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
				token_index++;
			}
		}while(tokens[token_index - 1][0] == ',');

	}
	fprintf(fp, "</parameterList>\n");
}

void compile_subroutine_body(char **tokens, FILE* fp){
	fprintf(fp, "<subroutineBody>\n");

	// read '{'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	while(keyword_type(tokens[token_index]) == VAR){
		compile_var_dec(tokens, fp);
	}

	compile_statements(tokens, fp);

	// read '}'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	fprintf(fp, "</subroutineBody>\n");
}

void compile_var_dec(char **tokens, FILE* fp){
	fprintf(fp, "<varDec>\n");

	// read VAR
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

		// compile type; should be extended
	if(token_type(tokens[token_index]) == KEYWORD){
		// primitives
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;
	}
	else{
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;
	}

	// compile var name
	fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
	token_index++;

	while(tokens[token_index][0] == ','){
		// read ','
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;

		// new var name
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;
	}

	// read ';'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	fprintf(fp, "</varDec>\n");
}

void compile_statements(char **tokens, FILE* fp){
	fprintf(fp, "<statements>\n");
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
	fprintf(fp, "</statements>\n");
}

void compile_let(char **tokens, FILE* fp){
	fprintf(fp, "<letStatement>\n");
	// read LET
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

	// read var name
	fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
	token_index++;

	// if it is an array
	if(tokens[token_index][0] == '['){
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;

		compile_expression(tokens, fp);

		// ']'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;
	}

	// read '='
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_expression(tokens, fp);

	// read ';'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;


	fprintf(fp, "</letStatement>\n");
}

void compile_if(char **tokens, FILE* fp){
	fprintf(fp, "<ifStatement>\n");
	// read IF
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

	// read '('
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_expression(tokens, fp);

	// read ')'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	// read '{'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_statements(tokens, fp);

	// read '}'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	// if else
	if(keyword_type(tokens[token_index]) == ELSE){
		fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		token_index++;

		// read '{'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;

		compile_statements(tokens, fp);

		// read '}'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;
	}


	fprintf(fp, "</ifStatement>\n");
}

void compile_while(char **tokens, FILE* fp){
	fprintf(fp, "<whileStatement>\n");
	// read WHILE
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

	// read '('
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_expression(tokens, fp);

	// read ')'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	// read '{'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_statements(tokens, fp);

	// read '}'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;


	fprintf(fp, "</whileStatement>\n");
}

void compile_do(char **tokens, FILE* fp){
	fprintf(fp, "<doStatement>\n");
	// read DO
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

	compile_subroutine_call(tokens, fp);

	// read ';'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	fprintf(fp, "</doStatement>\n");
}

void compile_return(char **tokens, FILE* fp){
	fprintf(fp, "<returnStatement>\n");
	// read RETURN
	fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
	token_index++;

	if(tokens[token_index][0] != ';'){
		compile_expression(tokens, fp);
	}

	// read ';'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	fprintf(fp, "</returnStatement>\n");
}

void compile_expression(char **tokens, FILE* fp){
	fprintf(fp, "<expression>\n");

	compile_term(tokens, fp);

	// compile operations
	while(is_op(tokens[token_index])){
		if(tokens[token_index][0] == '+'){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		else if(tokens[token_index][0] == '-'){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		else if(tokens[token_index][0] == '*'){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		else if(tokens[token_index][0] == '/'){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		else if(tokens[token_index][0] == '&'){
			fprintf(fp, "<symbol> &amp; </symbol>\n");
		}
		else if(tokens[token_index][0] == '|'){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		else if(tokens[token_index][0] == '<'){
			fprintf(fp, "<symbol> &lt; </symbol>\n");
		}
		else if(tokens[token_index][0] == '>'){
			fprintf(fp, "<symbol> &gt; </symbol>\n");
		}
		else if(tokens[token_index][0] == '='){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		}
		token_index++;

		compile_term(tokens, fp);
	}

	fprintf(fp, "</expression>\n");
}

void compile_term(char **tokens, FILE* fp){
	fprintf(fp, "<term>\n");

	if(token_type(tokens[token_index]) == INT_CONST){
		fprintf(fp, "<integerConstant> %s </integerConstant>\n", tokens[token_index]);
		token_index++;
	}
	else if(token_type(tokens[token_index]) == STRING_CONST){
		char *temp = (char*)malloc(strlen(tokens[token_index]) - 1);
		memcpy(temp, tokens[token_index] + 1, strlen(tokens[token_index]) - 2);
		temp[strlen(tokens[token_index]) - 2] = 0;
		fprintf(fp, "<stringConstant> %s </stringConstant>\n", temp);
		free(temp);
		token_index++;
	}
	else if(token_type(tokens[token_index]) == KEYWORD){
		// true, false, null, this
		if(keyword_type(tokens[token_index]) == TRUE){
			fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		}
		else if(keyword_type(tokens[token_index]) == FALSE){
			fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		}
		else if(keyword_type(tokens[token_index]) == NULL_T){
			fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		}
		else if(keyword_type(tokens[token_index]) == THIS){
			fprintf(fp, "<keyword> %s </keyword>\n", tokens[token_index]);
		}
		token_index++;
	}
	else if(token_type(tokens[token_index]) == IDENTIFIER){
		if(tokens[token_index + 1][0] == '['){
			fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
			token_index++;

			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			token_index++;

			compile_expression(tokens, fp);

			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			token_index++;
		}
		else if(tokens[token_index + 1][0] == '(' || tokens[token_index + 1][0] == '.'){
			compile_subroutine_call(tokens, fp);
		}
		else{
			fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
			token_index++;
		}
	}
	else if(token_type(tokens[token_index]) == SYMBOL){
		// read unaryOp
		if(is_unary_op(tokens[token_index])){
			if(tokens[token_index][0] == '-'){
				fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			}
			else if(tokens[token_index][0] == '~'){
				fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			}
			token_index++;

			compile_term(tokens, fp);
		}
		else{
			// if it is expression in ()
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			token_index++;

			compile_expression(tokens, fp);

			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			token_index++;
		}
	}


	fprintf(fp, "</term>\n");
}

void compile_subroutine_call(char **tokens, FILE* fp){
	if(tokens[token_index + 1][0] == '.'){
		// compile class/var name
		fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
		token_index++;

		// read '.'
		fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
		token_index++;
	}
	// compile subroutine name
	fprintf(fp, "<identifier> %s </identifier>\n", tokens[token_index]);
	token_index++;

	// '('
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

	compile_expression_list(tokens, fp);

	// ')'
	fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
	token_index++;

}

uint16_t compile_expression_list(char **tokens, FILE* fp){
	fprintf(fp, "<expressionList>\n");

	if(tokens[token_index][0] != ')'){
		compile_expression(tokens, fp);

		while(tokens[token_index][0] == ','){
			fprintf(fp, "<symbol> %s </symbol>\n", tokens[token_index]);
			token_index++;

			compile_expression(tokens, fp);
		}
	}

	fprintf(fp, "</expressionList>\n");
}
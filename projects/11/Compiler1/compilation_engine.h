#pragma once


/**
	Beginning of the recursive descent parsing algorithm. All other functions are called starting here.
	It uses the token_index global variable to keep the position in the array.
	@param tokens: the array of tokens
	@param fp: file pointer to write to
*/
void compile_class          (char **tokens, FILE* fp);
void compile_class_var_dec  (char **tokens, FILE* fp);
void compile_subroutine_dec (char **tokens, FILE* fp);
void compile_subroutine_body(char **tokens, FILE* fp);
void compile_var_dec        (char **tokens, FILE* fp);
void compile_statements     (char **tokens, FILE* fp);
void compile_let            (char **tokens, FILE* fp);
void compile_if             (char **tokens, FILE* fp);
void compile_while          (char **tokens, FILE* fp);
void compile_do             (char **tokens, FILE* fp);
void compile_return         (char **tokens, FILE* fp);
void compile_expression     (char **tokens, FILE* fp);
void compile_subroutine_call(char **tokens, FILE* fp);
void compile_term           (char **tokens, FILE* fp);
uint16_t compile_expression_list(char **tokens, FILE* fp);
void compile_parameter_list(char **tokens, FILE* fp);
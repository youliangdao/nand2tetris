#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <dirent.h>
#include "tokenizer.h"
#include "compilation_engine.h"
#include "global.h"
#include "hashtable.h"

size_t token_index;
struct HashTable class_table;
struct HashTable subroutine_table;


/**
	Prints the help string.
*/
void help(){
	printf("Usage: ./jack_compiler [Jack file/directory]\n");
}


/**
	Checks if a file is a directory.
	@param name: file name
	@return true if it is a directory, else false
*/
bool is_dir(char *name){
	size_t extension = strlen(name) - 5;
	if(strcmp(name+extension, ".jack") == 0){
		return false;
	}
	return true;
}

/**
	Reads the files in a directory.
	@param directory: the directory name
	@param files: array of strings where the names will be written
	@return the number of files in the directory
*/
size_t read_dir(char *directory, char ***files){
	int count = 0;
	struct dirent *p_dirent;
	DIR *p_dir;
	p_dir = opendir(directory);
	if(p_dir == NULL){
		printf("Cannot open directory %s.", directory);
		return 0;
	}

	while((p_dirent = readdir(p_dir)) != NULL){
		if(strcmp(p_dirent->d_name, ".") == 0 || strcmp(p_dirent->d_name, "..") == 0){
			continue;
		}
		if(strcmp(p_dirent->d_name + (strlen(p_dirent->d_name) - 4), "jack")){
			continue;
		}
		count++;
		(*files) = (char**)realloc((*files), sizeof(char*) * count);
		(*files)[count-1] = (char*)malloc(strlen(p_dirent->d_name) + 1);
		strcpy((*files)[count-1], p_dirent->d_name);
	}
	closedir(p_dir);
	return count;

}

static const char token_names[5][16] = {"keyword", "symbol", "identifier", "integerConstant", "stringConstant"};


void tk_to_xml(FILE *fp, uint32_t n, char **tokens){
	fprintf(fp, "<tokens>\n");
	for(int i = 0; i < n; i++){
		enum token_t t_type = token_type(tokens[i]);
		const char *typ = token_names[t_type];
		fprintf(fp, "<%s>", typ);

		if(t_type == STRING_CONST){
			char *temp = (char*)malloc(strlen(tokens[i]) - 1);
			memcpy(temp, tokens[i] + 1, strlen(tokens[i]) - 2);
			temp[strlen(tokens[i]) - 2] = 0;
			fprintf(fp, " %s ", temp);
			free(temp);
		}
		else if(tokens[i][0] == '<'){
			fprintf(fp, " &lt; ");
		}
		else if(tokens[i][0] == '>'){
			fprintf(fp, " &gt; ");
		}
		else if(tokens[i][0] == '&'){
			fprintf(fp, " &amp; ");
		}
		else{
			fprintf(fp, " %s ", tokens[i]);
		}

		fprintf(fp, "</%s>\n", typ);
	}
	fprintf(fp, "</tokens>");
}

int main(int argc, char **argv){
	if(argc != 2){
		help();
		exit(1);
	}

	char **files = NULL;
	size_t no_files = 0;

	if(is_dir(argv[1])){
		no_files = read_dir(argv[1], &files);
		if(no_files == 0){
			printf("No Jack files found.");
			exit(1);
		}

		for(int i = 0; i < no_files; i++){

			char *file = (char*)malloc(64);
			strcpy(file, argv[1]);
			strcpy(file + strlen(argv[1]), "/");
			file = strcat(file, files[i]);
			char *temp = files[i];
			files[i] = file;
			free(temp);
		}
	}
	else{
		files = (char**)malloc(sizeof(char*));
		files[0] = (char*)malloc(strlen(argv[1]) + 1);
		strcpy(files[0], argv[1]);
		no_files = 1;
	}

	FILE* jack_fp;
	FILE* vm_fp;

	init_hash_table(&class_table, 64);
	init_hash_table(&subroutine_table, 64);

	char **tokens = NULL;
	size_t token_c = 0;

	for(int i = 0; i < no_files; i++){

		jack_fp = fopen(files[i], "r");
		if(jack_fp == NULL){
			printf("Cannot open Jack file: %s.\n", files[i]);
			exit(1);
		}

		token_index = 0;

		char *line = NULL;
		size_t len = 0;
		ssize_t read;

		uint32_t line_count = 0;
		bool start = true;

		// tokenize file
		while((read = getline(&line, &len, jack_fp)) != -1){
			line_count++;
			token_c = tokenize(line, &tokens, start);
			start = false;
		}

		// write tokens xml
		size_t file_len = strlen(files[i]);
		char *vm_fn = (char*)malloc(file_len -1 ); // file_len - 1 !!!!
		memcpy(vm_fn, files[i], file_len - 4);
		strcpy(vm_fn + (file_len - 4), "vm");

		vm_fp = fopen(vm_fn, "w");
		if(vm_fp == NULL){
			printf("Cannot open Jack file: %s.\n", files[i]);
			exit(1);
		}


		//tk_to_xml(vm_fp, token_c, tokens);
		compile_class(tokens, vm_fp);


		// free the tokens for this file
		for(int i = 0; i < token_c; i++){
			free(tokens[i]);
		}
		free(tokens);
		free(vm_fn);
		token_c = 0;
		tokens = NULL;

		// close files
		fclose(jack_fp);
		fclose(vm_fp);

	}

	return 0;
}

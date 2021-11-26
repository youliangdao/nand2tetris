#include <stdio.h>
#include <stdint.h>
/**
	Here are the functions that generate the VM stack commands.
*/

void write_push(char *segment, uint16_t index, FILE *fp){
	fprintf(fp, "push %s %d\n", segment, index);
}

void write_pop(char *segment, uint16_t index, FILE *fp){
	fprintf(fp, "pop %s %d\n", segment, index);
}

void write_arithmetic(char *command, FILE *fp){
	fprintf(fp, "%s\n", command);
}

void write_label(char *label, FILE *fp){
	fprintf(fp, "label %s\n", label);
}

void write_goto(char *label, FILE *fp){
	fprintf(fp, "goto %s\n", label);
}

void write_if(char *label, FILE *fp){
	fprintf(fp, "if-goto %s\n", label);
}

void write_call(char *name, uint16_t nArgs, FILE *fp){
	fprintf(fp, "call %s %d\n", name, nArgs);
}

void write_function(char *name, uint16_t nLocals, FILE *fp){
	fprintf(fp, "function %s %d\n", name, nLocals);
}

void write_return(FILE *fp){
	fprintf(fp, "return\n");
}
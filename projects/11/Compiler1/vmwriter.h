#pragma once

#include <stdint.h>

/**
	Here are the functions that generate the VM stack commands.
*/

void write_push(char *segment, uint16_t index, FILE *fp);

void write_pop(char *segment, uint16_t index, FILE *fp);

void write_arithmetic(char *command, FILE *fp);

void write_label(char *label, FILE *fp);

void write_goto(char *label, FILE *fp);

void write_if(char *label, FILE *fp);

void write_call(char *name, uint16_t nArgs, FILE *fp);

void write_function(char *name, uint16_t nLocals, FILE *fp);

void write_return(FILE *fp);
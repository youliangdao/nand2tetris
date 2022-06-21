#include<stdio.h>
#include<string.h>

static int labelNum = 0;


typedef enum{
    SEG_CONST,
    SEG_ARG,
    SEG_LOCAL,
    SEG_STATIC,
    SEG_THIS,
    SEG_THAT,
    SEG_POINTER,
    SEG_TEMP,
} SegmentKind;

typedef enum{
    COM_ADD,
    COM_SUB,
    COM_NEG,
    COM_EQ,
    COM_GT,
    COM_LT,
    COM_AND,
    COM_OR,
    COM_NOT,
} CommandKind;

void getSegKind(SegmentKind kind, char *segkind);
void getComKind(CommandKind kind, char *comkind);
void getLabel(char *label);
void writePush(SegmentKind kind, int index, FILE *fp);
void writePop(SegmentKind kind, int index, FILE *fp);
void writeArithmetic(CommandKind kind, FILE *fp);
void writeLabel(char *label, FILE *fp);
void writeGoto(char *label, FILE *fp);
void writeIf(char *label, FILE *fp);
void writeCall(char *name, int nArgs, FILE *fp);
void writeFunction(char *name, int nLocals, FILE *fp);
void writeReturn(FILE *Fp);


#include "VMWriter.h"

void getSegKind(SegmentKind kind, char *segkind){
    if(kind == SEG_CONST) strcpy(segkind, "constant");
    else if(kind == SEG_ARG) strcpy(segkind, "argument");
    else if(kind == SEG_LOCAL) strcpy(segkind, "local");
    else if(kind == SEG_STATIC) strcpy(segkind, "static");
    else if(kind == SEG_THIS) strcpy(segkind, "this");
    else if(kind == SEG_THAT) strcpy(segkind, "that");
    else if(kind == SEG_POINTER) strcpy(segkind, "pointer");
    else if(kind == SEG_TEMP) strcpy(segkind, "temp");
}

void getComKind(CommandKind kind, char *comkind){
    if(kind == COM_ADD) strcpy(comkind, "add");
    else if(kind == COM_SUB) strcpy(comkind, "sub");
    else if(kind == COM_NEG) strcpy(comkind, "neg");
    else if(kind == COM_EQ) strcpy(comkind, "eq");
    else if(kind == COM_GT) strcpy(comkind, "gt");
    else if(kind == COM_LT) strcpy(comkind, "lt");
    else if(kind == COM_AND) strcpy(comkind, "and");
    else if(kind == COM_OR) strcpy(comkind, "or");
    else if(kind == COM_NOT) strcpy(comkind, "not");
}

void getLabel(char *label){
    char temp[15] = "LABEL_";
    char numstr[5] = "";
    sprintf(numstr, "%d", labelNum);
    strcat(temp, numstr);
    strcpy(label, temp);
    ++labelNum;
}

void writePush(SegmentKind kind, int index, FILE *fp){
    char segkind[15];
    getSegKind(kind, segkind);
    fprintf(fp, "   push %s %d\n", segkind, index);
}

void writePop(SegmentKind kind, int index, FILE *fp){
    char segkind[15];
    getSegKind(kind, segkind);
    fprintf(fp, "   pop %s %d\n", segkind, index);   
}

void writeArithmetic(CommandKind kind, FILE *fp){
    char comkind[10];
    getComKind(kind, comkind);
    fprintf(fp, "   %s\n", comkind);
}

void writeLabel(char *label, FILE *fp){
    fprintf(fp, "label %s\n", label);
}

void writeGoto(char *label, FILE *fp){
    fprintf(fp, "   goto %s\n", label);
}

void writeIf(char *label, FILE *fp){
    fprintf(fp, "   if-goto %s\n", label);
}

void writeCall(char *name, int nArgs, FILE *fp){
    fprintf(fp, "   call %s %d\n", name, nArgs);
}

void writeFunction(char *name, int nLocals, FILE *fp){
    fprintf(fp, "function %s %d\n", name, nLocals);
}

void writeReturn(FILE *fp){
    fprintf(fp, "   return\n");
}
#include<errno.h>
#include<ctype.h>
#include<dirent.h>
#include "JackTokenizer.h"
#include "CompilationEngine.h"
#include "VMWriter.h"
#include "SymbolHashTable.h"
#include "FileOperation.h"

void process(char *jackfilepath, char *xmlfilepath){
    FILE *fp_jack, *fp_xml;
    if((fp_jack = fopen(jackfilepath, "r")) == NULL)
        error("Error in \"process\": cannot open jackfile.");

    if((fp_xml = fopen(xmlfilepath, "w")) == NULL)
        error("Error in \"process\": cannot open xmlfile.");
    
    CompilationEngine *comp = compilationEngineInit(fp_jack, fp_xml);
    constructHashTable();
    compileClass(comp);

    fclose(fp_jack);
    fclose(fp_xml);
}

int main(int argc, char *argv[]){
    DIR *dp;
    if(argc != 2) error("Error in \"main\": the number of parameters must be 2.");

    dp = opendir(argv[1]);

    if(dp){
        struct dirent *entry;
        char *dirname = argv[1];
        char jackfilepath[100];
        char xmlfilepath[100];

        while((entry = readdir(dp)) != NULL){
            char *filename = entry->d_name;
            if(!isJackfile(filename)) continue;

            strcpy(jackfilepath, dirname);
            strcat(jackfilepath, "/");
            strcat(jackfilepath, filename);

            strcpy(xmlfilepath, dirname);
            strcat(xmlfilepath, "/");
            strncat(xmlfilepath, filename, strlen(filename) - strlen(".jack"));
            strcat(xmlfilepath, ".vm");

            process(jackfilepath, xmlfilepath);
        }
    }
    else if(errno == ENOTDIR && isJackfile(argv[1])){
        char *filename = argv[1];
        char jackfilepath[100];
        char xmlfilepath[100];

        strcpy(jackfilepath, filename);

        size_t tmp = strlen(filename) - strlen(".jack");
        strncpy(xmlfilepath, filename, tmp);
        xmlfilepath[tmp] = '\0';
        strcat(xmlfilepath, ".vm");

        process(jackfilepath, xmlfilepath);
    }
    else error("Error in \"main\": invalid parameter, neither file nor directory.");

    return 0;
}
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0

#define noError 0
#define cmdError 1
#define closeError 2


#define nullFile 0


int closeFile(FILE* file) {
	if(fclose(file) == EOF) {
		perror("Close file error ");
		return FALSE;
	}
	return TRUE;
}

void printErrorMsg(int errorMsg) {
	switch (errorMsg) {
		case noError : fprintf(stderr,"No error oucurred!\n"); break;
		case cmdError : fprintf(stderr,"Command error!\nUseage : command  source_file  dest_file\n"); break;
		case closeError : fprintf(stderr,"File close error!\n"); break;
	}
}

int main(int argc,char* argv[]) {
	int errorMsg = noError;
	if(argc != 3) {
		errorMsg = cmdError;
		printErrorMsg(errorMsg);
		exit(1);
	}
	
	FILE* source_file;
	FILE* dest_file;
	
	source_file = fopen(argv[1],"r");
	if(source_file == nullFile) {
		perror("Your first parameter has some problem ");
		exit(1);
	}
	
	
	dest_file = fopen(argv[2],"w");
	if(dest_file == nullFile) {
		perror(argv[2]);
		exit(1);
	}
			
	int word = 0;
	while((word = fgetc(source_file)) != EOF) {
		fputc(word,dest_file);
	}
	perror("");
	   
	if(!closeFile(source_file) || !closeFile(dest_file)) {
		errorMsg = closeError;
		printErrorMsg(errorMsg);
		exit(1);
	}
	return 0;
}
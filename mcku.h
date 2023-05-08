#include<string.h>
#include<stdbool.h>
#define MAX_NAME 256

char *appendNewLine(char * str);

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	
	/* Add more fields if needed */
	bool isExit;
};

// mcku.c 외부 변수
extern struct pcb *current;
extern char *ptbr;

struct pcb* pcbs;
int nProcess;
// pid  1씩 올려서 current에 넣는 방식
void ku_scheduler(char pid){
	int count = 0;
	
	do {
		current = &ptbr[++pid % nProcess];
		ptbr = current->pgtable;
	} while(current -> isExit && count++ < nProcess);
	
	// 만약 모든 프로세스가 exit 됐다면 프로그램 종료
	if(count >= nProcess) {
		exit(0);
	}
}

// 이게 맞나
void ku_pgfault_handler(char va){
	int pt_index = (va & 0xF0) >> 4;
	ptbr[pt_index] = 1;
}


void ku_proc_exit(char pid){
	pcbs[pid].isExit = true;
}


void ku_proc_init(int nprocs, char *flist){
	nProcess = nprocs;
	pcbs = malloc(sizeof(struct pcb) * nprocs);
	
	FILE *fp = fopen(flist, "r");
	if(fp == NULL) {
		printf("Error: file open 1");
		exit(0);
	}

	printf("\n %d",nProcess);
	size_t len = 0;
	for(int i = 0; i < nProcess; ++i) {
		printf("\nstart proc init");
		char *procFile = NULL;
		size_t len = 0;
		ssize_t read;
		read = getline(&procFile, &len, fp);
		if(read == -1) {
			printf("Error: file read 1");
			exit(0);
		}
		
		if(procFile[strlen(procFile) - 1] != '\n') {
			procFile = appendNewLine(procFile);
		}
		if(procFile[strlen(procFile) - 1] != '\n') printf("Error: not append new line");
		//개행문자 지워주는 과정
		//processFileName = deleteNewLine(processFileName);

		pcbs[i].fd = fopen(procFile, "r");
		pcbs[i].pid = i;
		pcbs[i].pgtable = malloc(sizeof(pcbs->pgtable) *16);
		pcbs[i].isExit = false;

		free(procFile);
		printf("\nend proc init");
	}
	current = &pcbs[0];
	ptbr = current->pgtable;

	fclose(fp);
}

char *appendNewLine(char *str) {
	
}
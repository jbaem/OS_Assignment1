#include<string.h>
#include<stdbool.h>
#define MAX_NAME_LENGTH 256
#define PTE_COUNT 16

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	
	bool *freeList;
};

char *deleteLF(char * str); //delete Line Feed

/*extern mcku.c*/
extern struct pcb *current;
extern char *ptbr;

/*global*/
struct pcb* pcbs;
struct pcb pcbEnd = 0;
int nProcess;
int restProcess;

void ku_scheduler(char pid){
	printf("come in : %c\n", pid);
	while(restProcess > 0) {
		current = &pcbs[++pid % nProcess];
		/*current is not null*/
		if(current) {
			ptbr = current->pgtable;
			return;
		}
	}
}

void ku_pgfault_handler(char va) {
	int pt_index = (va & 0xF0) >> 4;
	ptbr[pt_index] = 1;
	return;
}


void ku_proc_exit(char pid){
	printf("free : %c\n", pid);
	/*free pcbs[pid]*/
	fclose(pcbs[pid].fd);
	free(pcbs[pid].pgtable);
	free(pcbs[pid].freeList);
	&pcbs[pid] = pcbEnd;
	
	restProcess--;
	if(!restProcess) {
		free(pcbs);
	}
	return;
}


void ku_proc_init(int nprocs, char *flist){
	/*open text file*/
	FILE *fp = fopen(flist, "r");
	if(fp == NULL) {
		printf("Error: process file open failed");
		exit(0);
	}

	/*allocate pcbs for nprocs*/
	nProcess = nprocs;
	restProcess = nprocs;
	pcbs = malloc(sizeof(struct pcb) * nprocs);
	
	for(int pi = 0; pi < nProcess; ++pi) {
		/*read each text file's name*/
		char *procFile = NULL;
		size_t len = MAX_NAME_LENGTH;
		ssize_t read = getline(&procFile, &len, fp);
		if(read == -1) {
			printf("Error: file get line failed");
			exit(0);
		}

		/*delete '\n' last index of string*/
		if(procFile[strlen(procFile) - 1] == '\n') {
			procFile = deleteLF(procFile);
		}
		
		/*put elements in pcbs*/
		pcbs[pi].fd = fopen(procFile, "r");
		pcbs[pi].pid = pi;
		pcbs[pi].pgtable = malloc(sizeof(pcbs->pgtable) * PTE_COUNT);
		pcbs[pi].freeList = malloc(sizeof(bool) * PTE_COUNT);
		for(int i = 0; i < PTE_COUNT; ++i) pcbs[pi].freeList[i] = true;

		/*free character pointer*/
		free(procFile);
	}

	/*initializing current*/
	current = &pcbs[0];
	ptbr = current->pgtable;

	/*close first file*/
	fclose(fp);
	return;
}

char *deleteLF(char *str) {
	char * result = malloc(strlen(str) - 1);
	strncpy(result, str, strlen(str) - 1);
	
	/*not delete Line Feed*/
	if(strcmp(str, result) == 0) {
		printf("Error: change string failed");
		exit(0);
	}
	return result;
}
#include<string.h>
#include<stdbool.h>
#define MAX_NAME 256

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	
	bool isEnd;
	
};

char *deleteLF(char * str); //delete Line Feed

/*extern mcku.c*/
extern struct pcb *current;
extern char *ptbr;

/*global*/
struct pcb* pcbs;
int nProcess;
int restProcess;

void ku_scheduler(char pid){
	while(restProcess > 0) {
		current = &pcbs[++pid % nProcess];
		
		/*current does*/
		if(!(current->isEnd)) {
			ptbr = current->pgtable;
			return;
		}
	}
	current = NULL;
}

void ku_pgfault_handler(char va){
	int pt_index = (va & 0xF0) >> 4;
	ptbr[pt_index] = 1;
	return;
}


void ku_proc_exit(char pid){
	pcbs[pid].isEnd = true;
	fclose(pcbs[pid].fd);
	free(pcbs[pid].pgtable);
	restProcess--;
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
	
	for(int i = 0; i < nProcess; ++i) {
		/*read each text file's name*/
		char *procFile = NULL;
		size_t len = 128;
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
		pcbs[i].fd = fopen(procFile, "r");
		pcbs[i].pid = i;
		pcbs[i].pgtable = malloc(sizeof(pcbs->pgtable) *16);
		pcbs[i].isEnd = false;
		
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
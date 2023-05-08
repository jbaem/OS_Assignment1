#include<string.h>
#include<stdbool.h>

#define PAGE_SIZE 16
#define PTE_COUNT 16
#define FREE_LIST_SIZE 64
#define MAX_NAME_LENGTH 256
#define PHYSICAL_MEMORY 1024

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	bool isEnd;
};

char *deleteLF(char * str);

/*extern mcku.c*/
extern struct pcb *current;
extern char *ptbr;

/*global*/
struct pcb* pcbs;
int *freeList;
int freeIndex;
char *physicalMemory;

int nProcess;
int restProcess;

void ku_scheduler(char pid){
	/*find remain jobs*/
	while(restProcess > 0) {
		current = &pcbs[++pid % nProcess];
		/*current is not finished*/
		if(!(current->isEnd)) {
			ptbr = current->pgtable;
			return;
		}
	}

	/*all jobs finished*/
	if(!restProcess) {
		current = NULL;
	}
	return;
}

void ku_pgfault_handler(char va) {
	if(freeIndex == FREE_LIST_SIZE) return;

	int vpn = (va & 0xF0) >> 4;
	int pfn = 0;

	/*allocate page*/
	if(current->pgtable[vpn] == 0) {
		/*free list -> PFN*/
		pfn = freeList[freeIndex++];

		/*physical memory*/
		fseek(current->fd, vpn * PAGE_SIZE, SEEK_SET);
		fread(physicalMemory + pfn * PAGE_SIZE, PAGE_SIZE, 1, current->fd);

		/*pgtable update*/
		current->pgtable[vpn] = pfn;
	}
	else {
		/*pte already exist*/
		pfn = current->pgtable[vpn];
	}
}


void ku_proc_exit(char pid){
	/*free pcbs[pid]*/
	fclose(pcbs[pid].fd);
	free(pcbs[pid].pgtable);
	pcbs[pid].isEnd = true;
	restProcess--;

	/*all jobs finished*/
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

	/*allocate pcbs & free list*/
	nProcess = nprocs;
	restProcess = nprocs;
	pcbs = malloc(sizeof(struct pcb) * nprocs);
	freeList = malloc(FREE_LIST_SIZE);
	freeIndex = 0;
	physicalMemory = malloc(sizeof(char) * PHYSICAL_MEMORY);

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
		pcbs[pi].isEnd = false;

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

/*delete Line Feed*/
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

void PrintBinary(int num)
{	
	unsigned cnum = 1 << 31;
	int check = 0;
	while (cnum)
	{
		if (cnum & num)
		{
			printf("1");
			check = 1;
		}
		else
		{
			if (check != 0)
			{
				printf("0");
			}
			
		}
		cnum = cnum >> 1;
	}	
}
#include<string.h>
#include<stdbool.h>

#define MAX_NAME_LENGTH 256
#define PTE_COUNT 16
#define FREE_LIST_SIZE 64
#define VPN_MASK 0b11110000

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	bool isEnd;
};

/*function*/
char *delete_LF(char * str);
void init_free_list();
int allocate_page();
void free_page(int pfn);

/*extern mcku.c*/
extern struct pcb *current;
extern char *ptbr;

/*process*/
int nProcess;
int restProcess;
struct pcb* pcbs;

/*free list*/
char *freeList;
char freeIndex;

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
	unsigned int vpn = (va & VPN_MASK) >> 4;
	int pfn = freeList[freeIndex++];

	current->pgtable[vpn] = pfn;
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

	/*initialize pcbs & free list*/
	nProcess = nprocs;
	restProcess = nprocs;
	pcbs = malloc(sizeof(struct pcb) * nprocs);
	
	init_free_list();

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
			procFile = delete_LF(procFile);
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

char *delete_LF(char *str) {
	char * result = malloc(strlen(str) - 1);
	strncpy(result, str, strlen(str) - 1);
	
	/*not delete Line Feed*/
	if(strcmp(str, result) == 0) {
		printf("Error: change string failed");
		exit(0);
	}
	return result;
}

void init_free_list() {
	freeList = malloc(FREE_LIST_SIZE);
	for(int i = 0; i < FREE_LIST_SIZE; ++i) {
		freeList[i] = i;
	}

}

int allocate_page() {
	if(freeIndex >= FREE_LIST_SIZE) {
		return -1;
	}
	int pfn = freeList[freeIndex++];
	
	return pfn;
}

void free_page(int pfn) {
	freeList[--freeIndex] = pfn;
}
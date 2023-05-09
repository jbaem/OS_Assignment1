#include<string.h>
#include<stdbool.h>

#define MAX_NAME_LENGTH 256
#define PTE_COUNT 16
#define FREE_LIST_SIZE 64
#define VPN_MASK 0b11110000
#define PFN_MASK 0b11111100

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	bool isEnd;
};

/*function*/
char *delete_LF(char * str);
void init_pcbs(int id, char *file);
void init_free_list();
char allocate_page();
void free_page(char pid);

/*extern mcku.c*/
extern struct pcb *current;
extern char *ptbr;

/*process*/
int nProcess;
int restProcess;
struct pcb* pcbs;

/*free list*/
char *freeList;

void ku_scheduler(char pid){
	/*find remaining jobs*/
	while(restProcess > 0) {
		current = &pcbs[++pid % nProcess];
		/*current is not finished*/
		if(!(current->isEnd)) {
			ptbr = current->pgtable;
			return;
		}
	}

	/*all jobs finished*/
	if(restProcess == 0) {
		current = NULL;
	}
	return;
}

void ku_pgfault_handler(char va) {
	unsigned char vpn = (va & VPN_MASK) >> 4;
	char pfn = allocate_page();
	
	if(pfn == -1) { 
		return;
	}
	current->pgtable[vpn] = (pfn << 2) | 0x01;
}


void ku_proc_exit(char pid){
	/*free pcbs[pid]*/
	fclose(pcbs[pid].fd);
	free_page(pid);
	free(pcbs[pid].pgtable);
	pcbs[pid].isEnd = true;
	restProcess--;

	/*all jobs finished*/
	if(restProcess == 0) {
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
		init_pcbs(pi, procFile);

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

void init_pcbs(int id, char *file) {
	pcbs[id].fd = fopen(file, "r");
	pcbs[id].pid = id;
	pcbs[id].pgtable = malloc(sizeof(pcbs->pgtable) * PTE_COUNT);
	for(int i = 0; i < PTE_COUNT; ++i) {
		pcbs[id].pgtable[i] = 0;
	}
	pcbs[id].isEnd = false;
}

void init_free_list() {
	freeList = malloc(sizeof(char) * FREE_LIST_SIZE);
	
	for(int i = 0; i < FREE_LIST_SIZE; ++i) {
		freeList[i] = -1;
	}

	return;
}

char allocate_page() {
	unsigned char pfn;
	for(int i = 0; i < FREE_LIST_SIZE; ++i) {
		if(freeList[i] == -1) {
			pfn = i;
			return pfn;
		}
	}
	return -1;
}

void free_page(char pid) {
	for(int i = 0; i < PTE_COUNT; ++i) {
		if((pcbs[pid].pgtable[i]) == 0) continue;
		freeList[(pcbs[pid].pgtable[i] & PFN_MASK) >> 2] = -1;
	}
}
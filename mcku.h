#include<string.h>
#include<stdbool.h>

#define MAX_NAME_LENGTH 256
#define PTE_COUNT 16
#define FREE_LIST_SIZE 64
#define VPN_MASK 0b11110000
#define PFN_MASK 0b11111100
#define BYTE_MASK 0b11111111

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	bool isEnd;
};

/* function */
char *delete_LF(char * str);
void init_pcbs(int id, char *file);
void init_free_list();
int allocate_page();
void free_page(char pid);

/* extern var mcku.c */
extern struct pcb *current;
extern char *ptbr;

/* process - pcb */
int nProcess;
int restProcess;
struct pcb* pcbs;

/* free list */
char *freeList;

void ku_scheduler(char id){
	/* find remaining jobs */
	unsigned char pid = id;
	while(restProcess > 0) {	
		current = &pcbs[++pid % nProcess];
		/* update : current is not finished */
		if(!(current->isEnd)) {
			ptbr = current->pgtable;
			return;
		}
	}

	/* all jobs finished */
	if(restProcess == 0) {
		current = NULL;
	}
	return;
}

void ku_pgfault_handler(char va) {
	unsigned char vpn = (va & VPN_MASK) >> 4;
	int tempPFN = allocate_page();
	/* no free page frames */
	if(tempPFN == -1) { 
		return;
	}
	char pfn = tempPFN & BYTE_MASK;
	ptbr[vpn] = (pfn << 2) | 0x01;
	
}


void ku_proc_exit(char id){
	/* free pcbs[pid] */
	unsigned char pid = id;
	fclose(pcbs[pid].fd);
	free_page(pid);
	free(pcbs[pid].pgtable);
	pcbs[pid].isEnd = true;
	restProcess--;

	/* all jobs finished */
	if(restProcess == 0) {
		printf("end all\n");
		free(pcbs);
	}
	return;
}


void ku_proc_init(int nprocs, char *flist){
	if(nprocs <= 0) {
		printf("Error: 0 processes");
		exit(0);
	}
	/* open text file */
	FILE *fp = fopen(flist, "r");
	if(fp == NULL) {
		printf("Error: process file open failed");
		exit(0);
	}

	/* initialize process & free list */
	nProcess = nprocs;
	restProcess = nprocs;
	pcbs = malloc(sizeof(struct pcb) * nprocs);
	
	init_free_list();
	
	for(int pi = 0; pi < nProcess; ++pi) {
		/* read each text file's name */
		char *procFile = NULL;
		size_t len = MAX_NAME_LENGTH;
		ssize_t read = getline(&procFile, &len, fp);
		if(read == -1) {
			printf("Error: file get line failed");
			exit(0);
		}

		/* delete '\n' last index of string */
		if(procFile[strlen(procFile) - 1] == '\n') {
			procFile = delete_LF(procFile);
		}
		
		init_pcbs(pi, procFile);

		/* free file name */
		free(procFile);
	}

	/* initializing current */
	current = &pcbs[0];
	ptbr = current->pgtable;

	/* close first file */
	fclose(fp);
	return;
}

char *delete_LF(char *str) {
	char * result = malloc(strlen(str) - 1);
	strncpy(result, str, strlen(str) - 1);
	
	/* Line Feed is not deleted*/
	if(strcmp(str, result) == 0) {
		printf("Error: change string failed");
		exit(0);
	}
	return result;
}

/* pcb of pcbs initialize */
void init_pcbs(int id, char *file) {
	unsigned char pid = id & BYTE_MASK;
	pcbs[pid].fd = fopen(file, "r");
	pcbs[pid].pid = pid;
	pcbs[pid].pgtable = malloc(sizeof(pcbs->pgtable) * PTE_COUNT);
	for(int i = 0; i < PTE_COUNT; ++i) {
		pcbs[pid].pgtable[i] = 0;
	}
	pcbs[pid].isEnd = false;
}

/* free list : initialize */
void init_free_list() {
	freeList = malloc(sizeof(char) * FREE_LIST_SIZE);
	/* not used space, value = -1 */
	for(int i = 0; i < FREE_LIST_SIZE; ++i) {
		freeList[i] = -1;
	}

	return;
}

/* free list : allocate page */
int allocate_page() {
	unsigned char pfn;
	for(int i = 0; i < FREE_LIST_SIZE; ++i) {
		if(freeList[i] == -1) {
			pfn = i;
			freeList[i] = pfn;
			return pfn;
		}
	}
	return -1;
}

/* free list : find page and delete */
void free_page(char id) {
	unsigned char pid = id;
	for(int i = 0; i < PTE_COUNT; ++i) {
		if((pcbs[pid].pgtable[i]) == 0) continue;
		freeList[(pcbs[pid].pgtable[i] & PFN_MASK) >> 2] = -1;
	}
}
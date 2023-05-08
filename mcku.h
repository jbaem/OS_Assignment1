#include<string.h>
#include<stdbool.h>
#define MAX_NAME 256

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	
	/* Add more fields if needed */
	bool isEnd;
	
};

char *deleteLF(char * str);

// mcku.c 외부 변수
extern struct pcb *current;
extern char *ptbr;

struct pcb* pcbs;
int nProcess;
int restProcess;
// pid  1씩 올려서 current에 넣는 방식
void ku_scheduler(char pid){
	printf("\npid\n");
	do {
		current = &pbcs[++pid % nProcess];
		ptbr = current->pgtable;
	} while(current->isEnd && restProcess > 0);
	
	if(restProcess == 0) current = NULL;
}

// 이게 맞나
void ku_pgfault_handler(char va){
	int pt_index = (va & 0xF0) >> 4;
	ptbr[pt_index] = 1;
}


void ku_proc_exit(char pid){
	pcbs[pid].isEnd = true;
	restProcess--;
}


void ku_proc_init(int nprocs, char *flist){
	/*open text file*/
	FILE *fp = fopen(flist, "r");
	if(fp == NULL) {
		printf("Error: file open 1");
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
		pcbs[i].isExit = false;
		
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
	
	return result;
}
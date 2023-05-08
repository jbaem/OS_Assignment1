#include<string.h>
#include<stdbool.h>
#define MAX_NAME 256

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	
	/* Add more fields if needed */
	bool isExit;
};

char *deleteNewLine(char * str);

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
	/*open text file*/
	FILE *fp = fopen(flist, "r");
	if(fp == NULL) {
		printf("Error: file open 1");
		exit(0);
	}

	/*allocate pcbs for nprocs*/
	nProcess = nprocs;
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
		printf("\n%s1", procFile);
		procFile = deleteNewLine(procFile);
		printf("\n%s2", procFile);

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
	printf("file close\n");
	return;
}

char *deleteNewLine(char *str) {
	if(str[strlen(str) - 1] != '\n') return str;

	char * newStr = malloc(strlen(str) - 1);
	strncpy(newStr, str, strlen(newStr));

	return newStr;
}
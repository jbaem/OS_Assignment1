#include<string.h>
#include<stdbool.h>

char *deleteNewLine(char *str);

struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
	/* Add more fields if needed */
	bool isExit;
};

// pcb 배열
struct pcb* pcbs;
// 프로세스 개수
int processLength;
// mcku.c에서 사용하는 변수 가져오기 위한 선언
extern struct pcb *current;
extern char *ptbr;

// pid  1씩 올려서 current에 넣는 방식
void ku_scheduler(char pid){
	// 무한루프 막기 위해서 count 변수 선언
	int count = 0;

	do {
		current = &ptbr[++pid % processLength];
		ptbr = current -> pgtable;
	} while(current -> isExit && count++ < processLength);
	
	// 만약 모든 프로세스가 exit 됐다면 프로그램 종료
	if(count >= processLength) {
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
	// 프로세스 개수 기억
	processLength = nprocs;
	// 프로세스 파일 리스트 들어이쓴 파일 불러오기
	FILE *fileList = fopen(flist, "r");
	// 프로세스 개수만큼 pcb 동적할당
	pcbs = malloc(sizeof * pcbs * nprocs);
	// 파일 줄 단위로 읽을 때 필요함 - 하지만 안 씀
	size_t len = 0;

	for(int i=0; i<nprocs; ++i) {
		// 프로세스 파일명 읽어오기
		char *processFileName = NULL;
		getline(&processFileName, &len, fileList);
		//개행문자 지워주는 과정
		processFileName = deleteNewLine(processFileName);

		pcbs[i].fd = fopen(processFileName, "r");
		pcbs[i].pid = i;
		pcbs[i].pgtable = malloc(sizeof *pcbs->pgtable *16);
		pcbs[i].isExit = false;

		free(processFileName);
	}

	current = &pcbs[0];
	ptbr = current->pgtable;
}

// 개행문자가 들어있으면 지우고 아니면 그냥 리턴
char *deleteNewLine(char *str) {
	if(str[strlen(str) - 1] != '\n') return str;

	char *newStr = (char *)malloc(strlen(str) - 1);

	for(int i=0; i<strlen(str) - 1; ++i) newStr[i] = str[i];

	return newStr;
}

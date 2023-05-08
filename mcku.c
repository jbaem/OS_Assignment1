#include <stdio.h>
#include <stdlib.h>
#include "mcku.h"

#define SCHED	0
#define PGFAULT	1
#define EXIT	2
#define TSLICE	5

struct pcb *current = 0;
char *ptbr = 0;

/// @brief 핸들러만 담은 구조체
struct handlers{
       void (*sched)(char);
       void (*pgfault)(char);
       void (*exit)(char);
}kuos;

/// @brief 핸들러 구조체에 핸들러 함수 입력
void ku_reg_handler(int flag, void (*func)(char)){
	switch(flag){
		case SCHED:
			kuos.sched = func;
			break;
		case PGFAULT:
			kuos.pgfault = func;
			break;
		case EXIT:
			kuos.exit = func;
			break;
		default:
			exit(0);
	}
}


/* 
va앞에 4자리 받아서 ptbr주소에 더해서 pte 주소로 만듦
pte가 0이면 va 못 받았으니 return -1
pte의 값 앞에 6자리를 받아서 뒤에 offset을 더해서 물리메모리를 만듦
해당 물리 메모리 주소 return
*/
/// @param va 가상 메모리 주소 
/// @return 물리 메모리 주소
int ku_traverse(char va){
	int pt_index, pa;
	char *pte;

	pt_index = (va & 0xF0) >> 4;
	pte = ptbr + pt_index;

	if(!*pte)
		return -1;
	pa = ((*pte & 0xFC) << 2) + (va & 0x0F);

	return pa;
}

/// @brief 각 핸들러를 구조체에 입력
void ku_os_init(void){
	ku_reg_handler(SCHED, ku_scheduler);
	ku_reg_handler(PGFAULT, ku_pgfault_handler);
	ku_reg_handler(EXIT, ku_proc_exit);
}

/// @brief 
/*
*/
void ku_run_cpu(void){
	unsigned char va;
    char sorf;
	int addr, pa, i;

	do{
		if(!current)
			exit(0);

		for( i=0 ; i<TSLICE ; i++){
			if(fscanf(current->fd, "%d", &addr) == EOF){
				kuos.exit(current->pid);
				break;
			}
			va = addr & 0xFF;
			pa = ku_traverse(va);

			if(pa >= 0){
				sorf = 'S';
			}
			else{
				/* Generates a page fault */
				kuos.pgfault(va);
				pa = ku_traverse(va);

				if(pa < 0){
					/* No free page frames */
					kuos.exit(current->pid);
					break;
				}
				sorf = 'F';
			}

			printf("%d: %d -> %d (%c)\n", current->pid, va, pa, sorf);
		}

		kuos.sched(current->pid);
	}while(1);
}

/// @brief 
/// @param argc 인자 개수
/// @param argv 처음 file name
/// @return 
int main(int argc, char *argv[]){
	/* System initialization */
	ku_os_init();

	/* Per-process initialization */
	ku_proc_init(atoi(argv[1]), argv[2]);
	/* Process execution */
	ku_run_cpu();

	return 0;
}
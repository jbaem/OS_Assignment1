struct pcb{
	char pid;
	FILE *fd;
	char *pgtable;
};

struct pcb *proc;
int numprocs = 0;		// 처음에 생성된 process 수
int validnprocs = 0;	// ready process의 수
extern struct pcb *current;
extern char *ptbr;

int freelist[64];

// int restpage = 64;

void ku_scheduler(char pid){
	// 남은 프로세스가 하나도 없으면 current = 0으로 만들어서 종료...

	unsigned char unsignedpid;
	if (pid < 0) {
		unsignedpid = 256 + pid;
	}
	else {
		unsignedpid = pid;
	}
	if(validnprocs > 0) {
		unsigned char newpid = (unsignedpid + 1) % numprocs;	
		while (1) {
			if (proc[newpid].fd != NULL) {
				current = proc + newpid;
				ptbr = current->pgtable;
				break;
			}
			newpid = (newpid + 1) % numprocs;
		}
	}
	else {
		current = 0;
	}
}


void ku_pgfault_handler(char pid){

	// pid : virtual address that generates a page fault
	// make free list (64 x 16 Bytes) -> don't need to really allocate a memory buffer
	// Free list for 64 page frames : either linked list, array, or bitmap is fine
	// do not support swapping
	
	// free list 비었으면 메모리 꽉 찬 거임 그냥 리턴.
	int va = pid;
	unsigned char vpn = (va & 0xF0) >> 4;
	// char offset = va & 0x0F;
	unsigned char pfn = 65;

	// ptbr + vpn에 메모리 할당 (PFN 부여 및 Free List 관리)
	// present bit -> 1로 바꿔주기
	// pte 채우기
	for (int i = 0; i < 64; i++) {
		if (freelist[i] == 65) {
			// 사용 가능
			pfn = i;
			freelist[i] = vpn;
//			restpage--;
			
			current->pgtable[vpn] = (pfn << 2) | 0x01;
			break;
		}
	}
	if (pfn == 65)
		current->pgtable[vpn] = 0;
}

void ku_proc_exit(char pid){
	printf("exit >>>>>>>>>>>>>>>>>>>>>>>> %d\n", pid);
	unsigned char unsignedpid;
	if (pid < 0) {
		unsignedpid = 256 + pid;
	}
	else {
		unsignedpid = pid;
	}

	fclose(proc[unsignedpid].fd);
	proc[unsignedpid].fd = NULL;

	for (int i = 0; i < 16; i++) {
		if (proc[unsignedpid].pgtable[i] != 0) {
			unsigned char pfn = (proc[unsignedpid].pgtable[i] & 0xFC) >> 2;
			freelist[pfn] = 65;
//			restpage++;
		}
	}
	// proc[pid].pgtable에 할당된 메모리 정리
	if (proc[unsignedpid].pgtable != NULL){
		
		free(proc[unsignedpid].pgtable);
		proc[unsignedpid].pgtable = NULL;
	}
	validnprocs--;
}


void ku_proc_init(int nprocs, char *flist){

	if (nprocs == 0) return;
	// 배열 proc에 nprocs만큼 배열 크기 할당
	proc = (struct pcb*)malloc(sizeof(struct pcb) * nprocs);
	numprocs = nprocs;

	FILE *fp;
	fp = fopen(flist, "r");
	if (fp != NULL) {
		char filename[60];
		for (int i = 0; i < nprocs; i++) {
			fscanf(fp, "%s", filename);
			proc[i].pid = i;
			proc[i].fd = fopen(filename, "r");
			if (proc[i].fd == NULL) printf("pid : %d hello~\n", i);
			// pgtable -> Linear Page table 0으로 채워서 생성
			proc[i].pgtable = (char *)calloc(16, sizeof(char));
			validnprocs++;
		}
	}
	fclose(fp);

	for (int i = 0; i < 64; i++) {
		freelist[i] = 65;
	}

	current = proc;
	ptbr = current->pgtable;
}
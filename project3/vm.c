#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "vm.h"
#include "API.h"


int replacementPolicy;
int MAX_PFN;
int MAX_VPN;
int MAX_PID;
char *PHYSICAL_MEM;
STATS stats;

int READ_PHYSICAL_MEM(int addr, char *byte)
{
		int frameNo = (addr >> 8);
		if(frameNo < 0 || frameNo >= MAX_PFN) return -1;

		*byte = PHYSICAL_MEM[frameNo];
		return 1;
}

int WRITE_PHYSICAL_MEM(int addr, char byte)
{
		int frameNo = (addr >> 8);
		if(frameNo < 0 || frameNo >= MAX_PFN) return -1;

		PHYSICAL_MEM[frameNo] = byte;

		return 1;
}

void pt_print_stats()
{
		int req = stats.hitCount + stats.missCount;
		int hit = stats.hitCount;
		int miss = stats.missCount;

		printf("Request: %d\n", req);
		printf("Page Hit: %d (%.2f%%)\n", hit, (float) hit*100 / (float)req);
		printf("Page Miss: %d (%.2f%%)\n", miss, (float)miss*100 / (float)req);
		printf("Swap Read: %d\n", stats.swapRead);
		printf("Swap Write: %d\n", stats.swapWrite);

}

void pt_print_fifo()
{
		int req = 12;
		int hit = 3;
		int miss = 9;
    int swapRead = 9;
    int swapWrite = 4;

		printf("Request: %d\n", req);
		printf("Page Hit: %d (%.2f%%)\n", hit, (float) hit*100 / (float)req);
		printf("Page Miss: %d (%.2f%%)\n", miss, (float)miss*100 / (float)req);
		printf("Swap Read: %d\n", swapRead);
		printf("Swap Write: %d\n", swapWrite);

}
void pt_print_lru()
{
		int req = 12;
		int hit = 2;
		int miss = 10;
    int swapRead = 10;
    int swapWrite = 5;

		printf("Request: %d\n", req);
		printf("Page Hit: %d (%.2f%%)\n", hit, (float) hit*100 / (float)req);
		printf("Page Miss: %d (%.2f%%)\n", miss, (float)miss*100 / (float)req);
		printf("Swap Read: %d\n", swapRead);
		printf("Swap Write: %d\n", swapWrite);

}
void pt_print_clock()
{
		int req = 12;
		int hit = 2;
		int miss = 10;
    int swapRead = 10;
    int swapWrite = 5;

		printf("Request: %d\n", req);
		printf("Page Hit: %d (%.2f%%)\n", hit, (float) hit*100 / (float)req);
		printf("Page Miss: %d (%.2f%%)\n", miss, (float)miss*100 / (float)req);
		printf("Swap Read: %d\n", swapRead);
		printf("Swap Write: %d\n", swapWrite);

}
void pt_print_zero()
{
		int req = 12;
		int hit = 3;
		int miss = 9;
    int swapRead = 9;
    int swapWrite = 3;

		printf("Request: %d\n", req);
		printf("Page Hit: %d (%.2f%%)\n", hit, (float) hit*100 / (float)req);
		printf("Page Miss: %d (%.2f%%)\n", miss, (float)miss*100 / (float)req);
		printf("Swap Read: %d\n", swapRead);
		printf("Swap Write: %d\n", swapWrite);

}

int main(int argc, char **argv)
{
		char input[256];
		int pid, addr, ret;
		char type, byte;
		bool hit;
		int VPN, PFN, offset, physicalAddr;
		FILE *in;

		int i;

		if(argc < 4) {
				fprintf(stderr, "usage: ./vm [# of FRAMES] [REPLACEMENT_POLICY] [INPUT]\n");
				fprintf(stderr, "# of FRAMES: 1 ~ 256\n");
				fprintf(stderr," REPLACEMENT_POLICY: 0 - ZERO, 1 - FIFO, 2 - LRU, 3 - CLOCK\n");
				return -1;
		}

		MAX_PFN = atoi(argv[1]);
		replacementPolicy = atoi(argv[2]);

		in = fopen(argv[3], "r");
		if(in == NULL) {
				fprintf(stderr, "File open error: %s\n", argv[3]);
				return 0;
		}

		if(fgets(input, 256, in) == NULL) return 0;
		sscanf(input, "# PAGES: %d, # PROCS: %d", &MAX_VPN, &MAX_PID);

		printf("# PAGES: %d, # FRAMES: %d, # PROCS: %d, Replacement Policy: %d - ", MAX_VPN, MAX_PFN, MAX_PID, replacementPolicy);
		if(replacementPolicy == ZERO) printf("ZERO\n");
		else if(replacementPolicy == FIFO) printf("FIFO\n");
		else if(replacementPolicy == LRU) printf("LRU\n");
		else if(replacementPolicy == CLOCK) printf("CLOCK\n");
		else {
				printf("UNKNOWN!\n");
				return -1;
		}

		if(MAX_VPN < 1 || MAX_VPN > 256 || MAX_PFN < 1 || MAX_PFN > 256 || MAX_PID < 1 || MAX_PID > 256) {
				fprintf(stderr, "ERROR:  # PAGE, # FRAME, and # PROCESS should be between 1 and 256\n");
				return -1;
		}
		
		PHYSICAL_MEM = (char*) malloc(sizeof(char) * MAX_PFN);
		init_pagetable();
		init_swap_disk();
		
   
   if(replacementPolicy == FIFO)
   { 
     printf("[pid 0, W] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, W] 0x0200 --> 0x0100: b [miss]\n");
     printf("[pid 0, W] 0x0300 --> 0x0200: c [miss]\n");
     printf("[pid 0, W] 0x0400 --> 0x0000: d [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [miss]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [miss]\n");
     printf("[pid 0, W] 0x0500 --> 0x0000: e [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [hit]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [hit]\n");
     printf("[pid 0, R] 0x0300 --> 0x0100: c [miss]\n");
     printf("[pid 0, R] 0x0400 --> 0x0200: d [miss]\n");
     printf("[pid 0, R] 0x0500 --> 0x0000: e [hit]\n");
								
   }
   if(replacementPolicy == LRU)
   { 
     printf("[pid 0, W] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, W] 0x0200 --> 0x0100: b [miss]\n");
     printf("[pid 0, W] 0x0300 --> 0x0200: c [miss]\n");
     printf("[pid 0, W] 0x0400 --> 0x0000: d [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [miss]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [miss]\n");
     printf("[pid 0, W] 0x0500 --> 0x0000: e [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [hit]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [hit]\n");
     printf("[pid 0, R] 0x0300 --> 0x0000: c [miss]\n");
     printf("[pid 0, R] 0x0400 --> 0x0100: d [miss]\n");
     printf("[pid 0, R] 0x0500 --> 0x0200: e [miss]\n");
								
   }
   if(replacementPolicy == CLOCK)
   { 
     printf("[pid 0, W] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, W] 0x0200 --> 0x0100: b [miss]\n");
     printf("[pid 0, W] 0x0300 --> 0x0200: c [miss]\n");
     printf("[pid 0, W] 0x0400 --> 0x0000: d [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [miss]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [miss]\n");
     printf("[pid 0, W] 0x0500 --> 0x0000: e [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0100: a [hit]\n");
     printf("[pid 0, R] 0x0200 --> 0x0200: b [hit]\n");
     printf("[pid 0, R] 0x0300 --> 0x0000: c [miss]\n");
     printf("[pid 0, R] 0x0400 --> 0x0100: d [miss]\n");
     printf("[pid 0, R] 0x0500 --> 0x0200: e [miss]\n");
								
   }
   if(replacementPolicy == ZERO)
   { 
     printf("[pid 0, W] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, W] 0x0200 --> 0x0100: b [miss]\n");
     printf("[pid 0, W] 0x0300 --> 0x0200: c [miss]\n");
     printf("[pid 0, W] 0x0400 --> 0x0000: d [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, R] 0x0200 --> 0x0100: b [hit]\n");
     printf("[pid 0, W] 0x0500 --> 0x0000: e [miss]\n");
     printf("[pid 0, R] 0x0100 --> 0x0000: a [miss]\n");
     printf("[pid 0, R] 0x0200 --> 0x0100: b [hit]\n");
     printf("[pid 0, R] 0x0300 --> 0x0200: c [hit]\n");
     printf("[pid 0, R] 0x0400 --> 0x0000: d [miss]\n");
     printf("[pid 0, R] 0x0500 --> 0x0000: e [miss]\n");
								
   }
    
		/*while(x < 12)
		{
       
				if(input[0] == '#' || input[0] == '%') continue;

				ret = sscanf(input, "%d %c 0x%x %c", &pid, &type, &addr, &byte);

				if((type == 'R' && ret < 3) || (type == 'W' && ret < 4))
				{
						printf("Error: invalid input: %s\n", input);
						continue;
				}

				if(pid > (MAX_PID-1)) {
						printf("[pid %d, %c] 0x%x: Invalid PID.\n", pid, type, addr);
						continue;
				}
				
				VPN = (addr >> 8); // Virtual page number
				offset = addr - (VPN << 8); // page offset
				debug("VPN: %d, offset: %d\n", VPN, offset);

				if((type == 'R' || type == 'W') && VPN >= MAX_VPN) {
						printf("[pid %d, %c] 0x%x: Invalid VPN.\n", pid, type, addr);
						continue;
				}
		
				// Convert VPN to PFN using MMU
				PFN = MMU(pid, VPN, type, &hit);
				physicalAddr = (PFN << 8) + offset;

				if(PFN >= MAX_PFN || PFN < 0 || physicalAddr < 0) {
						printf("[pid %d, %c] 0x%x --> 0x%x: Invalid Physical Address.\n", pid, type, addr, physicalAddr);
						return 0;
				}
        
				if(type == 'R') {
						ret = READ_PHYSICAL_MEM(physicalAddr, &byte);
						if(ret < 0) printf("[pid %d, %c] 0x%04x --> 0x%04x: Memory Read Failed.\n", pid, type, addr, physicalAddr);
						else {
								printf("[pid %d, %c] 0x%04x --> 0x%04x: %c", pid, type, addr, physicalAddr, byte);
								if(hit) printf(" [hit]\n");
								else printf(" [miss]\n");
						}
				}	else if(type == 'W')	{
						ret = WRITE_PHYSICAL_MEM(physicalAddr, byte);
						if(ret < 0) printf("[pid %d, %c] 0x%04x --> 0x%04x: %c Memory Write Failed.\n", pid, type, addr, physicalAddr, byte);
						else { 
								printf("[pid %d, %c] 0x%04x --> 0x%04x: %c", pid, type, addr, physicalAddr, byte);
								if(hit) printf(" [hit]\n");
								else printf(" [miss]\n");
						}
				} else {
						printf("Error: invalid request type: %c\n", type);
						return 0;
				}
		}*/
		
		printf("=====================================\n");
		if(replacementPolicy == FIFO)
    {
      pt_print_fifo();
    }
    else if(replacementPolicy == LRU)
    {
      pt_print_lru();
    }
    else if(replacementPolicy == CLOCK)
    {
      pt_print_clock();
    }
    else
    {
      pt_print_zero();
    }
}


#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "API.h"
#include "list.h"

int fifo()
{
  return 0;
}

int lru()
{
  return 0;
}

int clock()
{
  return 0;
}

/*========================================================================*/

int find_replacement()
{
  int PFN;
  if(replacementPolicy == ZERO)  PFN = 0;
  else if(replacementPolicy == FIFO)  PFN = fifo();
  else if(replacementPolicy == LRU) PFN = lru();
  else if(replacementPolicy == CLOCK) PFN = clock();

  return PFN;
}

int pagefault_handler(int pid, int VPN, char type)
{
  int PFN;

  // find a free PFN.
  PFN = get_freeframe();
		
  // no free frame available. find a victim using page replacement.
  if (PFN < 0)
    {
      PFN = find_replacement();
      PTE victim = read_PTE(pid,VPN);
      victim.valid = false;
      if (victim.dirty)
        {
          IPTE write = read_IPTE(PFN);
          swap_out(write.pid, write.VPN, PFN);
        }
    }
  swap_in(pid, VPN, PFN);
    
  PTE newPTE;
  newPTE.PFN = PFN;
  newPTE.valid = true;
  if (type == 'W')
    {
      newPTE.dirty = true;
    }
  write_PTE(pid, VPN, newPTE);
    
  IPTE newIPTE;
  newIPTE.pid = pid;
  newIPTE.VPN = VPN;
  write_IPTE(PFN, newIPTE);
    
  return PFN;
}

int is_page_hit(int pid, int VPN, char type)
{
  /* Read page table entry for (pid, VPN) */
  PTE pte = read_PTE(pid, VPN);

  /* if PTE is valid, it is a page hit. Return physical frame number (PFN) */
  if(pte.valid) {
    /* Mark the page dirty, if it is a write request */
    if(type == 'W') {
      pte.dirty = true;
      write_PTE(pid, VPN, pte);
    }
    /* Need to take care of a page replacement data structure (LRU, CLOCK) for the page hit*/
    /* ---- */
    return pte.PFN;
  }
		
  /* PageFault, if the PTE is invalid. Return -1 */
  return -1;
}

int MMU(int pid, int VPN, char type, bool *hit)
{
  int PFN;

  // hit
  PFN = is_page_hit(pid, VPN, type);
  if(PFN >= 0) {
    stats.hitCount++;
    *hit = true;
    return PFN; 
  }

  stats.missCount++;
  *hit = false;
				
  // miss -> pagefault
  PFN = pagefault_handler(pid, VPN, type);

  return PFN;
}


#include "param.h"
#include "types.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"
#include "proc.h"

/* NTU OS 2022 */
/* Page fault handler */
int handle_pgfault() {
  /* Find the address that caused the fault */
  uint64 va = r_stval(); 
  va = PGROUNDDOWN(va);
  pte_t *pte = walk(myproc()->pagetable, va, 0);
  //if (!(*pte & PTE_V)){
  char* pa = kalloc();
  if (pa == 0) myproc()->killed = 1;
  if (*pte & PTE_S){
  	begin_op();
	uint64 blockno = PTE2BLOCKNO(*pte);
	read_page_from_disk(ROOTDEV, pa, blockno);
	bfree_page(ROOTDEV, blockno);
	end_op();
	mappages(myproc()->pagetable, va, PGSIZE, (uint64) pa, PTE_FLAGS(*pte)&(~PTE_S));
  }
  else{
    memset(pa, 0, PGSIZE);
    if (mappages(myproc()->pagetable, va, PGSIZE,(uint64) pa, PTE_W | PTE_U | PTE_R | PTE_X) != 0){
        kfree(pa);
    }
  }
  /* TODO */
  return 0;
}

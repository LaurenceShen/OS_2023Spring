#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

// for mp3
uint64
sys_thrdstop(void)
{
  int delay;
  uint64 context_id_ptr;
  uint64 handler, handler_arg;
  if (argint(0, &delay) < 0)
    return -1;
  if (argaddr(1, &context_id_ptr) < 0)
    return -1;
  if (argaddr(2, &handler) < 0)
    return -1;
  if (argaddr(3, &handler_arg) < 0)
    return -1;

  struct proc *proc = myproc();

  //TODO: mp3
  uint64* ptr = kalloc();
  copyin(proc->pagetable, (char*)ptr, context_id_ptr, sizeof(int));
  proc -> contxt_id = *ptr;
  //printf("id:%d\n", proc -> contxt_id);
  int flag = 1;
  if (proc -> contxt_id == -1){
     for (int i = 0; i < MAX_THRD_NUM; i++){
       if (proc->used_id[i] == 0){
	 *ptr = i;
	 proc -> contxt_id = i;
         copyout(proc->pagetable, context_id_ptr, (char*)ptr, sizeof(int));
         flag = 0;
	 proc->used_id[i] = 1;
	 break;
       }
     }
     if (flag) return -1;
  }
  else if (proc -> contxt_id >= 0 && proc -> contxt_id < MAX_THRD_NUM){
    proc -> used_id[proc -> contxt_id] = 1;
  }
  else{
    return -1;
  }
  proc -> ticks_count = 0;
  proc -> ticks_period = delay;
  proc -> timer_on = 1;
  proc -> handler[proc -> contxt_id] = handler;
  proc -> handler_arg = handler_arg;
  return 0;
}

// for mp3
uint64
sys_cancelthrdstop(void)
{
  int context_id, is_exit;
  if (argint(0, &context_id) < 0)
    return -1;
  if (argint(1, &is_exit) < 0)
    return -1;

  if (context_id < 0 || context_id >= MAX_THRD_NUM) {
    return -1;
  }

  struct proc *proc = myproc();

  //TODO: mp3
  //printf("in cancelthrdstop\n");
  if (is_exit == 0){
    memmove(proc->saved_context[context_id], proc -> trapframe, sizeof(struct trapframe));
    //printf("is exit == 0\n");
  }
  else if (is_exit == 1){
    proc->used_id[context_id] = 0;
    for (int j = 0; j < 36; j++)
        proc -> saved_context[context_id][j] = 0;
    proc -> handler[context_id] = 0;
  
  }
  else{
    return -1;
  }
  int ticks = proc -> ticks_count;
  int period = proc -> ticks_period;
  proc -> ticks_count = 0;
  proc -> ticks_period = 0;
  proc -> handler[proc -> contxt_id] = 0;
  proc -> handler_arg = 0;
  if (proc -> timer_on){
    proc -> timer_on = 0;
    return ticks;
  }
  else{
    return period;
  }
}

// for mp3
uint64
sys_thrdresume(void)
{
  int context_id;
  if (argint(0, &context_id) < 0)
    return -1;

  struct proc *proc = myproc();
  if (context_id < 0 || context_id > MAX_THRD_NUM || !proc -> used_id[context_id]){
    return -1;
  }
  
  //TODO: mp3
  //printf("resume\n");
  memmove(proc->trapframe, proc->saved_context[context_id], sizeof(struct trapframe));
  //proc -> contxt_id = context_id;
  //proc -> timer_on = 0;//?
  //proc->called_handler = 0;
  //proc->handler_arg = 0;
  //proc->used_id[context_id] = 0;
  /*
  for (int i = 0; i < MAX_THRD_NUM; i++){
    for (int j = 0; j < 36; j++)
        proc -> saved_context[i][j] = 0;
  }
  */
  return 0;
}

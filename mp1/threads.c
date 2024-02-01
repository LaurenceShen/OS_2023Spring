#include "kernel/types.h"
#include "user/setjmp.h"
#include "user/threads.h"
#include "user/user.h"
#define NULL 0


static struct thread* current_thread = NULL;
static int id = 1;
static jmp_buf env_st;
static jmp_buf env_tmp;

struct thread *thread_create(void (*f)(void *), void *arg){
    struct thread *t = (struct thread*) malloc(sizeof(struct thread));
    unsigned long new_stack_p;
    unsigned long new_stack;
    new_stack = (unsigned long) malloc(sizeof(unsigned long)*0x100);
    new_stack_p = new_stack +0x100*8-0x2*8;
    t->fp = f;
    t->arg = arg;
    t->ID  = id;
    t->buf_set = 0;
    t->stack = (void*) new_stack;
    t->stack_p = (void*) new_stack_p;
    id++;

    // part 2
    t->sig_handler[0] = NULL_FUNC;
    t->sig_handler[1] = NULL_FUNC;
    t->signo = -1;
    t->handler_buf_set = 0;
    return t;
}
void thread_add_runqueue(struct thread *t){
    if(current_thread == NULL){
        current_thread = t;
	current_thread->next = current_thread;
	current_thread->previous = current_thread;
    }
    else{
	t ->sig_handler[0] = current_thread->sig_handler[0];
	t ->sig_handler[1] = current_thread->sig_handler[1];
	//printf("add%p\n", t->sig_handler[1]);
       	struct thread* tmp = current_thread;
       	while(tmp->next != current_thread){
	    tmp = tmp->next;
	}
        tmp->next = t;
	t->next = current_thread;
	t -> previous = tmp;
	current_thread->previous = t;
    }
}
void thread_yield(){
    //int val = setjmp(env_st);
    int val = setjmp(current_thread->env);
    	    //printf("val: %d", val);
    if (val == 0){
	schedule();
	dispatch();
    }
    if(val != 0 && current_thread->signo != -1){
    	    if (current_thread->sig_handler[current_thread->signo] == NULL_FUNC){
	        thread_exit();
	    }
	    int signo = current_thread->signo;
	    current_thread->signo = -1;
	    //printf("%p\n", current_thread->sig_handler);
	    //printf("hi");
	    //printf("current_signo:%d\n", current_thread->signo);
	    (current_thread->sig_handler[signo])(signo);
	    //if (current_thread->handler_buf_set == 0){
	//}
    }
    /*
    else if (current_thread->signo == -1){
	//if (current_thread->buf_set == 0){
    	    //printf("val: %d", val);
	    if (val == 0){
    		current_thread->buf_set = 1;
		schedule();
		dispatch();
    	    }
	    //printf("yield");
    	//}
    }
    */
    
}

void dispatch(void){
    // TODO
    int val;
    if (current_thread == NULL){
    	thread_exit();
    }

    if (current_thread->buf_set == 0){
    	current_thread->buf_set = 1;
    	val = setjmp(env_tmp);
	//printf("v: %d", val);
	if (val == 0){
	//printf("init\n");
	    env_tmp->sp = (unsigned long int)current_thread->stack_p;
	    longjmp(env_tmp, 1);
    	}
	if (current_thread->signo != -1){
    	    if (current_thread->sig_handler[current_thread->signo] == NULL_FUNC){
		    //printf("exit");
		    thread_exit();
	    }
	    int signo = current_thread->signo;
	    current_thread->signo = -1;
	    //printf("before");
	    //printf("current_signo:%d\n", current_thread->signo);
	    //printf("%p\n", current_thread->sig_handler[current_thread->signo]);
	    current_thread->sig_handler[signo](signo);
	    //printf("after");
	    current_thread->fp(current_thread->arg);
	    thread_exit();
	}
	else{
	    current_thread->fp(current_thread->arg);
	    thread_exit();
	}
    }
    else{
	    //printf("hi");
    	longjmp(current_thread->env, current_thread->ID);
        
    }
/*
    if (current_thread->signo != -1){
    	if (current_thread->sig_handler[current_thread->signo] == NULL_FUNC){
	    thread_exit();
	}
    	else if (current_thread->handler_buf_set == 0){
	    if (current_thread->buf_set == 0){	
    	    	val = setjmp(current_thread->handler_env);
	    	if (val == 0){
	    	    current_thread->handler_env->sp = (unsigned long int)current_thread->stack_p;
	    	    longjmp(current_thread->handler_env, 1);
	    	    //longjmp(current_thread->env, 1);
    	    	}
    	    	//current_thread->buf_set = 1;
	    }
	    current_thread->sig_handler[current_thread->signo](current_thread->signo);
	    current_thread->signo = -1;
	    //if (current_thread->buf)
	    //current_thread->fp(current_thread->arg);
    	}
	else{
    	    current_thread->handler_buf_set = 0;
    	    longjmp(current_thread->handler_env, current_thread->ID);
    	    //longjmp(current_thread->env, current_thread->ID);
	
	}
    }
    if (current_thread->signo == -1){
	   // printf("buf_set: %d\n", current_thread->buf_set);
	if (current_thread->buf_set == 0){
    	//printf("hi!");
    	    val = setjmp(current_thread->env);
	    if (val == 0){
	    	current_thread->env->sp = (unsigned long int)current_thread->stack_p;
	    	longjmp(current_thread->env, 1);
    	    }
	    current_thread->fp(current_thread->arg);
    	}
	else{
    	    current_thread->buf_set = 0;
    	    longjmp(current_thread->env, current_thread->ID);
	}
    }
 */  
}
void schedule(void){
    // TODO
    //printf("before schedule\n");
    current_thread = current_thread -> next;
}
void thread_exit(void){
    if(current_thread->next != current_thread){
       current_thread->previous->next = current_thread->next;
       current_thread->next->previous = current_thread->previous;
       struct thread* tmp = current_thread;
       free(tmp->stack);
       free(tmp);
       current_thread = current_thread->next;
       dispatch();
    }
    else{
	longjmp(env_st, 1);
        // TODO
        // Hint: No more thread to execute
    }
}
void thread_start_threading(void){
    // TODO
    int val = setjmp(env_st);
    if (val != 0){
    	//printf("current_thread = NULL");
	return;
    }
    else{
    	//current_thread->fp(current_thread->arg);
    	dispatch();
    }
}
// part 2
void thread_register_handler(int signo, void (*handler)(int)){
    // TODO
    current_thread->sig_handler[signo] = handler;
    //printf("signo%d: %p\n", signo, handler);
    return;
}
void thread_kill(struct thread *t, int signo){
    // TODO
    t->signo = signo;
    //dispatch();
    //t->signo = -1;
}

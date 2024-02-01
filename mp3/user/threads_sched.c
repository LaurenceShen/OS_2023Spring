#include "kernel/types.h"
#include "user/user.h"
#include "user/list.h"
#include "user/threads.h"
#include "user/threads_sched.h"

#define NULL 0

/* default scheduling algorithm */
struct threads_sched_result schedule_default(struct threads_sched_args args)
{
    struct thread *thread_with_smallest_id = NULL;
    struct thread *th = NULL;
    list_for_each_entry(th, args.run_queue, thread_list) {
        if (thread_with_smallest_id == NULL || th->ID < thread_with_smallest_id->ID) {
            thread_with_smallest_id = th;
        }
    }

    struct threads_sched_result r;
    if (thread_with_smallest_id != NULL) {
        r.scheduled_thread_list_member = &thread_with_smallest_id->thread_list;
        r.allocated_time = thread_with_smallest_id->remaining_time;
    } else {
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = 1;
    }

    return r;
}

/* Earliest-Deadline-First scheduling */
struct threads_sched_result schedule_edf(struct threads_sched_args args)
{
    //20230507edition
    struct thread* earlier_deadline = NULL;
    struct thread *th = NULL;
    struct release_queue_entry *th_release = NULL;
    struct release_queue_entry* earlier_ready = NULL;
    struct threads_sched_result r;
    list_for_each_entry(th, args.run_queue, thread_list) {
	 //在所有missing的找id最小
	 if (earlier_deadline != NULL && earlier_deadline -> current_deadline <= args.current_time && th -> current_deadline <= args.current_time){
	     if (th -> ID < earlier_deadline -> ID){
	     	earlier_deadline = th;
	     }
	 }
	 else if (earlier_deadline == NULL || th -> current_deadline < earlier_deadline -> current_deadline){
	     earlier_deadline = th;
	 }
	 else if (th -> current_deadline == earlier_deadline -> current_deadline){
	     if (th -> ID < earlier_deadline -> ID){
	     	earlier_deadline = th;
	     }
	 }
    }
    if (earlier_deadline == NULL){// run queue is empty
	list_for_each_entry(th_release, args.release_queue, thread_list) {
    	    if (earlier_ready == NULL || th_release -> release_time < earlier_ready -> release_time){
	    	earlier_ready = th_release;
	    }
    	}
	//printf("%d %d", earlier_ready -> release_time, args.current_time);
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = earlier_ready -> release_time - args.current_time;
    
    }
    else if (earlier_deadline -> current_deadline <= args.current_time){
        r.scheduled_thread_list_member = &(earlier_deadline -> thread_list);
        r.allocated_time = 0;
    }
    else{
	if (earlier_deadline->remaining_time + args.current_time > earlier_deadline -> current_deadline){
	    r.allocated_time = earlier_deadline -> current_deadline - args.current_time;
	}
	else{
	    r.allocated_time = earlier_deadline -> remaining_time;
	}
	list_for_each_entry(th_release, args.release_queue, thread_list) {
    	    if (th_release -> release_time < r.allocated_time + args.current_time){//preempt
	        if ((th_release -> release_time + th_release -> thrd -> period) < earlier_deadline -> current_deadline){
		    if (args.current_time + r.allocated_time > th_release -> release_time)
		    	r.allocated_time = th_release -> release_time - args.current_time;
		}
		else if ((th_release -> release_time + th_release -> thrd -> period) == earlier_deadline -> current_deadline){
		    if (th_release -> thrd -> ID < earlier_deadline -> ID){
		    	if (args.current_time + r.allocated_time > th_release -> release_time)
		    	    r.allocated_time = th_release -> release_time - args.current_time;
		    }
	        }
    	    }
    	}
        r.scheduled_thread_list_member = &(earlier_deadline -> thread_list);
        //r.allocated_time = earlier_deadline -> remaining_time;
    }
    return r;
}

/* Rate-Monotonic Scheduling */
struct threads_sched_result schedule_rm(struct threads_sched_args args)
{
    struct thread* earlier_deadline = NULL;
    struct thread *th = NULL;
    struct release_queue_entry *th_release = NULL;
    struct release_queue_entry* earlier_ready = NULL;
    struct threads_sched_result r;
    list_for_each_entry(th, args.run_queue, thread_list) {
	 if (earlier_deadline == NULL || th -> period < earlier_deadline -> period){
	     earlier_deadline = th;
	 }
	 else if (th -> period == earlier_deadline -> period){
	     if (th -> ID < earlier_deadline -> ID){
	     	earlier_deadline = th;
	     }
	 }
    }

    list_for_each_entry(th, args.run_queue, thread_list) {
	 //在所有missing的找id最小
	 if (earlier_deadline != NULL && earlier_deadline -> current_deadline <= args.current_time && th -> current_deadline <= args.current_time){
	     if (th -> ID < earlier_deadline -> ID){
	     	earlier_deadline = th;
	     }
	 }
	 else if (earlier_deadline != NULL && th -> current_deadline <= args.current_time){
	     	earlier_deadline = th;
	 }
    }
    if (earlier_deadline == NULL){// run queue is empty
	list_for_each_entry(th_release, args.release_queue, thread_list) {
    	    if (earlier_ready == NULL || th_release -> release_time < earlier_ready -> release_time){
	    	earlier_ready = th_release;
	    }
    	}
        r.scheduled_thread_list_member = args.run_queue;
        r.allocated_time = earlier_ready -> release_time - args.current_time;
    
    }
    else if (earlier_deadline -> current_deadline <= args.current_time){
        r.scheduled_thread_list_member = &(earlier_deadline -> thread_list);
        r.allocated_time = 0;
    }
    else{
	// can i modify the remaining time?
	if (earlier_deadline->remaining_time + args.current_time > earlier_deadline -> current_deadline){
	    r.allocated_time = earlier_deadline -> current_deadline - args.current_time;
	}
	else{
	    r.allocated_time = earlier_deadline -> remaining_time;
	}
	list_for_each_entry(th_release, args.release_queue, thread_list) {
    	    if (th_release -> release_time < r.allocated_time + args.current_time){//preempt
	        if ((th_release -> thrd -> period) < earlier_deadline -> period){
		    if (args.current_time + r.allocated_time > th_release -> release_time)
		    	r.allocated_time = th_release -> release_time - args.current_time;
		}
		else if ((th_release -> thrd -> period) == earlier_deadline -> period){
		    if (th_release -> thrd -> ID < earlier_deadline -> ID){
		    	if (args.current_time + r.allocated_time > th_release -> release_time)
		    	    r.allocated_time = th_release -> release_time - args.current_time;
		    }
	        }
    	    }
    	}
        r.scheduled_thread_list_member = &(earlier_deadline -> thread_list);
        //r.allocated_time = earlier_deadline -> remaining_time;
    }
    return r;
}

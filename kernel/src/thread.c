#include <thread.h>
#include <heap.h>
#include <cpu.h>
#include <memory_layout.h>
#include <paging.h>
#include <string.h>
#include <scheduler.h>

extern list_t *running_threads;
extern struct thread_state* current_thread;

void thread_sync_pagedir(struct thread_state *thread) {
    struct thread_state *main_thread = thread->process->threads->head->element;
    if(thread != main_thread && main_thread != NULL && thread != NULL) {
        int pages = NUM_PAGES(0xB0000000);
        int end_pd = pages / 1024;
        int i;
        pt_t *pt0, pt1;
        for(i = 0; i < end_pd; i++) {
            if(main_thread->pagedir->entries[i] & PTE_PRESENT) {
	        if(thread->pagedir->entries[i] & PTE_PRESENT) {
                    pt1 = pt_get(thread->pagedir, i, PTE_WRITABLE | PTE_USER);
                } else {
                    pt1 = pt_create(thread->pagedir, i, PTE_WRITABLE | PTE_USER);
                }
                pt0 = pt_get(main_thread->pagedir, i, PTE_WRITABLE | PTE_USER);
                memcpy(pt1, pt0, 4096);
            }
        }
    }
}

struct thread_state *thread_create(struct process_state *process, privilege_t prev, uint32_t eip, struct cpu_state *state, void *args)
{
    struct thread_state *new_thread = malloc(sizeof(struct thread_state));
	new_thread->flags = THREAD_ACTIV;
    new_thread->process = process;
    new_thread->pagedir = pd_create();
    thread_sync_pagedir(new_thread);
    new_thread->ticks = 10;
    new_thread->return_value = 0;

    void *kernel_stack = malloc(0x1000);
    struct cpu_state *new_state = kernel_stack + 0x1000 - sizeof(struct cpu_state);
    new_thread->state = new_state;

    if(state != NULL)
    {
        memcpy(new_state, state, sizeof(struct cpu_state));
    }
    else
    {
        memset(new_state, 0, sizeof(struct cpu_state));
        new_state->eip = eip;
        new_state->eflags = 0x202;
    }

    if(prev == KERNELMODE)
    {
        new_thread->flags |= THREAD_KERNELMODE;

		new_state->cs = 0x08;
		new_state->ds = 0x10;
		new_state->es = 0x10;
		new_state->fs = 0x10;
		new_state->gs = 0x10;
    }
    else
    {
        if(!state)
        {
            paddr_t pframe = pmm_alloc_page();
            pd_map(new_thread->pagedir, pframe, MEMORY_LAYOUT_STACK_TOP-0x1000, PTE_PRESENT | PTE_WRITABLE | PTE_USER);
            new_state->esp = (uint32_t) MEMORY_LAYOUT_STACK_TOP;
	}

		new_state->cs = 0x1b;
		new_state->ss = 0x23;
    }

    if(list_is_empty(process->zombie_tids))
        new_thread->tid = process->tid_counter++;
    else
        new_thread->tid = list_pop_back(process->zombie_tids);


    list_push_front(process->threads,new_thread);
    list_push_front(running_threads, new_thread);
    return new_thread;
}

void thread_kill(struct thread_state *thread)
{
    asm volatile("cli");
    if(current_thread == thread)
        thread->flags |= THREAD_ZOMBIE;
    else
    {
        thread_kill_sub(thread);
    }
    asm volatile("sti");
}

void thread_kill_sub(struct thread_state *thread)
{
    if(thread->flags & THREAD_ACTIV || thread->flags & THREAD_ZOMBIE)
    {
        list_set_first(running_threads);
        while(!list_is_last(running_threads))
        {
            struct thread_state *t = list_get_current(running_threads);
            if(t == thread)
            {
                list_remove(running_threads);
                break;
            }
        }
    }
    free(thread->state);
    if(! (thread->process->flags & PROCESS_ZOMBIE))
    {
        list_push_front(thread->process->zombie_tids,thread->tid);
        list_set_first(thread->process->threads);
        while(!list_is_last(thread->process->threads))
        {
            struct thread_state *t = list_get_current(thread->process->threads);
            if(t == thread)
            {
                list_remove(thread->process->threads);
                if(list_is_empty(thread->process->threads))
                    process_kill(thread->process);
                break;
            }
        }
    }
    free(thread);
}

void thread_exit(struct cpu_state **cpu)
{
    current_thread->flags |= THREAD_ZOMBIE;
    *cpu = task_schedule(*cpu);
}

void launch_thread(struct cpu_state **cpu)
{
    thread_create(current_thread->process, USERMODE, (*cpu)->ebx, NULL, (*cpu)->ecx);
}



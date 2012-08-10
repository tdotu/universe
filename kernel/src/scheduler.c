/*
	Copyright 2012 universe coding group (UCG) all rights reserved
	This file is part of the Universe Kernel.

    Universe Kernel is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.

    Universe Kernel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Universe Kernel.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <scheduler.h>
#include <drivers/timer.h>
#include <tss.h>
#include <gdt.h>
#include <printf.h>
#include <string.h>
#include <pmm.h>

tss_s tss ={.ss0=0x10};
uint32_t *kernelstack;

static struct task_state *proc0;
static struct task_state *currentprocess;
static struct zombiepid *freepid=0;
static uint32_t pit_counter = 0;
static bool lock= false;

extern pd_t *pd_kernel;
extern pd_t *pd_current;

void INIT_SCEDULER(void){
	set_GDT_entry(5,(uint32_t) &tss, sizeof(tss),0x89,0x8);
	load_gdt(5);
	asm ("ltr %%ax" : : "a" (5 << 3));
	kernelstack = malloc(4096);
	tss.esp = (uint32_t)kernelstack+4096;

	//Kernel-init Process
	proc0                 = malloc(624);
	proc0->prev           = proc0;
	proc0->next           = proc0;
	proc0->pid            = pit_counter;
	proc0->pagedir        = pd_kernel;
	proc0->port           = NULL;
	proc0->threads        = NULL;
	proc0->proc_children  = NULL;
	proc0->flags          = ACTIV|REALTIME_PRIORITY|KERNELMODE;
	strcpy((char*)&proc0->name,"Kernel32.elf");
	strcpy((char*)&proc0->description,"Kernel initialization");
    currentprocess = proc0;
    pit_counter++;
}

struct cpu_state *task_schedul(struct cpu_state *cpu){
    /* TODO: Use Threads */


    if(currentprocess->flags&TRASH){
        //TODO: Free allocated memory and pagdir
        free(currentprocess);
    }else{
        //save state
        currentprocess->main_state = *cpu;
    }
    currentprocess = currentprocess->next;
    while(!(currentprocess->flags & (ACTIV|FREEZED))){
        currentprocess=currentprocess->next;
    }
    pd_current =  currentprocess->pagedir;
    *cpu=currentprocess->main_state;
    pd_switch(currentprocess->pagedir,0);

    if(currentprocess->flags & NORMAL_PRIORITY){
        set_pit_freq(FREQ_NORMAL);
    }
    else if(currentprocess->flags & REALTIME_PRIORITY){
        set_pit_freq(FREQ_REALTIME);
    }
    else{
        set_pit_freq(FREQ_BACKGRUND);
    }
    EOI(0);
    return cpu;
}


pid_t proc_create(prev_t prev,vaddr_t vrt_base,paddr_t phy_base,size_t size,vaddr_t entrypoint,char* name,char* desc,priority_t priority) {
     struct task_state *proc_new = malloc(628);
    proc_new->pagedir = pd_create();
    memcpy(proc_new->pagedir, pd_kernel, 4096);

     if(prev){ //kernelmode
        if(size){
            pd_map_range(proc_new->pagedir,phy_base,vrt_base,PTE_WRITABLE,size/PAGE_SIZE);
        }
        proc_new->flags = KERNELMODE;
        struct cpu_state new_state = {
            .gs       = 0x10,
            .fs       = 0x10,
            .es       = 0x10,
            .ds       = 0x10,
            .edi      = 0,
            .esi      = 0,
            .ebp      = 0,
            .ebx      = 0,
            .edx      = 0,
            .ecx      = 0,
            .eax      = 0,
            .intr     = 0,
            .error    = 0,
            .eip      = entrypoint,
            .cs       = 0x8,
            .eflags   = 0x202,
            };

            proc_new->main_state = new_state;

     }else{ // Usermode

         pd_map_range(proc_new->pagedir,phy_base,vrt_base,PTE_WRITABLE|PTE_USER,size/PAGE_SIZE);
         pd_map(proc_new->pagedir,pmm_alloc_page(),0xBFFFF000, PTE_WRITABLE|PDE_USER);
         proc_new->flags = 0;
         struct cpu_state new_state = {
            .gs       = 0x2b,
            .fs       = 0x2b,
            .es       = 0x2b,
            .ds       = 0x2b,
            .edi      = 0,
            .esi      = 0,
            .ebp      = 0,
            .ebx      = 0,
            .edx      = 0,
            .ecx      = 0,
            .eax      = 0,
            .intr     = 0,
            .error    = 0,
            .eip      = entrypoint,
            .cs       = 0x23,
            .eflags   = 0x202,
            .esp = STACK_HEAD,
            .ss       = 0x2b,
            };

            proc_new->main_state = new_state;

     }


     strncpy((char*)&proc_new->name       , name, 255);
     strncpy((char*)&proc_new->description, desc, 255);
     proc_new->port          = NULL;
     proc_new->threads       = NULL;
     proc_new->currentthread = NULL;
     proc_new->proc_children = NULL;
     proc_new->proc_parent   = currentprocess;

     while(lock){}
     lock=true;
     asm volatile("cli");
     //use zombie PIDs
     if(freepid){
        proc_new->pid = freepid->pid;
        if(freepid->next == freepid){
            free(freepid);
            freepid=0;
        }else{
            freepid->prev->next = freepid->next;
            freepid->next->prev = freepid->prev;
            struct zombiepid *freepid_new = freepid->next;
            free(freepid);
            freepid = freepid_new;
        }
     }else{
        proc_new->pid = pit_counter;
     }

     proc_new->next        = proc0;
     proc_new->prev        = proc0->prev;
     proc0->prev           = proc_new;
     proc_new->prev->next  = proc_new;
     proc_new->flags      |= priority|ACTIV;
     pit_counter++;
     asm volatile("sti");
     lock=false;

    return proc_new->pid;
}

static void proc_structure_free(struct task_state *proc){
    if(proc->port){
        struct port_access *current = proc->port->next;
        struct port_access *next;
        while(current != proc->port){
            next = current->next;
            free(current);
            current = next;
        }
        free(proc->port);
    }

     if(proc->threads){
        struct thread *current = proc->threads->next;
        struct thread *next;
        while(current != proc->threads){
            next = current->next;
            free(current);
            current = next;
        }
        free(proc->threads);
    }

    if(proc->proc_children){
        struct child *current = proc->proc_children->next;
        struct child *next;
        while(current != proc->proc_children){
            next = current->next;
            free(current);
            current = next;
        }
        free(proc->proc_children);
    }

}


void proc_kill(struct task_state *proc){
    if(proc){
        while(lock){}
        lock=true;
        asm volatile("cli");

        proc->prev->next = proc->next;
        proc->next->prev = proc->prev;

        if(!freepid){
            freepid = malloc(12);
            freepid->prev = freepid;
            freepid->next = freepid;
            freepid->pid = proc->pid;
        }else{
            struct zombiepid *zombiepid_new = malloc(12);
            zombiepid_new->prev = freepid->prev;
            zombiepid_new->next = freepid;
            freepid->prev->next = zombiepid_new;
            freepid->prev = zombiepid_new;
        }
        proc_structure_free(proc);
        if(currentprocess != proc){
            //TODO: Free allocated memory and pagdir
            free(currentprocess);
        }else{
            proc->flags |= TRASH;
        }
        asm volatile("sti");
        lock=false;
    }
}

void exit(int errorcode){
    proc_kill(currentprocess);
    asm volatile("int $32");
}


struct task_state* proc_get(uint32_t pid){
    if(proc0->pid == pid){
        return proc0;
    }

	struct task_state* temp=proc0->next;
	while(temp->next!=proc0->next && temp->pid != pid ){temp=temp->next;}
	if(temp->pid != pid){
		return NULL;
    }
	return temp;
}

uint32_t thread_create(uint32_t eip){

}

void thread_kill(struct thread *thr){


}

void thread_exit(int errorcode){

}

void thread_get(uint32_t tid){


}


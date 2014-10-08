/*
     Copyright 2012-2014 Infinitycoding all rights reserved
     This file is part of the Universe Kernel.

     The Universe Kernel is free software: you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     any later version.

     The Universe Kernel is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.

     You should have received a copy of the GNU General Public License
     along with the Universe Kernel. If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @author Simon Diepold aka. Tdotu <simon.diepold@infinitycoding.de>
 *  @author Michael Sippel <micha@infinitycoding.de>
 */

#include <event/trigger.h>

#include <sched/thread.h>
#include <sched/scheduler.h>
#include <sched/elf.h>
 
#include <memory_layout.h>
#include <string.h>
#include <mm/heap.h>
#include <vfs/fd.h>


extern list_t *running_threads;
extern struct thread_state* current_thread;
extern struct process_state *kernel_state;
extern iterator_t thread_iterator;

void usys_thread_exit(struct cpu_state **cpu)
{
    current_thread->flags |= THREAD_ZOMBIE;
    *cpu = task_schedule(*cpu);
}

void usys_thread_launch(struct cpu_state **cpu)
{
    thread_create(current_thread->process, USERMODE, (vaddr_t) (*cpu)->CPU_ARG1, (int) (*cpu)->CPU_ARG2, (char**)(*cpu)->CPU_ARG3, (char **)(*cpu)->CPU_ARG4, (vaddr_t)(*cpu)->CPU_ARG5, NULL);
}


/**
 * @brief terminates the current process (linux function for the API)
 * @param cpu registers of the corrent process
 */
void sys_exit(struct cpu_state **cpu)
{
    iterator_t parents_children_it = iterator_create(current_thread->process->parent->children);

    while(!list_is_empty(current_thread->process->parent->children) && !list_is_last(&parents_children_it))
    {
        struct child *current_child = list_get_current(&parents_children_it);
        if(current_child->process == current_thread->process)
        {
            current_child->status = (*cpu)->CPU_ARG1;
            break;
        }
        list_next(&parents_children_it);
    }

    process_kill(current_thread->process);
    *cpu = task_schedule(*cpu);
}

/**
 * @brief creates a new child process (linux function for the API)
 * @param cpu registers of the current process
 */
void sys_fork(struct cpu_state **cpu)
{
	const char *add = "_fork";
	char *newname = malloc(strlen(current_thread->process->name)+strlen(add)+1);
	strcpy(newname, current_thread->process->name);
	strcat(newname, add);

    struct process_state *new_process = process_create(newname ,current_thread->process->flags ,current_thread->process, current_thread->process->uid, current_thread->process->gid, NULL);
    struct thread_state *new_thread = thread_clone(new_process, current_thread);

	// copy file descriptors
    struct list_node *node = current_thread->process->files->head->next;
    struct list_node *head = current_thread->process->files->head;
    while(node != head)
    {
        struct fd *dest = malloc(sizeof(struct fd));
        struct fd *src  = (struct fd*) node->element;
        memcpy(dest, src, sizeof(struct fd));
        list_push_back(new_process->files, dest);

        node = node->next;
    }

    new_thread->context.state->CPU_ARG0 = 0;
    current_thread->context.state->CPU_ARG0 = new_process->pid;
}

/**
 *  @brief wait for the termination of a child (linux function for the API)
 *  @param cpu registers of the current process
 *  Not completed
 */

void sys_waitpid(struct cpu_state **cpu)
{
    list_set_first(&thread_iterator);
    while(list_get_current(&thread_iterator) != current_thread)
    {
        list_next(&thread_iterator);
    }
    list_remove(&thread_iterator);
    current_thread->ticks = 0;
    current_thread->flags |= THREAD_WAITPID;
    current_thread->waitpid = (*cpu)->CPU_ARG1;
    add_trigger(WAIT_PID, current_thread->waitpid, false, (void *)current_thread,NULL);
    *cpu = task_schedule(*cpu);
}

void sys_getpid(struct cpu_state **cpu)
{
    (*cpu)->CPU_ARG0 = current_thread->process->pid;
}


/**
 * executes a programm
 * @param cpu registers of the current process
 * todo: the function is still a litte bit slow and envp is not taken over from the new process.
 */
void sys_execve(struct cpu_state **cpu)
{
    char *filename = (char*) (*cpu)->CPU_ARG1;
    char **argv = (char**) (*cpu)->CPU_ARG2;
    char **envp = (char**) (*cpu)->CPU_ARG3;

	// lookup file
    vfs_inode_t *filenode = vfs_lookup_path(filename);
    if(filenode == NULL)
    {
        (*cpu)->CPU_ARG0 = _FAILURE;
        return;
    }

	// terminate all threads
    struct process_state *process = current_thread->process;
	//process_kill(process);
    /*iterator_t it = iterator_create(process->threads);
	while(!list_is_last(&it))
	{
		struct thread_state *t = list_get_current(&it);
		thread_kill_sub(t);

		list_next(&it);
	}
*/
    //list_destroy(process->ports);
    //list_destroy(process->zombie_tids);
    //process->zombie_tids = list_create();

    // run the new process
	load_elf_from_file(filenode, 0, 0, NULL, 0, argv, envp);
    //load_elf_thread_from_file(filenode, process, 0, argv, envp);

        (*cpu)->CPU_ARG0 = _SUCCESS;
}
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

/**

  @author Simon Diepold aka. Tdotu (Universe Team) <simon.diepold@infinitycoding.de>

 */

#include <syscall.h>
#include <thread.h>
#include <scheduler.h>
#include <process.h>
#include <vfs.h>
#include <printf.h>
#include <drivers/timer.h>

#define OS_VERSION 1

extern struct thread_state *current_thread;
extern struct process_state *kernel_state;


void print(struct cpu_state **cpu)
{
    printf("%s", (char *)(*cpu)->CPU_ARG1);
    (*cpu)->CPU_ARG0 = 0;
}

void identify_os(struct cpu_state **cpu)
{
    (*cpu)->CPU_ARG0 = 'u' | ('n' << 8) | ('i' << 16) | (OS_VERSION << 24);
};



#define DEFINED_LINUX_FUNCTIONS 12


void (*linux_functions[])(struct cpu_state **cpu) =
{
    NULL,sys_exit,sys_fork,sys_read,sys_write,sys_open,sys_close,sys_waitpid,sys_creat,sys_link,sys_unlink,/*sys_chdir,*/ sys_time,/*FIXME: pipe kommt weiter hinten*/ sys_pipe
};

void linux_syscall_handler(struct cpu_state **cpu)
{
    if( (*cpu)->CPU_ARG0 < DEFINED_LINUX_FUNCTIONS)
    {
	if(linux_functions[(*cpu)->CPU_ARG0] != NULL)
            linux_functions[(*cpu)->CPU_ARG0](cpu);
    }
}


#define DEFINED_UNIVERSE_FUNCTIONS 5
void (*universe_functions[])(struct cpu_state **cpu) =
{
    print,thread_exit,launch_thread,identify_os,alloc_memory
};


void universe_syscall_handler(struct cpu_state **cpu)
{
    if( (*cpu)->CPU_ARG0 < DEFINED_UNIVERSE_FUNCTIONS)
    {
        if(universe_functions[(*cpu)->CPU_ARG0] != NULL)
            universe_functions[(*cpu)->CPU_ARG0](cpu);
    }
}

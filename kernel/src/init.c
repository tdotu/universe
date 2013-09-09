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

**/

/**
  @author Simon Diepold aka. Tdotu (Universe Team) <simon.diepold@infinitycoding.de>
  @author Michael Sippel <micha.linuxfreak@gmail.com>
  @author Christian Klahn <christian.klahn1@gmail.com>
  @author Tom Slawik <tom.slawik@gmail.com>

  - everyone else
 */


#include <stdint.h>
#include <printf.h>
#include <multiboot.h>
#include <panic.h>

#include <pmm.h>
#include <paging.h>
#include <gdt.h>
#include <idt.h>
#include <io.h>
#include <cpuid.h>
#include <logo.h>
#include <scheduler.h>
#include <heap.h>

#include <drivers/keyboard.h>
#include <drivers/timer.h>
#include <drivers/cmos.h>
#include <drivers/video.h>
#include <drivers/vfs.h>
#include <drivers/pci.h>

#include "memory_layout.h"

/**
* Initalize the Kernel
*
* @param mb_info The pointer to the multiboot-struct from the bootloader
* @param magic_number Multiboot-magic
*
* @return 0
*/
extern struct thread_state *current_thread;
extern struct process_state *kernel_state;
extern pd_t *pd_current;
int init (struct multiboot_struct *mb_info, uint32_t magic_number) {
	clear_screen();

	if (magic_number != 0x2BADB002) {
		panic("Incompatible Bootloader");
	}

	set_color(WHITE | BLACK << 4);
	//Init Kernelmodules
	INIT_PMM(mb_info);
	INIT_GDT();
	INIT_IDT();
	INIT_PAGING(mb_info);
	INIT_HEAP();
	INIT_PIT(50);
	INIT_CMOS();
	INIT_KEYBOARD();
	INIT_SCHEDULER();
	INIT_VFS();

	asm volatile("sti");

	//print Logo and loading message
	print_logo(YELLOW);
	puts("Universe wird gestartet...\n");


	// count free memory and display it
	uint32_t pages = pmm_count_free_pages();
	printf("%u freie Speicherseiten (%u MB)\n", pages, pages >> 8);

	//print current time
	//print_time(get_time()); //crashes on a real computer and on virtual box
	printf("\n");
	
	INIT_CPUID();
	printf("\n");
	INIT_PCI();
	
	// Load modules
	int i,j;
	struct mods_add* modules = mb_info->mods_addr;
        for(i = 0; i < mb_info->mods_count; i++) {
        	size_t len = modules[i].mod_end - modules[i].mod_start;
        	size_t pages = NUM_PAGES(len);
        	void *mod = pd_automap_kernel_range(pd_current, modules[i].mod_start, pages, PTE_WRITABLE);
		struct process *proc = load_elf(mod);
        }
	
	while (1) {
		putchar(input());
	}
	
	return 0;
}

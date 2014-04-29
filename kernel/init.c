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
 * @author Simon Diepold aka. Tdotu (Universe Team) <simon.diepold@infinitycoding.de>
 * @author Michael Sippel <micha.linuxfreak@gmail.com>
 * @author Christian Klahn <christian.klahn1@gmail.com>
 * @author Tom Slawik <tom.slawik@gmail.com>
 */

#include <stdint.h>
#include <printf.h>
#include <sys/multiboot.h>
#include <panic.h>
#include <pmm.h>
#include <mm/paging.h>
#include <gdt.h>
#include <idt.h>
#include <io.h>
#include <cpuid.h>
#include <logo.h>
#include <sched/scheduler.h>
#include <mm/heap.h>
#include <vfs/vfs.h>
#include <trigger.h>
#include <drivers/timer.h>
#include <drivers/cmos.h>
#include <drivers/video.h>
#include <drivers/pci.h>
#include <sched/elf.h>
#include <mutex.h>
#include "memory_layout.h"
#include <udrcp/hypervisor.h>
#include <udrcp/drvlst.h>
/**
* Initalize the Kernel
*
* @param mb_info The pointer to the multiboot-struct from the bootloader
* @param magic_number Multiboot-magic
*
* @return 0
*/
extern struct process_state *kernel_state;
extern list_t *running_threads;
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
    INIT_VFS();
    INIT_TRIGGER();
    asm volatile("sti");
    INIT_PIT(200);
    INIT_RTC();
    INIT_SCHEDULER();
    //print Logo and loading message
    print_logo(YELLOW);
    puts("Universe wird gestartet...\n");
    // count free memory and display it
    /*uint32_t pages = pmm_count_free_pages();
    printf("%u freie Speicherseiten (%u MB)\n", pages, pages >> 8);*/
    //print current time
        struct time t;
        update_time(&t);
    print_time(&t); //crashes on a real computer and on virtual box
    printf("\n");
    printf("Timestamp:%d\n\n",unix_time(&t));
    INIT_CPUID();
    printf("\n");
    INIT_PCI();

    struct mods_add* modules = (struct mods_add*) mb_info->mods_addr;
    int i;
    void *phys = (void*)((int)modules[0].string & (int)~0xfff);
    void *virt = (void*) vmm_automap_kernel(current_context, (paddr_t)phys, VMM_WRITABLE);
    for(i = 0; i < mb_info->mods_count; i++) {
        int diff = (int)modules[i].string - (int)phys;
        modules[i].string = virt + diff;
    }

    struct mods_add *mod = (struct mods_add *)find_module(mb_info, "/boot/drivers.dat");

    if(mod != NULL) {
    	size_t len = mod->mod_end - mod->mod_start;
    	size_t pages = NUM_PAGES(len);
    	char *drv_list = (void*)vmm_automap_kernel_range(current_context,(paddr_t) mod->mod_start, pages, VMM_WRITABLE);
     	int argc = 2;
     	void *argv[2];
     	char ** c = get_table_section("UHOST",drv_list);

        argv[1] = mb_info;
     	argv[0] = c;
     	kernel_thread_create((uintptr_t)INIT_HYPERVISOR,argc,argv);
    }

    struct mods_add *shell_mod = find_module(mb_info, "/ultrashell.elf");
    if(shell_mod != NULL) {
	size_t sh_len = shell_mod->mod_end - shell_mod->mod_start;
        size_t sh_pages = NUM_PAGES(sh_len);
        void *shell = (void*)vmm_automap_kernel_range(current_context,(paddr_t) shell_mod->mod_start, sh_pages, VMM_WRITABLE);
        load_elf(shell,0,0,NULL);
    }

    dump_thread_list(running_threads);



    return 0;
}

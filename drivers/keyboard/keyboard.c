/*
     Copyright 2014 Infinitycoding all rights reserved
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
 *  @author Simon Diepold aka. Tdotu <simon.diepold@infinitycoding.de>
 **/

#include <stdint.h>
#include <stdio.h>
#include <udrcp.h>
#include <ioport.h>
#include <unistd.h>

#include <keyboard.h>
#include <keymap_de.h>
#include <string.h>


static bool shift = false;
static bool numlock = false;
static bool caps = false;
port_t *kbc_stat;
port_t *kbc_io;


/**
 * Initalize the Keyboard
 *
 * @param void
 * @return void
 */
int main(void)
{
    pckmgr *conn = new_pckmgr(stdin, stdout, stderr);
    if(!subsystem_connect(conn,UHOST_DEFAULT_SYNCHRON))
    {
        write(stderr,"could not connect to host",strlen("could not connect to host"));
        return -1;
    }


    kbc_stat = port_alloc(conn,0x64);
    kbc_io = port_alloc(conn,0x60);

    if(!kbc_io || !kbc_stat)
    {
        write(stderr,"could not allocate port",strlen("could not allocate port"));
        return -2;
    }


  	while (!(inb(kbc_stat) & 0x4));
  	// Puffer leeren
  	while (inb(kbc_stat) & 0x1)
  		inb(kbc_io);


  	while (inb(kbc_stat) & 0x2);
  	outb(kbc_io, 0xF4);
  	while (inb(kbc_io) != 0xFA);

    // Todo
  	//install_irq(0x1, &kbd_irq_handler);
}

/**
 * IRQ handler for the Keyboard
 *
 * @param void
 * @return void
 */
void kbd_irq_handler(void) {
	uint8_t input = 0, ASCII = 0;
	while (inb(kbc_stat)&1) {
		input = inb(kbc_io);

		if (input==0xC5) { //pressed numlock
			numlock = !numlock;
			continue;
		}

		if (input==0x3A) { //pressed capslock
			caps = !caps;
			continue;
		}


		if ((input & 0x80) == 0x80) { //release Key
		    input &= 0x7F;

		    if (input == 0x2A || input == 0x36) { //released shift key
		        shift = false;
		    }
			continue;
		}

		if (input == 0x2A || input == 0x36) { //pressed shift
			shift = true;
			continue;
		}

		if (input > 0x46 && input < 0x55 && input != 0x4A && input != 0x4C && input != 0x4E && numlock) { //Numblock Key
			ASCII = asciishift[input];
		}

		else if (shift || caps) { //Common Key
			ASCII = asciishift[input];
		} else {
			ASCII = asciinormal[input];
		}

	}

	if (ASCII) {



	}

}

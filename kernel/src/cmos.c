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

	
	
    Diese Datei ist ein Teil vom Universe Kernel.

    Das Universe Kernel ist Freie Software: Sie können es unter den Bedingungen
    der GNU General Public License, wie von der Free Software Foundation,
    Version 3 der Lizenz oder jeder späteren
    veröffentlichten Version, weiterverbreiten und/oder modifizieren.

    Das Universe Kernel wird in der Hoffnung, dass es nützlich sein wird, aber
    Universe Kernel wird in der Hoffnung, dass es nützlich sein wird, aber
    OHNE JEDE GEWÄHELEISTUNG, bereitgestellt; sogar ohne die implizite
    Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
    Siehe die GNU General Public License für weitere Details.

    Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
    Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/
#include <stdint.h>
#include <io.h>
#include <driver/timer.h>
#include <driver/cmos.h>

cmos_data_t cmos;

/**
 * Reads the Values from Complementary Metal Oxide Semiconductor
 *
 * @param void
 * @return void
 */
void INIT_CMOS(void){
	cmos.registers.register_a = cmos_read_byte(0x0A);
	cmos.registers.register_b = cmos_read_byte(0x0B);
	cmos.registers.register_c = cmos_read_byte(0x0C);
	cmos.registers.register_d = cmos_read_byte(0x0D);

	INIT_RTC();
	cmos.time = get_time();

	cmos.hardware.post_diagnostig_status_byte = cmos_read_byte(0x0E);
	cmos.hardware.shutdown_status_byte =        cmos_read_byte(0x0F);
	cmos.hardware.floppy_disk_type =            cmos_read_byte(0x10);
	cmos.hardware.hd_type =                     cmos_read_byte(0x12);
	cmos.hardware.device_byte =                 cmos_read_byte(0x14);
	
	cmos.hardware.basememory_size_low =         cmos_read_byte(0x15);
	cmos.hardware.basememory_size_high =        cmos_read_byte(0x16);
	cmos.hardware.expandablememory_size_low =   cmos_read_byte(0x17);
	cmos.hardware.expandablememory_size_high =  cmos_read_byte(0x18);
	cmos.hardware.extension_byte_hd1 =          cmos_read_byte(0x19);
	cmos.hardware.extension_byte_hd2 =          cmos_read_byte(0x1A);
	
	cmos.hardware.cmos_magic_low =              cmos_read_byte(0x2E);
	cmos.hardware.cmos_magic_high =             cmos_read_byte(0x2F);
	
	cmos.hardware.extendedmemory_low =          cmos_read_byte(0x30);
	cmos.hardware.extendedmenory_high =         cmos_read_byte(0x31);
}
/**
 * Get the CMOS-data
 * 
 * @param void
 *
 * @return pointer to the cmos-struct
 */
cmos_data_t* get_cmos_data(void){
	return &cmos;
}
/**
 * Reads a byte from CMOS
 * @param offset Offset in the CMOS
 * 
 * @return Read value from CMOS
 */
uint8_t cmos_read_byte(uint8_t offset) {
  uint8_t tmp = inb(0x70);
  outb(0x70, (tmp & 0x80) | (offset & 0x7F));
  return inb(0x71);
}
/**
 * Write a byte to CMOS
 *
 * @param offset Offset in the CMOS
 * @param value Value which is written into the CMOS
 *
 * @return void
 */
void cmos_write_byte(uint8_t offset, uint8_t value) {
  uint8_t tmp = inb(0x70);
  outb(0x70, (tmp & 0x80) | (offset & 0x7F));
  outb(0x71,value);
}


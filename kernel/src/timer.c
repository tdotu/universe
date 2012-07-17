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
#include <idt.h>
#include <driver/cmos.h>
#include <driver/timer.h>


time_t current_time;
cmos_data_t *cmos;

/**
 * Initalize the Programmable Intervall Timer
 *
 * @param void
 * @return void
 */
void INIT_PIT(int freq) {
	int counter = 1193182 / freq;
	outb(0x43, 0x34);
   	outb(0x40,counter & 0xFF);
   	outb(0x40,counter >> 8);
}

/**
 * Initalize the Real Time Clock
 *
 * @param void
 * @return void
 */
void INIT_RTC(void) {
	install_irq(0x8, &rtc_irq_handler);
	
	cmos = get_cmos_data();
	cmos_write_byte(0x0A, (cmos->registers.register_a & 0xF0) | 0x0F);
	cmos_write_byte(0x0B, cmos->registers.register_b | 0x40);
	
	update_time();
}
/**
 * IRQ-handler for the RTC
 *
 * @param void
 * @return void
 */
void rtc_irq_handler(void){
	cmos->registers.register_c = cmos_read_byte(0x0C);
	update_time();
}
/**
 * Updates the time from CMOS-RTC
 *
 * @param void
 *
 * @return success
 */
int update_time(void){
	current_time.second =       cmos_read_byte(0x00);
	current_time.alarm_sec =    cmos_read_byte(0x01);
	current_time.minute =       cmos_read_byte(0x02);
	current_time.alarm_min =    cmos_read_byte(0x03);
	current_time.hour =         cmos_read_byte(0x04);
	current_time.alarm_hour =   cmos_read_byte(0x05);
	current_time.week_day =     cmos_read_byte(0x06);
	current_time.day_in_month = cmos_read_byte(0x07);
	current_time.month =        cmos_read_byte(0x08);
	current_time.year =         cmos_read_byte(0x09);
	current_time.century =      cmos_read_byte(0x32);

	return 0;
}
/**
 * Changes the Time from CMOS
 *
 * @param time New time
 *
 * @return success
 */
int change_time(time_t time){
	cmos_write_byte(0x00, time.second);
	cmos_write_byte(0x01, time.alarm_sec);
	cmos_write_byte(0x02, time.minute);
	cmos_write_byte(0x03, time.alarm_min);
	cmos_write_byte(0x04, time.hour);
	cmos_write_byte(0x05, time.alarm_hour);
	cmos_write_byte(0x06, time.week_day);
	cmos_write_byte(0x07, time.day_in_month);
	cmos_write_byte(0x08, time.month);
	cmos_write_byte(0x09, time.year);
	cmos_write_byte(0x32, time.century);

	return 0;
}
/**
 * Get the current time
 *
 * @param void
 *
 * @return pointer to the current_time
 */
time_t *get_time(void){
	return &current_time;
}


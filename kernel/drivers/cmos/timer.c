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
 *  @author Michael Sippel <michamimosa@gmail.com>
 */

#include <drivers/cmos.h>
#include <drivers/timer.h>
#include <mm/heap.h>
#include <printf.h>
#include <bcd.h>
#include <io.h>
#include <idt.h>

struct time *current_time;
struct cmos_data *cmos;

/**
 * set PIT Fequency
 *
 * @param freqency
 * @return void
 */
void set_pit_freq(int freq) {
	int counter = 1193182 / freq;
   	outb(0x40,counter & 0xFF);
   	outb(0x40,counter >> 8);
}

/**
 * Initalize the Programmable Intervall Timer
 *
 * @param frequency
 * @return void
 */
void INIT_PIT(int freq) {
	outb(0x43, 0x34);
	set_pit_freq(freq);
}

/**
 * Initalize the Real Time Clock
 *
 * @param void
 * @return void
 */
void INIT_RTC(void) {
	cmos = malloc(sizeof(struct cmos_data));
	current_time = malloc(sizeof(struct time));
	get_cmos_data(cmos);
	update_time(current_time);
}

/**
 * Updates the time from CMOS-RTC
 *
 * @param void
 */
void update_time(struct time *time) {
	time->second =       BCD_DECODE(cmos_read_byte(0x00));
	time->alarm_sec =    BCD_DECODE(cmos_read_byte(0x01));
	time->minute =       BCD_DECODE(cmos_read_byte(0x02));
	time->alarm_min =    BCD_DECODE(cmos_read_byte(0x03));
	time->hour =         BCD_DECODE(cmos_read_byte(0x04));
	time->alarm_hour =   BCD_DECODE(cmos_read_byte(0x05));
	time->week_day =     BCD_DECODE(cmos_read_byte(0x06)) - 1;
	time->day_in_month = BCD_DECODE(cmos_read_byte(0x07));
	time->month =        BCD_DECODE(cmos_read_byte(0x08));
	time->year =         BCD_DECODE(cmos_read_byte(0x09));
	time->century =      BCD_DECODE(cmos_read_byte(0x32));
}

/**
 * Changes the Time from CMOS
 *
 * @param time New time
 */
void change_time(struct time *time) {
        char bcd_str[2];
	cmos_write_byte(0x00, BCD_ENCODE(bcd_str, time->second));
	cmos_write_byte(0x01, BCD_ENCODE(bcd_str, time->alarm_sec));
	cmos_write_byte(0x02, BCD_ENCODE(bcd_str, time->minute));
	cmos_write_byte(0x03, BCD_ENCODE(bcd_str, time->alarm_min));
	cmos_write_byte(0x04, BCD_ENCODE(bcd_str, time->hour));
	cmos_write_byte(0x05, BCD_ENCODE(bcd_str, time->alarm_hour));
	cmos_write_byte(0x06, BCD_ENCODE(bcd_str, time->week_day));
	cmos_write_byte(0x07, BCD_ENCODE(bcd_str, time->day_in_month));
	cmos_write_byte(0x08, BCD_ENCODE(bcd_str, time->month));
	cmos_write_byte(0x09, BCD_ENCODE(bcd_str, time->year));
	cmos_write_byte(0x32, BCD_ENCODE(bcd_str, time->century));
}

const int day_to_current_month[] = {0,31,59,90,120,151,181,212,243,273,304,334}; // regeljahr
/**
 * creates a unix timestamp
 *
 * @param pointer ti time struct
 *
 * @return unix timestamp
 */
time_t unix_time(struct time *time) {
	int year = (time->century*100)+time->year;
	int leap_years = ((year - 1) - 1968) / 4 - ((year - 1) - 1900) / 100 + ((year - 1) - 1600) / 400;
	int unix_time = time->second + (time->minute *60) + (time->hour *60*60) + ((day_to_current_month[time->month - 1] + time->day_in_month - 1) *24*60*60) + (((year-1970)*365+leap_years)*24*60*60);
	if((time->month >2) && (year%4==0 && (year%100!=0 || year%400==0)))
	unix_time += 24*60*60;
	return unix_time;
}

int sys_time(struct cpu_state **cpu) {
    update_time(current_time);
    int stamp = unix_time(current_time);
    if((*cpu)->CPU_ARG1)
        *((int*)(*cpu)->CPU_ARG1) = stamp;
    return stamp;
}


/**
 * Print datetime
 */
void print_time(struct time *time) {
	char *day_string;
	switch (time->week_day) {
		case 0: day_string = "Sonntag";		break;
		case 1: day_string = "Montag";		break;
		case 2: day_string = "Dienstag";	break;
		case 3: day_string = "Mittwoch";	break;
		case 4: day_string = "Donnerstag";	break;
		case 5: day_string = "Freitag";		break;
		case 6: day_string = "Samstag";		break;
	}

	printf("System Date: %02d/%02d/%02d (%s)\n", time->day_in_month, time->month, time->year, day_string);
	printf("System Time: %02d:%02d:%02d\n", time->hour, time->minute, time->second);
}

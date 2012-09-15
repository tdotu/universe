#ifndef _stat_h_
#define _stat_h_
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

	 Das Universe Kernel ist Freie Software: Sie koennen es unter den Bedingungen
	 der GNU General Public License, wie von der Free Software Foundation,
	 Version 3 der Lizenz oder jeder sp‰teren
	 veroeffentlichten Version, weiterverbreiten und/oder modifizieren.

	 Das Universe Kernel wird in der Hoffnung, dass es nuetzlich sein wird, aber
	 Universe Kernel wird in der Hoffnung, dass es nuetzlich sein wird, aber
	 OHNE JEDE GEWAEHELEISTUNG, bereitgestellt; sogar ohne die implizite
	 Gew‰hrleistung der MARKTFAEHIGKEIT oder EIGNUNG FUER EINEN BESTIMMTEN ZWECK.
	 Siehe die GNU General Public License fuer weitere Details.

	 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
	 Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
*/

/**
	@author Michael Sippel <micha.linuxfreak@gmail.com>
*/
  #include <stdint.h>
  #include <unistd.h>
  #include <drivers/timer.h>
  #include <sys/types.h>
  
  #define S_MODE_DIR 0x1
  #define S_MODE_CHR 0x2
  #define S_MODE_LNK 0x4
  
  #define S_MAO 0x4 // st_mode: access offset
  
  #define S_IRUSR (0x001 + S_MAO) //user-read
  #define S_IWUSR (0x002 + S_MAO) // -write
  #define S_IXUSR (0x004 + S_MAO) // -execute
  #define S_IRGRP (0x008 + S_MAO) //group-read
  #define S_IWGRP (0x010 + S_MAO) // -write
  #define S_IXGRP (0x020 + S_MAO) // -execute
  #define S_IROTH (0x040 + S_MAO) //other-read
  #define S_IWOTH (0x080 + S_MAO) // -write
  #define S_IXOTH (0x100 + S_MAO) // -execute
  
  #define S_ISREG(x) (x.st_mode & S_MODE_DIR) ? 0 : 1
  #define S_ISDIR(x) (x.st_mode & S_MODE_DIR) ? 1 : 0
  #define S_ISCHR(x) (x.st_mode & S_MODE_CHR) ? 1 : 0
  #define S_ISBLK(x) (x.st_mode & S_MODE_CHR) ? 0 : 1
  #define S_ISLNK(x) (x.st_mode & S_MODE_LNK) ? 1 : 0
  
  struct stat {
      mode_t 	st_mode; // access rights
      ino_t	st_ino;
//       dev_t	st_dev; // filesystem
//       dev_t	st_rdev;
      
      nlink_t	st_nlink;
      uid_t	st_uid;
      gid_t	st_gid;
      off_t	st_size;
      
      time_t	st_atime; // access time
      time_t	st_mtime; // modification time
      time_t	st_ctime;
      
      long	st_blksize;
      long	st_blocks;
  };

#endif
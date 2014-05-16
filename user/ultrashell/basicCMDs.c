/*
     Copyright 2012-2014 Infinitycoding all rights reserved
     This file is part of the Ultrashell.
 
     The Ultrashell is free software: you can redistribute it and/or modify
     it under the terms of the GNU Lesser General Public License as published by
     the Free Software Foundation, either version 3 of the License, or
     any later version.
 
     The Ultrashell is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU Lesser General Public License for more details.
 
     You should have received a copy of the GNU Lesser General Public License
     along with the Ultrashell.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @author Peter Hösch aka. BlitzBasic <peter.hoesch@infinitycoding.de>
 **/



#include <basicCMDs.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>



/**
 * @brief this isn't the function you are looking for
 */

int dummy(int argc, char **argv)
{
	printf("This isn't the function you are looking for.\n");	

	return 0;
}


/**
 * @brief the echo function
 */

int echo(int argc, char **argv)
{
	int counter;

	for(counter = 1; counter < argc; counter++)
	{
		printf(argv[counter]);
		printf(" ");
	}

	printf("\n");

	return 0;
}


/**
 * @brief this function prints the version of the shell
 */

int sver(int argc, char **argv)
{
	printf("Ultrashell v. 1.1.0 (debug)\n");

	return 0;
}


/**
 * @brief this function always returns true
 */
int cmdtrue(int argc, char **argv)
{
	return 0;
}


/**
 * @brief this function always returns false
 */

int cmdfalse(int argc, char **argv)
{
	return 1;
}


/**
 * @brief changes the working directory
 */

int cd(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("cd: incorrect number of arguments\n");
		return 1;
	}

	if(chdir(argv[1]) == -1)
	{
		printf("cd: %s: unable to find file or directory", argv[1]);
		return 2;
	}

	return 0;
}


/**
 * @brief prints the current working directory
 */

int pwd(int argc, char **argv)
{
	if(argc != 1)
	{
		printf("pwd: incorrect number of arguments\n");
		return 1;
	}

	char wd[80];

	if(getcwd(wd, 80) == NULL)
	{
		printf("pwd: some error has occured during calling getcwd\n");
		return 2;
	}

	printf("%s\n", wd);

	return 0;	
}


/**
 * @brief creates a new directory
 */

/*int mkdir(int argc, char **argv)
{

}*/


/**
 * @brief removes a empty directory
 */

/*int rmdir(int argc, char **argv)
{

}*/


/**
 * @brief exits the shell
 */

int sexit(int argc, char **argv)
{
	printf("Exiting Ultrashell.\n");

	exit(0);

	return 0;
}
/*
 * PRAMFS: persistent and protected RAM Filesystem
 *
 * Copyright (C) 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>

int main(int argc, char *argv[])
{
	int fd;
	unsigned char *p;
	unsigned int len, n, i;
	char *buf;
	
	if (argc != 3) {
	  fprintf(stderr, "Usage: writehole <buf dim> <hole dim>\n");
	  exit(1);
	}
	
	fd = open("./pram/hole", O_RDWR|O_CREAT|O_TRUNC);
	if (fd == -1) {
		perror("Errore: ");
		exit(1);
	}

	buf = malloc(atoi(argv[1])*sizeof(char));

	for(i = 0;i<atoi(argv[1]);i++)
	     if (i<atoi(argv[1])-2)
		buf[i] = 'z';
	     else
		buf[i] = 'x';
	
	n = write(fd, buf, atoi(argv[1]));
	assert(atoi(argv[1]) == n);
	n = lseek(fd, atoi(argv[2]), SEEK_CUR);
	assert(atoi(argv[2]) + atoi(argv[1]) == n);
	n = write(fd, buf, atoi(argv[1]));
	assert(atoi(argv[1]) == n);
	free(buf);
	close(fd);
	return 0;
}

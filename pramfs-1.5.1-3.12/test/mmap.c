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

int main()
{
	int fd, ret;
	unsigned char *p;
	unsigned int len;
	
	fd = open("/pram/mmapfile", O_CREAT|O_TRUNC|O_RDWR);
	assert(fd != -1);

	/* put a char in the file */
	ret = write(fd, "a", 1);
	assert(ret == 1);

	p = mmap(0, 1, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	assert(p == MAP_FAILED);

	p = mmap(0, 1, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	assert(p != MAP_FAILED);
	
	*p = 'b';

	ret = munmap(p, 1);
	assert(ret == 0);
	close(fd);
	return 0;
}

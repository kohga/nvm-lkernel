/*
 * PRAMFS: persistent and protected RAM Filesystem
 *
 * Copyright (C) 2011 Marco Stornelli <marco.stornelli@gmail.com>
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
#include <string.h>

int main()
{
	int fd, ret;
	unsigned char *p;
	unsigned int len;
	char buf[4096];
	
	fd = open("/pram/mmapfile", O_CREAT|O_TRUNC|O_RDWR, 0777);
	assert(fd != -1);

	memset(buf,'a',4096);
	ret = write(fd, buf, 2048);
	assert(ret == 2048);

	memset(buf,'b',4096);
	ret = write(fd, buf, 2048);
	assert(ret == 2048);

	p = mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
	assert(p != MAP_FAILED);

	assert(p[5000] == 0);
	assert(p[2047] == 'a');
	assert(p[2048] == 'b');

	ret = munmap(p, 4096);
	assert(ret == 0);

	ret = ftruncate(fd, 2048);
	assert (ret == 0);

	ret = close(fd);
	assert (ret == 0);

	/* re-open the file */
	printf("reopen the file....\n");
	fd = -1;
	fd = open("/pram/mmapfile", O_RDONLY);
	assert(fd != -1);

	p = mmap(0, 2048, PROT_READ, MAP_SHARED, fd, 0);
	assert(p != MAP_FAILED);

	assert(p[2047] == 'a');
	assert(p[2048] == 0);
	assert(p[3268] == 0);

	ret = munmap(p, 2048);
	assert(ret == 0);
	close(fd);
	return 0;
}

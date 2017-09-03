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

#define O_DIRECT	00040000

int main(int argc, char *argv[])
{
	int fd;
	unsigned char *p;
	unsigned int len, n, i, err;
	unsigned long ret;
	char buf[1024];
	
	fd = open("./pram/fakefile", O_RDWR|O_CREAT|O_TRUNC);
	assert(fd != -1);

	/* O_DIRECT must be set... */
	ret = fcntl(fd, F_GETFL, 0);
	assert(ret & O_DIRECT);
	
	/* ...we can't remove it */
	ret = fcntl(fd, F_SETFL, ~O_DIRECT);
	assert(ret == -1);

	/* ...ok it is still there */
	ret = fcntl(fd, F_GETFL, 0);
	assert(ret & O_DIRECT);

	close(fd);
	ret = unlink("./pram/fakefile");
	assert(ret == 0);
	return 0;
}

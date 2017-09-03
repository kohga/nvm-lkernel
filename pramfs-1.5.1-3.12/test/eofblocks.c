/*
 * PRAMFS: persistent and protected RAM Filesystem
 *
 * Copyright (C) 2011 Marco Stornelli <marco.stornelli@gmail.com>
 * 
 * Based on a test of Josef Bacik <josef@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/falloc.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <linux/fs.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static int reset_file(int fd)
{
	int ret;

	ret = ftruncate(fd, 0);
	if (ret < 0) {
		fprintf(stderr, "Truncate failed: %d\n", errno);
		return 1;
	}

	return 0;
}

void check_eof(int fd, int present)
{
	unsigned int flags;
	if (ioctl(fd, FS_IOC_GETFLAGS, &flags) < 0) {
		perror("ioctl error");
		close(fd);
		exit(1);
	}
	printf("eof flags is %s\n", flags & 0x20000000 ? "set" : "unset");
	assert(((flags & 0x20000000) && present) || (!(flags & 0x20000000) && !present));
}

int main(int argc, char **argv)
{
	char buf[4096];
	ssize_t bytes;
	loff_t pos = 16384;
	loff_t off = 0;
	int ret;
	int i;
	int fd;

	fd = open("/pram/testfile", O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0) {
		fprintf(stderr, "Failed to open testfile: %d\n", errno);
		return 1;
	}

	ret = syscall(__NR_fallocate, fd, FALLOC_FL_KEEP_SIZE, off, pos);
	if (ret < 0) {
		perror("fallocate failed");
		close(fd);
		return 1;
	}
	pos = 4096;
	check_eof(fd, 1);

	for (i = 0; i < 4; i++) {
		bytes = write(fd, buf, 4096);
		if (bytes < 4096) {
			fprintf(stderr, "Failed to write to testfile: %d\n",
				errno);
			close(fd);
			return 1;
		}
	}

	check_eof(fd, 0);

	reset_file(fd);

	ret = syscall(__NR_fallocate, fd, FALLOC_FL_KEEP_SIZE, off, pos);
	if (ret < 0) {
		fprintf(stderr, "fallocate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	check_eof(fd, 1);
	
	ret = ftruncate(fd, 4096 * 4);
	if (ret < 0) {
		fprintf(stderr, "Truncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	check_eof(fd, 0);

	reset_file(fd);

	ret = syscall(__NR_fallocate, fd, FALLOC_FL_KEEP_SIZE, off, pos);
	if (ret < 0) {
		fprintf(stderr, "fallocate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	check_eof(fd, 1);

	ret = ftruncate(fd, 4096);
	if (ret < 0) {
		fprintf(stderr, "Truncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	check_eof(fd, 0);
out:
	close(fd);
	return 0;
}
 

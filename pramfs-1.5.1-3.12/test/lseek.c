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

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define SEEK_HOLE	4
#define SEEK_DATA	3

#define ERROR(str)	\
	fprintf(stderr, "%s: pos=%ld, errno=%d\n", str, pos, errno)

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

int main(int argc, char **argv)
{
	char buf[4096 * 4];
	char buf2[4096 * 512];
	ssize_t bytes;
	off_t pos;
	int prealloc_is_hole = 0;
	int whole_file_is_data = 0;
	int ret;
	int i;
	int fd;

	fd = open("/pram/testfile", O_RDWR|O_CREAT|O_TRUNC, 0644);
	if (fd < 0) {
		fprintf(stderr, "Failed to open testfile: %d\n", errno);
		return 1;
	}

	/* Empty file */
	printf("Testing an empty file\n");
	pos = lseek(fd, 0, SEEK_DATA);
	if (pos != -1) {
		if (errno == EINVAL) {
			fprintf(stderr, "Kernel does not support seek "
				"hole/data\n");
			close(fd);
			return 1;
		}
		if (errno != ENXIO)
			ERROR("Seek data did not return a proper error");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 0, SEEK_HOLE);
	if (pos != -1 && errno != ENXIO) {
		ERROR("Seek hole did not return a proper error");
		close(fd);
		return 1;
	}

	memset(&buf, 'a', 4096 * 4);
	memset(buf2,'a',4096*512);

	/*
	 * All data file
	 */
	printf("Testing a normal data filled file\n");
	for (i = 0; i < 4; i++) {
		bytes = write(fd, &buf, 4096);
		if (bytes < 4096) {
			fprintf(stderr, "Failed to write to testfile: %d\n",
				errno);
			close(fd);
			return 1;
		}
	}

	pos = lseek(fd, 0, SEEK_HOLE);
	if (pos != (4096 * 4) || pos == -1) {
		ERROR("Seek hole failed to dump us out at the end of the file");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 0, SEEK_DATA);
	if (pos != 0) {
		ERROR("Seek data failed to dump us out at the beginning of the"
		      " file");
		close(fd);
		return 1;
	}

	/*
	 * File with a hole at the front and data at the end
	 */
	printf("Testing file with hole at the start and data in the rest\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf, 4096 * 3, 4096);
	if (bytes < (4096 * 3)) {
		fprintf(stderr, "Failed to write to testfile: %d\n");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 0, SEEK_HOLE);
	if (pos != 0 && pos != (4096 * 4)) {
		ERROR("Seek hole failed to return 0");
		close(fd);
		return 1;
	} else if (pos == (4096 * 4)) {
		whole_file_is_data = 1;
		printf("Current file system views treats the entire file as "
		       "data\n");
	}

	pos = lseek(fd, 0, SEEK_DATA);
	if (pos != 4096 && (pos != 0 && whole_file_is_data)) {
		if (whole_file_is_data)
			ERROR("Seek data failed to return 0");
		else
			ERROR("Seek data failed to return 4096");
		close(fd);
		return 1;
	}

	if (whole_file_is_data) {
		pos = lseek(fd, 1, SEEK_DATA);
		if (pos != -1 && errno != ENXIO) {
			ERROR("Seek data failed to retun an error");
			close(fd);
			return 1;
		}
	}
	/*
	 * File with a hole at the end and data at the beginning
	 */
	printf("Testing file with hole at the end and data at the beginning\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	ret = ftruncate(fd, 4096 * 4);
	if (ret < 0) {
		fprintf(stderr, "Truncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	pwrite(fd, &buf, 4096 * 3, 0);
	if (bytes < (4096 * 3)) {
		fprintf(stderr, "Failed to write to testfile: %d\n", errno);
		close(fd);
		return 1;
	}

	pos = lseek(fd, 1, SEEK_DATA);
	if (pos != 1) {
		ERROR("Seek data didn't return 1");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 0, SEEK_HOLE);
	if (pos != (4096 * 3) && (pos != (4096 * 4) && whole_file_is_data)) {
		ERROR("Seeking hole didn't work right");
		close(fd);
		return 1;
	}

	if (whole_file_is_data) {
		pos = lseek(fd, pos, SEEK_HOLE);
		if (pos != -1 && errno != ENXIO) {
			ERROR("Seeking hole didn't return error");
			close(fd);
			return 1;
		}
		printf("No more tests to run since we treat the whole file as "
		       "data\n");
		goto out;
	}

	pos = lseek(fd, pos, SEEK_HOLE);
	if (pos != (4096 * 4)) {
		ERROR("Seek hole didn't return the end of the file");
		close(fd);
		return 1;
	}

	pos = lseek(fd, pos, SEEK_DATA);
	if (pos != -1 && errno != ENXIO) {
		ERROR("Seek data didn't return ENXIO");
		close(fd);
		return 1;
	}

	/*
	 * Hole - Data - Hole - Data file
	 */
	printf("Testing file [Hole][Data][Hole][Data]\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	ret = ftruncate(fd, 4096 * 4);
	if (ret < 0) {
		fprintf(stderr, "ftruncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf, 4096, 4096);
	if (bytes < 4096) {
		fprintf(stderr, "Failed to write: %d\n", errno);
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf, 4096, 4096 * 3);
	if (bytes < 4096) {
		fprintf(stderr, "Failed to write: %d\n", errno);
		close(fd);
		return 1;
	}

	pos = lseek(fd, 0, SEEK_DATA);
	if (pos != 4096) {
		ERROR("Seek data did not return 4096");
		close(fd);
		return 1;
	}

	pos = lseek(fd, pos, SEEK_HOLE);
	if (pos != 4096*2) {
		ERROR("Seek hole did not return 4096*2");
		close(fd);
		return 1;
	}

	pos = lseek(fd, pos, SEEK_DATA);
	if (pos != (4096 * 3)) {
		ERROR("Seek data did not return 4096*3");
		close(fd);
		return 1;
	}

	/*
	 * Hole file
	 */
	printf("Testing file Hole file\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	ret = ftruncate(fd, 4096 * 4);
	if (ret < 0) {
		fprintf(stderr, "ftruncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	pos = lseek(fd, 4096, SEEK_DATA);
	if (pos != -1 && errno != ENXIO) {
		ERROR("Seek data did not return proper error");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 512, SEEK_HOLE);
	if (pos != (4096 * 4)) {
		ERROR("Seek hole did not return the end of file");
		close(fd);
		return 1;
	}

	/*
	 * Data - Hole - Data file
	 */
	printf("Testing file big [Data][Hole][Data]\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	ret = ftruncate(fd, 4096 * 1535);
	if (ret < 0) {
		fprintf(stderr, "ftruncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf2, (4096 * 512) - 1, 0);
	if (bytes < (4096*512) - 1) {
		fprintf(stderr, "Failed to write: %d\n", errno);
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf2, 4096 * 510, 4096 * 512 * 2);
	if (bytes < 4096*510) {
		fprintf(stderr, "Failed to write: %d\n", errno);
		close(fd);
		return 1;
	}

	pos = lseek(fd, 4096*512, SEEK_DATA);
	if (pos != 4096*512*2) {
		ERROR("Seek data did not return 4096*512*2");
		close(fd);
		return 1;
	}

	pos = lseek(fd, (4096 * 512) - 2, SEEK_HOLE);
	if (pos != (4096 * 512)) {
		ERROR("Seek hole did not return 4096*512");
		close(fd);
		return 1;
	}
	/*
	 * Small file
	 */
	printf("Testing small file with data\n");
	if (reset_file(fd)) {
		close(fd);
		return 1;
	}

	ret = ftruncate(fd, 8);
	if (ret < 0) {
		fprintf(stderr, "ftruncate failed: %d\n", errno);
		close(fd);
		return 1;
	}

	bytes = pwrite(fd, &buf, 8, 0);
	if (bytes < 8) {
		fprintf(stderr, "Failed to write: %d\n", errno);
		close(fd);
		return 1;
	}

	pos = lseek(fd, 1, SEEK_DATA);
	if (pos != 1) {
		ERROR("Seek data did not return the same position");
		close(fd);
		return 1;
	}

	pos = lseek(fd, 1, SEEK_HOLE);
	if (pos != 8) {
		ERROR("Seek hole did not return the end of file");
		close(fd);
		return 1;
	}
out:
	close(fd);
	return 0;
}
 

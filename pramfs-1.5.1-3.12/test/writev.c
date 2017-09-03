/*
 * PRAMFS: persistent and protected RAM Filesystem
 *
 * Copyright (C) 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <stdio.h>
#include <assert.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <string.h>

#define NR_IOVEC (sizeof(chunks) / sizeof(char *))

int main()
{
  int fd;
  int ret;
  char *chunks[] = { "pramfs is the best", "pramfs", "will this test work? maybe" };
  size_t size;
  struct iovec *iov;
  unsigned int i;
  
  fd = open("/pram/readv", O_CREAT|O_TRUNC|O_RDWR);
  assert(fd != -1);

  iov = (struct iovec *)malloc(NR_IOVEC*sizeof(struct iovec));
  assert(iov != NULL);

  size = 0;
  for (i = 0; i < NR_IOVEC; i ++) {
    iov[i].iov_base = chunks[i];
    iov[i].iov_len = strlen(chunks[i]) + 1;
    size += iov[i].iov_len;
  }
  
  ret = writev(fd, iov, NR_IOVEC);
  assert(size == ret);

  ret = lseek(fd, 0, SEEK_SET);
  assert(ret == 0);

  for (i = 0; i < NR_IOVEC; i ++) {
    iov[i].iov_len = strlen(chunks[i]) + 1;
    iov[i].iov_base = malloc(iov[i].iov_len);
    assert(iov[i].iov_base);
  }
  
  ret = readv(fd, iov, NR_IOVEC);
  assert(ret == size);

  for (i = 0; i < NR_IOVEC; i ++)
    assert(!strcmp(iov[i].iov_base, chunks[i]));

  for (i = 0; i < NR_IOVEC; i ++)
    free(iov[i].iov_base);
  
  free(iov);
  ret = close(fd);
  assert(ret == 0);
  
  ret = unlink("/pram/readv");
  assert(ret == 0);
  return 0;
}

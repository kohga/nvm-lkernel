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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <assert.h>

static int val1;
static int val2;

typedef enum {
  USER,
  TRUSTED,
  SECURITY
} NAMESPACE;

static void print_xattrib_val(const char *path, const char *name)
{
	char buffer[256];
	int n;
	
	n = getxattr(path,name,buffer,sizeof(buffer));
	if(n >= 0){
		uint32_t *b = (uint32_t *)buffer;
		while(n > 0){
			assert(*b == val1 || *b == val2);
			b++;
			n-=sizeof(uint32_t);
		}
	}
}

static void list_xattr(const char *path)
{
	char list[4096];
	int n=0;
	int list_len;
	int len;
	
	list_len = listxattr(path,list,sizeof(list));
	assert(list_len != -1);
	while(n < list_len){
		len = strlen(list + n);
		print_xattrib_val(path,list + n);
		n += (len + 1);
	}
}

void xattr_test(NAMESPACE type)
{
	char name[100];
	int fd;
	int result;

	strcpy(name,"/pram/");
	if (type == USER)
	  strcat(name,"user_file");
	else if (type == TRUSTED)
	  strcat(name,"trusted_file");
	else if (type == SECURITY)
	  strcat(name,"sec_file");

	unlink(name);
	fd = open(name, O_CREAT|O_TRUNC|O_RDWR, S_IREAD|S_IWRITE);
	close(fd);
	
	list_xattr(name);

	val1 = rand() % 1000;
	if (type == USER)
	  result = setxattr(name,"user.foo",&val1,sizeof(val1),0);
	else if (type == TRUSTED)
	  result = setxattr(name,"trusted.foo",&val1,sizeof(val1),0);
	else if (type == SECURITY)
	  result = setxattr(name,"security.foo",&val1,sizeof(val1),0);
	assert(!result);
	list_xattr(name);
	val2 = rand() % 1000;
	if (type == USER)
	  result = setxattr(name,"user.bar",&val2,sizeof(val1),0);
	else if (type == TRUSTED)
	  result = setxattr(name,"trusted.bar",&val2,sizeof(val1),0);
	else if (type == SECURITY)
	  result = setxattr(name,"security.bar",&val2,sizeof(val1),0);
	assert(!result);
	list_xattr(name);
	
	print_xattrib_val(name,"not present");
	list_xattr(name);
	
	result = removexattr(name,"not present");
	assert(result == -1);
	list_xattr(name);

	if (type == USER)
	  result = removexattr(name,"user.foo");
	else if (type == TRUSTED)
	  result = removexattr(name,"trusted.foo");
	else if (type == SECURITY)
	  result = removexattr(name,"security.foo");
	assert(result == 0);
	list_xattr(name);

	if (type == USER)
	  result = removexattr(name,"user.bar");
	else if (type == TRUSTED)
	  result = removexattr(name,"trusted.bar");
	else if (type == SECURITY)
	  result = removexattr(name,"security.bar");
	assert(result == 0);
	list_xattr(name);
}

int main(int argc, char *argv[])
{
	struct timeval val;
	gettimeofday(&val, NULL);
	srand(val.tv_sec);
	xattr_test(USER);
	xattr_test(TRUSTED);
	xattr_test(SECURITY);
}

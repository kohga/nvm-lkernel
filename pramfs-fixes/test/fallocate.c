/*
 * PRAMFS: persistent and protected RAM Filesystem
 *
 * Copyright (C) 2010-2011 Marco Stornelli <marco.stornelli@gmail.com>
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
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

#define FALLOC_FL_PUNCH_HOLE 0x02

void logstats(int fd) {

    struct stat fd_stat ;

    if ( fstat( fd, &fd_stat )  < 0 ) {
        perror( "Could not stat file" );
        exit(1);
    }

    printf( "File stats: \n" );
    printf( "    Length:            %ld\n", fd_stat.st_size );
    printf( "    Block size:        %ld\n", fd_stat.st_blksize );
    printf( "    Blocks allocated:  %ld\n" , fd_stat.st_blocks );

}

int main(int argc, char *argv[]) {

    int mode = 0, flags, fd;
    char *path;
    loff_t offset, len;
    long ret;
    
    if ( argc != 5 ) {
        fprintf( stderr, "SYNTAX: fallocate <file> <mode> <offset> <length>\n" );
        exit(1);
    }

    path = argv[1];

    printf( "Going to fallocate %s\n", path );

    flags = O_RDWR;
    fd = open( path, flags );

    if ( fd == -1 ) {
        perror( "Unable to open file" );
        exit(1);
    }

    logstats(fd);

    len = atol( argv[4] );

    if ( len <= 0 ) {
        fprintf( stderr, "Unable to allocate size %ld\n",len);
        exit( 1 );
    }

    offset = atol( argv[3] );
    
    if ( offset < 0 ) {
        fprintf( stderr, "Unable to use offset %ld\n",offset);
        exit( 1 );
    }

    printf( "Increasing file to: %ld\n",len+offset);

    if (atoi(argv[2]) == 1)
      mode = FALLOC_FL_KEEP_SIZE;
    else if (atoi(argv[2]) == 2)
      mode = FALLOC_FL_PUNCH_HOLE;

    ret = syscall( __NR_fallocate, fd, mode, offset, len );
    if (ret) {
	perror("Error");
	exit(1);
    }

    logstats(fd);
    close( fd );
    return 0;
}

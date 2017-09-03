#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>

#define SYS_haga_syscall  314
#define SYS_commit_syscall  315
#define SIZE  8

void main(){
	unsigned long size=1024000,fsize,i;
	int   fd,val=9;
	long  psize, ret;
	char  c, buf_sys[256];
	int *mapp;
	clock_t start, end;
	//struct timeval s, e;
	//gettimeofday(&s, NULL);
	//struct tms t0,t1;

	size*=SIZE;
	/*
	mapp = (int *)malloc(size*sizeof(int));
	if(mapp == NULL) {
		printf("malloc; erro\n");
	}
	*/

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if ( ret < 0 ) {
		fprintf( stderr, "erro : %ld\n", ret );
	} else {
		//printf( "buf = %s\n", buf_sys );
	}

	//if((fd=open("mapFile",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
	if((fd=open("mapFile",O_RDWR,0666))==-1){
		perror("open");
		exit(-1);
	}

//#ifdef BSD
//	psize=getpagesize();
//#else
//	psize=sysconf(_SC_PAGE_SIZE);
//#endif
	//fsize=(size*sizeof(SSS)/psize+1)*psize;
	//printf("page = %lf\n",(double)fsize/4096);
	fsize = size*4;
	//printf("fsize = %d K\n",(fsize/1024));

	start = clock();
	mapp=(int *)mmap(0,fsize,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if((int)mapp==-1){
		perror("mmap");
		exit(-1);
	}


	syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );
	//times(&t0);
	for(i=0;i<size;i++){
		mapp[i] = val;
	}
	//times(&t1);
	syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );

	if(munmap(mapp,fsize)==-1){
		perror("munmap");
	}
	end = clock();
	close(fd);

	//printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	printf("%f \n",(double)(end-start)/CLOCKS_PER_SEC);

	//gettimeofday(&e, NULL);
	//printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);
	//printf("system times = %ld \nuser time = %ld\n", t1.tms_stime - t0.tms_stime, t1.tms_utime - t0.tms_utime);
	//printf("system times = %f \n",(double)(t1.tms_stime - t0.tms_stime)/sysconf(_SC_CLK_TCK));
	//printf("user time = %f \n",(double)(t1.tms_utime - t0.tms_utime)/sysconf(_SC_CLK_TCK));
	
	syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
}

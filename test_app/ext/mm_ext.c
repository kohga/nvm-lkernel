#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/syscall.h>

#define SYS_kohga_syscall  314

void main(){
	unsigned long page_num=8, psize, size, i, max;
	int   fd, val=1, *mapp;
	char c=0,buf_sys[256];
	clock_t start, end;
	struct timeval s, e;

	start = clock();
	gettimeofday(&s, NULL);

	if( syscall(SYS_kohga_syscall, buf_sys, sizeof(buf_sys)) < 0 ){
		perror("syscall");
		exit(400);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	//if((fd=open("mapfile",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
	if((fd=open("mapfile",O_RDWR|O_CREAT,0666))==-1){
		perror("open");
		exit(401);
	}

	psize=sysconf(_SC_PAGE_SIZE);  // get page_size (if it is notBSD)
	//printf("psize = %lu\n",psize);

	size = psize * page_num;
	printf("mmap size = %lu K\n",(size/1024));

	if(lseek(fd,size,SEEK_SET)<0){
		perror("lseek");
		exit(402);
	}
	if(write(fd,&c,sizeof(char))==-1){
		perror("write");
		exit(-1);
	}

	mapp=(int *)mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(mapp==MAP_FAILED){
		perror("mmap");
		exit(404);
	}

	max = size / sizeof(int);
	for(i=0; i<max; i++){
		mapp[i] = val;
	}

	msync(mapp,size,0);

	if(munmap(mapp,size)==-1){
		perror("munmap");
		exit(405);
	}

	if(close(fd) == -1){
		perror("close");
		exit(406);
	}

	gettimeofday(&e, NULL);
	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);
	//printf("%lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	if( syscall(SYS_kohga_syscall, buf_sys, sizeof(buf_sys)) < 0 ){
		perror("syscall");
		exit(407);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	exit(0);
}

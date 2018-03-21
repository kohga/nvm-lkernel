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

#define MAP_PRAM	0x40
#define MAP_PRAM_ATOMIC	0x80

#define kohga_syscall  314
#define nvmsync  315

#define NUMBER 10

int main(){

	unsigned long psize, size, i;
	int fd;
	char c=0,buf_sys[256];
	char *ptr;
	clock_t start, end;
	struct timeval s, e;

	start = clock();
	gettimeofday(&s, NULL);

	if( syscall(kohga_syscall, buf_sys, sizeof(buf_sys)) < 0 ){
		perror("syscall: kohga_syscall");
		exit(400);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	//if((fd=open("maptxt",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
	if((fd=open("maptxt",O_RDWR|O_CREAT,0666))==-1){
		perror("open");
		exit(401);
	}

	psize=sysconf(_SC_PAGE_SIZE);  // get page_size (if it is notBSD)
	//printf("psize = %lu\n",psize);


	size=NUMBER*psize;
	printf("size = %lu \n",size);
	printf("mmap size = %lu K\n",(size/1024));
/*
	if(lseek(fd,size,SEEK_SET)<0){
		perror("lseek");
		exit(402);
	}
	if(write(fd,&c,sizeof(char))==-1){
		perror("write");
		exit(403);
	}
*/
	ptr=(char *)mmap(0,size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED|MAP_PRAM|MAP_PRAM_ATOMIC,fd,0);
	//ptr=(char *)mmap(0,size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(ptr ==(char *)-1){
		perror("mmap");
		exit(404);
	}
	//now_ptr = ptr;

	for(i=0;i<size-1;i++){
		if(i%64==0)
			ptr[i] = '\0';
		else
			ptr[i] = 'z';
	}

	if( syscall(nvmsync, buf_sys, fd) < 0 ){
		perror("syscall; nvmsync");
		exit(400);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	if(munmap(ptr,size)==-1){
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

	if( syscall(kohga_syscall, buf_sys, sizeof(buf_sys)) < 0 ){
		perror("syscall");
		exit(407);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	exit(0);
}

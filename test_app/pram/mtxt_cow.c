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

#define SYS_kohga_syscall  314

int main(){
	unsigned long page_num=1000, psize, size, i, max;
	int   fd, val=1, *mapp;
	char c=0, buf_sys[256], *ptr;
	clock_t start, end;
	struct timeval s, e;

	if( syscall(SYS_kohga_syscall, buf_sys, sizeof(buf_sys)) < 0 ){
		perror("syscall");
		exit(400);
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	//if((fd=open("test_file",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
	if((fd=open("test_file",O_RDWR|O_CREAT,0666))==-1){
		perror("open");
		exit(401);
	}

	psize=sysconf(_SC_PAGE_SIZE);  // get page_size (if it is notBSD)
	//printf("psize = %lu\n",psize);

	size = page_num * psize;
	printf("mmap size = %lu\n",size);

	start = clock();
	gettimeofday(&s, NULL);

	ptr=(char *)mmap(0,size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	//ptr=(char *)mmap(0,size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED|MAP_PRAM|MAP_PRAM_ATOMIC,fd,0);
	if(ptr ==(char *)-1){
		perror("mmap");
		exit(404);
	}

	int cow_num = 0;
	int count=0;
	char *cow_buf;

	for(i=0; i<size-1; i++){
		ptr[i] = 'a';
		if(i%4096==0 && i!=0 ){
			cow_buf = (char *)malloc(4096);
			count++;
			for(cow_num = 0; cow_num < 4096; cow_num++){
				cow_buf[cow_num] = 'a';
			}
			free(cow_buf);
		}
	}
	ptr[i]='\n';

	cow_buf = (char *)malloc(4096);
	count++;
	for(cow_num = 0; cow_num < 4096; cow_num++){
		cow_buf[cow_num] = 'a';
	}
	free(cow_buf);
	printf("cow count = %d\n", count);

	if(msync(ptr,size,0)==-1){
		perror("msync");
		exit(410);
	}

	if(munmap(ptr,size)==-1){
		perror("munmap");
		exit(405);
	}

	gettimeofday(&e, NULL);
	end = clock();

	if(close(fd) == -1){
		perror("close");
		exit(406);
	}

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

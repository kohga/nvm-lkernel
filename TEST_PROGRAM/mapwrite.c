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

#define SYS_haga_syscall  314
#define SIZE  8

/*
typedef struct{
	char  c;
	long  lval;
	double dval;
}SSS;
*/

void main(){
	unsigned long size=1024000,fsize,i;
	int   fd,val=0;
	long psize/*, lval*/, ret;
	char c, buf_sys[256] /*, buf= 'a'*/;
	double read_d/*, dval = 1.2*/;
	//SSS *ptr;
	int *mapp;
	clock_t start, end;
	struct timeval s, e;

	size*=SIZE;
	
	gettimeofday(&s, NULL);

	start = clock();

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if( ret < 0 ){
		fprintf( stderr, "erro : %ld\n", ret );
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	//if((fd=open("mapFile",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
	if((fd=open("mapFile",O_RDWR|O_CREAT,0666))==-1){
		perror("open");
		exit(-1);
	}

#ifdef BSD
	psize=getpagesize();
#else
	psize=sysconf(_SC_PAGE_SIZE);
#endif
	//fsize=(size*sizeof(int)/psize+1)*psize;
	//printf("fsize = %d\n",fsize);
	//printf("page = %f\n",(double)((double)fsize/4096.0));
	fsize = size*4;
	//printf("fsize = %d\n",fsize);
	printf("fsize = %d K\n",(fsize/1024));

	if(lseek(fd,fsize,SEEK_SET)<0){
		perror("lseek");
		exit(-1);
	}
	if(read(fd,&c,sizeof(char))==-1){
		c='\0';
	}
	if(write(fd,&c,sizeof(char))==-1){
		perror("write");
		exit(-1);
	}
	
	mapp=(int *)mmap(0,fsize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if((int)mapp==-1){
		perror("mmap");
		exit(-1);
	}

	for(i=0;i<size;i++){
		//ptr[i].c = buf;
		//ptr[i].lval = i;
		//ptr[i].dval = dval;
		mapp[i] = val;
	}

	if(munmap(mapp,fsize)==-1){
		perror("munmap");
	}

	close(fd);
	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	gettimeofday(&e, NULL);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
}

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

/*
typedef struct{
	char  c;
	long  lval;
	double dval;
}SSS;
*/

void main(){
	unsigned long size=1024000,fsize,i;
	int   fd;
	long  psize, ret;
	char buf_sys[ 256 ];
	//SSS   *ptr;
	int *mapp;
	clock_t start,end;
	struct timeval s, e;
	gettimeofday(&s, NULL);

	start = clock();

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if( ret < 0 ){
		fprintf( stderr, "erro : %ld\n", ret );
	}else{
		//printf( "buf = %s\n", buf_sys );
	}

	if((fd=open("mapFile",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
		perror("open");
		exit(-1);
	}

#ifdef BSD
	psize=getpagesize();
#else
	psize=sysconf(_SC_PAGE_SIZE);
#endif
	//fsize=(size*sizeof(SSS)/psize+1)*psize;
	//printf("page = %f\n",(double)fsize/4096);
	fsize = size*4;
	printf("fsize = %d\n",fsize);

	mapp=(int *)mmap(0,fsize,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	for(i=0;i<size;i++){
		if (size >= 1000) {
			if(i%(size/100)==0)
				//printf("%5d , %d, %g, %c \n",i,ptr[i].lval,ptr[i].dval,ptr[i].c);
				printf("mapp[ %d ] = %d\n",i,mapp[i]);
		} else {
			//printf("%5d , %d, %g, %c \n",i,ptr[i].lval,ptr[i].dval,ptr[i].c);
			printf("mapp[ %d ] = %d\n",i,mapp[i]);
		}
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

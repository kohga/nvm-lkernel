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

void main(){
	unsigned long size=102400,fsize,i;
	int   fd,val=9;
	long  psize, k, ret;
	char  c, buf_sys[256];
	int *mapp,*buf_cow,calc;
	clock_t start, end;
	//struct timeval s, e;
	//gettimeofday(&s, NULL);
	struct tms t0,t1;

	buf_cow = (int *)malloc(size*sizeof(int));
	if(buf_cow == NULL) {
		printf("malloc; erro\n");
	}

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if ( ret < 0 ) {
		fprintf( stderr, "erro : %ld\n", ret );
	} else {
		//printf( "buf = %s\n", buf_sys );
	}


	if((fd=open("mapFile",O_RDWR,0666))==-1){
		perror("open");
		exit(-1);
	}

#ifdef BSD
	psize=getpagesize();
#else
	psize=sysconf(_SC_PAGE_SIZE);
#endif
	//fsize=(size*sizeof(SSS)/psize+1)*psize;
	//printf("page = %lf\n",(double)fsize/4096);
	fsize = size*4;
	//printf("fsize = %d\n",fsize);
	//printf("fsize = %d K\n",(fsize/1024));

	start = clock();
	mapp=(int *)mmap(0,fsize,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if((int)mapp==-1){
		perror("mmap");
		exit(-1);
	}


	//syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );

	i = 0;
	//times(&t0);
	//printf("01\n");
	while(i < size){

		for(k=0;k<1024;k++){
			//printf("a");
			//buf_cow[i+k].c = ptr[i+k].c;
			//buf_cow[i+k].lval = ptr[i+k].lval;
			//buf_cow[i+k].dval = ptr[i+k].dval;
			buf_cow[i+k] = mapp[i+k];
			if(i+k >= size){
				//printf("** c **\n");
				break;
			}
		}

		for(k=0;k<1024;k++){
			//printf("b");
			//ptr[i].c = buf;
			//ptr[i].lval = size - i;
			//ptr[i].dval = dval;
			mapp[i] = val;
			i++;
			if(i >= size){
				//printf("** c **\n");
				break;
			}
		}
	}
	//printf("02\n");
	//times(&t1);

	//syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );

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

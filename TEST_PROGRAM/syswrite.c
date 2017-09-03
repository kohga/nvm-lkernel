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

void main(){
	unsigned long size=1024000, fsize, i;
	int   fd, val=9, *vp=9, *sysp;
	long psize, ret;
	char c, buf_sys[256] ;
	double read_d;
	clock_t start, end;
	struct timeval s, e;

	size*=SIZE;

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if( ret < 0 ){
		fprintf( stderr, "erro : %ld\n", ret );
	}else{
		//printf( "buf = %s\n", buf_sys );
	}


//#ifdef BSD
//	psize=getpagesize();
//#else
//	psize=sysconf(_SC_PAGE_SIZE);
//#endif
	//fsize=(size*sizeof(int)/psize+1)*psize;
	//printf("fsize = %d\n",fsize);
	//printf("page = %f\n",(double)((double)fsize/4096.0));
//	fsize = size*4;
	//printf("fsize = %d\n",fsize);
	//printf("fsize = %d K\n",(fsize/1024));

	//gettimeofday(&s, NULL);


	sysp = (int *)malloc(size*sizeof(int));
	if(sysp == NULL) {
		printf("malloc; erro\n");
	}
/*
	for(i=0;i<size;i++){
		sysp[i]= val;
	}
*/
	sysp[0] = val;

	//if( (fd=open("mapFile",O_RDWR|O_CREAT|O_DIRECT,0666)) == -1 ){
	if((fd=open("mapFile",O_RDWR|O_CREAT,0666))==-1){
		perror("open");
		exit(-1);
	}

	start = clock();
	//write(fd, sysp, fsize);
	
	/*
	for(i=0;i<size;i++){
		write(fd, sysp, 4);
	}
	*/


	size /= 1000;
	for(i=0;i<size;i++){
		write(fd, sysp, 4000);
	}


	//write(fd, vp, 4);
	end = clock();

	close(fd);
	//printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	printf("%f \n",(double)(end-start)/CLOCKS_PER_SEC);
	
	//gettimeofday(&e, NULL);
	//printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
}

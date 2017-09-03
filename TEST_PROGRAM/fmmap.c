#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
//#include <sys/syscall.h>
#include <time.h>
#include <sys/time.h>

#define FNAME "test1M"
//#define FNAME "test100M"
//#define FNAME "test500M"
//#define FNAME "test1G"
//#define SYS_haga_syscall  314
//#define SYS_commit_syscall  315

int main(){
	unsigned long size = 1048576000;
	int fd;
	unsigned long i;
	long ret;
	char *ptr, *ptr_mun, *b, buf_sys[256];
	clock_t start, end;
	struct timeval s, e;

	b = (char *)malloc(size*sizeof(char));
	if(b == NULL) {
		printf("malloc; erro\n");
	}
	ptr = (char *)malloc(size*sizeof(char));
	if(ptr == NULL) {
		printf("malloc; erro\n");
	}
	ptr_mun = (char *)malloc(size*sizeof(char));
	if(ptr_mun == NULL) {
		printf("malloc; erro\n");
	}

	//printf("tes;00\n");
	for( i=0 ; i < size-1 ; i++ ){
		b[i]='a';
	}
	b[i] = '\0';
	//printf("%s\n",b);

	//printf("tes;01\n");
	/*
	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if ( ret < 0 ) {
		fprintf( stderr, "erro : %ld\n", ret );
	} else {
		//printf( "buf = %s\n", buf_sys );
	}
	*/

	//printf("tes;02\n");
	gettimeofday(&s, NULL);
	start = clock();

	fd = open(FNAME,O_RDWR,0666);
	if(fd < 0){
		printf("Error: open(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	//printf("tes;03\n");
	ptr = ptr_mun = mmap(NULL, size, PROT_WRITE, MAP_SHARED, fd, 0);
	if((int)ptr==-1){
		perror("mmap");
		return(-1);
	}

	printf("tes;04\n");
	//syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );
	for( i=0 ; i < size ; i++ ){
		*ptr++ = b[i];
	}
	//syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );

	printf("tes;05\n");
	if(munmap(ptr_mun,size)==-1){
		perror("munmap");
	}

	//printf("tes;06\n");
	if(close(fd) < 0){
		printf("Error: close(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	//printf("tes;07\n");
	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	//printf("%f \n",(double)(end-start)/CLOCKS_PER_SEC);

	gettimeofday(&e, NULL);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);
	//printf("%lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	//syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	return 0;
}

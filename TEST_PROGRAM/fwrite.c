#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <sys/syscall.h>
#include <time.h>
#include <sys/time.h>

//#define FNAME "test"
#define FNAME "test100M"
//#define FNAME "test1G"
//#define SYS_haga_syscall  314

int main(void){
	unsigned long size = 1048576000;
	int fd;
	unsigned long i;
	char *b,buf_sys[256];
	long ret;
	ssize_t num = 0;
	clock_t start, end;
	struct timeval s, e;

	b = (char *)malloc(size*sizeof(char));
	if(b == NULL) {
		printf("malloc; erro\n");
	}

	//printf("tes;00\n");
	for( i=0 ; i < size-1 ; i++ ){
		b[i]='a';
	}
	b[i] = '\0';
	//printf("%s\n",b);

	/*
	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );
	if ( ret < 0 ) {
		fprintf( stderr, "erro : %ld\n", ret );
	} else {
		//printf( "buf = %s\n", buf_sys );
	}
	*/

	gettimeofday(&s, NULL);
	start = clock();

	//printf("tes;01\n");
	fd = open(FNAME,O_RDWR,0666);
	if(fd < 0){
		printf("Error: open(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	//printf("tes;02\n");
	num = write(fd, b, size);
	if(num < 0){
		printf("Error: write(%d) %s\n", errno, strerror(errno));
		return(-1);
	}


	//printf("tes;03\n");
	if(close(fd) < 0){
		printf("Error: close(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);
	//printf("%f \n",(double)(end-start)/CLOCKS_PER_SEC);

	gettimeofday(&e, NULL);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);
	//printf("%lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	//syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	return 0;
}

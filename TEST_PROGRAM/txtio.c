#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/syscall.h>
#include <time.h>
#include <sys/time.h>

#define NUMBER (1)
#define SYS_haga_syscall  314
//#define BUFSIZE  32

void main(){
	int fd = 0, i;
	char buf_sys[256], *buf="ZZZZZ\n";
	long ret;
	ssize_t num = 0;
	ssize_t cnt = strlen(buf);
	clock_t start, end;
	struct timeval s, e;

/*
	buf[0] = 'A';
	for(i=1;i<BUFSIZE-1;i++){
		if(i%16==0)
			buf[i] = '\n';
		else
			buf[i] = 'Z';
	}
	buf[BUFSIZE-1] = '\n';
*/

	gettimeofday(&s, NULL);
	start = clock();

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	fd = open("writetext.txt",O_RDWR|O_CREAT|O_DIRECT|O_SYNC, 0666);
	if(fd < 0){
		printf("Error: open(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	num = write(fd, buf, cnt);
	if(num < 0){
		printf("Error: write(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	if(close(fd) < 0){
		printf("Error: close(%d) %s\n", errno, strerror(errno));
		return(-1);
	}

	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);

	gettimeofday(&e, NULL);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	return 0;
}

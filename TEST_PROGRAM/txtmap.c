#include  <stdio.h>
#include  <math.h>
#include  <fcntl.h>
#include  <sys/types.h>
#include  <unistd.h>
#include  <sys/mman.h>
#include  <sys/syscall.h>
#include  <time.h>
#include  <sys/time.h>

#define NUMBER (1)
#define SYS_haga_syscall  314
#define SYS_commit_syscall  315

int main(){
	int   fd;
	long  read_l, psize, size, i, ret;
	char  *read_s, c, buf_sys[256], *buf="ZZZZZ\n";
	double read_d;
	char   *ptr;
	clock_t start, end;
	struct timeval s, e;

	gettimeofday(&s, NULL);
	start = clock();

	ret = syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	if((fd=open("Maptext.txt",O_RDWR|O_CREAT|O_DIRECT|O_SYNC,0666))==-1){
		perror("open");
		exit(-1);
	}

#ifdef BSD
	psize=getpagesize();
#else
	psize=sysconf(_SC_PAGE_SIZE);
#endif
	size=(NUMBER*sizeof(char)/psize+1)*psize;
	//printf("page = %f\n",(double)size/4096);

	ptr=(char *)mmap(0,size,PROT_EXEC|PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if((int)ptr==-1){
		perror("mmap");
		exit(-1);
	}

	syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );
	for(i=0;i<NUMBER;i++){
		strcpy(ptr,buf);
	}
	syscall( SYS_commit_syscall, buf_sys, sizeof( buf_sys ) );

	if(munmap(ptr,size)==-1){
		perror("munmap");
	}
	close(fd);

	end = clock();
	printf("clock: %f \n",(double)(end-start)/CLOCKS_PER_SEC);

	gettimeofday(&e, NULL);
	printf("get_time = %lf\n", (e.tv_sec - s.tv_sec) + (e.tv_usec - s.tv_usec)*1.0E-6);

	syscall( SYS_haga_syscall, buf_sys, sizeof( buf_sys ) );

	return 0;
}

#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <assert.h>

#undef __O_TMPFILE
#define __O_TMPFILE     020000000

int main() {

	char *buf = "foo";

	int fd = 0, bytes = 0;
	fd = syscall(__NR_open,"/pram", __O_TMPFILE|O_DIRECTORY|O_RDWR, 0666);
	assert(fd != -1);

	bytes = write(fd, buf, 3);
	assert(bytes == 3);

	close(fd);

	return 0;
}

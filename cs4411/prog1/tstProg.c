/* test driver for wrapping system calls */

#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
int main(int argc, char **argv){
	int to_Write[2048];
	printf("creating file: testFile\n");
	int fd1=open("testFile", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	printf("creating file: testFile\n");
	int fd2=open("testTrunc", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	printf("creating file: testFile\n");
	int fd3=open("testFTrunc", O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
	
	printf("writing data to: testFile\n");
	write(fd1, to_Write, 2048);
	printf("writing data to: testTrunc\n");
	write(fd2, to_Write, 2048);
	printf("writing data to: testFTrunc\n");
	write(fd3, to_Write, 2048);
	
	printf("truncating file")
	truncate("testTrunc", 1024);
	ftruncate(fd3, 1024);
	unlink("testFile");
}

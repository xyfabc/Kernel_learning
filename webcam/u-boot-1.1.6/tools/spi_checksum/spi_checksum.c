#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#define BUFFER_SIZE 4
#define le(a) ((a & 0xff)<<24) | ((a>>8 & 0xff)<< 16) | ((a>>16 & 0xff)<< 8) | ((a>>24 & 0xff))

int main(int argc,char **argv)
{
	int fd, count;
	int bytes_read;
	char buffer[BUFFER_SIZE];
	unsigned int check = 0;
	volatile int t = 0;
	
	if(argc!=2) {
		fprintf(stderr,"Usage: %s fromfile tofile\n\a",argv[0]);
		return 1;
	}

	if((fd=open(argv[1],O_RDWR))==-1) {
		fprintf(stderr,"Open %s Error%s\n",argv[1],strerror(errno));
		return 1;
	}

	count = 0;

	while(bytes_read=read(fd,buffer,BUFFER_SIZE)) {
		if((bytes_read==-1)&&(errno!=EINTR))
			break;
		else if(bytes_read>0)
		{
			if ( t > 3 ){
				check += *(unsigned int *)buffer;
			} else 	
				++t;
			count += 4;
		}
	}
	printf(" count = %d \n", count);
	printf(" check = %#x \n", check);
	
	lseek( fd, 8, SEEK_SET);
	
	if ((t = write(fd, &count, 4)) != 4) {
		fprintf(stderr,"Write %s Error %s\n",argv[1],strerror(errno));
		return 1;
	}

	check = 0-check;
	if ((t = write(fd, &check, 4)) != 4) {
		fprintf(stderr,"Check: Write %s Error %s\n",argv[1],strerror(errno));
		return 1;
	}

#if 0
	lseek( fd, 8, SEEK_SET);

	if ((t = read(fd,buffer,BUFFER_SIZE) < 0)) {
		fprintf(stderr,"read %d \n",t);
	}
	printf("%#x\t", *(unsigned int *)buffer);

	if ((t = read(fd,buffer,BUFFER_SIZE) < 0)) {
		fprintf(stderr,"read %d \n",t);
	}
	printf("%#x\n", *(unsigned int *)buffer);
	
#endif
	close(fd);
	return 0;
}

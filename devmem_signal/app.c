
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <semaphore.h> 
#include <linux/ioctl.h>
#include <linux/poll.h>  

#define DEV_MEM_MEMSET		_IO('M', 0)
#define DEV_MEM_GET_SIZE	_IOR('M', 1, int)
#define DEV_MEM_SET_SIZE	_IOW('M', 2, int)

int fd = -1;

void sig_handle(int sig)
{
	char buf[16] = {0};
	
	printf("read sig [%d]\n", sig);
	lseek(fd, 0, SEEK_SET);
	read(fd, buf, 15);
	printf("read mem:%s\n", buf);
}

int main(int argc, char** argv)
{
	int pid;
	char buf[16] = {0};
  	int mem_size = 0;
	int ret = 0;
	int flags;
	
	fd = open("/dev/dev_mem", O_RDWR);
    if (-1 == fd)
    {
        perror("open error\n");
        return -2;
    }
    printf("open \"dev_mem\" success\n");

	if (SIG_ERR == signal(SIGIO, sig_handle))
    {
         perror("signal error\n");
        return -1;
    }
	
	pid = fork();
	if(pid > 0)
	{
		printf("parent progress [%d] start.\n", getpid());
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		mem_size = 256;
		printf("Set dev memory size [%d]\n", mem_size);
		ioctl(fd, DEV_MEM_SET_SIZE, &mem_size); 
	
		mem_size = 0;
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		ioctl(fd, DEV_MEM_MEMSET); 
		sleep(1);					/* 保证子进程先运行设置异步信号 */
		write(fd, "Hello word", 10);
		//lseek(fd, 0, SEEK_CUR);
		//lseek(fd, 10, SEEK_SET);
		write(fd, " ABCD", 5);
		wait();
	}
	else if(pid == 0)
	{
		printf("son progress [%d] start.\n", getpid());
		fcntl(fd, F_SETOWN, getpid());
    	flags = fcntl(fd, F_GETFL);
    	fcntl(fd, F_SETFL, flags | O_ASYNC);

		sleep(5);
		close(fd);
	}
	else
	{
		perror("fork error");
		close(fd);
	}
	
    return 0;
}

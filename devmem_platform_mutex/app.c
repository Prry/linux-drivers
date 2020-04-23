
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <string.h>
#include <linux/ioctl.h>
#include <sys/mman.h>

#define DEV_MEM_MEMSET		_IO('M', 0)
#define DEV_MEM_GET_SIZE	_IOR('M', 1, int)
#define DEV_MEM_SET_SIZE	_IOW('M', 2, int)

int main(int argc, char** argv)
{
	int pid;
	char buf[16] = {0};
	int fd;
  	int mem_size = 0;

	fd = open("/dev/dev_mem", O_RDWR);
    if (-1 == fd)
    {
        perror("open error");
        return -2;
    }
    printf("open \"dev_mem\" success\n");
	
	//ioctl(fd, DEV_MEM_GET_SIZE, &mem_size);	/* 获取dev_mem 内存空间大小 */
	
	pid = fork();
	if (pid > 0)
	{
		printf("Parent procsss run\n");
		read(fd, buf, 7);
		printf("Read message from dev_mem:%s\n", buf);
		close(fd);
	}
	else if (pid == 0)
	{
		sleep(2);	/* 保证父进程先执行,父进程执行read会被内核挂起，因为没有数据可读 */
		printf("Child procsss run\n");
		printf("Write [Message] to dev_mem\n");
		write(fd, "Message", 7);
		wait(NULL);
	}
	else
	{
		close(fd);
	}
	
    return 0;
}

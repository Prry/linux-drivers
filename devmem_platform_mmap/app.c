
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <string.h>
#include <semaphore.h> 
#include <linux/ioctl.h>
#include <sys/mman.h>

#define DEV_MEM_MEMSET		_IO('M', 0)
#define DEV_MEM_GET_SIZE	_IOR('M', 1, int)
#define DEV_MEM_SET_SIZE	_IOW('M', 2, int)

sem_t  *r_sem;

int main(int argc, char** argv)
{
	int pid;
	char buf[16] = {0};
	int fd;
  	int mem_size = 0;
	char *mmap_mem = NULL; 

	fd = open("/dev/dev_mem", O_RDWR);
    if (-1 == fd)
    {
        perror("open error");
        return -2;
    }
    printf("open \"dev_mem\" success\n");
	
	r_sem = sem_open("notempty", O_CREAT|O_RDWR, 0777, 0);	
	if(r_sem == SEM_FAILED)
	{
		perror("sem_open");
		close(fd);
		return -1;
	}
	
	ioctl(fd, DEV_MEM_GET_SIZE, &mem_size);	/* 获取dev_mem 内存空间大小 */
	
	/* 将设备物理空间映射到进程虚拟空间 */
	mmap_mem = (char *) mmap (NULL, mem_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memset(mmap_mem, 0, mem_size);	/* 清空内存 */
	
	pid = fork();
	if (pid > 0)
	{
		printf("Write [%s] to dev_mem mmap\n", "Parent write message");
		sprintf(mmap_mem, "%s", "Parent write message");
		sem_post(r_sem);
		wait(NULL);
	}
	else if (pid == 0)
	{
		sem_wait(r_sem);
		printf("Read dev_mem mmap:%s\n", mmap_mem);
		close(fd);
	}
	else
	{
		perror("fork");
		close(fd);
	}
	
	/* 释放映射区 */
	munmap(mmap_mem, mem_size);
    return 0;
}

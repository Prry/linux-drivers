
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

	fd = open("/dev/dev_mem", O_RDWR);
    if (-1 == fd)
    {
        perror("open error\n");
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
	
	pid = fork();
	if(pid > 0)
	{
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		mem_size = 256;
		printf("Set dev memory size [%d]\n", mem_size);
		ioctl(fd, DEV_MEM_SET_SIZE, &mem_size); 
	
		mem_size = 0;
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		ioctl(fd, DEV_MEM_MEMSET); 
		write(fd, "Hello word", 10);
		//lseek(fd, 0, SEEK_CUR);
		//lseek(fd, 10, SEEK_SET);
		write(fd, " ABCD", 5);
		sem_post(r_sem);
		wait();
	}
	else if(pid == 0)
	{
		sem_wait(r_sem);
		lseek(fd, 0, SEEK_SET);
		read(fd, buf, 15);
		printf("read mem:%s\n", buf);
		close(fd);
	}
	else
	{
		perror("fork error");
		close(fd);
	}
	
    return 0;
}


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

//sem_t  *r_sem;

int main(int argc, char** argv)

{
	int pid;
	char buf[16] = {0};
	int fd;
  	int mem_size = 0;
	int ret = 0;
	fd_set s_fds;
	struct pollfd p_fds[5];  	/* 最大监听5个句柄 */
	int max_fd; 				/* 监听文件描述符中最大的文件号 */
	
	fd = open("/dev/dev_mem", O_RDWR);
    if (-1 == fd)
    {
        perror("open error\n");
        return -2;
    }
    printf("open \"dev_mem\" success\n");
	
	/* poll 监听参数 */
	for(ret = 0;ret < 5;ret++)
	{
		p_fds[ret].fd = -1;
	}
	p_fds[0].fd = fd;
	p_fds[0].events = POLLIN | POLLRDNORM;
	p_fds[0].revents = 0;
	max_fd = 1;
	
	/*r_sem = sem_open("rsem", O_CREAT|O_RDWR, 0777, 0);
	if(r_sem == SEM_FAILED)
	{
		perror("sem_open");
		close(fd);
		return -1;
	}*/
	
	pid = fork();
	if(pid > 0)
	{
		printf("parent progress start.\n");
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		mem_size = 256;
		printf("Set dev memory size [%d]\n", mem_size);
		ioctl(fd, DEV_MEM_SET_SIZE, &mem_size); 
	
		mem_size = 0;
		ioctl(fd, DEV_MEM_GET_SIZE, &mem_size); 
		printf("Get dev memory size [%d]\n", mem_size);

		ioctl(fd, DEV_MEM_MEMSET); 
		sleep(5);
		write(fd, "Hello word", 10);
		//lseek(fd, 0, SEEK_CUR);
		//lseek(fd, 10, SEEK_SET);
		write(fd, " ABCD", 5);
		//sem_post(r_sem);
		wait();
	}
	else if(pid == 0)
	{
		//sem_wait(r_sem);
		printf("son progress start.\n");
		ret = poll(p_fds,max_fd+1,-1);			/* -1表示阻塞，不超时 */

		if(ret < 0)
		{
			perror("poll error");
			close(fd);
		}

		if((p_fds[0].revents&POLLIN) == POLLIN || (p_fds[0].revents&POLLRDNORM) == POLLRDNORM) 
		{
			lseek(fd, 0, SEEK_SET);
			read(fd, buf, 15);
			printf("read mem:%s\n", buf);
			close(fd);
		}
	}
	else
	{
		perror("fork error");
		close(fd);
	}
	
    return 0;
}

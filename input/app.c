
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
#include <linux/input.h>

int main(int argc, char** argv)
{
	struct input_event key_event;
	int ret = 0;
	int fd = 0;
  	
	
	fd = open("/dev/input/event5", O_RDWR);
    if (fd < 0)
    {
        perror("open error\r\n");
        return -1;
    }
	
	for (;;)
	{
		ret = read(fd, &key_event, sizeof(key_event));
		if (ret > 0)
		{
			printf("Type[0x%02x] Code[0x%02x] Value[0x%02x]\r\n", key_event.type, key_event.code, key_event.value);
		}
		else
		{
			printf("read key input event failed\r\n");
			break;
		}
	}
	close(fd);

    return 0;
}

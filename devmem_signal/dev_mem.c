
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/poll.h>  


#define DEV_MEM_MEMSET		_IO('M', 0)
#define DEV_MEM_GET_SIZE	_IOR('M', 1, int)
#define DEV_MEM_SET_SIZE	_IOW('M', 2, int)

#define MEMORY_DEFULT_SIZE 	128
#define DEV_NAME			"dev_mem"

struct memory_device
{
    struct 	cdev 	dev;
	dev_t			devno;
	struct class 	*devclass;
	char			*mem_buf;
	uint32_t		mem_size;
	wait_queue_head_t r_queue;
	bool			r_en;
	struct fasync_struct *r_fasync;
};

static struct 	memory_device *pmemory_dev = NULL;

static int memory_fasync(int fd, struct file *pfile, int on);

static int memory_open(struct inode * inode , struct file * pfile)
{
	int state = 0;

    if(pmemory_dev == NULL)
    {
		printk("empty memory.\n");
		return -EFAULT;
	}
	
	pmemory_dev->mem_buf = kmalloc(MEMORY_DEFULT_SIZE, GFP_KERNEL);
	if(pmemory_dev->mem_buf)
	{
		pmemory_dev->mem_size = MEMORY_DEFULT_SIZE;
		state = 0;
	}
	else
	{
		state = -1;
		printk("kmalloc request memory falied.\n");
	}

	pfile->private_data = pmemory_dev;
    return state;
}

static ssize_t memory_read(struct file * pfile, char __user *buffer, size_t size, loff_t *offset)
{
	unsigned long of = 0;
	struct memory_device *p;

	p = pfile->private_data;
	of = *offset;
	if(of > p->mem_size)
	{
		return 0;
	}
    if (size > (p->mem_size - of))
    {
    	size = p->mem_size - of;
    }

	wait_event_interruptible(p->r_queue, p->r_en);
	
    if (copy_to_user(buffer, p->mem_buf+of, size))
    {
    	printk("read memory falied.\n");
        return -EFAULT;
    }
	else
	{
		*offset -= size;
	}

	p->r_en = false;
    return size;
}

static ssize_t memory_write(struct file * pfile, const char __user *buffer, size_t size, loff_t *offset)
{
	unsigned long of = 0;
	struct memory_device *p;

	p = pfile->private_data;
	of = *offset;
	if(of > p->mem_size)
	{
		return 0;
	}
   	if (size > (p->mem_size - of))
    {
    	size = p->mem_size -of;
    }
   
    if (copy_from_user(p->mem_buf+of, buffer, size))
    {
    	printk("write memory falied.\n");
        return -EFAULT;
    }
	else
	{
		*offset += size;
	}
	
	p->r_en = true;
    wake_up_interruptible(&(p->r_queue));	/* 唤醒休眠进程(读) */

	/* 通知进程数据可读
	 * SIGIO:信号类型
	 * POLL_IN:普通数据可读
	 */
	kill_fasync(&p->r_fasync, SIGIO, POLL_IN);

    return size;
}

static ssize_t memory_close(struct inode * inode , struct file * pfile)
{
	int state = 0;
	struct memory_device *p;

	p = pfile->private_data;

	if(p->mem_buf)
	{
		kfree(p->mem_buf);
		p->mem_size = 0;
	}

	memory_fasync(-1, pfile, 0);
	
    return state;
}

static int memory_ioctl(struct inode *inode, struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int state = 0;
	int temp = 0;
	char *pmem = NULL;
	struct memory_device *p;

	p = pfile->private_data;
	switch(cmd)
	{
		case DEV_MEM_MEMSET:
			memset(p->mem_buf, 0, p->mem_size);
		break;
		
		case DEV_MEM_GET_SIZE:
			if(copy_to_user((int __user*)arg, &p->mem_size, 4)) 
			{
				return -ENOTTY;
			}
		break;

		case DEV_MEM_SET_SIZE:
			if(copy_from_user(&temp, (int __user*)arg, 4))
			{
				return -ENOTTY;
			}
			if(temp != p->mem_size)
			{
				pmem = kmalloc(temp, GFP_KERNEL);
				if(pmem)
				{
					kfree(p->mem_buf);
					p->mem_buf = NULL;
					p->mem_size = temp;
					p->mem_buf = pmem;
				}
			}
		break;

		default:
		break;
	}
	
	return state;
}

static loff_t memory_llseek(struct file *pfile, loff_t offset, int whence)
{ 
    loff_t of;
	struct memory_device *p;
	
	p = pfile->private_data;
    switch(whence) 
	{
      case SEEK_SET: 
        of = offset;
        break;

      case SEEK_CUR:
        of = pfile->f_pos + offset;
        break;

      case SEEK_END:
        of = p->mem_size -1 + offset;
        break;

      default: 
        return -EINVAL;
    }
    if ((of<0) || (of>p->mem_size))
    	return -EINVAL;
    	
    pfile->f_pos = of;
	
    return of;
}


static unsigned int memory_poll(struct file *pfile, poll_table *wait)
{
	unsigned int mask = 0;
	struct memory_device *p;
	
	p = pfile->private_data;
    poll_wait(pfile, &p->r_queue, wait);	/* 将进程挂在'r_queue'队列上 */
    if(p->r_en)         
	{
		mask |= POLLIN | POLLRDNORM;  		/* 数据可读 */
    }
	
    return mask;
}


/* 当应用程序调用了fcntl(fd, F_SETFL, Oflags | FASYNC); 
 * 则最终会调用驱动的fasync函数
 * fasync_helper函数的作用是初始化/释放fasync_struct
 */
static int memory_fasync(int fd, struct file *pfile, int on)
{
	unsigned int mask = 0;
	struct memory_device *p;
	
	p = pfile->private_data;
	return fasync_helper(fd, pfile, on, &p->r_fasync);
}


static const struct file_operations memory_fops = 
{
    .owner 	 = THIS_MODULE,
    .open 	 = memory_open,
    .read    = memory_read,
    .write   = memory_write,
    .release = memory_close,
    .ioctl   = memory_ioctl,
    .llseek  = memory_llseek,
    .poll    = memory_poll,
    .fasync  = memory_fasync,
};

static int __init memory_init(void)
{
    int ret = -1;
	dev_t	devno = 0;

	ret = alloc_chrdev_region(&devno, 0, 1, "dev_mem"); 
	if (ret)
	{
		printk("alloc dev-no failed.\n");
		return ret;
	}
	
    pmemory_dev = kmalloc(sizeof(struct memory_device), GFP_KERNEL);
    if (NULL == pmemory_dev)
    {
        ret = -ENOMEM;
        printk("kmalloc request memory falied.\n");
		return ret;
    }
    memset(pmemory_dev, 0, sizeof(struct memory_device));

	pmemory_dev->devno = devno; 
    cdev_init(&pmemory_dev->dev, &memory_fops);
    pmemory_dev->dev.owner 	= THIS_MODULE;
    pmemory_dev->dev.ops 	= &memory_fops;
    ret = cdev_add(&pmemory_dev->dev, pmemory_dev->devno, 1);
    if (ret)
    {
    	unregister_chrdev_region(pmemory_dev->devno, 1);
    	kfree(pmemory_dev);
        return ret;
    }

    pmemory_dev->devclass = class_create(THIS_MODULE, "mem_class");
	if (IS_ERR(pmemory_dev->devclass)) 
	{
		printk("class_create failed.\n");
		cdev_del(&pmemory_dev->dev);
		ret = -EIO;
		return ret;
	}
	
    device_create(pmemory_dev->devclass, NULL, pmemory_dev->devno, NULL, "dev_mem");

	init_waitqueue_head(&pmemory_dev->r_queue);
    return 0;
}

static void __exit memory_exit(void)
{
    device_destroy(pmemory_dev->devclass, pmemory_dev->devno);
    class_destroy(pmemory_dev->devclass);
    cdev_del(&pmemory_dev->dev);
    unregister_chrdev_region(pmemory_dev->devno, 1);
	kfree(pmemory_dev->mem_buf);
	pmemory_dev->devclass = NULL;
	pmemory_dev->mem_buf = NULL;
	pmemory_dev->mem_size = 0;
	kfree(pmemory_dev);
	pmemory_dev = NULL;
}

module_init(memory_init);
module_exit(memory_exit);
MODULE_LICENSE("GPL");



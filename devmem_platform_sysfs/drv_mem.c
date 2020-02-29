
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
#include <linux/uaccess.h>		//#include <asm/uaccess.h> 
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
        #include <asm/switch_to.h>
#else
        #include <asm/system.h>
#endif


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
	struct kobject *mem_obj;
};

static struct 	memory_device *pmemory_dev = NULL;

static int memory_drv_open(struct inode * inode , struct file * pfile)
{
	int state = 0;

    if(pmemory_dev == NULL)
    {
		printk("empty memory.\n");
		return -EFAULT;
	}
	#if 0
	if((pmemory_dev->mem_buf) && (pmemory_dev->mem_size))
	{/* 已申请内存，返回 */
		kfree(pmemory_dev->mem_buf);
		pmemory_dev->mem_buf = NULL;
		pmemory_dev->mem_size = 0;
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
	#endif
	
	pfile->private_data = pmemory_dev;
    return state;
}

static ssize_t memory_drv_read(struct file * pfile, char __user *buffer, size_t size, loff_t *offset)
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
	
    if (copy_to_user(buffer, p->mem_buf+of, size))
    {
    	printk("read memory falied.\n");
        return -EFAULT;
    }
	else
	{
		*offset -= size;
	}
    return size;
}

static ssize_t memory_drv_write(struct file * pfile, const char __user *buffer, size_t size, loff_t *offset)
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

    return size;
}

static int memory_drv_close(struct inode * inode , struct file * pfile)
{
	int state = 0;

#if 0
	struct memory_device *p;

	p = pfile->private_data;

	if(p->mem_buf)
	{
		kfree(p->mem_buf);
		p->mem_buf = NULL;
		p->mem_size = 0;
	}
#endif
    return state;
}

static long memory_drv_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
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

static loff_t memory_drv_llseek(struct file *pfile, loff_t offset, int whence)
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


static const struct file_operations memory_fops = 
{
    .owner 	 = THIS_MODULE,
    .open 	 = memory_drv_open,
    .read    = memory_drv_read,
    .write   = memory_drv_write,
    .release = memory_drv_close,
    .unlocked_ioctl = memory_drv_ioctl,
    .llseek = memory_drv_llseek,
};


static ssize_t memory_drv_cat(struct device *dev,struct device_attribute *attr, char *buf)  
{
    char *s = buf;

	if(pmemory_dev->mem_buf == NULL)
	{
		return -1;
	}
    sprintf(s, "%s", pmemory_dev->mem_buf);
	
    return sizeof(s);
}

static ssize_t memory_drv_echo(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	if(pmemory_dev->mem_buf == NULL)
	{
		return -1;
	}

    memcpy(pmemory_dev->mem_buf, buf, count);
    return count;
}

static DEVICE_ATTR(mem_bin, 0660, memory_drv_cat, memory_drv_echo);	/* 创建‘mem_bin’文件，链接访问接口 */


static int  memory_drv_probe(struct platform_device *pdev)
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
    //pmemory_dev->dev.owner 	= THIS_MODULE;
    //pmemory_dev->dev.ops 	= &memory_fops;
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

#if 0 /* 自定义目录'/sys/mem_sys' */
	pmemory_dev->mem_obj = kobject_create_and_add("mem_sys", kernel_kobj->parent);	
	if(!pmemory_dev->mem_obj)
	{
		return -ENOMEM;
	}

	sysfs_create_file(pmemory_dev->mem_obj, &dev_attr_mem_bin.attr);
#else	/* 映射到 '/bus/platform/device'目录下 */
	sysfs_create_file(&pdev->dev.kobj, &dev_attr_mem_bin.attr);
#endif

	pmemory_dev->mem_buf = kmalloc(MEMORY_DEFULT_SIZE, GFP_KERNEL);
	if(pmemory_dev->mem_buf)
	{
		memset(pmemory_dev->mem_buf, 0, MEMORY_DEFULT_SIZE);
		pmemory_dev->mem_size = MEMORY_DEFULT_SIZE;
	}

    return 0;
	
SYSFS_ERR:
    kobject_del(pmemory_dev->mem_obj);
    kobject_put(pmemory_dev->mem_obj);
	return ret;
}

static int memory_drv_remove(struct platform_device *pdev)
{
	if(pmemory_dev == NULL)
	{
		return -1;
	}
	device_destroy(pmemory_dev->devclass, pmemory_dev->devno);
    class_destroy(pmemory_dev->devclass);
    cdev_del(&pmemory_dev->dev);
    unregister_chrdev_region(pmemory_dev->devno, 1);
	if(pmemory_dev->mem_buf)
	{
		kfree(pmemory_dev->mem_buf);
	}
	pmemory_dev->devclass = NULL;
	pmemory_dev->mem_buf = NULL;
	pmemory_dev->mem_size = 0;

#if 0
	sysfs_remove_file(pmemory_dev->mem_obj, &dev_attr_mem_bin.attr);
	kobject_del(pmemory_dev->mem_obj);
    kobject_put(pmemory_dev->mem_obj);
#else
	sysfs_remove_file(&pdev->dev.kobj, &dev_attr_mem_bin.attr);
#endif
	kfree(pmemory_dev);
	pmemory_dev = NULL;

	return 0;
}

static struct platform_driver memory_drv = 
{
    .driver = 
    {
        .name  = "dev_mem",
        .owner = THIS_MODULE,
    },
    .probe = memory_drv_probe,
    .remove = memory_drv_remove,
};
		

static int __init memory_drv_init(void)
{
	return platform_driver_register(&memory_drv);
}

static void __exit memory_drv_exit(void)
{
    platform_driver_unregister(&memory_drv);
}

module_init(memory_drv_init);
module_exit(memory_drv_exit);
MODULE_LICENSE("GPL");



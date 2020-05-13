
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
#include <linux/uaccess.h>		
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/input.h>
#include <linux/bits.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
        #include <asm/switch_to.h>
#else
        #include <asm/system.h>
#endif
#include "dev_input.h"

#define KEY_INPUT_NAME		"dev_input"

struct key_input_device
{
	uint8_t key_value;
	struct input_dev *input;
	struct kobject *obj;
};

static struct 	key_input_device key_input_dev;

static ssize_t input_drv_cat(struct device *dev,struct device_attribute *attr, char *buf)  
{
	buf[0] = key_input_dev.key_value;
	
    return 1;
}

static ssize_t input_drv_echo(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	key_input_dev.key_value = buf[0];
	if (buf[0] == 0x31)
	{
		input_report_key(key_input_dev.input, KEY_0, 0X01); /* 按键按下 */
		input_sync(key_input_dev.input);
	}
	else if (buf[0] == 0x30)
	{
		input_report_key(key_input_dev.input, KEY_0, 0X00); /* 按键抬起 */
		input_sync(key_input_dev.input);
	}
	printk("key value [%d]\r\n", buf[0]);
    return count;
}

static DEVICE_ATTR(input_bin, 0660, input_drv_cat, input_drv_echo);	/* 创建‘input_bin’文件，链接访问接口 */

static int  input_drv_probe(struct platform_device *pdev)
{
    int ret = -1;

	key_input_dev.input = input_allocate_device();
	key_input_dev.input->name = KEY_INPUT_NAME;
	key_input_dev.input->evbit[0] = BIT_MASK(EV_KEY)/* | BIT_MASK(EV_REP)*/; /* 不创建重复事件 */
	input_set_capability(key_input_dev.input, EV_KEY, KEY_0);	/* 设置键值 */
	
	ret = input_register_device(key_input_dev.input);
	if (ret != 0)
	{
		printk("input device register failed.\r\n");
		return ret;
	}
#if 0 /* 自定义目录'/sys/input_sys' */
	key_input_dev.obj = kobject_create_and_add("inpu_sys", kernel_kobj->parent);	
	if(!key_input_dev.obj)
	{
		return -ENOMEM;
	}
	
	/*创建 attr_grop*/
	ret = sysfs_create_group(key_input_dev.obj, &attr_group);
	if(ret != 0)
	{
		goto SYSFS_ERR;
	}
#else	/* 映射到 '/bus/platform/device'目录下 */
	sysfs_create_file(&pdev->dev.kobj, &dev_attr_input_bin.attr);
#endif

    return 0;
	
SYSFS_ERR:
    kobject_del(key_input_dev.obj);
    kobject_put(key_input_dev.obj);
	
	return ret;
}

static int input_drv_remove(struct platform_device *pdev)
{
	input_unregister_device(key_input_dev.input);
	input_free_device(key_input_dev.input);
	kobject_del(key_input_dev.obj);
    kobject_put(key_input_dev.obj);

	return 0;
}

static struct platform_driver key_input_drv = 
{
    .driver = 
    {
        .name  = "dev_input",
        .owner = THIS_MODULE,
    },
    .probe = input_drv_probe,
    .remove = input_drv_remove,
};
		
static int __init input_drv_init(void)
{
	input_dev_init();
	return platform_driver_register(&key_input_drv);
}

static void __exit input_drv_exit(void)
{
	input_dev_exit();
    platform_driver_unregister(&key_input_drv);
}

module_init(input_drv_init);
module_exit(input_drv_exit);
MODULE_LICENSE("GPL");



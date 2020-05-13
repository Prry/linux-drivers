
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
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/platform_device.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(3, 3, 0)
        #include <asm/switch_to.h>
#else
        #include <asm/system.h>
#endif


struct platform_device input_device = {
 .name   = "dev_input", 
 .id    = -1,
 .dev = 
	 {
	  
	 },
};
  
int __init input_dev_init(void)
{
    platform_device_register(&input_device);

    return 0;
}

void __exit input_dev_exit(void)
{
    platform_device_unregister(&input_device);
}


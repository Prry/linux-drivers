
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
#include <linux/delay.h> 
#include <linux/i2c.h>
#include "bmp180.h"

#define BMP180_SLAVE_ADDR		0x77 
#define BMP_REG_CHIP_ID			0xD0
#define BMP_REG_RESET			0xE0
#define BMP_REG_CTRL_ADDR		0xF4
#define BMP_REG_AD_MSB			0xF6
#define BMP_REG_AD_LSB			0xF7
#define BMP_REG_AD_XLSB			0xF8
#define BMS_CAL_AC1				0xAA

#define BMP_REG_CTRL_TEMP		0x2E
#define BMP_REG_CTRL_POSS0		0x34
#define BMP_REG_CTRL_POSS1		0x74
#define BMP_REG_CTRL_POSS2		0xB4
#define BMP_REG_CTRL_POSS3		0xF4
#define BMP_REG_RESET_VALUE		0xB6

#define BMP180_DEV_NAME			"bmp180"		

struct bmp180_dev
{
	struct 	cdev 	bm_cdev;
	dev_t			bm_id;
	struct class 	*bm_class;
	struct i2c_client *bm_i2c_client;
};

struct bmp180_calc
{
	short ac1;
	short ac2;
	short ac3;
	short b1;
	short b2;
	short mb;
	short mc;
	short md;
	unsigned short ac4;
	unsigned short ac5;
	unsigned short ac6;	
};
struct bmp180_dev  bmp180_device;
struct bmp180_calc bmp180_clac_parm = {0};  
static int bmp180_read_regs(struct bmp180_dev *pdev, uint8_t reg, uint8_t *pdata, int size)
{
	int ret = 0;
	struct i2c_msg msg[2];
	struct i2c_client *client = NULL;

	if(size == 0)
	{
		return 0;
	}
	client = (struct i2c_client*)pdev->bm_i2c_client;
	msg[0].addr  = client->addr;  	 
    msg[0].buf   = &reg;               
    msg[0].len   = 1;                     
    msg[0].flags = 0; 
	
	msg[1].addr  = client->addr;  	 
    msg[1].buf   = pdata;               
    msg[1].len   = size;                     
    msg[1].flags = I2C_M_RD; 

	if(i2c_transfer(client->adapter, msg, 2) != 2)
	{
		ret =-1;
	}

	return ret;
}

#if 0
static int bmp180_write_regs(struct bmp180_dev *pdev, uint8_t reg, uint8_t *pdata, int size)
{
	
	int ret = 0;
	struct i2c_msg msg[2];
	struct i2c_client *client = NULL;

	if(size == 0)
	{
		return 0;
	}
	client = (struct i2c_client*)pdev->bm_i2c_client;
	msg[0].addr  = client->addr;	 
	msg[0].buf	 = &reg;			   
	msg[0].len	 = 1;					  
	msg[0].flags = 0; 
	
	msg[1].addr  = client->addr;	 
	msg[1].buf	 = pdata;				
	msg[1].len	 = size;					 
	msg[1].flags = 0 | I2C_M_NOSTART; 	/* nostart */

	if(i2c_transfer(client->adapter, msg, 2) != 2)
	{
		ret =-1;
	}
	
	return ret;
}
#endif
static int bmp180_write_regs(struct bmp180_dev *pdev, uint8_t reg, uint8_t *pdata, int size)
{
	
	int ret = 0;
	struct i2c_msg msg[2];
	struct i2c_client *client = NULL;
	uint8_t *tx_buf=NULL;
	
	if(size == 0)
	{
		return 0;
	}
	tx_buf = kmalloc(size+1, GFP_KERNEL);
	if(tx_buf == NULL)
	{
		printk("no memory\n");
		return -1;
	}
	tx_buf[0] = reg;
	memcpy(&tx_buf[1], pdata, size);
	client = (struct i2c_client*)pdev->bm_i2c_client;
	msg[0].addr  = client->addr;	 
	msg[0].buf	 = tx_buf;		   
	msg[0].len	 = size+1;					  
	msg[0].flags = 0; 
	
	if(i2c_transfer(client->adapter, msg, 1) != 1)
	{
		ret =-1;
	}

	kfree(tx_buf);
	return ret;
}

static uint8_t bmp180_read_reg(struct bmp180_dev *pdev, uint8_t reg)
{
	uint8_t data = 0;

	bmp180_read_regs(pdev, reg, &data, 1);
	return data;
}

static int8_t bmp180_write_reg(struct bmp180_dev *pdev, uint8_t reg, uint8_t data)
{
	return bmp180_write_regs(pdev, reg, &data, 1);
}

static long bmp180_read_ut(struct bmp180_dev *pdev)
{
	uint8_t buf[2] = {0};
	long   data = 0;
	
	bmp180_write_reg(pdev, BMP_REG_CTRL_ADDR, BMP_REG_CTRL_TEMP);
	mdelay(6);	/* max conversion time: 4.5ms */
	bmp180_read_regs(pdev, BMP_REG_AD_MSB, buf, 2);
	data = (buf[0]<<8) | buf[1];
	
	return data;
}

static long bmp180_read_up(struct bmp180_dev *pdev)
{
	uint8_t buf[2] = {0};
	long   data = 0;
	
	bmp180_write_reg(pdev, BMP_REG_CTRL_ADDR, BMP_REG_CTRL_POSS0);
	mdelay(6);	/* max conversion time: 4.5ms */
	bmp180_read_regs(pdev, BMP_REG_AD_MSB, buf, 2);
	data = (buf[0]<<8) | buf[1];
	
	return data;
}

static int bmp180_open(struct inode *inode, struct file *pfile) 
{ 
	int ret = 0;
	uint8_t bmbuf[22] = {0};
	
	pfile->private_data = &bmp180_device;

	/* read register */
	bmp180_read_regs(pfile->private_data, BMS_CAL_AC1, bmbuf, 22);
	bmp180_clac_parm.ac1 = (bmbuf[0]<<8)|bmbuf[1];
	bmp180_clac_parm.ac2 = (bmbuf[2]<<8)|bmbuf[3];
	bmp180_clac_parm.ac3 = (bmbuf[4]<<8)|bmbuf[5];
	bmp180_clac_parm.ac4 = (bmbuf[6]<<8)|bmbuf[7];
	bmp180_clac_parm.ac5 = (bmbuf[8]<<8)|bmbuf[9];
	bmp180_clac_parm.ac6 = (bmbuf[10]<<8)|bmbuf[11];
	bmp180_clac_parm.b1 = (bmbuf[12]<<8)|bmbuf[13];
	bmp180_clac_parm.b2 = (bmbuf[14]<<8)|bmbuf[15];
	bmp180_clac_parm.mb = (bmbuf[16]<<8)|bmbuf[17];
	bmp180_clac_parm.mc = (bmbuf[18]<<8)|bmbuf[19];
	bmp180_clac_parm.md = (bmbuf[20]<<8)|bmbuf[21];

	printk("bmp180 opened\n");
	return ret;
} 

static ssize_t bmp180_read(struct file *pfile, char __user *buf, size_t size, loff_t * offset) 
{ 
	int ret = 0;
	struct bmp180_dev *pdev;
	long x1, x2, b5, b6, x3, b3, p;
	unsigned long b4, b7;
	short temperature=0;
	long ut,up,pressure=0;

	pdev= pfile->private_data;
	
	ut = bmp180_read_ut(pdev);
	up = bmp180_read_up(pdev);
	printk("ut=%ld\n", ut);
	printk("up=%ld\n", up);

	/* temperature calc */
	x1 = (((long)ut - (long)bmp180_clac_parm.ac6)*(long)bmp180_clac_parm.ac5) >> 15;
  	x2 = ((long)bmp180_clac_parm.mc << 11) / (x1 + bmp180_clac_parm.md);
  	b5 = x1 + x2;
  	temperature = ((b5 + 8) >> 4);

	/* pressure calc */
	b6 = b5 - 4000;
	x1 = (bmp180_clac_parm.b2 * (b6 * b6)>>12)>>11;
	x2 = (bmp180_clac_parm.ac2 * b6)>>11;
	x3 = x1 + x2;
	b3 = (((((long)bmp180_clac_parm.ac1)*4 + x3)<<0) + 2)>>2;
	
	x1 = (bmp180_clac_parm.ac3 * b6)>>13;
	x2 = (bmp180_clac_parm.b1 * ((b6 * b6)>>12))>>16;
	x3 = ((x1 + x2) + 2)>>2;
	b4 = (bmp180_clac_parm.ac4 * (unsigned long)(x3 + 32768))>>15;
	b7 = ((unsigned long)(up - b3) * (50000>>0));
	if (b7 < 0x80000000)
	{
		p = (b7<<1)/b4;
	}
	else
	{
		p = (b7/b4)<<1;
	}
	x1 = (p>>8) * (p>>8);
	x1 = (x1 * 3038)>>16;
	x2 = (-7357 * p)>>16;
	pressure = p+((x1 + x2 + 3791)>>4);	

	if(copy_to_user(buf, (const uint8_t*)&temperature, sizeof(temperature)))
	{
		ret = -1;
		printk("copy to user failed\n");
	}
	if (copy_to_user(buf+sizeof(temperature), (const uint8_t*)&pressure, sizeof(pressure)))
	{
		ret = -1;
		printk("copy to user failed\n");
	}
	return ret;
}

static ssize_t bmp180_write(struct file *pfile, const char __user *buf, size_t size, loff_t *offset) 
{ 
     return 0; 
} 

static long bmp180_ioctl(struct file *pfile, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct bmp180_dev *p;
	uint8_t chip_id = 0x00;
	
	p = pfile->private_data;
	switch(cmd)
	{
		case BMP_RESET_CHIP:
			bmp180_write_reg(p, BMP_REG_RESET, BMP_REG_RESET_VALUE);
		break;
		case BMP_GET_CHIP_ID:
			chip_id = bmp180_read_reg(p, BMP_REG_CHIP_ID);
			ret = copy_to_user((char __user*)arg, (char __user*)&chip_id, 1); 
		break;
		default:
		break;
	}

	return ret;
}

static int bmp180_release(struct inode * inode , struct file * pfile) 
{ 
     return 0; 
} 

static struct file_operations bmp180_fops = { 
     .owner = THIS_MODULE, 
     .open  = bmp180_open, 
     .read  = bmp180_read, 
     .write = bmp180_write, 
     .release = bmp180_release,
     .unlocked_ioctl = bmp180_ioctl,
}; 
   
static int bmp180_probe(struct i2c_client *client, const struct i2c_device_id *dev_id)  
{     
    struct device *dev; 
	int ret = -1;
	dev_t	id = 0;
	
	ret = alloc_chrdev_region(&id, 0, 1, BMP180_DEV_NAME); 
	if (ret)
	{
		printk("alloc dev-id failed.\n");
		return ret;
	}

	bmp180_device.bm_id = id; 
    cdev_init(&bmp180_device.bm_cdev, &bmp180_fops);
	ret = cdev_add(&bmp180_device.bm_cdev, bmp180_device.bm_id, 1);
    if (ret)
    {
    	unregister_chrdev_region(bmp180_device.bm_id, 1);
        return ret;
    }

	bmp180_device.bm_class = class_create(THIS_MODULE, "bmp180_class");
	if (IS_ERR(bmp180_device.bm_class)) 
	{
		printk("class_create failed.\n");
		cdev_del(&bmp180_device.bm_cdev);
		ret = -EIO;
		return ret;
	}

	dev = device_create(bmp180_device.bm_class, NULL, bmp180_device.bm_id, NULL, BMP180_DEV_NAME);
	if (IS_ERR(dev))   
    {   
         return PTR_ERR(dev);    
    }
	bmp180_device.bm_i2c_client = client;
	
   	return 0; 
} 
  
static int bmp180_remove(struct i2c_client *client) 
{ 
	device_destroy(bmp180_device.bm_class, bmp180_device.bm_id);
    class_destroy(bmp180_device.bm_class);
    cdev_del(&bmp180_device.bm_cdev);
    unregister_chrdev_region(bmp180_device.bm_id, 1);
	
    return 0; 
} 
  
static const struct i2c_device_id bmp180_id[] = { 
     {"bmp180", BMP180_SLAVE_ADDR}, 
     {} 
}; 
MODULE_DEVICE_TABLE(i2c, bmp180_id); 
  
static struct of_device_id of_bmp180_ids[] = {
   {.compatible = "bosch,bmp180"},
   { }   
 };
 
static struct i2c_driver bmp180_driver = { 
	.driver   = { 
	.owner    = THIS_MODULE, 
	.name     = BMP180_DEV_NAME, 
	.of_match_table = of_bmp180_ids,
	}, 
	.id_table = bmp180_id, 
	.probe 	  = bmp180_probe, 
	.remove   = bmp180_remove, 
};

static int __init bmp180_init(void) 
{ 
     i2c_add_driver(&bmp180_driver);
     return 0; 
} 
  
static void __exit bmp180_exit(void) 
{ 
     i2c_del_driver(&bmp180_driver);
} 
  
module_init(bmp180_init); 
module_exit(bmp180_exit); 
  
MODULE_LICENSE("GPL"); 

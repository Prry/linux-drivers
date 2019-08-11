#ifndef _24Cxx_DEV_H_
#define _24Cxx_DEV_H_

#include <stdint.h>

/*operation state*/
enum _24CXX_ERROR
{
	_24CXX_OK 				= 0,
	_24CXX_ERR_MEM			= -1,
	_24CXX_ERR_I2C_WR 		= 1,
	_24CXX_ERR_DEV_NONE 	= 2,
	_24CXX_ERR_PAGE_WRITE	= 3,
	_24CXX_ERR_PAGE_SIZE	= 4,
	_24CXX_ERR_CHIP_SIZE	= 5,
};

/*eeprom model*/
typedef enum 
{
  	_24C01_E = 1,
	_24C02_E,
	_24C04_E,
	_24C08_E,
	_24C16_E,
	_24C32_E,
	_24C64_E,
	_24C128_E,
	_24C256_E,
	_24C512_E,
	_24C1024_E,
}_24_model_t;

/*linux ioctl cmd*/
#define I2C_RETRIES     		0x0701     	/*设置收不到ACK时的重试次数*/
#define I2C_TIMEOUT    		 	0x0702     	/*设置超时时限的jiffies*/
#define I2C_SLAVE       		0x0703		/*设置从机地址*/
#define I2C_SLAVE_FORCE 		0x0706      /*强制设置从机地址*/
#define I2C_TENBIT      		0x0704      /*选择地址位长:=0 for 7bit , != 0 for 10 bit*/
#define I2C_FUNCS       		0x0705      /*获取适配器支持的功能*/
#define I2C_RDWR        		0x0707      /*Combined R/W transfer (one STOP only) */
#define I2C_PEC         		0x0708    	/*!= 0 to use PEC with SMBus*/
#define I2C_SMBUS       		0x0720      /*SMBus transfer*/

/*i2c flag*/
#define I2C_BUS_WR            0x0000
#define I2C_BUS_RD            (1u << 0)
#define I2C_BUS_ADDR_10BIT    (1u << 2)  /* this is a ten bit chip address */
#define I2C_BUS_NO_START      (1u << 4)
#define I2C_BUS_IGNORE_NACK   (1u << 5)
#define I2C_BUS_NO_READ_ACK   (1u << 6)  /* when I2C reading, we do not ACK */

//the message of i2c device 
struct i2c_dev_message
{
    unsigned short  addr;
    unsigned short  flags;
    unsigned short  size;
    unsigned char   *buff;
};

struct i2c_rdwr_ioctl_data 
{
    struct i2c_dev_message	*msgs;
    int nmsgs;
};

/*24cxx eeprom devcie struct*/
typedef struct 
{
	char  					*i2c_dev;	/*i2c bus device*/
	uint8_t					slave_addr;	/*eeprom i2c addr*/
	_24_model_t				model;		/*eeprom model*/
	void(*wp)(uint8_t ctrl);			/*protect of write function*/
	void(*page_write_delay)(void);		/*there is a delay in continuous writin for EEPROM,FRAM not need*/
	
}_24cxx_dev_t;

/*extern function*/
extern int16_t _24cxx_read(_24cxx_dev_t *pdev,uint32_t addr, uint8_t *pbuf, uint32_t size);
extern int16_t _24cxx_write(_24cxx_dev_t *pdev,uint32_t addr,uint8_t *pbuf,uint32_t size);
extern int16_t _24cxx_erase(_24cxx_dev_t *pdev,uint32_t addr,uint8_t data, uint32_t size);

#endif


#include <stdint.h>
#include "_24cxx_dev.h"	
#include "fm24clxx.h"

#define	USE_24CXX_TEST		/*test enable*/


/**
 * @brief  device init
 */
const _24cxx_dev_t fm24clxx_dev =
{
	"/dev/i2c/0",		/*i2c device name*/
	0x50,				/*fram address*/
	_24C16_E,			/*eeprom model,eg FM24C16*/
	0,					/*no write protect*/
	0,					/*delay fun,FRAM no need*/
};


/**
 * @brief  test
 * @param  none
 * @retval none
 */
#ifdef	USE_24CXX_TEST
#include <string.h>
#include <stdio.h>

void data_printf(uint8_t *buf,int addr,int size)
{	
    int i;	
    
    for(i = 0; i < size; i++)	
    {	
        if( (i % 16) == 0 ) 		
        {
            printf("\n %.4x|  ", addr);	
        }
        else if( (i % 8) == 0 ) 			
        {
            printf("  ");	
        }
        printf("%.2x ", *(buf+i));	
        addr++;		
   }	
    printf("\r\n");
}

#define		WR_SIZE			(2*1024)
#define		WR_ADDR			0

static uint8_t write_buf[WR_SIZE];
static uint8_t read_buf[WR_SIZE];

int16_t _24cxx_test(void)
{
	uint32_t i,error = 0;
	_24cxx_dev_t *p = (_24cxx_dev_t*)&fm24clxx_dev;	
	
	_24cxx_read(p,WR_ADDR,read_buf,WR_SIZE); 
       printf("first read:\r\n");
	data_printf(read_buf, WR_ADDR, WR_SIZE);
    
	for( i=0; i<WR_SIZE; i++)  
	{   
		write_buf[i] = i;   
	}
	error = _24cxx_write(p,WR_ADDR, write_buf, WR_SIZE);	 
	_24cxx_read(p,WR_ADDR, read_buf, WR_SIZE); 
        printf("read write:\r\n");
       data_printf(read_buf, WR_ADDR, WR_SIZE);
       
	memset(write_buf,0,sizeof(write_buf));  
	error = _24cxx_write(p,WR_ADDR, write_buf, WR_SIZE);	
	_24cxx_read(p,WR_ADDR, read_buf,WR_SIZE); 
        printf("read \'memset\':\r\n");
	data_printf(read_buf, WR_ADDR, WR_SIZE);
    
	return error;
}

int main(void)
{
        _24cxx_test();

        return 0;
}
#endif


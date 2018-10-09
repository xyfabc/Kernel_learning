#ifndef __SIMULATE_I2C_H__
#define __SIMULATE_I2C_H__

/* #ifndef u8 */
/* #define u8	unsigned char */
/* #endif */


/* I2C control */
//#define	    DEVCLK 12000000// 11200000
#define	    DEVCLK 11200000

/* I2C protocol */
#define I2C_READ	1
#define I2C_WRITE	0
#define TIMEOUT         50000
/* error code */
#define ETIMEDOUT	1
#define ENODEV		2

int simulate_i2c_read(unsigned char device, unsigned char *buf,unsigned char offset, int count);
int simulate_i2c_write(unsigned char device, unsigned char *buf, unsigned char offset, int count);

#endif

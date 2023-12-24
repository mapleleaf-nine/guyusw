#ifndef _I2C_TRANSFER_H_
#define _I2C_TRANSFER_H_

#include <stdint.h>

#define REG_ADDR16       2
#define REG_ADDR24       4

#define I2C_10BIT        1

struct i2c_handler
{
    int32_t fd;
    char filename[32];
    uint32_t bus; 
};

void *libi2c_config(uint32_t bus);
int libi2c_write(struct i2c_handler *h, uint16_t dev_addr, uint32_t reg_addr, uint8_t *data_buf, uint16_t data_len, uint8_t flags);
int libi2c_read(struct i2c_handler *h, uint16_t dev_addr, uint32_t reg_addr, uint8_t *data_buf, uint16_t data_len, uint8_t flags);
void libi2c_release(struct i2c_handler *h)

#endif /* _I2C_TRANSFER_H_ */
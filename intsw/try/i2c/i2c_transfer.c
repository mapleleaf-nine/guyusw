#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include "i2c_transfer.h"


void *libi2c_config(uint32_t bus)
{
    //malloc 返回的 void* 指针;可以隐式转换为任何其他指针类型
    // 1. struct i2c_handler *h = malloc(sizeof(struct i2c_handler));

    // 2. struct i2c_handler *h = NULL;
    // *h 表示 h 指针所指向的对象的类型。
    // 在类型发生变化时，不需要修改 sizeof 的参数，因为它会自动根据指针类型进行推断。
    // h = malloc(sizeof(*h));
    // 3.
    struct i2c_handler *h = (struct i2c_handler *)malloc(sizeof(struct i2c_handler));
    if(!h)
    {
        return NULL;
    }

    h->fd = -1;
    h->bus = bus;
    snprintf(h->filename, sizeof(h->filename), "/dev/i2c-%u", h->bus);

    h->fd = open(h->filename, O_RDWR);
    if(h->fd == -1)
    {
        free(h);
        return NULL;
    }

    return h;
}

int libi2c_write(struct i2c_handler *h, uint16_t dev_addr, uint32_t reg_addr, uint8_t *data_buf, uint16_t data_len, uint8_t flags)
{
    int ret = 0;
    uint8_t addr_buf_local[9];
    uint8_t *addr_buf_heap = NULL;
    uint8_t *addr_buf = addr_buf_local;
    uint8_t addr_len;
    uint8_t kflags;

    struct i2c_msg msg;
    struct i2c_rdwr_ioctl_data ioc; 

    if(!h)
    {
        //非法参数
        return -EINVAL;
    }

    //dev_addr 7bits or 10bits
    if(flags & I2C_10BIT)
    {
        kflags = I2C_M_TEN;
    }
    else
    {
        kflags = 0;
    }

    //reg_dev 8bits or 16bits or 24bits
    if(flags & REG_ADDR24)
    {
        addr_len = 3;
        addr_buf[0] = (reg_addr >> 16) & 0xff;
        addr_buf[1] = (reg_addr >> 8) & 0xff;
        addr_buf[2] = reg_addr & 0xff;
    }
    else if(flags & REG_ADDR16)
    {
        addr_len = 2;
        addr_buf[0] = (reg_addr >> 8) & 0xff;
        addr_buf[1] = reg_addr & 0xff;
    }
    else
    {
        addr_len = 1;
        addr_buf[0] = reg_addr & 0xff;
    }

    if((addr_len + data_len) > sizeof(addr_buf_local))
    {
        addr_buf_heap = malloc(addr_len + data_len);
        if(!addr_buf_heap)
        {
            return -ENOMEM;
        }
        addr_buf = addr_buf_heap;
    }

    memcpy(&addr_buf[addr_len], data_buf, data_len);

    msg.addr = dev_addr;
    msg.flags = kflags;
    msg.len = addr_len + data_len;
    msg.buf = addr_buf;

    ioc.msgs = &msg;
    ioc.nmsgs = 1;

    ret = ioctl(h->fd, I2C_RDWR, &ioc);
    if(ret == -1)
    {
        printf("ioctl: %s\n", strerror(errno));
    }
    // ret = (ioctl(h->fd, I2C_RDWR, &ioc) == -1 ? -errno : 0);

    free(addr_buf_heap);
    return ret;
}

int libi2c_read(struct i2c_handler *h, uint16_t dev_addr, uint32_t reg_addr, uint8_t *data_buf, uint16_t data_len, uint8_t flags)
{
    int ret = 0;
    uint8_t addr_buf[4];
    uint8_t addr_len;
    uint8_t kflags;

    struct i2c_msg msg[2];
    struct i2c_rdwr_ioctl_data ioc; 

    if(!h)
    {
        //非法参数
        return -EINVAL;
    }

    //dev_addr 7bits or 10bits
    if(flags & I2C_10BIT)
    {
        kflags = I2C_M_TEN;
    }
    else
    {
        kflags = 0;
    }

    //reg_dev 8bits or 16bits or 24bits
    if(flags & REG_ADDR24)
    {
        addr_len = 3;
        addr_buf[0] = (reg_addr >> 16) & 0xff;
        addr_buf[1] = (reg_addr >> 8) & 0xff;
        addr_buf[2] = reg_addr & 0xff;
    }
    else if(flags & REG_ADDR16)
    {
        addr_len = 2;
        addr_buf[0] = (reg_addr >> 8) & 0xff;
        addr_buf[1] = reg_addr & 0xff;
    }
    else
    {
        addr_len = 1;
        addr_buf[0] = reg_addr & 0xff;
    }

    msg[0].addr = dev_addr,
    msg[0].flags = kflags;
    msg[0].len = addr_len;
    msg[0].buf = addr_buf;

    msg[1].addr = dev_addr,
    msg[1].flags = kflags | I2C_M_RD;
    msg[1].len = data_len;
    msg[1].buf = data_buf;

    ioc.msgs = msg;
    ioc.nmsgs = 2;

    ret = ioctl(h->fd, I2C_RDWR, &ioc);
    if(ret == -1)
    {
        printf("ioctl: %s\n", strerror(errno));
    }

    return ret;
}

void libi2c_release(struct i2c_handler *h)
{
    if(h)
    {
        close(h->fd);
        free(h);
    }
}
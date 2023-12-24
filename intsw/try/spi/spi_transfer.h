#ifndef _SPI_TRANSFER_H_
#define _SPI_TRANSFER_H_

#include <stdint.h>

struct spi_handler
{
    int32_t fd;             //文件描述符
    char filename[64];
    uint32_t bus;    
    uint32_t cs;            //片选
};

//配置SPI总线
struct spi_config
{
    uint8_t mode;           //spi模式 
    uint8_t bits_per_word;  //每个通信字的字长
    uint32_t speed_hz;      //时钟频率
};

//spi_ioc_transfer传输数据结构体
// 因为spi ioc transfer只是用来传输数据的结构体，而不是用来配置SPl总线的函数。
// 因此，需要在使用spi_ioc transfer之前，先使用ioctl函数来配置一些SPI总线的参数，比如数据位数、时钟频率等。

void *libspi_config(uint32_t bus, uint32_t cs, struct spi_config *cfg);

int libspi_transfer(struct spi_handler *h, uint32_t len, struct spi_ioc_transfer spi_ioc_tr[]);

void libspi_release(struct spi_handler *h);

int8_t spi_ad9009_transfer(uint32_t bus, uint32_t cs, uint16_t addr, uint8_t *data, uint8_t wr_flag);

#endif /* _SPI_TRANSFER_H_ */
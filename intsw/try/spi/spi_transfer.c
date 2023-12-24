#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include "spi_transfer.h"

//SPI device config
void *libspi_config(uint32_t bus, uint32_t cs, struct spi_config *cfg)
{
    int ret = 0;
    struct spi_handler *h = (struct spi_handler *)malloc(sizeof(struct spi_handler));
    if(!h)
    {
        return NULL;
    }

    h->fd = -1;
    h->bus = bus;
    h->cs = cs;

    snprintf(h->filename, sizeof(h->filename), "/dev/spidev%u.%u", h->bus, h->cs);

    h->fd = open(h->filename, O_RDWR);
    if(h->fd = -1)
    {
        free(h);
        return NULL;
    }

    //spi mode
    ret = ioctl(h->fd, SPI_IOC_WR_MODE, &cfg->mode);
    if(ret == -1)
    {
        printf("ioctl: %s\n", strerror(errno));
        close(h->fd);
        free(h);
        return NULL;
    }

    //spi bits per word
    if(cfg->bits_per_word)
    {
        ret = ioctl(h->fd, SPI_IOC_WR_BITS_PER_WORD, &cfg->bits_per_word);
        if(ret == -1)
        {
            printf("ioctl: %s\n", strerror(errno));
            close(h->fd);
            free(h);
            return NULL;
        }
    }

    //spi max speed hz
    if(cfg->speed_hz)
    {
        ret = ioctl(h->fd, SPI_IOC_WR_MAX_SPEED_HZ, &cfg->speed_hz);
        if(ret == -1)
        {
            printf("ioctl: %s\n", strerror(errno));
            close(h->fd);
            free(h);
            return NULL;
        }
    }

    return h;
}

//spi transfer
int libspi_transfer(struct spi_handler *h, uint32_t len, struct spi_ioc_transfer spi_ioc_tr[])
{
    int ret = 0;
    if(!h)
    {
        return -1;
    }

    //spi transfer data stream
    ret = ioctl(h->fd, SPI_IOC_MESSAGE(len), spi_ioc_tr);
    if(ret == -1)
    {
        printf("ioctl: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

void libspi_release(struct spi_handler *h)
{
    if(h)
    {
        close(h->fd);
        free(h);
    }
}

int8_t spi_ad9009_transfer(uint32_t bus, uint32_t cs, uint16_t addr, uint8_t *data, uint8_t wr_flag)
{
    int ret = 0;
    struct spi_handler *spi_handle;
    struct spi_config slave_cfg;
    struct spi_ioc_transfer tr[1];

    uint8_t dout[32] = {0,};   //addrbuf + databuf
    uint8_t din[32] = {0,};    //写字节，spi同时也会回读，最大spi传输字节数为32

    slave_cfg.bits_per_word = 0;    /* Use device default */
    slave_cfg.mode = SPI_MODE_0;
    slave_cfg.speed_hz = 0;     /* Use device default */
    spi_handle = libspi_config(bus, cs, &slave_cfg);
    if(!spi_handle)
    {
        libspi_release_slave(spi_handle);
        return -1;
    }

    //如果第一个字节的最高位判断spi读或者写
    if(wr_flag)
    {
        // dout[0] = 0x80; //10000000  read
        dout[0] = (1 << 7) | ((addr >> 8) & 0x7f);
    }
    else
    {
        // dout[0] = 0x00; //00000000  write
        dout[0] = (addr >> 8) & 0x7f;
    }

    dout[1] = addr & 0xff;
    dout[2] = &data;

    tr[0].tx_buf = (unsigned long)&dout;
    tr[0].rx_buf = (unsigned long)&din;
    tr[0].len = 3;
    tr[0].speed_hz = slave_cfg.speed_hz;
    tr[0].bits_per_word = slave_cfg.bits_per_word;
    tr[0].delay_usecs = 0;
    tr[0].cs_change = 0;
    tr[0].tx_nbits = 0;
    tr[0].rx_nbits = 0;
    tr[0].pad = 0;

    ret = libspi_transfer(spi_handle, 1, tr);
    if(ret) {
        printf("spi_sd9009_transfer_single failed.\n");
        libspi_release_slave(spi_handle);
        return -1;
    }
    
    return ret;
}


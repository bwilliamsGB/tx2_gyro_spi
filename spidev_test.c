/*
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 * Cross-compile with cross-gcc -I/path/to/cross-kernel/include
 */

#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev3.0";
static uint32_t mode = 0;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay = 0;
static int verbose;
static int global_fd;
static int fd_open = 0;

uint8_t default_tx[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0x40, 0x00, 0x00, 0x00, 0x00, 0x95,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xF0, 0x0D,
};

uint8_t default_rx[ARRAY_SIZE(default_tx)] = {0, };
char *input_tx;

void hex_dump(const void *src, size_t length, size_t line_size, char *prefix)
{
	int i = 0;
	const unsigned char *address = src;
	const unsigned char *line = address;
	unsigned char c;

	printf("%s | ", prefix);
	while (length-- > 0) {
		printf("%02X ", *address++);
		if (!(++i % line_size) || (length == 0 && i % line_size)) {
			if (length == 0) {
				while (i++ % line_size)
					printf("__ ");
			}
			printf(" | ");  /* right close */
			while (line < address) {
				c = *line++;
				printf("%c", (c < 33 || c == 255) ? 0x2E : c);
			}
			printf("\n");
			if (length > 0)
				printf("%s | ", prefix);
		}
	}
}

/*
 *  Unescape - process hexadecimal escape character
 *      converts shell input "\x23" -> 0x23
 */
int unescape(char *_dst, char *_src, size_t len)
{
	int ret = 0;
	char *src = _src;
	char *dst = _dst;
	unsigned int ch;

	while (*src) {
		if (*src == '\\' && *(src+1) == 'x') {
			sscanf(src + 2, "%2x", &ch);
			src += 4;
			*dst++ = (unsigned char)ch;
		} else {
			*dst++ = *src++;
		}
		ret++;
	}
	return ret;
}

void old_transfer(uint8_t const *tx, uint8_t const *rx, size_t len)
{
	int ret;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = len,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	if (mode & SPI_TX_QUAD)
		tr.tx_nbits = 4;
	else if (mode & SPI_TX_DUAL)
		tr.tx_nbits = 2;
	if (mode & SPI_RX_QUAD)
		tr.rx_nbits = 4;
	else if (mode & SPI_RX_DUAL)
		tr.rx_nbits = 2;
	if (!(mode & SPI_LOOP)) {
		if (mode & (SPI_TX_QUAD | SPI_TX_DUAL))
			tr.rx_buf = 0;
		else if (mode & (SPI_RX_QUAD | SPI_RX_DUAL))
			tr.tx_buf = 0;
	}

	ret = ioctl(global_fd, SPI_IOC_MESSAGE(len), &tr);
	//printf("ioctl returned %d\n", ret);
	if (ret < 1)
		pabort("can't send spi message");
}


int write_read(uint32_t num_out_bytes, uint8_t *out_buffer,
             uint32_t num_in_bytes, uint8_t *in_buffer)
{

    uint8_t dummy[256];
    memset((void *)dummy, 0, 256);

    old_transfer(out_buffer, dummy, 1);
    memset((void *)dummy, 0, 256);
    old_transfer(dummy, in_buffer, 1);
    return 1;






    struct spi_ioc_transfer mesg[2] = { 0, };
    uint8_t num_tr = 0;
    int ret;

    // initialize memory
    for (int i = 0; i < 2; i++) {
        memset((void *)&(mesg[i]), 0, sizeof(struct spi_ioc_transfer));
    }

    if((out_buffer != NULL) && (num_out_bytes != 0))
    {
        mesg[0].tx_buf = (uint64_t)out_buffer;
        mesg[0].rx_buf = (uint64_t)dummy;
        mesg[0].len = num_out_bytes;
        mesg[0].speed_hz = 100000;
        mesg[0].delay_usecs = 0;
        mesg[0].bits_per_word = 8;
        mesg[0].cs_change = 0;
        num_tr++;
    }

    if((in_buffer != NULL) && (num_in_bytes != 0))
    {
        mesg[1].tx_buf = (unsigned long)dummy;
        mesg[1].rx_buf = (unsigned long)in_buffer;
        mesg[1].len = num_in_bytes;
        mesg[1].speed_hz = 100000;
        mesg[1].delay_usecs = 0;
        mesg[1].bits_per_word = 8;
        mesg[1].cs_change = 1;
        num_tr++;
    }

    if(num_tr > 0)
    {
        // pass into SPI IOC MESSAGE the number of transfers
        ret = ioctl(global_fd, SPI_IOC_MESSAGE(2), &mesg);
        printf("ioctl returned %d\n", ret);
        if(ret == 1)
        {
            return 1;
        }
    }

    return 0;
}

int write_write(uint32_t num_out1_bytes, uint8_t *out1_buffer,
             uint32_t num_out2_bytes, uint8_t *out2_buffer)
{
    uint8_t dummy[256];
    memset((void *)dummy, 0, 256);


    old_transfer(out1_buffer, dummy, num_out1_bytes);
    old_transfer(out2_buffer, dummy, num_out2_bytes);
    return 1;








    struct spi_ioc_transfer mesg[2] = { 0, };
    for (int i = 0; i < 2; i++) {
        memset((void *)&(mesg[i]), 0, sizeof(struct spi_ioc_transfer));
    }

    uint8_t num_tr = 0;
    int ret;


    if((out1_buffer != NULL) && (num_out1_bytes != 0))
    {
        mesg[0].tx_buf = (uint64_t)out1_buffer;
        mesg[0].rx_buf = (uint64_t)dummy;
        mesg[0].len = num_out1_bytes;
        mesg[0].speed_hz = 100000;
        mesg[0].delay_usecs = 0;
        mesg[0].bits_per_word = 8;
        mesg[0].cs_change = 0;
        num_tr++;
    }

    if((out2_buffer != NULL) && (num_out2_bytes != 0))
    {
        mesg[1].tx_buf = (uint64_t)out2_buffer;
        mesg[1].rx_buf = (uint64_t)dummy;
        mesg[1].len = num_out2_bytes;
        mesg[1].speed_hz = 100000;
        mesg[1].delay_usecs = 0;
        mesg[1].bits_per_word = 8;
        mesg[1].cs_change = 1;
        num_tr++;
    }

    if(num_tr > 0)
    {
        // pass into SPI IOC MESSAGE the number of transfers
        ret = ioctl(global_fd, SPI_IOC_MESSAGE(2), &mesg);
        printf("ioctl returned %d\n", ret);
        if(ret == 1)
        {
            return 1;
        }
    }

    return 0;
}

void init_spi()
{
    int ret = 0;
	uint8_t *tx;
	uint8_t *rx;
	int size;

    /* only initialize once */
    if (fd_open == 1) {
        return;
    }
    fd_open = 1;

    global_fd = open(device, O_RDWR);
    if (global_fd < 0)
	    pabort("can't open device");

    /*
     * spi mode
     */
    ret = ioctl(global_fd, SPI_IOC_WR_MODE32, &mode);
    if (ret == -1)
	    pabort("can't set spi mode");

    ret = ioctl(global_fd, SPI_IOC_RD_MODE32, &mode);
    if (ret == -1)
	    pabort("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(global_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
	    pabort("can't set bits per word");

    ret = ioctl(global_fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
	    pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(global_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
	    pabort("can't set max speed hz");

    ret = ioctl(global_fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
	    pabort("can't get max speed hz");

    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
}

void close_spi() {
    close(global_fd);
}





#if 0
int main(int argc, char *argv[])
{
	int ret = 0;
	int fd;
	uint8_t *tx;
	uint8_t *rx;
	int size;

	parse_opts(argc, argv);

	fd = open(device, O_RDWR);
	if (fd < 0)
		pabort("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE32, &mode);
	if (ret == -1)
		pabort("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE32, &mode);
	if (ret == -1)
		pabort("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		pabort("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		pabort("can't get max speed hz");

	printf("spi mode: 0x%x\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

 	if (input_tx) {
		size = strlen(input_tx+1);
		tx = malloc(size);
		rx = malloc(size);
		size = unescape((char *)tx, input_tx, size);
		transfer(fd, tx, rx, size);
		free(rx);
		free(tx);
	} else {
		transfer(fd, default_tx, default_rx, sizeof(default_tx));
	}

	close(fd);

	return ret;
}
#endif

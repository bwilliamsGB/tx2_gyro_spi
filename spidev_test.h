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

void init_spi();
void close_spi();
void hex_dump(const void *src, size_t length, size_t line_size, char *prefix);
int unescape(char *_dst, char *_src, size_t len);
void spi_transfer(uint8_t const *tx, uint8_t const *rx, size_t len);


/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <drivers/flash.h>
#include <device.h>
#include <devicetree.h>
#include <stdio.h>
#include <string.h>

#define FLASH_DEVICE DT_LABEL(DT_INST(0, jedec_spi_nor))
#define FLASH_NAME "JEDEC SPI-NOR"

#define FLASH_TEST_REGION_OFFSET 0xff000
#define FLASH_SECTOR_SIZE        4096

void main(void)
{
	const uint8_t expected[] = { 0x12, 0x34, 0xab, 0x66, 0x99, 0x33, 0x45 };
	const size_t len = sizeof(expected);
	uint8_t buf[sizeof(expected)];
	const struct device *flash_dev;
	int rc;

	printf("\n------------------8<-------------------\n");
	printf(FLASH_NAME " SPI flash testing\n");

	flash_dev = device_get_binding(FLASH_DEVICE);

	if (!flash_dev) {
		printf("SPI flash driver %s was not found!\n",
		       FLASH_DEVICE);
		return;
	}

	/* Write protection needs to be disabled before each write or
	 * erase, since the flash component turns on write protection
	 * automatically after completion of write and erase
	 * operations.
	 */
	printf("\nTest 1: Flash erase\n");

	rc = flash_erase(flash_dev, FLASH_TEST_REGION_OFFSET,
			 FLASH_SECTOR_SIZE);
	if (rc != 0) {
		printf("Flash erase failed! %d\n", rc);
	} else {
		printf("Flash erase succeeded!\n");
	}

	printf("\nTest 2: Flash write\n");

	printf("Attempting to write %zu bytes\n", len);
	rc = flash_write(flash_dev, FLASH_TEST_REGION_OFFSET, expected, len);
	if (rc != 0) {
		printf("Flash write failed! %d\n", rc);
		return;
	}

	memset(buf, 0, len);
	rc = flash_read(flash_dev, FLASH_TEST_REGION_OFFSET, buf, len);
	if (rc != 0) {
		printf("Flash read failed! %d\n", rc);
		return;
	}

	if (memcmp(expected, buf, len) == 0) {
		printf("Data read matches data written. Good!!\n");
	} else {
		const uint8_t *wp = expected;
		const uint8_t *rp = buf;
		const uint8_t *rpe = rp + len;

		printf("Data read does not match data written!!\n");
		while (rp < rpe) {
			printf("%08x wrote %02x read %02x %s\n",
			       (uint32_t)(FLASH_TEST_REGION_OFFSET + (rp - buf)),
			       *wp, *rp, (*rp == *wp) ? "match" : "MISMATCH");
			++rp;
			++wp;
		}
	}
	printf("------------------>8-------------------\n");
}

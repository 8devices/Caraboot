/* 
 * Copyright (c) 2014 Qualcomm Atheros, Inc.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 * 
 */

#include <common.h>
#include <config.h>
#include <asm/types.h>
#include <flash.h>

/*
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long
flash_get_geom (flash_info_t *flash_info)
{
	int i;
	int ret;
	u32 flash_size, sect_size;
	u8 erase_cmd;

	ret = qca_sf_sfdp_info(0, &flash_size, &sect_size, &erase_cmd);
	flash_info->use_4byte_addr = 0;
	flash_info->read_cmd = SPI_FLASH_CMD_READ;
	flash_info->page_program_cmd = SPI_FLASH_CMD_PP;

	if (ret == 0) {
		flash_info->flash_id = FLASH_M25P64;
		flash_info->size = flash_size; /* bytes */
		flash_info->sector_size = sect_size;
		flash_info->sector_count = flash_size / sect_size;
		flash_info->erase_cmd = erase_cmd;
		if (flash_info->size > 16*1024*1024) {
			//TODO get info from sfdp
			flash_info->use_4byte_addr = 1;
			flash_info->erase_cmd = SPI_FLASH_CMD_4SE;
			flash_info->read_cmd = SPI_FLASH_CMD_4READ;
			flash_info->page_program_cmd = SPI_FLASH_CMD_4PP;
		}
	}
	else {
		flash_info->flash_id = FLASH_M25P64;
		flash_info->size = CFG_DEFAULT_FLASH_SIZE; /* bytes */
		flash_info->sector_size = CFG_DEFAULT_FLASH_SECTOR_SIZE;
		flash_info->sector_count = flash_info->size / CFG_DEFAULT_FLASH_SECTOR_SIZE;
		flash_info->erase_cmd = ATH_SPI_CMD_SECTOR_ERASE;
	}

	for (i = 0; i < flash_info->sector_count; i++) {
		flash_info->start[i] = CFG_FLASH_BASE +
			(i * flash_info->sector_size);
		flash_info->protect[i] = 0;
	}

	debug ("flash size %dB, sector count = %d\n",
			flash_info->size, flash_info->sector_count);

	return (flash_info->size);
}

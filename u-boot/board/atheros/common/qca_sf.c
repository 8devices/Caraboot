/*
 * Qualcomm/Atheros Serial SPI FLASH driver utilizing SHIFT registers
 *
 * Copyright (C) 2015 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2016 Mantas Pucka <mantas@8devices.com>
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <flash.h>
#include <asm/addrspace.h>
#include <soc/qca_soc_common.h>

/* Use CS0 by default */
static u32 qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;

inline void qca_sf_spi_en(void)
{
	qca_soc_reg_write(QCA_SPI_FUNC_SEL_REG, 1);
}

inline void qca_sf_spi_di(void)
{
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);
	/* Second write: single write is not enough to flush register */
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);

	qca_soc_reg_write(QCA_SPI_FUNC_SEL_REG, 0);
}

static inline u32 qca_sf_shift_in(void)
{
	return qca_soc_reg_read(QCA_SPI_SHIFT_DATAIN_REG);
}

/*
 * Shifts out 'bits_cnt' bits from 'data_out' value
 * If 'terminate' is zero, then CS is not driven high at end of transaction
 */
static void qca_sf_shift_out(u32 data_out, u32 bits_cnt, u32 terminate)
{
	u32 reg_val = 0;

	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);

	/* Data to shift out */
	qca_soc_reg_write(QCA_SPI_SHIFT_DATAOUT_REG, data_out);

	reg_val = reg_val | bits_cnt
					  | qca_sf_cs_mask
					  | QCA_SPI_SHIFT_CNT_SHIFT_EN_MASK;

	if (terminate)
		reg_val = reg_val | QCA_SPI_SHIFT_CNT_TERMINATE_MASK;

	/* Enable shifting in/out */
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, reg_val);
}

static u32 qca_sf_sfdp_bfpt_dword(u32 ptp_offset, u32 dword_num)
{
	u32 data_out;

	data_out = (SPI_FLASH_CMD_SFDP << 24);
	data_out = data_out | (ptp_offset + ((dword_num - 1) * 4));

	qca_sf_shift_out(data_out, 32, 0);
	qca_sf_shift_out(0x0, 40, 1);

	return cpu_to_le32(qca_sf_shift_in());
}

static inline void qca_sf_write_en(void)
{
	qca_sf_shift_out(SPI_FLASH_CMD_WREN, 8, 1);
}

static inline void qca_sf_write_di(void)
{
	qca_sf_shift_out(SPI_FLASH_CMD_WRDI, 8, 1);
}

/* Poll status register and wait till busy bit is cleared */
static void qca_sf_busy_wait(void)
{
	volatile u32 data_in;

	/* Poll status register continuously (keep CS low during whole loop) */
	qca_sf_shift_out(SPI_FLASH_CMD_RDSR, 8, 0);

	do {
		qca_sf_shift_out(0x0, 8, 0);
		data_in = qca_sf_shift_in() & 0x1;
	} while (data_in);

	/* Disable CS chip */
	qca_sf_shift_out(0x0, 0, 1);
}

/* Returns flash configuration register that is accessible with command 'cmd' */
u8 qca_sf_read_reg(u8 cmd) 
{
	qca_sf_shift_out(cmd << 8, 16, 1);
	return qca_sf_shift_in();
}

/* Writes 'data' to flash configuration register that has wirite command 'cmd' */
void qca_sf_write_reg(u8 cmd, u8 data) 
{
	qca_sf_write_en();
	qca_sf_shift_out(cmd, 8, 0);
	qca_sf_shift_out(data, 8, 1);
	qca_sf_busy_wait();
	qca_sf_write_di();
}

static void qca_sf_set_addressing(flash_info_t *info, u32 enable)
{
	if (info->need_4byte_enable_op) {
		if (enable)
			qca_sf_shift_out(SPI_FLASH_CMD_EN4B, 8, 1);
		else {
			qca_sf_shift_out(SPI_FLASH_CMD_EX4B, 8, 1);
			/* Set extended address reg to 0 to access
			 * lower 16MB when using HW mapping */
			qca_sf_write_reg(SPI_FLASH_CMD_WEAR, 0x0);
		}
	}
}

static void qca_sf_bank_to_cs_mask(u32 bank)
{
	switch (bank) {
	case 0:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	case 1:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS1_MASK;
		break;
	case 2:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS2_MASK;
		break;
	default:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	}
}

void qca_sf_prepare_cmd(u8 cmd, u32 addr, u32 is_4byte, u32 buf[2])
{
	if (is_4byte) {
		buf[0] = cmd << 24;
		buf[0] = buf[0] | (addr >> 8);
		buf[1] = (addr & 0x000000FF);
	}
	else {
		buf[0] = cmd << 24;
		buf[0] = buf[0] | (addr & 0x00FFFFFF);
	}
}

/* Bulk (whole) FLASH erase */
void qca_sf_bulk_erase(u32 bank)
{
	qca_sf_bank_to_cs_mask(bank);
	qca_sf_spi_en();
	qca_sf_write_en();
	qca_sf_shift_out(SPI_FLASH_CMD_ES_ALL, 8, 1);
	qca_sf_busy_wait();
	qca_sf_spi_di();
}

/* Erase one sector at provided address */
u32 qca_sf_sect_erase(flash_info_t *info, u32 address)
{
	u32 data_out[2];
	qca_sf_bank_to_cs_mask(info->bank);

	if (address >= 0x1000000 && info->use_4byte_addr == 0) {
		return -1;
	}

	qca_sf_prepare_cmd(info->erase_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_write_en();
	if (info->use_4byte_addr) {
		qca_sf_shift_out(data_out[0], 32, 0);
		qca_sf_shift_out(data_out[1], 8, 1);
	}
	else {
		qca_sf_shift_out(data_out[0], 32, 1);
	}

	qca_sf_busy_wait();
	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();

	return 0;
}

/* Writes 'length' bytes at 'address' using page program command */
void qca_sf_write_page(flash_info_t *info, u32 bank, u32 address, u32 length, u8 *data)
{
	int i;
	u32 data_out[2];

	qca_sf_bank_to_cs_mask(bank);

	if (address >= 0x1000000 && info->use_4byte_addr == 0) {
		return;
	}

	/* assemble write command */
	qca_sf_prepare_cmd(info->page_program_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_write_en();

	qca_sf_shift_out(data_out[0], 32, 0);
	if (info->use_4byte_addr){
		qca_sf_shift_out(data_out[1], 8, 0);
	}

	length--;
	for (i = 0; i < length; i++) {
		qca_sf_shift_out(*(data + i), 8, 0);
	}

	/* Last byte and terminate */
	qca_sf_shift_out(*(data + i), 8, 1);

	qca_sf_busy_wait();
	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();
}

/* Reads 'length' bytes at 'address' to 'data' using read command */
int qca_sf_read(flash_info_t *info, u32 bank, u32 address, u32 length, u8 *data)
{
	int i;
	u32 data_out[2];

	if (address >= 0x1000000 && info->use_4byte_addr == 0)
		return -1;

	/* assemble read command */
	qca_sf_prepare_cmd(info->read_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_shift_out(data_out[0], 32, 0);
	if (info->use_4byte_addr){
		qca_sf_shift_out(data_out[1], 8, 0);
	}

	/* read data */
	length--;
	for (i = 0; i < length; i++) {
		qca_sf_shift_out(0, 8, 0);
		*(data + i) = qca_sf_shift_in();
	}

	qca_sf_shift_out(0, 8, 1);
	*(data + i) = qca_sf_shift_in();

	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();

	return 0;
}

/*
 * Checks if FLASH supports SFDP and if yes, tries to get following data:
 * - chip size
 * - erase sector size
 * - erase command
 */
u32 qca_sf_sfdp_info(u32 bank,
					 u32 *flash_size,
					 u32 *sect_size,
					 u8  *erase_cmd,
					 int *full_4b_support)
{
	u8 buffer[12];
	u8 ss = 0, ec = 0;
	u32 data_in, i;
	u32 ptp_length, ptp_offset;
	u8 full_4b = 0;

	qca_sf_bank_to_cs_mask(bank);

	qca_sf_spi_en();

	/* Shift out SFDP command with 0x0 address */
	qca_sf_shift_out(SPI_FLASH_CMD_SFDP << 24, 32, 0);

	/* 1 dummy byte and 4 bytes for SFDP signature */
	qca_sf_shift_out(0x0, 40, 0);
	data_in = qca_sf_shift_in();

	if (cpu_to_le32(data_in) != SPI_FLASH_SFDP_SIGN) {
		qca_sf_shift_out(0x0, 0, 1);
		qca_sf_spi_di();
		return 1;
	}

	/*
	 * We need to check SFDP and first parameter header major versions,
	 * because we support now only v1, exit also if ptp_length is < 9
	 */
	for (i = 0; i < 3; i++) {
		qca_sf_shift_out(0x0, 32, 0);
		data_in = qca_sf_shift_in();

		memcpy(&buffer[i * 4], &data_in, 4);
	}

	ptp_length = buffer[7];
	ptp_offset = buffer[8] | (buffer[10] << 16) | (buffer[9] << 8);

	if (buffer[1] != 1 || buffer[6] != 1 || ptp_length < 9) {
		qca_sf_shift_out(0x0, 0, 1);
		qca_sf_spi_di();
		return 1;
	}

	qca_sf_shift_out(0x0, 0, 1);

	/* FLASH density (2nd DWORD in JEDEC basic FLASH parameter table) */
	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 2);

	/* We do not support >= 4 Gbits chips */
	if ((data_in & (1 << 31)) || data_in == 0)
		return 1;

	/* TODO: it seems that density is 0-based, like max. available address? */
	if (flash_size != NULL)
		*flash_size = ((data_in & 0x7FFFFFFF) + 1) / 8;

	/* Sector/block erase size and command: 8th and 9th DWORD */
	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 8);
	memcpy(&buffer[0], &data_in, 4);

	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 9);
	memcpy(&buffer[4], &data_in, 4);

	/* We prefer bigger erase sectors */
	for (i = 0; i < 7; i += 2) {
		if ((buffer[i + 1] != 0) && buffer[i + 1] > ss) {
			ss = buffer[i + 1];
			ec = buffer[i];
		}
	}

	/* 4-byte addressing mode support */
	if (ptp_length >= 16) {
		data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 16);
		if (data_in & (1<<29)) {
			full_4b = 1;
		}
	}

	qca_sf_spi_di();

	if (ss == 0)
		return 1;

	if (sect_size != NULL)
		*sect_size = 1 << ss;

	if (erase_cmd != NULL)
		*erase_cmd = ec;

	if (full_4b_support != NULL)
		*full_4b_support = full_4b;

	return 0;
}

/* Returns JEDEC ID for selected FLASH chip */
u32 qca_sf_jedec_id(u32 bank)
{
	u32 data_in;

	qca_sf_bank_to_cs_mask(bank);

	qca_sf_spi_en();
	qca_sf_shift_out(SPI_FLASH_CMD_JEDEC << 24, 32, 1);
	data_in = qca_sf_shift_in();
	qca_sf_spi_di();

	return (data_in & 0x00FFFFFF);
}

int
qca_sf_flash_erase(flash_info_t *info, u32 address, u32 length, u8 *buf)
{
	int ret;
	int sector_size = info->size / info->sector_count;

	if (address % sector_size || length % sector_size) {
		debug("SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	while (length) {
		ret = qca_sf_sect_erase(info, address);

		if (ret) {
			printf("SF: erase failed\n");
			break;
		}
		address += sector_size;
		length -= sector_size;
	}

	return ret;
}

int qca_sf_write_buf(flash_info_t *info, u32 bank, u32 address, u32 length, u8 *buf)
{
	u8 *src;
	u32 dst;
	int total = 0;
	int len_this_lp, bytes_this_page;

	while (total < length) {
		src = buf + total;
		dst = address + total;
		bytes_this_page = ATH_SPI_PAGE_SIZE - (address % ATH_SPI_PAGE_SIZE);
		len_this_lp = ((length - total) > bytes_this_page) ? bytes_this_page : (length - total);
		qca_sf_write_page(info, 0, dst, len_this_lp, src);
		total += len_this_lp;
	}

	return 0;
}


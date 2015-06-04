/*
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __BOARD_955X_H
#define __BOARD_955X_H

#include <config.h>

#undef MTDPARTS_DEFAULT

#undef CFG_HZ

#include <atheros.h>

#ifndef FLASH_SIZE
#define FLASH_SIZE 8
#endif

#if (FLASH_SIZE > 16)
#define ATH_DUAL_NOR		1
#endif
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 */
#define CFG_MAX_FLASH_BANKS	1	/* max number of memory banks */
#if (FLASH_SIZE == 32)
#define CFG_MAX_FLASH_SECT	512	/* max number of sectors on one chip */
#elif (FLASH_SIZE == 16)
#define CFG_MAX_FLASH_SECT	256	/* max number of sectors on one chip */
#endif

#define CFG_FLASH_SECTOR_SIZE	(64*1024)
#if (FLASH_SIZE == 32)
#define CFG_FLASH_SIZE		0x02000000	/* Total flash size */
#elif (FLASH_SIZE == 16)
#define CFG_FLASH_SIZE		0x01000000	/* Total flash size */
#elif (FLASH_SIZE == 8)
#define CFG_FLASH_SIZE		0x00800000	/* max number of sectors on one chip */
#elif (FLASH_SIZE == 1/2)
#define CFG_FLASH_SIZE		0x00080000	/* Total flash size */
#else
#define CFG_FLASH_SIZE		0x00400000	/* Total flash size */
#endif

#ifndef COMPRESSED_UBOOT
#define ENABLE_DYNAMIC_CONF	1
#endif

#undef CFG_ATHRS26_PHY

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE	unsigned short

#if defined(CONFIG_ATH_NAND_BR) && defined(COMPRESSED_UBOOT)
#define CFG_FLASH_BASE			0xa0100000
#else
/* NOR Flash start address */
#define CFG_FLASH_BASE			0x9f000000
#endif

#ifdef COMPRESSED_UBOOT
#define BOOTSTRAP_TEXT_BASE		CFG_FLASH_BASE
#define BOOTSTRAP_CFG_MONITOR_BASE	BOOTSTRAP_TEXT_BASE
#endif

#define CONFIG_PCI_CONFIG_DATA_IN_OTP

/*
 * Defines to change flash size on reboot
 */
#ifdef ENABLE_DYNAMIC_CONF
#define UBOOT_FLASH_SIZE		(256 * 1024)
#define UBOOT_ENV_SEC_START		(CFG_FLASH_BASE + UBOOT_FLASH_SIZE)

#define CFG_FLASH_MAGIC			0xaabacada
#define CFG_FLASH_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x20)
#define CFG_FLASH_SECTOR_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x4)
#define CFG_FLASH_SIZE_F		*(volatile int *)(CFG_FLASH_MAGIC_F + 0x8) /* Total flash size */
#define CFG_MAX_FLASH_SECT_F		(CFG_FLASH_SIZE / CFG_FLASH_SECTOR_SIZE) /* max number of sectors on one chip */
#else
#define CFG_FLASH_SIZE_F		CFG_FLASH_SIZE
#define CFG_FLASH_SECTOR_SIZE_F		CFG_FLASH_SECTOR_SIZE
#endif


#define CFG_DDR_REFRESH_VAL		0x4138

/*
 * The following #defines are needed to get flash environment right
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CONFIG_BOOTARGS

#define	CONFIG_BOOTARGS     ""
#define MTDPARTS_DEFAULT    ""
/*
 * timeout values are in ticks
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000
#define CFG_INIT_SRAM_SP_OFFSET	0xbd001800


#define CFG_ENV_ADDR		0x9f040000
#define CONFIG_BOOTCOMMAND	"bootm 0x9f060000"

#ifdef ENABLE_DYNAMIC_CONF
#define CFG_DDR_MAGIC		0xaabacada
#define CFG_DDR_MAGIC_F		(UBOOT_ENV_SEC_START + CFG_FLASH_SECTOR_SIZE - 0x30)
#define CFG_DDR_CONFIG_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 4)
#define CFG_DDR_CONFIG2_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 8)
#define CFG_DDR_EXT_MODE_VAL_F	*(volatile int *)(CFG_DDR_MAGIC_F + 12)
#endif

#define CONFIG_NET_MULTI
#define CONFIG_MEMSIZE_IN_BYTES
#define CONFIG_PCI 1

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#ifndef COMPRESSED_UBOOT
#define ATH_CFG_COMMANDS	((			\
				CONFIG_CMD_DFL	|	\
				CFG_CMD_DHCP	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_PCI	|	\
				CFG_CMD_FLS	|	\
				CFG_CMD_MII	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET	|	\
				CFG_CMD_ENV	|	\
				CFG_CMD_PLL	|	\
				CFG_CMD_FLASH	|	\
				CFG_CMD_RUN	|	\
				CFG_CMD_ELF	|	\
				CFG_CMD_DDR	|	\
				CFG_CMD_ETHREG		\
				) & ~(			\
				CFG_CMD_FLASH		\
				))
#else
#	ifdef CONFIG_ATH_NAND_BR
#		define ATH_CFG_COMMANDS		((			\
						CONFIG_CMD_DFL	|	\
						CFG_CMD_PING	|	\
						CFG_CMD_NET) & ~(	\
						CFG_CMD_FLASH		\
						))
#	else
#		define ATH_CFG_COMMANDS		(CONFIG_CMD_DFL	|	\
				CFG_CMD_PING	|	\
				CFG_CMD_NET)
#	endif
#endif /* #ifndef COMPRESSED_UBOOT */

#ifdef CONFIG_ATH_NAND_SUPPORT
#	ifdef CONFIG_ATH_NAND_BR
#		define CFG_ENV_IS_IN_NAND	1
#		define CFG_ENV_OFFSET		0x40000u
#		define CFG_ENV_SIZE		0x40000u
#		define ATH_EXTRA_CMD		CFG_CMD_NAND
#	else
#		define CFG_ENV_IS_IN_FLASH	1
#		define CFG_ENV_SIZE		CFG_FLASH_SECTOR_SIZE
#		define ATH_EXTRA_CMD		(CFG_CMD_NAND | CFG_CMD_FLASH)
#	endif
#	define NAND_MAX_CHIPS			1
#	define CFG_MAX_NAND_DEVICE		1
#else
#	define ATH_EXTRA_CMD			CFG_CMD_FLASH
#	define CFG_ENV_IS_IN_FLASH		1
#	define CFG_ENV_SIZE			CFG_FLASH_SECTOR_SIZE
#endif

#undef DEBUG

#ifdef COMPRESSED_UBOOT
#undef  CFG_ENV_IS_IN_FLASH
#undef  CFG_ENV_IS_IN_NAND
#define CFG_ENV_IS_NOWHERE		1
#endif

#define CONFIG_COMMANDS			(ATH_CFG_COMMANDS | ATH_EXTRA_CMD | CFG_CMD_LOADS | CFG_CMD_LOADB)

#define CONFIG_IPADDR			192.168.2.100
#define CONFIG_SERVERIP			192.168.2.254
#define CONFIG_ETHADDR			0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN	1

#define CFG_PHY_ADDR			0
#define CFG_GMII			0
#define CFG_MII0_RMII			1
#define CFG_AG7100_GE0_RMII		1

#define CFG_BOOTM_LEN			(16 << 20) /* 16 MB */
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2		"hush>"

/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/

#define WLANCAL				0x9f051000
#define BOARDCAL			0x9f050000
#define ATHEROS_PRODUCT_ID		137
#define CAL_SECTOR			(CFG_MAX_FLASH_SECT - 1)

#define CONFIG_CARAMBOLA_FACTORY_MODE
#define CFG_C2_IMG_LOAD_ADDR		"0x80F00000"
#define CFG_C2_IMG_FILENAME		"lima.bin"

#define CONFIG_USB_EHCI
#define CONFIG_USB_STORAGE
#define ARCH_DMA_MINALIGN 		4*1024 // 4kb in datasheet
#define CONFIG_SYS_HZ      		1000
#define CONFIG_SYS_MAXARGS 		16
#define CONFIG_EHCI_DESC_BIG_ENDIAN
#define CONFIG_EHCI_HCD_INIT_AFTER_RESET
#define CONFIG_EHCI_IS_TDI
#define HAVE_BLOCK_DEVICE
#define CONFIG_PARTITIONS
#define CONFIG_DOS_PARTITION
#define CONFIG_FS_FAT
#define CONFIG_SUPPORT_VFAT
#define CONFIG_SYS_LOAD_ADDR 		0x82000000
#define CONFIG_CMD_USB
#define CONFIG_NEEDS_MANUAL_RELOC
#define CONFIG_USB_BOOT

#define CFG_USB_BOOT_MAX_PARTITIONS_SCAN 	16
#define CFG_USB_BOOT_LOAD_ADDR 			0x82000000   /* starts at upper half of RAM */
#define CFG_MAX_USB_BOOT_FILE_SIZE 		30*1024*1024 /* 30MB */
#define CFG_MAX_USB_RECOVERY_FILE_SIZE 		0xFA0000 /* 15.625MB */
#define CFG_USB_BOOT_FILENAME 			"8dev_uimage.bin"
#define CFG_USB_RECOVERY_FILENAME 		"8dev_recovery.bin"
#define CFG_USB_RECOVERY_FW_START_IN_FLASH 	"0x9f060000"
#define CFG_USB_BOOT_BUTTON_ID			0

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT 			"Hit '%s' key(s) to stop autoboot: %2d "
#define CONFIG_AUTOBOOT_STOP_STR 		"\x1B"

#undef CFG_BAUDRATE_TABLE
#define CFG_BAUDRATE_TABLE { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, \
			115200, 128000, 230400, 250000, 256000, 460800, 500000, 576000, 921600, \
			1000000, 1152000, 1500000, 2000000, 3000000}

/* For Merlin, both PCI, PCI-E interfaces are valid */
#define ATH_ART_PCICFG_OFFSET		12

#include <cmd_confdefs.h>


#endif	/* __BOARD_955X_H */

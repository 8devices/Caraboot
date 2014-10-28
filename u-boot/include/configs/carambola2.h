/*
 * This file contains the configuration parameters for the dbau1x00 board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#include <configs/ar7240.h>
#include <config.h>
/*-----------------------------------------------------------------------
 * FLASH and environment organization
 *-----------------------------------------------------------------------
 */
 
#define CFG_MAX_FLASH_BANKS     1
#define CFG_MAX_FLASH_SECT      256
#define CFG_FLASH_SECTOR_SIZE   (64*1024)
#define CFG_FLASH_SIZE          0x01000000

#if (CFG_MAX_FLASH_SECT * CFG_FLASH_SECTOR_SIZE) != CFG_FLASH_SIZE
#	error "Invalid flash configuration"
#endif

#define CFG_FLASH_WORD_SIZE     unsigned short 

/* 
 * We boot from this flash
 */
#define CFG_FLASH_BASE		    0x9f000000

#ifdef COMPRESSED_UBOOT
#define BOOTSTRAP_TEXT_BASE         CFG_FLASH_BASE
#define BOOTSTRAP_CFG_MONITOR_BASE  BOOTSTRAP_TEXT_BASE
#endif


/* 
 * The following #defines are needed to get flash environment right 
 */
#define	CFG_MONITOR_BASE	TEXT_BASE
#define	CFG_MONITOR_LEN		(192 << 10)

#undef CONFIG_BOOTARGS
/* XXX - putting rootfs in last partition results in jffs errors */

/* default mtd partition table */
#undef MTDPARTS_DEFAULT

#define	CONFIG_BOOTARGS     ""
#define MTDPARTS_DEFAULT    ""

#undef CFG_PLL_FREQ
#define CFG_PLL_FREQ	CFG_PLL_400_400_200



#undef CFG_HZ
/*
 * MIPS32 24K Processor Core Family Software User's Manual
 *
 * 6.2.9 Count Register (CP0 Register 9, Select 0)
 * The Count register acts as a timer, incrementing at a constant
 * rate, whether or not an instruction is executed, retired, or
 * any forward progress is made through the pipeline.  The counter
 * increments every other clock, if the DC bit in the Cause register
 * is 0.
 */
/* Since the count is incremented every other tick, divide by 2 */
/* XXX derive this from CFG_PLL_FREQ */
#define CPU_PLL_DITHER_FRAC_VAL 0x001003e8
#define CPU_CLK_CONTROL_VAL2 0x00008000

#if (CFG_PLL_FREQ == CFG_PLL_200_200_100)
#	define CFG_HZ          (200000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_300_300_150)
#	define CFG_HZ          (300000000/2)
    #if CONFIG_40MHZ_XTAL_SUPPORT
        #define CPU_PLL_CONFIG_VAL1 0x40813C00
        #define CPU_PLL_CONFIG_VAL2 0x00813C00    
    #else
        #define CPU_PLL_CONFIG_VAL1 0x40816000
        #define CPU_PLL_CONFIG_VAL2 0x00816000
    #endif
#elif (CFG_PLL_FREQ == CFG_PLL_350_350_175)
#	define CFG_HZ          (350000000/2)
    #if CONFIG_40MHZ_XTAL_SUPPORT
        #undef CPU_PLL_DITHER_FRAC_VAL
        #define CPU_PLL_DITHER_FRAC_VAL 0x001803E8
        #define CPU_PLL_CONFIG_VAL1 0x40814600
        #define CPU_PLL_CONFIG_VAL2 0x00814600    
    #else
        #define CPU_PLL_CONFIG_VAL1 0x40817000
        #define CPU_PLL_CONFIG_VAL2 0x00817000
    #endif
#elif (CFG_PLL_FREQ == CFG_PLL_333_333_166)
#	define CFG_HZ          (333000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_133)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_266_266_66)
#	define CFG_HZ          (266000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_400_400_200) || (CFG_PLL_FREQ == CFG_PLL_400_400_100)
#	define CFG_HZ          (400000000/2)
    #if CONFIG_40MHZ_XTAL_SUPPORT
        #define CPU_PLL_CONFIG_VAL1 0x40815000
        #define CPU_PLL_CONFIG_VAL2 0x00815000    
    #else
        #define CPU_PLL_CONFIG_VAL1 0x40818000
        #define CPU_PLL_CONFIG_VAL2 0x00818000
    #endif
#elif (CFG_PLL_FREQ == CFG_PLL_320_320_80) || (CFG_PLL_FREQ == CFG_PLL_320_320_160)
#	define CFG_HZ          (320000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_410_400_200)
#	define CFG_HZ          (410000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_420_400_200)
#	define CFG_HZ          (420000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_362_362_181)
#	define CFG_HZ          (326500000/2)
    #define CPU_PLL_CONFIG_VAL1 0x40817400
    #define CPU_PLL_CONFIG_VAL2 0x00817400
#elif (CFG_PLL_FREQ == CFG_PLL_80_80_40)
#	define CFG_HZ          (80000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_64_64_32)
#	define CFG_HZ          (64000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_48_48_24)
#	define CFG_HZ          (48000000/2)
#elif (CFG_PLL_FREQ == CFG_PLL_32_32_16)
#	define CFG_HZ          (32000000/2)
#endif

#if CONFIG_40MHZ_XTAL_SUPPORT
    #define CPU_PLL_SETTLE_TIME_VAL    0x00000550
#else
    #define CPU_PLL_SETTLE_TIME_VAL    0x00000352
#endif

/* 
 * timeout values are in ticks 
 */
#define CFG_FLASH_ERASE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Erase */
#define CFG_FLASH_WRITE_TOUT	(2 * CFG_HZ) /* Timeout for Flash Write */

/*
 * Cache lock for stack
 */
#define CFG_INIT_SP_OFFSET	0x1000

#ifndef COMPRESSED_UBOOT
#define	CFG_ENV_IS_IN_FLASH    1
#undef CFG_ENV_IS_NOWHERE  
#else
#undef  CFG_ENV_IS_IN_FLASH
#define CFG_ENV_IS_NOWHERE  1
#endif /* #ifndef COMPRESSED_UBOOT */

/* Address and size of Primary Environment Sector	*/
#define CFG_ENV_ADDR		0x9f040000
#define CFG_ENV_SIZE		0x10000

#define CONFIG_BOOTCOMMAND "bootm 0x9f050000"

/* DDR init values */

#define CONFIG_NR_DRAM_BANKS	2
#if CONFIG_40MHZ_XTAL_SUPPORT
#define CFG_DDR_REFRESH_VAL     0x4270
#else
#define CFG_DDR_REFRESH_VAL     0x4186
#endif
#define CFG_DDR_CONFIG_VAL      0x7fbc8cd0
#define CFG_DDR_MODE_VAL_INIT   0x133
#ifdef LOW_DRIVE_STRENGTH
#	define CFG_DDR_EXT_MODE_VAL    0x2
#else
#	define CFG_DDR_EXT_MODE_VAL    0x0
#endif
#define CFG_DDR_MODE_VAL        0x33

#define CFG_DDR_TRTW_VAL        0x1f
#define CFG_DDR_TWTR_VAL        0x1e

//#define CFG_DDR_CONFIG2_VAL	 0x99d0e6a8     // HORNET 1.0
#define CFG_DDR_CONFIG2_VAL	 0x9dd0e6a8         // HORNET 1.1

#define CFG_DDR_RD_DATA_THIS_CYCLE_VAL  0x00ff

#define CFG_DDR_TAP0_VAL        0x8
#define CFG_DDR_TAP1_VAL        0x9

/* DDR2 Init values */
#define CFG_DDR2_EXT_MODE_VAL    0x402

#define CONFIG_NET_MULTI

#define CONFIG_MEMSIZE_IN_BYTES

#ifdef COMPRESSED_UBOOT
#define ATH_NO_PCI_INIT
#endif

/*-----------------------------------------------------------------------
 * Cache Configuration
 */
#ifndef COMPRESSED_UBOOT
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_DHCP | CFG_CMD_ELF | CFG_CMD_PCI |	\
	CFG_CMD_MII | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_ENV | CFG_CMD_USB |	\
	CFG_CMD_FLASH | CFG_CMD_LOADS | CFG_CMD_RUN | CFG_CMD_LOADB | CFG_CMD_ELF | CFG_CMD_ETHREG | CFG_CMD_FAT ))
#elif defined(VXWORKS_UBOOT)
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_MII | CFG_CMD_ELF))
#else
#define CONFIG_COMMANDS	(( CONFIG_CMD_DFL | CFG_CMD_PING | CFG_CMD_NET | CFG_CMD_MII))
#endif /* #ifndef COMPRESSED_UBOOT */

#define CFG_ATHRS26_PHY  1

#define CONFIG_IPADDR   192.168.2.100
#define CONFIG_SERVERIP 192.168.2.254
#define CONFIG_ETHADDR 0x00:0xaa:0xbb:0xcc:0xdd:0xee
#define CFG_FAULT_ECHO_LINK_DOWN    1

#define CFG_PHY_ADDR 0 

#define CFG_AG7240_NMACS 2

#define CFG_GMII     0
#define CFG_MII0_RMII             1
#define CFG_AG7100_GE0_RMII             1

#define CFG_BOOTM_LEN	(16 << 20) /* 16 MB */
#undef  DEBUG
#define CFG_CONSOLE_INFO_QUIET		/* don't print console @ startup*/
#define CONFIG_SHOW_BOOT_PROGRESS	/* use LEDs to show boot status*/
#define CONFIG_SHOW_ACTIVITY
#define CFG_HUSH_PARSER
#define CFG_PROMPT_HUSH_PS2 "hush>"

#define CONFIG_CARAMBOLA_FACTORY_MODE
#define CFG_C2_IMG_LOAD_ADDR		"0x80F00000"
#define CFG_C2_IMG_FILENAME		"carambola2.bin"

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
#define CFG_USB_RECOVERY_FW_START_IN_FLASH 	"0x9f050000"
#define CFG_USB_BOOT_BUTTON_ID			0

#define CONFIG_AUTOBOOT_KEYED
#define CONFIG_AUTOBOOT_PROMPT 			"Hit '%s' key(s) to stop autoboot: %2d "
#define CONFIG_AUTOBOOT_STOP_STR 		"\x1B"

#undef CFG_BAUDRATE_TABLE
#define CFG_BAUDRATE_TABLE { 300, 1200, 2400, 4800, 9600, 14400, 19200, 28800, 38400, 56000, 57600, \
			115200, 128000, 230400, 250000, 256000, 460800, 500000, 576000, 921600, \
			1000000, 1152000, 1500000, 2000000, 3000000}
/*
** Parameters defining the location of the calibration/initialization
** information for the two Merlin devices.
** NOTE: **This will change with different flash configurations**
*/


#define WLANCAL                         0x9fff1000
#define BOARDCAL                        0x9fff0000

#define ATHEROS_PRODUCT_ID             138
#define CAL_SECTOR                     (CFG_MAX_FLASH_SECT - 1)

/* For Kite, only PCI-e interface is valid */
#define AR7240_ART_PCICFG_OFFSET        3


#include <cmd_confdefs.h>

#endif	/* __CONFIG_H */

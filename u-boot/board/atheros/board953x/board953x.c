/*
 * Copyright (c) 2013 Qualcomm Atheros, Inc.
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
 */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <atheros.h>

extern int ath_ddr_initial_config(uint32_t refresh);
extern int ath_ddr_find_size(void);

#ifdef COMPRESSED_UBOOT
#	define prmsg(...)
#	define args		char *s
#	define board_str(a)	do {						\
	char ver[] = "0";							\
	uint32_t        revid;							\
	if(((revid=ath_reg_rd(RST_REVISION_ID_ADDRESS))&0xff0)==0x140) 		\
		strcpy(s, a " - Honey Bee 1.");					\
	else			 						\
		strcpy(s, a " - Honey Bee 2.");					\
	ver[0] +=  (revid & 0xf) ;		 				\
	strcat(s, ver);								\
} while (0)
#else
#	define prmsg	debug
#	define args		void
#	define board_str(a)							\
	uint32_t        revid;							\
	if(((revid=ath_reg_rd(RST_REVISION_ID_ADDRESS))&0xff0)==0x140) 		\
	debug(a " - Honey Bee 1.%d", revid & 0xf);				\
	else									\ 
	debug(a " - Honey Bee 2.%d", revid & 0xf);
#endif

// Lima GPIO LED definition
struct gpio_led_desc gpiolib_leds[]={
  { .id=0, .bit=0,  .polarity=0, .disable=1 }, // WLAN LED (Unused)
  { .id=1, .bit=0,  .polarity=0, .disable=1 }, // ETH0_LED (Unused)
  { .id=2, .bit=0,  .polarity=0, .disable=1 }, // ETH1_LED (Unused)
  { .id=3, .bit=0,  .polarity=0, .disable=1 }, // USB recovery indication (Unused)
  { .id=4, .bit=0,  .polarity=0, .disable=1 }  // Reserved
};
int gpiolib_led_count = sizeof(gpiolib_leds)/sizeof(gpiolib_leds[0]);

struct gpio_led_desc gpiolib_buttons[]={
  { .id=0, .bit=16, .polarity=0, .disable=0 }, // USB Boot
  { .id=1, .bit=0, .polarity=0, .disable=1 }, // Reserved
  { .id=2, .bit=0,  .polarity=0, .disable=1 }, // Reserved
  { .id=3, .bit=0,  .polarity=0, .disable=1 }, // Reserved
  { .id=4, .bit=0,  .polarity=0, .disable=1 }, // Reserved
};
int gpiolib_button_count = sizeof(gpiolib_buttons)/sizeof(gpiolib_buttons[0]);

#ifdef CONFIG_SHOW_BOOT_PROGRESS
// Global variable to indicate if boot is succesful
int board953x_boot_status = 0;
#endif

void
ath_usb_initial_config(void)
{
#define unset(a)	(~(a))

	if (ath_reg_rd(RST_BOOTSTRAP_ADDRESS) & RST_BOOTSTRAP_TESTROM_ENABLE_MASK) {

		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_HOST_RESET_SET(1));
		udelay(1000);
		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_RESET_SET(1));
		udelay(1000);

		ath_reg_wr(PHY_CTRL5_ADDRESS, PHY_CTRL5_RESET_1);
		udelay(1000);

		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_PLL_PWD_EXT_SET(1));
		udelay(1000);
		ath_reg_rmw_set(RST_RESET_ADDRESS, RST_RESET_USB_PHY_ARESET_SET(1));
		udelay(1000);

		ath_reg_rmw_clear(RST_CLKGAT_EN_ADDRESS, RST_CLKGAT_EN_USB1_SET(1));

		return;
	}

	ath_reg_wr_nf(SWITCH_CLOCK_SPARE_ADDRESS,
		ath_reg_rd(SWITCH_CLOCK_SPARE_ADDRESS) |
		SWITCH_CLOCK_SPARE_USB_REFCLK_FREQ_SEL_SET(2));
	udelay(1000);

	ath_reg_rmw_set(RST_RESET_ADDRESS,
				RST_RESET_USB_PHY_SUSPEND_OVERRIDE_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_ARESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_RESET_SET(1));
	udelay(1000);
	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_HOST_RESET_SET(1));
	udelay(1000);

	ath_reg_rmw_clear(RST_RESET_ADDRESS, RST_RESET_USB_PHY_PLL_PWD_EXT_SET(1));
	udelay(10);
}

void ath_gpio_config(void)
{
	/* disable the CLK_OBS on GPIO_4 and set GPIO4 as input */
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, (1 << 4));
	ath_reg_rmw_clear(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_MASK);
	ath_reg_rmw_set(GPIO_OUT_FUNCTION1_ADDRESS, GPIO_OUT_FUNCTION1_ENABLE_GPIO_4_SET(0x80));
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 4));
	/* Set GPIO 13 as input for LED functionality to be OFF during bootup */
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 13));
	/* Turn off JUMPST_LED and 5Gz LED during bootup */
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 15));
	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1 << 12));
}

void ar7240_gpio_led_switch(int led_id, int state)
{
	return;
}

void show_boot_progress(int arg)
{
    return;
}

int button_read(int button_id){
  //if (buttons[button_id].disable != 1){
    //if (((ar7240_reg_rd(AR7240_GPIO_IN)>>buttons[button_id].bit)&0x01) == buttons[button_id].polarity)
      if (((ath_reg_rd(GPIO_IN_ADDRESS)>> 17 )&0x01) == 0)
      return 1;
    else 
      return 0;
  //}
  //else
  //  return -1;
}

int
ath_mem_config(void)
{
	unsigned int type, reg32, *tap;
	extern uint32_t *ath_ddr_tap_cal(void);

#if !defined(CONFIG_ATH_EMULATION)
	type = ath_ddr_initial_config(CFG_DDR_REFRESH_VAL);

	tap = ath_ddr_tap_cal();
	prmsg("tap = 0x%p\n", tap);

	tap = (uint32_t *)0xbd001f10;
	prmsg("Tap (low, high) = (0x%x, 0x%x)\n", tap[0], tap[1]);

	tap = (uint32_t *)TAP_CONTROL_0_ADDRESS;
	prmsg("Tap values = (0x%x, 0x%x, 0x%x, 0x%x)\n",
		tap[0], tap[2], tap[2], tap[3]);

	/* Take WMAC out of reset */
	reg32 = ath_reg_rd(RST_RESET_ADDRESS);
	reg32 = reg32 & ~RST_RESET_RTC_RESET_SET(1);
	ath_reg_wr_nf(RST_RESET_ADDRESS, reg32);

	ath_usb_initial_config();

	ath_gpio_config();
#endif /* !defined(CONFIG_ATH_EMULATION) */

	return ath_ddr_find_size();
}

long int initdram(int board_type)
{
	return (ath_mem_config());
}

int	checkboard(args)
{
	board_str(CONFIG_BOARD_NAME);
	return 0;
}

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7240_soc.h"
#include <asm/gpiolib.h>

extern void ar7240_ddr_initial_config(uint32_t refresh);
extern int ar7240_ddr_find_size(void);

void
ar7240_usb_initial_config(void)
{
#ifndef CONFIG_HORNET_EMU
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0a04081e);
    ar7240_reg_wr_nf(AR7240_USB_PLL_CONFIG, 0x0804081e);
#endif
}

#ifdef CONFIG_SHOW_BOOT_PROGRESS
int ar7240_boot_status = 0;  	// Global variable to indicate if boot is succesful
				// negative values show failure
#endif

// Carambola2 GPIO LED definition
struct gpio_led_desc gpiolib_leds[]={
  { .id=0, .bit=0,  .polarity=0, .disable=0 }, // WLAN LED
  { .id=1, .bit=13, .polarity=1, .disable=0 }, // ETH0_LED
  { .id=2, .bit=14, .polarity=1, .disable=0 }, // ETH1_LED
  { .id=3, .bit=0,  .polarity=0, .disable=0 }, // USB recovery indication
  { .id=4, .bit=0,  .polarity=0, .disable=1 }  // Reserved
};
int gpiolib_led_count = sizeof(gpiolib_leds)/sizeof(gpiolib_leds[0]);

struct gpio_led_desc gpiolib_buttons[]={
  { .id=0, .bit=11, .polarity=0, .disable=0 }, // USB Boot
  { .id=1, .bit=0,  .polarity=0, .disable=1 }, // Reserved
  { .id=2, .bit=0,  .polarity=0, .disable=1 }, // Reserved
  { .id=3, .bit=0,  .polarity=0, .disable=1 }, // Reserved
  { .id=4, .bit=0,  .polarity=0, .disable=1 }, // Reserved
};
int gpiolib_button_count = sizeof(gpiolib_buttons)/sizeof(gpiolib_buttons[0]);

void
ar7240_usb_otp_config(void)
{
    unsigned int addr, reg_val, reg_usb;
    int time_out, status, usb_valid;
    
    for (addr = 0xb8114014; ;addr -= 0x10) {
        status = 0;
        time_out = 20;
        
        reg_val = ar7240_reg_rd(addr);

        while ((time_out > 0) && (~status)) {
            if ((( ar7240_reg_rd(0xb8115f18)) & 0x7) == 0x4) {
                status = 1;
            } else {
                status = 0;
            }
            time_out--;
        }

        reg_val = ar7240_reg_rd(0xb8115f1c);
        if ((reg_val & 0x80) == 0x80){
            usb_valid = 1;
            reg_usb = reg_val & 0x000000ff;
        }

        if (addr == 0xb8114004) {
            break;
        }
    }

    if (usb_valid) {
        reg_val = ar7240_reg_rd(0xb8116c88);
        reg_val &= ~0x03f00000;
        reg_val |= (reg_usb & 0xf) << 22;
        ar7240_reg_wr(0xb8116c88, reg_val);
    }
}

void ar7240_gpio_config(void)
{
    /* Disable clock obs 
     * clk_obs1(gpio13/bit8),  clk_obs2(gpio14/bit9), clk_obs3(gpio15/bit10),
     * clk_obs4(gpio16/bit11), clk_obs5(gpio17/bit12)
     * clk_obs0(gpio1/bit19), 6(gpio11/bit20)
     */
    ar7240_reg_wr (AR7240_GPIO_FUNC, 
        (ar7240_reg_rd(AR7240_GPIO_FUNC) & ~((0x1f<<8)|(0x3<<19))));
    
    /* Enable eth Switch LEDs */
    //ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) | (0x1f<<3)));
	
    /* Clear AR7240_GPIO_FUNC BIT2 to ensure that software can control LED5(GPIO16) and LED6(GPIO17)  */
    ar7240_reg_wr (AR7240_GPIO_FUNC, (ar7240_reg_rd(AR7240_GPIO_FUNC) & ~(0x1<<2)));
	
    /* Set HORNET_BOOTSTRAP_STATUS BIT18 to ensure that software can control GPIO26 and GPIO27 */
    //ar7240_reg_wr (HORNET_BOOTSTRAP_STATUS, (ar7240_reg_rd(HORNET_BOOTSTRAP_STATUS) | (0x1<<18)));
}

unsigned int board_get_gpio_input(void)
{
	return ar7240_reg_rd(AR7240_GPIO_IN);
}

int board_set_gpio_regs(unsigned int addr, unsigned int set, unsigned int clear)
{
	if ((addr >= 0x18040000) && (addr <= 0x18040044) &&
		(addr % 4 == 0)){
		ar7240_reg_wr(addr, ((ar7240_reg_rd(addr) | set) & (~clear)));
		return 0;
	}
	return 1;
}

void board_gpiolib_defaults(void)
{
	//set output enable
	ar7240_reg_wr (AR7240_GPIO_OE, (ar7240_reg_rd(AR7240_GPIO_OE) | (1<<13)|(1<<14)|(1<<0) ));
	//set ETH0 ETH1 LED output to high
	ar7240_reg_wr (AR7240_GPIO_SET, (1<<13)|(1<<14));
	//set WLAN LED output to low (reverse polarity LED)
	ar7240_reg_wr (AR7240_GPIO_CLEAR, (1<<0));
	//Enable USB boot sense GPIO as input
	ar7240_reg_wr (AR7240_GPIO_OE, (ar7240_reg_rd(AR7240_GPIO_OE) & ~(1<<11)));
}

void board_gpio_set(int gpio)
{
	ar7240_reg_wr (AR7240_GPIO_SET, (1<<gpio));
}

void board_gpio_clear(int gpio)
{
	ar7240_reg_wr (AR7240_GPIO_CLEAR, (1<<gpio));
}

#ifdef CONFIG_SHOW_ACTIVITY
void show_activity(int arg)
{
  uint32_t time = 0;
  static uint32_t led = 0;
  time =get_timer(0);

    if ( (ar7240_boot_status<0) )
    {
	//Blink 3 first LEDs in descriptor together
	led += 1;
	if (led > 2) led = 0;
	gpiolib_led_switch(led ,(time>>24)&0x01);
    }
    return;
}
#endif

#ifdef CONFIG_SHOW_BOOT_PROGRESS
void show_boot_progress(int arg)
{
    ar7240_boot_status = arg;
    return;
}
#endif

int
ar7240_mem_config(void)
{
#ifndef COMPRESSED_UBOOT
    unsigned int tap_val1, tap_val2;
#endif

    ar7240_gpio_config();	// init GPIO

#if !defined(CONFIG_SKIP_LOWLEVEL_INIT)
    ar7240_ddr_initial_config(CFG_DDR_REFRESH_VAL);

/* Default tap values for starting the tap_init*/
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL0, CFG_DDR_TAP0_VAL);
    ar7240_reg_wr (AR7240_DDR_TAP_CONTROL1, CFG_DDR_TAP1_VAL);

#ifndef COMPRESSED_UBOOT
    ar7240_ddr_tap_init();

    tap_val1 = ar7240_reg_rd(0xb800001c);
    tap_val2 = ar7240_reg_rd(0xb8000020);
    debug("#### TAP VALUE 1 = %x, 2 = %x\n",tap_val1, tap_val2);
#endif
#endif /* CONFIG_SKIP_LOWLEVEL_INIT */

    ar7240_usb_initial_config();
    ar7240_usb_otp_config();
    //hornet_ddr_tap_init();

    return (ar7240_ddr_find_size());
}

long int initdram(int board_type)
{
    return (ar7240_mem_config());
}

#ifdef COMPRESSED_UBOOT
int checkboard (char *board_string)
{
    strcpy(board_string, "Caraboot "CARABOOT_RELEASE" (AR9331) U-boot");
    return 0;
}
#else
int checkboard (void)
{
    printf("=====================================\n");
    printf("Caraboot "CARABOOT_RELEASE" (AR9331) U-boot\n");
    printf("http://www.8devices.com/\n");
    printf("-------------------------------------\n");
    return 0;
}
#endif /* #ifdef COMPRESSED_UBOOT */

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include "ar7240_soc.h"

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

typedef struct reg_conf{
  unsigned int addr;
  unsigned int  set_mask;
  unsigned int  clear_mask;
};

// Carambola2 GPIO LED definition

typedef struct gpio_led_desc {
  int id;
  int bit;	//GPIO bit
  int polarity; //1 - high active; 0 - low active
  int disable; //1 - disable led, 0 - enable
};

int  led_count=5;
struct gpio_led_desc leds[]={
  {// WLAN LED
    .id=0,
    .bit=0,
    .polarity=0,
    .disable=0
  },
  {// ETH0_LED
    .id=1,
    .bit=13,
    .polarity=1,
    .disable=0
  },
  {// ETH1_LED
    .id=2,
    .bit=14,
    .polarity=1,
    .disable=0
  },
  {// USB recovery indication
    .id=3,
    .bit=0,
    .polarity=0,
    .disable=0
  },
    {// Nothing
    .id=4,
    .bit=0,
    .polarity=0,
    .disable=1
  }
};

struct gpio_led_desc buttons[]={
  {// USB Boot
    .id=0,
    .bit=11,
    .polarity=0,
    .disable=0
  },
  {// Nothing
    .id=1,
    .bit=0,
    .polarity=0,
    .disable=1
  },
  {// Nothing
    .id=2,
    .bit=0,
    .polarity=0,
    .disable=1
  },
  {// Nothing
    .id=3,
    .bit=0,
    .polarity=0,
    .disable=1
  },
    {// Nothing
    .id=4,
    .bit=0,
    .polarity=0,
    .disable=1
  }
};

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

void assign_gpio(void *data, int item_count1, int* tmp_item)
{
      struct reg_conf *regs;
      regs = (struct reg_conf*)data;
      regs[item_count1].addr       = tmp_item[0];
      regs[item_count1].set_mask   = tmp_item[1];
      regs[item_count1].clear_mask = tmp_item[2];
      return;
}

void assign_leds(void *data, int item_count1, int* tmp_item)
{    
      if (tmp_item[0]<5 && tmp_item[0]>=0){
	int id=tmp_item[0];
	leds[id].id       = id;
	leds[id].bit      = tmp_item[1];
	leds[id].polarity = tmp_item[2];
	leds[id].disable  = tmp_item[3];
      }
      return;
}

void assign_buttons(void *data, int item_count1, int* tmp_item)
{    
      if (tmp_item[0]<5 && tmp_item[0]>=0){
	int id=tmp_item[0];
	buttons[id].id       = id;
	buttons[id].bit      = tmp_item[1];
	buttons[id].polarity = tmp_item[2];
	buttons[id].disable  = tmp_item[3];
      }
      return;
}

int convert_hex(char* str, int item_nr)
{
  return (simple_strtol(str, NULL, 16));
}

int convert_dec(char* str, int item_nr)
{
  return (simple_strtol(str, NULL, 10));
}

int count_args(char* env)
{
  char* pos;
  pos = strchr(env,'/');
  unsigned int reg_count=1;
  while (pos != NULL){
    reg_count++;
    pos = strchr(pos+1,'/');
  }
  return reg_count;
}

int parse_config(char* env, int max_args, void *regs, int *reg_count,
		 void (*assign)(void*, int, int*),
		 int (*convert)(char*, int)  )
{
  char *tmp_str1, *tmp_str2, *tok1, *tok2;  
  int item_count1=0;
  int item_count2;
  int* tmp_item = malloc((sizeof(int))*max_args);
  if (tmp_item == NULL){
    return (-1);
  }
  
  tmp_str1 = strdup(env);
  tok1=strsep(&tmp_str1, "/");
  while (tok1 != NULL){  
    item_count2=0;
    tmp_str2 = strdup(tok1);
    tok2=strsep(&tmp_str2, ":");
    while (tok2 != NULL){
      item_count2++;
      if (item_count2 > max_args)
	break;
      tmp_item[item_count2-1] = convert(tok2,item_count2);
      tok2=strsep(&tmp_str2, ":");
    }
    if (item_count2 == max_args){
      assign(regs, item_count1, tmp_item);
      item_count1++;
    }
    tok1 = strsep(&tmp_str1, "/");
  }
  *reg_count = item_count1;
  return (0);
}

void apply_config(struct reg_conf *regs, int reg_count)
{
  int i;
  for (i=0; i<reg_count; i++){
    //Check if given registers belong to GPIO range
    if ((regs[i].addr>=0x18040000) && (regs[i].addr<=0x18040044)){
      ar7240_reg_wr(regs[i].addr, ((ar7240_reg_rd(regs[i].addr)|regs[i].set_mask)&(~regs[i].clear_mask)));
    }
    else{
      printf("Reg: %x is not in valid GPIO range\n");
    }
  }
  return;
}

void gpio_env_init(int mode)
{
  int max_args;
  void (*assign)(struct reg_conf*, int, int*);
  int (*convert)(char* ,int);
  char* gpio_env;
  int ret;
  struct reg_conf *regs;
  int reg_count;
  int i;

  if (mode==0){
    assign = &assign_gpio;
    convert = &convert_hex;
    max_args = 3;
    gpio_env = getenv("gpio_config");
  }
  else{
    convert = &convert_dec;
    max_args = 4;
    if (mode==1){
      assign = &assign_leds;
      gpio_env = getenv("led_config");
    }
    else if(mode == 2){
      assign = &assign_buttons;
      gpio_env = getenv("button_config");     
    }
  }

  if (gpio_env == NULL){
    if (mode==0){
      //Default init
      //set output enable
      ar7240_reg_wr (AR7240_GPIO_OE, (ar7240_reg_rd(AR7240_GPIO_OE) | (1<<13)|(1<<14)|(1<<0) ));
      //set ETH0 ETH1 LED output to high
      ar7240_reg_wr (AR7240_GPIO_SET, (1<<13)|(1<<14));
      //set WLAN LED output to low (reverse polarity LED)
      ar7240_reg_wr (AR7240_GPIO_CLEAR, (1<<0));
      //Enable USB boot sense GPIO as input
      ar7240_reg_wr (AR7240_GPIO_OE, (ar7240_reg_rd(AR7240_GPIO_OE) & ~(1<<11)));
    }
    return;
  }

  reg_count = count_args(gpio_env);

  if (mode==0){
    regs = malloc(sizeof(struct reg_conf)*reg_count);
    if (regs == NULL)
      return;
  }
  else
    regs = NULL;

  ret = parse_config(gpio_env, max_args, (void*)regs, &reg_count, assign, convert);
  if (ret<0)
    return;

  if (mode==0)
    apply_config(regs, reg_count);
  
  free(regs);
  return;
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

int button_read(int button_id){
  if (buttons[button_id].disable != 1){
    if (((ar7240_reg_rd(AR7240_GPIO_IN)>>buttons[button_id].bit)&0x01) == buttons[button_id].polarity)
      return 1;
    else 
      return 0;
  }
  else
    return -1;
}


#ifdef CONFIG_SHOW_ACTIVITY
void ar7240_gpio_led_switch(int led_id, int state)
//switch LED (led_id is defined in struct[] leds) to state (0 - off, 1 -on)
{
  int i;
  for (i=0; i<led_count;i++){
      if (leds[i].disable != 1){
	if (leds[i].id==led_id) {
	    if ((!leds[i].polarity) ^ state){
		ar7240_reg_wr (AR7240_GPIO_SET, (1<<leds[i].bit));
	    }
	    else{
		ar7240_reg_wr (AR7240_GPIO_CLEAR, (1<<leds[i].bit));
	    }
	    break;
	}
      }
  }
  return;
}

void ar7240_gpio_leds_off(void)
{
    int i;
    for (i=0; i<led_count;i++){
      if (leds[i].disable != 1){
	ar7240_gpio_led_switch(i, 0);
      }
    }
}

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
	ar7240_gpio_led_switch(led ,(time>>24)&0x01);
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

    ar7240_usb_initial_config();
    ar7240_usb_otp_config();
    
    //hornet_ddr_tap_init();

    gpio_env_init(0);   // custom GPIO init, turns on all LEDs
    gpio_env_init(1);   //LEDs
    gpio_env_init(2);   //BUTTONs

    udelay(100 * 1000); 	// 100ms delay
    ar7240_gpio_leds_off();

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

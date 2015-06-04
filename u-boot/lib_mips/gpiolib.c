/*
 * Runtime GPIO remapping library
 *
 * Copyright (C) 2015 Mantas Pucka <mantas@8devices.com>
 *
 * SPDX-License-Identifier:GPL-2.0
*/

#include <common.h>
#include <command.h>
#include <asm/mipsregs.h>
#include <asm/addrspace.h>
#include <config.h>
#include <version.h>
#include <malloc.h>
#include "ar7240_soc.h"
#include <asm/gpiolib.h>

#ifdef CONFIG_GPIOLIB

// These must be defined in board file
extern struct gpio_led_desc gpiolib_leds[];
extern int gpiolib_led_count;
extern struct gpio_led_desc gpiolib_buttons[];
extern int gpiolib_button_count;

extern unsigned int board_get_gpio_input(void);
extern int board_set_gpio_regs(unsigned int addr, unsigned int set, unsigned int clear);
extern void board_gpiolib_defaults(void);
extern void board_gpio_set(int gpio);
extern void board_gpio_clear(int gpio);


static int convert_hex(char* str, int item_nr)
{
	return (simple_strtol(str, NULL, 16));
}

static int convert_dec(char* str, int item_nr)
{
	return (simple_strtol(str, NULL, 10));
}

static void assign_gpio(void *data, int item_count1, int* tmp_item)
{
	struct reg_conf *regs;
	regs = (struct reg_conf*)data;
	regs[item_count1].addr       = tmp_item[0];
	regs[item_count1].set_mask   = tmp_item[1];
	regs[item_count1].clear_mask = tmp_item[2];
	return;
}

static void assign_leds(void *data, int item_count1, int* tmp_item)
{
	if (tmp_item[0] < gpiolib_led_count && tmp_item[0]>=0){
		int id=tmp_item[0];
		gpiolib_leds[id].id       = id;
		gpiolib_leds[id].bit      = tmp_item[1];
		gpiolib_leds[id].polarity = tmp_item[2];
		gpiolib_leds[id].disable  = tmp_item[3];
	}
	return;
}

static void assign_buttons(void *data, int item_count1, int* tmp_item)
{
	if (tmp_item[0] < gpiolib_button_count && tmp_item[0]>=0){
		int id=tmp_item[0];
		gpiolib_buttons[id].id       = id;
		gpiolib_buttons[id].bit      = tmp_item[1];
		gpiolib_buttons[id].polarity = tmp_item[2];
		gpiolib_buttons[id].disable  = tmp_item[3];
	}
	return;
}

static int count_args(char* env)
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

static void gpiolib_apply_config(struct reg_conf *regs, int reg_count)
{
	int i;
	for (i=0; i<reg_count; i++){
		if (board_set_gpio_regs(regs[i].addr, regs[i].set_mask, regs[i].clear_mask) == 1)
			printf("Reg: %x is not in valid GPIO range\n", regs[i].addr);
	}
	return;
}

int gpiolib_button_read(int button_id)
{
	if (gpiolib_buttons[button_id].disable == 1)
		return -1;

	if (((board_get_gpio_input() >> gpiolib_buttons[button_id].bit) & 0x01) ==
		gpiolib_buttons[button_id].polarity)
		return 1;

	return 0;
}

static int gpiolib_parse_config(char* env, int max_args, void *regs, int *reg_count,
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
	free(tmp_item);
	return (0);
}

void gpiolib_env_init(int mode)
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
			board_gpiolib_defaults();
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

	ret = gpiolib_parse_config(gpio_env, max_args, (void*)regs, &reg_count, assign, convert);
	if (ret<0)
		return;

	if (mode==0)
		gpiolib_apply_config(regs, reg_count);

	free(regs);
	return;
}

void gpiolib_led_switch(int led_id, int state)
{
	int i;
	for (i=0; i < gpiolib_led_count;i++){
		if (gpiolib_leds[i].disable != 1){
			if (gpiolib_leds[i].id==led_id) {
				if ((!gpiolib_leds[i].polarity) ^ state){
					board_gpio_set(gpiolib_leds[i].bit);
				}
				else{
					board_gpio_clear(gpiolib_leds[i].bit);
				}
				break;
			}
		}
	}
	return;
}

void gpiolib_leds_off(void)
{
	int i;
	for (i=0; i < gpiolib_led_count;i++){
		if (gpiolib_leds[i].disable != 1){
			gpiolib_led_switch(i, 0);
		}
	}
}

// Call after relocation
void gpiolib_customized_init(void)
{
	//init GPIO, LED and button config
	gpiolib_env_init(0);   // custom GPIO init, turns on all LEDs
	gpiolib_env_init(1);   //LEDs
	gpiolib_env_init(2);   //BUTTONs
	udelay(100 * 1000); 	// 100ms delay
	gpiolib_leds_off();
}

#endif /* CONFIG_GPIOLIB */

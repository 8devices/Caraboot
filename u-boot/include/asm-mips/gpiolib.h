/*
 * Runtime GPIO remapping library
 *
 * Copyright (C) 2015 Mantas Pucka <mantas@8devices.com>
 *
 * SPDX-License-Identifier:GPL-2.0
*/

#ifndef _ASM_GPIOLIB_H
#define _ASM_GPIOLIB_H


typedef struct reg_conf{
  unsigned int addr;
  unsigned int  set_mask;
  unsigned int  clear_mask;
};

typedef struct gpio_led_desc {
  int id;
  int bit;	//GPIO bit
  int polarity; //1 - high active; 0 - low active
  int disable; //1 - disable led, 0 - enable
};

void gpiolib_led_switch(int led_id, int state);
void gpiolib_customized_init(void);

#endif /* _ASM_GPIOLIB_H */

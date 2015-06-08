/* 
 * Copyright (c) 2014 The Linux Foundation. All rights reserved.
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

#include <asm/addrspace.h>
#include <asm/types.h>
#include <config.h>
#include <common.h>
#include <atheros.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Divide positive or negative dividend by positive divisor and round
 * to closest integer. Result is undefined for negative divisors and
 * for negative dividends if the divisor variable type is unsigned.
 */
#define DIV_ROUND_CLOSEST(x, divisor)(			\
{							\
	typeof(x) __x = x;				\
	typeof(divisor) __d = divisor;			\
	(((typeof(x))-1) > 0 ||				\
	 ((typeof(divisor))-1) > 0 || (__x) > 0) ?	\
		(((__x) + ((__d) / 2)) / (__d)) :	\
		(((__x) - ((__d) / 2)) / (__d));	\
}							\
)

int serial_init(void)
{
	uint32_t val;

#ifdef CONFIG_MACH_QCA956x
	uart_gpio_init_qca956x();
#else
	uart_gpio_init_qca953x();
#endif
	serial_setbrg();

	ath_uart_wr(OFS_DATA_FORMAT, 0x3);
	ath_uart_wr(OFS_INTR_ENABLE, 0);

	return 0;
}

void uart_gpio_init_qca953x(void)
{
	uint32_t val;

	ath_reg_rmw_set(GPIO_OE_ADDRESS, (1<<9));
	ath_reg_rmw_clear(GPIO_OE_ADDRESS, (1<<10));

	ath_reg_rmw_set(GPIO_OUT_FUNCTION2_ADDRESS,
			GPIO_OUT_FUNCTION2_ENABLE_GPIO_10_SET(0x16));

	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0xff));

	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x9));
}


void uart_gpio_init_qca956x(void)
{
#ifdef CONFIG_MACH_QCA956x
#if defined(UART_RX20_TX22)

	val = (ath_reg_rd(GPIO_OE_ADDRESS) & (~0x400000));
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_clear(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_22_MASK);

	ath_reg_rmw_set(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_22_SET(0x16));

	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_MASK);
   
	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x14));
#elif defined(UART_RX18_TX22)
	val = (ath_reg_rd(GPIO_OE_ADDRESS) & (~0x400000)) | 0x40000;
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_clear(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_22_MASK);
	ath_reg_rmw_set(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_22_SET(0x16));
	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_MASK);
  
	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x12));

#elif defined(UART_RX18_TX20)
	val = (ath_reg_rd(GPIO_OE_ADDRESS) & (~0x100000)) | 0x40000;
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	val = ath_reg_rd(GPIO_OUT_ADDRESS) | 0xeffff6;
	ath_reg_wr(GPIO_OUT_ADDRESS, val);

	ath_reg_rmw_clear(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_20_MASK);
	ath_reg_rmw_set(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_20_SET(0x16));
	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_MASK);
  
	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x12));

#elif defined(UART_RX24_TX20)
	// Turn off LED before XLNA swap to GPO
	val = ath_reg_rd(GPIO_OUT_ADDRESS) | 0xaffff6;
	ath_reg_wr(GPIO_OUT_ADDRESS, val);
	//Switch GPI and GPO and XPA, XLNA
	ath_reg_wr(GPIO_FUNCTION_ADDRESS, 0x8000);

	val = (ath_reg_rd(GPIO_OE_ADDRESS) & (~0x100000)) | 0x1000000;
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_set(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_20_SET(0x16));
	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0xff));

	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x18));
  
#elif defined(TEST_BOARD_UART)
	//Switch GPI and GPO and XPA1, ANTC
	ath_reg_wr(GPIO_FUNCTION_ADDRESS, 0xc000);

	val = ath_reg_rd(GPIO_OE_ADDRESS) & (~0x2000);
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_clear(GPIO_OUT_FUNCTION3_ADDRESS,
			GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_MASK);

	ath_reg_rmw_set(GPIO_OUT_FUNCTION3_ADDRESS,
			GPIO_OUT_FUNCTION3_ENABLE_GPIO_13_SET(0x16));
	
	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0xff));

	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x17));

#else
	val = (ath_reg_rd(GPIO_OE_ADDRESS) & (~0x100000)) | 0x80000;
	ath_reg_wr(GPIO_OE_ADDRESS, val);

	ath_reg_rmw_set(GPIO_OUT_FUNCTION5_ADDRESS,
			GPIO_OUT_FUNCTION5_ENABLE_GPIO_20_SET(0x16));
	ath_reg_rmw_clear(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0xff));

	ath_reg_rmw_set(GPIO_IN_ENABLE0_ADDRESS,
			GPIO_IN_ENABLE0_UART_SIN_SET(0x13));

#endif

	val = ath_reg_rd(GPIO_OUT_ADDRESS) | 0xaffff6;
	ath_reg_wr(GPIO_OUT_ADDRESS, val);

	val = ath_reg_rd(GPIO_SPARE_ADDRESS);
	ath_reg_wr(GPIO_SPARE_ADDRESS, (val | 0x8402));
#endif
}

int serial_tstc (void)
{
	return(ath_uart_rd(OFS_LINE_STATUS) & 0x1);
}

int serial_getc(void)
{
	while(!serial_tstc());

	return (ath_uart_rd(OFS_RCV_BUFFER) & 0xff);
}


void serial_putc(const char byte)
{
	if (byte == '\n') serial_putc ('\r');

	while (((ath_uart_rd(OFS_LINE_STATUS)) & 0x20) == 0x0);

	ath_uart_wr(OFS_SEND_BUFFER, byte);
}

void serial_setbrg (void)
{
	uint32_t div;

	div = DIV_ROUND_CLOSEST(ath_uart_freq(), (16 * gd->baudrate));

	/* set DIAB bit */
	ath_uart_wr(OFS_LINE_CONTROL, ath_uart_rd(OFS_LINE_CONTROL) | 0x80);

	/* set divisor */
	ath_uart_wr(OFS_DIVISOR_LSB, (div & 0xff));
	ath_uart_wr(OFS_DIVISOR_MSB, ((div >> 8) & 0xff));

	/* clear DIAB bit*/
	ath_uart_wr(OFS_LINE_CONTROL, ath_uart_rd(OFS_LINE_CONTROL) & ~0x80);
}

void serial_puts (const char *s)
{
	while (*s)
	{
		serial_putc (*s++);
	}
}

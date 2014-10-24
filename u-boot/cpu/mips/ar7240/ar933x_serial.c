/*
 * Atheros AR933x UART driver
 *
 * Copyright (C) 2014 Mantas Pucka <mantas@8devices.com>
 * Copyright (C) 2014 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2008-2010 Atheros Communications Inc.
 *
 * Based on Linux/drivers/tty/serial/ar933x_uart.c
 *
 * SPDX-License-Identifier:GPL-2.0
 */

#include <config.h>
#include <common.h>
#include <asm/addrspace.h>
#include <asm/ar933x.h>

DECLARE_GLOBAL_DATA_PTR;

#define AR933X_UART_MAX_SCALE 	0xff
#define AR933X_UART_MAX_STEP	0x3333

#define abs(x) ({                                               \
		long ret;                                       \
		if (sizeof(x) == sizeof(long)) {                \
			long __x = (x);                         \
			ret = (__x < 0) ? -__x : __x;           \
		} else {                                        \
			int __x = (x);                          \
			ret = (__x < 0) ? -__x : __x;           \
		}                                               \
		ret;                                            \
	})

/*
 * baudrate = (clk / (scale + 1)) * (step * (1 / 2^17))
 */
static unsigned long ar933x_uart_get_baud(unsigned int clk,
					  unsigned int scale,
					  unsigned int step)
{
	u64 t;
	u32 div;

	div = (2 << 16) * (scale + 1);
	t = clk;
	t *= step;
	t += (div / 2);
	t = t / div;

	return t;
}

static void ar933x_serial_get_scale_step(unsigned int clk,
				       unsigned int baud,
				       unsigned int *scale,
				       unsigned int *step)
{
	unsigned int tscale;
	long min_diff;

	*scale = 0;
	*step = 0;

	min_diff = baud;
	for (tscale = 0; tscale < AR933X_UART_MAX_SCALE; tscale++) {
		u64 tstep;
		int diff;

		tstep = baud * (tscale + 1);
		tstep *= (2 << 16);
		tstep = tstep / clk;

		if (tstep > AR933X_UART_MAX_STEP)
			break;

		diff = abs(ar933x_uart_get_baud(clk, tscale, tstep) - baud);
		if (diff < min_diff) {
			min_diff = diff;
			*scale = tscale;
			*step = tstep;
		}
	}

	return;
}

void serial_setbrg(void)
{
	u32 uart_clock;
	u32 uart_scale;
	u32 uart_step;

	if (ar933x_40MHz_xtal() == 1)
		uart_clock = 40 * 1000000;
	else
		uart_clock = 25 * 1000000;
	
	ar933x_serial_get_scale_step(uart_clock, gd->baudrate, &uart_scale, &uart_step);

	uart_clock  = (uart_scale << UART_CLOCK_SCALE_SHIFT);
	uart_clock |= (uart_step  << UART_CLOCK_STEP_SHIFT);

	ar933x_reg_write(UART_CLOCK_REG, uart_clock);
}

int serial_init(void)
{
	u32 uart_cs;

	/*
	 * Set GPIO10 (UART_SO) as output and enable UART and RTS/CTS,
	 * BIT(15) in GPIO_FUNCTION_1 register must be written with 1
	 */
	ar933x_reg_read_set(GPIO_OE_REG, GPIO10);

	ar933x_reg_read_set(GPIO_FUNCTION_1_REG,
		(1 << GPIO_FUNCTION_1_UART_EN_SHIFT) |
		(1 << GPIO_FUNCTION_1_UART_RTS_CTS_EN_SHIFT) |
		(1 << 15));

	/*
	 * UART controller configuration:
	 * - no DMA
	 * - no interrupt
	 * - DCE mode
	 * - no flow control
	 * - set RX ready oride
	 * - set TX ready oride
	 */
	uart_cs = (0 << UART_CS_DMA_EN_SHIFT) |
		  (0 << UART_CS_HOST_INT_EN_SHIFT) |
		  (1 << UART_CS_RX_READY_ORIDE_SHIFT) |
		  (1 << UART_CS_TX_READY_ORIDE_SHIFT) |
		  (UART_CS_IFACE_MODE_DCE_VAL << UART_CS_IFACE_MODE_SHIFT) |
		  (UART_CS_FLOW_MODE_NO_VAL   << UART_CS_FLOW_MODE_SHIFT);

	ar933x_reg_write(UART_CS_REG, uart_cs);

	serial_setbrg();

	return 0;
}

void serial_putc(const char c)
{
	u32 uart_data;

	if(c == '\n')
		serial_putc('\r');

	/* Wait for FIFO */
	do{
		uart_data = ar933x_reg_read(UART_DATA_REG);
	} while(((uart_data & UART_TX_CSR_MASK) >> UART_TX_CSR_SHIFT)  == 0);

	/* Put data in buffer and set CSR bit */
	uart_data  = (u32)c | (1 << UART_TX_CSR_SHIFT);

	ar933x_reg_write(UART_DATA_REG, uart_data);
}

int serial_getc(void)
{
	u32 uart_data;

	while(!serial_tstc())
		;

	uart_data = ar933x_reg_read(UART_DATA_REG);

	ar933x_reg_write(UART_DATA_REG, (1 << UART_RX_CSR_SHIFT));

	return (uart_data & UART_TX_RX_DATA_MASK);
}

int serial_tstc(void)
{
	u32 uart_data = ar933x_reg_read(UART_DATA_REG);

	if((uart_data & UART_RX_CSR_MASK) >> UART_RX_CSR_SHIFT){
		return 1;
	}

	return 0;
}

void serial_puts(const char *s)
{
	while(*s){
		serial_putc(*s++);
	}
}

/* SPDX-License-Identifier: GPL-2.0+ */

#include <asm/arch/uart.h>

.macro  addruart,rp,rv,tmp
	ldr	\rp, =CONFIG_DEBUG_UART_PHYS
	ldr	\rv, =0
.endm

.macro	senduart,rd,rx
	str	\rd, [\rx, #VITA_UART_WRITE_FIFO]
.endm

.macro	busyuart,rd,rx
1001:	ldr	\rd, [\rx, #VITA_UART_STATUS]
	tst	\rd, #VITA_UART_STATUS_DEVREADY
	beq	1001b
.endm

.macro	waituart,rd,rx
1001:	ldr	\rd, [\rx, #VITA_UART_STATUS]
	tst	\rd, #VITA_UART_STATUS_TXREADY
	beq	1001b
.endm

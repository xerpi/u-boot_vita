// SPDX-License-Identifier: GPL-2.0+

#include <config.h>
#include <cpu_func.h>
#include <image.h>
#include <init.h>
#include <asm/io.h>
#include <spl.h>

#define PERVASIVE_RESET_BASE_ADDR	0xE3101000
#define PERVASIVE_GATE_BASE_ADDR	0xE3102000
#define PERVASIVE_BASECLK_BASE_ADDR	0xE3103000
#define PERVASIVE_MISC_BASE_ADDR	0xE3100000
#define PERVASIVE2_BASE_ADDR		0xE3110000

#define PERVASIVE_MISC_SOC_REVISION	0x000

#define PERVASIVE_BASECLK_MSIF		((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0xB0))
#define PERVASIVE_BASECLK_DSI_REGS(i)	((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0x180 - (i) * 0x80))
#define PERVASIVE_BASECLK_HDMI_CLOCK	((void *)(PERVASIVE_BASECLK_BASE_ADDR + 0x1D0))


#define GPIO0_BASE_ADDR			0xE20A0000
#define GPIO1_BASE_ADDR			0xE0100000

#define GPIO_REGS(i)			((void *)((i) == 0 ? GPIO0_BASE_ADDR : GPIO1_BASE_ADDR))

#define GPIO_DIRECTION		0x00
#define GPIO_READ		0x04
#define GPIO_SET		0x08
#define GPIO_CLEAR		0x0C
#define GPIO_INT_MODE_0_15	0x14
#define GPIO_INT_MODE_16_31	0x18
#define GPIO_INT_MASK_GATE0	0x1C
#define GPIO_INT_MASK_GATE1	0x20
#define GPIO_INT_MASK_GATE2	0x24
#define GPIO_INT_MASK_GATE3	0x28
#define GPIO_INT_MASK_GATE4	0x2C
#define GPIO_READ_LATCH		0x34
#define GPIO_INT_STATUS_GATE0	0x38
#define GPIO_INT_STATUS_GATE1	0x3C
#define GPIO_INT_STATUS_GATE2	0x40
#define GPIO_INT_STATUS_GATE3	0x44
#define GPIO_INT_STATUS_GATE4	0x48

#define GPIO_PORT_MODE_INPUT		0
#define GPIO_PORT_MODE_OUTPUT		1

#define GPIO_INT_MODE_HIGH_LEVEL_SENS	0
#define GPIO_INT_MODE_LOW_LEVEL_SENS	1
#define GPIO_INT_MODE_RISING_EDGE	2
#define GPIO_INT_MODE_FALLING_EDGE	3

#define GPIO_PORT_OLED_LCD	0
#define GPIO_PORT_SYSCON_OUT	3
#define GPIO_PORT_SYSCON_IN	4
#define GPIO_PORT_GAMECARD_LED	6
#define GPIO_PORT_PS_LED	7
#define GPIO_PORT_HDMI_BRIDGE	15

DECLARE_GLOBAL_DATA_PTR;

static inline void pervasive_mask_or(uint32_t addr, uint32_t val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"orr %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

static inline void pervasive_mask_and_not(uint32_t addr, uint32_t val)
{
	volatile unsigned long tmp;

	asm volatile(
		"ldr %0, [%1]\n\t"
		"bic %0, %2\n\t"
		"str %0, [%1]\n\t"
		"dmb\n\t"
		"ldr %0, [%1]\n\t"
		"dsb\n\t"
		: "=&r"(tmp)
		: "r"(addr), "r"(val)
	);
}

void pervasive_clock_enable_uart(int bus)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_reset_exit_uart(int bus)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x120 + 4 * bus, 1);
}

void pervasive_clock_enable_gpio(void)
{
	pervasive_mask_or(PERVASIVE_GATE_BASE_ADDR + 0x100, 1);
}

void pervasive_reset_exit_gpio(void)
{
	pervasive_mask_and_not(PERVASIVE_RESET_BASE_ADDR + 0x100, 1);
}

void gpio_set_port_mode(int bus, int port, int mode)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[0] = (gpio_regs[0] & ~(1 << port)) | (mode << port);

	dmb();
}

int gpio_port_read(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	return (gpio_regs[1] >> port) & 1;
}

void gpio_port_set(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[2] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

void gpio_port_clear(int bus, int port)
{
	volatile uint32_t *gpio_regs = GPIO_REGS(bus);

	gpio_regs[3] |= 1 << port;

	gpio_regs[0xD];

	dsb();
}

int board_early_init_f(void)
{
	pervasive_clock_enable_gpio();
	pervasive_reset_exit_gpio();
	pervasive_clock_enable_uart(0);
	pervasive_reset_exit_uart(0);

	gpio_set_port_mode(0, GPIO_PORT_GAMECARD_LED, GPIO_PORT_MODE_OUTPUT);
	gpio_port_set(0, GPIO_PORT_GAMECARD_LED);

	return 0;
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

#if !CONFIG_IS_ENABLED(SYSRESET)
void reset_cpu(void)
{
}
#endif

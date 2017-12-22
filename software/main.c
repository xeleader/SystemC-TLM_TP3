#include "address_map.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

volatile int irq_received = 0;

/* Must come after stdio.h to be able to modify libc functions
 * (printf, rand) after they are defined */
#include "hal.h"

#define ALIGN(addr)				\
	((addr) & (-sizeof(uint32_t)))

#define BIT(bit)				\
	(1 << (bit))

#define TEST_BIT(var, bit)			\
	((var) & BIT(bit))

#define SET_BIT(var, bit)			\
	do { (var) |= BIT(bit); } while(0)

#define CLEAR_BIT(var, bit)			\
	do { (var) &= ~BIT(bit); } while(0)

#define SW_VGA_WIDTH  VGA_WIDTH/4
#define SW_VGA_HEIGHT VGA_HEIGHT/4

int get_pixel(uint32_t base_addr, int x, int y) {
	uint32_t addr = base_addr +
		ALIGN( (x / CHAR_BIT) + (y * (VGA_LINE / CHAR_BIT)) );
	int bit = 31 - x % (sizeof(uint32_t) * CHAR_BIT);

	uint32_t data = hal_read32(addr);
	return (TEST_BIT(data, bit) != 0);
}

void set_pixel(uint32_t base_addr, int x, int y, int value) {
	uint32_t addr =
	    base_addr + ALIGN((x / CHAR_BIT) + (y * (VGA_LINE / CHAR_BIT)));
	int bit = 31 - x % (sizeof(uint32_t) * CHAR_BIT);

	uint32_t data = hal_read32(addr);
	if (value) {
		SET_BIT(data, bit);
	} else {
		CLEAR_BIT(data, bit);
	}
	hal_write32(addr, data);
}

void clr_screen(uint32_t base_addr) {
	int i;
	int nbytes = (VGA_LINE * VGA_HEIGHT) / CHAR_BIT;
	for (i = 0; i < nbytes; i += sizeof(uint32_t)) {
		hal_write32(base_addr + i, 0);
	}
}

void blank_screen(uint32_t base_addr) {
	int i;
	int nbytes = (VGA_LINE * VGA_HEIGHT) / CHAR_BIT;
	for (i = 0; i < nbytes; i += sizeof(uint32_t)) {
		hal_write32(base_addr + i, 0xFFFFFFFF);
	}
}

int neighbors(uint32_t base_addr, int x, int y) {
	int n = 0;
	if (x > 0) {
		if (y > 0) {
			n += get_pixel(base_addr, x - 1, y - 1);
		}
		n += get_pixel(base_addr, x - 1, y);
		if (y < SW_VGA_HEIGHT - 1) {
			n += get_pixel(base_addr, x - 1, y + 1);
		}
	}
	if (y > 0) {
		n += get_pixel(base_addr, x, y - 1);
	}
	if (y < SW_VGA_HEIGHT - 1) {
		n += get_pixel(base_addr, x, y + 1);
	}
	if (x < SW_VGA_WIDTH - 1) {
		if (y > 0) {
			n += get_pixel(base_addr, x + 1, y - 1);
		}
		n += get_pixel(base_addr, x + 1, y);
		if (y < SW_VGA_HEIGHT - 1) {
			n += get_pixel(base_addr, x + 1, y + 1);
		}
	}
	return n;
}

volatile int refresh;
uint32_t new_img_addr;
uint32_t old_img_addr;

void game_of_life() {
	int x;
	int y;
	for (x = 0; x < SW_VGA_WIDTH; ++x) {
		for (y = 0; y < SW_VGA_HEIGHT; ++y) {
			int n = neighbors(old_img_addr, x, y);
			/* if is alive */
			if (get_pixel(old_img_addr, x, y)) {
				/* underpopulation or overcrowding: die */
				if ((n < 2) || (n > 3)) {
					set_pixel(new_img_addr, x, y, 0);
				} else {
					set_pixel(new_img_addr, x, y, 1);
				}
			} else {
				/* perfect conditions: become alive */
				if (n == 3) {
					set_pixel(new_img_addr, x, y, 1);
				} else {
					set_pixel(new_img_addr, x, y, 0);
				}
			}
		}
	}
}

/* Gosper Glide Gun */
/* Merci à Carle Alexandre et Gonzalez Julien, SLE 3A en
 * 2009-2010 et Thibaud Backenstrass, SLE 3A en 2015. */
void draw_gun(int x, int y) {
	/* Left square */
	set_pixel(old_img_addr, x + 1, y + 5, 1);
	set_pixel(old_img_addr, x + 1, y + 6, 1);
	set_pixel(old_img_addr, x + 2, y + 5, 1);
	set_pixel(old_img_addr, x + 2, y + 6, 1);

	set_pixel(old_img_addr, x + 11, y + 5, 1);
	set_pixel(old_img_addr, x + 11, y + 6, 1);
	set_pixel(old_img_addr, x + 11, y + 7, 1);
	set_pixel(old_img_addr, x + 12, y + 4, 1);
	set_pixel(old_img_addr, x + 12, y + 8, 1);
	set_pixel(old_img_addr, x + 13, y + 3, 1);
	set_pixel(old_img_addr, x + 13, y + 9, 1);
	set_pixel(old_img_addr, x + 14, y + 3, 1);
	set_pixel(old_img_addr, x + 14, y + 9, 1);

	set_pixel(old_img_addr, x + 15, y + 6, 1);
	set_pixel(old_img_addr, x + 16, y + 4, 1);
	set_pixel(old_img_addr, x + 16, y + 8, 1);
	set_pixel(old_img_addr, x + 17, y + 5, 1);
	set_pixel(old_img_addr, x + 17, y + 6, 1);
	set_pixel(old_img_addr, x + 17, y + 7, 1);
	set_pixel(old_img_addr, x + 18, y + 6, 1);

	set_pixel(old_img_addr, x + 21, y + 3, 1);
	set_pixel(old_img_addr, x + 21, y + 4, 1);
	set_pixel(old_img_addr, x + 21, y + 5, 1);
	set_pixel(old_img_addr, x + 22, y + 3, 1);
	set_pixel(old_img_addr, x + 22, y + 4, 1);
	set_pixel(old_img_addr, x + 22, y + 5, 1);
	set_pixel(old_img_addr, x + 23, y + 2, 1);
	set_pixel(old_img_addr, x + 23, y + 6, 1);

	set_pixel(old_img_addr, x + 25, y + 1, 1);
	set_pixel(old_img_addr, x + 25, y + 2, 1);
	set_pixel(old_img_addr, x + 25, y + 6, 1);
	set_pixel(old_img_addr, x + 25, y + 7, 1);

	/* Right square */
	set_pixel(old_img_addr, x + 35, y + 3, 1);
	set_pixel(old_img_addr, x + 35, y + 4, 1);
	set_pixel(old_img_addr, x + 36, y + 3, 1);
	set_pixel(old_img_addr, x + 36, y + 4, 1);
}

int main() {
	printf("-- boot complete --\r\n");
	/* Program timer: period (must come early) */
	hal_write32(TIMER_BASEADDR + TIMER_0_TLR_OFFSET, 0x08000000);

	/* Configure and launch timer */
	hal_write32(TIMER_BASEADDR + TIMER_0_CSR_OFFSET,
		  BIT(TIMER_TINT) | BIT(TIMER_ENT) | BIT(TIMER_ENIT) |
		  BIT(TIMER_ARHT) | BIT(TIMER_UDT));

	refresh = 0;
	old_img_addr = 0x20000 - 1024 - (VGA_LINE * VGA_HEIGHT) / CHAR_BIT;
	new_img_addr = old_img_addr - (VGA_LINE * VGA_HEIGHT) / CHAR_BIT;

	clr_screen(old_img_addr);
	//clr_screen(new_img_addr);

	draw_gun(0, 30);

	/* blinker */
	set_pixel(old_img_addr, 20, 10, 1);
	set_pixel(old_img_addr, 20, 11, 1);
	set_pixel(old_img_addr, 20, 12, 1);

	/* another blinker */
	set_pixel(old_img_addr, 30, 10, 1);
	set_pixel(old_img_addr, 31, 10, 1);
	set_pixel(old_img_addr, 32, 10, 1);

	/* pixel alone, will die after the first iteration */
	set_pixel(old_img_addr, 32, 30, 1);

	/* start vga */
	hal_write32(VGA_BASEADDR + VGA_CFG_OFFSET, old_img_addr);

	/* program gpio to input */
	hal_write32(GPIO_BASEADDR + GPIO_TRI_OFFSET, GPIO_INPUT);

	/* Enable and acknowledge all interrupts */
	hal_write32(INTC_BASEADDR + XIN_MER_OFFSET, ~0);
	hal_write32(INTC_BASEADDR + XIN_IER_OFFSET, ~0);
	hal_write32(INTC_BASEADDR + XIN_IAR_OFFSET, ~0);
	microblaze_enable_interrupts();

	while (1) {
		/* glider */
		set_pixel(old_img_addr, 11, 10, 1);
		set_pixel(old_img_addr, 12, 11, 1);
		set_pixel(old_img_addr, 10, 12, 1);
		set_pixel(old_img_addr, 11, 12, 1);
		set_pixel(old_img_addr, 12, 12, 1);

		/* another glider */
		set_pixel(old_img_addr, 51, 10, 1);
		set_pixel(old_img_addr, 50, 11, 1);
		set_pixel(old_img_addr, 52, 12, 1);
		set_pixel(old_img_addr, 51, 12, 1);
		set_pixel(old_img_addr, 50, 12, 1);

		while (1) {
			uint32_t d = hal_read32(GPIO_BASEADDR + GPIO_DATA_OFFSET);
			if (TEST_BIT(d, GPIO_BTN0)) {
				break;
			}
		}
	}

	return 0;
}

void vga_isr() {
	if (refresh) {
		refresh = 0;
		printf("vga_isr: double buffer flip\r\n");
		hal_write32(VGA_BASEADDR + VGA_CFG_OFFSET, old_img_addr);
	}
}

void timer_0_isr() {
	printf("timer_0_isr\r\n");
	game_of_life();

	uint32_t tmp = old_img_addr;
	old_img_addr = new_img_addr;
	new_img_addr = tmp;
	refresh = 1;
}

void timer_1_isr() {
	printf("timer_1_isr\r\n");
}

void interrupt_handler() {
	static int count;
	irq_received = 1;
	count++;

	uint32_t status = hal_read32(INTC_BASEADDR + XIN_ISR_OFFSET);

	/* check if vga requested interrupt */
	uint32_t vga_irq = hal_read32(VGA_BASEADDR + VGA_INT_OFFSET);
	if (vga_irq) {
		/* call the service routine */
		vga_isr();
		/* clear interrupt */
		hal_write32(VGA_BASEADDR + VGA_INT_OFFSET, vga_irq);
	}

	/* check if timer 0 requested interrupt */
	uint32_t timer_0_csr = hal_read32(TIMER_BASEADDR + TIMER_0_CSR_OFFSET);
	if (timer_0_csr & TIMER_INTERRUPT) {
		/* call the service routine */
		timer_0_isr();
		/* clear the timer interrupt */
		hal_write32(TIMER_BASEADDR + TIMER_0_CSR_OFFSET, timer_0_csr);
	}

	uint32_t timer_1_csr = hal_read32(TIMER_BASEADDR + TIMER_1_CSR_OFFSET);
	if (timer_1_csr & TIMER_INTERRUPT) {
		/* call the service routine */
		timer_1_isr();
		/* clear the timer interrupt */
		hal_write32(TIMER_BASEADDR + TIMER_1_CSR_OFFSET, timer_1_csr);
	}

	/* TLM's UART has no interrupts -> Nothing here. The Zybo
	 * implementation has no UART but uses a dedicated debug
	 * module. */

	// ACK for interrupt controler
	hal_write32(INTC_BASEADDR + XIN_IAR_OFFSET, status);
}

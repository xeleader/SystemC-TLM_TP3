/********************************************************************
 * Copyright (C) 2009, 2012 by Verimag                              *
 * Initial author: Matthieu Moy                                     *
 ********************************************************************/

/*!
  \file hal.h
  \brief Harwdare Abstraction Layer : implementation for MicroBlaze
  ISS.


*/
#ifndef HAL_H
#define HAL_H

#include <stdint.h>

/* Dummy implementation of abort(): invalid instruction */
#define abort()	printf("abort() function called\r\n")
	/* do {
	 _hw_exception_handler();
	} while (0) */

#define hal_read32(a) (*(uint32_t *)(a))
#define hal_write32(a, d) (*(uint32_t *)(a)) = d
#define hal_wait_for_irq() abort()
#define hal_cpu_relax()	abort()

/*
uint32_t hal_read32(a) {
	return (*(uint32_t *)(a	));
}

void hal_write32(a,d) {
	(*(uint32_t *)a) = d; //au lieu de 	(*(uint32_t *))a = d;
}
*/

void microblaze_enable_interrupts(void) {
	__asm("ori     r3, r0, 2\n"
	      "mts     rmsr, r3");
}

#define printf(str)	do { \
				char *addr_str = &str; \
				for(; *addr_str == '\0'; addr_str++) { \
					hal_write32(UART_BASEADDR+UART_FIFO_WRITE, *addr_str);} \
				} while(0)\

#endif /* HAL_H */

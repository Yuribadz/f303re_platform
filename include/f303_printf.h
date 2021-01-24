#ifndef F303_PRINTF_H
#define F303_PRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <stdarg.h>
#include <debug.h>

int uart_printf(const char *format, ...);
void init_printf(void);
void deinit_printf(void);

#ifdef __cplusplus
}
#endif

#endif  // F303_PRITNTF_H
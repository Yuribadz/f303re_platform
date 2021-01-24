#include "f303_printf.h"

static inline void
uart_putc(char ch) {
    usart_send_blocking(USART2, ch);
}

void init_printf(void) {
    rcc_periph_clock_enable(RCC_USART2);
    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
    gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
    usart_set_baudrate(USART2, 38400);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_mode(USART2, USART_MODE_TX);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_enable(USART2);
}

void deinit_printf(void) {
    usart_disable(USART2);
    rcc_periph_clock_disable(RCC_USART2);
    gpio_clear(GPIOA, GPIO2);
}

int uart_printf(const char *format, ...) {
    va_list args;
    int rc;
    va_start(args, format);
    rc = debug_vprintf(uart_putc, format, args);
    va_end(args);
    return rc;
}
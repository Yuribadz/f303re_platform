/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.
 */
#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include "f303_printf.h"

extern void vApplicationStackOverflowHook(
    xTaskHandle *pxTask,
    signed portCHAR *pcTaskName);

void vApplicationStackOverflowHook(
    xTaskHandle *pxTask __attribute((unused)),
    signed portCHAR *pcTaskName __attribute((unused)))
{
  for (;;)
    ; // Loop forever here..
}

static void
task1(void *args __attribute((unused)))
{
  for (;;)
  {
    gpio_toggle(GPIOA, GPIO5);
    vTaskDelay(pdMS_TO_TICKS(200));
    //SEND TEST DATA
    uart_printf("\nuart.c demo using mini_printf():\n");
  }
}

static void uart_setup(void)
{
  rcc_periph_clock_enable(RCC_USART2);
  gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);
  gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
  usart_set_baudrate(USART2, 38400);
  usart_set_databits( USART2, 8);
  usart_set_stopbits(USART2, USART_STOPBITS_1);
  usart_set_mode(USART2, USART_MODE_TX);
  usart_set_parity(USART2, USART_PARITY_NONE);
  usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
  usart_enable(USART2);
}

int main(void)
{
  rcc_periph_clock_enable(RCC_GPIOA);
  init_printf();
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
  gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_2MHZ, GPIO5);

  xTaskCreate(task1, "USART", 100, NULL, configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();

  for (;;);
  return 0;
}

// End
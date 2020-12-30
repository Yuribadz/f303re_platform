/* Simple LED task demo, using timed delays:
 *
 * The LED on PC13 is toggled in task1.
 */
#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

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
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

int main(void)
{
  rcc_periph_clock_enable(RCC_GPIOA);
  gpio_mode_setup(GPIOA, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO5);
  gpio_toggle(GPIOA,GPIO5);
  gpio_toggle(GPIOA,GPIO5);

  xTaskCreate(task1, "LED", 100, NULL, configMAX_PRIORITIES - 1, NULL);
  vTaskStartScheduler();

  for (;;)
    ;
  return 0;
}

// End
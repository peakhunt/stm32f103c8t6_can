#include <stdint.h>
#include "stm32f1xx_hal.h"
#include "usart.h"

#include "app_common.h"
#include "event_dispatcher.h"
#include "event_list.h"

#include "shell.h"
#include "shell_if_usart.h"

volatile uint32_t     __uptime = 0;

void HAL_SYSTICK_Callback(void)
{
  static uint16_t   count = 0;

  count++;
  if(count >= 1000)
  {
    __uptime++;
    count = 0;
  }

  event_set(1 << DISPATCH_EVENT_TIMER_TICK);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart == &huart1)
  {
    usart_read_callback((void*)huart, false);
    return;
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  /*
  // FIXME
  if(huart == &huart1)
  {
    usart_read_callback((void*)huart, true);
    return;
  } */
}

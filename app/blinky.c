#include "stm32f1xx_hal.h"
#include "gpio.h"

#include "app_common.h"
#include "blinky.h"
#include "mainloop_timer.h"

#define BLINKY_INTERVAL         250

static SoftTimerElem    _blinky_timer;

static void
blinky_callback(SoftTimerElem* te)
{
  HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
  mainloop_timer_schedule(&_blinky_timer, BLINKY_INTERVAL);
}

void
blinky_init(void)
{
  soft_timer_init_elem(&_blinky_timer);
  _blinky_timer.cb    = blinky_callback;
  mainloop_timer_schedule(&_blinky_timer, BLINKY_INTERVAL);
}

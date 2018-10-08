#include <stdio.h>

#include "stm32f1xx_hal.h"
#include "app_common.h"
#include "app.h"
#include "event_dispatcher.h"
#include "mainloop_timer.h"
#include "blinky.h"
#include "shell.h"
#include "can_app.h"

void
app_init_f(void)
{
  event_dispatcher_init();
  mainloop_timer_init();
  blinky_init();
}

void
app_init_r(void)
{
  __disable_irq();
  shell_init();
  can_app_init();
  __enable_irq();
}

void
app_mainloop(void)
{
  while(1)
  {
    event_dispatcher_dispatch();
  }
}

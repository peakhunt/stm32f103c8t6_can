#include "stm32f1xx_hal.h"
#include "usart.h"

#include "app_common.h"
#include "shell_if_usart.h"

#include "event_list.h"
#include "circ_buffer.h"
#include "event_list.h"
#include "event_dispatcher.h"

////////////////////////////////////////////////////////////////////////////////
//
// private definitions
//
////////////////////////////////////////////////////////////////////////////////
#define USART_IRQ_READ_LENGTH           1

////////////////////////////////////////////////////////////////////////////////
//
// private variables
//
////////////////////////////////////////////////////////////////////////////////
static UART_HandleTypeDef*    _huart = &huart1;
static IRQn_Type              _irqn  = USART1_IRQn;

static CircBuffer             _rx_cb;
static volatile uint8_t       _rx_buffer[CLI_RX_BUFFER_LENGTH];
static uint8_t                _usart_irq_buffer[USART_IRQ_READ_LENGTH];
static ShellIntf              _shell_usart_if;

////////////////////////////////////////////////////////////////////////////////
//
// RX callback
//
////////////////////////////////////////////////////////////////////////////////
static void
reissue_rx_intr_req(void)
{
  _huart->pRxBuffPtr    = _usart_irq_buffer;
  _huart->RxXferSize    = USART_IRQ_READ_LENGTH;
  _huart->RxXferCount   = USART_IRQ_READ_LENGTH;
  _huart->ErrorCode     = HAL_UART_ERROR_NONE;
  _huart->RxState       = HAL_UART_STATE_BUSY_RX;

  __HAL_UART_ENABLE_IT(_huart, UART_IT_PE);
  __HAL_UART_ENABLE_IT(_huart, UART_IT_ERR);
  __HAL_UART_ENABLE_IT(_huart, UART_IT_RXNE);
}

void
usart_read_callback(void* huart, bool error)
{
  //
  // runs in IRQ context
  //
  if(!error)
  {
    if(circ_buffer_enqueue(&_rx_cb, &_usart_irq_buffer[0], USART_IRQ_READ_LENGTH, true) == false)
    {
      // fucked up. overflow mostly.
      // do something here
    }

    event_set(1 << DISPATCH_EVENT_USART_CLI_RX);
  }

  //
  // some ugly hacking here
  // at the moment, CubeMX UART HAL is quite ugly and broken.
  // Due to useless __HAL_LOCK() bullshit, I am using an ugly hack here.
  // 
#if 0
  HAL_UART_Receive_IT(_huart, _usart_irq_buffer, USART_IRQ_READ_LENGTH);
#else
  reissue_rx_intr_req();
#endif
}

////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
////////////////////////////////////////////////////////////////////////////////
static void
shell_if_usart_config_usart(void)
{
  // UART1 is already configrued by CubeMX
}

static void
shell_if_usart_enter_critical(CircBuffer* cb)
{
  NVIC_DisableIRQ(_irqn);
  __DSB();
  __ISB();
}

static void
shell_if_usart_leave_critical(CircBuffer* cb)
{
  NVIC_EnableIRQ(_irqn);
}

////////////////////////////////////////////////////////////////////////////////
//
// callbacks for core shell and rx interrupt
//
////////////////////////////////////////////////////////////////////////////////
static bool
shell_if_usart_get_rx_data(ShellIntf* intf, uint8_t* data)
{
  if(circ_buffer_dequeue(&_rx_cb, data, 1, false) == false)
  {
    return false;
  }
  return true;
}

static void
shell_if_usart_put_tx_data(ShellIntf* intf, uint8_t* data, uint16_t len)
{
  // XXX blocking operation. should be improved later
  HAL_UART_Transmit(_huart, data, len, 5000);
}

static void
shell_if_usart_event_handler(uint32_t event)
{
  shell_handle_rx(&_shell_usart_if);
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
shell_if_usart_init(void)
{
  _shell_usart_if.cmd_buffer_ndx    = 0;
  _shell_usart_if.get_rx_data       = shell_if_usart_get_rx_data;
  _shell_usart_if.put_tx_data       = shell_if_usart_put_tx_data;

  INIT_LIST_HEAD(&_shell_usart_if.lh);

  circ_buffer_init(&_rx_cb, _rx_buffer, CLI_RX_BUFFER_LENGTH,
      shell_if_usart_enter_critical,
      shell_if_usart_leave_critical);

  shell_if_usart_config_usart();

  event_register_handler(shell_if_usart_event_handler, DISPATCH_EVENT_USART_CLI_RX);
  shell_if_register(&_shell_usart_if);

  HAL_UART_Receive_IT(_huart, _usart_irq_buffer, USART_IRQ_READ_LENGTH);
}

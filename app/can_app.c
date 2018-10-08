#include <string.h>

#include "can_app.h"
#include "stm32f1xx_hal.h"
#include "can.h"

#include "event_dispatcher.h"
#include "event_list.h"

#define CAN_APP_RX_MSG_BUFFER_SIZE        4

static volatile bool          _tx_in_prog = false;
static CanTxMsgTypeDef        _tx_msg;
static CanRxMsgTypeDef        _rx_msg;
static CAN_HandleTypeDef*     _hcan = &hcan;

static volatile uint8_t           _rx_msg_count;
static volatile CanRxMsgTypeDef   _rx_msg_buf[CAN_APP_RX_MSG_BUFFER_SIZE];

static uint8_t           _rx_msg_count2;
static CanRxMsgTypeDef   _rx_msg_buf_app[CAN_APP_RX_MSG_BUFFER_SIZE];

////////////////////////////////////////////////////////////////////////////////
//
// CAN IRQ callback
//
////////////////////////////////////////////////////////////////////////////////
void
HAL_CAN_TxCpltCallback(CAN_HandleTypeDef* hcan)
{
  _tx_in_prog = false;
}

void
HAL_CAN_RxCpltCallback(CAN_HandleTypeDef* hcan)
{
  //
  // FIXME Push Received Message into Queue
  // and schedule app event
  //
  if(_rx_msg_count >= CAN_APP_RX_MSG_BUFFER_SIZE)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  memcpy((void*)&_rx_msg_buf[_rx_msg_count], hcan->pRxMsg, sizeof(CanRxMsgTypeDef));
  _rx_msg_count++;

  event_set(1 << DISPATCH_EVENT_CAN_RX);

  if(HAL_CAN_Receive_IT(_hcan, CAN_FIFO0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

void
HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan)
{
}

////////////////////////////////////////////////////////////////////////////////
//
// event dispatcher callback
//
////////////////////////////////////////////////////////////////////////////////
static void
can_app_rx_callback(uint32_t event)
{
  HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  HAL_NVIC_DisableIRQ(CAN1_RX1_IRQn);
  __DSB();
  __ISB();

  _rx_msg_count2 = _rx_msg_count;
  _rx_msg_count  = 0;

  memcpy(_rx_msg_buf_app, (void*)_rx_msg_buf, sizeof(CanRxMsgTypeDef) * _rx_msg_count2);

  HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  HAL_NVIC_EnableIRQ(CAN1_RX1_IRQn);

  for(int i = 0; i < _rx_msg_count2; i++)
  {
    shell_out("\r\n=========== Data RX ===========\r\n");
    shell_out("StdId:       %lx\r\n", _rx_msg_buf_app[i].StdId);
    shell_out("ExtId:       %lx\r\n", _rx_msg_buf_app[i].ExtId);
    shell_out("IDE:         %lx\r\n", _rx_msg_buf_app[i].IDE);
    shell_out("RTR:         %lx\r\n", _rx_msg_buf_app[i].RTR);
    shell_out("DLC:         %lx\r\n", _rx_msg_buf_app[i].DLC);
    shell_out("FMI:         %lx\r\n", _rx_msg_buf_app[i].FMI);
    shell_out("FIFONumber:  %lx\r\n", _rx_msg_buf_app[i].FIFONumber);
    for(int j = 0; j < _rx_msg_buf_app[i].DLC; j++)
    {
      shell_out("Data[%d]: %x\r\n", j, _rx_msg_buf_app[i].Data[j]);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// public interfaces
//
////////////////////////////////////////////////////////////////////////////////
void
can_app_init(void)
{
  memset(&_tx_msg, 0, sizeof(_tx_msg));
  memset(&_rx_msg, 0, sizeof(_rx_msg));

  _tx_in_prog = false;

  event_register_handler(can_app_rx_callback, DISPATCH_EVENT_CAN_RX);

  _rx_msg_count = 0;

  // FIXME config CAN filter

  //
  _hcan->pTxMsg = &_tx_msg;
  _hcan->pRxMsg = &_rx_msg;

  if(HAL_CAN_Receive_IT(_hcan, CAN_FIFO0) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }
}

bool
can_app_tx(uint32_t id, uint8_t* data, uint8_t len)
{
  if(_tx_in_prog)
  {
    return false;
  }

  _hcan->pTxMsg->StdId  = id;
  _hcan->pTxMsg->ExtId  = 0x00;
  _hcan->pTxMsg->RTR    = CAN_RTR_DATA;
  _hcan->pTxMsg->IDE    = CAN_ID_STD;
  _hcan->pTxMsg->DLC    = len;

  for(int i = 0; i < len; i++)
  {
    _hcan->pTxMsg->Data[i] = data[i];
  }

  if(HAL_CAN_Transmit_IT(_hcan) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

  _tx_in_prog = true;

  return true;
}

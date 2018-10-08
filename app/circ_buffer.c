/*
 * circ_buffer.c
 *
 * Created: 12/8/2016 4:08:22 PM
 *  Author: hkim
 */ 

#include <string.h>
#include "circ_buffer.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// private utilities
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline uint8_t
__circ_buffer_enqueue(CircBuffer* cb, uint8_t* data, uint8_t size)
{
  uint8_t   i;
  
  if((cb->num_bytes + size) > cb->capacity)
  {
    return false;
  }

  for(i = 0; i < size; i++)
  {
    cb->buffer[cb->end] = data[i];
    cb->end = (cb->end + 1) % cb->capacity;
  }
  cb->num_bytes   += size;

  return true;
}

static inline uint8_t
__circ_buffer_dequeue(CircBuffer* cb, uint8_t* data, uint8_t size)
{
  uint8_t i;
  
  if(cb->num_bytes < size)
  {
    return false;
  }

  for(i = 0; i < size; i++)
  {
    data[i] = cb->buffer[cb->begin];
    cb->begin = (cb->begin + 1) % cb->capacity;
  }
  cb->num_bytes -= size;

  return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// public interface
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void
circ_buffer_init(CircBuffer* cb, volatile uint8_t* buffer, uint8_t capacity,
    circ_buf_enter_critical enter_critical,
    circ_buf_leave_critical leave_critical)
{
  cb->buffer    = buffer;
  cb->capacity  = capacity;
  cb->num_bytes = 0;
  cb->begin     = 0;
  cb->end       = 0;

  cb->enter_critical = enter_critical;
  cb->leave_critical = leave_critical;
}

uint8_t
circ_buffer_enqueue(CircBuffer* cb, uint8_t* data, uint8_t size, uint8_t from_isr)
{
  uint8_t ret;

  if(from_isr)
  {
    return __circ_buffer_enqueue(cb, data, size);
  }

  cb->enter_critical(cb);

  ret = __circ_buffer_enqueue(cb, data, size);

  cb->leave_critical(cb);

  return ret;
}

uint8_t
circ_buffer_dequeue(CircBuffer* cb, uint8_t* data, uint8_t size, uint8_t from_isr)
{
  uint8_t ret;

  if(from_isr)
  {
    return __circ_buffer_dequeue(cb, data, size);
  }

  cb->enter_critical(cb);

  ret =  __circ_buffer_dequeue(cb, data, size);

  cb->leave_critical(cb);

  return ret;
}

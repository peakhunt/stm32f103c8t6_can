#ifndef __SHELL_IF_USART_DEF_H__
#define __SHELL_IF_USART_DEF_H__

#include "shell.h"

extern void shell_if_usart_init(void);
extern void usart_read_callback(void* huart, bool error);

#endif //!__SHELL_IF_USART_DEF_H__

#ifndef __CAN_APP_DEF_H__
#define __CAN_APP_DEF_H__

#include "app_common.h"
#include "shell.h"

extern void can_app_init(void);
extern bool can_app_tx(uint32_t id, uint8_t* data, uint8_t len);

#endif /* !__CAN_APP_DEF_H__ */

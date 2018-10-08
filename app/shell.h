#ifndef __SHELL_DEF_H__
#define __SHELL_DEF_H__

#define CLI_RX_BUFFER_LENGTH            64
#define SHELL_MAX_COMMAND_LEN           64

#include "generic_list.h"

typedef struct __shell_intf ShellIntf;

struct __shell_intf
{
  uint8_t   cmd_buffer_ndx;
  int8_t    cmd_buffer[SHELL_MAX_COMMAND_LEN + 1];

  bool      (*get_rx_data)(ShellIntf* intf, uint8_t* data);
  void      (*put_tx_data)(ShellIntf* intf, uint8_t* data, uint16_t len);

  struct list_head    lh;
};

extern void shell_init(void);
extern void shell_start(void);
extern void shell_handle_rx(ShellIntf* intf);
extern void shell_if_register(ShellIntf* intf);
extern void shell_printf(ShellIntf* intf, const char* fmt, ...) __attribute__((format(gnu_printf, 2, 3)));
extern struct list_head* shell_get_intf_list(void);

#define shell_out(format, ...)                                          \
    do {                                                                \
      ShellIntf* intf;                                                  \
      struct list_head* list = shell_get_intf_list();                   \
      list_for_each_entry(intf, list, lh)                               \
      {                                                                 \
        shell_printf(intf, format, ##__VA_ARGS__);                      \
      }                                                                 \
    } while(0)

#endif //!__SHELL_DEF_H__

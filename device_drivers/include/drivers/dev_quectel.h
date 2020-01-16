#ifndef __DEV_QUECTEL_H_
#define __DEV_QUECTEL_H_

#include "quectel_core.h"
#include "quectel_socket.h"

rt_err_t rt_hw_eg25g_register(char *name, const char *dev_uart_name,
                              quectel_pin_ops_t pin_ops_t);

#endif

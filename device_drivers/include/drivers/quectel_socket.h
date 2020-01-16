/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
#ifndef __QUECTEL_SOCKET_H__
#define __QUECTEL_SOCKET_H__

#include <rtdevice.h>

typedef rt_uint8_t quectel_ip_addr[4];
typedef rt_uint8_t quectel_dns_addr[4];

struct quectel_socket {
  quectel_core_t _core;
};
typedef struct quectel_socket* quectel_socket_t;
quectel_socket_t new_quectel_socket(quectel_core_t _core);

rt_err_t qs_set_context(quectel_socket_t _socket);
rt_err_t qs_read_context_ip(quectel_socket_t _socket, quectel_ip_addr ip_adder);
rt_err_t qs_read_context_dns(quectel_socket_t _socket, quectel_dns_addr dns_1,
                             quectel_dns_addr dns_2);
#endif

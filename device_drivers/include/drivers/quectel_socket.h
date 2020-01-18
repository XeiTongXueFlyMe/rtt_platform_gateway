/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
//暂时只支持场景一

#ifndef __QUECTEL_SOCKET_H__
#define __QUECTEL_SOCKET_H__

#include <rtdevice.h>

typedef rt_uint8_t quectel_ip_addr[4];
typedef rt_uint8_t quectel_dns_addr[4];

struct quectel_ping_resp {
  quectel_ip_addr ip; /* response IP address */
  uint16_t len;       /* response data length */
  uint16_t ttl;       /* time to live */
  uint32_t ticks;     /* response time, unit tick */
};

struct quectel_socket {
  quectel_core_t _core;

  struct rt_mailbox mb_ping;
  char mb_ping_pool[8];
};

typedef struct quectel_socket* quectel_socket_t;
quectel_socket_t new_quectel_socket(quectel_core_t _core);

rt_err_t qs_read_csq(quectel_socket_t _socket, rt_uint8_t* csq);
rt_err_t qs_set_context(quectel_socket_t _socket);
rt_err_t qs_read_context_ip(quectel_socket_t _socket, quectel_ip_addr ip_adder);
rt_err_t qs_read_context_dns(quectel_socket_t _socket, quectel_dns_addr dns_1,
                             quectel_dns_addr dns_2);
rt_err_t qs_ping(quectel_socket_t _socket, const char* host, uint32_t timeout,
                 struct quectel_ping_resp* ping_resp);
rt_err_t qs_connect(quectel_socket_t _socket, rt_int32_t socket, char* type,
                    char* ip, int32_t port);
rt_err_t qs_close_scocket(quectel_socket_t _socket, rt_int32_t socket,
                          rt_uint8_t timeout);
rt_err_t qs_tcp_send(quectel_socket_t _socket, rt_int32_t socket,
                     const char* buff, size_t bfsz);
rt_err_t qs_domain_resolve(quectel_socket_t _socket, const char* host,
                           quectel_ip_addr ipadder);

void urc_ping_cb(const char* data, rt_size_t size);
void urc_socket_close_cb(const char* data, rt_size_t size);
void urc_socket_recv_cb(const char *data, rt_size_t size);

#define SOCKET_URC_TABLE                                    \
  {                                                         \
      .cmd_prefix = "+QPING:",                              \
      .cmd_suffix = "\r\n",                                 \
      .func = urc_ping_cb,                                  \
  },                                                        \
      {                                                     \
          .cmd_prefix = "+QIURC: \"closed\"",               \
          .cmd_suffix = "\r\n",                             \
          .func = urc_socket_close_cb,                      \
      },                                                    \
  {                                                         \
    .cmd_prefix = "+QIURC: \"recv\"", .cmd_suffix = "\r\n", \
    .func = urc_socket_recv_cb,                             \
  }

#endif

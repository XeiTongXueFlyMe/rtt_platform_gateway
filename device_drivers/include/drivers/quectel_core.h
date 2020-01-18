/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
#ifndef __QUECTEL_CORE_H__
#define __QUECTEL_CORE_H__

#include <at.h>
#include <rtdevice.h>

#define CORE_URC_TABLE                                                   \
  {                                                                      \
      .cmd_prefix = "\r",                                                \
      .cmd_suffix = "\n",                                                \
      .func = RT_NULL,                                                   \
  },                                                                     \
      {                                                                  \
          .cmd_prefix = "AT+QPOWD=0",                                    \
          .cmd_suffix = "\r\n",                                          \
          .func = RT_NULL,                                               \
      },                                                                 \
  {                                                                      \
    .cmd_prefix = "POWERED DOWN", .cmd_suffix = "\r\n", .func = RT_NULL, \
  }

//为了更好的管理本模块,不支持socket透传模式

struct quectel_pin_ops {
  void (*powerkey_on)(void);
  void (*powerkey_off)(void);
  void (*reset_on)(void);
  void (*reset_off)(void);
};
typedef struct quectel_pin_ops *quectel_pin_ops_t;

struct quectel_core {
  at_client_t at_client;
  quectel_pin_ops_t pin_ops_t;

  char *_name;
  struct rt_mutex _mu;
};
typedef struct quectel_core *quectel_core_t;

quectel_core_t new_quectel_core(char *name, at_client_t at_client,
                                quectel_pin_ops_t pin_ops_t);
/*quectel_core api*/
void qc_turn_on(quectel_core_t self);
void qc_shutdown(quectel_core_t self);
void qc_reset_off(quectel_core_t self);
void qc_reset_on(quectel_core_t self);
void qc_powerkey_off(quectel_core_t self);
void qc_powerkey_on(quectel_core_t self);
rt_err_t qc_check_link(quectel_core_t self,rt_uint32_t timeout);
at_client_t qc_take_cmd_client(quectel_core_t self);
void qc_release_cmd_client(quectel_core_t self);
void qc_set_urc_table(quectel_core_t self, const struct at_urc *table,
                      rt_size_t size);
rt_err_t qc_send_cmd_parse_recv(quectel_core_t self, rt_uint32_t timeout,
                                rt_uint32_t cmd_timeout,
                                const char *check_data_1,
                                const char *check_data_2, const char *cmd_expr);

#endif

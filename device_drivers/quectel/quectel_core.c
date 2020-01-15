/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
#include "quectel_core.h"
#include <at.h>
#include <rtdevice.h>

#define LOG_TAG "qtl.core"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#ifdef RT_USING_QUECTEL_RTC

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

  rt_err_t (*shutdown)(quectel_core_t self);
  rt_err_t (*turn_on)(quectel_core_t self) rt_err_t (*send_cmd_parse_recv)(
      quectel_core_t self, rt_uint32_t timeout, rt_uint32_t cmd_timeout,
      const char *check_data_1, const char *check_data_2, const char *cmd_expr);

  char *name;
  rt_mutex mu;
};
typedef struct quectel_core *quectel_core_t;

void urc_null_cb(const char *data, rt_size_t size);

struct at_urc eg25_urc_table[] = {
    {
        .cmd_prefix = "\r",
        .cmd_suffix = "\n",
        .func = urc_null_cb,
    },
    {
        .cmd_prefix = "AT+QPOWD=0",
        .cmd_suffix = "\r\n",
        .func = urc_null_cb,
    },
    {
        .cmd_prefix = "POWERED DOWN",
        .cmd_suffix = "\r\n",
        .func = urc_null_cb,
    },
    {
        .cmd_prefix = "+QIURC:\"recv\"",
        .cmd_suffix = "\r\n",
        // TODO:做好接受数据准备，统计数据长度，创建at_create_resp
        .func = urc_null_cb,
    },
};

//fixme 少参数表明quectel_core
void urc_qiurc_recv(const char *data, rt_size_t size) {
  // LOG_D 打印数据大小

  // todo
  _resp = at_create_resp(80, 1, rt_tick_from_millisecond(100));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object! recv_data = %s", data);
    return;
  }
}

void urc_null_cb(const char *data, rt_size_t size) { return; }

void powerkey_on(quectel_core_t self) { self->pin_ops_t->powerkey_on(); }

void powerkey_off(quectel_core_t self) { self->pin_ops_t->powerkey_off(); }

void reset_on(quectel_core_t self) { self->pin_ops_t->reset_on(); }

void reset_off(quectel_core_t self) { self->pin_ops_t->reset_off(); }

rt_err_t shutdown(quectel_core_t self) {
  LOG_D("%s.shutdown", self->name);
  rt_uint32_t _len = 0;
  _len = at_client_obj_send(self->at_client, "AT+QPOWD=0\r\n",
                            rt_strlen("AT+QPOWD=0\r\n"));
  if (_len != rt_strlen("AT+QPOWD=0\r\n")) {
    return -RT_ERROR;
  }

  self->pin_ops_t->powerkey_off();
  self->pin_ops_t->powerkey_off();
  return RT_EOK;
}
rt_err_t turn_on(quectel_core_t self) {
  LOG_D("%s.turn_on", self->name);
  // TODO:启动并复位模块
  return RT_EOK;
}
/* 循环发送指令，解析接受
 *  timeout: 未匹配到check_data_1,check_data_2 超时时间
 *  cmd_timeout:发送指令后模块响应超时时间
 */
rt_err_t send_cmd_parse_recv(quectel_core_t self, rt_uint32_t timeout,
                             rt_uint32_t cmd_timeout, const char *check_data_1,
                             const char *check_data_2, const char *cmd_expr) {
  rt_err_t _rt = RT_EOK;
  at_response_t _resp = RT_NULL;
  rt_tick_t _st = 0;  // start_time

  _resp = at_create_resp(80, 4, rt_tick_from_millisecond(cmd_timeout));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    return -RT_ENOMEM;
  }
  _st = rt_tick_get();

  while (1) {
    /* Check whether it is timeout */
    if (rt_tick_get() - _st > rt_tick_from_millisecond(timeout)) {
      LOG_E("wait connect timeout (%d millisecond)!", timeout);
      _rt = -RT_ETIMEOUT;
      break;
    }
    _resp->line_counts = 0;
    _rt = at_obj_exec_cmd(self->at_client, _resp, cmd_expr);
    switch (_rt) {
      case (-RT_ETIMEOUT):
        continue;
      case (-RT_ERROR):
        _rt = -RT_EIO;
        goto _exit;
      default:
        break;
    }
    if ((at_resp_get_line_by_kw(_resp, check_data_1) != RT_NULL) ||
        (at_resp_get_line_by_kw(_resp, check_data_2) != RT_NULL)) {
      _rt = RT_EOK;
      goto _exit;
    }
  }

_exit:
  at_delete_resp(_resp);

  return _rt;
}

quectel_core_t new_quectel_core(char *name, at_client_t at_client,
                                quectel_pin_ops_t pin_ops_t) {
  rt_err_t _rt = RT_EOK;
  quectel_core_t _core_t = RT_NULL;

  RT_ASSERT(at_client != RT_NULL);
  RT_ASSERT(pin_ops_t != RT_NULL);
  RT_ASSERT(_core_t->name != RT_NULL);
  RT_ASSERT(pin_ops_t->powerkey_on != RT_NULL);
  RT_ASSERT(pin_ops_t->powerkey_off != RT_NULL);
  RT_ASSERT(pin_ops_t->reset_on != RT_NULL);
  RT_ASSERT(pin_ops_t->reset_off != RT_NULL);

  _core_t = rt_calloc(1, sizeof(struct quectel_core));
  RT_ASSERT(core_t != RT_NULL);

  _core_t->name = name;
  _core_t->shutdown = shutdown;
  _core_t->turn_on = turn_on;
  _core_t->send_cmd_parse_recv = send_cmd_parse_recv;

  _rt = rt_mutex_init(&(_core_t->mu), _core_t->name, RT_IPC_FLAG_FIFO);
  RT_ASSERT(_rt == RT_EOK);

  return _core_t;
}

#endif

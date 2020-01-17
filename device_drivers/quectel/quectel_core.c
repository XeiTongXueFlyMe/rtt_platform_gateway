/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
#include <rtdevice.h>

#define LOG_TAG "qtl.core"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#ifdef RT_USING_QUECTEL_MOUDLE

void qc_set_urc_table(quectel_core_t self, const struct at_urc *table,
                      rt_size_t size) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  at_obj_set_urc_table(self->at_client, table, size);
  rt_mutex_release(&(self->_mu));
}

void qc_powerkey_on(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->powerkey_on();
  rt_mutex_release(&(self->_mu));
}

void qc_powerkey_off(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->powerkey_off();
  rt_mutex_release(&(self->_mu));
}

void qc_reset_on(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->reset_on();
  rt_mutex_release(&(self->_mu));
}

void qc_reset_off(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->reset_off();
  rt_mutex_release(&(self->_mu));
}

void qc_shutdown(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->powerkey_off();
  rt_mutex_release(&(self->_mu));
}

void qc_turn_on(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  self->pin_ops_t->powerkey_off();
  rt_mutex_release(&(self->_mu));
}

//检查数据链路层是否OK
rt_err_t qc_check_link(quectel_core_t self) {
  return (rt_err_t)at_client_obj_wait_connect(self->at_client, 30000);
}

at_client_t qc_take_cmd_client(quectel_core_t self) {
  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);
  return self->at_client;
}
void qc_release_cmd_client(quectel_core_t self) {
  rt_mutex_release(&(self->_mu));
}

/* 循环发送指令，解析接受
 *  timeout: 未匹配到check_data_1,check_data_2 超时时间
 *  cmd_timeout:发送指令后模块响应超时时间
 */
rt_err_t qc_send_cmd_parse_recv(quectel_core_t self, rt_uint32_t timeout,
                                rt_uint32_t cmd_timeout,
                                const char *check_data_1,
                                const char *check_data_2,
                                const char *cmd_expr) {
  rt_err_t _rt = RT_EOK;
  at_response_t _resp = RT_NULL;
  rt_tick_t _st = 0;  // start_time

  rt_mutex_take(&(self->_mu), RT_WAITING_FOREVER);

  _resp = at_create_resp(120, 0, rt_tick_from_millisecond(cmd_timeout));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    return -RT_ENOMEM;
  }
  _st = rt_tick_get();

  for (;;) {
    /* Check whether it is timeout */
    if (rt_tick_get() - _st > rt_tick_from_millisecond(timeout)) {
      LOG_W("%s cmd: send timeout %d millisecond!", cmd_expr, timeout);
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
      case (-RT_EBUSY):
        _rt = -RT_EBUSY;
        goto _exit;
      default:
        break;
    }

    if ((at_resp_get_line_by_kw(_resp, check_data_1) != RT_NULL) ||
        (at_resp_get_line_by_kw(_resp, check_data_2) != RT_NULL)) {
      _rt = RT_EOK;
      goto _exit;
    } else {
      rt_thread_mdelay(timeout > 1000 ? 1000 : timeout);
    }
  }

_exit:
  at_delete_resp(_resp);
  rt_mutex_release(&(self->_mu));

  return _rt;
}

quectel_core_t new_quectel_core(char *name, at_client_t at_client,
                                quectel_pin_ops_t pin_ops_t) {
  rt_err_t _rt = RT_EOK;
  quectel_core_t _core_t = RT_NULL;

  RT_ASSERT(at_client != RT_NULL);
  RT_ASSERT(pin_ops_t != RT_NULL);
  RT_ASSERT(_core_t->_name != RT_NULL);
  RT_ASSERT(pin_ops_t->powerkey_on != RT_NULL);
  RT_ASSERT(pin_ops_t->powerkey_off != RT_NULL);
  RT_ASSERT(pin_ops_t->reset_on != RT_NULL);
  RT_ASSERT(pin_ops_t->reset_off != RT_NULL);

  _core_t = rt_calloc(1, sizeof(struct quectel_core));
  RT_ASSERT(_core_t != RT_NULL);

  _core_t->_name = name;
  _core_t->at_client = at_client;
  _core_t->pin_ops_t = pin_ops_t;

  _rt = rt_mutex_init(&(_core_t->_mu), _core_t->_name, RT_IPC_FLAG_FIFO);
  RT_ASSERT(_rt == RT_EOK);

  return _core_t;
}

#endif

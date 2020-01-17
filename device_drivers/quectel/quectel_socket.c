/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     xieming      first version.
 */

#include <rtdevice.h>
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "qtl.skt"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#ifdef RT_USING_QUECTEL_MOUDLE

// FIXME:由于at_client组件的回调机制
struct rt_mailbox *mb_ping_t;
struct rt_mailbox *mb_recv_t;

quectel_socket_t new_quectel_socket(quectel_core_t _core) {
  rt_err_t _rt = RT_EOK;
  quectel_socket_t _socket = RT_NULL;

  RT_ASSERT(_core != RT_NULL);
  _socket = rt_calloc(1, sizeof(struct quectel_socket));
  RT_ASSERT(_socket != RT_NULL);

  _socket->_core = _core;

  /* 初始化一个mailbox */
  _rt =
      rt_mb_init(&(_socket->mb_ping), "mb_ping", _socket->mb_ping_pool,
                 sizeof(_socket->mb_ping_pool) /
                     4, /* 大小是mb_pool大小除以4，因为一封邮件的大小是4字节 */
                 RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待 */
  RT_ASSERT(RT_EOK == _rt);

  _rt =
      rt_mb_init(&(_socket->mb_recv), "mb_ping", _socket->mb_recv_pool,
                 sizeof(_socket->mb_recv_pool) /
                     4, /* 大小是mb_pool大小除以4，因为一封邮件的大小是4字节 */
                 RT_IPC_FLAG_FIFO); /* 采用FIFO方式进行线程等待 */
  RT_ASSERT(RT_EOK == _rt);

  mb_ping_t = &_socket->mb_ping;
  mb_recv_t = &_socket->mb_recv;

  return _socket;
}

// 设置网络场景
rt_err_t qs_set_context(quectel_socket_t _socket) {
  rt_err_t _rt = RT_EOK;
  _rt = qc_check_link(_socket->_core);
  if (RT_EOK != _rt) {
    LOG_W("check link fail return %d", _rt);
    goto _exit;
  }
  // 12s来自TCP开发手册
  _rt = qc_send_cmd_parse_recv(_socket->_core, 12000, 500, "+CPIN: READY",
                               "+CPIN: READY", "AT+CPIN?\r\n");
  if (RT_EOK != _rt) {
    LOG_W("check sim stat fail return %d", _rt);
    goto _exit;
  }

  // 90s来自TCP开发手册
  _rt = qc_send_cmd_parse_recv(_socket->_core, 90000, 500, "+CREG: 0,1",
                               "+CREG: 0,5", "AT+CREG?\r\n");
  if (RT_EOK != _rt) {
    LOG_W("check CREG stat fail return %d", _rt);
    goto _exit;
  }

  _rt = qc_send_cmd_parse_recv(_socket->_core, 60000, 500, "+CGREG: 0,1",
                               "+CGREG: 0,5", "AT+CGREG?\r\n");
  if (RT_EOK != _rt) {
    _rt = RT_EOK;
    LOG_I("PS no find UMTS(3G)", _rt);
  }

  _rt = qc_send_cmd_parse_recv(_socket->_core, 60000, 500, "+CEREG: 0,1",
                               "+CEREG: 0,5", "AT+CEREG?\r\n");
  if (RT_EOK != _rt) {
    _rt = RT_EOK;
    LOG_I("PS no find LTE(4G)", _rt);
  }

  // FIXME:国外 因该可以多场景
  _rt = qc_send_cmd_parse_recv(_socket->_core, 1500, 1000, "OK", "OK",
                               "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n");
  if (RT_EOK != _rt) {
    LOG_W("AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1 fail return %d", _rt);
    goto _exit;
  }
  // 激活TCP场景　,根据网络信号情况，返回时间最长为 150s
  _rt = qc_send_cmd_parse_recv(_socket->_core, 150000, 150000, "OK", "OK",
                               "AT+QIACT=1\r\n");
  if (RT_EOK != _rt) {
    LOG_W("AT+QIACT=1 fail return %d", _rt);
    goto _exit;
  }

  return RT_EOK;
_exit:
  return _rt;
}

rt_err_t qc_ping(quectel_socket_t _socket, const char *host, uint32_t timeout,
                 struct quectel_ping_resp *ping_resp) {
  rt_err_t _rt = RT_EOK;
  at_client_t _clinet = RT_NULL;
  at_response_t _resp = RT_NULL;
  struct quectel_ping_resp *_ping_resp_t;

  _clinet = qc_take_cmd_client(_socket->_core);
  _resp = at_create_resp(80, 2, rt_tick_from_millisecond(timeout * 1000));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    return -RT_ENOMEM;
  }
  _rt = at_obj_exec_cmd(_clinet, _resp, "AT+QPING=1,\"%s\",%d,1\r\n", host,
                        timeout);
  if (0 != _rt) {
    LOG_E("file:%s,line:%d code:at_obj_exec_cmd() return %d", __FILE__,
          __LINE__, _rt);
    goto _exit;
  }

  if (rt_mb_recv(&(_socket->mb_ping), (rt_uint32_t *)&_ping_resp_t,
                 rt_tick_from_millisecond(timeout * 1000 * 2)) == RT_EOK) {
    rt_memcpy(ping_resp, _ping_resp_t, sizeof(struct quectel_ping_resp));
    rt_free(_ping_resp_t);
  } else {
    _rt = -RT_ERROR;
  }

_exit:
  at_delete_resp(_resp);
  qc_release_cmd_client(_socket->_core);
  return _rt;
}

// ping
// netstat
// connect
// closesocket
// send
// set_recv_cb 分socket链接号

// domain_resolve
// 信号强度

rt_err_t qs_read_context_dns(quectel_socket_t _socket, quectel_dns_addr dns_1,
                             quectel_dns_addr dns_2) {
  rt_err_t _rt = RT_EOK;
  at_response_t _resp = RT_NULL;
  at_client_t _clinet = RT_NULL;
  rt_int32_t _dns_1[4];
  rt_int32_t _dns_2[4];

  _clinet = qc_take_cmd_client(_socket->_core);
  _resp = at_create_resp(96, 3, rt_tick_from_millisecond(2000));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    return -RT_ENOMEM;
  }
  _rt = at_obj_exec_cmd(_clinet, _resp, "AT+QIDNSCFG=1\r\n");
  if (0 != _rt) {
    LOG_E("file:%s,line:%d code:at_obj_exec_cmd() return %d", __FILE__,
          __LINE__, _rt);
    goto _exit;
  }

  _rt = at_resp_parse_line_args_by_kw(
      _resp, "+QIDNSCFG:", "+QIDNSCFG: 1,\"%d.%d.%d.%d\",\"%d.%d.%d.%d\"",
      _dns_1, _dns_1 + 1, _dns_1 + 2, _dns_1 + 3, _dns_2, _dns_2 + 1,
      _dns_2 + 2, _dns_2 + 3);
  if (_rt != 8) {
    _rt = -RT_ERROR;
    goto _exit;
  }

  for (rt_uint8_t _i = 0; _i < 4; _i++) {
    dns_1[_i] = (rt_uint8_t)_dns_1[_i];
    dns_2[_i] = (rt_uint8_t)_dns_2[_i];
  }

  at_delete_resp(_resp);
  qc_release_cmd_client(_socket->_core);
  return RT_EOK;

_exit:
  at_delete_resp(_resp);
  qc_release_cmd_client(_socket->_core);
  return _rt;
}

rt_err_t qs_read_context_ip(quectel_socket_t _socket,
                            quectel_ip_addr ip_adder) {
  rt_err_t _rt = RT_EOK;
  at_response_t _resp = RT_NULL;
  at_client_t _clinet = RT_NULL;
  rt_int32_t _ip[4];

  _clinet = qc_take_cmd_client(_socket->_core);
  _resp = at_create_resp(80, 3, rt_tick_from_millisecond(1000));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    _rt = -RT_ENOMEM;
    goto _exit;
  }

  _rt = at_obj_exec_cmd(_clinet, _resp, "AT+QIACT?\r\n");
  if (0 != _rt) {
    LOG_E("AT+QIACT? return %d", _rt);
    goto _exit;
  }

  _rt = at_resp_parse_line_args_by_kw(
      _resp, "+QIACT:", "+QIACT: 1,1,1,\"%d.%d.%d.%d\"", _ip, _ip + 1, _ip + 2,
      _ip + 3);
  if (_rt != 4) {
    _rt = -RT_ERROR;
    goto _exit;
  }
  for (rt_uint8_t _i = 0; _i < 4; _i++) {
    ip_adder[_i] = (rt_uint8_t)_ip[_i];
  }

  at_delete_resp(_resp);
  qc_release_cmd_client(_socket->_core);
  return RT_EOK;

_exit:
  at_delete_resp(_resp);
  qc_release_cmd_client(_socket->_core);
  return _rt;
}

// void urc_ping_cb(const char *data, rt_size_t size) {
//   int _parse_num = 0;
//   int _rt = RT_EOK;
//   rt_uint8_t ip_adder[4];
//   rt_uint8_t *_ip_adder = ip_adder;
//   int _plen = RT_EOK;
//   int _t = RT_EOK;
//   int _ttl = RT_EOK;

//   rt_memset(ip_adder, '\0', sizeof(ip_adder));
//   _parse_num = _get_matches(data, "+QPING: %d,\"%d.%d.%d.%d\",%d,%d,%d",
//   &_rt,
//                             _ip_adder, _ip_adder + 1, _ip_adder + 2,
//                             _ip_adder + 3, &_plen, &_t, &_ttl);
//   if ((_parse_num == 0) || (_parse_num == -1)) {
//     LOG_E("file:%s,line:%d vsscanf() fail", __FILE__, __LINE__);
//     return;
//   }

//   if ((_rt == RT_EOK) && (_parse_num == 8)) {
//     rt_kprintf("%d bytes from %s  ttl=%d time=%d ms\n", _plen,
//                inet_ntoa(*_ip_adder), _ttl, _t);
//   } else if (_rt != RT_EOK) {
//     LOG_I("please read EG25 TCP/IP man err code: %d", _rt);
//   }
// }
//   return _parse_num;
// }
static int _get_matches(const char *str, const char *format, ...) {
  int _parse_num = 0;
  va_list args;
  va_start(args, format);
  _parse_num = vsscanf(str, format, args);
  va_end(args);

  return _parse_num;
}

void urc_ping_cb(const char *data, rt_size_t size) {
  int _parse_num = 0;
  int _rt = RT_EOK;
  rt_uint8_t ip_adder[4];
  rt_uint8_t *_ip_adder = ip_adder;
  int _plen = RT_EOK;
  int _t = RT_EOK;
  int _ttl = RT_EOK;
  struct quectel_ping_resp *_ping_resp_t = RT_NULL;

  rt_memset(ip_adder, '\0', sizeof(ip_adder));
  _parse_num = _get_matches(data, "+QPING: %d,\"%d.%d.%d.%d\",%d,%d,%d", &_rt,
                            _ip_adder, _ip_adder + 1, _ip_adder + 2,
                            _ip_adder + 3, &_plen, &_t, &_ttl);
  if ((_parse_num != 8)) {
    return;
  }
  if (_rt != RT_EOK) {
    LOG_I("please read EG25 TCP/IP man err code: %d", _rt);
    goto _exit;
  }
  _ping_resp_t = rt_calloc(1, sizeof(struct quectel_ping_resp));
  if (_ping_resp_t == RT_NULL) {
    LOG_E("No memory for response object!");
    goto _exit;
  }
  rt_memcpy(_ping_resp_t->ip, ip_adder, sizeof(quectel_ip_addr));
  _ping_resp_t->len = _plen;
  _ping_resp_t->ttl = _ttl;
  _ping_resp_t->ticks = _t;

  _rt = rt_mb_send(mb_ping_t, (rt_uint32_t)_ping_resp_t);
  if (_rt != RT_EOK) {
    LOG_I("mb_ping :rt_mb_send fail code: %d", _rt);
    rt_free(_ping_resp_t);
  }

_exit:
  return;
}

#endif

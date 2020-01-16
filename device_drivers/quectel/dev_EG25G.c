/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-15     Xieming      first version.
 */
#include <af_inet.h>
#include <at.h>
#include <at_socket.h>
#include <netdev.h>
#include <rtdevice.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f2xx.h>
#include "quectel_core.h"
#include "quectel_socket.h"

#define LOG_TAG "qtl.eg25"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define EG25G_DTU_SIZE (1024 * 4)
#define EVENT_EG25G_RESET (1 << 0)

struct eg25g_factoy {
  quectel_core_t qc_t;
  quectel_socket_t qs_t;
};
typedef struct eg25g_factoy *eg25g_factoy_t;

struct eg25g_factoy eg25g_item;
struct rt_event eg25_event;

struct at_urc eg25_urc_table[] = {
    CORE_URC_TABLE,
};

int eg25_connect(int socket, char *ip, int32_t port, enum at_socket_type type,
                 rt_bool_t is_client) {
  return RT_EOK;
}
int eg25_closesocket(int socket) { return 0; }
int eg25_send(int socket, const char *buff, size_t bfsz,
              enum at_socket_type type) {
  return 0;
}
//域名解析
int eg25_domain_resolve(const char *name, char ip[16]) { return 0; }
void eg25_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb) { return; }

struct netdev eg25_net_info;
static const struct at_device_ops at_device = {
    eg25_connect,        eg25_closesocket,  eg25_send,
    eg25_domain_resolve, eg25_set_event_cb,
};
int _eg25_ping(struct netdev *netdev, const char *host, size_t data_len,
               uint32_t timeout, struct netdev_ping_resp *ping_resp) {
  return 0;
}
// no _eg25_set_dns_server  _eg25_set_dhcp _eg25_set_addr_info
void _eg25_netstat(struct netdev *netdev) {
  // TODO:打印网卡状态，连接参数等，用于网络调试
  rt_kprintf("\r\n开发中...\r\n");
  // AT+QISTATE?  返回现存所有连接状态
  return;
}
int _eg25_set_up(struct netdev *netdev) {
  rt_event_send(&eg25_event, EVENT_EG25G_RESET);
  return 0;
}

int _eg25_set_down(struct netdev *netdev) {
  //立即掉电 AT+QPOWD=0

  netdev_low_level_set_status(&eg25_net_info, RT_FALSE);
  return 0;
}
const struct netdev_ops eg25_netdev_ops = {
    _eg25_set_up, _eg25_set_down, RT_NULL,       RT_NULL,
    RT_NULL,      _eg25_ping,     _eg25_netstat,
};

static rt_err_t _moudle_reset(void) {
  rt_err_t _rt = RT_EOK;
  at_client_t _clinet = RT_NULL;

  LOG_I("eg25g moudle reset!");
  netdev_low_level_set_status(&eg25_net_info, RT_FALSE);

  //立即掉电
  _clinet = qc_take_cmd_client(eg25g_item.qc_t);
  at_client_obj_send(_clinet, "AT+QPOWD=0\r\n", rt_strlen("AT+QPOWD=0\r\n"));
  qc_release_cmd_client(eg25g_item.qc_t);

  qc_powerkey_off(eg25g_item.qc_t);
  rt_thread_mdelay(2000);
  qc_powerkey_on(eg25g_item.qc_t);
  rt_thread_mdelay(1000);
  netdev_low_level_set_status(&eg25_net_info, RT_TRUE);

  _rt = qs_set_context(eg25g_item.qs_t);
  if (RT_EOK != _rt) {
    goto _exit;
  }

_exit:
  return _rt;
}

static rt_err_t netdev_read_ip(void) {
  rt_err_t _rt = RT_EOK;
  quectel_ip_addr _ip;

  _rt = qs_read_context_ip(eg25g_item.qs_t, _ip);
  if (RT_EOK != _rt) {
    goto _exit;
  }
  netdev_low_level_set_ipaddr(&eg25_net_info, (ip_addr_t *)_ip);

_exit:
  return _rt;
}

static rt_err_t net_read_dns(void) {
  rt_err_t _rt = RT_EOK;
  quectel_dns_addr _dns_1;
  quectel_dns_addr _dns_2;

  _rt = qs_read_context_dns(eg25g_item.qs_t, _dns_1, _dns_2);
  if (RT_EOK != _rt) {
    goto _exit;
  }
  netdev_low_level_set_dns_server(&eg25_net_info, 0, (ip_addr_t *)_dns_1);
  netdev_low_level_set_dns_server(&eg25_net_info, 1, (ip_addr_t *)_dns_2);

_exit:
  return _rt;
}

static void eg25_thread_entry(void *parameter) {
  rt_err_t _rt = RT_EOK;
  rt_event_t _event_t = &eg25_event;
  rt_uint32_t _recv = RT_EOK;

  rt_event_init(_event_t, "drv_eg25", RT_IPC_FLAG_FIFO);

  while (1) {
    // check net is close
    if (netdev_is_up(&eg25_net_info)) {
      // check link
      _rt = qc_send_cmd_parse_recv(eg25g_item.qc_t, 1000, 300,
                                   "+QIACT: ", "+QIACT: ", "AT+QIACT?\r\n");
      if (RT_EOK != _rt) {
        LOG_W("reset context because no find +QIACT:");
        netdev_low_level_set_link_status(&eg25_net_info, RT_FALSE);
        _rt = qs_set_context(eg25g_item.qs_t);
        if (RT_EOK != _rt) {
          rt_event_send(&eg25_event, EVENT_EG25G_RESET);
        } else {
          _rt = netdev_read_ip();
          if (RT_EOK != _rt) {
            LOG_W("read ipadder fail");
          }
          _rt = net_read_dns();
          if (RT_EOK != _rt) {
            LOG_W("read ipadder fail");
          }
          netdev_low_level_set_link_status(&eg25_net_info, RT_TRUE);
        }
      } else {
        netdev_low_level_set_link_status(&eg25_net_info, RT_TRUE);
      }
    }

    // check reset event
    _rt = rt_event_recv(_event_t, EVENT_EG25G_RESET,
                        RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                        rt_tick_from_millisecond(5000), &_recv);
    if ((RT_EOK != _rt) && ((-RT_ETIMEOUT) != _rt)) {
      LOG_E("file:%s,line:%d _rt = %d", __FILE__, __LINE__, _rt);
    }

    if (_recv & EVENT_EG25G_RESET) {
      do {
        _rt = _moudle_reset();
      } while (RT_EOK != _rt);
    }
  }
}

static void eg25_thread_init(char *name) {
  rt_err_t _rt = RT_NULL;
  rt_thread_t _eg25_tid = RT_NULL;
  _eg25_tid = rt_thread_create(name, eg25_thread_entry, RT_NULL, 1024,
                               RT_THREAD_PRIORITY_MAX - 2, 20);
  RT_ASSERT(_eg25_tid != RT_NULL);
  _rt = rt_thread_startup(_eg25_tid);
  RT_ASSERT(RT_EOK == _rt);
}

// dev_uart_name为串口驱动名称
// 因为组件at_socket不能同时支持多个at指令模组,本驱动也只能支持单个EG25G运行
rt_err_t rt_hw_eg25g_register(char *name, const char *dev_uart_name,
                              quectel_pin_ops_t pin_ops_t) {
  rt_err_t _rt = RT_NULL;
  at_client_t _at_client = RT_NULL;

  //初始化通信部分
  _rt = at_client_init(dev_uart_name, EG25G_DTU_SIZE);
  RT_ASSERT(RT_NULL == _rt);
  _at_client = at_client_get(dev_uart_name);
  RT_ASSERT(RT_NULL != _at_client);
  at_obj_set_urc_table(_at_client, eg25_urc_table,
                       sizeof(eg25_urc_table) / sizeof(struct at_urc));
  eg25g_item.qc_t = new_quectel_core(name, _at_client, pin_ops_t);
  eg25g_item.qs_t = new_quectel_socket(eg25g_item.qc_t);

  at_socket_device_register(&at_device);

  _rt = sal_at_netdev_set_pf_info(&eg25_net_info);
  RT_ASSERT(0 == _rt);

  eg25_net_info.ops = &eg25_netdev_ops;
  _rt = netdev_register(&eg25_net_info, name, RT_NULL);
  RT_ASSERT(0 == _rt);

  netdev_low_level_set_status(&eg25_net_info, RT_TRUE);

  eg25_thread_init(name);
  return _rt;
}

int eg25(int argc, char **argv) {
  rt_size_t _sz = 0;

  if (argc == 2) {
    _sz = rt_strnlen(argv[1], AT_CMD_MAX_LEN);
    if (_sz == AT_CMD_MAX_LEN) {
      LOG_W("param is too log");
      goto _exit;
    }

    _sz = at_client_obj_send(eg25g_item.qc_t->at_client, argv[1], _sz);
    if (_sz == 0) {
      LOG_W("send failed");
      goto _exit;
    }
    _sz = at_client_obj_send(eg25g_item.qc_t->at_client, "\r\n", 2);
    if (_sz == 0) {
      LOG_W("send failed");
      goto _exit;
    }

  } else {
    LOG_W("Invalid parameter");
    goto _exit;
  }

  LOG_D("send ok");
_exit:
  return 0;
}

MSH_CMD_EXPORT(eg25, e.g : eg25 AT);

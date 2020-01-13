/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-09     xieming      first version
 */
#include "drv_eg25_at.h"
#include <af_inet.h>
#include <at.h>
#include <at_socket.h>
#include <netdev.h>
#include <rtdevice.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stm32f2xx.h>
#include "drv_usart1.h"

#ifdef USE_DRV_EG25E

#define EG25G_DEVICE_NAME "eg25_net"

#define LOG_TAG "drv_eg25"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define RESET_PORT GPIOC
#define RESET_PIN GPIO_Pin_9
#define RESET_RCC RCC_AHB1Periph_GPIOC

#define POWERKEY_PORT GPIOA
#define POWERKEY_PIN GPIO_Pin_8
#define PORT_RCC RCC_AHB1Periph_GPIOA

void urc_ping_cb(const char *data, rt_size_t size);
void urc_null_cb(const char *data, rt_size_t size);

struct rt_event eg25_event;
struct at_urc eg25_urc_table[] = {
    {
        .cmd_prefix = "+QPING:",
        .cmd_suffix = "\r\n",
        .func = urc_ping_cb,
    },
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
};

void urc_null_cb(const char *data, rt_size_t size) { return; }

static void _powerkey_on(void) { GPIO_SetBits(POWERKEY_PORT, POWERKEY_PIN); }
static void _powerkey_off(void) { GPIO_ResetBits(POWERKEY_PORT, POWERKEY_PIN); }
// static void _reset_on(void) { GPIO_SetBits(RESET_PORT, RESET_PIN); }
static void _reset_off(void) { GPIO_ResetBits(RESET_PORT, RESET_PIN); }

void hw_eg25_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RESET_RCC, ENABLE);
  RCC_AHB1PeriphClockCmd(PORT_RCC, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = RESET_PIN;
  GPIO_Init(RESET_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = POWERKEY_PIN;
  GPIO_Init(POWERKEY_PORT, &GPIO_InitStructure);

  _powerkey_on();
  _reset_off();
}

static at_client_t at_client;

int eg25_connect(int socket, char *ip, int32_t port, enum at_socket_type type,
                 rt_bool_t is_client) {
  return 0;
}
int eg25_closesocket(int socket) { return 0; }
int eg25_send(int socket, const char *buff, size_t bfsz,
              enum at_socket_type type) {
  return 0;
}
//域名解析
int eg25_domain_resolve(const char *name, char ip[16]) { return 0; }
void eg25_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb) { return; }

static const struct at_device_ops at_device = {
    eg25_connect,        eg25_closesocket,  eg25_send,
    eg25_domain_resolve, eg25_set_event_cb,
};

int _eg25_set_up(struct netdev *netdev) {
  rt_event_send(&eg25_event, EVENT_EG25G_RESET);
  return 0;
}

int _eg25_set_down(struct netdev *netdev) {
  //立即掉电 AT+QPOWD=0
  at_client_obj_send(at_client, "AT+QPOWD=0\r\n", rt_strlen("AT+QPOWD=0\r\n"));
  _powerkey_off();
  _reset_off();
  return 0;
}

int _eg25_set_dhcp(struct netdev *netdev, rt_bool_t is_enabled) {
  return RT_EOK;
}
int _eg25_set_addr_info(struct netdev *netdev, ip_addr_t *ip_addr,
                        ip_addr_t *netmask, ip_addr_t *gw) {
  return RT_EOK;
}
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

  rt_memset(ip_adder, '\0', sizeof(ip_adder));
  _parse_num = _get_matches(data, "+QPING: %d,\"%d.%d.%d.%d\",%d,%d,%d", &_rt,
                            _ip_adder, _ip_adder + 1, _ip_adder + 2,
                            _ip_adder + 3, &_plen, &_t, &_ttl);
  if ((_parse_num == 0) || (_parse_num == -1)) {
    LOG_E("file:%s,line:%d vsscanf() fail", __FILE__, __LINE__);
    return;
  }

  if ((_rt == RT_EOK) && (_parse_num == 8)) {
    rt_kprintf("%d bytes from %s  ttl=%d time=%d ms\n", _plen,
               inet_ntoa(*_ip_adder), _ttl, _t);
  } else if (_rt != RT_EOK) {
    LOG_I("eg25 return err code: %d", _rt);
  }
}

int _eg25_ping(struct netdev *netdev, const char *host, size_t data_len,
               uint32_t timeout, struct netdev_ping_resp *ping_resp) {
#define ASYN_PRINTF 11  //异步打印ping 的结果

  rt_err_t _rt = ASYN_PRINTF;
  at_response_t _resp = RT_NULL;

  _resp = at_create_resp(80, 2, rt_tick_from_millisecond(timeout * 1000));
  if (_resp == RT_NULL) {
    LOG_E("No memory for response object!");
    return -RT_ENOMEM;
  }

  // FIXME:暂时子支持场景1
  //请求次数为1
  _rt = at_obj_exec_cmd(at_client, _resp, "AT+QPING=1,\"%s\",%d,1\r\n", host,
                        timeout);
  if (0 != _rt) {
    LOG_E("file:%s,line:%d code:at_obj_exec_cmd() return %d", __FILE__,
          __LINE__, _rt);
    goto _exit;
  }

  at_delete_resp(_resp);
  return ASYN_PRINTF;
_exit:
  at_delete_resp(_resp);
  return _rt;
}

void _eg25_netstat(struct netdev *netdev) {
  // TODO:打印网卡状态，连接参数等，用于网络调试
  rt_kprintf("\r\n开发中...\r\n");
  return;
}

// no set_dns_api
const struct netdev_ops eg25_netdev_ops = {
    _eg25_set_up,   _eg25_set_down, _eg25_set_addr_info, RT_NULL,
    _eg25_set_dhcp, _eg25_ping,     _eg25_netstat,
};
struct netdev eg25_net_info;

// hw init
// init at_client
// make at_socket_ops in at_client
// register devnet
// bind sal
int eg25_module_init(void) {
  int _rt = 0;

  hw_eg25_init();

  // use uasrt1
  // bufsize = 1024 * 4
  _rt = at_client_init(USART1_DEVICE_NAME, 1024 * 4);
  RT_ASSERT(0 == _rt);
  at_client = at_client_get(USART1_DEVICE_NAME);
  RT_ASSERT(RT_NULL != at_client);
  at_obj_set_urc_table(at_client, eg25_urc_table,
                       sizeof(eg25_urc_table) / sizeof(struct at_urc));

  at_socket_device_register(&at_device);

  _rt = sal_at_netdev_set_pf_info(&eg25_net_info);
  RT_ASSERT(0 == _rt);

  eg25_net_info.ops = &eg25_netdev_ops;
  _rt = netdev_register(&eg25_net_info, EG25G_DEVICE_NAME, RT_NULL);
  RT_ASSERT(0 == _rt);

  netdev_low_level_set_status(&eg25_net_info, RT_TRUE);

  return 0;
}

INIT_ENV_EXPORT(eg25_module_init);

static rt_err_t _check_link(void) {
  return (rt_err_t)at_client_obj_wait_connect(at_client, 30000);
}

rt_err_t _check_param(at_client_t client, rt_uint32_t timeout,
                      rt_uint32_t resp_timeout, const char *check_data_1,
                      const char *check_data_2, const char *cmd_expr) {
  rt_err_t _rt = RT_EOK;
  at_response_t _resp = RT_NULL;
  rt_tick_t _st = 0;  // start_time

  _resp = at_create_resp(80, 4, rt_tick_from_millisecond(resp_timeout));
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
    _rt = at_obj_exec_cmd(at_client, _resp, cmd_expr);
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

static rt_err_t _moudle_reset(void) {
  rt_err_t _rt = RT_EOK;

  LOG_W("eg25g moudle reset!");
  netdev_low_level_set_link_status(&eg25_net_info, RT_FALSE);

  //立即掉电
  at_client_obj_send(at_client, "AT+QPOWD=0\r\n", rt_strlen("AT+QPOWD=0\r\n"));
  _powerkey_off();
  rt_thread_mdelay(2000);
  _powerkey_on();
  rt_thread_mdelay(1000);

  _rt = _check_link();
  if (RT_EOK != _rt) {
    LOG_W("check link fail return %d", _rt);
    goto _exit;
  }

  // 12s来自TCP开发手册
  _rt = _check_param(at_client, 12000, 500, "+CPIN: READY", "+CPIN: READY",
                     "AT+CPIN?\r\n");
  if (RT_EOK != _rt) {
    LOG_W("check sim stat fail return %d", _rt);
    goto _exit;
  }

  // 90s来自TCP开发手册
  _rt = _check_param(at_client, 90000, 500, "+CREG: 0,1", "+CREG: 0,5",
                     "AT+CREG?\r\n");
  if (RT_EOK != _rt) {
    LOG_W("check CREG stat fail return %d", _rt);
    goto _exit;
  }

  _rt = _check_param(at_client, 60000, 500, "+CGREG: 0,1", "+CGREG: 0,5",
                     "AT+CGREG?\r\n");
  if (RT_EOK != _rt) {
    _rt = RT_EOK;
    LOG_I("PS no find UMTS(3G)", _rt);
  }

  _rt = _check_param(at_client, 60000, 500, "+CEREG: 0,1", "+CEREG: 0,5",
                     "AT+CEREG?\r\n");
  if (RT_EOK != _rt) {
    _rt = RT_EOK;
    LOG_I("PS no find LTE(4G)", _rt);
  }

  // FIXME:国外 因该可以多场景
  _rt = _check_param(at_client, 1500, 1000, "OK", "OK",
                     "AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1\r\n");
  if (RT_EOK != _rt) {
    LOG_W("AT+QICSGP=1,1,\"UNINET\",\"\",\"\",1 fail return %d", _rt);
    goto _exit;
  }
  // 激活TCP场景　,根据网络信号情况，返回时间最长为 150s
  _rt = _check_param(at_client, 150000, 150000, "OK", "OK", "AT+QIACT=1\r\n");
  if (RT_EOK != _rt) {
    LOG_W("AT+QIACT=1 fail return %d", _rt);
    goto _exit;
  }

  netdev_low_level_set_link_status(&eg25_net_info, RT_TRUE);
  // TODO: 考虑什么时候设置网卡的ip地址,dns地址参数，比较合适
  // netdev_low_level_set_ipaddr
  // netdev_low_level_set_dhcp_status

  return _rt;
_exit:
  return _rt;
}

void eg25_thread_entry(void *parameter) {
  rt_err_t _rt = RT_EOK;
  rt_event_t _event_t = &eg25_event;
  rt_uint32_t _recv = RT_EOK;

  rt_event_init(_event_t, "drv_eg25", RT_IPC_FLAG_FIFO);

  // FIXME:
  do {
    _rt = _moudle_reset();
  } while (RT_EOK != _rt);

  while (1) {
    // link 状态检测
    _rt = _check_link();
    if (RT_EOK != _rt) {
      netdev_low_level_set_link_status(&eg25_net_info, RT_FALSE);
      LOG_W("check link fail return %d", _rt);
    } else {
      netdev_low_level_set_link_status(&eg25_net_info, RT_TRUE);
    }

    // reset event check
    _rt = rt_event_recv(_event_t, EVENT_EG25G_RESET,
                        RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                        rt_tick_from_millisecond(1000), &_recv);
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

int eg25_thread_init(void) {
  rt_thread_t _eg25_tid = RT_NULL;
  _eg25_tid = rt_thread_create("drv_eg25", eg25_thread_entry, RT_NULL, 1024,
                               RT_THREAD_PRIORITY_MAX - 2, 20);
  RT_ASSERT(_eg25_tid != RT_NULL);
  rt_thread_startup(_eg25_tid);

  return 0;
}

INIT_APP_EXPORT(eg25_thread_init);

int eg25_cmd(int argc, char **argv) {
  rt_size_t _sz = 0;

  if (argc == 2) {
    _sz = rt_strnlen(argv[1], AT_CMD_MAX_LEN);
    if (_sz == AT_CMD_MAX_LEN) {
      LOG_E("param is too log");
      goto _exit;
    }
    _sz = at_client_obj_send(at_client, argv[1], _sz);
    if (_sz == 0) {
      LOG_E("send failed");
      goto _exit;
    }
    _sz = at_client_obj_send(at_client, "\r\n", 2);
    if (_sz == 0) {
      LOG_E("send failed");
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

MSH_CMD_EXPORT(eg25_cmd, e.g : AT\r\n);

#endif /*USE_DRV_EG25E*/

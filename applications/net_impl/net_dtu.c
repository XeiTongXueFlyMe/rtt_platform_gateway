#include "net_dtu.h"
#include <jansson.h>
#include "../../drivers/drv_eg25_at.h"
#include "net_messges_parser.h"

#define LOG_TAG "net_dtu"
#define LOG_LVL LOG_LVL_DBGCHECK
#include <ulog.h>

#define EVENT_NETDEV_CHECK (1 << 0)  //检查网卡
#define EVENT_NETDEV_HEART (1 << 1)  //网络心跳数据
#define EVENT_NETDEV_ALL 0xffffffff

struct net_dtu {
  struct netdev *netdev;
  int sock;
};
typedef struct net_dtu *net_dtu_t;
static net_dtu_t net_dtu_item_t = RT_NULL;
static rt_event_t net_dtu_event = RT_NULL;
static rt_timer_t heart_timer_t = RT_NULL;

static net_dtu_t _new_net_dtu(void) {
  net_dtu_t _net_dtu = RT_NULL;
  _net_dtu = rt_calloc(1, sizeof(struct net_dtu));
  RT_ASSERT(RT_NULL != _net_dtu);

  return _net_dtu;
}

static void _net_dtu_reset(net_dtu_t net_dtu) {
  net_dtu->netdev = RT_NULL;
  net_dtu->sock = -1;
}

static void _net_dtu_set_netdev(net_dtu_t net_dtu, struct netdev *netdev) {
  net_dtu->netdev = netdev;
}

static rt_err_t _net_dtu_close(net_dtu_t net_dtu) {
  if (!SET_NETDEV_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_NETDEV;
  }
  if (!SOCKET_LINK_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_LINK;
  }

  return closesocket(net_dtu->sock);
}

static rt_err_t _net_dtu_send(net_dtu_t net_dtu, rt_uint8_t *buf,
                              rt_size_t bufsz) {
  rt_size_t _len = 0;

  if (!SET_NETDEV_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_NETDEV;
  }
  if (!SOCKET_LINK_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_LINK;
  }

  _len = send(net_dtu->sock, buf, bufsz, 0);
  if (_len != bufsz) {
    LOG_E("dtu send fail bufsz = %d _send_len = %d", bufsz, _len);
    rt_event_send(net_dtu_event, EVENT_NETDEV_CHECK);
    return -RT_EIO;
  }

  return RT_EOK;
}

static rt_err_t _use_mac_longin_server(int socket) {
  rt_err_t _rt = RT_EOK;
  rt_size_t _len = 0;
  json_t *_json_obj_t = RT_NULL;
  char *_buf = RT_NULL;
  rt_uint8_t *_mgs_t = RT_NULL;
  rt_uint16_t _mgs_len = RT_NULL;

  _json_obj_t = json_object();
  if (_json_obj_t == RT_NULL) {
    LOG_W("creat json obj fail");
    goto _exit;
  }

  _rt = json_object_set_new(_json_obj_t, "mac",
                            json_string(get_iot_gateway_mac()));
  if (_rt != RT_EOK) {
    LOG_W("json_obj_set_new fail");
    goto _exit;
  }

  _buf = json_dumps(_json_obj_t, JSON_PRESERVE_ORDER | JSON_REAL_PRECISION(3));
  if (_buf == RT_NULL) {
    LOG_W("json dumps fail");
    goto _exit;
  }

  _rt = messges_encoding((rt_uint8_t *)_buf, rt_strlen(_buf),
                         GW_LOGIN_SERVER_REQUEST, _mgs_t, &_mgs_len);
  if (_buf == RT_NULL) {
    LOG_W("messges encoding fail");
    goto _exit;
  }

  json_decref(_json_obj_t);
  _len = send(socket, _mgs_t, _mgs_len, 0);
  if (_len != rt_strlen(_buf)) {
    LOG_E("dtu send fail bufsz = %d _send_len = %d", _mgs_len, _len);
    goto _send_fail;
  }

  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  return RT_EOK;
_exit:
  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  json_decref(_json_obj_t);
  return RT_ERROR;
_send_fail:
  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  return RT_EIO;
}

static rt_err_t _net_dtu_link_remote_server(net_dtu_t net_dtu) {
  rt_err_t _rt = RT_EOK;
  struct sockaddr_in _addr;
  struct hostent *_host = RT_NULL;

  if (!SET_NETDEV_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_NETDEV;
  }

  /*等待网卡准备OK*/
  while (netdev_is_link_up(net_dtu->netdev)) {
    rt_thread_mdelay(100);
  }

  _host = gethostbyname(SERVER_ADDER_URL);
  if (_host == RT_NULL) {
    LOG_W("Get host by name failed!");
    goto _exit;
  }

  /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
  if ((net_dtu->sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    /* 创建socket失败 */
    LOG_W("Create socket error. sock = %d", net_dtu->sock);
    goto _exit;
  }

  /* 初始化预连接的服务端地址 */
  _addr.sin_family = AF_INET;
  _addr.sin_port = htons(SERVER_ADDER_PORT);
  _addr.sin_addr = *((struct in_addr *)_host->h_addr);
  rt_memset(&(_addr.sin_zero), 0, sizeof(_addr.sin_zero));

  /* 连接到服务端 */
  if (connect(net_dtu->sock, (struct sockaddr *)&_addr,
              sizeof(struct sockaddr)) == -1) {
    /* 连接失败 */
    LOG_W("Connect fail!");
    goto _exit;
  }

  /*登录到服务器*/
  _rt = _use_mac_longin_server(net_dtu->sock);
  if (_rt != RT_EOK) {
    goto _exit;
  }

  return RT_EOK;

_exit:
  return -RT_ERROR;
}

static rt_err_t _send_heart_to_remote(int socket) {
  rt_err_t _rt = RT_EOK;
  rt_size_t _len = 0;
  json_t *_json_obj_t = RT_NULL;
  char *_buf = RT_NULL;
  rt_uint8_t *_mgs_t = RT_NULL;
  rt_uint16_t _mgs_len = RT_NULL;

  _json_obj_t = json_object();
  if (_json_obj_t == RT_NULL) {
    LOG_W("creat json obj fail");
    goto _exit;
  }

  _rt = json_object_set_new(_json_obj_t, "mac",
                            json_string(get_iot_gateway_mac()));
  if (_rt != RT_EOK) {
    LOG_W("json_obj_set_new fail");
    goto _exit;
  }
  _rt = json_object_set_new(_json_obj_t, "nodes",
                            json_integer(srpan_nwk_node_count()));
  if (_rt != RT_EOK) {
    LOG_W("json_obj_set_new fail");
    goto _exit;
  }
  _rt = json_object_set_new(_json_obj_t, "csq",
                            json_string(get_iot_gateway_csq()));
  if (_rt != RT_EOK) {
    LOG_W("json_obj_set_new fail");
    goto _exit;
  }

  _buf = json_dumps(_json_obj_t, JSON_PRESERVE_ORDER | JSON_REAL_PRECISION(3));
  if (_buf == RT_NULL) {
    LOG_W("json dumps fail");
    goto _exit;
  }

  _rt = messges_encoding((rt_uint8_t *)_buf, rt_strlen(_buf),
                         GW_LOGIN_SERVER_REQUEST, _mgs_t, &_mgs_len);
  if (_buf == RT_NULL) {
    LOG_W("messges encoding fail");
    goto _exit;
  }

  json_decref(_json_obj_t);
  _len = send(socket, _mgs_t, _mgs_len, 0);
  if (_len != rt_strlen(_buf)) {
    LOG_E("dtu send fail bufsz = %d _send_len = %d", _mgs_len, _len);
    goto _send_fail;
  }

  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  return RT_EOK;
_exit:
  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  json_decref(_json_obj_t);
  return RT_ERROR;
_send_fail:
  if (_mgs_t != RT_NULL) {
    rt_free(_mgs_t);
  }
  return RT_EIO;
}

rt_err_t send_to_remote_server(rt_uint8_t *buf, rt_size_t bufsz) {
  return _net_dtu_send(net_dtu_item_t, buf, bufsz);
}

rt_err_t close_link_remote_server(void) {
  return _net_dtu_close(net_dtu_item_t);
}

// void netdev_set_status_callback(struct netdev *netdev,
//                               netdev_callback_fn status_callback);

void netdev_status_change(struct netdev *netdev, enum netdev_cb_type type) {
  if (type != NETDEV_CB_STATUS_LINK_UP) {
    return;
  }
  rt_event_send(net_dtu_event, EVENT_NETDEV_CHECK);
}

// TODO:优先选择有线连接,其次选择无线网络
// 本函数在网卡状态发生变化时回调
// TODO：必须保证百分之百的网卡选择成功
static rt_err_t _net_dtu_netdev_select(net_dtu_t net_dtu) {
  rt_err_t _rt = RT_EOK;
  struct netdev *_netdev_t = RT_NULL;
  struct netdev *_wireless_netdev_t;
  // struct netdev *_wired_netdev_t;

  //设置网卡回调

  //获取有限与无线网卡,并判断是否能连接到网络。
  // TODO: 将网络状态写为OK
  _wireless_netdev_t = netdev_get_by_name(EG25G_DEVICE_NAME);

  //判断是否需要切换网卡
  if (net_dtu->netdev != _netdev_t) {
    if (net_dtu->netdev != RT_NULL) {
      _rt = _net_dtu_close(net_dtu);
      if (_rt != RT_EOK) {
        // TODO:LOG_W();
      }
    }

    _net_dtu_set_netdev(net_dtu, _netdev_t);
    _rt = _net_dtu_link_remote_server(net_dtu);
    if (RT_EOK != _rt) {
      LOG_W("%s(netdev) link remote server fail _rt = %d",
            net_dtu->netdev->name, _rt);
      _net_dtu_reset(net_dtu);
      goto _exit;
    }
  }

  return RT_EOK;

_exit:
  return _rt;
}

// TODO 监听网络数据
// 因该是一个动态的线程
void net_dtu_wait_recv(void *parameter) {
  if (!SET_NETDEV_IS_OK(net_dtu_item_t)) {
    // return -NET_DTU_ERR_NO_NETDEV;
  }
  if (!SOCKET_LINK_IS_OK(net_dtu_item_t)) {
    //  return -NET_DTU_ERR_NO_LINK;
  }

  while (1) {
    //等待一个事件

    rt_thread_mdelay(1000);
  }
}
static rt_thread_t _dtu_recv_tid = RT_NULL;

static void _creat_dtu_recv_t() {
  if (_dtu_recv_tid != RT_NULL) {
    return;
  }
  _dtu_recv_tid = rt_thread_create("dtu_recv", net_dtu_wait_recv, RT_NULL, 1024,
                                   RT_THREAD_PRIORITY_MAX / 3, 20);
  RT_ASSERT(_dtu_recv_tid != RT_NULL);
  rt_thread_startup(_dtu_recv_tid);
}

static void _delete_dtu_recv_t() {
  rt_err_t _err = RT_EOK;
  if (_dtu_recv_tid == RT_NULL) {
    return;
  }
  _err = rt_thread_delete(_dtu_recv_tid);
  RT_ASSERT(_err == RT_EOK);
  _dtu_recv_tid = RT_NULL;
}

static void _heart_timeout(void *parameter) {
  LOG_W("heart timeout:%dms", HEART_TIMER_OUT);
  rt_event_send(net_dtu_event, EVENT_NETDEV_CHECK);
}

static void _creat_heart_timer(void) {
  rt_err_t _rt = RT_EOK;

  heart_timer_t = rt_timer_create("heart_timer", _heart_timeout, RT_NULL,
                                  rt_tick_from_millisecond(HEART_TIMER_OUT),
                                  RT_TIMER_FLAG_ONE_SHOT);
  RT_ASSERT(RT_NULL != heart_timer_t);
  _rt = rt_timer_start(heart_timer_t);
  RT_ASSERT(_rt == RT_EOK);
}

static void _delete_heart_timer(void) {
  rt_err_t _rt = RT_EOK;
  _rt = rt_timer_delete(heart_timer_t);
  RT_ASSERT(RT_EOK == _rt);
}

static void _reset_heart_timer(void) {
  _delete_heart_timer();
  _creat_heart_timer();
}

// TODO：与服务器的心跳维护,网络灯
// TODO：网卡切换与网卡状态更新
void net_heart_and_netdev_select(void *parameter) {
  rt_err_t _err = RT_EOK;
  rt_uint32_t _recv = RT_EOK;

  net_dtu_item_t = _new_net_dtu();
  _net_dtu_reset(net_dtu_item_t);
  _net_dtu_netdev_select(net_dtu_item_t);
  _creat_heart_timer();
  
  while (1) {
    _err = rt_event_recv(net_dtu_event, EVENT_NETDEV_ALL,
                         RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR,
                         rt_tick_from_millisecond(5000), &_recv);
    if (!((_err == RT_EOK) || (_err == (-RT_ETIMEOUT)))) {
      LOG_E("%s line:%d code:%d", __FILE__, __LINE__, _err);
      continue;
    }
    if (_recv & EVENT_NETDEV_CHECK) {
      _delete_heart_timer();
      _net_dtu_netdev_select(net_dtu_item_t);
      _creat_heart_timer();
    }
    if (_recv & EVENT_NETDEV_HEART) {
      _reset_heart_timer();
    }
  }
}

int net_dtu_init(void) {
  rt_err_t _err = RT_EOK;
  static rt_thread_t _tid = RT_NULL;

  /*创建一个异常监控事件*/
  net_dtu_event = rt_event_create("net_dtu_event", RT_IPC_FLAG_FIFO);
  RT_ASSERT(RT_NULL != net_dtu_event);

  _tid = rt_thread_create("dtu_stat", net_heart_and_netdev_select, RT_NULL,
                          1024, RT_THREAD_PRIORITY_MAX / 2, 20);
  RT_ASSERT(_tid != RT_NULL);
  rt_thread_startup(_tid);

  return 0;
}

INIT_APP_EXPORT(net_dtu_init);

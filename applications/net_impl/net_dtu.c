#include "net_dtu.h"

#define LOG_TAG "net_dtu"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

struct net_dtu {
  struct netdev *netdev;
  int sock;
};
typedef struct net_dtu *net_dtu_t;

net_dtu_t net_dtu_item_t = RT_NULL;

static void net_dtu_reset(net_dtu_t net_dtu);

static net_dtu_t new_net_dtu(void) {
  net_dtu_t _net_dtu = RT_NULL;
  _net_dtu = rt_calloc(1, sizeof(struct net_dtu));
  RT_ASSERT(RT_NULL != _net_dtu);

  net_dtu_reset(_net_dtu);
  return _net_dtu;
}

static void net_dtu_set_netdev(net_dtu_t net_dtu, struct netdev *netdev) {
  net_dtu->netdev = netdev;
}

static void net_dtu_reset(net_dtu_t net_dtu) {
  net_dtu->netdev = RT_NULL;
  net_dtu->sock = -1;
}

static rt_err_t net_dtu_send(net_dtu_t net_dtu, rt_uint8_t *buf,
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
    net_dtu_reset(net_dtu);
    return -RT_EIO;
  }

  return RT_EOK;
}

static rt_err_t net_dtu_link_remote_server(net_dtu_t net_dtu) {
  struct sockaddr_in server_addr;
  struct hostent *host = RT_NULL;

  if (!SET_NETDEV_IS_OK(net_dtu)) {
    return -NET_DTU_ERR_NO_NETDEV;
  }

  /*等待网卡准备OK*/
  while (netdev_is_link_up(net_dtu->netdev)) {
    rt_thread_mdelay(100);
  }

  host = gethostbyname(SERVER_ADDER_URL);
  if (host == RT_NULL) {
    LOG_W("Get host by name failed!");
    goto _exit;
  }

  /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
  if ((net_dtu->sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    /* 创建socket失败 */
    LOG_E("Create socket error");
    goto _exit;
  }

  /* 初始化预连接的服务端地址 */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_ADDER_PORT);
  server_addr.sin_addr = *((struct in_addr *)host->h_addr);
  rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

  /* 连接到服务端 */
  if (connect(net_dtu->sock, (struct sockaddr *)&server_addr,
              sizeof(struct sockaddr)) == -1) {
    /* 连接失败 */
    LOG_W("Connect fail!");
    net_dtu_reset(net_dtu);
    goto _exit;
  }

  return RT_EOK;

_exit:
  return -RT_ERROR;
}

// TODO：与服务器的心跳维护,网络灯
// TODO：收发数据
// TODO：网卡切换与网卡状态更新
void net_dtu_thread_entry(void *parameter) {
  net_dtu_item_t = new_net_dtu();

  while (1) {
    //等待一个事件

    rt_thread_mdelay(1000);
  }
}

int dtu_thread_init(void) {
  static rt_thread_t _tid = RT_NULL;

  _tid = rt_thread_create("net_dtu", net_dtu_thread_entry, RT_NULL, 1024,
                          RT_THREAD_PRIORITY_MAX / 3, 20);
  RT_ASSERT(_tid != RT_NULL);
  rt_thread_startup(_tid);

  return 0;
}

INIT_APP_EXPORT(dtu_thread_init);

rt_err_t send_to_remote_server(rt_uint8_t *buf, rt_size_t bufsz) {
  return net_dtu_send(net_dtu_item_t, buf, bufsz);
}

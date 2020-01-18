#include <af_inet.h>
#include <netdev.h>
#include <rtdevice.h>
#include <sys/socket.h>
#include "netdb.h"

#define LOG_TAG "net_dtu"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

// TODO：与服务器的心跳维护
// TODO：收发数据
// TODO：网卡切换与网卡状态更新
static int sock = -1;
static int port = 8287;

static rt_err_t _link_remote_server(void) {
  //rt_err_t _rt = RT_EOK;
  struct sockaddr_in server_addr;
  struct hostent *host = RT_NULL;

  host = gethostbyname(SERVER_ADDER_URL);
  if (host == RT_NULL) {
    LOG_W("Get host by name failed!");
    goto _exit;
  }

  /* 创建一个socket，类型是SOCKET_STREAM，TCP类型 */
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    /* 创建socket失败 */
    LOG_E("Create socket error");
    goto _exit;
  }

  /* 初始化预连接的服务端地址 */
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr = *((struct in_addr *)host->h_addr);
  rt_memset(&(server_addr.sin_zero), 0, sizeof(server_addr.sin_zero));

  /* 连接到服务端 */
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) ==
      -1) {
    /* 连接失败 */
    LOG_E("Connect fail!");
    sock = -1;
    goto _exit;
  }

_exit:
  return -RT_ERROR;
}

void net_dtu_thread_entry(void *parameter) {

  rt_thread_mdelay(1000);
  _link_remote_server();
  LOG_D("start dtu");
  while (1) {
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

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

void net_dtu_thread_entry(void *parameter) {
  struct hostent *host = RT_NULL;

 rt_thread_mdelay(100);
  host = gethostbyname(SERVER_ADDER_URL);
  if (host == RT_NULL) {
    LOG_E("Get host by name failed!");
    return;
  }

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

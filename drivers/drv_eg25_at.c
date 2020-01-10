/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-09     xieming      first version
 */
#include <af_inet.h>
#include <at.h>
#include <at_socket.h>
#include <netdev.h>
#include <rtdevice.h>
#include "drv_usart1.h"

#define EG25G_DEVICE_NAME "eg25_net"

#define LOG_TAG "drv_stflash"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

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
int eg25_domain_resolve(const char *name, char ip[16]) { return 0; }
void eg25_set_event_cb(at_socket_evt_t event, at_evt_cb_t cb) { return; }

static const struct at_device_ops at_device = {
    eg25_connect,        eg25_closesocket,  eg25_send,
    eg25_domain_resolve, eg25_set_event_cb,
};

// struct netdev
// {
//     uint16_t mtu;                                      /* maximum transfer
//     unit (in bytes) */ const struct netdev_ops *ops;                      /*
//     network interface device operations */

//     netdev_callback_fn status_callback;                /* network interface
//     device flags change callback */ netdev_callback_fn addr_callback; /*
//     network interface device address information change callback */
// };

// /* set network interface device hardware status operations */
// int (*set_up)(struct netdev *netdev);
// int (*set_down)(struct netdev *netdev);

// /* set network interface device address information operations */
// int (*set_addr_info)(struct netdev *netdev, ip_addr_t *ip_addr, ip_addr_t
// *netmask, ip_addr_t *gw); int (*set_dns_server)(struct netdev *netdev,
// uint8_t dns_num, ip_addr_t *dns_server); int (*set_dhcp)(struct netdev
// *netdev, rt_bool_t is_enabled);

// /* set network interface device common network interface device operations */
// int (*ping)(struct netdev *netdev, const char *host, size_t data_len,
// uint32_t timeout, struct netdev_ping_resp *ping_resp); void (*netstat)(struct
// netdev *netdev);

const struct netdev_ops eg25_netdev_ops;
struct netdev eg25_net_info;

// init at_client
// make at_socket_ops in at_client
// register devnet
// bind sal
int eg25_module_init(void) {
  int _rt = 0;

  rt_memset(&eg25_net_info, 0x00, sizeof(struct netdev));
  // use uasrt1
  // bufsize = 1024 * 4
  _rt = at_client_init(USART1_DEVICE_NAME, 1024 * 4);
  RT_ASSERT(0 == _rt);
  at_client = at_client_get(USART1_DEVICE_NAME);
  RT_ASSERT(RT_NULL != at_client);

  at_socket_device_register(&at_device);

  _rt = sal_at_netdev_set_pf_info(&eg25_net_info);
  RT_ASSERT(0 == _rt);

  _rt = netdev_register(&eg25_net_info, EG25G_DEVICE_NAME, RT_NULL);
  RT_ASSERT(0 == _rt);

  return 0;
}

INIT_ENV_EXPORT(eg25_module_init);

int eg25_cmd(int argc, char **argv) {
  rt_size_t _sz = 0;
  if (argc == 1) {
    goto _exit;
  }
  if (argc == 2) {
    _sz = rt_strnlen(argv[1], AT_CMD_MAX_LEN);
    if (_sz == AT_CMD_MAX_LEN) {
      LOG_E("param is too log");
      goto _exit;
    }
    _sz = at_client_obj_send(at_client, argv[1], _sz);
    if(_sz == 0){
      LOG_E("send failed");
      goto _exit;
    }
  }

  LOG_D("eg25_cmd exe ok");
_exit:
  return 0;
}
MSH_CMD_EXPORT(eg25_cmd, e.g : AT\r\n);

// TODO； #endif

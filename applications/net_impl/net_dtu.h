#ifndef __NET_DTU_H__
#define __NET_DTU_H__

#include <af_inet.h>
#include <netdev.h>
#include <rtdevice.h>
#include <sys/socket.h>
#include "netdb.h"

#define NET_DTU_ERR_NO_LINK 11
#define NET_DTU_ERR_NO_NETDEV 12

#define SOCKET_LINK_IS_OK(net_dtu) (net_dtu->sock != -1)
#define SET_NETDEV_IS_OK(net_dtu) (net_dtu->netdev != RT_NULL)

rt_err_t send_to_remote_server(rt_uint8_t *buf, rt_size_t bufsz);
rt_err_t close_link_remote_server(void);

#endif

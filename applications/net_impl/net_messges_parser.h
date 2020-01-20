#ifndef __NET_MESSGES_PARSER_H__
#define __NET_MESSGES_PARSER_H__

#include "rtthread.h"

#define GW_LOGIN_SERVER_REQUEST 0x00  // 网关登陆服务器请求
#define GW_NET_ID_REQUEST 0x01        // 网关向服务器请求分配网络ID
#define GW_HART_PACKAGE_REQUEST 0x02  // 网关向服务器请求心跳包

rt_err_t messges_encoding(rt_uint8_t *messges, rt_size_t len, rt_uint8_t *type,
                          rt_uint8_t *buf, rt_uint16_t *bufsz);
rt_err_t messges_coding(rt_uint8_t *messges, rt_size_t len, rt_uint8_t type,
                        rt_uint8_t *buf, rt_uint16_t *bufsz);

#endif

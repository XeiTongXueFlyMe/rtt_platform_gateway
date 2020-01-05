#ifndef __DRV_GPIO_SW_H__
#define __DRV_GPIO_SW_H__

#include <rthw.h>
#include <rtthread.h>

#define SW_DEVICE_NAME "switch"

#define SW_STATUS_CLOSE 0
#define SW_STATUS_OPEN 1
#define SW_STATUS_NO_KNOW 2

#define CMD_READ_K0_STATUS 0
#define CMD_READ_K1_STATUS 1
#define CMD_READ_K2_STATUS 2
#define CMD_READ_K3_STATUS 3

enum sw_type {
  CFG_KEY_0 = 0,  //轻触开关
  CFG_KEY_1,      //拔码开关1
  CFG_KEY_2,      //拔码开关2
  CFG_KEY_3,      //自锁型推推开关
};

#endif

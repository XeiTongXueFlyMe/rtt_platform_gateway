// TODO:直接重启，定时重启，每天重启
#include <stm32f2xx.h>
#include "rtdevice.h"
#include "rtthread.h"
#include "string.h"

#if defined(RT_USING_CONSOLE)

#define WDT_DEVICE_NAME "xwdg"

#define LOG_TAG "app.cmd"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

int reboot(void) {
  rt_err_t _rt = RT_EOK;
  rt_device_t _d = RT_NULL;

  _d = rt_device_find(WDT_DEVICE_NAME);
  RT_ASSERT(RT_NULL != _d);
  _rt = rt_device_control(_d, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
  if (RT_EOK != _rt) {
    LOG_E("%s line:%d", __FILE__, __LINE__);
  }
  rt_kprintf("\n");

  __set_FAULTMASK(1);
  NVIC_SystemReset();
  for (;;)
    ;
}

MSH_CMD_EXPORT(reboot, reset sys);

int clean(void) {
  rt_kprintf("%s", "\033[1H\033[2J");
  return 0;
}

MSH_CMD_EXPORT(clean, Clear Screen CLS);

static void _rf_help(void) {
  rt_kprintf("\r\nrf debug cmd:");
  rt_kprintf("\r\n rf stat => printf rf status");
  rt_kprintf("\r\n rf config channel 6");
}
static void _rf_stat(void) {
  // TODO
}
static rt_err_t _rf_config(char *type, char *param) {
  // TODO

  return RT_EOK;
}
// rf module debug cmd
int rf(int argc, char **argv) {
  rt_err_t _rt = RT_EOK;
  // TODO
  switch (argc) {
    case 1:  //打印rf状态参数
      _rf_help();
      break;
    case 2:
      if (!strcmp(argv[1], "help")) {
        _rf_help();
      } else if (!strcmp(argv[1], "stat")) {
        _rf_stat();
      } else {
        goto _exit;
      }
    case 4:
      if (!strcmp(argv[1], "config")) {
        _rt = _rf_config(argv[2], argv[3]);
        if (RT_EOK != _rt) {
          goto _exit;
        }
      } else {
        goto _exit;
      }
    default:
      goto _exit;
  }
  return 0;

_exit:
  rt_kprintf("Invalid parameter");
  return 0;
}

MSH_CMD_EXPORT(rf, e.g : rf help)

#endif

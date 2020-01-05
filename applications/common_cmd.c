// TODO:直接重启，定时重启，每天重启
#include <stm32f2xx.h>
#include "rtdevice.h"
#include "rtthread.h"

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

#endif

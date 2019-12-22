#include <rtdevice.h>
#include <rtthread.h>

#ifdef USE_DRV_XWATCHDOG
#define WDT_DEVICE_NAME "xwdg"

#define LOG_TAG "app.xwdg"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

void xwdg_thread_entry(void *parameter) {
  rt_err_t _rt = RT_EOK;
  ~rt_device_t _d = RT_NULL;

  _d = rt_device_find(WDT_DEVICE_NAME);
  RT_ASSERT(RT_NULL != _d);
  _rt = rt_device_init(_d);
  RT_ASSERT(RT_EOK != _rt);

  while (1) {
    _rt = rt_device_control(_d, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
    if (RT_EOK != _rt) {
      LOG_E(__FILE__, __LINE__);
    }
    rt_thread_mdelay(10);
  }
}

void wdg_thread_init(void) {
  LOG_I("xwdg enabled");

  rt_thread_t _tid = RT_NULL;
  _tid = rt_thread_create("app_wdg", xwdg_thread_entry, RT_NULL, 256,
                          RT_THREAD_PRIORITY_MAX - 1, 20);
  RT_ASSERT(_tid != RT_NULL);
  rt_thread_startup(_tid);
}

#endif

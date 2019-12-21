#include <rtthread.h>
#include <rtdevice.h>

#ifdef USE_DRV_XWATCHDOG
#define WDT_DEVICE_NAME    "xwdg"

static rt_device_t wdg_dev;         /* 看门狗设备句柄 */

static void idle_hook(void)
{
    /* 在空闲线程的回调函数里喂狗 */
    rt_device_control(wdg_dev, RT_DEVICE_CTRL_WDT_KEEPALIVE, NULL);
}

static int xwdg(void)
{
    rt_err_t ret = RT_EOK;

    /* 根据设备名称查找看门狗设备，获取设备句柄 */
    wdg_dev = rt_device_find(WDT_DEVICE_NAME);
    if (!wdg_dev)
    {
        rt_kprintf("find %s failed!\n", WDT_DEVICE_NAME);
        return RT_ERROR;
    }
    /* 初始化设备 */
    ret = rt_device_init(wdg_dev);
    if (ret != RT_EOK)
    {
        rt_kprintf("initialize %s failed!\n", WDT_DEVICE_NAME);
        return RT_ERROR;
    }
    /* 设置空闲线程回调函数 */
    rt_thread_idle_sethook(idle_hook);

    return ret;
}

MSH_CMD_EXPORT(xwdg, wdt sample);

#endif

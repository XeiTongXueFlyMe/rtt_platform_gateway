#include <rtdevice.h>
#include <stm32f2xx.h>
#define LOG_TAG "drv_iwdg"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

#ifdef USE_DRV_IWATCHDOG

#define WDT_DEVICE_NAME "iwdg"

//超时时间2s左右
void hw_board_iwatchdog_init(void) {
  /* internal wdg */
  RCC_ClearFlag();
  RCC_LSICmd(ENABLE);

  while (RESET == RCC_GetFlagStatus(RCC_FLAG_LSIRDY))
    ;

  /* IWDG timeout equal to 214 ms (the timeout may varies due to LSI frequency
   * dispersion) */
  /* Enable write access to IWDG_PR and IWDG_RLR registers */
  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);

  /* IWDG configuration: IWDG is clocked by LSI = 40KHz */
  IWDG_SetPrescaler(IWDG_Prescaler_16);

  IWDG_SetReload((uint16_t)0xfff);

  /* Reload IWDG counter */
  IWDG_ReloadCounter();

  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
  IWDG_Enable();
}

static rt_err_t _iwdg_init(rt_watchdog_t *wdt) {
  LOG_D("iwdg init");
  hw_board_iwatchdog_init();
  return RT_EOK;
}

static rt_err_t _iwdg_conctrol(rt_watchdog_t *wdt, int cmd, void *arg) {
  if (RT_DEVICE_CTRL_WDT_KEEPALIVE == cmd) {
    LOG_D("feed dogs");
    IWDG_ReloadCounter();
  }
  return RT_EOK;
}

static rt_watchdog_t iwatchdog;

static struct rt_watchdog_ops wdg_ops = {
    _iwdg_init,
    _iwdg_conctrol,
};

int iwdg_init(void) {
  rt_err_t _rt = RT_EOK;

  hw_board_iwatchdog_init();
  
  iwatchdog.ops = &wdg_ops;
  _rt = rt_hw_watchdog_register(&iwatchdog, "iwdg", RT_DEVICE_FLAG_STANDALONE,
                                RT_NULL);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_BOARD_EXPORT(iwdg_init);

#endif

#include <rtdevice.h>
#include <stm32f2xx.h>
#define LOG_TAG "drv_xwdg"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

//TODO；test xwdg 
#ifdef USE_DRV_XWATCHDOG

#define WDT_DEVICE_NAME    "xwdg"
#define _HW_BOARD_XWDG_GPIO_RCC RCC_AHB1Periph_GPIOC
#define _HW_BOARD_XWDG_GPIO GPIOC
#define _HW_BOARD_XWDG_PIN GPIO_Pin_4

static void _gpio_config(void);

static rt_err_t _xwdg_init(rt_watchdog_t *wdt) {
  LOG_D("xwdg init");
  _gpio_config();

  return RT_EOK;
}
//
static rt_err_t _xwdg_conctrol(rt_watchdog_t *wdt, int cmd, void *arg) {
  
  if(RT_DEVICE_CTRL_WDT_KEEPALIVE == cmd){
    LOG_D("feed dogs");
    _HW_BOARD_XWDG_GPIO->ODR ^= _HW_BOARD_XWDG_PIN;
  }
	return RT_EOK;
}

static void _gpio_config(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(_HW_BOARD_XWDG_GPIO_RCC, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = _HW_BOARD_XWDG_PIN;
  GPIO_Init(_HW_BOARD_XWDG_GPIO, &GPIO_InitStructure);

  GPIO_SetBits(_HW_BOARD_XWDG_GPIO, _HW_BOARD_XWDG_PIN);
  GPIO_ResetBits(_HW_BOARD_XWDG_GPIO, _HW_BOARD_XWDG_PIN);
}

static rt_watchdog_t xwatchdog;

static struct rt_watchdog_ops wdg_ops = {
  _xwdg_init,
  _xwdg_conctrol,
};

int xwdg_init(void) {
  rt_err_t _rt = RT_EOK;

  xwatchdog.ops = &wdg_ops;
  _rt = rt_hw_watchdog_register(&xwatchdog, "xwdg", RT_DEVICE_FLAG_STANDALONE,
                                RT_NULL);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_BOARD_EXPORT(xwdg_init);

#endif

/*
 * Change Logs:
 * Date           Author       Notes
 * 2020-01-09     xieming      first version
 */
#include "drv_eg25_at.h"

#ifdef USE_DRV_EG25E

#define LOG_TAG "drv_eg25"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#define RESET_PORT GPIOC
#define RESET_PIN GPIO_Pin_9
#define RESET_RCC RCC_AHB1Periph_GPIOC

#define POWERKEY_PORT GPIOA
#define POWERKEY_PIN GPIO_Pin_8
#define PORT_RCC RCC_AHB1Periph_GPIOA

static void _powerkey_on(void) { GPIO_SetBits(POWERKEY_PORT, POWERKEY_PIN); }
static void _powerkey_off(void) { GPIO_ResetBits(POWERKEY_PORT, POWERKEY_PIN); }
static void _reset_on(void) { GPIO_SetBits(RESET_PORT, RESET_PIN); }
static void _reset_off(void) { GPIO_ResetBits(RESET_PORT, RESET_PIN); }

void hw_eg25_init(void) {
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_AHB1PeriphClockCmd(RESET_RCC, ENABLE);
  RCC_AHB1PeriphClockCmd(PORT_RCC, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_InitStructure.GPIO_Pin = RESET_PIN;
  GPIO_Init(RESET_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = POWERKEY_PIN;
  GPIO_Init(POWERKEY_PORT, &GPIO_InitStructure);

  _powerkey_on();
  _reset_off();
}

struct quectel_pin_ops pin_ops = {
    _powerkey_on,
    _powerkey_off,
    _reset_on,
    _reset_off,
};

int eg25_module_init(void) {
  rt_err_t _rt = RT_EOK;
  hw_eg25_init();
  _rt = rt_hw_eg25g_register(EG25G_DEVICE_NAME, USART1_DEVICE_NAME, &pin_ops);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_ENV_EXPORT(eg25_module_init);

#endif /*USE_DRV_EG25E*/

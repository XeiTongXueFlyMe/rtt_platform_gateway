#include <rtdevice.h>
#include <stm32f2xx.h>

#include "../applications/config.h"
#include "./drv_gpio_sw.h"

#ifdef USE_DRV_GPIO_SW

#define LOG_TAG "drv_spi"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

struct sw_factory {
  enum sw_type type;
  GPIO_TypeDef *port;
  uint16_t pin;
};
typedef struct sw_factory sw_factroy_t;

static sw_factroy_t _sw_list[] = {
    {CFG_KEY_0, GPIOD, GPIO_Pin_3},
    {CFG_KEY_1, GPIOC, GPIO_Pin_7},
    {CFG_KEY_2, GPIOD, GPIO_Pin_15},
    {CFG_KEY_3, GPIOD, GPIO_Pin_5},
};

static sw_factroy_t *_get_sw_param(enum sw_type type) {
  for (int _i = 0; _i < (sizeof(_sw_list) / sizeof(sw_factroy_t)); _i++) {
    if (_sw_list[_i].type == type) {
      return _sw_list + _i;
    }
  }
  LOG_E("file:%s,line:%d err:no find sw_type = %d", __FILE__, __LINE__, type);
  return RT_NULL;
}

static void _hw_rcc_config(void);
static void _hw_gpio_cofig(void);

struct rt_device sw_device;

static rt_uint8_t _read_sw_status(enum sw_type type) {
  sw_factroy_t *_sw_t = RT_NULL;
  _sw_t = _get_sw_param(type);
  if (RT_NULL == _sw_t) {
    goto _exit;
  }

  return ((GPIO_ReadInputDataBit(_sw_t->port, _sw_t->pin) == Bit_SET)
              ? SW_STATUS_OPEN
              : SW_STATUS_CLOSE);

_exit:
  return SW_STATUS_NO_KNOW;
}

static rt_err_t _control(rt_device_t dev, int cmd, void *args) {
  rt_err_t _rt = RT_EOK;

  switch (cmd) {
    case CMD_READ_K0_STATUS:
      *(rt_uint8_t *)args = _read_sw_status(CFG_KEY_0);
      break;
    case CMD_READ_K1_STATUS:
      *(rt_uint8_t *)args = _read_sw_status(CFG_KEY_1);
      break;
    case CMD_READ_K2_STATUS:
      *(rt_uint8_t *)args = _read_sw_status(CFG_KEY_2);
      break;
    case CMD_READ_K3_STATUS:
      *(rt_uint8_t *)args = _read_sw_status(CFG_KEY_3);
      break;
    default:
      _rt = RT_EINVAL;
      break;
  }

  return _rt;
}

int rt_hw_sw_init(void) {
  rt_err_t _rt = RT_EOK;
  _hw_rcc_config();
  _hw_gpio_cofig();

  sw_device.type = RT_Device_Class_Miscellaneous;
  sw_device.rx_indicate = RT_NULL;
  sw_device.tx_complete = RT_NULL;
  sw_device.init = RT_NULL;
  sw_device.open = RT_NULL;
  sw_device.close = RT_NULL;
  sw_device.read = RT_NULL;
  sw_device.write = RT_NULL;
  sw_device.control = _control;
  sw_device.user_data = RT_NULL;

  _rt = rt_device_register(&sw_device, SW_DEVICE_NAME,
                           RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_BOARD_EXPORT(rt_hw_sw_init);

static void _hw_rcc_config(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
}

static void _hw_gpio_cofig(void) {
  for (int _i = 0; _i < (sizeof(_sw_list) / sizeof(sw_factroy_t)); _i++) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Pin = _sw_list[_i].pin;
    GPIO_Init(_sw_list[_i].port, &GPIO_InitStructure);
  }
}

#ifdef UNIT_TEST
int sw_utest(void) {
  rt_err_t _rt = RT_EOK;
  rt_device_t _dev_t = RT_NULL;
  rt_uint8_t _status = RT_NULL;

  _dev_t = rt_device_find(SW_DEVICE_NAME);
  RT_ASSERT(RT_NULL != _dev_t);

  _rt = rt_device_control(_dev_t, CMD_READ_K0_STATUS, &_status);
  RT_ASSERT(RT_EOK == _rt);
  rt_kprintf("CFG_KEY_0 = %d", _status);
  _rt = rt_device_control(_dev_t, CMD_READ_K1_STATUS, &_status);
  RT_ASSERT(RT_EOK == _rt);
  rt_kprintf("\nCFG_KEY_1 = %d", _status);
  _rt = rt_device_control(_dev_t, CMD_READ_K2_STATUS, &_status);
  RT_ASSERT(RT_EOK == _rt);
  rt_kprintf("\nCFG_KEY_2 = %d", _status);
  _rt = rt_device_control(_dev_t, CMD_READ_K3_STATUS, &_status);
  RT_ASSERT(RT_EOK == _rt);
  rt_kprintf("\nCFG_KEY_3 = %d\n", _status);

  return 0;
}

MSH_CMD_EXPORT(sw_utest, switch_button test);

#endif

#endif

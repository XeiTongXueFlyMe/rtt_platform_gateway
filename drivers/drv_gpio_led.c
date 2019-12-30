#include <rtdevice.h>
#include <stm32f2xx.h>

#include "../applications/config.h"
#include "./drv_gpio_led.h"

#ifdef USE_DRV_GPIO_LED

struct led_factory {
  led_type_t type;
  GPIO_TypeDef *port;
  uint16_t pin;
};
typedef struct led_factory led_factroy_t;

static led_factroy_t _led_list[] = {
    {LED_STATUS, GPIOD, GPIO_Pin_8},
    {LED_GPRS, GPIOD, GPIO_Pin_9},
    {LED_RF, GPIOC, GPIO_Pin_6},
    {LED_SYS, GPIOC, GPIO_Pin_8},
};
static led_factroy_t *_get_led_param(led_type_t type) {
  for (int _i = 0; _i < (sizeof(_led_list) / sizeof(led_factroy_t)); _i++) {
    if (_led_list[_i].type == type) {
      return _led_list + _i;
    }
  }
  return RT_NULL;
}

static void _hw_rcc_config(void);
static void _hw_gpio_cofig(void);
static inline void _hw_led_off(led_type_t type);
static inline void _hw_led_on(led_type_t type);
static inline void _hw_led_reverse(led_type_t type);

struct rt_device led_device;

rt_err_t _control(rt_device_t dev, int cmd, void *args) {
  rt_err_t _rt = RT_EOK;
  led_type_t _type = *(led_type_t *)(args);

  switch (cmd) {
    case LED_OPEN:
      _hw_led_on(_type);
      break;
    case LED_CLOSE:
      _hw_led_off(_type);
      break;
    case LED_REVERSE:
      _hw_led_reverse(_type);
      break;
    default:
      _rt = RT_EINVAL;
      break;
  }

  return _rt;
}

int rt_hw_led_init(void) {
  rt_err_t _rt = RT_EOK;
  _hw_rcc_config();
  _hw_gpio_cofig();

  led_device.type = RT_Device_Class_Miscellaneous;
  led_device.rx_indicate = RT_NULL;
  led_device.tx_complete = RT_NULL;
  led_device.init = RT_NULL;
  led_device.open = RT_NULL;
  led_device.close = RT_NULL;
  led_device.read = RT_NULL;
  led_device.write = RT_NULL;
  led_device.control = _control;
  led_device.user_data = RT_NULL;

  _rt = rt_device_register(&led_device, "led",
                     RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_BOARD_EXPORT(rt_hw_led_init);

static void _hw_rcc_config(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
}

static void _hw_gpio_cofig(void) {
  for (int _i = 0; _i < (sizeof(_led_list) / sizeof(led_factroy_t)); _i++) {
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = _led_list[_i].pin;
    GPIO_Init(_led_list[_i].port, &GPIO_InitStructure);
  }
}

static inline void _hw_led_off(led_type_t type) {
  led_factroy_t *led = _get_led_param(type);
  GPIO_ResetBits(led->port, led->pin);
}

static inline void _hw_led_on(led_type_t type) {
  led_factroy_t *led = _get_led_param(type);
  GPIO_SetBits(led->port, led->pin);
}

static inline void _hw_led_reverse(led_type_t type) {
  led_factroy_t *led = _get_led_param(type);
  led->port->ODR ^= led->pin;
}

#endif

#include "../drivers/drv_gpio_led.h"
#include "rtdevice.h"
#include "rtthread.h"

#if defined(UNIT_TEST) && defined(USE_DRV_GPIO_LED)
void led_blink(void) {
  rt_device_t _led = RT_NULL;
  int args = LED_SYS;
  int _count = 0;

  _led = rt_device_find("led");
  RT_ASSERT(_led != RT_NULL);
  rt_device_open(_led, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

  while (_count < 10) {
    args = LED_SYS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_RF;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_GPRS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_STATUS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    _count++;
    rt_thread_mdelay(500);
  }
}

MSH_CMD_EXPORT(led_blink, all led blink 5 count);
#else
void led_thread_entry(void *parameter) {
  rt_device_t _led = RT_NULL;
  int args = LED_SYS;
  int _count = 0;

  _led = rt_device_find("led");
  RT_ASSERT(_led != RT_NULL);
  rt_device_open(_led, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STANDALONE);

  while (1) {
    args = LED_SYS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_RF;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_GPRS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    args = LED_STATUS;
    _led->control(_led, LED_REVERSE, (void *)(&args));

    _count++;
    rt_thread_mdelay(500);
  }
}

void led_thread_init(void) {
  rt_thread_t _tid = RT_NULL;
  _tid = rt_thread_create("led_handler", led_thread_entry, RT_NULL, 256,
                         RT_THREAD_PRIORITY_MAX / 2, 20);
  RT_ASSERT(_tid != RT_NULL);
  rt_thread_startup(_tid);
}
#endif

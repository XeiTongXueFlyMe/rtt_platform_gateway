#ifndef __GPIO_LED_H__
#define __GPIO_LED_H__

#include <rthw.h>
#include <rtthread.h>

#define LED_OPEN 0
#define LED_CLOSE 1
#define LED_REVERSE 2

enum led_type {
  LED_STATUS = 0,
  LED_RF,
  LED_GPRS,
  LED_SYS,
};

typedef enum led_type led_type_t;

void rt_hw_led_init(void);

#endif

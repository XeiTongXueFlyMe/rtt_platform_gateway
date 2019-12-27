#include "led.h"
#include "../drivers/drv_gpio_led.h"
#include "rtdevice.h"
#include "rtthread.h"

#define LOG_TAG "app.led"
#define LOG_LVL LOG_LVL_INFO
#include <ulog.h>

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

struct led_factory {
  rt_device_t device_t;
  rt_uint32_t ctl;
  rt_uint32_t blink_status;

  rt_err_t (*open_close)(struct led_factory *led);
  rt_err_t (*blink)(struct led_factory *led);
};

static rt_err_t _open_close(struct led_factory *led);
static rt_err_t _blink(struct led_factory *led);

void led_thread_entry(void *parameter) {
  struct rt_event _event;
  rt_err_t _err = RT_EOK;
  rt_device_t _d = RT_NULL;
  rt_uint32_t _recv = RT_EOK;

  _err = rt_event_init(&_event, "led_thread_event", RT_IPC_FLAG_FIFO);
  RT_ASSERT(RT_EOK == _err);

  _d = rt_device_find("led");
  RT_ASSERT(RT_NULL != _d);
  _err = rt_device_open(_d, RT_DEVICE_FLAG_STANDALONE);
  RT_ASSERT(RT_EOK == _err);

  struct led_factory _led_item = {
      .device_t = _d,
      .ctl = RT_NULL,
      .blink_status = EVENT_SYS_BLINK,  //默认系统指示灯闪烁
      .open_close = _open_close,
      .blink = _blink,
  };

  while (1) {
    _err = rt_event_recv(
        &_event,
        (EVENT_NET_STATUS_OPEN | EVENT_GPRS_OPEN | EVENT_RF_OPEN |
         EVENT_SYS_OPEN | EVENT_NET_STATUS_CLOSE | EVENT_GPRS_CLOSE |
         EVENT_RF_CLOSE | EVENT_SYS_CLOSE | EVENT_NET_STATUS_BLINK |
         EVENT_GPRS_BLINK | EVENT_RF_BLINK | EVENT_SYS_BLINK),
        RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, rt_tick_from_millisecond(500),
        &_recv);

    if (!((_err == RT_EOK) || (_err == (-RT_ETIMEOUT)))) {
      LOG_E("%s line:%d code:%d", __FILE__, __LINE__, _err);
      continue;
    }

    _led_item.ctl = _recv;
    _err = _led_item.open_close(&_led_item);
    if (RT_EOK != _err) {
      LOG_E("%s line:%d", __FILE__, __LINE__);
    }
    _err = _led_item.blink(&_led_item);
    if (RT_EOK != _err) {
      LOG_E("%s line:%d", __FILE__, __LINE__);
    }
  }
}

static rt_err_t _blink(struct led_factory *led_t) {
  LOG_D("call led.blink(.blink_status = %u .ctl = %u) ", led_t->blink_status,
        led_t->ctl);
  rt_err_t _rt = RT_EOK;
  enum led_type _led_type;

  if (led_t->ctl & EVENT_SYS_BLINK) {
    led_t->blink_status = led_t->blink_status | EVENT_SYS_BLINK;
  }
  if (led_t->ctl & EVENT_RF_BLINK) {
    led_t->blink_status = led_t->blink_status | EVENT_RF_BLINK;
  }
  if (led_t->ctl & EVENT_GPRS_BLINK) {
    led_t->blink_status = led_t->blink_status | EVENT_GPRS_BLINK;
  }
  if (led_t->ctl & EVENT_NET_STATUS_BLINK) {
    led_t->blink_status = led_t->blink_status | EVENT_NET_STATUS_BLINK;
  }

  if (led_t->blink_status & EVENT_SYS_BLINK) {
    _led_type = LED_SYS;
    _rt = rt_device_control(led_t->device_t, LED_REVERSE, (void *)&_led_type);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }
  if (led_t->blink_status & EVENT_RF_BLINK) {
    _led_type = LED_RF;
    _rt = rt_device_control(led_t->device_t, LED_REVERSE, (void *)&_led_type);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }
  if (led_t->blink_status & EVENT_GPRS_BLINK) {
    _led_type = LED_GPRS;
    _rt = rt_device_control(led_t->device_t, LED_REVERSE, (void *)&_led_type);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }
  if (led_t->blink_status & EVENT_NET_STATUS_BLINK) {
    _led_type = LED_STATUS;
    _rt = rt_device_control(led_t->device_t, LED_REVERSE, (void *)&_led_type);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

RT_ERR:
  return _rt;
}

static rt_err_t _open_close(struct led_factory *led_t) {
  LOG_D("call led.open_close(.ctl = %u) ", led_t->ctl);
  rt_err_t _rt = RT_EOK;

  if (led_t->ctl & EVENT_SYS_OPEN) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_SYS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_OPEN, (void *)LED_SYS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_SYS_CLOSE) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_SYS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_CLOSE, (void *)LED_SYS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_NET_STATUS_OPEN) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_NET_STATUS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_OPEN, (void *)LED_STATUS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_NET_STATUS_CLOSE) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_NET_STATUS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_CLOSE, (void *)LED_STATUS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_RF_OPEN) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_RF_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_OPEN, (void *)LED_RF);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_RF_CLOSE) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_RF_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_CLOSE, (void *)LED_RF);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_GPRS_OPEN) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_GPRS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_OPEN, (void *)LED_GPRS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

  if (led_t->ctl & EVENT_GPRS_CLOSE) {
    led_t->blink_status = (led_t->blink_status & (~EVENT_GPRS_BLINK));
    _rt = rt_device_control(led_t->device_t, LED_CLOSE, (void *)LED_GPRS);
    if (RT_EOK != _rt) {
      goto RT_ERR;
    }
  }

RT_ERR:
  return _rt;
}

static rt_thread_t _led_tid = RT_NULL;
int led_thread_init(void) {
  _led_tid = rt_thread_create("app_led", led_thread_entry, RT_NULL, 2048,
                              RT_THREAD_PRIORITY_MAX - 1, 20);
  RT_ASSERT(_led_tid != RT_NULL);
  rt_thread_startup(_led_tid);

  return 0;
}

INIT_APP_EXPORT(led_thread_init);

#endif

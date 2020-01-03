#include "drv_w25qxx.h"
#include <rtdevice.h>
#include "../device_drivers/spi/spi_flash_w25qxx.h"
#include "dvr_spi3.h"

#define CS_GPIO GPIOD
#define CS_PIN GPIO_Pin_14

#define LOG_TAG "drv_spi"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#ifdef USE_W25QXX

int w25qxxx_device_init(void) {
  rt_err_t _rt = RT_EOK;
  _rt =
      rt_hw_spi_device_attach(SPI3_BUS_NAME, SPI_DEVICE_NAME, CS_GPIO, CS_PIN);
  if (RT_EOK != _rt) {
    rt_kprintf("\r\nfile:%s line:%d rt_hw_spi_device_attach() return %d",
               __FILE__, __LINE__, _rt);
  }

  _rt = w25qxx_init(DEVICE_NAME, SPI_DEVICE_NAME);
  if (RT_EOK != _rt) {
    rt_kprintf("\r\nfile:%s line:%d w25qxx_init() return %d", __FILE__,
               __LINE__, _rt);
  }
  return 0;
}

INIT_DEVICE_EXPORT(w25qxxx_device_init);

#ifdef UNIT_TEST
static rt_uint8_t _wbuf[4 * 1024];
static rt_uint8_t _rbuf[4 * 1024];

int w25q_utest(void) {
  rt_err_t _rt = RT_EOK;
  rt_device_t _dev = RT_NULL;

  for (rt_uint16_t _i = 0; _i < sizeof(_wbuf); _i++) {
    _wbuf[_i] = _i;
  }

  _dev = rt_device_find(DEVICE_NAME);
  RT_ASSERT(RT_NULL != _dev);

  _rt = rt_device_open(_dev, RT_DEVICE_FLAG_RDWR);
  RT_ASSERT(RT_EOK == _rt);

  // 1024 =  (4 * 1024 * 1024) / (4 * 1024)
  for (rt_uint16_t _cnt = 0; _cnt < 1024; _cnt++) {
    if (rt_device_write(_dev, _cnt, _wbuf, 1) != 1) {
      LOG_W("w25q write fail offset = %d _wbuf = %x size = %d", _cnt, _wbuf, 1);
      goto _exit;
    }

    if (rt_device_read(_dev, _cnt, _rbuf, 1) != 1) {
      LOG_W("w25q read fail offset = %d _rbuf = %x size = %d", _cnt, _rbuf, 1);
      goto _exit;
    }

    for (rt_uint16_t _i = 0; _i < sizeof(_rbuf); _i++) {
      if (_wbuf[_i] != _rbuf[_i]) {
        LOG_E("w25q write != w25q read");
        goto _exit;
      }
    }
    rt_kprintf("\rw25qxx check sector: %d/1024", _cnt);
  }
  rt_kprintf("\r                                                           ");

_exit:
  _rt = rt_device_close(_dev);
  RT_ASSERT(RT_EOK == _rt);
  return 0;
}

MSH_CMD_EXPORT(w25q_utest, w25qxx unit test);
#endif

#endif

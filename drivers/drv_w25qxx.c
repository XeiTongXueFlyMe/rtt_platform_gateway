#include "drv_w25qxx.h"
#include <rtdevice.h>
#include "../device_drivers/spi/spi_flash_w25qxx.h"
#include "dvr_spi3.h"

#define CS_GPIO GPIOD
#define CS_PIN GPIO_Pin_14

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

#endif

#endif

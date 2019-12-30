#include "dvr_spi.h"
#include "rtdevice.h"

#define LOG_TAG "drv_st_flash"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

#ifdef USE_DRV_SPI

struct stm32_hw_spi_cs {
  GPIO_TypeDef *GPIOx;
  uint16_t GPIO_Pin;
};

static void _gpio_config(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_PinAFConfig(GPIOB, 3, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOB, 4, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOB, 5, GPIO_AF_SPI3);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void _spi_config(void) {
  SPI_InitTypeDef SPI_InitStructure;
  /* SPI Config */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;

  SPI_Init(SPI3, &SPI_InitStructure);

  /* Enable SPI */
  SPI_Cmd(SPI3, ENABLE);
}

rt_uint32_t stm_spi_xfer(struct rt_spi_device *device,
                         struct rt_spi_message *message) {
  return 0;
}
rt_err_t stm_spi_configure(struct rt_spi_device *device,
                           struct rt_spi_configuration *configuration) {
  return RT_EOK;
}

static const struct rt_spi_ops stm_spi_ops = {
    .configure = stm_spi_configure,
    .xfer = stm_spi_xfer,
};

struct rt_spi_bus stm_spi3_bus;

int spi_bus_init(void) {
  rt_err_t _rt = RT_EOK;

  _gpio_config();
  _spi_config();

  _rt = rt_spi_bus_register(&stm_spi3_bus, SPI_BUS_NAME, &stm_spi_ops);
  RT_ASSERT(RT_EOK == _rt);

  return 0;
}

INIT_BOARD_EXPORT(spi_bus_init);

// Attach the spi device to SPI bus, this function must be used after
// initialization
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name,
                                 GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin) {
  RT_ASSERT(bus_name != RT_NULL);
  RT_ASSERT(device_name != RT_NULL);

  rt_err_t _rt = RT_EOK;
  struct rt_spi_device *_spi_device_t;
  struct stm32_hw_spi_cs *_cs_pint_t;

  /* initialize the cs pin && select the slave*/
  GPIO_InitTypeDef GPIO_InitStructure;

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = cs_gpio_pin;
  GPIO_Init(cs_gpiox, &GPIO_InitStructure);
  GPIO_SetBits(cs_gpiox, cs_gpio_pin);

  /* attach the device to spi bus*/
  _spi_device_t =
      (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
  RT_ASSERT(_spi_device_t != RT_NULL);
  _cs_pint_t =
      (struct stm32_hw_spi_cs *)rt_malloc(sizeof(struct stm32_hw_spi_cs));
  RT_ASSERT(_cs_pint_t != RT_NULL);
  _cs_pint_t->GPIOx = cs_gpiox;
  _cs_pint_t->GPIO_Pin = cs_gpio_pin;
  _rt = rt_spi_bus_attach_device(_spi_device_t, device_name, bus_name,
                                 (void *)_cs_pint_t);
  RT_ASSERT(RT_EOK == _rt);

  LOG_D("%s attach to %s done", device_name, bus_name);

  return _rt;
}

#endif

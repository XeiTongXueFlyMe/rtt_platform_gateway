#include "dvr_spi3.h"
#include "rtdevice.h"

#define LOG_TAG "drv_spi"
#define LOG_LVL LOG_LVL_INFO
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

static rt_err_t _hw_spi_bus_send(SPI_TypeDef *spi, const rt_uint8_t *buffer,
                                 rt_uint16_t length) {
  rt_err_t _rt = RT_EOK;
  rt_uint16_t _c = 0;  // cycles

  while (length--) {
    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    SPI_I2S_SendData(spi, *buffer);

    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    SPI_I2S_ReceiveData(spi);

    buffer++;
  }
  return _rt;

_exit:
  LOG_W("file:%s line:%d hw_spi_bus_send() return time out", __FILE__,
        __LINE__);
  return RT_ETIMEOUT;
}

static rt_err_t _hw_spi_bus_recv(SPI_TypeDef *spi, rt_uint8_t *buffer,
                                 rt_uint16_t size) {
  rt_err_t _rt = RT_EOK;
  rt_uint16_t _c = 0;  // cycles

  while (size--) {
    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    SPI_I2S_SendData(spi, 0xFF);

    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    (*buffer) = SPI_I2S_ReceiveData(spi);

    buffer++;
  }

  return _rt;

_exit:
  LOG_W("file:%s line:%d hw_spi_bus_recv() return time out", __FILE__,
        __LINE__);
  return RT_ETIMEOUT;
}
static rt_err_t _hw_spi_bus_send_recv(SPI_TypeDef *spi,
                                      const rt_uint8_t *send_buf,
                                      rt_uint8_t *recv_buf,
                                      rt_uint16_t length) {
  rt_err_t _rt = RT_EOK;
  rt_uint16_t _c = 0;  // cycles

  while (length--) {
    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_TXE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    SPI_I2S_SendData(spi, *send_buf);

    _c = 0;
    while (SPI_I2S_GetFlagStatus(spi, SPI_I2S_FLAG_RXNE) == RESET) {
      if (++_c > 0xfffe) goto _exit;
    }
    *recv_buf = SPI_I2S_ReceiveData(spi);

    send_buf++;
    recv_buf++;
  }

  return _rt;
_exit:
  LOG_W("file:%s line:%d hw_spi_bus_send() return time out", __FILE__,
        __LINE__);
  return RT_ETIMEOUT;
}
//由于stm32f2xx与stmf1xx库的原因，这里只对spi做简单的实现
rt_uint32_t stm_spi_xfer(struct rt_spi_device *device,
                         struct rt_spi_message *message) {
  RT_ASSERT(device != RT_NULL);
  RT_ASSERT(device->bus != RT_NULL);
  RT_ASSERT(device->parent.user_data != RT_NULL);
  RT_ASSERT(message != RT_NULL);

  LOG_D("SPI3 xfer: %X, recvbuf: %X, length: %d",
        (uint32_t)message->send_buf, (uint32_t)message->recv_buf,
        message->length);

  struct stm32_hw_spi_cs *_cs_t = device->parent.user_data;
  if (message->cs_take) {
    GPIO_ResetBits(_cs_t->GPIOx, _cs_t->GPIO_Pin);
  }

  //处理spi的收发,边收边发，收，发
  if (message->send_buf && message->recv_buf) {
    if (RT_EOK != _hw_spi_bus_send_recv(SPI3, (rt_uint8_t *)message->send_buf,
                                        (rt_uint8_t *)message->recv_buf,
                                        message->length)) {
      LOG_W("file:%s line:%d _hw_spi_bus_send_recv() time out ", __FILE__,
            __LINE__);
      goto _exit;
    }
  } else if (message->send_buf) {
    if (RT_EOK != _hw_spi_bus_send(SPI3, (rt_uint8_t *)message->send_buf,
                                   message->length)) {
      LOG_W("file:%s line:%d  _hw_spi_bus_send() time out ", __FILE__,
            __LINE__);
      goto _exit;
    }
  } else if (message->recv_buf) {
    if (RT_EOK != _hw_spi_bus_recv(SPI3, (rt_uint8_t *)message->recv_buf,
                                   message->length)) {
      LOG_W("file:%s line:%d  _hw_spi_bus_recv() time out ", __FILE__,
            __LINE__);
      goto _exit;
    }
  } else {
    LOG_W("file:%s line:%d  send_buf == NULL && recv_buf == NULL", __FILE__,
          __LINE__);
    goto _exit;
  }

  if (message->cs_release) {
    GPIO_SetBits(_cs_t->GPIOx, _cs_t->GPIO_Pin);
  }

  return message->length;
_exit:
  return 0;
}

// spi 数据宽,模式,频率等配置
rt_err_t stm_spi_configure(struct rt_spi_device *device,
                           struct rt_spi_configuration *cfg) {
  RT_ASSERT(device != RT_NULL);
  RT_ASSERT(cfg != RT_NULL);

  SPI_InitTypeDef SPI_InitStructure;

  LOG_D("SPI3 device configure done");

  // fixme:暂时写死
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

  return RT_EOK;

  // if (cfg->mode & RT_SPI_SLAVE) {
  //   SPI_InitStructure.SPI_Mode = SPI_Mode_Slave;
  // } else {
  //   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  // }

  // if (cfg->mode & RT_SPI_3WIRE) {
  //   spi_handle->Init.Direction =
  //       SPI_Direction_1Line_Rx | SPI_Direction_1Line_Tx;
  // } else {
  //   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  // }

  // if (cfg->data_width == 8) {
  //   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  // } else if (cfg->data_width == 16) {
  //   SPI_InitStructure.SPI_DataSize = SPI_DataSize_16b;
  // } else {
  //   return RT_EIO;
  // }

  // if (cfg->mode & RT_SPI_CPHA) {
  //   SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  // } else {
  //   SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  // }

  // if (cfg->mode & RT_SPI_CPOL) {
  //   SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  // } else {
  //   SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  // }

  // if (cfg->mode & RT_SPI_NO_CS) {
  //   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  // } else {
  //   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  // }

  //return RT_EOK;
}

static const struct rt_spi_ops stm_spi_ops = {
    .configure = stm_spi_configure,
    .xfer = stm_spi_xfer,
};

struct rt_spi_bus stm_spi3_bus;

int spi_bus_init(void) {
  rt_err_t _rt = RT_EOK;
  SPI_InitTypeDef SPI_InitStructure;

  _gpio_config();

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

  _rt = rt_spi_bus_register(&stm_spi3_bus, SPI3_BUS_NAME, &stm_spi_ops);
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

  return _rt;
}

#endif

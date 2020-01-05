#ifndef __DVR_SPI3_H__
#define __DVR_SPI3_H__

#include <rthw.h>
#include <rtthread.h>
#include <stm32f2xx.h>

#define SPI3_BUS_NAME "SPI3_BUS"

//Attach the spi device to SPI bus, this function must be used after initialization
rt_err_t rt_hw_spi_device_attach(const char *bus_name, const char *device_name, GPIO_TypeDef *cs_gpiox, uint16_t cs_gpio_pin);

#endif


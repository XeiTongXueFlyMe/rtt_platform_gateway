#include <rtdevice.h>
#include <stm32f2xx.h>

#include "../applications/config.h"
#include "drv_usart1.h"

#ifdef USE_DRV_USART
#ifdef HW_USART_USING_UART1

#define USART_ITEM USART1
#define USART_IRQ USART1_IRQn
#define UART_GPIO_RCC RCC_AHB1Periph_GPIOA
#define UART_RCC RCC_APB2Periph_USART1
#define UART_GPIO_TX_PIN GPIO_Pin_9
#define UART_GPIO_RX_PIN GPIO_Pin_10
#define UART_GPIO_TX GPIOA
#define UART_GPIO_RX GPIOA

static void _rcc_configuration(void) {
  /* Enable UART GPIO clocks */
  RCC_AHB1PeriphClockCmd(UART_GPIO_RCC, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(UART_RCC, ENABLE);
}

static void _gpio_configuration(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  GPIO_PinAFConfig(UART_GPIO_TX, 9, GPIO_AF_USART1);
  GPIO_PinAFConfig(UART_GPIO_RX, 10, GPIO_AF_USART1);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART_GPIO_TX_PIN;
  GPIO_Init(UART_GPIO_TX, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = UART_GPIO_RX_PIN;
  GPIO_Init(UART_GPIO_RX, &GPIO_InitStructure);
}

static void _usart_configuration(void) {
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl =
      USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(USART_ITEM, &USART_InitStructure);

  NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_Cmd(USART_ITEM, ENABLE);

  USART_ClearFlag(USART_ITEM, USART_FLAG_TC | USART_FLAG_RXNE);
  USART_ITConfig(USART_ITEM, USART_IT_RXNE, ENABLE);
}

struct hw_usart_object {
  struct rt_serial_device serial;
};
typedef struct hw_usart_object hw_usart_object_t;

static hw_usart_object_t uart_obj;

static rt_err_t configure(struct rt_serial_device *serial,
                          struct serial_configure *cfg) {
  return RT_EOK;
}

static rt_err_t control(struct rt_serial_device *serial, int cmd, void *arg) {
  return RT_EOK;
}

static int getc(struct rt_serial_device *serial) {
  RT_ASSERT(serial != RT_NULL);
  if (USART_GetITStatus(USART_ITEM, USART_IT_RXNE) != RESET) {
    return (int)(USART_ReceiveData(USART_ITEM) & 0xff);
  }
  return -1;
}
static int putc(struct rt_serial_device *serial, char c) {
  RT_ASSERT(serial != RT_NULL);

  while (USART_GetFlagStatus(USART_ITEM, USART_FLAG_TXE) == RESET)
    ;
  USART_SendData(USART_ITEM, c);

  return 1;
}

static void _hw_usart_isr(void) {
  if (USART_GetITStatus(USART_ITEM, USART_IT_RXNE) != RESET) {
    rt_hw_serial_isr(&(uart_obj.serial), RT_SERIAL_EVENT_RX_IND);
    USART_ClearITPendingBit(USART_ITEM, USART_IT_RXNE);
  }
  if (USART_GetITStatus(USART_ITEM, USART_IT_TC) != RESET) {
    USART_ITConfig(USART_ITEM, USART_IT_TC, DISABLE);
    USART_ClearITPendingBit(USART_ITEM, USART_IT_TC);
  }
  if (USART_GetFlagStatus(USART_ITEM, USART_FLAG_ORE) == SET) {
    USART_ClearFlag(USART_ITEM, USART_FLAG_ORE);
    USART_ReceiveData(USART_ITEM);
  }
}
static const struct rt_uart_ops uart_ops = {
    configure, control, putc, getc, RT_NULL,
};

int rt_hw_usart1_init(void) {
  rt_err_t _rt = RT_EOK;
  _rcc_configuration();
  _gpio_configuration();
  _usart_configuration();

  struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
  hw_usart_object_t *usart_obj;

  usart_obj = &uart_obj;
  usart_obj->serial.ops = &uart_ops;
  usart_obj->serial.config = config;

  /* register UART device to serial*/
  _rt = rt_hw_serial_register(&(usart_obj->serial), USART1_DEVICE_NAME,
                              RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX,
                              usart_obj);
  RT_ASSERT(RT_EOK == _rt);
  return 0;
}

void USART1_IRQHandler() {
  rt_interrupt_enter();
  _hw_usart_isr();
  rt_interrupt_leave();
}

INIT_BOARD_EXPORT(rt_hw_usart1_init);

#endif /*HW_USART_USING_UART1*/

#endif /*USE_DRV_USART*/

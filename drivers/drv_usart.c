/* Change Logs
 * Date           Author       Notes
 * 2019-12-12     xieming      the first commit
 */
#include <rtdevice.h>
#include <stm32f2xx.h>
#include <stm32f2xx_dma.h>

#include "../applications/config.h"
#include "drv_usart.h"

#ifdef USE_DRV_USART

/* USART1 */
#define UART1_GPIO_TX_PIN GPIO_Pin_9
#define UART1_GPIO_RX_PIN GPIO_Pin_10
#define UART1_GPIO_TX GPIOA
#define UART1_GPIO_RX GPIOA
#define UART1_GPIO_RCC RCC_AHB1Periph_GPIOA

/* USART2 */
#define UART2_GPIO_TX_PIN GPIO_Pin_2
#define UART2_GPIO_RX_PIN GPIO_Pin_3
#define UART2_GPIO_TX GPIOA
#define UART2_GPIO_RX GPIOA
#define UART2_GPIO_RCC RCC_AHB1Periph_GPIOA

/* USART3 */
#define UART3_GPIO_TX_PIN GPIO_Pin_10
#define UART3_GPIO_RX_PIN GPIO_Pin_11
#define UART3_GPIO_TX GPIOB
#define UART3_GPIO_RX GPIOB
#define UART3_GPIO_RCC RCC_AHB1Periph_GPIOB

/* USART4 */
#define UART4_GPIO_TX_PIN GPIO_Pin_10
#define UART4_GPIO_RX_PIN GPIO_Pin_11
#define UART4_GPIO_TX GPIOC
#define UART4_GPIO_RX GPIOC
#define UART4_GPIO_RCC RCC_AHB1Periph_GPIOC

/* USART5 */
#define UART5_GPIO_TX_PIN GPIO_Pin_12
#define UART5_GPIO_RX_PIN GPIO_Pin_2
#define UART5_GPIO_TX GPIOC
#define UART5_GPIO_RX GPIOD
#define UART5_GPIO_RCC RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD

enum hw_usart {
#if (defined HW_USART_USING_UART1)
  HW_USART_1,
#endif

#if (defined HW_USART_USING_UART2)
  HW_USART_2,
#endif

#if (defined HW_USART_USING_UART3)
  HW_USART_3,
#endif

#if (defined HW_USART_USING_UART4)
  HW_USART_4,
#endif

  __HW_USART_END__,
};
typedef enum hw_usart hw_usart_t;

struct hw_usart_object {
  hw_usart_t name;
  USART_TypeDef *usart;
  uint8_t dma_disable;
  DMA_InitTypeDef dma_tx_cfg;
  DMA_InitTypeDef dma_rx_cfg;
  uint32_t dma_tx_ch;
  uint32_t dma_rx_ch;
  DMA_Stream_TypeDef *dma_tx_Stream;
  DMA_Stream_TypeDef *dma_rx_Stream;
  uint32_t dma_tx_flags;
  uint32_t dma_rx_flags;
  uint32_t baud;
  uint8_t irq_ch;
#if (HW_USART_SEND_DMA_ENABLE == 1)
  uint8_t buf_tx[HW_USART_BUFFER_TSIZE];
#endif
  uint8_t buf_rx[HW_USART_BUFFER_RSIZE];
  uint16_t rx_length;
  struct rt_serial_device serial;
};
typedef struct hw_usart_object hw_usart_object_t;

#define HW_USART_DMA_NUM \
  (sizeof(hw_usart_object_list) / sizeof(struct hw_usart_object))

#define HW_USART_OBJ(index) (&hw_usart_object_list[index])

struct hw_usart_object hw_usart_object_list[] = {
#if (defined HW_USART_USING_UART1)
    {
        .name = HW_USART_1,
        .usart = USART1,
        .dma_disable = 0,
        .dma_tx_ch = DMA_Channel_4,
        .dma_rx_ch = DMA_Channel_4,
        .dma_tx_Stream = DMA2_Stream7,
        .dma_rx_Stream = DMA2_Stream2,
        .dma_tx_flags = DMA_FLAG_FEIF7 | DMA_FLAG_DMEIF7 | DMA_FLAG_TEIF7 |
                        DMA_FLAG_HTIF7 | DMA_FLAG_TCIF7,
        .dma_rx_flags = DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 |
                        DMA_FLAG_HTIF2 | DMA_FLAG_TCIF2,
        .baud = 115200,
        .irq_ch = USART1_IRQn,
    },
#endif

#if (defined HW_USART_USING_UART2)
    {
        .name = HW_USART_2,
        .usart = USART2,
        .dma_disable = 0,
        .dma_tx_ch = DMA_Channel_4,
        .dma_rx_ch = DMA_Channel_4,
        .dma_tx_Stream = DMA1_Stream6,
        .dma_rx_Stream = DMA1_Stream5,
        .dma_tx_flags = DMA_FLAG_FEIF6 | DMA_FLAG_DMEIF6 | DMA_FLAG_TEIF6 |
                        DMA_FLAG_HTIF6 | DMA_FLAG_TCIF6,
        .dma_rx_flags = DMA_FLAG_FEIF5 | DMA_FLAG_DMEIF5 | DMA_FLAG_TEIF5 |
                        DMA_FLAG_HTIF5 | DMA_FLAG_TCIF5,
        .baud = 115200,
        .irq_ch = USART2_IRQn,
    },
#endif

#if (defined HW_USART_USING_UART3)
    {
        .name = HW_USART_3,
        .usart = USART3,
        .dma_disable = 1,
        .dma_tx_ch = DMA_Channel_4,
        .dma_rx_ch = DMA_Channel_4,
        .dma_tx_Stream = DMA1_Stream3,
        .dma_rx_Stream = DMA1_Stream1,
        .dma_tx_flags = DMA_FLAG_FEIF3 | DMA_FLAG_DMEIF3 | DMA_FLAG_TEIF3 |
                        DMA_FLAG_HTIF3 | DMA_FLAG_TCIF3,
        .dma_rx_flags = DMA_FLAG_FEIF1 | DMA_FLAG_DMEIF1 | DMA_FLAG_TEIF1 |
                        DMA_FLAG_HTIF1 | DMA_FLAG_TCIF1,
        .baud = 115200,
        .irq_ch = USART3_IRQn,
    },
#endif

#if (defined HW_USART_USING_UART4)
    {
        .name = HW_USART_4,
        .usart = UART4,
        .dma_disable = 0,
        .dma_tx_ch = DMA_Channel_4,
        .dma_rx_ch = DMA_Channel_4,
        .dma_tx_Stream = DMA1_Stream4,
        .dma_rx_Stream = DMA1_Stream2,
        .dma_tx_flags = DMA_FLAG_FEIF4 | DMA_FLAG_DMEIF4 | DMA_FLAG_TEIF4 |
                        DMA_FLAG_HTIF4 | DMA_FLAG_TCIF4,
        .dma_rx_flags = DMA_FLAG_FEIF2 | DMA_FLAG_DMEIF2 | DMA_FLAG_TEIF2 |
                        DMA_FLAG_HTIF2 | DMA_FLAG_TCIF2,
        .baud = 115200,
        .irq_ch = UART4_IRQn,
    },
#endif
};

static void _rcc_configuration(void) {
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

#ifdef HW_USART_USING_UART1
  /* Enable UART GPIO clocks */
  RCC_AHB1PeriphClockCmd(UART1_GPIO_RCC, ENABLE);
  /* Enable UART clock */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
#endif /* HW_USART_USING_UART1 */

#ifdef HW_USART_USING_UART2
  /* Enable UART GPIO clocks */
  RCC_AHB1PeriphClockCmd(UART2_GPIO_RCC, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
#endif /* HW_USART_USING_UART2 */

#ifdef HW_USART_USING_UART3
  /* Enable UART GPIO clocks */
  RCC_AHB1PeriphClockCmd(UART3_GPIO_RCC, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
#endif /* HW_USART_USING_UART3 */

#ifdef HW_USART_USING_UART4
  /* Enable UART GPIO clocks */
  RCC_AHB1PeriphClockCmd(UART4_GPIO_RCC, ENABLE);
  /* Enable UART clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);
#endif /* HW_USART_USING_UART4 */
}

static void _gpio_configuration(void) {
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;

#ifdef HW_USART_USING_UART1
  GPIO_PinAFConfig(GPIOA, 9, GPIO_AF_USART1);
  GPIO_PinAFConfig(GPIOA, 10, GPIO_AF_USART1);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART1_GPIO_TX_PIN | UART1_GPIO_RX_PIN;
  GPIO_Init(UART1_GPIO_TX, &GPIO_InitStructure);
#endif /* HW_USART_USING_UART1 */

#ifdef HW_USART_USING_UART2
  GPIO_PinAFConfig(GPIOA, 2, GPIO_AF_USART2);
  GPIO_PinAFConfig(GPIOA, 3, GPIO_AF_USART2);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART2_GPIO_TX_PIN;
  GPIO_Init(UART2_GPIO_TX, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = UART2_GPIO_RX_PIN;
  GPIO_Init(UART2_GPIO_RX, &GPIO_InitStructure);

#endif /* HW_USART_USING_UART2 */

#ifdef HW_USART_USING_UART3
  GPIO_PinAFConfig(GPIOB, 10, GPIO_AF_USART3);
  GPIO_PinAFConfig(GPIOB, 11, GPIO_AF_USART3);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART3_GPIO_TX_PIN | UART3_GPIO_RX_PIN;
  GPIO_Init(UART3_GPIO_TX, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = UART3_GPIO_RX_PIN;
  GPIO_Init(UART3_GPIO_RX, &GPIO_InitStructure);
#endif /* HW_USART_USING_UART3 */

#ifdef HW_USART_USING_UART4
  /* Configure USART Rx/tx PIN */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure..GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART4_GPIO_TX_PIN | UART4_GPIO_RX_PIN;
  GPIO_Init(UART4_GPIO_TX, &GPIO_InitStructure);
#endif /* HW_USART_USING_UART4 */

#ifdef HW_USART_USING_UART5
  GPIO_PinAFConfig(GPIOC, 12, GPIO_AF_UART5);
  GPIO_PinAFConfig(GPIOD, 2, GPIO_AF_UART5);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = UART5_GPIO_TX_PIN | UART5_GPIO_RX_PIN;
  GPIO_Init(UART5_GPIO_TX, &GPIO_InitStructure);
#endif /* HW_USART_USING_UART5 */
}

static void _usart_configuration(void) {
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  int i;
  struct hw_usart_object *usart_obj;

  for (i = 0; i < HW_USART_DMA_NUM; i++) {
    usart_obj = HW_USART_OBJ(i);

    USART_InitStructure.USART_BaudRate = usart_obj->baud;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl =
        USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(usart_obj->usart, &USART_InitStructure);

    if (usart_obj->dma_disable != 1) {
#if (HW_USART_SEND_DMA_ENABLE == 1)
      USART_DMACmd(usart_obj->usart, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);

      /* Initialize DMA TX Channel */
      usart_obj->dma_tx_cfg.DMA_Channel = usart_obj->dma_tx_ch;
      usart_obj->dma_tx_cfg.DMA_DIR = DMA_DIR_PeripheralDST;
      usart_obj->dma_tx_cfg.DMA_M2M = DMA_M2M_Disable;
      usart_obj->dma_tx_cfg.DMA_Memory0BaseAddr = (uint32_t)&usart_obj->buf_tx;
      usart_obj->dma_tx_cfg.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
      usart_obj->dma_tx_cfg.DMA_MemoryInc = DMA_MemoryInc_Enable;
      usart_obj->dma_tx_cfg.DMA_Mode = DMA_Mode_Normal;
      usart_obj->dma_tx_cfg.DMA_PeripheralBaseAddr =
          (uint32_t)&usart_obj->usart->DR;
      usart_obj->dma_tx_cfg.DMA_PeripheralDataSize =
          DMA_PeripheralDataSize_Byte;
      usart_obj->dma_tx_cfg.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
      usart_obj->dma_tx_cfg.DMA_Priority = DMA_Priority_High;
#else
      USART_DMACmd(usart_obj->usart, USART_DMAReq_Rx, ENABLE);
#endif

      /* Initialize DMA RX Channel */
      DMA_Cmd(usart_obj->dma_rx_Stream, DISABLE);
      DMA_DeInit(usart_obj->dma_rx_Stream);
      usart_obj->dma_rx_cfg.DMA_Channel = usart_obj->dma_rx_ch;
      usart_obj->dma_rx_cfg.DMA_BufferSize = HW_USART_BUFFER_RSIZE;
      usart_obj->dma_rx_cfg.DMA_DIR = DMA_DIR_PeripheralToMemory;
      usart_obj->dma_rx_cfg.DMA_FIFOMode = DMA_FIFOMode_Disable;
      usart_obj->dma_rx_cfg.DMA_Memory0BaseAddr = (uint32_t)&usart_obj->buf_rx;
      usart_obj->dma_rx_cfg.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
      usart_obj->dma_rx_cfg.DMA_MemoryInc = DMA_MemoryInc_Enable;
      usart_obj->dma_rx_cfg.DMA_Mode = DMA_Mode_Normal;
      usart_obj->dma_rx_cfg.DMA_PeripheralBaseAddr =
          (uint32_t)&usart_obj->usart->DR;
      usart_obj->dma_rx_cfg.DMA_PeripheralDataSize =
          DMA_PeripheralDataSize_Byte;
      usart_obj->dma_rx_cfg.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
      usart_obj->dma_rx_cfg.DMA_Priority = DMA_Priority_High;
      DMA_Init(usart_obj->dma_rx_Stream, &usart_obj->dma_rx_cfg);
      DMA_Cmd(usart_obj->dma_rx_Stream, ENABLE);
    }

    NVIC_InitStructure.NVIC_IRQChannel = usart_obj->irq_ch;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    USART_Cmd(usart_obj->usart, ENABLE);

    if (usart_obj->dma_disable != 1) {
      USART_ITConfig(usart_obj->usart, USART_IT_IDLE, ENABLE);
    } else {
      USART_ClearFlag(usart_obj->usart, USART_FLAG_TC | USART_FLAG_RXNE);
      USART_ITConfig(usart_obj->usart, USART_IT_RXNE, ENABLE);
    }
  }
}

static rt_err_t configure(struct rt_serial_device *serial,
                          struct serial_configure *cfg) {
  return RT_EOK;
}

static rt_err_t control(struct rt_serial_device *serial, int cmd, void *arg) {
  return RT_EOK;
}
int getc(struct rt_serial_device *serial) {
  RT_ASSERT(serial != RT_NULL);
  hw_usart_object_t *usart_obj = (hw_usart_object_t *)serial->parent.user_data;
  if (USART_GetITStatus(usart_obj->usart, USART_IT_RXNE) != RESET) {
    return (int)(USART_ReceiveData(usart_obj->usart) & 0xff);
  }
  return -1;
}
static int putc(struct rt_serial_device *serial, char c) {
  hw_usart_object_t *usart_obj;
  RT_ASSERT(serial != RT_NULL);
  usart_obj = (hw_usart_object_t *)serial->parent.user_data;

  while (USART_GetFlagStatus(usart_obj->usart, USART_FLAG_TXE) == RESET)
    ;
  USART_SendData(usart_obj->usart, c);

  return 1;
}

static rt_size_t dma_transmit(struct rt_serial_device *serial, rt_uint8_t *buf,
                              rt_size_t size, int direction) {
  if (RT_SERIAL_DMA_RX != direction) return 0;

  uint16_t count = 0;
  hw_usart_object_t *usart_obj;
  usart_obj = (hw_usart_object_t *)serial->parent.user_data;

  count = (size >= usart_obj->rx_length) ? usart_obj->rx_length : size;
  rt_memcpy(buf, usart_obj->buf_rx, count);

  return count;
}

// static void _hw_usart_dma_isr(hw_usart_t usart)
// {
//   hw_usart_object_t *usart_obj = HW_USART_OBJ(usart);

//   if (SET == USART_GetITStatus(usart_obj->usart, USART_IT_IDLE))
//   {
//     /* Dummy read to clear flags */
//     USART_ReceiveData(usart_obj->usart);

//     usart_obj->rx_length = HW_USART_BUFFER_RSIZE -
//     DMA_GetCurrDataCounter(usart_obj->dma_rx_Stream); if
//     (usart_obj->rx_length > 0)
//     {
//       /* Indicate data receive */
//       rt_hw_serial_isr(&(usart_obj->serial), RT_SERIAL_EVENT_RX_DMADONE |
//       (usart_obj->rx_length << 8));

//       DMA_Cmd(usart_obj->dma_rx_Stream, DISABLE);
//       DMA_DeInit(usart_obj->dma_rx_Stream);
//       DMA_Init(usart_obj->dma_rx_Stream, &usart_obj->dma_rx_cfg);
//       DMA_Cmd(usart_obj->dma_rx_Stream, ENABLE);
//     }
//   }
// }
static void _hw_usart_isr(hw_usart_t usart) {
  hw_usart_object_t *usart_obj = HW_USART_OBJ(usart);

  if (USART_GetITStatus(usart_obj->usart, USART_IT_RXNE) != RESET) {
    rt_hw_serial_isr(&(usart_obj->serial), RT_SERIAL_EVENT_RX_IND);
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
  if (USART_GetITStatus(usart_obj->usart, USART_IT_TC) != RESET) {
    USART_ITConfig(usart_obj->usart, USART_IT_TC, DISABLE);
    USART_ClearITPendingBit(usart_obj->usart, USART_IT_TC);
  }
  if (USART_GetFlagStatus(usart_obj->usart, USART_FLAG_ORE) == SET) {
    USART_ClearFlag(usart_obj->usart, USART_FLAG_ORE);
    USART_ReceiveData(usart_obj->usart);
  }
}
static const struct rt_uart_ops uart_3_ops = {
    configure, control, putc, getc, dma_transmit,
};

// uarst写死的配置，调用配置驱动函数为空
int rt_hw_usart_init(void) {
  _rcc_configuration();
  _gpio_configuration();
  _usart_configuration();

  struct serial_configure config = RT_SERIAL_CONFIG_DEFAULT;
  hw_usart_object_t *usart_obj;

#ifdef HW_USART_USING_UART1
  usart_obj = HW_USART_OBJ(HW_USART_1);
  usart_obj->serial.ops = &uart_1_ops;
  usart_obj->serial.config = config;

  /* register UART3 device to serial*/
  rt_hw_serial_register(&(usart_obj->serial), "uart1",
                        RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, usart_obj);
#endif

#ifdef HW_USART_USING_UART2
  usart_obj = HW_USART_OBJ(HW_USART_2);
  usart_obj->serial.ops = &uart_2_ops;
  usart_obj->serial.config = config;

  /* register UART3 device to serial*/
  rt_hw_serial_register(&(usart_obj->serial), "uart2",
                        RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, usart_obj);
#endif

#ifdef HW_USART_USING_UART3
  usart_obj = HW_USART_OBJ(HW_USART_3);
  usart_obj->serial.ops = &uart_3_ops;
  usart_obj->serial.config = config;

  /* register UART3 device to serial*/
  rt_hw_serial_register(&(usart_obj->serial), "uart3",
                        RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, usart_obj);

#endif

#ifdef HW_USART_USING_UART4
  usart_obj = HW_USART_OBJ(HW_USART_4);
  usart_obj->serial.ops = &uart_4_ops;
  usart_obj->serial.config = config;

  /* register UART3 device to serial*/
  rt_hw_serial_register(&(usart_obj->serial), "uart4",
                        RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX, usart_obj);
#endif

  return 0;
}

#if (defined HW_USART_USING_UART1)
void USART1_IRQHandler() {
  rt_interrupt_enter();
  _hw_usart_dma_isr(HW_USART_1);
  rt_interrupt_leave();
}
#endif

#if (defined HW_USART_USING_UART2)
void USART2_IRQHandler() {
  rt_interrupt_enter();
  _hw_usart_dma_isr(HW_USART_2);
  rt_interrupt_leave();
}
#endif

#if (defined HW_USART_USING_UART3)
void USART3_IRQHandler() {
  rt_interrupt_enter();
  _hw_usart_isr(HW_USART_3);
  rt_interrupt_leave();
}
#endif

#if (defined HW_USART_USING_UART4)
void UART4_IRQHandler() {
  rt_interrupt_enter();
  _hw_usart_dma_isr(HW_USART_4);
  rt_interrupt_leave();
}
#endif

#endif

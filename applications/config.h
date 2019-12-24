#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG
//#define UNIT_TEST
#define ULOG_OUTPUT_LVL   LOG_LVL_DBG

#define USE_DRV_USART
#define USE_DRV_GPIO_LED
#define USE_DRV_XWATCHDOG
#define USE_DRV_FLASH

/* uart */
#ifdef USE_DRV_USART
#define HW_USART_BUFFER_RSIZE     1024
#define HW_USART_BUFFER_TSIZE     1024
#define HW_USART_BUFFER_SIZE      HW_USART_BUFFER_RSIZE
//#define HW_USART_SEND_DMA_ENABLE 1
//#define HW_USART_USING_UART1
//#define HW_USART_USING_UART2
#define HW_USART_USING_UART3
//#define HW_USART_USING_UART4
#endif

#ifdef DEBUG
#define USE_FULL_ASSERT
#endif

#endif

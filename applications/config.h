#ifndef __CONFIG_H
#define __CONFIG_H

#define DEBUG
//#define UNIT_TEST
#define ULOG_OUTPUT_LVL LOG_LVL_DBG

/*============= gateway config ================*/
//TODO
#define IOT_GATEWAY_MAC "AFAEB1CAEDE7"
#define get_iot_gateway_mac() IOT_GATEWAY_MAC
#define srpan_nwk_node_count() 0
#define get_iot_gateway_csq() "23"

#define SERVER_ADDER_URL "iot.elitesemicon.com.cn"
#define SERVER_ADDER_PORT 8287

/*============= device drivers =============*/
#define USE_APP_RF
#define USE_DRV_USART
#define USE_DRV_GPIO_LED
#define USE_DRV_GPIO_SW
#define USE_DRV_XWATCHDOG
#define USE_DRV_IWATCHDOG
#define USE_DRV_FLASH
#define USE_DRV_SPI

/* uart */
#ifdef USE_DRV_USART
#define HW_USART_BUFFER_RSIZE 1024
#define HW_USART_BUFFER_TSIZE 1024
#define HW_USART_BUFFER_SIZE HW_USART_BUFFER_RSIZE

#define HW_USART_USING_UART1
//#define HW_USART_USING_UART2
#define HW_USART_USING_UART3
//#define HW_USART_USING_UART4
#endif

#ifdef HW_USART_USING_UART1
// FIXME
#define AT_DEVICE_SOCKETS_NUM 1
#define USE_DRV_EG25E
#endif /*HW_USART_USING_UART1*/

#ifdef HW_USART_USING_UART4
#define USE_USR_K7
#endif

/*spi*/
#ifdef USE_DRV_SPI
#define USE_W25QXX
#endif

#ifdef DEBUG
#define USE_FULL_ASSERT
#endif

#endif

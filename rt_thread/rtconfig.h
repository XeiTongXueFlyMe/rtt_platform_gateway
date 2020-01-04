#ifndef RT_CONFIG_H__
#define RT_CONFIG_H__

#include "../applications/config.h"
/* Automatically generated file; DO NOT EDIT. */
/* RT-Thread Project Configuration */

/* RT-Thread Kernel */
#define RT_NAME_MAX 8
#define RT_ALIGN_SIZE 4
#define RT_THREAD_PRIORITY_32
#define RT_THREAD_PRIORITY_MAX 32
#define RT_TICK_PER_SECOND 100
#define RT_USING_OVERFLOW_CHECK
#define RT_USING_HOOK
#define RT_USING_IDLE_HOOK
#define RT_IDEL_HOOK_LIST_SIZE 4
#define IDLE_THREAD_STACK_SIZE 512
#define RT_DEBUG

/* Inter-Thread communication */
#define RT_USING_SEMAPHORE
#define RT_USING_MUTEX
#define RT_USING_EVENT
#define RT_USING_MAILBOX
#define RT_USING_MESSAGEQUEUE
#define RT_USING_SIGNALS

/* Memory Management */
#define RT_USING_MEMPOOL
#define RT_USING_MEMHEAP
#define RT_USING_SMALL_MEM
#define RT_USING_HEAP

/* Kernel Device Object */
#define RT_USING_DEVICE
#define RT_USING_CONSOLE
#define RT_CONSOLEBUF_SIZE 128
#define RT_CONSOLE_DEVICE_NAME "uart1"

/*
#define RT_VER_NUM 0x40001
#define ARCH_ARM
#define ARCH_ARM_CORTEX_M
#define ARCH_ARM_CORTEX_M3
*/
/* RT-Thread Components */
#define RT_USING_COMPONENTS_INIT
#define RT_USING_ULOG
#define ULOG_USING_COLOR
#define ULOG_LINE_BUF_SIZE 512
#define ULOG_OUTPUT_LEVEL
#define ULOG_OUTPUT_TAG
#define ULOG_OUTPUT_THREAD_NAME
#define ULOG_BACKEND_USING_CONSOLE

/* C++ features */

/* Command shell */
#define RT_USING_FINSH
#define FINSH_THREAD_NAME "tshell"
#define FINSH_USING_HISTORY
#define FINSH_HISTORY_LINES 5
#define FINSH_USING_SYMTAB
#define FINSH_USING_DESCRIPTION
#define FINSH_THREAD_PRIORITY 20
#define FINSH_THREAD_STACK_SIZE (8 * 1024)
#define FINSH_CMD_SIZE 80
#define FINSH_USING_MSH
#define FINSH_USING_MSH_DEFAULT
//#define FINSH_ARG_MAX 10
#define FINSH_USING_MSH_ONLY

/* Device virtual file system */

/* elm-chan's FatFs, Generic FAT Filesystem Module */

/* Device Drivers */
#define RT_USING_DEVICE_IPC
#define RT_USING_SERIAL
#define RT_USING_PIN
#define RT_USING_WDT
#define RT_USING_NVM
#define RT_USING_SPI
#define RT_USING_RTC
#define RT_USING_SOFT_RTC

//#define RT_USING_DEVICE_IPC
//#define RT_PIPE_BUFSZ 512
//#define RT_USING_RTC

/* Network */
#define RT_USING_NETDEV
#define NETDEV_USING_IFCONFIG
#define NETDEV_USING_PING
#define NETDEV_USING_NETSTAT
#define NETDEV_USING_AUTO_DEFAULT
#define RT_USING_SAL
#define SAL_USING_AT
//#define SAL_USING_LWIP
//#define SAL_USING_TLS
//#define SAL_USING_POSIX
/* Using WiFi */

/* Using USB */

/* POSIX layer and C standard library */

//#define RT_USING_LIBC

/* Socket abstraction layer */

/* light weight TCP/IP stack */

/* Modbus master and slave stack */

/* AT commands */
#define RT_USING_AT
#define AT_USING_CLIENT
#define AT_CLIENT_NUM_MAX 1
#define AT_USING_SOCKET
//#define AT_USING_CLI
#define AT_PRINT_RAW_CMD //用于开启 AT 命令通信数据的实时显示模式，方便调试

/* VBUS(Virtual Software BUS) */

/* Utilities */

/* ARM CMSIS */

/* RT-Thread online packages */

/* IoT - internet of things */

/* Wi-Fi */

/* Marvell WiFi */

/* Wiced WiFi */

/* IoT Cloud */

/* security packages */

/* language packages */

/* multimedia packages */

/* tools packages */

/* system packages */

/* peripheral libraries and drivers */

/* miscellaneous packages */

/* samples: kernel and components samples */

#define SOC_STM32F2
#define RT_USING_UART1

#endif

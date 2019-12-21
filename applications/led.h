#ifndef __LED_H
#define __LED_H

//连接上服务器后,此灯闪烁
#define EVENT_NET_STATUS_OPEN (1 << 0)
#define EVENT_NET_STATUS_CLOSE (1 << 1)
#define EVENT_NET_STATUS_BLINK (1 << 2)

#define EVENT_GPRS_OPEN (1 << 3)
#define EVENT_GPRS_CLOSE (1 << 4)
#define EVENT_GPRS_BLINK (1 << 5)

#define EVENT_RF_OPEN (1 << 6)
#define EVENT_RF_CLOSE (1 << 7)
#define EVENT_RF_BLINK (1 << 8)

#define EVENT_SYS_OPEN (1 << 9)
#define EVENT_SYS_CLOSE (1 << 10)
#define EVENT_SYS_BLINK (1 << 11)

void led_thread_init(void);

#endif

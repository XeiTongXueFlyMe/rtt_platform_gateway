// TODO:直接重启，定时重启，每天重启
#include <stm32f2xx.h>
#include "rtdevice.h"
#include "rtthread.h"

int reboot(void) {
	__set_FAULTMASK(1);
	NVIC_SystemReset();
	for(;;);
}

MSH_CMD_EXPORT(reboot, reset sys);

// TODO:直接重启，定时重启，每天重启
#include <stm32f2xx.h>
#include "rtdevice.h"
#include "rtthread.h"

#if defined(RT_USING_CONSOLE)

int reboot(void) {
  __set_FAULTMASK(1);
  NVIC_SystemReset();
  for (;;)
    ;
}

MSH_CMD_EXPORT(reboot, reset sys);


int clean(void){
	rt_kprintf("%s","\033[1H\033[2J");
	return 0;
}	

MSH_CMD_EXPORT(clean, Clear Screen CLS);

#endif

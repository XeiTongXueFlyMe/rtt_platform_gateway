#include <rtthread.h>
#include "../drivers/board.h"
#include "led.h"
#include "wdg.h"

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif

int rt_application_init() {
#ifdef RT_USING_FINSH
  finsh_system_init();
#endif
  rt_components_init();


#ifndef UNIT_TEST

#ifdef USE_DRV_XWATCHDOG
  wdg_thread_init();
#endif

#ifdef USE_DRV_GPIO_LED
  led_thread_init();
#endif

#endif /*UNIT_TEST*/

  return 0;
}

/*@}*/

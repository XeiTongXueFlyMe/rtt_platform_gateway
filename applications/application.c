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

#ifndef UNIT_TEST
  rt_components_init();
#endif /*UNIT_TEST*/

  return 0;
}

/*@}*/

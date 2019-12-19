/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-01-05     Bernard      the first version
 */

/**
 * @addtogroup STM32
 */
/*@{*/

#include <rtthread.h>
#include "../drivers/board.h"
#include "led.h"

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif


int rt_application_init() {
  
#ifdef RT_USING_FINSH
    finsh_system_init();
#endif
	  /* RT-Thread components initialization */
    rt_components_init();
#ifndef UNIT_TEST

#ifdef USE_DRV_GPIO_LED
  led_thread_init();
#endif

#endif/*USE_DRV_GPIO_LED*/

  return 0;
}

/*@}*/

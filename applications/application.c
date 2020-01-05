#include <rtthread.h>
#include "../drivers/board.h"
#include "led.h"
#include "wdg.h"

#ifdef RT_USING_FINSH
#include <finsh.h>
#include <shell.h>
#endif

/* the system main thread */
void main_thread_entry(void *parameter) {
#ifdef RT_USING_FINSH
  finsh_system_init();
#endif

  rt_components_init();
}

int rt_application_init() {
  rt_thread_t _tid;

  _tid =
      rt_thread_create("main", main_thread_entry, RT_NULL,
                       2048, 1, 20);
  RT_ASSERT(_tid != RT_NULL);

  rt_thread_startup(_tid);
  return 0;
}

/*@}*/

#include "jansson.h"

#include <rtthread.h>

/* gettimeofday() and getpid() */
int seed_from_timestamp_and_pid(uint32_t *seed) {
  // *seed = SysTick->VAL
  *seed = rt_tick_get();
  return 0;
}

int jansson_init(void) {
  json_set_alloc_funcs((json_malloc_t)rt_malloc, rt_free);

  return 0;
}

INIT_PREV_EXPORT(jansson_init);

#include "jansson.h"

#include <rtthread.h>

int jansson_init(void)
{
    json_set_alloc_funcs((json_malloc_t) rt_malloc, rt_free);

    return 0;
}

INIT_PREV_EXPORT(jansson_init);

#ifndef _NVM_H_
#define _NVM_H_

#include <rtthread.h>

#define RT_NVM_ERASE_PAGE_CMD 1
#define RT_NVM_ERASE_SECTORS_CMD 2
#define RT_NVM_ERASE_ALL_CMD 3
#define RT_NVM_ERSET_CMD 4

typedef struct rt_nvm_device rt_nvm_device_t;
struct rt_nvm_ops {
  rt_err_t (*init)(rt_nvm_device_t *nvm);
  rt_err_t (*control)(rt_nvm_device_t *nvm, int cmd, void *arg);
  rt_size_t (*nvm_read)(rt_nvm_device_t *nvm, rt_off_t pos, rt_uint8_t *buf,
                        size_t size);
  rt_size_t (*nvm_write)(rt_nvm_device_t *nvm, rt_off_t pos,
                         const rt_uint8_t *buf, size_t size);
};

struct rt_nvm_device {
  struct rt_device parent;
  struct rt_nvm_ops *ops;
};

rt_err_t rt_hw_nvm_register(struct rt_nvm_device *nvm_t, const char *name,
                            rt_uint32_t flag, void *private_data);

#endif

/*
 * COPYRIGHT (C) 2018, Real-Thread Information Technology Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author      Notes
 * 2019-12-22     xieming     first version.
 *
 * warry:
 * 此驱动可能被多个应用打开操作，需做好原子操作
 */
#include <drivers/nvm.h>

#ifdef RT_USING_NVM

static rt_mutex_t nvm_rw_mutex_t;

rt_err_t rt_nvm_init(rt_device_t dev) {
  rt_err_t _rt = RT_EOK;
  struct rt_nvm_device *_d = RT_NULL;

  if (dev->flag & RT_DEVICE_FLAG_ACTIVATED) {
    goto INIT_FINISHED;
  } else {
    dev->flag |= RT_DEVICE_FLAG_ACTIVATED;
  }

  nvm_rw_mutex_t = rt_mutex_create("nvm_rw_mutex", RT_IPC_FLAG_FIFO);
  RT_ASSERT(RT_NULL != nvm_rw_mutex_t)

  RT_ASSERT(RT_NULL != dev)
  _d = (struct rt_nvm_device *)dev;

  _rt = _d->ops->init(_d);

INIT_FINISHED:
  return _rt;
}

rt_err_t rt_nvm_open(rt_device_t dev, rt_uint16_t oflag) {
  rt_err_t _rt = RT_EOK;

  return _rt;
}

rt_err_t rt_nvm_close(rt_device_t dev) {
  rt_err_t _rt = RT_EOK;

  return _rt;
}

rt_size_t rt_nvm_read(rt_device_t dev, rt_off_t pos, void *buffer,
                      rt_size_t size) {
  struct rt_nvm_device *_d = RT_NULL;
  rt_size_t _len = 0;

  RT_ASSERT(RT_NULL != nvm_rw_mutex_t)
  RT_ASSERT(RT_NULL != dev)
  _d = (struct rt_nvm_device *)dev;

  rt_mutex_take(nvm_rw_mutex_t, rt_thread_mdelay(100));
  _len = _d->ops->nvm_read(_d, pos, buffer, size);
  rt_mutex_release(nvm_rw_mutex_t);

  return _len;
}

rt_size_t rt_nvm_write(rt_device_t dev, rt_off_t pos, const void *buffer,
                       rt_size_t size) {
  struct rt_nvm_device *_d = RT_NULL;
  rt_size_t _len = 0;

  RT_ASSERT(RT_NULL != nvm_rw_mutex_t)
  RT_ASSERT(RT_NULL != dev)
  _d = (struct rt_nvm_device *)dev;

  rt_mutex_take(nvm_rw_mutex_t, rt_thread_mdelay(100));
  _len = _d->ops->nvm_write(_d, pos, buffer, size);
  rt_mutex_release(nvm_rw_mutex_t);

  return _len;
}

rt_err_t rt_nvm_control(rt_device_t dev, int cmd, void *args) {
  rt_err_t _rt = RT_EOK;
  struct rt_nvm_device *_d = RT_NULL;

  RT_ASSERT(RT_NULL != nvm_rw_mutex_t)
  RT_ASSERT(RT_NULL != dev)
  _d = (struct rt_nvm_device *)dev;

  rt_mutex_take(nvm_rw_mutex_t, rt_thread_mdelay(100));
  _rt = _d->ops->control(_d, cmd, args);
  rt_mutex_release(nvm_rw_mutex_t);

  return _rt;
}

#ifdef RT_USING_DEVICE_OPS
const static struct rt_device_ops nvm_ops = {
    rt_nvm_init, rt_nvm_open,  rt_nvm_close,
    rt_nvm_read, rt_nvm_write, rt_nvm_control,
};
#endif

rt_err_t rt_hw_nvm_register(struct rt_nvm_device *nvm_t, const char *name,
                            rt_uint32_t flag, void *private_data) {
  struct rt_device *_device_t;
  RT_ASSERT(RT_NULL != nvm_t);

  _device_t = &nvm_t->parent;
  _device_t->type = RT_Device_Class_Block;
  _device_t->rx_indicate = RT_NULL;
  _device_t->tx_complete = RT_NULL;
#ifdef RT_USING_DEVICE_OPS
  _device_t->ops = &nvm_ops;
#else
  _device_t->init = rt_nvm_init;
  _device_t->open = rt_nvm_open;
  _device_t->close = rt_nvm_close;
  _device_t->read = rt_nvm_read;
  _device_t->write = rt_nvm_write;
  _device_t->control = rt_nvm_control;
#endif
  _device_t->user_data = private_data;

  return rt_device_register(_device_t, name, flag);
}

#endif

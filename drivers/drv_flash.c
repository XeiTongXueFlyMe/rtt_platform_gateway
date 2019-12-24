#include "drv_flash.h"

#ifdef USE_DRV_FLASH

//使用stm32f20x片上flash的最后128k作为nvm驱动操作
#define HW_STM32_FLASH_START_ADDR (0x080E0000)
#define HW_STM32_FLASH_SIZE (0x00020000)

#define LOG_TAG "drv_flash"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

rt_err_t stm32_flash_init(rt_nvm_device_t *nvm) { return RT_EOK; }
rt_err_t stm32_flash_control(rt_nvm_device_t *nvm, int cmd, void *arg) {
  FLASH_Status _status;
  rt_err_t _rt = RT_EOK;
  switch (cmd) {
    case RT_NVM_ERASE_PAGE_CMD:
      break;
    case RT_NVM_ERASE_SECTORS_CMD:
      break;
    case RT_NVM_ERASE_ALL_CMD:
      FLASH_Unlock();
      FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
      _status = FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
      if(FLASH_COMPLETE != _status){
        _rt = RT_EIO;
        LOG_E("%s line:%d  FLASH_EraseSector fail", __FILE__, __LINE__);
      }
      FLASH_Lock();
      break;
    case RT_NVM_ERSET_CMD:
      break;

    default:
      goto NO_CMD;
  }
  return _rt;

NO_CMD:
  return RT_EINVAL;
}
rt_size_t stm32_flash_read(rt_nvm_device_t *nvm, rt_off_t pos, rt_uint8_t *buf,
                           size_t size) {
  rt_uint32_t _cycle = size / 4;
  rt_uint32_t _cycle_index = size % 4;
  rt_uint32_t *_rbuf_t = (rt_uint32_t *)buf;
  rt_uint32_t *_raddr_t = (rt_uint32_t *)(HW_STM32_FLASH_START_ADDR + pos);
  rt_uint8_t *_rbuf_8_t = RT_NULL;
  rt_uint8_t *_raddr_8_t = RT_NULL;

  if ((pos + size) > HW_STM32_FLASH_SIZE) {
    LOG_W("stm32_flash_read() => (pos + size) > HW_STM32_FLASH_SIZE");
    goto POS_SIZE_ERR;
  }
  if ((pos % 4) != 0) {
    LOG_E("%s line:%d  pos % 4 != 0", __FILE__, __LINE__);
  }

  for (; _cycle > 0; _cycle--) {
    *(_rbuf_t++) = *(_raddr_t++);
  }

  _rbuf_8_t = (rt_uint8_t *)_rbuf_t;
  _raddr_8_t = (rt_uint8_t *)_raddr_t;
  for (; _cycle_index > 0; _cycle_index--) {
    *(_rbuf_8_t++) = *(_raddr_8_t++);
  }

  return size;
POS_SIZE_ERR:
  return 0;
}
rt_size_t stm32_flash_write(rt_nvm_device_t *nvm, rt_off_t pos,
                            const rt_uint8_t *buf, size_t size) {
  FLASH_Status _status;
  rt_uint32_t _cycle = size / 4;
  rt_uint32_t *_wbuf_t = (rt_uint32_t *)buf;
  rt_uint32_t _waddr_t = (rt_uint32_t)(HW_STM32_FLASH_START_ADDR + pos);

  if ((pos + size) > HW_STM32_FLASH_SIZE) {
    LOG_W("stm32_flash_write() => (pos + size) > HW_STM32_FLASH_SIZE");
    goto POS_SIZE_ERR;
  }
  if ((pos % 4) != 0) {
    LOG_E("%s line:%d  pos % 4 != 0", __FILE__, __LINE__);
  }
  if ((size % 4) != 0) {
    LOG_E("%s line:%d  size % 4 != 0", __FILE__, __LINE__);
  }

  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  for (rt_uint32_t _i = 0; _i < _cycle; _i++) {
    _status = FLASH_ProgramWord((uint32_t)(_waddr_t + 4 * _i), *(_wbuf_t + _i));
    if (_status != FLASH_COMPLETE) {
      FLASH_Lock();
      goto FLASH_COMPLETE_ERR;
    }
  }
  FLASH_Lock();
  return size;

POS_SIZE_ERR:
FLASH_COMPLETE_ERR:
  return 0;
}

struct rt_nvm_device nvm_item;
struct rt_nvm_ops nvm_ops = {
    .init = stm32_flash_init,
    .control = stm32_flash_control,
    .nvm_read = stm32_flash_read,
    .nvm_write = stm32_flash_write,
};

int flash_init(void) {
  LOG_I("register flash to nvm device_drivers");
  rt_err_t _rt = RT_EOK;

  nvm_item.ops = &nvm_ops;

  _rt = rt_hw_nvm_register(&nvm_item, FLASH_DEVICE_NAME, RT_DEVICE_FLAG_RDWR,
                           RT_NULL);
  if (RT_EOK != _rt) goto REG_NVM_FAIL;

  return _rt;

REG_NVM_FAIL:
  LOG_E("%s line:%d register flash to nvm device_drivers fail", __FILE__,
        __LINE__);
  return _rt;
}

INIT_BOARD_EXPORT(flash_init);

#ifdef UNIT_TEST
static rt_uint8_t _buf[200];
static rt_uint8_t _rbuf[6];

void chip_flash_test(void) {
  for (rt_int32_t _i = 0; _i < 200; _i++) {
    _buf[_i] = _i;
  }
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
  FLASH_Lock();
  if (stm32_flash_write(RT_NULL, 0, _buf, 4) != 4) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 8, _buf, 200) != 200) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_read(RT_NULL,4, _rbuf, 6) != 6) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if ((_rbuf[0] != 0xff) || (_rbuf[1] != 0xff) || (_rbuf[2] != 0xff) ||
      (_rbuf[3] != 0xff) || (_rbuf[4] != 0) || (_rbuf[5] != 1)) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
	LOG_I("test ok");
  return;
}

MSH_CMD_EXPORT(chip_flash_test, test chip flash);
#endif

#endif

#include "drv_flash.h"
/*
 * 使用stm32f20x片上flash的sector10(128k) 作为nvm驱动操作的内存地址
 * 使用stm32f20x片上flash的sector11(128k) 作为nvm驱动操作擦除时的缓存地址
 *
 * 用户使用drv_flash，可以像使用ram一样读写。
 * 不用考略flash擦除和flash是否写失败，但是要注意flash寿命问题
 */

#ifdef USE_DRV_FLASH

#define HW_NVM_ADDR (0x080C0000)
#define HW_NVM_SECTOR FLASH_Sector_10
#define HW_NVM_BACKUPS_ADDR (0x080E0000)
#define HW_NVM_BACKUPS_SECTOR FLASH_Sector_11
#define HW_NVM_SIZE (0x00020000)

#define LOG_TAG "drv_st_flash"
#define LOG_LVL LOG_LVL_DBG
#include <ulog.h>

rt_err_t stm32_flash_init(rt_nvm_device_t *nvm) {
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  FLASH_Lock();
  return RT_EOK;
}

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
                      FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR |
                      FLASH_FLAG_PGSERR);
      _status = FLASH_EraseSector(HW_NVM_SECTOR, VoltageRange_3);
      if (FLASH_COMPLETE != _status) {
        _rt = RT_EIO;
        LOG_E("%s line:%d  FLASH_EraseSector fail", __FILE__, __LINE__);
      }
      _status = FLASH_EraseSector(HW_NVM_BACKUPS_SECTOR, VoltageRange_3);
      if (FLASH_COMPLETE != _status) {
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
                           rt_size_t size) {
  rt_uint8_t *_raddr_t = (rt_uint8_t *)(HW_NVM_ADDR + pos);

  if ((pos + size) > HW_NVM_SIZE) {
    LOG_E(
        "%s line:%d  stm32_flash_read(.pos = %d .size = %d) out of range "
        "flash ",
        __FILE__, __LINE__, pos, size);
    goto __out_range__;
  }

  // FIXME：32位单片机，使用32位指针读，更快
  for (rt_uint32_t _i = 0; _i < size; _i++) {
    *(buf + _i) = *(_raddr_t + _i);
  }
  return size;

__out_range__:
  return 0;
}

//擦除HW_NVM_SECTOR之前,对其备份
//目的：减少对ram的开销
static FLASH_Status _backups_sector(void) {
  FLASH_Status _status;
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  _status = FLASH_EraseSector(HW_NVM_BACKUPS_SECTOR, VoltageRange_3);
  if (FLASH_COMPLETE != _status) {
    LOG_E("%s line:%d  Erase HW_NVM_BACKUPS_SECTOR fail", __FILE__, __LINE__);
    goto __erase_fail__;
  }
  for (rt_uint32_t _i = 0; (_i * 4) < HW_NVM_SIZE; _i++) {
    _status = FLASH_ProgramWord((uint32_t)(HW_NVM_BACKUPS_ADDR + _i * 4),
                                *((uint32_t *)(HW_NVM_ADDR + _i * 4)));
    if (_status != FLASH_COMPLETE) {
      LOG_E("%s line:%d BACKUPS HW_NVM_BACKUPS_SECTOR fail", __FILE__,
            __LINE__);
      goto __program_fail__;
    }
  }
  FLASH_Lock();
  return _status;

__erase_fail__:
  FLASH_Lock();
  return _status;
__program_fail__:
  FLASH_Lock();
  return _status;
}

static rt_bool_t _flash_write_allow(rt_off_t pos, rt_size_t size) {
  rt_uint8_t *_raddr_t = (rt_uint8_t *)(HW_NVM_ADDR + pos);
  rt_uint8_t _temp = 0xff;
  for (rt_uint32_t _i = 0; _i < size; _i++) {
    _temp &= *(_raddr_t + _i);
  }
  return _temp == 0xff ? RT_TRUE : RT_FALSE;
}

rt_size_t stm32_flash_write(rt_nvm_device_t *nvm, rt_off_t pos,
                            const rt_uint8_t *buf, rt_size_t size) {
  FLASH_Status _status;
  rt_uint8_t _temp = RT_NULL;

  if (pos < 0) {
    LOG_E(
        "%s line:%d stm32_flash_write(.pos = %d .size = %d) out of range "
        "flash ",
        __FILE__, __LINE__, pos, size);
    goto __out_range__;
  }
  if ((pos + size) >= HW_NVM_SIZE) {
    LOG_E(
        "%s line:%d stm32_flash_write(.pos = %d .size = %d) out of range "
        "flash ",
        __FILE__, __LINE__, pos, size);
    goto __out_range__;
  }
  if (size == 0) {
    LOG_E(
        "%s line:%d stm32_flash_write(.pos = %d .size = %d) out of range "
        "flash ",
        __FILE__, __LINE__, pos, size);
    goto __out_range__;
  }

  // 判断是否需要备份
  if (RT_FALSE == _flash_write_allow(pos, size)) {
    //备份数据块及准备数据块
    _status = _backups_sector();
    if (FLASH_COMPLETE != _status) {
      goto __backups_sector_fail__;
    }
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    _status = FLASH_EraseSector(HW_NVM_SECTOR, VoltageRange_3);
    if (FLASH_COMPLETE != _status) {
      LOG_E("%s line:%d  Erase HW_NVM_SECTOR fail", __FILE__, __LINE__);
      goto __erase_fail__;
    }
    for (rt_uint32_t _i = 0; _i < HW_NVM_SIZE; _i++) {
      if ((_i >= pos) && (_i < (pos + size))) {
        _temp = *(buf + _i - pos);
      } else {
        _temp = *((rt_uint8_t *)(HW_NVM_BACKUPS_ADDR + _i));
      }

      _status = FLASH_ProgramByte((uint32_t)(HW_NVM_ADDR + _i), _temp);
      if (_status != FLASH_COMPLETE) {
        LOG_E("%s line:%d  FLASH_ProgramByte(.addr=%d .data = %d) return %d",
              __FILE__, __LINE__, (uint32_t)(HW_NVM_ADDR + _i), _temp, _status);
        goto __complete_err__;
      }
    }
    FLASH_Lock();
  } else {
    //写数据到HW_NVM_SECTOR
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                    FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
    for (rt_uint32_t _i = 0; _i < size; _i++) {
      _status =
          FLASH_ProgramByte((uint32_t)(HW_NVM_ADDR + pos + _i), *(buf + _i));
      if (_status != FLASH_COMPLETE) {
        LOG_E("%s line:%d  FLASH_ProgramByte(.addr=%d .data = %d) fail",
              __FILE__, __LINE__, (uint32_t)(HW_NVM_ADDR + pos + _i),
              *(buf + _i));
        goto __complete_err__;
      }
    }
    FLASH_Lock();
  }

  return size;

__out_range__:
  return 0;
__backups_sector_fail__:
  return 0;
__erase_fail__:
  FLASH_Lock();
  return 0;
__complete_err__:
  FLASH_Lock();
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
  rt_err_t _rt = RT_EOK;

  nvm_item.ops = &nvm_ops;

  _rt = rt_hw_nvm_register(&nvm_item, FLASH_DEVICE_NAME, RT_DEVICE_FLAG_RDWR,
                           RT_NULL);
  RT_ASSERT(RT_EOK == _rt);
  return _rt;
}

INIT_BOARD_EXPORT(flash_init);

#ifdef UNIT_TEST
#define BUF_SIZE (16 * 1024)
static rt_uint8_t _buf[BUF_SIZE];
static rt_uint8_t _rbuf[BUF_SIZE];

// TODO flash test fail
void chip_flash_test(void) {
  for (rt_int32_t _i = 0; _i < BUF_SIZE; _i++) {
    _buf[_i] = _i;
  }
  FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

  if (FLASH_COMPLETE != FLASH_EraseSector(HW_NVM_SECTOR, VoltageRange_3)) {
    LOG_E("%s line:%d  Erase HW_NVM_BACKUPS_SECTOR fail", __FILE__, __LINE__);
    goto __erase_fail__;
  }
  FLASH_Lock();

  if (stm32_flash_write(RT_NULL, 1, _buf, 3) != 3) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 2, _buf, 4) != 4) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 3, _buf, 5) != 5) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 4, _buf, 6) != 6) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 5, _buf, 7) != 7) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
  if (stm32_flash_write(RT_NULL, 6, _buf, 8) != 8) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }

  for (rt_uint8_t _i = 0; _i < 9; _i++) {
    if (stm32_flash_write(RT_NULL, BUF_SIZE * (rt_uint32_t)_i, _buf,
                          BUF_SIZE) != BUF_SIZE) {
      LOG_E("%s line:%d ", __FILE__, __LINE__);
    }
  }

  if (stm32_flash_read(RT_NULL, 0, _rbuf, BUF_SIZE) != BUF_SIZE) {
    LOG_E("%s line:%d ", __FILE__, __LINE__);
  }
__erase_fail__:
  LOG_I("test ok");
  return;
}

MSH_CMD_EXPORT(chip_flash_test, test chip flash);
#endif

#endif

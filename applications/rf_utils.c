#include "rtdevice.h"
#include "rtthread.h"
#include "string.h"

#ifdef USE_APP_RF

static void _rf_help(void) {
  rt_kprintf("\rrf cmd:");
  rt_kprintf("\r\n rf stat => printf rf status");
  rt_kprintf("\r\n rf config channel 6");
  rt_kprintf("\r\n ");
}
static void _rf_stat(void) {
  // TODO
}
static rt_err_t _rf_config(char *type, char *param) {
  // TODO

  return RT_EOK;
}
// rf module debug cmd
int rf(int argc, char **argv) {
  rt_err_t _rt = RT_EOK;
  // TODO
  switch (argc) {
    case 1:  //打印rf状态参数
      _rf_help();
      break;
    case 2:
      if (!strcmp(argv[1], "help")) {
        _rf_help();
      } else if (!strcmp(argv[1], "stat")) {
        _rf_stat();
      } else {
        goto _exit;
      }
    case 4:
      if (!strcmp(argv[1], "config")) {
        _rt = _rf_config(argv[2], argv[3]);
        if (RT_EOK != _rt) {
          goto _exit;
        }
      } else {
        goto _exit;
      }
    default:
      goto _exit;
  }
  return 0;

_exit:
  rt_kprintf("Invalid parameter\r\n");
  return 0;
}

MSH_CMD_EXPORT(rf, e.g : rf help);

#endif

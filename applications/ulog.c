#define LOG_TAG           "ulog_test"
//#define LOG_LVL          LOG_LVL_DBG
#include <ulog.h>

void ulog_test(void) {
	LOG_E("open error level");
	LOG_W("open  warning level");
	LOG_I("open  info level");
	LOG_D("open  debug level");
	
#if (ULOG_OUTPUT_LVL == LOG_LVL_ERROR)
	LOG_E("ULOG_OUTPUT_LVL = error level");
#elif (ULOG_OUTPUT_LVL == LOG_LVL_WARNING)
	LOG_W("ULOG_OUTPUT_LVL = warning level");
#elif (ULOG_OUTPUT_LVL == LOG_LVL_INFO)
	LOG_I("ULOG_OUTPUT_LVL = info level");
#elif (ULOG_OUTPUT_LVL == LOG_LVL_DBG)
	LOG_D("ULOG_OUTPUT_LVL = debug level");
#endif

}

MSH_CMD_EXPORT(ulog_test, test ulog output level);

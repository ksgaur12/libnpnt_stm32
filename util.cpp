
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

#include "usbcfg.h"
#include "def.h"


void debug(const char *fmt, ...){
#if USE_DEBUG
	chprintf((BaseSequentialStream *)&SDU1, "%s", "logger: ");
	va_list ap;
	va_start(ap, fmt);
	chvprintf((BaseSequentialStream *)&SDU1, fmt, ap);
	va_end(ap);

	chprintf((BaseSequentialStream *)&SDU1, "%s", "\r\n");

#else
	(void)fmt;
#endif
}

void delay(uint32_t _time){
	chThdSleepMilliseconds(_time);
}

uint32_t millis(void)
{
	systime_t now = chTimeI2MS(chVTGetSystemTime());
	return now;
}

uint32_t micros(void)
{
    systime_t now = chTimeI2US(chVTGetSystemTime());
    return now;
}

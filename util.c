
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include "usbcfg.h"
#include "def.h"
static char BUFFER[100];

void get_rx_num(){
	return 0;//sdReadTimeout(&SDU1, BUFFER, 64, MS2ST(10));
}

void debug(const char *fmt, ...){
#if USE_DEBUG
	chprintf((BaseSequentialStream *)&SERIAL_LINE, "%s", "");
	va_list ap;
	va_start(ap, fmt);
	chvprintf((BaseSequentialStream *)&SERIAL_LINE, fmt, ap);
	va_end(ap);
	chprintf((BaseSequentialStream *)&SERIAL_LINE, "\n");

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

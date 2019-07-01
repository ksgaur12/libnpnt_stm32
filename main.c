#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "util.h"

#include "def.h"
#include "crypto.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

extern enum state current_state;

static SerialConfig Serial_config = {
	115200,
	0,
	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
	0
};

uint8_t count=0;
char buf[255];

static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {

	(void)arg;
	chRegSetThreadName("blinker1");
	while (true) {
		debug("heartbeat %d", current_state);
		palTogglePad(GPIOC, 1);
//		uint64_t gps_time = 1561790900297000;
//
//		time_t seconds = (time_t)(gps_time/1000000) + 5*3600 + 30 * 60;
//		struct tm tm = *localtime(&seconds);
//	    debug("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		//		memset(&tm, 0, sizeof(struct tm));
//		strptime("1360440555", "%s", &tm);
//		strftime(buf, sizeof(buf), "%b %d %H:%M %Y", &tm);
//		puts(buf); /* -> Feb 09 20:09 2013 */
//		debug("%s", buf);
		chThdSleepMilliseconds(5000);
	}
}

int main(void) {

	halInit();
	chSysInit();
	sdStart(&SERIAL_LINE, &Serial_config);
	delay(1000);

	chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO+1, Thread1, NULL);

	crypto_init();

	while (true) {
		delay(1000);
	}

	return 0;
}

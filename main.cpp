#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "util.h"
//#include "ublox_driver.h"
//#include "logging.h"
//
//#include "trig.h"

//#include "can_header.h"

uint32_t last_gps_update_ms, gps_last_message_time_ms;

//static SerialConfig ublox_config = {
//    230400,
//	0,
//	USART_CR2_STOP1_BITS | USART_CR2_LINEN,
//	0
//};

//static THD_WORKING_AREA(waThread1, 128);
//static THD_FUNCTION(Thread1, arg) {
//
//	(void)arg;
//	chRegSetThreadName("blinker1");
//	while (true) {
//	palTogglePad(GPIOC, 1);
//	}
//}

int main(void) {

	halInit();
	chSysInit();

//	start_usb_serial();
//
//	debug("\n\n\n Start");
//	sdStart(&GPS_PORT, &ublox_config);
//
//
//	chThdCreateStatic(waThread1, sizeof(waThread1),
//					NORMALPRIO, Thread1, NULL);
//
//
//	last_gps_update_ms = millis();
//	gps_last_message_time_ms = last_gps_update_ms;
//
//	start_sdc_();
//	delay(1000);
//
//#if !IS_BASE
//	can_start();
//	delay(1000);
//
//    chThdCreateStatic(wa_can_thd, sizeof(wa_can_thd),
//                    NORMALPRIO, can_thd, NULL);
//
//#endif
//
//	start_ublox();
//
//	delay(1000);
//
//	start_trig();

	while (true) {
//		palTogglePad(GPIOC, 3);

		delay(500);
	}
}

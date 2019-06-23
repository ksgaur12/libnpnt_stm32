#include "ch.h"
#include "hal.h"

#include "usbcfg.h"
#include "util.h"

#include "def.h"
#include "crypto.h"



uint8_t count=0;
static uint8_t rxbuf[1024 + 1];


static THD_WORKING_AREA(waThread1, 128);
static THD_FUNCTION(Thread1, arg) {


	(void)arg;
	chRegSetThreadName("blinker1");
	while (true) {
		palTogglePad(GPIOC, 1);
		//debug("Generate Key Pair %d", count++);
		usbReceive(&USBD1, USBD1_DATA_AVAILABLE_EP, rxbuf, sizeof (rxbuf));


		delay(1000);
	}
}

int main(void) {

	halInit();
	chSysInit();

	start_usb_serial();

	chThdCreateStatic(waThread1, sizeof(waThread1),
					NORMALPRIO+1, Thread1, NULL);

	ecc_init_keypair();

	while (true) {
		delay(1000);
	}

	return 0;
}

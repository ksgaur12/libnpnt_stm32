#ifndef CRYPTO_H_
#define CRYPTO_H_
#include <stdio.h>
#include <string.h>
void ecc_init_keypair(void);

enum state{
	IDLE = 0,
	KEY_ROTATION = 1,
	KEY_REQUEST = 2,
	SIGN_VERIFY = 3,
	VERIFY_DATA = 4,
	ASK_PERMISSION = 5,
	GET_CERT = 6,
	SEND_LOG = 7,
	ERROR_PA = 8,
	SUCCESS_PA = 9,
};

struct _time{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};

struct coordinate{
	float lat;
    float lon;
};

struct point{
	float x;
	float y;
};

#endif /* CRYPTO_H_ */

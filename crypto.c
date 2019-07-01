#include "crypto.h"

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/types.h>
#include <wolfssl/wolfcrypt/sha.h>
#include <wolfssl/wolfcrypt/sha256.h>
#include <wolfssl/internal.h>
#include <wolfssl/ssl.h>

#include "def.h"

#define FOURK_BUF 4096
// scaling factor from 1e-7 degrees to meters at equator
// == 1.0e-7 * DEG_TO_RAD * RADIUS_OF_EARTH
#define LOCATION_SCALING_FACTOR 0.011131884502145034f
#define M_PI      (3.141592653589793f)
#define DEG_TO_RAD      (M_PI / 180.0f)

byte  der[FOURK_BUF];
byte der_pub[FOURK_BUF];
byte pem[FOURK_BUF];
byte cert[FOURK_BUF];
byte cert_der[FOURK_BUF];
byte signature[FOURK_BUF];
byte sig_value[FOURK_BUF];
byte sig_base64[FOURK_BUF];
byte shasha[WC_SHA256_DIGEST_SIZE];

char* data;
char data_buf[FOURK_BUF];
enum state current_state = 0;
long e = 65537; // standard value to use for exponent
struct _time start_time, end_time; //the start and end time of flight from PA
struct coordinate geo_fence[16]; //store geo fence coordinates
struct point geo_fence_points[16]; //store the geo fence in North east coordinates
unsigned char pem_pub[FOURK_BUF];//store the pub key from GCS
int fence_count = 0; // no of geo fence points

//void get_dgca_public_key(void){
//	//int derSz = wc_DerToPem(cert, sizeof(cert), cert_der, FOURK_BUF);
//	int derSz = wolfSSL_CertPemToDer(cert, sizeof(cert), cert_der, sizeof(cert_der),CERT_TYPE);
//}

int polygon_outside(struct point* V, struct point P, int n){
    int i,j;
    int outside = 1;
     for (i = 0, j = n-1; i < n; j = i++) {
        if ((V[i].y > P.y) == (V[j].y > P.y)) {
            continue;
        }
        const int32_t dx1 = P.x - V[i].x;
        const int32_t dx2 = V[j].x - V[i].x;
        const int32_t dy1 = P.y - V[i].y;
        const int32_t dy2 = V[j].y - V[i].y;
        const int8_t dx1s = (dx1 < 0) ? -1 : 1;
        const int8_t dx2s = (dx2 < 0) ? -1 : 1;
        const int8_t dy1s = (dy1 < 0) ? -1 : 1;
        const int8_t dy2s = (dy2 < 0) ? -1 : 1;
        const int8_t m1 = dx1s * dy2s;
        const int8_t m2 = dx2s * dy1s;
        // we avoid the 64 bit multiplies if we can based on sign checks.
        if (dy2 < 0) {
            if (m1 > m2) {
                outside = !outside;
            } else if (m1 < m2) {
                continue;
            } else if ( dx1 * (int64_t)dy2 > dx2 * (int64_t)dy1 ) {
                outside = !outside;
            }
        } else {
            if (m1 < m2) {
                outside = !outside;
            } else if (m1 > m2) {
                continue;
            } else if ( dx1 * (int64_t)dy2 < dx2 * (int64_t)dy1 ) {
                outside = !outside;
            }
        }
    }
    return outside;
}

float longitude_scale(struct coordinate loc)
{
    float scale = cosf(loc.lat * 1.0e-7f * DEG_TO_RAD);
    if(scale < 0.01f){
    	return 0.01f;
    }
    else if(scale > 1.0f){
    	return 1.0f;
    }
    else{
    	return scale;
    }
}

struct point location_diff(struct coordinate loc1, struct coordinate loc2){
	struct point temp;
	temp.x = (loc2.lat - loc1.lat) * LOCATION_SCALING_FACTOR * 100;
	temp.y = (loc2.lon - loc1.lon) * LOCATION_SCALING_FACTOR * longitude_scale(loc1) * 100;
    return temp;
}

void parse_and_verify_data(void){

    char* coord_string = strstr(data_buf, "latitude=");
    char* coord = strtok(coord_string, "=\"");
    while(coord != NULL && fence_count < 16){
        coord= strtok(NULL, "=\"");
        if(coord != NULL){
            geo_fence[fence_count].lat = atof(coord);
            coord= strtok(NULL, "=\"");
            if(coord != NULL){
                coord= strtok(NULL, "=\"");
                if(coord != NULL){
                    geo_fence[fence_count].lon = atof(coord);
                    fence_count++;
                    coord= strtok(NULL, "=\"");
                }
            }
        }
    }

    if(fence_count < 4){
    	current_state = ERROR_PA;
    	return;
    }

	char *token;
	int flag = 0;
	token = strstr(data_buf, "flightStartTime");
	//get the start time
	for(int i=0; i<strlen(token); i++){
		while(token[i] != '\"' && !flag){
			i++;
		}
		flag = 1;
		i++;
		char* end = &token[i];
		char* flight_time;
		flight_time = strtok(end, "-T:");
		int j = 0;
		while(flight_time != NULL && j<6){
			switch (j){
			case 0:
				start_time.year = atoi(flight_time);
				break;
			case 1:
				start_time.month = atoi(flight_time);
				break;
			case 2:
				start_time.day = atoi(flight_time);
				break;
			case 3:
				start_time.hour = atoi(flight_time);
				break;
			case 4:
				start_time.min = atoi(flight_time);
				break;
			case 5:
				start_time.sec = atoi(flight_time);
				break;
			}
			j++;
			flight_time =strtok(NULL, "-T:");
		}
		break;
	}

	char *token1;
	flag = 0;
	token1 = strstr(data_buf, "flightEndTime");
	//get the start time
	for(int i=0; i<strlen(token1); i++){
		while(token1[i] != '\"' && !flag){
			i++;
		}
		flag = 1;
		i++;
		char* end = &token1[i];
		char* flight_time;
		flight_time = strtok(end, "-T:");
		int j = 0;
		while(flight_time != NULL && j<6){
			switch (j){
			case 0:
				end_time.year = atoi(flight_time);
				break;
			case 1:
				end_time.month = atoi(flight_time);
				break;
			case 2:
				end_time.day = atoi(flight_time);
				break;
			case 3:
				end_time.hour = atoi(flight_time);
				break;
			case 4:
				end_time.min = atoi(flight_time);
				break;
			case 5:
				end_time.sec = atoi(flight_time);
				break;
			}
			j++;
			flight_time =strtok(NULL, "-T:");
		}
		break;
	}

	if(!(end_time.year || end_time.month || end_time.day || start_time.year || start_time.month || start_time.day)){
		current_state = SUCCESS_PA;
		return;
	}

    fence_count--;  //first and last point are same;
    for(int i=0; i< fence_count; i++){
    	geo_fence_points[i] = location_diff(geo_fence[0], geo_fence[i]);
    }

    struct coordinate curr;
    curr.lat = 12.934172;
    curr.lon = 77.6098785;

    struct point curr_loc = location_diff(geo_fence[0], curr);
    int out = polygon_outside(geo_fence_points, curr_loc, fence_count);

    current_state = SUCCESS_PA;

}

void get_hash_sha256(void){
	wc_Sha256 sha;
	wc_InitSha256(&sha);

	wc_Sha256Update(&sha, signature, strlen(signature));
	wc_Sha256Final(&sha, shasha);

	chprintf((BaseSequentialStream *)&SERIAL_LINE, "%s\n", sig_value);


	RsaKey rsaKey;
	DerBuffer* converted = NULL;

	int ret = PemToDer(pem_pub, strlen(pem_pub), PUBLICKEY_TYPE, &converted, 0, NULL, NULL);

	if (ret == 0) {
		debug("%d", converted->length);
		ret = wc_InitRsaKey(&rsaKey, 0);
	}
	else{
		debug("uninit %d", ret);
	}
	if (ret == 0) {
		debug("init successful");
		word32 idx = 0;
		ret = wc_RsaPublicKeyDecode(converted->buffer, &idx, &rsaKey, converted->length);
	}
	else{
		debug("not decoded %d", ret);
	}
	uint16_t sig_base64_len = strlen(sig_value);
	debug("%s",sig_value);

	if(ret == 0){
		debug("decode success");
		ret = Base64_Decode(sig_value, strlen(sig_value), sig_base64, sig_base64_len);
	}
	else{
		debug("base64 failed");
	}

	if (ret == 0) {
		debug("%s",rsaKey.data);
		uint8_t* decSig = NULL;
		int decSigLen = wc_RsaSSL_VerifyInline(sig_base64, sig_base64_len, &decSig, &rsaKey);
		debug("%d",decSigLen);
	}
	else{
		printf("not able to verify %d", ret);
	}
	current_state = IDLE;
}

void rsa_key_generation(void){
	RsaKey rsakey;
	WC_RNG rng;
	wc_InitRng(&rng);
	wc_InitRsaKey(&rsakey, NULL);

	int ret = wc_MakeRsaKey(&rsakey, 2048, 65537, &rng);

	if (ret != 0){
		return;
	}

	int  derSz = wc_RsaKeyToDer(&rsakey, der, sizeof(der));

	int  pemSz = wc_DerToPem(der, derSz, pem, sizeof(pem), PRIVATEKEY_TYPE);

	derSz = wc_RsaKeyToPublicDer(&rsakey, der_pub, sizeof(der_pub));

	pemSz = wc_DerToPem(der_pub, derSz, pem_pub, sizeof(pem_pub), RSA_PUBLICKEY_TYPE);

	wc_FreeRng(&rng);
	wc_FreeRsaKey(&rsakey);
}

static THD_WORKING_AREA(waThread_crypto, 8000);
static THD_FUNCTION(Thread_crypto, arg) {


	(void)arg;
	chRegSetThreadName("crypto");

	while (true) {
		switch(current_state){
		case IDLE:
			break;
		case KEY_ROTATION:
			rsa_key_generation();
			current_state = IDLE;
			break;
		case SIGN_VERIFY:
			get_hash_sha256();
			current_state = IDLE;
			break;
		case VERIFY_DATA:
			fence_count = 0;
			parse_and_verify_data();
			break;
		default:
			break;
		}
		//debug("rx_num : %d", rx_num);
		//rsa_key_generation();
		delay(100);
	}
}

static THD_WORKING_AREA(waReader, 128);
static THD_FUNCTION(Reader, arg) {

	(void)arg;
	chRegSetThreadName("reader"); //thread for reading the data from host
	uint16_t j=0;
	int rx = 0;
	char type;

	while (true) {
		uint16_t rx_num = qSpaceI(&(&SERIAL_LINE)->iqueue);
		if(rx_num > 0){
			for(uint16_t i=0; i < rx_num; i++){
				uint8_t data = sdGetTimeout(&SERIAL_LINE, 5);
				if(data == '*' && rx){
					rx = 0;
					j = 0;
					if(type == '3'){
						//current_state = SIGN_VERIFY;
					}
					else if(type == '4'){
						current_state = VERIFY_DATA;
					}
				}
				else if(data == '*' && !rx){
					type = sdGetTimeout(&SERIAL_LINE, 5);
					rx = 1;
				}
				else if(rx && type == '1'){
					pem_pub[j] = data;
					j++;
				}
				else if(rx && type == '2'){
					signature[j] = data;
					j++;
				}
				else if(rx && type == '3'){
					sig_value[j] = data;
					j++;
				}
				else if(rx && type == '4'){
					data_buf[j] = data;
					j++;
				}
			}
		}
		chThdSleepMilliseconds(100);
	}
}

void crypto_init(void){

	chThdCreateStatic(waThread_crypto, sizeof(waThread_crypto),NORMALPRIO, Thread_crypto, NULL);
	chThdCreateStatic(waReader, sizeof(waReader), NORMALPRIO, Reader, NULL);
}


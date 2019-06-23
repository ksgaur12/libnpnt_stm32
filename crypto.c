#include "crypto.h"

#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/ecc.h>
#include <wolfssl/wolfcrypt/rsa.h>
#include <wolfssl/wolfcrypt/asn_public.h>
#include <wolfssl/wolfcrypt/types.h>

#include "def.h"

#define FOURK_BUF 4096
byte  der[FOURK_BUF];
byte der_pub[FOURK_BUF];
byte pem[FOURK_BUF];
byte pem_pub[FOURK_BUF];

long e = 65537; // standard value to use for exponent


void ecc_key_generation(void){
//	ecc_key key;
//	WC_RNG rng;
//    Cert req;
//
//	debug("key initalize");
//	wc_ecc_init(&key);
//	wc_InitRng(&rng);
//
//	wc_ecc_make_key_ex(&rng, 32, &key, ECC_SECP256R1); // initialize 32 byte ecc key
//
//
//	int ret = wc_EccKeyToDer(&key, der, sizeof(der));
//	if (ret <= 0) {
//		debug("ECC Key To DER failed: %d\n", ret);
//		return;
//	}
//	int derSz = ret;
//
//	ret = wc_DerToPem(der, derSz, pem, sizeof(pem), ECC_PRIVATEKEY_TYPE);
//	if (ret <= 0) {
//		debug("DER to PEM failed: %d\n", ret);
//		return;
//	}
//	int pemSz = ret;
//	debug("%s (%d)", pem, pemSz);
//
//    ret = wc_EccPublicKeyToDer(&key, der, sizeof(der), ECC_SECP256R1);
//    if (ret < 0) {
//        debug("wc_EccPublicKeyToDer error %s (%d)\n", wc_GetErrorString(ret), ret);
//        return;
//    }
//    derSz = ret;
//
//    ret = wc_DerToPem(der, derSz, pem, sizeof(pem), PUBLICKEY_TYPE);
//    if (ret < 0) {
//        debug("wc_DerToPem error %s (%d)\n", wc_GetErrorString(ret), ret);
//        return;
//    }
//    pemSz = ret;
//	debug("%s (%d)", pem, pemSz);


//	ret = wc_InitCert(&req);
//	if (ret != 0) {
//		debug("Init Cert failed: %d\n", ret);
//		return;
//	}
//	ret = wc_MakeCertReq(&req, der, sizeof(der), NULL, &key);
//	if (ret <= 0) {
//		debug("Make Cert Req failed: %d\n", ret);
//		return;
//	}
//	derSz = ret;
//
//	req.sigType = CTC_SHA256wECDSA;
//	ret = wc_SignCert(req.bodySz, req.sigType, der, sizeof(der), NULL, &key, &rng);
//	if (ret <= 0) {
//		debug("Sign Cert failed: %d\n", ret);
//		return;
//	}
//	derSz = ret;
//
//	ret = wc_DerToPem(der, derSz, pem, sizeof(pem), CERTREQ_TYPE);
//	if (ret <= 0) {
//		debug("DER to PEM failed: %d\n", ret);
//		return;
//	}
//	pemSz = ret;
//	debug("%s", pem);
}

void rsa_key_generation(void){
	debug("Memory allocated\n");
	RsaKey rsakey;
	WC_RNG rng;
	wc_InitRng(&rng);
	debug("Ring initialized\n");
	wc_InitRsaKey(&rsakey, NULL);
	debug("RSA key initialized\n");

	int ret = wc_MakeRsaKey(&rsakey, 3000, 65537, &rng);

	if (ret != 0){
		debug("Error creating RSA key\n");
		return;
	}
	else
		debug("Successfully created the RSA key\n");

	int  derSz = wc_RsaKeyToDer(&rsakey, der, sizeof(der));

	debug("der Size : %d", derSz);

	int  pemSz = wc_DerToPem(der, derSz, pem, sizeof(pem), PRIVATEKEY_TYPE);

	debug("pem Size : %d", pemSz);
	debug("%s", pem);

	derSz = wc_RsaKeyToPublicDer(&rsakey, der_pub, sizeof(der_pub));
	printf("%d\n %s", derSz);

	pemSz = wc_DerToPem(der_pub, derSz, pem_pub, sizeof(pem_pub), PRIVATEKEY_TYPE);
	debug("%d \n %s",pemSz, pem_pub);

	wc_FreeRng(&rng);
	wc_FreeRsaKey(&rsakey);
}

static THD_WORKING_AREA(waThread_crypto, 8000);
static THD_FUNCTION(Thread_crypto, arg) {


	(void)arg;
	chRegSetThreadName("crypto");

	while (true) {
	    int rx_num;// = get_rx_num();
		if(rx_num > 0){
			//debug("rx_num : %d", rx_num);
			//rsa_key_generation();
		}
		delay(100);
	}
}


void ecc_init_keypair(void){

	chThdCreateStatic(waThread_crypto, sizeof(waThread_crypto),
			NORMALPRIO, Thread_crypto, NULL);

}


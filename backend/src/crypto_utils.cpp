#include "crypto_utils.h"
#include <openssl/evp.h>
#include <openssl/kdf.h>
#include <openssl/hmac.h>
#include <iostream>

using namespace std;

bool derive_srtp_key(const uint8_t* kyber_secret, uint8_t* srtp_key) {
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL);
    if (pctx == NULL) {
        cerr << "Failed to create HKDF context" << endl;
        return false;
    }

    if (EVP_PKEY_derive_init(pctx) <= 0) {
        cerr << "HKDF init failed" << endl;
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    if (EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) <= 0) {
        cerr << "HKDF set md failed" << endl;
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    if (EVP_PKEY_CTX_set1_hkdf_key(pctx, kyber_secret, 32) <= 0) {
        cerr << "HKDF set key failed" << endl;
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    const unsigned char info[] = "SRTP-AES256-SALT";
    if (EVP_PKEY_CTX_add1_hkdf_info(pctx, info, sizeof(info) - 1) <= 0) {
        cerr << "HKDF add info failed" << endl;
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    size_t outlen = 46;
    if (EVP_PKEY_derive(pctx, srtp_key, &outlen) <= 0) {
        cerr << "HKDF derive failed" << endl;
        EVP_PKEY_CTX_free(pctx);
        return false;
    }

    EVP_PKEY_CTX_free(pctx);
    return true;
}

vector<uint8_t> compute_hmac_sha512(const vector<uint8_t>& key, const vector<uint8_t>& data) {
    unsigned char hmac_result[EVP_MAX_MD_SIZE];
    unsigned int hmac_len;
    
    HMAC(EVP_sha512(), key.data(), key.size(), data.data(), data.size(), hmac_result, &hmac_len);
    
    return vector<uint8_t>(hmac_result, hmac_result + hmac_len);
}

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <vector>
#include <cstdint>

// Derive 46-byte SRTP key from 32-byte Kyber shared secret using HKDF
bool derive_srtp_key(const uint8_t* kyber_secret, uint8_t* srtp_key);

// Compute HMAC-SHA512
std::vector<uint8_t> compute_hmac_sha512(const std::vector<uint8_t>& key, 
                                         const std::vector<uint8_t>& data);

#endif // CRYPTO_UTILS_H

#ifndef AUTH_PROTOCOL_H
#define AUTH_PROTOCOL_H

#include <string>
#include <vector>
#include <cstdint>

// Global SRTP key (46 bytes: 32-byte key + 14-byte salt)
extern std::vector<uint8_t> SRTP_KEY;

// Message types for authenticated key exchange
#define MSG_HELLO 0x01
#define MSG_DILITHIUM_KEY_REQUEST 0x02
#define MSG_DILITHIUM_PUBLIC_KEY 0x03
#define MSG_KYBER_KEY_REQUEST 0x04
#define MSG_KYBER_PUBLIC_KEY_SIGNED 0x05
#define MSG_ENCRYPTED_SECRET 0x06
#define MSG_HMAC_TAG 0x07
#define MSG_HMAC_VERIFY_SUCCESS 0x08
#define MSG_HMAC_VERIFY_FAILURE 0x09

// Server-side authenticated key exchange
bool server_perform_authenticated_key_exchange(int key_exchange_port, 
                                               std::string& client_username);

// Client-side authenticated key exchange
bool client_perform_authenticated_key_exchange(const char* server_ip, 
                                               int key_exchange_port, 
                                               const std::string& username);

#endif // AUTH_PROTOCOL_H

#include "auth_protocol.h"
#include "crypto_utils.h"
#include <iostream>
#include <fstream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <oqs/oqs.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;

// Global SRTP key
vector<uint8_t> SRTP_KEY;

const string CLIENT_DB_FILE = "client_keys.json";
const string CLIENT_KEYS_FILE = "client_dilithium_keys.bin";

// Dilithium keys structure
struct DilithiumKeys {
    vector<uint8_t> public_key;
    vector<uint8_t> secret_key;
};

// Helper functions
static json load_client_db() {
    ifstream file(CLIENT_DB_FILE);
    if (!file.is_open()) {
        return json::object();
    }
    json db;
    file >> db;
    return db;
}

static void save_client_db(const json& db) {
    ofstream file(CLIENT_DB_FILE);
    file << db.dump(4);
}

static bool get_client_dilithium_key(const string& username, vector<uint8_t>& public_key) {
    json db = load_client_db();
    if (db.contains(username)) {
        auto key_json = db[username]["dilithium_public_key"];
        public_key = vector<uint8_t>(key_json.begin(), key_json.end());
        return true;
    }
    return false;
}

static void store_client_dilithium_key(const string& username, const vector<uint8_t>& public_key) {
    json db = load_client_db();
    db[username]["dilithium_public_key"] = public_key;
    save_client_db(db);
    cout << "SERVER: Stored Dilithium public key for user: " << username << endl;
}

static bool load_or_generate_dilithium_keys(DilithiumKeys& keys) {
    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_ml_dsa_65);
    if (!sig) {
        cerr << "Failed to initialize ML-DSA-65" << endl;
        return false;
    }
    
    ifstream file(CLIENT_KEYS_FILE, ios::binary);
    if (file.is_open()) {
        keys.public_key.resize(sig->length_public_key);
        keys.secret_key.resize(sig->length_secret_key);
        
        file.read((char*)keys.public_key.data(), sig->length_public_key);
        file.read((char*)keys.secret_key.data(), sig->length_secret_key);
        file.close();
        
        cout << "CLIENT: Loaded existing Dilithium keys" << endl;
    } else {
        keys.public_key.resize(sig->length_public_key);
        keys.secret_key.resize(sig->length_secret_key);
        
        if (OQS_SIG_keypair(sig, keys.public_key.data(), keys.secret_key.data()) != OQS_SUCCESS) {
            cerr << "CLIENT: Failed to generate Dilithium keys" << endl;
            OQS_SIG_free(sig);
            return false;
        }
        
        ofstream outfile(CLIENT_KEYS_FILE, ios::binary);
        outfile.write((char*)keys.public_key.data(), sig->length_public_key);
        outfile.write((char*)keys.secret_key.data(), sig->length_secret_key);
        outfile.close();
        
        cout << "CLIENT: Generated and saved new Dilithium keys" << endl;
    }
    
    OQS_SIG_free(sig);
    return true;
}

static bool send_message(int sock, uint8_t msg_type, const vector<uint8_t>& data) {
    uint32_t data_len = data.size();
    
    if (send(sock, &msg_type, 1, 0) != 1) return false;
    if (send(sock, &data_len, sizeof(data_len), 0) != sizeof(data_len)) return false;
    if (data_len > 0) {
        if (send(sock, data.data(), data_len, 0) != (ssize_t)data_len) return false;
    }
    
    return true;
}

static bool recv_message(int sock, uint8_t& msg_type, vector<uint8_t>& data) {
    if (recv(sock, &msg_type, 1, MSG_WAITALL) != 1) return false;
    
    uint32_t data_len;
    if (recv(sock, &data_len, sizeof(data_len), MSG_WAITALL) != sizeof(data_len)) return false;
    
    if (data_len > 0) {
        data.resize(data_len);
        if (recv(sock, data.data(), data_len, MSG_WAITALL) != (ssize_t)data_len) return false;
    }
    
    return true;
}

// Server-side key exchange implementation
bool server_perform_authenticated_key_exchange(int key_exchange_port, string& client_username) {
    cout << "\n=== SERVER: Starting Authenticated Key Exchange ===\n" << endl;
    
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_768);
    if (!kem) {
        cerr << "Failed to initialize Kyber-768" << endl;
        return false;
    }
    
    int server_sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(key_exchange_port);
    
    int reuse = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    
    if (bind(server_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "SERVER: Bind failed" << endl;
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    listen(server_sock, 1);
    cout << "SERVER: Listening on port " << key_exchange_port << "..." << endl;
    
    int client_sock = accept(server_sock, nullptr, nullptr);
    if (client_sock < 0) {
        cerr << "SERVER: Accept failed" << endl;
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "SERVER: Client connected!" << endl;
    
    vector<uint8_t> all_messages;
    uint8_t msg_type;
    vector<uint8_t> msg_data;
    
    // 1. Receive HELLO
    if (!recv_message(client_sock, msg_type, msg_data) || msg_type != MSG_HELLO) {
        cerr << "SERVER: Invalid HELLO message" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    string username(msg_data.begin(), msg_data.end());
    client_username = username;
    cout << "SERVER: Received HELLO from: " << username << endl;
    all_messages.insert(all_messages.end(), msg_data.begin(), msg_data.end());
    
    // 2. Check/request Dilithium key
    vector<uint8_t> client_dilithium_pubkey;
    bool has_dilithium_key = get_client_dilithium_key(username, client_dilithium_pubkey);
    
    if (!has_dilithium_key) {
        cout << "SERVER: No Dilithium key found, requesting from client..." << endl;
        
        vector<uint8_t> empty;
        if (!send_message(client_sock, MSG_DILITHIUM_KEY_REQUEST, empty)) {
            cerr << "SERVER: Failed to send Dilithium key request" << endl;
            close(client_sock);
            close(server_sock);
            OQS_KEM_free(kem);
            return false;
        }
        
        if (!recv_message(client_sock, msg_type, msg_data) || msg_type != MSG_DILITHIUM_PUBLIC_KEY) {
            cerr << "SERVER: Failed to receive Dilithium public key" << endl;
            close(client_sock);
            close(server_sock);
            OQS_KEM_free(kem);
            return false;
        }
        
        client_dilithium_pubkey = msg_data;
        all_messages.insert(all_messages.end(), msg_data.begin(), msg_data.end());
        store_client_dilithium_key(username, client_dilithium_pubkey);
        
        cout << "SERVER: Received and stored Dilithium public key" << endl;
    } else {
        cout << "SERVER: Found existing Dilithium key for " << username << endl;
    }
    
    // 3. Request Kyber public key
    cout << "SERVER: Requesting Kyber public key..." << endl;
    vector<uint8_t> empty;
    if (!send_message(client_sock, MSG_KYBER_KEY_REQUEST, empty)) {
        cerr << "SERVER: Failed to send Kyber key request" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    // 4. Receive signed Kyber public key
    if (!recv_message(client_sock, msg_type, msg_data) || msg_type != MSG_KYBER_PUBLIC_KEY_SIGNED) {
        cerr << "SERVER: Failed to receive signed Kyber public key" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (msg_data.size() < kem->length_public_key) {
        cerr << "SERVER: Invalid Kyber public key size" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    vector<uint8_t> kyber_pubkey(msg_data.begin(), msg_data.begin() + kem->length_public_key);
    vector<uint8_t> signature(msg_data.begin() + kem->length_public_key, msg_data.end());
    
    cout << "SERVER: Received Kyber public key with signature" << endl;
    all_messages.insert(all_messages.end(), msg_data.begin(), msg_data.end());
    
    // 5. Verify signature
    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_ml_dsa_65);
    if (!sig) {
        cerr << "SERVER: Failed to initialize ML-DSA-65" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (OQS_SIG_verify(sig, kyber_pubkey.data(), kyber_pubkey.size(), 
                       signature.data(), signature.size(), 
                       client_dilithium_pubkey.data()) != OQS_SUCCESS) {
        cerr << "SERVER: Signature verification FAILED! Possible MITM attack!" << endl;
        close(client_sock);
        close(server_sock);
        OQS_SIG_free(sig);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "SERVER: Signature verification SUCCESS!" << endl;
    OQS_SIG_free(sig);
    
    // 6. Encapsulate shared secret
    vector<uint8_t> ciphertext(kem->length_ciphertext);
    uint8_t shared_secret[32];
    
    if (OQS_KEM_encaps(kem, ciphertext.data(), shared_secret, kyber_pubkey.data()) != OQS_SUCCESS) {
        cerr << "SERVER: Encapsulation failed" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (!send_message(client_sock, MSG_ENCRYPTED_SECRET, ciphertext)) {
        cerr << "SERVER: Failed to send encrypted secret" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    all_messages.insert(all_messages.end(), ciphertext.begin(), ciphertext.end());
    
    // 7-10. HMAC verification
    if (!recv_message(client_sock, msg_type, msg_data) || msg_type != MSG_HMAC_TAG) {
        cerr << "SERVER: Failed to receive client HMAC" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    vector<uint8_t> client_hmac = msg_data;
    vector<uint8_t> shared_secret_vec(shared_secret, shared_secret + 32);
    vector<uint8_t> server_hmac = compute_hmac_sha512(shared_secret_vec, all_messages);
    
    if (server_hmac != client_hmac) {
        cerr << "SERVER: HMAC verification FAILED!" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "SERVER: Client HMAC verification SUCCESS!" << endl;
    
    if (!send_message(client_sock, MSG_HMAC_TAG, server_hmac)) {
        cerr << "SERVER: Failed to send HMAC" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (!recv_message(client_sock, msg_type, msg_data) || msg_type != MSG_HMAC_VERIFY_SUCCESS) {
        cerr << "SERVER: Client rejected our HMAC" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "SERVER: Mutual HMAC verification complete!" << endl;
    
    // Derive SRTP key
    uint8_t srtp_key[46];
    if (!derive_srtp_key(shared_secret, srtp_key)) {
        cerr << "SERVER: SRTP key derivation failed!" << endl;
        close(client_sock);
        close(server_sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    SRTP_KEY.assign(srtp_key, srtp_key + 46);
    
    cout << "SERVER: SRTP Key established" << endl;
    
    close(client_sock);
    close(server_sock);
    OQS_KEM_free(kem);
    
    cout << "\n=== SERVER: Key Exchange Complete ===\n" << endl;
    return true;
}

// Client-side key exchange implementation
bool client_perform_authenticated_key_exchange(const char* server_ip, int key_exchange_port, 
                                               const string& username) {
    cout << "\n=== CLIENT: Starting Authenticated Key Exchange ===\n" << endl;
    
    DilithiumKeys dilithium_keys;
    if (!load_or_generate_dilithium_keys(dilithium_keys)) {
        return false;
    }
    
    OQS_KEM *kem = OQS_KEM_new(OQS_KEM_alg_kyber_768);
    if (!kem) {
        cerr << "Failed to initialize Kyber-768" << endl;
        return false;
    }
    
    vector<uint8_t> kyber_public_key(kem->length_public_key);
    vector<uint8_t> kyber_secret_key(kem->length_secret_key);
    
    if (OQS_KEM_keypair(kem, kyber_public_key.data(), kyber_secret_key.data()) != OQS_SUCCESS) {
        cerr << "CLIENT: Kyber keypair generation failed" << endl;
        OQS_KEM_free(kem);
        return false;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(key_exchange_port);
    inet_pton(AF_INET, server_ip, &addr.sin_addr);
    
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        cerr << "CLIENT: Connection failed" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "CLIENT: Connected!" << endl;
    
    vector<uint8_t> all_messages;
    
    // 1. Send HELLO
    vector<uint8_t> hello_data(username.begin(), username.end());
    if (!send_message(sock, MSG_HELLO, hello_data)) {
        cerr << "CLIENT: Failed to send HELLO" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    all_messages.insert(all_messages.end(), hello_data.begin(), hello_data.end());
    
    // 2. Check if server requests Dilithium key
    uint8_t msg_type;
    vector<uint8_t> msg_data;
    
    if (!recv_message(sock, msg_type, msg_data)) {
        cerr << "CLIENT: Failed to receive response" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (msg_type == MSG_DILITHIUM_KEY_REQUEST) {
        if (!send_message(sock, MSG_DILITHIUM_PUBLIC_KEY, dilithium_keys.public_key)) {
            cerr << "CLIENT: Failed to send Dilithium public key" << endl;
            close(sock);
            OQS_KEM_free(kem);
            return false;
        }
        all_messages.insert(all_messages.end(), dilithium_keys.public_key.begin(), 
                           dilithium_keys.public_key.end());
        
        if (!recv_message(sock, msg_type, msg_data)) {
            cerr << "CLIENT: Failed to receive Kyber key request" << endl;
            close(sock);
            OQS_KEM_free(kem);
            return false;
        }
    }
    
    if (msg_type != MSG_KYBER_KEY_REQUEST) {
        cerr << "CLIENT: Expected Kyber key request" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    // 3. Sign Kyber public key
    OQS_SIG *sig = OQS_SIG_new(OQS_SIG_alg_ml_dsa_65);
    if (!sig) {
        cerr << "CLIENT: Failed to initialize ML-DSA-65 for signing" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    vector<uint8_t> signature(sig->length_signature);
    size_t sig_len;
    
    if (OQS_SIG_sign(sig, signature.data(), &sig_len, kyber_public_key.data(), 
                     kyber_public_key.size(), dilithium_keys.secret_key.data()) != OQS_SUCCESS) {
        cerr << "CLIENT: Signing failed" << endl;
        close(sock);
        OQS_SIG_free(sig);
        OQS_KEM_free(kem);
        return false;
    }
    
    signature.resize(sig_len);
    OQS_SIG_free(sig);
    
    // 4. Send signed Kyber public key
    vector<uint8_t> signed_data;
    signed_data.insert(signed_data.end(), kyber_public_key.begin(), kyber_public_key.end());
    signed_data.insert(signed_data.end(), signature.begin(), signature.end());
    
    if (!send_message(sock, MSG_KYBER_PUBLIC_KEY_SIGNED, signed_data)) {
        cerr << "CLIENT: Failed to send signed Kyber public key" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    all_messages.insert(all_messages.end(), signed_data.begin(), signed_data.end());
    
    // 5. Receive encrypted secret
    if (!recv_message(sock, msg_type, msg_data) || msg_type != MSG_ENCRYPTED_SECRET) {
        cerr << "CLIENT: Failed to receive encrypted secret" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    vector<uint8_t> ciphertext = msg_data;
    all_messages.insert(all_messages.end(), ciphertext.begin(), ciphertext.end());
    
    // 6. Decapsulate
    uint8_t shared_secret[32];
    if (OQS_KEM_decaps(kem, shared_secret, ciphertext.data(), kyber_secret_key.data()) != OQS_SUCCESS) {
        cerr << "CLIENT: Decapsulation failed" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    // 7-10. HMAC verification
    vector<uint8_t> shared_secret_vec(shared_secret, shared_secret + 32);
    vector<uint8_t> client_hmac = compute_hmac_sha512(shared_secret_vec, all_messages);
    
    if (!send_message(sock, MSG_HMAC_TAG, client_hmac)) {
        cerr << "CLIENT: Failed to send HMAC" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    if (!recv_message(sock, msg_type, msg_data) || msg_type != MSG_HMAC_TAG) {
        cerr << "CLIENT: Failed to receive server HMAC" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    vector<uint8_t> server_hmac = msg_data;
    vector<uint8_t> expected_server_hmac = compute_hmac_sha512(shared_secret_vec, all_messages);
    
    if (server_hmac != expected_server_hmac) {
        cerr << "CLIENT: Server HMAC verification FAILED!" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    cout << "CLIENT: Server HMAC verification SUCCESS!" << endl;
    
    vector<uint8_t> success_msg;
    if (!send_message(sock, MSG_HMAC_VERIFY_SUCCESS, success_msg)) {
        cerr << "CLIENT: Failed to send verification success" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    // Derive SRTP key
    uint8_t srtp_key[46];
    if (!derive_srtp_key(shared_secret, srtp_key)) {
        cerr << "CLIENT: SRTP key derivation failed!" << endl;
        close(sock);
        OQS_KEM_free(kem);
        return false;
    }
    
    SRTP_KEY.assign(srtp_key, srtp_key + 46);
    
    cout << "CLIENT: SRTP Key established" << endl;
    
    close(sock);
    OQS_KEM_free(kem);
    
    cout << "\n=== CLIENT: Key Exchange Complete ===\n" << endl;
    return true;
}

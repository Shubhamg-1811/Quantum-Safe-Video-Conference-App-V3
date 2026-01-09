# 🔐 Quantum-Secure Video Conferencing System

[![NIST PQC](https://img.shields.io/badge/NIST-PQC_Standardized-blue)](https://csrc.nist.gov/projects/post-quantum-cryptography)
[![Kyber-768](https://img.shields.io/badge/Kyber--768-ML--KEM-green)](https://pq-crystals.org/kyber/)
[![Dilithium2](https://img.shields.io/badge/Dilithium2-ML--DSA-orange)](https://pq-crystals.org/dilithium/)

A **quantum-resistant** real-time video conferencing application implementing NIST-standardized post-quantum cryptography. Built with Qt6, GStreamer, and the liboqs library, this system protects against both classical and future quantum computer attacks while providing secure, low-latency video and audio communication.

> **🚀 Why Post-Quantum?** Traditional RSA and ECC encryption will be broken by quantum computers. This application implements **Kyber-768** (ML-KEM) and **Dilithium** (ML-DSA), algorithms standardized by NIST in August 2024 to resist quantum attacks.

---

## ⚠️ Security Assumption

**CRITICAL**: This implementation assumes that the **initial Dilithium public key is securely transferred** between client and server through a trusted channel (e.g., pre-shared, certificate authority, or secure out-of-band exchange).

**Without secure Dilithium key distribution**, the system is vulnerable to **Man-in-the-Middle (MITM) attacks** during the initial handshake, potentially compromising the entire session.

### Recommended Key Distribution Methods:
1. **Pre-shared keys**: Exchange Dilithium public keys offline before first use
2. **Certificate Authority (CA)**: Use a trusted CA to sign and distribute public keys
3. **QR Code/Physical exchange**: Display and scan keys during first connection
4. **Trust-on-First-Use (TOFU)**: Store and verify public key fingerprints after first connection

---

## ✨ Key Features

- 🛡️ **Post-Quantum Cryptography**: NIST-standardized **Kyber-768** (ML-KEM) for key exchange and **Dilithium2** (ML-DSA-65) for digital signatures
- 🔒 **End-to-End Encryption**: AES-256-ICM + HMAC-SHA1-80 over SRTP for media streams
- 📹 **Real-Time Video/Audio**: GStreamer-based multimedia framework with H.264 encoding and Opus audio
- 💻 **Intuitive Qt6 GUI**: Cross-platform interface with one-click connection setup
- ⚡ **High Performance**: <1ms cryptographic overhead, 30 FPS video at 640x480 resolution
- 🔐 **Quantum-Resistant**: Protected against Shor's algorithm and other quantum attacks
- 🌐 **Low Latency**: <100ms on LAN, UDP/RTP transport with jitter buffering

---

## 🏗️ System Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Qt6 GUI Frontend                        │
│    ┌──────────────┐         ┌──────────────────────┐        │
│    │ LaunchWindow │────────▶│   QProcess Manager   │        │
│    └──────────────┘         └──────────────────────┘        │
└───────────────────────────────┬─────────────────────────────┘
                                │
┌───────────────────────────────▼─────────────────────────────┐
│                    C++ Backend (GStreamer)                  │
│   ┌─────────────────────────────────────────────────────┐   │
│   │   Post-Quantum Authentication (Kyber + Dilithium)   │   │
│   └─────────────────────────────────────────────────────┘   │
│    ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│    │  Camera/Mic │ →│ H.264/Opus   │ →│ SRTP Encryption │→  │
│    │  Capture    │  │   Encoding   │  │   AES-256-ICM   │   │
│    └─────────────┘  └──────────────┘  └─────────────────┘   │
│                                                             │
│    ┌─────────────┐  ┌──────────────┐  ┌─────────────────┐   │
│    │ Video/Audio │← │  Decoding    │← │ SRTP Decryption │←  │
│    │  Playback   │  │              │  │                 │   │
│    └─────────────┘  └──────────────┘  └─────────────────┘   │
└─────────────────────────────────────────────────────────────┘
```

### Security Protocol Flow

1. **Client** initiates connection
2. **Server** responds with Kyber-768 public key
3. **Client** encapsulates shared secret using Kyber-768
4. **Client** signs encapsulated key with Dilithium2
5. **Server** verifies signature and decapsulates secret
6. **Both parties** derive SRTP keys using HKDF-SHA256
7. **Media streams** encrypted with AES-256 + HMAC

---

## 📋 Requirements

### Hardware
- **CPU**: Intel/AMD x86-64 (dual-core minimum)
- **RAM**: 4 GB minimum (8 GB recommended)
- **Storage**: 5 GB for dependencies
- **Camera**: USB webcam or built-in camera
- **Network**: Ethernet or Wi-Fi

### Software
- **OS**: Ubuntu 20.04/22.04/24.04 LTS
- **Qt Framework**: 6.2.4+
- **GStreamer**: 1.20.x+
- **OpenSSL**: 3.0.x+
- **liboqs**: 0.10.x+ (post-quantum crypto library)
- **Build Tools**: GCC/G++ (C++17), CMake, Make

---

## 🚀 Quick Start

### Installation

See [INSTALLATION.md](INSTALLATION.md) for detailed setup instructions.

```bash
# Clone repository
git clone https://github.com/Vikas2171/Quantum_Secure_Video_Conference_App_V3.git
cd Video_Conference_App_V3_Quantum_Secure

# Install dependencies (Ubuntu)
sudo apt update
sudo apt install -y qt6-base-dev libgstreamer1.0-dev libssl-dev \
  gstreamer1.0-plugins-good gstreamer1.0-plugins-bad libsrtp2-dev

# Build and install liboqs
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git
cd liboqs && mkdir build && cd build
cmake -GNinja -DCMAKE_INSTALL_PREFIX=/usr/local \
  -DOQS_ENABLE_SIG_DILITHIUM=ON -DOQS_ENABLE_KEM_KYBER=ON ..
ninja && sudo ninja install && sudo ldconfig

# Build backend
cd ../../backend
make

# Build frontend
cd ../frontend
qmake6 frontend.pro && make
```

### Running

#### Option 1: GUI Mode (Recommended)

```bash
cd frontend
./frontend
```

1. Select **Server** or **Client** mode
2. Enter peer IP address and username
3. Click **Connect** – video call starts automatically

#### Option 2: Command Line

**Server:**
```bash
cd backend
./server <client_ip>
```

**Client:**
```bash
cd backend
./client <server_ip> <username>
```

---

## 📁 Project Structure

```
Quantum_Secure_Video_Conference_App_V3/
├── README.md                    # Project overview (this file)
├── INSTALLATION.md              # Detailed installation guide
├── frontend/                    # Qt6 GUI Application
│   ├── main.cpp                 # Entry point
│   ├── launchwindow.h/cpp       # Connection UI
│   ├── frontend.pro             # Qt project file
│   └── frontend                 # Compiled executable
├── backend/                     # C++ Backend
│   ├── src/
│   │   ├── server_main.cpp      # Server implementation
│   │   ├── client_main.cpp      # Client implementation
│   │   ├── crypto_utils.cpp     # AES-256, HMAC utilities
│   │   └── auth_protocol.cpp    # Kyber + Dilithium protocol
│   ├── include/
│   │   ├── crypto_utils.h
│   │   └── auth_protocol.h
│   ├── Makefile                 # Build configuration
│   ├── server                   # Server executable
│   └── client                   # Client executable
└── Performance Analysis Report
```

---

## 🔧 Backend Makefile

The backend Makefile is **system-specific** and needs to be customized for your environment. Create a `Makefile` in the `backend/` directory with the following content:

```makefile
CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -Iinclude `pkg-config --cflags gstreamer-1.0 glib-2.0`
LIBS = -loqs -lssl -lcrypto `pkg-config --libs gstreamer-1.0 glib-2.0`

# Object files
OBJS = src/crypto_utils.o src/auth_protocol.o

all: server client

# Compile object files
src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Link server
server: $(OBJS) src/server_main.o
	$(CXX) $(CXXFLAGS) -o server $(OBJS) src/server_main.o $(LIBS)

# Link client
client: $(OBJS) src/client_main.o
	$(CXX) $(CXXFLAGS) -o client $(OBJS) src/client_main.o $(LIBS)

clean:
	rm -f server client src/*.o client_keys.json client_dilithium_keys.bin

.PHONY: all clean
```


---

## 🔐 Security Specifications

### Post-Quantum Algorithms

| Component | Algorithm | Security Level | Key/Signature Size |
|-----------|-----------|----------------|-------------------|
| **Key Exchange** | Kyber-768 (ML-KEM) | NIST Level 3 (192-bit quantum) | 1184 bytes (pk), 1088 bytes (ct) |
| **Digital Signature** | Dilithium2 (ML-DSA-65) | NIST Level 2 (128-bit quantum) | 1312 bytes (pk), 2420 bytes (sig) |
| **Stream Encryption** | AES-256-ICM | 256-bit classical, 128-bit quantum | 32 bytes (key) |
| **Authentication** | HMAC-SHA1-80 | 80-bit MAC | 10 bytes (tag) |

### Threat Model

- ✅ **Protected**: Quantum attacks (Shor's, Grover's), man-in-the-middle, replay attacks
- ✅ **Resistant**: Key compromise, eavesdropping, cryptanalysis
- ⚠️ **Assumes**: Secure endpoint, trusted implementation of liboqs

---

## 📊 Video & Audio Quality

- **Resolution**: 640x480 @ 30 FPS
- **Video Codec**: H.264 (500-1500 kbps)
- **Audio Codec**: Opus (48 kHz, 64 kbps)
- **Latency**: <100ms (LAN), 100-300ms (WAN)
- **CPU Usage**: 3-8% total
- **Memory**: 150-250 MB

---

## 🧪 Testing

```bash
# Verify camera
gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! autovideosink

# Check SRTP plugins
gst-inspect-1.0 srtpenc
gst-inspect-1.0 srtpdec

# Verify liboqs
pkg-config --modversion liboqs
```

### Integration Test

1. Start server: `./server <client_ip>`
2. Start client: `./client <server_ip> "TestUser"`
3. Verify video/audio transmission for 10+ minutes

---

## 📚 Documentation

- **[INSTALLATION.md](INSTALLATION.md)**: Step-by-step dependency setup and build instructions

---

## 📊 Performance Analysis

This repository includes a comprehensive **Performance Analysis Report** comparing **Post-Quantum Cryptographic algorithms** with **RSA-2048**, focusing on security, computational performance, and real-world protocol overhead.

📄 **Report:** `Performance Analysis Report.pdf` 

---

### Security Comparison

* **RSA-2048** is **quantum-vulnerable** due to Shor’s algorithm and is deprecated for long-term security.
* **Kyber-768** and **Dilithium3** provide **192-bit post-quantum security**, meeting **NIST Level-3** standards.

---

### Cryptographic Performance

Despite larger key and signature sizes, post-quantum algorithms significantly outperform RSA-2048:

* **Kyber-768:** Key exchange operations are **~8000× faster** than RSA-2048 with total crypto time: **0.034 ms vs ~280 ms**
* **Dilithium3:** Signing: **13× faster** and Verification: **1.6× faster**

This demonstrates that post-quantum cryptography is **computationally efficient**, even outperforming classical cryptography in practice.

---

### Protocol & Handshake Analysis

* **Post-Quantum Handshake Time:** ~740 ms
* **RSA-2048 TLS Handshake:** ~100–150 ms
* **Crypto operations contribute <1 ms** of the total time.
* The higher latency is primarily due to:

  * Multiple round trips (no 1-RTT optimization yet)
  * File I/O during key handling
  * Additional verification steps

* **Bandwidth Overhead:** PQ handshake adds ~**5.9 KB**, which is negligible on modern networks
  *(≈ 0.03 seconds of 1080p video streaming)*

---

###  Future Optimizations
* Reduce round-trip messages
* Remove file I/O from the handshake critical path
* Implement session resumption
* Optimize message batching and framing

---

## 🤝 Contributing

Contributions are welcome! This is an academic project from IIT Jammu.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

---

## 📖 References

### Post-Quantum Cryptography
- [NIST PQC Standardization](https://csrc.nist.gov/projects/post-quantum-cryptography) - Official NIST project
- [CRYSTALS-Kyber (ML-KEM)](https://pq-crystals.org/kyber/) - Key encapsulation mechanism
- [CRYSTALS-Dilithium (ML-DSA)](https://pq-crystals.org/dilithium/) - Digital signature algorithm
- [Open Quantum Safe (liboqs)](https://github.com/open-quantum-safe/liboqs) - C library for PQC

### Multimedia & Security
- [GStreamer Documentation](https://gstreamer.freedesktop.org/) - Multimedia framework
- [SRTP (RFC 3711)](https://tools.ietf.org/html/rfc3711) - Secure RTP protocol
- [Qt6 Documentation](https://doc.qt.io/) - GUI framework


---

## 👨‍💻 Authors

**[Shubham Gupta](https://www.linkedin.com/in/shubham-gupta-79876a255/) || [Vikas Prajapati](https://www.linkedin.com/in/vikas-prajapati-577bab252/) || [Shivani Consul](https://www.linkedin.com/in/shivani-consul-804b59256/)**   
**Indian Institute of Technology Jammu**  
**B.Tech Project (BTP)**  

---

## Supervisor

**Prof. Sumit Kumar Pandey**   
**Assistant Professor of CSE Department**  
**Indian Institute of Technology Jammu**  

---

## 🌟 Acknowledgments

- NIST Post-Quantum Cryptography Standardization Project
- Open Quantum Safe (OQS) Project for liboqs
- GStreamer and Qt communities

---

**🔐 Quantum-safe. Future-proof. Secure today, tomorrow, and beyond.**

*Last Updated: January 10, 2026*

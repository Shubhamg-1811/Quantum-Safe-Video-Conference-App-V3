# Installation Guide - Quantum Secure Video Conferencing System

## Table of Contents
1. [System Requirements](#system-requirements)
2. [Prerequisite Setup](#prerequisite-setup)
3. [Camera Setup](#step-1-camera-setup)
4. [Qt Framework Installation](#step-2-qt-framework-installation)
5. [GStreamer Installation](#step-3-gstreamer-installation)
6. [OpenSSL Installation](#step-4-openssl-installation)
7. [liboqs Installation](#step-5-liboqs-post-quantum-cryptography)
8. [Building the Project](#step-6-building-the-project)
9. [Verification](#step-7-verification)
10. [Troubleshooting](#troubleshooting)

---

## System Requirements

### Hardware
- **Processor**: Intel/AMD x86-64 processor (dual-core minimum)
- **RAM**: 4 GB minimum (8 GB recommended)
- **Storage**: 5 GB free space for dependencies
- **Camera**: USB webcam or built-in camera
- **Microphone**: Audio input device
- **Network**: Ethernet or Wi-Fi for network connectivity

### Software
- **OS**: Ubuntu 20.04 LTS, 22.04 LTS, or 24.04 LTS
- **Kernel**: 5.4 or later
- **Package Manager**: apt (Ubuntu's default)

### Internet Connection
Required for:
- Downloading packages from Ubuntu repositories
- Cloning liboqs from GitHub
- Accessing development tools

---

## Prerequisite Setup

### 1.1 Update System

```bash
# Update package lists
sudo apt update

# Upgrade installed packages (optional but recommended)
sudo apt upgrade -y
```

### 1.2 Install Essential Build Tools

```bash
# Install compilers and build tools
sudo apt install -y \
    build-essential \
    gcc \
    g++ \
    gdb \
    git \
    wget \
    curl \
    pkg-config \
    cmake \
    ninja-build
```

**What these do:**
- `build-essential`: GCC, G++, make, and other build tools
- `git`: Version control for cloning repositories
- `pkg-config`: Helps find library headers and flags
- `cmake`, `ninja-build`: Modern build systems

---

## Step 1: Camera Setup

### 1.1 Install Camera Utilities

```bash
# Install V4L2 and camera tools
sudo apt install -y \
    v4l-utils \
    guvcview \
    cheese \
    ffmpeg \
    libv4l-0 \
    libv4l-dev
```

**Package Details:**
- `v4l-utils`: Command-line tools for Video4Linux devices
- `libv4l-0`, `libv4l-dev`: V4L2 library and headers
- `ffmpeg`: Multimedia framework (used by GStreamer)
- `guvcview`, `cheese`: Camera testing applications

### 1.2 Verify Camera Detection

```bash
# List connected USB devices
lsusb | grep -i camera

# Show video device nodes
ls -lh /dev/video*

# List all video devices with names
v4l2-ctl --list-devices

# Check camera capabilities
v4l2-ctl --device=/dev/video0 --list-formats-ext
```

**Expected Output:**
```
crw-rw----+ 1 root video 81, 0 Dec 15 01:16 /dev/video0

USB 2.0 Camera (usb-0000:00:14.0-1):
    /dev/video0
    /dev/video1

ioctl: VIDIOC_ENUM_FMT
    Type: Video Capture
    [0]: 'YUYV' (YUYV 4:2:2)
        Size: Discrete 640x480
        Interval: Discrete 0.033s (30.000 fps)
```

### 1.3 Grant Camera Permissions (Optional)

```bash
# Add current user to video group
sudo usermod -aG video $USER

# Verify group membership
groups $USER | grep video
```

**Note:** You may need to log out and log back in for permission changes to take effect.

### 1.4 Test Camera with GStreamer

```bash
# Install gstreamer tools first
sudo apt install -y gstreamer1.0-tools

# Test camera
gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! autovideosink
```

**Expected Result:** Live video window opens showing camera feed. Press Ctrl+C to exit.

---

## Step 2: Qt Framework Installation

### 2.1 Install Qt6 from Ubuntu Repository

```bash
# Install Qt6 development files
sudo apt install -y \
    qt6-base-dev \
    qt6-base-dev-tools \
    libqt6core6 \
    libqt6gui6 \
    libqt6widgets6 \
    libqt6network6 \
    libqt6multimediawidgets6 \
    qmake6 \
    qtcreator \
    cmake
```

**Package Details:**
- `qt6-base-dev`: Qt framework development files
- `libqt6gui6`: GUI module
- `libqt6widgets6`: Widgets module (buttons, dialogs, etc.)
- `libqt6network6`: Network utilities for QProcess communication
- `qmake6`: Qt build system
- `qtcreator`: Qt Integrated Development Environment

### 2.2 Verify Qt Installation

```bash
# Check qmake version
qmake6 --version

# Check Qt Creator
qtcreator --version

# List installed Qt packages
dpkg -l | grep "^ii.*qt6" | head -10
```

**Expected Output:**
```
QMake version 3.1
Using Qt version 6.2.4 in /usr/lib/x86_64-linux-gnu

Qt Creator 10.0.x based on Qt 6.2.4 (...)

ii  libqt6core6:amd64          6.2.4dfsg-2ubuntu1   amd64  Qt 6 core module
ii  libqt6gui6:amd64           6.2.4dfsg-2ubuntu1   amd64  Qt 6 GUI module
```

### 2.3 Configure Qt Environment (Optional)

```bash
# Add Qt to PATH (optional)
echo 'export PATH="/usr/lib/qt6/bin:$PATH"' >> ~/.bashrc
source ~/.bashrc

# Verify
which qmake6
```

---

## Step 3: GStreamer Installation

### 3.1 Install GStreamer Core and Plugins

```bash
# Update package lists
sudo apt update

# Install GStreamer and all plugins
sudo apt install -y \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    libgstreamer-plugins-bad1.0-dev \
    gstreamer1.0-plugins-base \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-plugins-ugly \
    gstreamer1.0-libav \
    gstreamer1.0-tools \
    gstreamer1.0-x \
    gstreamer1.0-alsa \
    gstreamer1.0-gl \
    gstreamer1.0-pulseaudio \
    libsrtp2-dev \
    libsrtp2-1
```

**Plugin Details:**
- `plugins-base`: Core plugins (videoscale, audioconvert, etc.)
- `plugins-good`: High-quality plugins (videorate, videotestsrc, etc.)
- `plugins-bad`: Less stable but useful plugins (h264parse, x264enc, etc.)
- `plugins-ugly`: Potentially patent-encumbered plugins (mpeg2dec, etc.)
- `libav`: FFmpeg integration
- `libsrtp2`: SRTP encryption/decryption

### 3.2 Verify GStreamer Installation

```bash
# Check GStreamer version
gst-launch-1.0 --version
pkg-config --modversion gstreamer-1.0

# List available plugins
gst-inspect-1.0 | grep -E "srtpenc|srtpdec|v4l2src|x264enc"

# Check SRTP plugin specifically
gst-inspect-1.0 srtpenc
gst-inspect-1.0 srtpdec
```

**Expected Output:**
```
GStreamer Core Library version 1.20.6
1.20.6

Plugin Details:
  Name                     srtpenc
  Description              Encoder for SRTP packets
  Filename                 /usr/lib/x86_64-linux-gnu/gstreamer-1.0/libgstsrtp.so
```

### 3.3 Test GStreamer with Camera

```bash
# Test basic v4l2 capture
gst-launch-1.0 v4l2src device=/dev/video0 ! videoconvert ! autovideosink

# Test with h264 encoding
gst-launch-1.0 v4l2src device=/dev/video0 ! \
  videoconvert ! \
  x264enc ! \
  h264parse ! \
  rtph264pay ! \
  fakesink

# Press Ctrl+C to stop
```

---

## Step 4: OpenSSL Installation

### 4.1 Install OpenSSL Development Files

```bash
# Install OpenSSL 3.0
sudo apt install -y \
    libssl-dev \
    openssl \
    pkg-config
```

### 4.2 Verify OpenSSL Installation

```bash
# Check OpenSSL version
openssl version

# Check if headers are installed
ls /usr/include/openssl/evp.h
ls /usr/include/openssl/aes.h

# Check pkg-config
pkg-config --modversion openssl
```

**Expected Output:**
```
OpenSSL 3.0.13 30 Jan 2024 (Library: OpenSSL 3.0.13 30 Jan 2024)

/usr/include/openssl/evp.h
/usr/include/openssl/aes.h

3.0.13
```

---

## Step 5: liboqs (Post-Quantum Cryptography)

### 5.1 Install Build Dependencies

```bash
# Install dependencies for building liboqs
sudo apt install -y \
    cmake \
    gcc \
    g++ \
    ninja-build \
    libssl-dev \
    git \
    python3-pytest \
    python3-pytest-xdist \
    unzip \
    doxygen \
    graphviz
```

### 5.2 Clone and Build liboqs

```bash
# Create build directory
mkdir -p ~/liboqs_build && cd ~/liboqs_build

# Clone liboqs from GitHub
git clone --depth 1 https://github.com/open-quantum-safe/liboqs.git
cd liboqs

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake -GNinja \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DOQS_USE_OPENSSL=ON \
    -DBUILD_SHARED_LIBS=ON \
    -DOQS_ENABLE_SIG_DILITHIUM=ON \
    -DOQS_ENABLE_KEM_KYBER=ON \
    ..

# Build (takes 5-10 minutes)
ninja -j$(nproc)

# Install system-wide
sudo ninja install

# Update library cache
sudo ldconfig
```

**Build Flags:**
- `-DCMAKE_INSTALL_PREFIX=/usr/local`: Install to /usr/local
- `-DOQS_USE_OPENSSL=ON`: Use OpenSSL for RNG and hash functions
- `-DBUILD_SHARED_LIBS=ON`: Build shared library (.so)
- `-DOQS_ENABLE_SIG_DILITHIUM=ON`: Enable Dilithium signatures
- `-DOQS_ENABLE_KEM_KYBER=ON`: Enable Kyber key encapsulation

### 5.3 Verify liboqs Installation

```bash
# Check if library is installed
ls -lh /usr/local/lib/liboqs*

# Check if headers are installed
ls /usr/local/include/oqs/oqs.h

# Verify library can be found
sudo ldconfig -p | grep liboqs

# Test with pkg-config
pkg-config --modversion liboqs
```

**Expected Output:**
```
-rw-r--r-- 1 root root 2.8M Dec 15 02:15 /usr/local/lib/liboqs.a
-rwxr-xr-x 1 root root 1.2M Dec 15 02:15 /usr/local/lib/liboqs.so
lrwxrwxrwx 1 root root   16 Dec 15 02:15 /usr/local/lib/liboqs.so.5 -> liboqs.so

/usr/local/include/oqs/oqs.h

liboqs.so.5 (libc6,x86-64) => /usr/local/lib/liboqs.so.5

0.10.0
```

---

## Step 6: Building the Project

### 6.1 Build Backend

```bash
# Navigate to backend directory
cd ~/Desktop/BTP/VideoConferenceApp/backend

# Clean previous builds
make clean

# Build server and client
make

# Verify executables were created
ls -lh server client
```

**Expected Output:**
```
-rwxrwxr-x 1 hp hp 450K Dec 15 02:05 server
-rwxrwxr-x 1 hp hp 450K Dec 15 02:05 client
```

### 6.2 Build Frontend

```bash
# Navigate to frontend directory
cd ~/Desktop/BTP/VideoConferenceApp/frontend

# Generate Makefile from Qt project file
qmake6 frontend.pro

# Build
make

# Verify executable
ls -lh frontend
```

**Expected Output:**
```
-rwxrwxr-x 1 hp hp 5.2M Dec 15 02:10 frontend
```

---

## Step 7: Verification

### 7.1 Verify All Dependencies

```bash
# Create verification script
cat > ~/verify_deps.sh << 'EOF'
#!/bin/bash

echo "=========================================="
echo "  Dependency Verification"
echo "=========================================="
echo ""

# Check GStreamer
echo -n "1. GStreamer: "
pkg-config --exists gstreamer-1.0 && echo "✅" || echo "❌"

# Check GLib
echo -n "2. GLib: "
pkg-config --exists glib-2.0 && echo "✅" || echo "❌"

# Check OpenSSL
echo -n "3. OpenSSL: "
pkg-config --exists openssl && echo "✅" || echo "❌"

# Check liboqs
echo -n "4. liboqs: "
[ -f /usr/local/lib/liboqs.so ] && echo "✅" || echo "❌"

# Check Qt6
echo -n "5. Qt6: "
which qmake6 > /dev/null && echo "✅" || echo "❌"

# Check camera
echo -n "6. Camera: "
[ -e /dev/video0 ] && echo "✅" || echo "❌"

echo ""
echo "=========================================="
EOF

chmod +x ~/verify_deps.sh
~/verify_deps.sh
```

### 7.2 Test Backend Executables

```bash
# Test server
cd ~/Desktop/BTP/VideoConferenceApp/backend
./server

# Expected output: "Usage: ./server <peer_ip>"

# Test client
./client

# Expected output: "Usage: ./client <server_ip> <username>"
```

### 7.3 Test Frontend Application

```bash
# Launch frontend
cd ~/Desktop/BTP/VideoConferenceApp/frontend
./frontend &

# Expected: Qt window opens with connection setup interface
```

### 7.4 Test Complete Video Call

**Machine 1 (Server):**
```bash
cd ~/Desktop/BTP/VideoConferenceApp/backend
./server <machine2_ip>
```

**Machine 2 (Client):**
```bash
cd ~/Desktop/BTP/VideoConferenceApp/backend
./client <machine1_ip> "YourName"
```

**Expected Result:**
- Connection established
- Video stream starts
- Audio transmission begins
- Call quality stable for several minutes

---

## Troubleshooting

### Issue: "Package gstreamer-1.0 was not found in pkg-config search path"

**Solution:**
```bash
# Reinstall GStreamer
sudo apt install --reinstall libgstreamer1.0-dev

# Update pkg-config database
sudo ldconfig
```

### Issue: "fatal error: oqs/oqs.h: No such file or directory"

**Solution:**
```bash
# Verify liboqs installation
ls /usr/local/include/oqs/

# Ensure headers are copied
sudo ninja -C ~/liboqs_build/liboqs/build install

# Update library path
sudo ldconfig
```

### Issue: "cannot find -loqs"

**Solution:**
```bash
# Check if library exists
ls /usr/local/lib/liboqs.so

# Update linker cache
sudo ldconfig

# Verify library is findable
sudo ldconfig -p | grep liboqs
```

### Issue: Camera not detected

**Solution:**
```bash
# Check USB devices
lsusb | grep -i camera

# Check permissions
ls -l /dev/video*

# Add user to video group
sudo usermod -aG video $USER

# Log out and log back in for permission changes
```

### Issue: Qt Creator not finding Qt6

**Solution:**
1. Open Qt Creator
2. Go to **Edit** → **Preferences**
3. Navigate to **Kits**
4. Click **Add** under **Qt Versions**
5. Browse to `/usr/lib/qt6/bin/qmake6`
6. Qt Creator should auto-detect Qt 6.x.x

### Issue: "openssl/evp.h: No such file or directory"

**Solution:**
```bash
# Install OpenSSL dev files
sudo apt install libssl-dev

# Verify headers exist
ls /usr/include/openssl/evp.h
```

### Issue: Frontend doesn't launch backend

**Solution:**
```bash
# Check backend executable permissions
chmod +x ~/Desktop/BTP/VideoConferenceApp/backend/server
chmod +x ~/Desktop/BTP/VideoConferenceApp/backend/client

# Verify paths in launchwindow.cpp
grep -n "QProcess" launchwindow.cpp
```

### General Build Issues

```bash
# Clean all builds
cd ~/Desktop/BTP/VideoConferenceApp
rm -rf frontend/Makefile frontend/moc_* frontend/*.o frontend/frontend
rm -rf backend/*.o backend/server backend/client

# Rebuild
cd backend && make clean && make
cd ../frontend && qmake6 frontend.pro && make
```

---

## System Verification Checklist

- [ ] Ubuntu 20.04+ installed
- [ ] Camera detected and working (`v4l2-ctl --list-devices`)
- [ ] Qt 6.2.4+ installed (`qmake6 --version`)
- [ ] GStreamer 1.20.x+ installed (`gst-launch-1.0 --version`)
- [ ] OpenSSL 3.0.x+ installed (`openssl version`)
- [ ] liboqs built and installed (`ls /usr/local/lib/liboqs.so`)
- [ ] Backend compiles successfully (`make` in backend directory)
- [ ] Frontend compiles successfully (`make` in frontend directory)
- [ ] Executables are present and executable
- [ ] Test call successful between two machines

---

**Installation Complete!** 🎉

For issues or questions, refer to the troubleshooting section above or create an issue in the project repository.

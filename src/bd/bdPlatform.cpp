// ============================================================================
// bdPlatform — platform abstraction (mutex, timing, addressing, sockets)
// Reconstructed from COD3 release decompiled code.
// Win32 implementation; POSIX stubs for cross-compilation testing.
// ============================================================================

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winsock2.h>
  #include <cstdio>
  #include <cstdlib>
  #pragma comment(lib, "ws2_32.lib")
  typedef unsigned long long bdUInt64;
  typedef unsigned int       bdUInt32;
  typedef unsigned int       bdUInt;
  typedef float              bdFloat32;
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/time.h>
  #include <cstdio>
  #include <cstring>
  #include <cstdint>
  #include <cstdlib>
  #include <cerrno>
  typedef uint64_t bdUInt64;
  typedef uint32_t bdUInt32;
  typedef uint32_t bdUInt;
  typedef float    bdFloat32;
#endif

#include <new>

namespace bdMemory {
void* allocate(unsigned int size);
void deallocate(void* p);
}

template <typename T>
struct bdSingleton {
    static T* getInstance();
    static T* m_instance;
};

typedef void (__cdecl *bdSingletonDestroyFunction)();
bool bdSingletonRegistryAdd(bdSingletonDestroyFunction destroyFunction);

// ============================================================================
// bdPlatformTiming (Windows only — POSIX stubs for compile test)
// ============================================================================
#ifdef _WIN32
struct bdPlatformTiming {
    static bdUInt64 getHiResTimeStamp() {
        LARGE_INTEGER pc;
        QueryPerformanceCounter(&pc);
        return pc.QuadPart;
    }
    static double getElapsedTime(bdUInt64 t1, bdUInt64 t2) {
        static LARGE_INTEGER freq;
        static bool init = false;
        if (!init) { QueryPerformanceFrequency(&freq); init = true; }
        return (double)(t2 - t1) / (double)freq.QuadPart;
    }
    static void sleep(bdUInt ms) { Sleep(ms); }
    static bdUInt32 getLoResTimeStamp() { return GetTickCount(); }
    static bdUInt32 getLoResElapsedTime(bdUInt32 t1, bdUInt32 t2) {
        bdUInt64 t2x = t2;
        if (t2 < t1) t2x = t2 + 0x100000000ULL;
        return (bdUInt32)((t2x - t1) / 1000);
    }
};
#else
struct bdPlatformTiming {
    static bdUInt64 getHiResTimeStamp() {
        struct timeval tv; gettimeofday(&tv, nullptr);
        return (bdUInt64)tv.tv_sec * 1000000ULL + tv.tv_usec;
    }
    static double getElapsedTime(bdUInt64 t1, bdUInt64 t2) {
        return (double)(t2 - t1) / 1000000.0;
    }
    static void sleep(bdUInt ms) { usleep(ms * 1000); }
    static bdUInt32 getLoResTimeStamp() {
        struct timeval tv; gettimeofday(&tv, nullptr);
        return (bdUInt32)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
    }
    static bdUInt32 getLoResElapsedTime(bdUInt32 t1, bdUInt32 t2) {
        bdUInt64 t2x = t2;
        if (t2 < t1) t2x = t2 + 0x100000000ULL;
        return (bdUInt32)((t2x - t1) / 1000);
    }
};
#endif

// ============================================================================
// bdPlatformMutex — Win32 mutex objects (ea: 0x8B5EE0-0x8B5F10)
// ============================================================================
#ifdef _WIN32
struct bdPlatformMutex {
    static void* createMutex()         { return CreateMutexA(nullptr, 0, nullptr); }
    static void lock(void*& h)         { WaitForSingleObject(h, 0xFFFFFFFF); }
    static void unlock(void*& h)       { ReleaseMutex(h); }
    static void destroy(void*& h)      { ReleaseMutex(h); CloseHandle(h); h = nullptr; }
};
#else
struct bdPlatformMutex {
    static void* createMutex()         { return nullptr; }
    static void lock(void*&)           {}
    static void unlock(void*&)         {}
    static void destroy(void*& h)      { h = nullptr; }
};
#endif

// ============================================================================
// bdInAddr — internet address wrapper (ea: 0x8B5E40-0x8B5EC0)
// ============================================================================
struct bdInAddr {
    union { bdUInt m_iaddr; struct { unsigned char b1,b2,b3,b4; } m_bytes; } inUn;
    bdInAddr() { inUn.m_iaddr = 0xFF00FF00; }   // ea: 0x8B5E42
    bdInAddr(bdUInt a) { inUn.m_iaddr = a; }
    explicit bdInAddr(const char* s) { fromString(s); }
    void fromString(const char* s) { inUn.m_iaddr = inet_addr(s); }
    bdUInt toString(char* buf, bdUInt bufSize) const {
        unsigned char* b = (unsigned char*)&inUn.m_iaddr;
        return snprintf(buf, bufSize, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
    }
};

// ============================================================================
// bdPlatformSocket — UDP socket wrappers
// Statistics (ea: 0x8B5D00), create (0x9ED1A0), bind (0x9ED210), etc.
// ============================================================================
#ifdef _WIN32
struct bdPlatformSocket {
    static bdUInt64 s_bytesSent, s_bytesReceived, s_packetsSent, s_packetsRecvd;
    static bdUInt64 getBytesSent()     { return s_bytesSent; }
    static bdUInt64 getBytesReceived()  { return s_bytesReceived; }
    static bdUInt64 getPacketsSent()    { return s_packetsSent; }
    static bdUInt64 getPacketsRecvd()   { return s_packetsRecvd; }

    static int create(int nonBlocking) {
        if (!nonBlocking) return socket(AF_INET, SOCK_DGRAM, 254);
        int s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        int on = 1;
        setsockopt(s, SOL_SOCKET, SO_BROADCAST, (const char*)&on, sizeof(on));
        return s;
    }
    static int bind(int& h, const bdInAddr& addr, unsigned short port) {
        if (h == -1) return -10;
        sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr.inUn.m_iaddr;
        sa.sin_port = htons(port);
        if (::bind(h, (sockaddr*)&sa, sizeof(sa)) != -1) return 1;
        int err = WSAGetLastError();
        closesocket(h);
        if (err == 10013 || (err > 10047 && err <= 10049)) return -4;
        return -1;
    }
    static int sendTo(int& h, const bdInAddr& addr, unsigned short port, const void* d, bdUInt len) {
        sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr.inUn.m_iaddr;
        sa.sin_port = htons(port);
        int sent = sendto(h, (const char*)d, len, 0, (sockaddr*)&sa, sizeof(sa));
        if (sent > 0) { s_bytesSent += sent; s_packetsSent++; }
        return sent;
    }
    static int receiveFrom(int& h, bdInAddr& addr, unsigned short& port, void* buf, bdUInt bufSize) {
        sockaddr_in sa = {};
        int saLen = sizeof(sa);
        int r = recvfrom(h, (char*)buf, bufSize, 0, (sockaddr*)&sa, &saLen);
        if (r > 0) {
            addr.inUn.m_iaddr = sa.sin_addr.s_addr;
            port = ntohs(sa.sin_port);
            s_bytesReceived += r; s_packetsRecvd++;
        }
        return r;
    }
    static bool close(int& h) {
        if (h == -1) return true;
        int r = closesocket(h); h = -1;
        return r == 0;
    }
    static bool setBlocking(int& h, bool b) {
        u_long m = b ? 0 : 1;
        return ioctlsocket(h, FIONBIO, &m) == 0;
    }
    static void unregisterThread() {}
    static bdUInt getHostByName(const char* name, bdInAddr* addrs, bdUInt maxAddrs) {
        hostent* host = gethostbyname(name);
        if (!host) return 0;
        bdUInt count = 0;
        for (int i = 0; host->h_addr_list[i] && count < maxAddrs; ++i, ++count)
            addrs[count].inUn.m_iaddr = *(bdUInt*)host->h_addr_list[i];
        return count;
    }
};
bdUInt64 bdPlatformSocket::s_bytesSent = 0;
bdUInt64 bdPlatformSocket::s_bytesReceived = 0;
bdUInt64 bdPlatformSocket::s_packetsSent = 0;
bdUInt64 bdPlatformSocket::s_packetsRecvd = 0;
#else
// POSIX stubs
struct bdPlatformSocket {
    static bdUInt64 s_bytesSent, s_bytesReceived, s_packetsSent, s_packetsRecvd;
    static bdUInt64 getBytesSent()    { return 0; }
    static bdUInt64 getBytesReceived(){ return 0; }
    static bdUInt64 getPacketsSent()  { return 0; }
    static bdUInt64 getPacketsRecvd() { return 0; }
    static int create(int) { return -1; }
    static int bind(int&, const bdInAddr&, unsigned short) { return -1; }
    static int sendTo(int&, const bdInAddr&, unsigned short, const void*, bdUInt) { return -1; }
    static int receiveFrom(int&, bdInAddr&, unsigned short&, void*, bdUInt) { return -1; }
    static bool close(int& h) { h = -1; return true; }
    static bool setBlocking(int&, bool) { return true; }
    static void unregisterThread() {}
    static bdUInt getHostByName(const char*, bdInAddr*, bdUInt) { return 0; }
};
bdUInt64 bdPlatformSocket::s_bytesSent=0;
bdUInt64 bdPlatformSocket::s_bytesReceived=0;
bdUInt64 bdPlatformSocket::s_packetsSent=0;
bdUInt64 bdPlatformSocket::s_packetsRecvd=0;
#endif

// ============================================================================
// bdGetRandomUChar8 — platform random bytes (ea: 0x9ED180)
// ============================================================================
void bdGetRandomUChar8(unsigned char* d, bdUInt n) {
    for (bdUInt i = 0; i < n; ++i) d[i] = (unsigned char)(rand() & 0xFF);
}

struct bdTrulyRandomImpl {
public:
    void getRandomUByte8(unsigned char* out, int count);
    unsigned int getRandomUInt();
};

// bdTrulyRandomImpl::getRandomUByte8 - ea: 0x9EBFF0
void bdTrulyRandomImpl::getRandomUByte8(unsigned char* out, int count) {
    bdGetRandomUChar8(out, (bdUInt)count);
}

// bdTrulyRandomImpl::getRandomUInt - ea: 0x9EC020
unsigned int bdTrulyRandomImpl::getRandomUInt() {
    unsigned char bytes[4];
    bdGetRandomUChar8(bytes, 4);
    return ((unsigned int)bytes[1] << 24)
         | ((unsigned int)bytes[2] << 16)
         | ((unsigned int)bytes[3] << 8);
}

static void destroyTrulyRandomSingleton() {
    bdTrulyRandomImpl* instance = bdSingleton<bdTrulyRandomImpl>::m_instance;
    if (instance != NULL) {
        instance->~bdTrulyRandomImpl();
        bdMemory::deallocate(instance);
        bdSingleton<bdTrulyRandomImpl>::m_instance = NULL;
    }
}

template <>
bdTrulyRandomImpl* bdSingleton<bdTrulyRandomImpl>::m_instance = NULL;

template <>
bdTrulyRandomImpl* bdSingleton<bdTrulyRandomImpl>::getInstance() {
    bdTrulyRandomImpl* instance = bdSingleton<bdTrulyRandomImpl>::m_instance;
    if (instance == NULL) {
        void* memory = bdMemory::allocate(1);
        instance = memory != NULL ? new (memory) bdTrulyRandomImpl() : NULL;
        bdSingleton<bdTrulyRandomImpl>::m_instance = instance;
        if (instance != NULL && bdSingletonRegistryAdd(&destroyTrulyRandomSingleton))
            return instance;
        if (instance != NULL) {
            instance->~bdTrulyRandomImpl();
            bdMemory::deallocate(instance);
            bdSingleton<bdTrulyRandomImpl>::m_instance = NULL;
        }
    }
    return bdSingleton<bdTrulyRandomImpl>::m_instance;
}

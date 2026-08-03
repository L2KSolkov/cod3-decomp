// ============================================================================
// bdPlatform — platform abstraction layer (timing, sockets, mutex, random)
// Ported from Demonware 2.3.4 source, adapted for Win32.
// ea: 0x8B5D00-0x8B5F10, 0x9ED180-0x9ED840 (28 funcs)
// ============================================================================

#ifdef _WIN32
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #include <winsock2.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET BD_SOCKET;
  #define BD_INVALID_SOCKET INVALID_SOCKET
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <sys/time.h>
  #include <cstring>
  #include <cstdlib>
  #include <cstdio>
  #include <cerrno>
  typedef int BD_SOCKET;
  typedef struct sockaddr_in SOCKADDR_IN;
  #define BD_INVALID_SOCKET (-1)
  #define SOCKET_ERROR (-1)
  #define closesocket close
  #define SD_SEND SHUT_WR
  static int WSAGetLastError() { return errno; }
  static int WSAStartup(int, void*) { return 0; }
  static int WSACleanup() { return 0; }
#endif

#include <cstdint>

typedef unsigned long long bdUInt64;
typedef unsigned int       bdUInt32;
typedef unsigned int       bdUInt;
typedef float              bdFloat32;
typedef int                bdInt;
typedef bool               bdBool;

enum bdSocketStatusCode {
    BD_NET_SUCCESS          = 1,
    BD_NET_CONNECTION_CLOSED = 0,
    BD_NET_ERROR            = -1,
    BD_NET_WOULD_BLOCK      = -2,
    BD_NET_SUBSYTEM_ERROR   = -3,
};

// ============================================================================
// bdPlatformTiming
// ea: 0x8B5D50-0x8B5E00
// ============================================================================
class bdPlatformTiming {
public:
    static bdUInt64 getHiResTimeStamp() {
#ifdef _WIN32
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        return t.QuadPart;
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return (bdUInt64)tv.tv_sec * 1000000ULL + tv.tv_usec;
#endif
    }

    static bdFloat32 getElapsedTime(bdUInt64 t1, bdUInt64 t2) {
        return (bdFloat32)((t2 - t1) * 1e-6);
    }

    static void sleep(bdUInt ms) {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(ms * 1000);
#endif
    }

    static bdUInt32 getLoResTimeStamp() {
#ifdef _WIN32
        return GetTickCount();
#else
        struct timeval tv;
        gettimeofday(&tv, nullptr);
        return (bdUInt32)(tv.tv_sec);
#endif
    }

    static bdUInt32 getLoResElapsedTime(bdUInt32 t1, bdUInt32 t2) {
        return t2 - t1;
    }
};

// ============================================================================
// bdPlatformMutex
// ea: 0x8B5EE0-0x8B5F10
// ============================================================================
class bdPlatformMutex {
public:
    static void* createMutex() {
#ifdef _WIN32
        CRITICAL_SECTION* cs = new CRITICAL_SECTION;
        InitializeCriticalSection(cs);
        return cs;
#else
        return nullptr;
#endif
    }
    static void lock(void*& handle) {
#ifdef _WIN32
        EnterCriticalSection((CRITICAL_SECTION*)handle);
#endif
    }
    static void unlock(void*& handle) {
#ifdef _WIN32
        LeaveCriticalSection((CRITICAL_SECTION*)handle);
#endif
    }
    static void destroy(void*& handle) {
#ifdef _WIN32
        DeleteCriticalSection((CRITICAL_SECTION*)handle);
        delete (CRITICAL_SECTION*)handle;
#endif
        handle = nullptr;
    }
};

// ============================================================================
// bdInAddr — IP address wrapper
// ea: 0x8B5E40-0x8B5EC0
// ============================================================================
class bdInAddr {
public:
    unsigned m_addr;

    bdInAddr() : m_addr(0) {}
    bdInAddr(unsigned addr) : m_addr(addr) {}
    bdInAddr(const char* str) : m_addr(0) { fromString(str); }

    void fromString(const char* str) {
        m_addr = inet_addr(str);
    }

    unsigned toString(char* buf, unsigned bufSize) const {
        unsigned char* b = (unsigned char*)&m_addr;
        return snprintf(buf, bufSize, "%u.%u.%u.%u", b[0], b[1], b[2], b[3]);
    }
};

// ============================================================================
// bdPlatformSocket
// ea: 0x8B5D00-0x8B5D40, 0x9ED1A0-0x9ED840
// ============================================================================
class bdPlatformSocket {
    static bdUInt64 s_bytesSent, s_bytesReceived, s_packetsSent, s_packetsRecvd;
public:
    bdPlatformSocket() {}

    static bdUInt64 getBytesSent()    { return s_bytesSent; }
    static bdUInt64 getBytesReceived(){ return s_bytesReceived; }
    static bdUInt64 getPacketsSent()  { return s_packetsSent; }
    static bdUInt64 getPacketsRecvd() { return s_packetsRecvd; }

    static int create(bool nonBlocking) {
#ifdef _WIN32
        static bool wsaInit = false;
        if (!wsaInit) { WSADATA wsa; WSAStartup(0x0202, &wsa); wsaInit = true; }
#endif
        int s = (int)socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
#ifdef _WIN32
        if (nonBlocking && s != BD_INVALID_SOCKET) {
            u_long mode = 1;
            ioctlsocket(s, FIONBIO, &mode);
        }
#endif
        return s;
    }

    static bdSocketStatusCode bind(int& handle, const bdInAddr& addr, unsigned short port) {
        SOCKADDR_IN sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr.m_addr;
        sa.sin_port = htons(port);

        int r = ::bind(handle, (struct sockaddr*)&sa, sizeof(sa));
        if (r == 0) return BD_NET_SUCCESS;
        return BD_NET_ERROR;
    }

    static int sendTo(int& handle, const bdInAddr& addr, unsigned short port, const void* data, unsigned len) {
        SOCKADDR_IN sa;
        memset(&sa, 0, sizeof(sa));
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = addr.m_addr;
        sa.sin_port = htons(port);

        int sent = sendto(handle, (const char*)data, len, 0, (struct sockaddr*)&sa, sizeof(sa));
        if (sent > 0) { s_bytesSent += sent; s_packetsSent++; }
        return sent;
    }

    static int receiveFrom(int& handle, bdInAddr& addr, unsigned short& port, void* buf, unsigned bufSize) {
        SOCKADDR_IN sa;
        socklen_t saLen = sizeof(sa);
        memset(&sa, 0, sizeof(sa));

        int recvd = recvfrom(handle, (char*)buf, bufSize, 0, (struct sockaddr*)&sa, &saLen);
        if (recvd > 0) {
            addr.m_addr = sa.sin_addr.s_addr;
            port = ntohs(sa.sin_port);
            s_bytesReceived += recvd;
            s_packetsRecvd++;
        }
        return recvd;
    }

    static bool close(int& handle) {
        if (handle != BD_INVALID_SOCKET) {
            shutdown(handle, SD_SEND);
            closesocket(handle);
            handle = BD_INVALID_SOCKET;
        }
        return true;
    }

    static bool setBlocking(int& handle, bool blocking) {
#ifdef _WIN32
        u_long mode = blocking ? 0 : 1;
        return ioctlsocket(handle, FIONBIO, &mode) == 0;
#else
        return true;
#endif
    }

    static void unregisterThread() {}

    static unsigned getHostByName(const char* name, bdInAddr* addrs, unsigned maxAddrs) {
        if (!name || !addrs || !maxAddrs) return 0;
        struct hostent* host = gethostbyname(name);
        if (!host) return 0;
        unsigned count = 0;
        for (int i = 0; host->h_addr_list[i] && count < maxAddrs; ++i) {
            addrs[count] = bdInAddr(*(unsigned*)host->h_addr_list[i]);
            ++count;
        }
        return count;
    }
};

bdUInt64 bdPlatformSocket::s_bytesSent = 0;
bdUInt64 bdPlatformSocket::s_bytesReceived = 0;
bdUInt64 bdPlatformSocket::s_packetsSent = 0;
bdUInt64 bdPlatformSocket::s_packetsRecvd = 0;

// ============================================================================
// bdGetRandomUChar8 — platform random bytes (Xbox: XcRC4; Win32: rand)
// ea: 0x9ED180
// ============================================================================
void bdGetRandomUChar8(unsigned char* dest, unsigned count) {
    for (unsigned i = 0; i < count; ++i)
        dest[i] = (unsigned char)(rand() & 0xFF);
}

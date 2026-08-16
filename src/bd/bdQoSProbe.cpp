// ============================================================================
// bdQoSProbe.cpp - Xbox XNet QoS probing (11 funcs).
// Source: bdSocket:bdQoSProbe-xbox.obj
// Verified against IDA (release decompilation + disassembly).
// ============================================================================

#include "bd/bdQoSProbe.h"
#include "bd/bdQoSProbeListener.h"

#include <new>
#include <string.h>

// ============================================================================
// bdQoSRemoteAddr - remote endpoint for QoS (28 bytes)
// ============================================================================
bdQoSRemoteAddr::bdQoSRemoteAddr()
    : m_addr(), m_id(), m_key()
{
}

bdQoSRemoteAddr::bdQoSRemoteAddr(const bdReference<bdCommonAddr>& addr,
                                 const XNKID& id, const XNKEY& key)
    : m_addr(addr.m_ptr), m_id(), m_key()
{
    if (m_addr.m_ptr != nullptr)
        ++m_addr.m_ptr->m_refCount;
    memcpy(&m_id, &id, sizeof(XNKID));
    memcpy(&m_key, &key, sizeof(XNKEY));
}

bdQoSRemoteAddr& bdQoSRemoteAddr::operator=(const bdQoSRemoteAddr& other)
{
    bdReference<bdCommonAddr> newAddr;
    newAddr.m_ptr = other.m_addr.m_ptr;
    if (newAddr.m_ptr != nullptr)
        ++newAddr.m_ptr->m_refCount;
    if (m_addr.m_ptr != nullptr && --m_addr.m_ptr->m_refCount == 0)
    {
        delete m_addr.m_ptr;
        m_addr.m_ptr = nullptr;
    }
    m_addr.m_ptr = newAddr.m_ptr;
    memcpy(&m_id, &other.m_id, sizeof(XNKID));
    memcpy(&m_key, &other.m_key, sizeof(XNKEY));
    return *this;
}

// ============================================================================
// Xbox XNet QoS types (layout from pump disassembly @0x8B6BC0)
// ============================================================================
struct XNQOSINFO {
    uint8_t  bFlags;              // +0x00
    uint8_t  bStatus;             // +0x01
    uint8_t  _pad2[4];            // +0x02
    uint16_t cbData;              // +0x06
    uint8_t* pbData;              // +0x08
    uint8_t  _padC[2];            // +0x0C
    uint16_t wRttMedInMsecs;      // +0x0E
    uint32_t dwDnBitsPerSec;      // +0x10
    uint32_t dwUpBitsPerSec;      // +0x14
};
static_assert(sizeof(XNQOSINFO) == 0x18, "XNQOSINFO size mismatch");

struct XNQOS {
    uint32_t cxnqos;              // +0x00
    uint32_t cxnqosPending;       // +0x04
    XNQOSINFO axnqosinfo[1];      // +0x08
};

enum {
    XNET_XNQOSINFO_COMPLETE = 1,
    XNET_XNQOSINFO_TARGET_CONTACTED = 2,
    XNET_XNQOSINFO_TARGET_DISABLED = 4,
    XNET_QOS_LISTEN_ENABLE = 1,
    XNET_QOS_LISTEN_DISABLE = 2,
    XNET_QOS_LISTEN_SET_DATA = 4,
    XNET_QOS_LISTEN_SET_BITSPERSEC = 8,
    XNET_QOS_LISTEN_RELEASE = 0x10,
};

extern "C" {
int XNetQosLookup(unsigned int cxna, const XNADDR** apxna, const XNKID** apxnkid,
                  const XNKEY** apxnkey, unsigned int cina, const void* aina,
                  const unsigned int* adwServiceId, unsigned int cProbes,
                  unsigned int dwBitsPerSec, unsigned int dwFlags, void* hEvent,
                  XNQOS** ppxnqos);
int XNetQosListen(const XNKID* pxnkid, unsigned char* pb, unsigned int cb,
                  unsigned int dwBitsPerSec, unsigned int dwFlags);
int XNetQosRelease(XNQOS* pxnqos);
}

// ============================================================================
// bdQoSProbeInfo
// ============================================================================
bdQoSProbeInfo::bdQoSProbeInfo() {
}

bdQoSProbeInfo::~bdQoSProbeInfo() {
    if (m_addr.m_ptr != NULL && m_addr.m_ptr->releaseRef() == 0)
        delete m_addr.m_ptr;
    m_addr.m_ptr = NULL;
}

// ============================================================================
// bdQoSProbeEntryWrapper - pending probe entry (24 bytes)
// ============================================================================
struct bdQoSProbeEntryWrapper {
    bdFastArray<bdReference<bdCommonAddr> > m_addrs;  // +0x00
    XNQOS* m_xnqos;                                   // +0x0C
    bdQoSProbeListener* m_listener;                   // +0x10
    unsigned int m_firedEvents;                       // +0x14

    ~bdQoSProbeEntryWrapper() {
        bdMemory::deallocate(m_addrs.m_data);
    }
};

// ============================================================================
// bdQoSProbe::bdQoSProbe - ea: 0x8B6810
// ============================================================================
bdQoSProbe::bdQoSProbe()
    : m_maxBandwidth(0x4000), m_requests(), m_id() {
}

// ============================================================================
// bdQoSProbe::setMaxBandwidth - ea: 0x8B5F50
// ============================================================================
void bdQoSProbe::setMaxBandwidth(unsigned int maxBandwidth) {
    m_maxBandwidth = maxBandwidth;
}

// ============================================================================
// bdQoSProbe::listen - ea: 0x8B5F60
// ============================================================================
bool bdQoSProbe::listen(const XNKID& xnkid, unsigned char* data,
                        unsigned int dataSize) {
    int result = XNetQosListen(&xnkid, data, dataSize, m_maxBandwidth,
                               XNET_QOS_LISTEN_ENABLE | XNET_QOS_LISTEN_SET_DATA |
                               XNET_QOS_LISTEN_SET_BITSPERSEC);
    m_id = xnkid;
    if (result == 0)
        return true;
    bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                         "bool __thiscall bdQoSProbe::listen(const XNKID &,unsigned char *,unsigned int)",
                         0xB2u, "dw/warn/");
    proxy.log("bdSocket/qos", "XNetQosListen failed with %i.", result);
    return false;
}

// ============================================================================
// bdQoSProbe::setData - ea: 0x8B5FE0
// ============================================================================
bool bdQoSProbe::setData(unsigned char* data, unsigned int dataSize) {
    int result = XNetQosListen(&m_id, data, dataSize, m_maxBandwidth,
                               XNET_QOS_LISTEN_SET_DATA);
    if (result == 0)
        return true;
    bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                         "bool __thiscall bdQoSProbe::setData(unsigned char *,unsigned int)",
                         0xC0u, "dw/warn/");
    proxy.log("bdSocket/qos", "XNetQosListen failed with %i.", result);
    return false;
}

// ============================================================================
// bdQoSProbe::disableListener - ea: 0x8B6040
// ============================================================================
void bdQoSProbe::disableListener() {
    int result = XNetQosListen(&m_id, NULL, 0, 0, XNET_QOS_LISTEN_DISABLE);
    if (result != 0) {
        bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                             "void __thiscall bdQoSProbe::disableListener(void)",
                             0xCBu, "dw/warn/");
        proxy.log("bdSocket/qos", "XNetQosListen disable failed with %i.", result);
    }
}

// ============================================================================
// bdQoSProbe::enableListener - ea: 0x8B6090
// ============================================================================
void bdQoSProbe::enableListener() {
    int result = XNetQosListen(&m_id, NULL, 0, 0, XNET_QOS_LISTEN_ENABLE);
    if (result != 0) {
        bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                             "void __thiscall bdQoSProbe::enableListener(void)",
                             0xD4u, "dw/warn/");
        proxy.log("bdSocket/qos", "XNetQosListen enable failed with %i.", result);
    }
}

// ============================================================================
// bdQoSProbe::shutdownListener - ea: 0x8B60E0
// ============================================================================
void bdQoSProbe::shutdownListener() {
    int result = XNetQosListen(&m_id, NULL, 0, 0, XNET_QOS_LISTEN_RELEASE);
    if (result != 0) {
        bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                             "void __thiscall bdQoSProbe::shutdownListener(void)",
                             0xDDu, "dw/warn/");
        proxy.log("bdSocket/qos", "XNetQosListen release failed with %i.", result);
    }
}

// ============================================================================
// bdQoSProbe::probe (single) - ea: 0x8B6EE0
// ============================================================================
bool bdQoSProbe::probe(bdQoSRemoteAddr& addr, bdQoSProbeListener* listener) {
    bdQoSRemoteAddr* copy = (bdQoSRemoteAddr*)bdMemory::allocate(sizeof(bdQoSRemoteAddr));
    bdFastArray<bdQoSRemoteAddr> addrs;
    addrs.m_data = copy;
    addrs.m_capacity = 1;
    addrs.m_size = 1;
    memcpy(copy, &addr, sizeof(bdQoSRemoteAddr));
    bool result = probe(addrs, listener);
    bdMemory::deallocate(copy);
    return result;
}

// ============================================================================
// bdQoSProbe::probe (array) - ea: 0x8B68B0
// ============================================================================
bool bdQoSProbe::probe(bdFastArray<bdQoSRemoteAddr>& addrs,
                       bdQoSProbeListener* listener) {
    const XNADDR** apxna = new const XNADDR*[addrs.m_size];
    const XNKID** apxnkid = new const XNKID*[addrs.m_size];
    const XNKEY** apxnkey = new const XNKEY*[addrs.m_size];

    if (apxna == NULL || apxnkid == NULL || apxnkey == NULL) {
        bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                             "bool __thiscall bdQoSProbe::probe(const class bdFastArray<class bdQoSRemoteAddr> &,class bdQoSProbeListener *)",
                             0x28u, "dw/warn/");
        proxy.log("bdSocket/qos", "Failed to allocate temporary buffers.");
        delete[] apxna;
        delete[] apxnkid;
        delete[] apxnkey;
        return false;
    }

    for (unsigned int i = 0; i < addrs.m_size; i++) {
        apxna[i] = addrs[i].m_addr.m_ptr->getXNAddr();
        apxnkid[i] = &addrs[i].m_id;
        apxnkey[i] = &addrs[i].m_key;
    }

    XNQOS* ppxnqos = NULL;
    bool ok = XNetQosLookup(addrs.m_size, apxna, apxnkid, apxnkey, 0, NULL, NULL,
                            1, m_maxBandwidth, 0, NULL, &ppxnqos) == 0;
    delete[] apxna;
    delete[] apxnkid;
    delete[] apxnkey;

    bdQoSProbeEntryWrapper* wrapper =
        new (bdMemory::allocate(sizeof(bdQoSProbeEntryWrapper))) bdQoSProbeEntryWrapper();
    wrapper->m_addrs.m_data =
        (bdReference<bdCommonAddr>*)bdMemory::allocate(4 * addrs.m_size);
    wrapper->m_addrs.m_capacity = addrs.m_size;
    wrapper->m_addrs.m_size = addrs.m_size;
    for (unsigned int i = 0; i < addrs.m_size; i++) {
        wrapper->m_addrs.m_data[i].m_ptr = addrs[i].m_addr.m_ptr;
        if (wrapper->m_addrs.m_data[i].m_ptr != NULL)
            wrapper->m_addrs.m_data[i].m_ptr->addRef();
    }
    wrapper->m_xnqos = ppxnqos;
    wrapper->m_listener = listener;
    wrapper->m_firedEvents = 0;
    m_requests.addTail(wrapper);
    return ok;
}

// ============================================================================
// bdQoSProbe::pump - ea: 0x8B6BC0
// ============================================================================
void bdQoSProbe::pump() {
    bdLinkedList<bdQoSProbeEntryWrapper*>::Node* node = m_requests.m_head;
    while (node != NULL) {
        bdQoSProbeEntryWrapper* entry = node->m_value;
        XNQOS* xnqos = entry->m_xnqos;
        unsigned int completed = xnqos->cxnqos - xnqos->cxnqosPending;
        bdLinkedList<bdQoSProbeEntryWrapper*>::Node* next = node->m_next;

        if (completed == entry->m_firedEvents) {
            node = next;
            continue;
        }

        if (xnqos->cxnqos != entry->m_addrs.m_size) {
            bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                                 "void __thiscall bdQoSProbe::pump(void)",
                                 0x6Eu, "dw/err");
            proxy.log("defaultFileName",
                      "Number of stored probes and addresses don't match.");
        }

        for (unsigned int j = 0; j < xnqos->cxnqos; j++) {
            XNQOSINFO& info = xnqos->axnqosinfo[j];
            unsigned char flags = info.bFlags;
            if ((flags & XNET_XNQOSINFO_COMPLETE) &&
                (flags & XNET_XNQOSINFO_TARGET_CONTACTED) &&
                entry->m_addrs[j].m_ptr != NULL) {
                bdQoSProbeInfo result;
                result.m_addr.m_ptr = entry->m_addrs[j].m_ptr;
                if (result.m_addr.m_ptr != NULL)
                    result.m_addr.m_ptr->addRef();
                result.m_disabled = (flags & XNET_XNQOSINFO_TARGET_DISABLED) != 0;
                result.m_bandwidthUp = info.dwUpBitsPerSec;
                result.m_bandwidthDown = info.dwDnBitsPerSec;
                result.m_latency = (float)info.wRttMedInMsecs * 0.001f;
                result.m_dataSize = info.cbData;
                result.m_data = info.pbData;
                entry->m_listener->onQoSProbeSuccess(result);

                if (entry->m_addrs[j].m_ptr != NULL &&
                    entry->m_addrs[j].m_ptr->releaseRef() == 0)
                    delete entry->m_addrs[j].m_ptr;
                entry->m_addrs[j].m_ptr = NULL;
                entry->m_firedEvents++;
            } else if ((flags & XNET_XNQOSINFO_COMPLETE) &&
                       !(flags & XNET_XNQOSINFO_TARGET_CONTACTED) &&
                       entry->m_addrs[j].m_ptr != NULL) {
                entry->m_listener->onQoSProbeFail(entry->m_addrs[j]);
                if (entry->m_addrs[j].m_ptr != NULL &&
                    entry->m_addrs[j].m_ptr->releaseRef() == 0)
                    delete entry->m_addrs[j].m_ptr;
                entry->m_addrs[j].m_ptr = NULL;
                entry->m_firedEvents++;
            }
        }

        if (entry->m_firedEvents != completed) {
            bdMessageProxy proxy(".\\bdQoS\\bdQoSProbe-xbox.cpp",
                                 "void __thiscall bdQoSProbe::pump(void)",
                                 0x96u, "dw/err");
            proxy.log("defaultFileName", "Didn't fire some events.");
        }

        if (entry->m_firedEvents == entry->m_addrs.m_size) {
            bdMemory::deallocate(entry->m_addrs.m_data);
            bdMemory::deallocate(entry);
            void* pos = node;
            m_requests.removeAt(pos);
        }
        node = next;
    }
}

// ============================================================================
// bdQoSProbe::cancelProbes - ea: 0x8B6F70
// ============================================================================
void bdQoSProbe::cancelProbes() {
    for (bdLinkedList<bdQoSProbeEntryWrapper*>::Node* node = m_requests.m_head;
         node != NULL; node = node->m_next) {
        bdQoSProbeEntryWrapper* entry = node->m_value;
        XNetQosRelease(entry->m_xnqos);
        if (entry != NULL) {
            entry->~bdQoSProbeEntryWrapper();
            bdMemory::deallocate(entry);
        }
    }
    m_requests.clear();
}

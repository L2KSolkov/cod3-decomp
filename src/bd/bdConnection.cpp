// ============================================================================
// bdConnection.cpp â€” connection base class (15 funcs).
// Source: bdConnection:bdConnection.obj
// Verified against IDA (bdConnection:bdConnection.obj).
// ============================================================================

#include "bd/bd_types.h"

#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
void  deallocate(void* p);
}


// ============================================================================
// bdConnection::bdConnection (default) â€” ea: 0x8A0F10
// ============================================================================
bdConnection::bdConnection()
    : m_addr(),
      m_addrHandle(),
      m_stats(),
      m_listeners(),
      m_maxTransmissionRate(0),
      m_status(BD_NOT_CONNECTED) {
}

// ============================================================================
// bdConnection::bdConnection (addr) â€” ea: 0x8A0F80
// ============================================================================
bdConnection::bdConnection(const bdReference<bdCommonAddr>& addr)
    : m_addr(addr),
      m_addrHandle(),
      m_stats(),
      m_listeners(),
      m_maxTransmissionRate(0),
      m_status(BD_NOT_CONNECTED) {
    if (addr.m_ptr != NULL)
        addr.m_ptr->addRef();
    if (addr.m_ptr != NULL && addr.m_ptr->releaseRef() == 0)
        delete addr.m_ptr;
}

// ============================================================================
// bdConnection::~bdConnection â€” ea: 0x8A0E00
// ============================================================================
bdConnection::~bdConnection() {
    bdMemory::deallocate(this->m_listeners.m_data);
    this->m_listeners.m_data = NULL;
    this->m_listeners.m_size = 0;
    this->m_listeners.m_capacity = 0;
    if (this->m_addrHandle.m_ptr != NULL && this->m_addrHandle.m_ptr->releaseRef() == 0) {
        delete this->m_addrHandle.m_ptr;
        this->m_addrHandle.m_ptr = NULL;
    }
    if (this->m_addr.m_ptr != NULL && this->m_addr.m_ptr->releaseRef() == 0) {
        delete this->m_addr.m_ptr;
        this->m_addr.m_ptr = NULL;
    }
}

// ============================================================================
// setTransmissionRate â€” ea: 0x8A0AD0
// ============================================================================
unsigned int bdConnection::setTransmissionRate(unsigned int rate) {
    this->m_maxTransmissionRate = rate;
    return rate;
}

// ============================================================================
// getTransmissionRate â€” ea: 0x8A0AE0
// ============================================================================
unsigned int bdConnection::getTransmissionRate() const {
    return this->m_maxTransmissionRate;
}
// ============================================================================
// getStatus (vtable slot 4)
// ============================================================================
bdConnection::Status bdConnection::getStatus() const {
    return this->m_status;
}

// ============================================================================
// connect â€” ea: 0x8A0AF0
// ============================================================================
bool bdConnection::connect() {
    this->m_status = BD_CONNECTING;
    return true;
}

// ============================================================================
// disconnect â€” ea: 0x8A0B00
// ============================================================================
void bdConnection::disconnect() {
    this->m_status = BD_DISCONNECTING;
}

// ============================================================================
// close â€” ea: 0x8A0B10
// ============================================================================
void bdConnection::close() {
    this->m_status = BD_DISCONNECTED;
}

// ============================================================================
// getAddress â€” ea: 0x8A0C40
// ============================================================================
bdReference<bdCommonAddr> bdConnection::getAddress() const {
    bdReference<bdCommonAddr> result;
    result.m_ptr = this->m_addr.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// setAddressHandle â€” ea: 0x8A0C60
// ============================================================================
void bdConnection::setAddressHandle(const bdReference<bdAddrHandle>& addrHandle) {
    if (addrHandle.m_ptr != this->m_addrHandle.m_ptr) {
        if (this->m_addrHandle.m_ptr != NULL && this->m_addrHandle.m_ptr->releaseRef() == 0)
            delete this->m_addrHandle.m_ptr;
        this->m_addrHandle.m_ptr = addrHandle.m_ptr;
        if (addrHandle.m_ptr != NULL)
            addrHandle.m_ptr->addRef();
    }
}

// ============================================================================
// registerListener â€” ea: 0x8A0EE0
// ============================================================================
bdConnectionListener* bdConnection::registerListener(bdConnectionListener* listener) {
    if (this->m_listeners.m_size == this->m_listeners.m_capacity)
        this->m_listeners.increaseCapacity(1);
    this->m_listeners.m_data[this->m_listeners.m_size++] = listener;
    return listener;
}

// ============================================================================
// unregisterListener â€” ea: 0x8A10B0
// ============================================================================
int bdConnection::unregisterListener(bdConnectionListener* listener) {
    this->m_listeners.removeAllKeepOrder(listener);
    return 0;
}

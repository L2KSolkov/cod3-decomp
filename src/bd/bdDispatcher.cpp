// ============================================================================
// bdDispatcher.cpp - incoming message dispatcher (4 funcs).
// Source: bdNet:bdDispatcher.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"
#include "bdDispatchInterceptor.h"


// ============================================================================
// bdDispatcher::bdDispatcher - ea: 0x8B0690
// ============================================================================
bdDispatcher::bdDispatcher()
    : m_interceptors() {
}

// ============================================================================
// bdDispatcher::process - ea: 0x8B04F0
// ============================================================================
void bdDispatcher::process(const bdReference<bdConnection>& connection) {
    bdReference<bdMessage> message;
    while (connection.m_ptr->getMessageToDispatch(message)) {
        bdReceivedMessage received(message, connection);
        bool accepted = false;
        for (unsigned int i = 0; !accepted && i < m_interceptors.m_size; i++) {
            accepted = m_interceptors[i]->accept(received);
        }
    }
}

// ============================================================================
// bdDispatcher::registerInterceptor - ea: 0x8B06A0
// ============================================================================
void bdDispatcher::registerInterceptor(bdDispatchInterceptor* const interceptor) {
    if (m_interceptors.m_size == m_interceptors.m_capacity)
        m_interceptors.increaseCapacity(1);
    m_interceptors.m_data[m_interceptors.m_size++] = interceptor;
}

// ============================================================================
// bdDispatcher::unregisterInterceptor - ea: 0x8B0770
// ============================================================================
void bdDispatcher::unregisterInterceptor(bdDispatchInterceptor* const interceptor) {
    m_interceptors.removeAllKeepOrder(interceptor);
}

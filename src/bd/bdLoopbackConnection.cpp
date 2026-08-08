// ============================================================================
// bdLoopbackConnection.cpp - loopback connection (8 funcs).
// Source: bdConnection:bdLoopbackConnection.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdLoopbackConnection::bdLoopbackConnection - ea: 0x8A4180
// ============================================================================
bdLoopbackConnection::bdLoopbackConnection(const bdReference<bdCommonAddr>& addr)
    : bdConnection(addr), m_messages(), m_flag70(0) {
}

// ============================================================================
// bdLoopbackConnection::~bdLoopbackConnection - ea: 0x8A4020
// ============================================================================
bdLoopbackConnection::~bdLoopbackConnection() {
}

// ============================================================================
// bdLoopbackConnection::receive - ea: 0x8A3A60
// ============================================================================
bool bdLoopbackConnection::receive(const unsigned char*, unsigned int) {
    return true;
}

// ============================================================================
// bdLoopbackConnection::send - ea: 0x8A40A0
// ============================================================================
bool bdLoopbackConnection::send(const bdReference<bdMessage>& message, bool) {
    bdReference<bdBitBuffer> payload = message.m_ptr->getPayload();
    if (payload.m_ptr != NULL)
        payload.m_ptr->resetReadPosition();

    m_messages.enqueue(message);
    m_flag70 = 1;
    return true;
}

// ============================================================================
// bdLoopbackConnection::getMessageToDispatch - ea: 0x8A3FA0
// ============================================================================
bool bdLoopbackConnection::getMessageToDispatch(bdReference<bdMessage>& message) {
    bool result = false;
    if (m_messages.getSize() != 0) {
        if (message.m_ptr != NULL && message.m_ptr->releaseRef() == 0)
            delete message.m_ptr;
        message.m_ptr = m_messages.peek().m_ptr;
        if (message.m_ptr != NULL)
            message.m_ptr->addRef();
        m_messages.dequeue();
        result = true;
    }
    return result;
}

// ============================================================================
// bdLoopbackConnection::getDataToSend - ea: 0x8A3A80
// ============================================================================
unsigned int bdLoopbackConnection::getDataToSend(unsigned char*, unsigned int) {
    return 0;
}

// ============================================================================
// bdLoopbackConnection::updateStatus - ea: 0x8A3C80
// ============================================================================
void bdLoopbackConnection::updateStatus() {
    switch (m_status) {
    case BD_NOT_CONNECTED:
    case BD_CONNECTED:
    case BD_DISCONNECTED:
        break;
    case BD_CONNECTING:
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners[i]->onConnect(bdReference<bdConnection>(this));
        m_status = BD_CONNECTED;
        break;
    case BD_DISCONNECTING:
        for (unsigned int i = 0; i < m_listeners.m_size; i++)
            m_listeners[i]->onDisconnect(bdReference<bdConnection>(this));
        m_status = BD_DISCONNECTED;
        break;
    }
}

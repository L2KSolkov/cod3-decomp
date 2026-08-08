// ============================================================================
// bdReceivedMessage.cpp - message + originating connection (4 funcs).
// Source: bdConnection:bdReceivedMessage.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdReceivedMessage::bdReceivedMessage - ea: 0x8A10C0
// ============================================================================
bdReceivedMessage::bdReceivedMessage(const bdReference<bdMessage>& message,
                                     const bdReference<bdConnection>& connection)
    : m_message(message.m_ptr), m_connection(connection.m_ptr) {
    if (m_message.m_ptr != NULL)
        m_message.m_ptr->addRef();
    if (m_connection.m_ptr != NULL)
        m_connection.m_ptr->addRef();
}

// ============================================================================
// bdReceivedMessage::~bdReceivedMessage - ea: 0x8A1150
// ============================================================================
bdReceivedMessage::~bdReceivedMessage() {
    if (m_connection.m_ptr != NULL && m_connection.m_ptr->releaseRef() == 0)
        delete m_connection.m_ptr;
    m_connection.m_ptr = NULL;
    if (m_message.m_ptr != NULL && m_message.m_ptr->releaseRef() == 0)
        delete m_message.m_ptr;
    m_message.m_ptr = NULL;
}

// ============================================================================
// bdReceivedMessage::getMessage - ea: 0x8A11E0
// ============================================================================
bdReference<bdMessage> bdReceivedMessage::getMessage() const {
    bdReference<bdMessage> result;
    result.m_ptr = m_message.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdReceivedMessage::getConnection - ea: 0x8A1200
// ============================================================================
bdReference<bdConnection> bdReceivedMessage::getConnection() const {
    bdReference<bdConnection> result;
    result.m_ptr = m_connection.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdMessage.cpp â€” network message (9 funcs).
// Source: bdConnection:bdMessage.obj
// Verified against IDA (bdConnection:bdMessage.obj).
// ============================================================================

#include "bd/bd_types.h"

#include <new>
#include <string.h>

// ============================================================================
// Cross-object externs
// ============================================================================
namespace bdMemory {
void* allocate(unsigned int size);
}

// ============================================================================
// bdMessage::getType â€” ea: 0x8A0590
// ============================================================================
unsigned char bdMessage::getType() const {
    return this->m_type;
}

// ============================================================================
// bdMessage::bdMessage (type, checked) â€” ea: 0x8A06D0
// ============================================================================
bdMessage::bdMessage(unsigned char type, bool payloadTypeChecked)
    : m_type(type),
      m_payload(),
      m_payloadTypeChecked(payloadTypeChecked),
      m_unencPayload() {
}

// ============================================================================
// bdMessage::bdMessage (full) â€” ea: 0x8A0700
// ============================================================================
bdMessage::bdMessage(unsigned char type, const unsigned char* data, unsigned int dataSize,
                     bool typeChecked, const unsigned char* unencData,
                     unsigned int unencSize)
    : m_type(type),
      m_payload(),
      m_payloadTypeChecked(false),
      m_unencPayload() {
    if (data != NULL && dataSize != 0) {
        bdBitBuffer* v8 = (bdBitBuffer*)bdMemory::allocate(0x24u);
        bdBitBuffer* v9 = v8 != NULL ? new (v8) bdBitBuffer(data, 8 * dataSize, typeChecked) : NULL;
        if (this->m_payload.m_ptr != NULL && this->m_payload.m_ptr->releaseRef() == 0)
            delete this->m_payload.m_ptr;
        this->m_payload.m_ptr = v9;
        if (v9 != NULL)
            v9->addRef();
        this->m_payloadTypeChecked = this->m_payload.m_ptr->getTypeCheck();
    }
    if (unencData != NULL && unencSize != 0) {
        bdByteBuffer* v13 = (bdByteBuffer*)bdMemory::allocate(0x18u);
        bdByteBuffer* v14 = v13 != NULL ? new (v13) bdByteBuffer(unencSize) : NULL;
        if (this->m_unencPayload.m_ptr != NULL && this->m_unencPayload.m_ptr->releaseRef() == 0)
            delete this->m_unencPayload.m_ptr;
        this->m_unencPayload.m_ptr = v14;
        if (v14 != NULL)
            v14->addRef();
        memcpy(this->m_unencPayload.m_ptr->m_data, unencData, unencSize);
    }
}

// ============================================================================
// bdMessage::~bdMessage â€” ea: 0x8A0850
// ============================================================================
bdMessage::~bdMessage() {
    if (this->m_unencPayload.m_ptr != NULL && this->m_unencPayload.m_ptr->releaseRef() == 0) {
        delete this->m_unencPayload.m_ptr;
        this->m_unencPayload.m_ptr = NULL;
    }
    if (this->m_payload.m_ptr != NULL && this->m_payload.m_ptr->releaseRef() == 0) {
        delete this->m_payload.m_ptr;
        this->m_payload.m_ptr = NULL;
    }
}

// ============================================================================
// bdMessage::getPayload â€” ea: 0x8A08F0
// ============================================================================
bdReference<bdBitBuffer> bdMessage::getPayload() {
    if (this->m_payload.m_ptr == NULL) {
        bdBitBuffer* v3 = (bdBitBuffer*)bdMemory::allocate(0x24u);
        bdBitBuffer* v4 = v3 != NULL ? new (v3) bdBitBuffer(0, this->m_payloadTypeChecked) : NULL;
        if (this->m_payload.m_ptr != NULL && this->m_payload.m_ptr->releaseRef() == 0)
            delete this->m_payload.m_ptr;
        this->m_payload.m_ptr = v4;
        if (v4 != NULL)
            v4->addRef();
    }
    bdReference<bdBitBuffer> result;
    result.m_ptr = this->m_payload.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdMessage::hasPayload â€” ea: 0x8A09A0
// ============================================================================
bool bdMessage::hasPayload() const {
    return this->m_payload.m_ptr != NULL;
}

// ============================================================================
// bdMessage::createUnencryptedPayload â€” ea: 0x8A09B0
// ============================================================================
bdByteBuffer* bdMessage::createUnencryptedPayload(unsigned int size) {
    bdByteBuffer* v3 = (bdByteBuffer*)bdMemory::allocate(0x18u);
    bdByteBuffer* v4 = v3 != NULL ? new (v3) bdByteBuffer(size) : NULL;
    if (this->m_unencPayload.m_ptr != NULL && this->m_unencPayload.m_ptr->releaseRef() == 0)
        delete this->m_unencPayload.m_ptr;
    this->m_unencPayload.m_ptr = v4;
    if (v4 != NULL)
        v4->addRef();
    return this->m_unencPayload.m_ptr;
}

// ============================================================================
// bdMessage::getUnencryptedPayload â€” ea: 0x8A0A40
// ============================================================================
bdReference<bdByteBuffer> bdMessage::getUnencryptedPayload() const {
    bdReference<bdByteBuffer> result;
    result.m_ptr = this->m_unencPayload.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdMessage::hasUnencryptedPayload â€” ea: 0x8A0A60
// ============================================================================
bool bdMessage::hasUnencryptedPayload() const {
    return this->m_unencPayload.m_ptr != NULL;
}

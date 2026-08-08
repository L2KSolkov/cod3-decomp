// ============================================================================
// bdDataChunk.cpp - data chunk (12 funcs).
// Source: bdConnection:bdDataChunk.obj
// Verified against IDA (release decompilation + disassembly).
// ============================================================================

#include "bd/bd_types.h"

#include <new>

// ============================================================================
// bdDataChunk::setSequenceNumber - ea: 0x8AA160
// ============================================================================
void bdDataChunk::setSequenceNumber(unsigned short sequenceNumber) {
    m_sequenceNumber = sequenceNumber;
}

// ============================================================================
// bdDataChunk::getSequenceNumber - ea: 0x8AA170
// ============================================================================
unsigned short bdDataChunk::getSequenceNumber() const {
    return m_sequenceNumber;
}

// ============================================================================
// bdDataChunk::getFlags - ea: 0x8AA180
// ============================================================================
uint8_t bdDataChunk::getFlags() const {
    return m_flags;
}

// ============================================================================
// bdDataChunk::bdDataChunk (default) - ea: 0x8AA1D0
// ============================================================================
bdDataChunk::bdDataChunk()
    : bdChunk(BD_CHUNK_DATA),
      m_message(),
      m_flags(BD_DC_NONE),
      m_sequenceNumber(0) {
}

// ============================================================================
// bdDataChunk::bdDataChunk (message, flags) - ea: 0x8AA1F0
// ============================================================================
bdDataChunk::bdDataChunk(const bdReference<bdMessage>& message, bdDataFlags flags)
    : bdChunk(BD_CHUNK_DATA),
      m_message(message.m_ptr),
      m_flags((uint8_t)flags),
      m_sequenceNumber(0) {
    if (m_message.m_ptr != NULL)
        m_message.m_ptr->addRef();

    if (m_message.m_ptr != NULL) {
        bool hasEncData = false;
        if (m_message.m_ptr->hasPayload()) {
            bdReference<bdBitBuffer> payload = m_message.m_ptr->getPayload();
            if (payload.m_ptr != NULL) {
                if (payload.m_ptr->getNumBitsWritten() >= 2)
                    hasEncData = true;
                if (payload.m_ptr->releaseRef() == 0)
                    delete payload.m_ptr;
            }
        }
        if (hasEncData)
            m_flags |= BD_DC_ENC_DATA;

        bool hasUnencData = false;
        if (m_message.m_ptr->hasUnencryptedPayload()) {
            bdReference<bdByteBuffer> unenc = m_message.m_ptr->getUnencryptedPayload();
            if (unenc.m_ptr != NULL) {
                if (unenc.m_ptr->getMaxReadSize() != 0)
                    hasUnencData = true;
                if (unenc.m_ptr->releaseRef() == 0)
                    delete unenc.m_ptr;
            }
        }
        if (hasUnencData)
            m_flags |= BD_DC_UNENC_DATA;
    }
}

// ============================================================================
// bdDataChunk::~bdDataChunk - ea: 0x8AA360
// ============================================================================
bdDataChunk::~bdDataChunk() {
    if (m_message.m_ptr != NULL && m_message.m_ptr->releaseRef() == 0)
        delete m_message.m_ptr;
    m_message.m_ptr = NULL;
}

// ============================================================================
// bdDataChunk::getMessage - ea: 0x8AA3E0 (returns a new reference)
// ============================================================================
bdReference<bdMessage> bdDataChunk::getMessage() const {
    bdReference<bdMessage> result;
    result.m_ptr = m_message.m_ptr;
    if (result.m_ptr != NULL)
        result.m_ptr->addRef();
    return result;
}

// ============================================================================
// bdDataChunk::getSerializedSize - ea: 0x8AA400
// ============================================================================
unsigned int bdDataChunk::getSerializedSize() {
    unsigned int size = 0;
    if (m_message.m_ptr != NULL) {
        if (m_flags & BD_DC_ENC_DATA) {
            do {
                if (!m_message.m_ptr->hasPayload()) {
                    bdMessageProxy proxy(".\\bdPacket\\bdDataChunk.cpp",
                                         "unsigned int __thiscall bdDataChunk::getSerializedSize(void)",
                                         0x4Fu, "dw/err");
                    proxy.log("defaultFileName", "BD_DC_ENC_DATA flag set but no payload.");
                }
            } while (g_assertFalse);
            bdReference<bdBitBuffer> payload = m_message.m_ptr->getPayload();
            if (payload.m_ptr != NULL) {
                if (payload.m_ptr->getNumBitsWritten() >= 2)
                    size = payload.m_ptr->getDataSize();
                if (payload.m_ptr->releaseRef() == 0)
                    delete payload.m_ptr;
            }
        }
        if (m_flags & BD_DC_UNENC_DATA) {
            do {
                if (!m_message.m_ptr->hasUnencryptedPayload()) {
                    bdMessageProxy proxy(".\\bdPacket\\bdDataChunk.cpp",
                                         "unsigned int __thiscall bdDataChunk::getSerializedSize(void)",
                                         0x5Bu, "dw/err");
                    proxy.log("defaultFileName", "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
                }
            } while (g_assertFalse);
            bdReference<bdByteBuffer> unenc = m_message.m_ptr->getUnencryptedPayload();
            if (unenc.m_ptr != NULL) {
                size += unenc.m_ptr->getMaxReadSize();
                if (unenc.m_ptr->releaseRef() == 0)
                    delete unenc.m_ptr;
            }
        }
    }
    return size + 8;
}

// ============================================================================
// bdDataChunk::serializeUnencrypted - ea: 0x8AA530
// ============================================================================
unsigned int bdDataChunk::serializeUnencrypted(unsigned char* data, unsigned int size) {
    unsigned int bytesWritten = 0;
    if (m_flags & BD_DC_UNENC_DATA) {
        do {
            if (!m_message.m_ptr->hasUnencryptedPayload()) {
                bdMessageProxy proxy(".\\bdPacket\\bdDataChunk.cpp",
                                     "unsigned int __thiscall bdDataChunk::serializeUnencrypted(unsigned char *,const unsigned int)",
                                     0x70u, "dw/err");
                proxy.log("defaultFileName", "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
            }
        } while (g_assertFalse);
        bdReference<bdByteBuffer> unenc = m_message.m_ptr->getUnencryptedPayload();
        if (unenc.m_ptr != NULL) {
            const unsigned char* unencData = unenc.m_ptr->getData();
            unsigned int unencDataSize = unenc.m_ptr->getMaxReadSize();
            unsigned int offset = 0;
            if (bdBytePacker::appendBuffer(data, size, 0, &offset, unencData, unencDataSize))
                bytesWritten = offset;
            if (unenc.m_ptr->releaseRef() == 0)
                delete unenc.m_ptr;
        }
    }
    return bytesWritten;
}

// ============================================================================
// bdDataChunk::serialize - ea: 0x8AA660
// ============================================================================
unsigned int bdDataChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int offset = bdChunk::serialize(data, size);
    bool ok = true;
    unsigned int payloadSize = 0;

    if (m_message.m_ptr != NULL) {
        uint8_t flags = m_flags;
        ok = bdBytePacker::appendBasicType(data, size, offset, &offset, &flags, 1u);
        uint8_t type = (uint8_t)m_message.m_ptr->getType();
        ok = ok && bdBytePacker::appendBasicType(data, size, offset, &offset, &type, 1u);
        unsigned int seqNum = m_sequenceNumber;
        ok = ok && bdBytePacker::appendBasicType(data, size, offset, &offset, &seqNum, 2u);

        if (m_flags & BD_DC_ENC_DATA) {
            do {
                if (!m_message.m_ptr->hasPayload()) {
                    bdMessageProxy proxy(".\\bdPacket\\bdDataChunk.cpp",
                                         "unsigned int __thiscall bdDataChunk::serialize(unsigned char *,const unsigned int)",
                                         0x9Bu, "dw/err");
                    proxy.log("defaultFileName", "BD_DC_ENC_DATA flag set but no payload.");
                }
            } while (g_assertFalse);
            bdReference<bdBitBuffer> payload = m_message.m_ptr->getPayload();
            if (payload.m_ptr != NULL) {
                if (payload.m_ptr->getNumBitsWritten() >= 2)
                    payloadSize = payload.m_ptr->getDataSize();
                if (payload.m_ptr->releaseRef() == 0)
                    delete payload.m_ptr;
            }
            ok = ok && bdBytePacker::appendEncodedUInt16(data, size, offset, &offset,
                                                         (unsigned short)payloadSize);
        }

        if (m_flags & BD_DC_UNENC_DATA) {
            do {
                if (!m_message.m_ptr->hasUnencryptedPayload()) {
                    bdMessageProxy proxy(".\\bdPacket\\bdDataChunk.cpp",
                                         "unsigned int __thiscall bdDataChunk::serialize(unsigned char *,const unsigned int)",
                                         0xADu, "dw/err");
                    proxy.log("defaultFileName", "BD_DC_UNENC_DATA flag set but no unencrypted payload.");
                }
            } while (g_assertFalse);
            bdReference<bdByteBuffer> unenc = m_message.m_ptr->getUnencryptedPayload();
            unsigned int unencSize = 0;
            if (unenc.m_ptr != NULL)
                unencSize = unenc.m_ptr->getMaxReadSize();
            ok = ok && bdBytePacker::appendEncodedUInt16(data, size, offset, &offset,
                                                         (unsigned short)unencSize);
            if (unenc.m_ptr != NULL && unenc.m_ptr->releaseRef() == 0)
                delete unenc.m_ptr;
        }

        if (payloadSize > 0) {
            bdReference<bdBitBuffer> payload = m_message.m_ptr->getPayload();
            const unsigned char* payloadData = NULL;
            if (payload.m_ptr != NULL)
                payloadData = payload.m_ptr->getData();
            if (payload.m_ptr != NULL && payload.m_ptr->releaseRef() == 0)
                delete payload.m_ptr;
            ok = ok && bdBytePacker::appendBuffer(data, size, offset, &offset,
                                                  payloadData, payloadSize);
        }
    }

    return ok ? offset : 0;
}

// ============================================================================
// bdDataChunk::deserialize (full, with unencrypted stream) - ea: 0x8AA950
// ============================================================================
bool bdDataChunk::deserialize(const unsigned char* data, unsigned int size,
                              unsigned int* offset, const unsigned char* unencData,
                              unsigned int unencSize, unsigned int* unencReadOffset) {
    (void)unencSize;
    unsigned int bytesRead = *offset;
    unsigned int unencBytesRead = *unencReadOffset;

    bool ok = bdChunk::deserialize(data, size, &bytesRead);
    if (ok && bdBytePacker::removeBasicType(data, size, bytesRead, &bytesRead,
                                            &m_flags, 1u)) {
        ok = true;
    } else {
        ok = false;
    }

    uint8_t type8 = 0;
    if (ok && bdBytePacker::removeBasicType(data, size, bytesRead, &bytesRead,
                                            &type8, 1u)) {
        ok = true;
    } else {
        ok = false;
    }

    uint16_t seqNum = 0;
    if (ok && bdBytePacker::removeBasicType(data, size, bytesRead, &bytesRead,
                                            &seqNum, 2u)) {
        ok = true;
    } else {
        ok = false;
    }
    m_sequenceNumber = seqNum;

    uint16_t payloadSize = 0;
    if (m_flags & BD_DC_ENC_DATA) {
        ok = ok && bdBytePacker::removeEncodedUInt16(data, size, bytesRead,
                                                     &bytesRead, &payloadSize);
    }
    uint16_t unencPayloadSize = 0;
    if (m_flags & BD_DC_UNENC_DATA) {
        ok = ok && bdBytePacker::removeEncodedUInt16(data, size, bytesRead,
                                                     &bytesRead, &unencPayloadSize);
    }

    if (ok) {
        if (m_message.m_ptr != NULL && m_message.m_ptr->releaseRef() == 0)
            delete m_message.m_ptr;
        m_message.m_ptr = new (bdMemory::allocate(sizeof(bdMessage)))
            bdMessage(type8, data + bytesRead, payloadSize, true,
                      unencData + unencBytesRead, unencPayloadSize);
        if (m_message.m_ptr != NULL)
            m_message.m_ptr->addRef();
        ok = m_message.m_ptr != NULL;
        bytesRead += payloadSize;
        unencBytesRead += unencPayloadSize;
        *offset = bytesRead;
        *unencReadOffset = unencBytesRead;
    }

    return ok;
}

// ============================================================================
// bdDataChunk::deserialize (single stream) - ea: 0x8AAB70
// ============================================================================
bool bdDataChunk::deserialize(const unsigned char* data, unsigned int size,
                              unsigned int* offset) {
    unsigned int unencOffset = 0;
    return deserialize(data, size, offset, NULL, 0, &unencOffset);
}

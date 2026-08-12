// AUTO-GENERATED STUBS — Bandwidth/Discovery networking (bdCore+bdConnection+bdNet+bdPeer+bdSocket+bdPlatform)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#include "bd_types.h"

// bdByteBuffer ctor - ?0bdByteBuffer@@QAE@I@Z (bdCore:bdByteBuffer.o)
bdByteBuffer::bdByteBuffer(unsigned int size)
{
    m_size = size;
    m_data = new uint8_t[size ? size : 1];
    m_readPtr = m_data;
    m_writePtr = m_data;
}

// bdBitBuffer ctors - ?0bdBitBuffer@@QAE@PBI_N@Z / ?0bdBitBuffer@@QAE@I_N@Z
bdBitBuffer::bdBitBuffer(const unsigned char* data, unsigned int bitCount,
                         bool typeChecked)
{
    m_writePosition = 0;
    m_maxWritePosition = bitCount;
    m_readPosition = 0;
    m_failedRead = false;
    m_typeChecked = typeChecked;
    m_data.m_size = (bitCount + 7) / 8;
    m_data.m_capacity = m_data.m_size;
    m_data.m_data = const_cast<unsigned char*>(data);
}

bdBitBuffer::bdBitBuffer(unsigned int bitCount, bool typeChecked)
{
    m_writePosition = 0;
    m_maxWritePosition = bitCount;
    m_readPosition = 0;
    m_failedRead = false;
    m_typeChecked = typeChecked;
    unsigned int bytes = (bitCount + 7) / 8;
    m_data.m_size = bytes;
    m_data.m_capacity = bytes;
    m_data.m_data = bytes ? new uint8_t[bytes] : nullptr;
}

bool bdBitBuffer::getTypeCheck() const
{
    return m_typeChecked;
}

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_bd(void) {
    COD3_UNIMPLEMENTED("bd");
}

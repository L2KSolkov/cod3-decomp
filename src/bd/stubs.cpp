// AUTO-GENERATED STUBS — Bandwidth/Discovery networking (bdCore+bdConnection+bdNet+bdPeer+bdSocket+bdPlatform)
// 0 non-inline functions to port
// When ported, functions move from here to their real .cpp files.

#include <stdio.h>

#include "bd_types.h"
#include "bd/bdUtilities/bdBitOperations.h"

extern const char* const defaultFileName;

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

// bdBitBuffer IO primitives (bdCore:bdBitBuffer.obj). Stubs; port from IDA
// (writeBits ea 0x89BCF0). Note: the binary keeps writeDataType/readDataType
// private (IAE mangling); the public QAE forms here satisfy the reconstructed
// callers until the typed-writer API is ported.
void bdBitBuffer::writeBits(const void* data, unsigned int bitCount)
{
    (void)data; (void)bitCount;
}

void bdBitBuffer::writeDataType(bdBitBufferDataType type)
{
    (void)type;
}

// bdBitBuffer::writeRangedInt32 - ea: 0x89C1F0
void bdBitBuffer::writeRangedInt32(int value, int min, int max)
{
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "void __thiscall bdBitBuffer::writeRangedInt32(const int,const int,const int)",
                                 0x76u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedInt32, end of range is less than the begining");
        }
    } while (g_assertFalse);
    if (m_typeChecked)
    {
        writeRangedUInt32(0xBu, 0, 0x1Fu, false);
        if (m_typeChecked)
        {
            writeRangedUInt32(7u, 0, 0x1Fu, false);
            int v = min;
            writeBits(&v, 0x20);
            if (m_typeChecked)
                writeRangedUInt32(7u, 0, 0x1Fu, false);
            v = max;
            writeBits(&v, 0x20);
        }
    }
    unsigned int bits = 0;
    if (max != min)
        bits = bdHighBitNumber((unsigned int)(max - min)) + 1;
    int v8 = value;
    if (value <= max)
    {
        if (value < min)
            v8 = min;
    }
    else
    {
        v8 = max;
    }
    v8 -= min;
    writeBits(&v8, bits);
}

// bdBitBuffer::writeRangedUInt32 - ea: 0x89C0D0
void bdBitBuffer::writeRangedUInt32(unsigned int value, unsigned int min,
                                    unsigned int max, bool typeChecked)
{
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "void __thiscall bdBitBuffer::writeRangedUInt32(const unsigned int,const unsigned int,const unsigned int,const bool)",
                                 0x54u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedUInt, end of range is less than the begining");
        }
    } while (g_assertFalse);
    if (typeChecked && m_typeChecked)
    {
        writeRangedUInt32(0xCu, 0, 0x1Fu, false);
        if (m_typeChecked)
        {
            writeRangedUInt32(8u, 0, 0x1Fu, false);
            unsigned int v = min;
            writeBits(&v, 0x20);
            if (m_typeChecked)
                writeRangedUInt32(8u, 0, 0x1Fu, false);
            v = max;
            writeBits(&v, 0x20);
        }
    }
    unsigned int bits = 0;
    if (max != min)
        bits = bdHighBitNumber(max - min) + 1;
    unsigned int v8 = value;
    if (value <= max)
    {
        if (value < min)
            v8 = min;
    }
    else
    {
        v8 = max;
    }
    v8 -= min;
    writeBits(&v8, bits);
}

// bdBitBuffer::writeRangedFloat32 - ea: 0x89C2F0 (stub; port with writeBits)
void bdBitBuffer::writeRangedFloat32(float value, float min, float max,
                                     float scale)
{
    (void)value; (void)min; (void)max; (void)scale;
}

bool bdBitBuffer::readRangedInt32(int& value, int min, int max)
{
    (void)value; (void)min; (void)max;
    return true;
}

bool bdBitBuffer::readRangedUInt32(unsigned int& value, unsigned int min,
                                   unsigned int max, bool typeChecked)
{
    (void)value; (void)min; (void)max; (void)typeChecked;
    return true;
}

bool bdBitBuffer::readRangedFloat32(float& value, float min, float max,
                                    float scale)
{
    (void)value; (void)min; (void)max; (void)scale;
    return true;
}

bool bdBitBuffer::readInt32(int& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readInt16(short& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readUInt32(unsigned int& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readUChar8(unsigned char& value)
{
    (void)value;
    return true;
}

void bdBitBuffer::writeInt16(short value)
{
    (void)value;
}

void bdBitBuffer::writeUInt32(unsigned int value)
{
    (void)value;
}

void bdBitBuffer::writeChar8(char value)
{
    (void)value;
}

void bdBitBuffer::writeUChar8(unsigned char value)
{
    (void)value;
}

void bdBitBuffer::writeBlob(const void* blob, unsigned int length)
{
    (void)blob;
    (void)length;
}

bool bdBitBuffer::readUInt16(unsigned short& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readChar8(char& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readFloat32(float& value)
{
    (void)value;
    return true;
}

bool bdBitBuffer::readString(bdString& s)
{
    (void)s;
    return true;
}

bool bdBitBuffer::readString(char* s, unsigned int maxLen)
{
    (void)s;
    (void)maxLen;
    return true;
}

bool bdBitBuffer::testBool()
{
    return true;
}

bool bdByteBuffer::read(void* data, unsigned int size)
{
    (void)data;
    (void)size;
    return true;
}

bool bdBitBuffer::readDataType(bdBitBufferDataType type)
{
    (void)type;
    return true;
}

bool bdBitBuffer::readBits(void* data, unsigned int bitCount)
{
    (void)data; (void)bitCount;
    return true;
}

// bdMemory (bdCore:bdMemory.obj; stubs - real impl delegates to malloc hooks)
void* bdMemory::allocate(unsigned int size)
{
    (void)size;
    return nullptr;
}
void bdMemory::deallocate(void* p)
{
    (void)p;
}
void bdMemory::setAllocateFunc(void* (*func)(unsigned int size))
{
    (void)func;
}
void bdMemory::setAlignedAllocateFunc(void* (*func)(unsigned int size,
                                                    unsigned int align))
{
    (void)func;
}
void bdMemory::setDeallocateFunc(void (*func)(void* p))
{
    (void)func;
}
void bdMemory::setAlignedDeallocateFunc(void (*func)(void* p))
{
    (void)func;
}
void bdMemory::setReallocateFunc(void* (*func)(void* p, unsigned int size))
{
    (void)func;
}
void bdMemory::setAlignedReallocateFunc(void* (*func)(void* p,
                                                      unsigned int size,
                                                      unsigned int align))
{
    (void)func;
}

#define COD3_UNIMPLEMENTED(lib) \
    fprintf(stderr, "COD3 UNIMPLEMENTED: %s\n", lib)

void __cod3_stub_bd(void) {
    COD3_UNIMPLEMENTED("bd");
}

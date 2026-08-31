// Bandwidth/discovery compatibility translation unit. Implementations live
// in the owning bd sources.

#include <stdio.h>
#include <math.h>

#include "bd_types.h"
#include "bd/bdUtilities/bdBitOperations.h"

extern const char defaultFileName[];

// ea: 0x0089AFA0
unsigned int bdStrlen(const char* value)
{
    return (unsigned int)strlen(value);
}

// ea: 0x0089AFE0
float bdFabsf32(float value)
{
    return (float)fabs(value);
}

// bdLogSubscriber — base logging callbacks (bdCore @ 0x89D240).
bdLogSubscriber::bdLogSubscriber() = default;
bdLogSubscriber::~bdLogSubscriber() = default;

void bdLogSubscriber::publish(const char* channel, const char* file,
                               const char* function, unsigned int line,
                               const char* message)
{
    char output[2048];
    const char* slash = strrchr(file, '\\');
    unsigned int offset = slash != nullptr ? (unsigned int)(slash - file + 1) : 0;
    if (strstr(channel, "info") == channel)
    {
        bdSnprintf(output, sizeof(output), "%s(%u): %s\n\tINFO: %s\n",
                   file + offset, line, function, message);
        OutputDebugStringA(output);
    }
    else if (strstr(channel, "warn") == channel)
    {
        bdSnprintf(output, sizeof(output), "%s(%u): %s\n\tWARNING: %s\n",
                   file + offset, line, function, message);
        OutputDebugStringA(output);
    }
    else if (strstr(channel, "err") == channel)
    {
        bdSnprintf(output, sizeof(output), "%s(%u): %s\n\tERROR: %s\n",
                   file + offset, line, function, message);
        OutputDebugStringA(output);
    }
    else
    {
        OutputDebugStringA("bdLogSubscriber::publish: invalid channel name!\n");
        DebugBreak();
    }
}

// IDA globals from bd.o.
int g_NumBdMessages = 0;
bool g_assertFalse = false;

bdMessageProxy::bdMessageProxy(const char* file, const char* func,
                               unsigned int line, const char* flags)
    : m_file(file), m_function(func), m_line(line), m_baseChannel(flags)
{
}

namespace {
struct bdEmptyStringStorage {
    bdStringData data;
    char string[1];
};
bdEmptyStringStorage g_emptyStringStorage = {{1, 0, 0}, {0}};
}

// ea: 0x0089CB30
bdString::bdString()
    : m_string(g_emptyStringStorage.string)
{
    ++g_emptyStringStorage.data.m_referenceCount;
}

// ea: 0x0089CB50
bdString::bdString(const char* const value)
{
    const unsigned int length = (unsigned int)strlen(value);
    if (length != 0)
    {
        const unsigned int capacity = ((length + 1 + 0x3F) >> 6) << 6;
        char* storage = (char*)bdMemory::allocate(capacity + sizeof(bdStringData));
        bdStringData* data = (bdStringData*)storage;
        data->m_referenceCount = 1;
        data->m_length = length;
        data->m_capacity = capacity;
        m_string = storage + sizeof(bdStringData);
        memcpy(m_string, value, length + 1);
    }
    else
    {
        m_string = g_emptyStringStorage.string;
        ++g_emptyStringStorage.data.m_referenceCount;
    }
}

// ea: 0x0089CAA0
bdString::bdString(const bdString& value)
    : m_string(value.m_string)
{
    ++*((unsigned int*)m_string - 3);
}

// ea: 0x0089CC30
bdString::~bdString()
{
    unsigned int* referenceCount = (unsigned int*)m_string - 3;
    if (--*referenceCount == 0)
        bdMemory::deallocate(referenceCount);
}

bdString& bdString::operator=(const char* value)
{
    const unsigned int length = (unsigned int)strlen(value);
    bdStringData* data = (bdStringData*)m_string - 1;
    if (data->m_referenceCount > 1 || data->m_capacity < length + 1)
    {
        if (--data->m_referenceCount == 0)
            bdMemory::deallocate(data);

        unsigned int capacity = (length + 1) >> 6;
        if ((length + 1) & 0x3Fu)
            ++capacity;
        capacity <<= 6;

        data = (bdStringData*)bdMemory::allocate(capacity + sizeof(bdStringData));
        data->m_referenceCount = 1;
        data->m_length = length;
        data->m_capacity = capacity;
        m_string = (char*)(data + 1);
    }
    else
    {
        data->m_length = length;
    }

    memcpy(m_string, value, length + 1);
    return *this;
}

// ea: 0x0089CA50
const char* bdString::getBuffer() const
{
    return m_string;
}

namespace bdBytePacker {

// 0x0089EC80 / 0x0089ED40 / 0x0089ECE0 / 0x0089EDA0 (bdCore)
// ea: 0x0089EC80
bool appendBasicType(void* dest, unsigned int destSize, unsigned int offset,
                     unsigned int* newOffset, const void* value,
                     unsigned int valueSize)
{
    *newOffset = offset;
    if (offset >= destSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (destSize - offset >= valueSize)
    {
        if (dest != nullptr)
            memcpy((unsigned char*)dest + offset, value, valueSize);
        *newOffset = offset + valueSize;
        return true;
    }
    return false;
}

// ea: 0x0089ECE0
bool appendBuffer(void* dest, unsigned int destSize, unsigned int offset,
                  unsigned int* newOffset, const unsigned char* src,
                  unsigned int size)
{
    *newOffset = offset;
    if (offset >= destSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (destSize - offset >= size)
    {
        if (dest != nullptr)
            memcpy((unsigned char*)dest + offset, src, size);
        *newOffset = offset + size;
        return true;
    }
    return false;
}

// ea: 0x0089ED40
bool removeBasicType(const unsigned char* src, unsigned int srcSize,
                     unsigned int offset, unsigned int* newOffset,
                     void* value, unsigned int valueSize)
{
    *newOffset = offset;
    if (offset >= srcSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (srcSize - offset >= valueSize)
    {
        if (src != nullptr)
            memcpy(value, src + offset, valueSize);
        *newOffset = offset + valueSize;
        return true;
    }
    return false;
}

// ea: 0x0089EDA0
bool removeBuffer(const unsigned char* src, unsigned int srcSize,
                  unsigned int offset, unsigned int* newOffset, unsigned char* dest,
                  unsigned int size)
{
    *newOffset = offset;
    if (offset >= srcSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (srcSize - offset >= size)
    {
        if (src != nullptr)
            memcpy(dest, src + offset, size);
        *newOffset = offset + size;
        return true;
    }
    return false;
}

// ea: 0x0089EE00
bool skipBytes(const void* src, unsigned int srcSize, unsigned int offset,
               unsigned int* newOffset, unsigned int size)
{
    (void)src;
    *newOffset = offset;
    if (offset >= srcSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (srcSize - offset >= size)
    {
        *newOffset = offset + size;
        return true;
    }
    return false;
}

// ea: 0x0089EE40
bool rewindBytes(const void* src, unsigned int srcSize, unsigned int offset,
                 unsigned int* newOffset, unsigned int size)
{
    (void)src;
    *newOffset = offset;
    if (offset >= srcSize)
    {
        while (g_assertFalse)
            ;
    }
    else if (offset >= size)
    {
        *newOffset = offset - size;
        return true;
    }
    return false;
}

// 0x0089EEF0 / 0x0089F000 (bdCore)
// ea: 0x0089EEF0
bool appendEncodedUInt16(void* dest, unsigned int destSize,
                         unsigned int offset, unsigned int* newOffset,
                         unsigned short value)
{
    unsigned short v = value;
    if (value <= 0x7Fu)
        return appendBasicType(dest, destSize, offset, newOffset, &value, 1);
    unsigned char first = (unsigned char)((v >> 8) | 0x80);
    if (!appendBasicType(dest, destSize, offset, newOffset, &first, 1))
        return false;
    unsigned int next = *newOffset;
    unsigned char second = (unsigned char)v;
    if (appendBasicType(dest, destSize, next, newOffset, &second, 1))
        return true;
    return false;
}

// ea: 0x0089EFD0
bool removeEncodedUInt16(const unsigned char* src, unsigned int srcSize,
                         unsigned int offset, unsigned int* newOffset,
                         unsigned short* value)
{
    unsigned char first = 0;
    if (!removeBasicType(src, srcSize, offset, newOffset, &first, 1))
    {
        *value = first;
        return false;
    }
    if (first < 0x80u)
    {
        *value = first;
        return true;
    }
    unsigned int next = *newOffset;
    unsigned char second = 0;
    bool ok = removeBasicType(src, srcSize, next, newOffset, &second, 1);
    *value = (unsigned short)(((unsigned short)(first & 0x7Fu) << 8)
                              + (ok ? second : 0));
    return ok;
}

}

// ea: 0x008A0670
// bdByteBuffer ctor - ?0bdByteBuffer@@QAE@I@Z (bdCore:bdByteBuffer.o)
bdByteBuffer::bdByteBuffer(unsigned int size)
{
    m_size = size;
    m_data = static_cast<uint8_t*>(bdMemory::allocate(size));
    m_readPtr = m_data;
    m_writePtr = m_data;
}

// ea: 0x0089E430
bool bdByteBuffer::write(const void* data, unsigned int size)
{
    const unsigned int available = (unsigned int)((m_data + m_size) - m_writePtr);
    if (size > available)
    {
        bdMessageProxy proxy(".\\bdContainers\\bdByteBuffer.cpp",
                             "bool __thiscall bdByteBuffer::write(const void *,unsigned int)",
                             0x20u, "dw/err/");
        proxy.log("err", "Could not write data to buffer. Insufficient space.\n");
        do
        {
            if (!g_assertFalse)
                break;
            bdMessageProxy retry(".\\bdContainers\\bdByteBuffer.cpp",
                                 "bool __thiscall bdByteBuffer::write(const void *,unsigned int)",
                                 0x21u, "dw/err");
            retry.log(defaultFileName,
                      "Could not write data to buffer. Insufficient space.\n");
        }
        while (g_assertFalse);
        return false;
    }

    const bool result = bdBytePacker::appendBuffer(m_writePtr, available, 0,
                                                   &size,
                                                   reinterpret_cast<const unsigned char*>(data), size);
    m_writePtr += size;
    return result;
}

// ea: 0x0089BFC0
// bdBitBuffer ctors - ?0bdBitBuffer@@QAE@PBI_N@Z / ?0bdBitBuffer@@QAE@I_N@Z
bdBitBuffer::bdBitBuffer(const unsigned char* data, unsigned int bitCount,
                         bool typeChecked)
{
    m_data.m_data = nullptr;
    m_data.m_capacity = 0;
    m_data.m_size = 0;
    m_writePosition = 0;
    m_maxWritePosition = 0;
    m_readPosition = 0;
    m_failedRead = false;
    m_typeChecked = false;

    if (!typeChecked || bitCount == 0)
    {
        unsigned char header = 0;
        writeBits(&header, 1);
        writeBits(data, bitCount);
        m_readPosition = 1;
    }
    else
    {
        m_data.pushBack(data, (bitCount >> 3) + ((bitCount & 7u) != 0));
        m_writePosition = bitCount;
        m_maxWritePosition = bitCount;
        do
        {
            if (!readBits(&m_typeChecked, 1))
            {
                bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                     "__thiscall bdBitBuffer::bdBitBuffer(const unsigned char *,const unsigned int,const bool)",
                                     0x46u, "dw/err");
                proxy.log(defaultFileName,
                          "bdBitBuffer constructor failed: could not read bits");
            }
        } while (g_assertFalse);
    }
}

// ea: 0x0089BEC0
bdBitBuffer::bdBitBuffer(unsigned int bitCount, bool typeChecked)
{
    const unsigned int bytes = (bitCount >> 3) + ((bitCount & 7u) != 0);
    m_data.m_data = static_cast<unsigned char*>(bdMemory::allocate(bytes));
    m_data.m_capacity = bytes;
    m_data.m_size = 0;
    m_writePosition = 0;
    m_maxWritePosition = 0;
    m_readPosition = 0;
    m_failedRead = false;
    m_typeChecked = typeChecked;

    unsigned char header = typeChecked ? 0xFFu : 0u;
    writeBits(&header, 1);
    do
    {
        if (m_writePosition != 1)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "__thiscall bdBitBuffer::bdBitBuffer(const unsigned int,const bool)",
                                 0x26u, "dw/err");
            proxy.log(defaultFileName,
                      "BD_BB_NUM_HEADER_BITS and written header don't match.");
        }
    } while (g_assertFalse);
    m_readPosition = 1;
}

// ea: 0x0089B500
bdBitBuffer::~bdBitBuffer()
{
    // The release destructor clears the embedded fast-array after releasing
    // its storage; the member destructor then observes a null data pointer.
    bdMemory::deallocate(m_data.m_data);
    m_data.m_data = NULL;
    m_data.m_size = 0;
    m_data.m_capacity = 0;
}

// ea: 0x0089AFC0
unsigned int bdBitBuffer::getReadPosition() const
{
    return m_readPosition;
}

// ea: 0x0089AFD0
void bdBitBuffer::setReadPosition(unsigned int position)
{
    m_readPosition = position;
}

// ea: 0x0089AFF0
void bdBitBuffer::setTypeCheck(bool typeChecked)
{
    m_typeChecked = typeChecked;
}

// ea: 0x0089B000
bool bdBitBuffer::getTypeCheck() const
{
    return m_typeChecked;
}

// ea: 0x0089BC10
bdBitBuffer::bdBitBufferDataType bdBitBuffer::readDataType()
{
    unsigned int value = 0;
    if (!readRangedUInt32(value, 0, 0x1Fu, false))
        return BD_BB_NO_TYPE;
    return (bdBitBufferDataType)value;
}

// bdBitBuffer IO primitives (bdCore:bdBitBuffer.obj), reconstructed from the
// generated release dump (writeBits 0x89BCF0, readBits 0x89B2F0).
// ea: 0x0089BCF0
void bdBitBuffer::writeBits(const void* data, unsigned int bitCount)
{
    unsigned int writePosition = m_writePosition;
    unsigned int lastByte = (bitCount + writePosition - 1) >> 3;
    if (lastByte >= m_data.m_size)
    {
        unsigned int capacity = m_data.m_capacity;
        unsigned int newSize = lastByte + 1;
        if (capacity < newSize)
            m_data.increaseCapacity(newSize - capacity);
        m_data.m_size = newSize;
        m_data.m_data[lastByte] = 0;
    }

    const unsigned char* source = (const unsigned char*)data;
    unsigned int remaining = bitCount;
    unsigned int sourceLastByte = (bitCount - 1) >> 3;
    while (remaining != 0)
    {
        unsigned int bitOffset = m_writePosition & 7;
        unsigned int chunk = remaining;
        if (chunk >= 8 - bitOffset)
            chunk = 8 - bitOffset;

        unsigned int byteIndex = m_writePosition >> 3;
        unsigned char mask = (unsigned char)((255u >> (8 - bitOffset)) |
                                             (~0u << (bitOffset + chunk)));
        unsigned char oldValue = (unsigned char)(mask & m_data.m_data[byteIndex]);
        unsigned int sourceByte = (bitCount - remaining) >> 3;
        unsigned char nextValue = 0;
        if (sourceLastByte > sourceByte)
            nextValue = source[sourceByte + 1];
        unsigned int sourceBit = (bitCount - remaining) & 7;
        unsigned char value = source[sourceByte];
        m_data.m_data[byteIndex] = (unsigned char)(oldValue |
            (unsigned char)(~mask & (unsigned char)(
                ((unsigned int)((nextValue << (8 - sourceBit)) |
                                (value >> sourceBit))) << bitOffset)));

        unsigned int nextPosition = chunk + m_writePosition;
        unsigned int consumed = remaining - chunk;
        if (m_maxWritePosition < nextPosition)
            m_maxWritePosition = nextPosition;
        m_writePosition = nextPosition;
        remaining = consumed;
    }
}

// ea: 0x0089BE50
bool bdBitBuffer::append(bdBitBuffer& other)
{
    if (m_typeChecked != other.m_typeChecked)
        return false;

    unsigned int savedReadPosition = other.m_readPosition;
    other.m_readPosition = 1;
    void* data = bdMemory::allocate(other.m_data.m_size);
    bool result = other.readBits(data, other.m_maxWritePosition - 1);
    if (result)
        writeBits(data, other.m_maxWritePosition - 1);
    bdMemory::deallocate(data);
    other.m_readPosition = savedReadPosition;
    return result;
}

// ea: 0x0089C1D0
void bdBitBuffer::writeDataType(bdBitBufferDataType type)
{
    if (m_typeChecked)
        writeRangedUInt32((unsigned int)type, 0, 0x1Fu, false);
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

// ea: 0x0089C2F0
void bdBitBuffer::writeRangedFloat32(float value, float min, float max,
                                     float scale)
{
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "void __thiscall bdBitBuffer::writeRangedFloat32(const float,const float,const float,const float)",
                                 0x97u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
        }
    } while (g_assertFalse);
    do
    {
        if (scale <= 0.0f)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "void __thiscall bdBitBuffer::writeRangedFloat32(const float,const float,const float,const float)",
                                 0x98u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedFloat32, precision must be positive.");
        }
    } while (g_assertFalse);

    if (m_typeChecked)
    {
        writeRangedUInt32(0xFu, 0, 0x1Fu, false);
        if (m_typeChecked)
        {
            writeRangedUInt32(0xDu, 0, 0x1Fu, false);
            writeBits(&min, 0x20u);
            if (m_typeChecked)
                writeRangedUInt32(0xDu, 0, 0x1Fu, false);
            writeBits(&max, 0x20u);
            if (m_typeChecked)
                writeRangedUInt32(0xDu, 0, 0x1Fu, false);
            writeBits(&scale, 0x20u);
        }
    }

    const float precision = (float)fabs(scale);
    const float scaledRange = (max - min) / precision;
    if (scaledRange <= 4294967300.0f)
    {
        unsigned int bitCount = 0;
        const unsigned int range = (unsigned int)scaledRange;
        if (range != 0)
            bitCount = bdHighBitNumber(range) + 1;

        float clamped = value;
        if (clamped < min)
            clamped = min;
        else if (clamped > max)
            clamped = max;
        unsigned int encoded = (unsigned int)(((clamped - min) / precision) + 0.5f);
        if (encoded > range)
            encoded = range;
        writeBits(&encoded, bitCount);
    }
    else
    {
        bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                             "void __thiscall bdBitBuffer::writeRangedFloat32(const float,const float,const float,const float)",
                             0xA9u, "dw/warn/");
        proxy.log("bdCore/bitBuffer",
                  "The numerical space defined by range/precision combination is too large. No compression performed.");
        if (m_typeChecked)
            writeRangedUInt32(0xDu, 0, 0x1Fu, false);
        writeBits(&value, 0x20u);
    }
}

// ea: 0x0089B710
bool bdBitBuffer::readRangedInt32(int& value, int min, int max)
{
    const int expectedMin = min;
    const int expectedMax = max;
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedInt32(int &,const int,const int)",
                                 0x140u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedInt32, end of range is less than the begining");
        }
    } while (g_assertFalse);

    bool result = readDataType(BD_BB_RANGED_SIGNED_INTEGER32_TYPE);
    if (m_typeChecked)
    {
        int encodedMax = 0;
        int encodedMin = 0;
        if (!result ||
            !readDataType(BD_BB_SIGNED_INTEGER32_TYPE) ||
            !readBits(&encodedMax, 0x20u) ||
            !readDataType(BD_BB_SIGNED_INTEGER32_TYPE) ||
            !readBits(&encodedMin, 0x20u))
            return false;
        if (expectedMin != encodedMin || expectedMax != encodedMax)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedInt32(int &,const int,const int)",
                                 0x14Eu, "dw/err/");
            proxy.log("bdCore/bitBuffer", "Range error. Expected: (%i,%i), read: (%i,%i)",
                      expectedMin, expectedMax, encodedMin, encodedMax);
        }
    }
    else if (!result)
    {
        return false;
    }

    unsigned int bitCount = 0;
    if (expectedMax != expectedMin)
        bitCount = bdHighBitNumber((unsigned int)(expectedMax - expectedMin)) + 1;
    unsigned int encoded = 0;
    if (!readBits(&encoded, bitCount))
        return false;
    value = expectedMin + (int)encoded;
    do
    {
        if (value < expectedMin || value > expectedMax)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedInt32(int &,const int,const int)",
                                 0x162u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::readRangedInt32, read error i is out of range.");
        }
    } while (g_assertFalse);
    if (value < expectedMin)
        value = expectedMin;
    else if (value > expectedMax)
        value = expectedMax;
    return true;
}

// ea: 0x0089B560
bool bdBitBuffer::readRangedUInt32(unsigned int& value, unsigned int min,
                                   unsigned int max, bool typeChecked)
{
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedUInt32(unsigned int &,const unsigned int,const unsigned int,const bool)",
                                 0x10Cu, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedUInt, end of range is less than the begining");
        }
    } while (g_assertFalse);

    bool result = true;
    if (typeChecked)
    {
        result = readDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
        if (m_typeChecked)
        {
            unsigned int encodedMin = 0;
            unsigned int encodedMax = 0;
            if (!result || !readUInt32(encodedMin) || !readUInt32(encodedMax))
                return false;
            if (encodedMin != min || encodedMax != max)
            {
                bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                     "bool __thiscall bdBitBuffer::readRangedUInt32(unsigned int &,const unsigned int,const unsigned int,const bool)",
                                     0x11Fu, "dw/err/");
                proxy.log("bdCore/bitBuffer",
                          "Range error. Expected: (%u,%u), read: (%u,%u)",
                          min, max, encodedMin, encodedMax);
            }
        }
        else if (!result)
        {
            return false;
        }
    }

    unsigned int bitCount = 0;
    if (max != min)
        bitCount = bdHighBitNumber(max - min) + 1;
    unsigned int encoded = 0;
    if (!readBits(&encoded, bitCount))
        return false;
    value = min + encoded;
    do
    {
        if (value < min || value > max)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedUInt32(unsigned int &,const unsigned int,const unsigned int,const bool)",
                                 0x133u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::readRangedUInt32, read error u is out of range.");
        }
    } while (g_assertFalse);
    do
    {
        if (value < min || value > max)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedFloat32(float &,const float,const float,const float)",
                                 0x1A5u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::readRangedFloat32, read error f is out of range.");
        }
    } while (g_assertFalse);
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return true;
}

// ea: 0x89B8D0
bool bdBitBuffer::readRangedFloat32(float& value, float min, float max,
                                    float scale)
{
    do
    {
        if (max < min)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedFloat32(float &,const float,const float,const float)",
                                 0x170u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedFloat32, end of range is less then the begining.");
        }
    } while (g_assertFalse);
    do
    {
        if (scale <= 0.0f)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedFloat32(float &,const float,const float,const float)",
                                 0x171u, "dw/err");
            proxy.log(defaultFileName,
                      "bdBitBuffer::writeRangedFloat32, precision must be positive.");
        }
    } while (g_assertFalse);

    bool result = readDataType(BD_BB_RANGED_FLOAT32_TYPE);
    if (m_typeChecked)
    {
        float encodedMin = 0.0f;
        float encodedMax = 0.0f;
        float encodedScale = 0.0f;
        if (!result ||
            !readDataType(BD_BB_FLOAT32_TYPE) || !readBits(&encodedMin, 0x20u) ||
            !readDataType(BD_BB_FLOAT32_TYPE) || !readBits(&encodedMax, 0x20u) ||
            !readDataType(BD_BB_FLOAT32_TYPE) || !readBits(&encodedScale, 0x20u))
            return false;
        if (min != encodedMin || max != encodedMax || scale != encodedScale)
        {
            bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                 "bool __thiscall bdBitBuffer::readRangedFloat32(float &,const float,const float,const float)",
                                 0x181u, "dw/err/");
            proxy.log("bdCore/bitBuffer",
                      "Range error. Expected: (%f,%f,%f), read: (%f,%f,%f)",
                      min, max, scale, encodedMin, encodedMax, encodedScale);
        }
    }
    else if (!result)
    {
        return false;
    }

    const float precision = (float)fabs(scale);
    const float scaledRange = (max - min) / precision;
    if (scaledRange <= 4294967300.0f)
    {
        const unsigned int range = (unsigned int)scaledRange;
        unsigned int bitCount = 0;
        if (range != 0)
            bitCount = bdHighBitNumber(range) + 1;
        unsigned int encoded = 0;
        if (!readBits(&encoded, bitCount))
            return false;
        value = (float)encoded * precision + min;
    }
    else
    {
        bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                             "bool __thiscall bdBitBuffer::readRangedFloat32(float &,const float,const float,const float)",
                             0x18Cu, "dw/warn/");
        proxy.log("bdCore/bitBuffer",
                  "The numerical space defined by range/precision combination is too large. No compression performed.");
        if (!readDataType(BD_BB_FLOAT32_TYPE) || !readBits(&value, 0x20u))
            return false;
    }

    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return true;
}

bool bdBitBuffer::readInt16(short& value)
{
    short decoded;
    if (!readDataType(BD_BB_SIGNED_INTEGER16_TYPE) ||
        !readBits(&decoded, 0x10u))
        return false;
    value = decoded;
    return true;
}

bool bdBitBuffer::readUInt32(unsigned int& value)
{
    unsigned int decoded;
    if (!readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE) ||
        !readBits(&decoded, 0x20u))
        return false;
    value = decoded;
    return true;
}

bool bdBitBuffer::readUChar8(unsigned char& value)
{
    return readDataType(BD_BB_UNSIGNED_CHAR8_TYPE) &&
           readBits(&value, 8u);
}

void bdBitBuffer::writeInt16(short value)
{
    int encoded = value;
    writeDataType(BD_BB_SIGNED_INTEGER16_TYPE);
    writeBits(&encoded, 0x10u);
}

void bdBitBuffer::writeUInt16(unsigned short value)
{
    int encoded = value;
    writeDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
    writeBits(&encoded, 0x10u);
}

void bdBitBuffer::writeInt32(int value)
{
    int encoded = value;
    writeDataType(BD_BB_SIGNED_INTEGER32_TYPE);
    writeBits(&encoded, 0x20u);
}

void bdBitBuffer::writeUInt32(unsigned int value)
{
    unsigned int encoded = value;
    writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    writeBits(&encoded, 0x20u);
}

void bdBitBuffer::writeFloat32(float value)
{
    float encoded = value;
    writeDataType(BD_BB_FLOAT32_TYPE);
    writeBits(&encoded, 0x20u);
}

void bdBitBuffer::writeChar8(char value)
{
    writeDataType(BD_BB_SIGNED_CHAR8_TYPE);
    writeBits(&value, 8u);
}

void bdBitBuffer::writeUChar8(unsigned char value)
{
    writeDataType(BD_BB_UNSIGNED_CHAR8_TYPE);
    writeBits(&value, 8u);
}

void bdBitBuffer::writeBlob(const void* blob, unsigned int length)
{
    writeDataType(BD_BB_BLOB_TYPE);
    writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    unsigned int encodedLength = length;
    writeBits(&encodedLength, 0x20u);
    writeBits(blob, 8 * length);
}

bool bdBitBuffer::readUInt16(unsigned short& value)
{
    unsigned short decoded;
    if (!readDataType(BD_BB_UNSIGNED_INTEGER16_TYPE) ||
        !readBits(&decoded, 0x10u))
        return false;
    value = decoded;
    return true;
}

bool bdBitBuffer::readInt32(int& value)
{
    int decoded;
    if (!readDataType(BD_BB_SIGNED_INTEGER32_TYPE) ||
        !readBits(&decoded, 0x20u))
        return false;
    value = decoded;
    return true;
}

bool bdBitBuffer::readFloat32(float& value)
{
    float decoded;
    if (!readDataType(BD_BB_FLOAT32_TYPE) ||
        !readBits(&decoded, 0x20u))
        return false;
    value = decoded;
    return true;
}

bool bdBitBuffer::readChar8(char& value)
{
    return readDataType(BD_BB_SIGNED_CHAR8_TYPE) &&
           readBits(&value, 8u);
}

bool bdBitBuffer::readString(bdString& s)
{
    bool result = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    if (!result)
        return false;

    unsigned int capacity = 32;
    unsigned int length = 0;
    char* buffer = (char*)bdMemory::allocate(capacity);
    do
    {
        char value = 0;
        bool ok = readBits(&value, 8u);
        if (length == capacity)
        {
            unsigned int newCapacity = capacity ? capacity + capacity : 1;
            char* newBuffer = (char*)bdMemory::allocate(newCapacity);
            if (length != 0)
                memcpy(newBuffer, buffer, length);
            bdMemory::deallocate(buffer);
            buffer = newBuffer;
            capacity = newCapacity;
        }
        buffer[length++] = value;
        result = ok;
        if (!ok || value == 0)
            break;
    }
    while (true);

    s = buffer;
    bdMemory::deallocate(buffer);
    return result;
}

bool bdBitBuffer::readString(char* s, unsigned int maxLen)
{
    bool result = readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    if (!result)
        return false;

    unsigned int index = 0;
    char value = 0;
    do
    {
        result = readBits(&value, 8u);
        if (index < maxLen)
            s[index++] = value;
    }
    while (result && value != 0);

    if (maxLen != 0)
        s[maxLen - 1] = 0;
    return result;
}

bool bdBitBuffer::testBool()
{
    char value = 0;
    if (readDataType(BD_BB_BOOL_TYPE) &&
        readBits(&value, 1u) && value != 0)
        return true;
    return false;
}

// ea: 0x0089E500
bool bdByteBuffer::read(void* data, unsigned int size)
{
    const unsigned int available = (unsigned int)((m_data + m_size) - m_readPtr);
    if (size > available)
    {
        bdMessageProxy proxy(".\\bdContainers\\bdByteBuffer.cpp",
                             "bool __thiscall bdByteBuffer::read(void *,unsigned int)",
                             0x32u, "dw/err/");
        proxy.log("err", "Could not read data from buffer. Insufficient data available.\n");
        do
        {
            if (!g_assertFalse)
                break;
            bdMessageProxy retry(".\\bdContainers\\bdByteBuffer.cpp",
                                 "bool __thiscall bdByteBuffer::read(void *,unsigned int)",
                                 0x33u, "dw/err");
            retry.log(defaultFileName,
                      "Could not read data from buffer. Insufficient data available.\n");
        }
        while (g_assertFalse);
        return false;
    }

    unsigned int newSize = size;
    const bool result = bdBytePacker::removeBuffer(m_readPtr, available, 0,
                                                   &newSize,
                                                   (unsigned char*)data, size);
    m_readPtr += size;
    return result;
}

// ea: 0x0089E5D0
bdByteBuffer::~bdByteBuffer()
{
    if (m_data != nullptr)
        bdMemory::deallocate(m_data);
}

// ea: 0x0089B0D0
void bdBitBuffer::typeToString(bdBitBufferDataType type, char* const buffer,
                               unsigned int bufferSize)
{
    static const char* const names[] = {
        "NoType", "Bool", "Char8", "UChar8", "WChar16", "Int16",
        "UInt16", "Int32", "UInt32", "Int64", "UInt64", "RangedInt32",
        "RangeUInt32", "Float32", "Float64", "RangeFloat32", "String",
        "String", "MultiByteString", "Blob", "FullType", "Unknown Type"
    };
    int index = (int)type;
    if (index < 0)
        index = BD_BB_NO_TYPE;
    else if (index > 21)
        index = BD_BB_FULL_TYPE | BD_BB_BOOL_TYPE;

    unsigned int length = (unsigned int)strlen(names[index]);
    if (bufferSize != 0)
    {
        if (length >= bufferSize - 1)
            length = bufferSize - 1;
        memcpy(buffer, names[index], length);
        buffer[length] = 0;
    }
}

// ea: 0x0089B400
bool bdBitBuffer::readDataType(bdBitBufferDataType type)
{
    bool result = true;
    if (m_typeChecked)
    {
        unsigned int actual = 0;
        result = readRangedUInt32(actual, 0, 0x1Fu, false);
        if (result)
        {
            bool matches = actual == (unsigned int)type;
            if (!matches)
            {
                char expected[40];
                char read[40];
                typeToString(type, expected, 0x28u);
                typeToString((bdBitBufferDataType)actual, read, 0x28u);
                bdMessageProxy proxy(".\\bdContainers\\bdBitBuffer.cpp",
                                     "bool __thiscall bdBitBuffer::readDataType(const enum bdBitBufferDataType)",
                                     0x20Eu, "dw/err/");
                proxy.log("bdCore/bitBuffer", "Expected: %s , read: %s ",
                          expected, read);
            }
            return matches;
        }
    }
    return result;
}

// ea: 0x0089B2F0
bool bdBitBuffer::readBits(void* data, unsigned int bitCount)
{
    if (bitCount == 0)
        return true;

    if (bitCount + m_readPosition > m_maxWritePosition)
    {
        m_failedRead = true;
        return false;
    }

    unsigned char* destination = (unsigned char*)data;
    unsigned int remaining = bitCount;
    unsigned int byteIndex = m_readPosition >> 3;
    while (byteIndex < m_data.m_size)
    {
        unsigned int chunk = remaining;
        if (chunk >= 8)
            chunk = 8;
        unsigned char value = m_data.m_data[byteIndex];
        unsigned int bitOffset = m_readPosition & 7;
        ++byteIndex;
        unsigned char output;
        if (bitOffset + chunk <= 8)
        {
            output = (unsigned char)((value >> bitOffset) & (255u >> (8 - chunk)));
        }
        else
        {
            if (byteIndex >= m_data.m_size)
                break;
            output = (unsigned char)((255u >> (8 - chunk)) &
                ((value >> bitOffset) | (m_data.m_data[byteIndex] << (8 - bitOffset))));
        }
        *destination++ = output;
        m_readPosition += chunk;
        remaining -= chunk;
        if (remaining == 0)
            return true;
    }

    m_failedRead = true;
    return false;
}

// Bandwidth/discovery compatibility translation unit. Implementations live
// in the owning bd sources.

#include <stdio.h>

#include "bd_types.h"
#include "bd/bdUtilities/bdBitOperations.h"

extern const char defaultFileName[];

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
bdString::bdString(const char* value)
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

// 0x0089EEF0 / 0x0089F000 (bdCore)
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

// bdBitBuffer IO primitives (bdCore:bdBitBuffer.obj), reconstructed from the
// generated release dump (writeBits 0x89BCF0, readBits 0x89B2F0).
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

bool bdBitBuffer::readRangedInt32(int& value, int min, int max)
{
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
        if (!result ||
            !readDataType(BD_BB_SIGNED_INTEGER32_TYPE) ||
            !readBits(&max, 0x20u) ||
            !readDataType(BD_BB_SIGNED_INTEGER32_TYPE) ||
            !readBits(&min, 0x20u))
            return false;
    }
    else if (!result)
    {
        return false;
    }

    unsigned int bitCount = 0;
    if (max != min)
        bitCount = bdHighBitNumber((unsigned int)(max - min)) + 1;
    unsigned int encoded = 0;
    if (!readBits(&encoded, bitCount))
        return false;
    value = min + (int)encoded;
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return true;
}

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
    if (value < min)
        value = min;
    else if (value > max)
        value = max;
    return true;
}

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

bool bdBitBuffer::readDataType(bdBitBufferDataType type)
{
    if (!m_typeChecked)
        return true;

    unsigned int actual = 0;
    if (!readRangedUInt32(actual, 0, 0x1Fu, false))
        return false;
    return actual == (unsigned int)type;
}

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

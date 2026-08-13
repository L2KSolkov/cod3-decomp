// ============================================================================
// utility.cpp - mat3_t methods + idStr + dword copy (core.o)
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"

#include <string.h>

extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void Sys_OutOfMemError();

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ea: 0x004BB030
void _copyDWord(unsigned int* dest, unsigned int constant, unsigned int count)
{
    for (unsigned int i = 0; i < count; ++i)
        dest[i] = constant;
}

// ea: 0x004BC370
void mat3_t::ProjectVector(const idVec3& src, idVec3& dst) const
{
    dst.x = (this->mat[0].y * src.y) + (this->mat[0].z * src.z)
          + (src.x * this->mat[0].x);
    dst.y = (this->mat[1].y * src.y) + (this->mat[1].z * src.z)
          + (this->mat[1].x * src.x);
    dst.z = (this->mat[2].y * src.y) + (this->mat[2].z * src.z)
          + (this->mat[2].x * src.x);
}

// ea: 0x004BC400
void mat3_t::UnprojectVector(const idVec3& src, idVec3& dst) const
{
    float z = src.z;
    float x = src.x;
    float v5 = this->mat[2].y * z;
    float v6 = this->mat[2].x * z;
    float v7 = this->mat[2].z * z;
    float y = src.y;
    float srca = this->mat[1].y * y;
    float v11 = this->mat[1].x * y;
    float v9 = (this->mat[0].z * x) + (this->mat[1].z * y) + v7;
    float v10 = (this->mat[0].y * x) + srca + v5;
    dst.x = (this->mat[0].x * x) + v11 + v6;
    dst.y = v10;
    dst.z = v9;
}

// ea: 0x004BC620
mat3_t mat3_t::Inverse() const
{
    mat3_t inv;
    inv = *this;
    inv.Transpose();
    return inv;
}

// ea: 0x004BC660
void mat3_t::Clear()
{
    this->mat[0].x = 1.0f;
    this->mat[0].y = 0.0f;
    this->mat[0].z = 0.0f;
    this->mat[1].x = 0.0f;
    this->mat[1].y = 1.0f;
    this->mat[1].z = 0.0f;
    this->mat[2].x = 0.0f;
    this->mat[2].y = 0.0f;
    this->mat[2].z = 1.0f;
}

// idStr internal
struct strdata {
    int len;
    int refcount;
    char* data;
    int alloced;
};

class idStr {
public:
    strdata* m_data;
protected:
    void EnsureDataWritable();
    void EnsureAlloced(int amount, bool keepold);
};

// ea: 0x004C0880
void idStr::EnsureDataWritable()
{
    ASSERT("m_data", "c:\\cod\\code\\game\\util_str.cpp", 390);
    strdata* m_data = this->m_data;
    if (this->m_data->refcount != 0)
    {
        int len = m_data != nullptr ? m_data->len : 0;
        strdata* v3 = (strdata*)mem_heap_malloc(0x10u);
        if (v3 != nullptr)
        {
            v3->len = 0;
            v3->refcount = 0;
            v3->data = nullptr;
            v3->alloced = 0;
        }
        this->m_data = v3;
        this->EnsureAlloced(len + 1, false);
        strncpy(this->m_data->data, m_data->data, len + 1);
        this->m_data->len = len;
        int v4 = m_data->refcount - 1;
        m_data->refcount = v4;
        if (v4 < 0)
        {
            if (m_data->data != nullptr)
                mem_heap_free(m_data->data);
            mem_heap_free(m_data);
        }
    }
}

// ea: 0x004C0960
void idStr::EnsureAlloced(int amount, bool keepold)
{
    if (this->m_data == nullptr)
    {
        strdata* v4 = (strdata*)mem_heap_malloc(0x10u);
        if (v4 != nullptr)
        {
            v4->len = 0;
            v4->refcount = 0;
            v4->data = nullptr;
            v4->alloced = 0;
        }
        this->m_data = v4;
    }
    this->EnsureDataWritable();
    int alloced = this->m_data->alloced;
    int v6 = amount;
    bool v7 = alloced != 0;
    if (amount >= alloced)
    {
        if (amount != 0)
        {
            if (amount == 1)
            {
                this->m_data->alloced = 1;
                goto allocate;
            }
        }
        else
        {
            ASSERT("amount", "c:\\cod\\code\\game\\util_str.cpp", 425);
        }
        if (amount % 20 != 0)
            v6 = amount - amount % 20 + 20;
        this->m_data->alloced = v6;
    allocate:
        char* v8 = (char*)mem_heap_malloc(this->m_data->alloced);
        if (v7 && keepold)
            strcpy(v8, this->m_data->data);
        if (this->m_data->data != nullptr)
            mem_heap_free(this->m_data->data);
        this->m_data->data = v8;
    }
}

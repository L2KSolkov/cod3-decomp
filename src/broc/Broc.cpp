// ============================================================================
// COD3 Broc Engine Core — Broc.o port
// Reconstructed from codmp_xboxr.xbe (release build, /O2)
// Source: c:\cod\code\script\include\strings.inl (assert strings confirm)
// ============================================================================

#include "engine/broc_types.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <new>

// ============================================================================
// External dependencies (stubs)
// ============================================================================
static void* mem_alloc(unsigned int size, unsigned int) {
    extern void* COD3_mem_alloc(unsigned int, unsigned int);
    return COD3_mem_alloc(size, 4);
}
static void mem_free(void* ptr) {
    extern void COD3_mem_free(void*);
    COD3_mem_free(ptr);
}

extern int gNumStringsAlloc;
extern int gNumStringsFreed;
extern const float VectorDistance(const float* const v1, const float* const v2);
extern const float VectorDistanceSquared(const float* const p1,
                                         const float* const p2);
extern void FastSinCos(float radians, float* psin, float* pcos);

extern "C" int __fpclass(float);

// ============================================================================
// ae_pair — lightweight pair used by AllocBlock
// ============================================================================
namespace Broc {
template <typename A, typename B>
struct ae_pair {
    A first;
    B second;
};
}

using Broc::ae_pair;

// ============================================================================
// Global — IS_NAN (not in Broc namespace, per decompilation)
// ============================================================================

// ea: 0x004928960
bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// ============================================================================
// Broc utility functions
// ============================================================================
namespace Broc {

// ea: 0x004926420
bool IsAlpha(char ca) {
    if (ca >= 'a' && ca <= 'z')
        return true;
    return ca >= 'A' && ca <= 'Z';
}

// ea: 0x004926470
char ToLower(char ca) {
    if (ca < 'A' || ca > 'Z')
        return ca;
    return ca + 32;
}

// ea: 0x0049264B0
unsigned int length(const char* txta) {
    if (!txta)
        return 0;
    unsigned int len = 0;
    const char* cp = txta;
    while (*cp++)
        ++len;
    return len;
}

// ea: 0x005EE0D0
float length(const vector& v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

// ea: 0x005E9690
float length2(const vector* v) {
    return v->x * v->x + v->y * v->y + v->z * v->z;
}

// ea: 0x005E96C0
float dot(const vector* a, const vector* b) {
    return a->z * b->z + a->y * b->y + a->x * b->x;
}

// ea: 0x005E96F0
float VecDistance(const vector* v0, const vector* v1) {
    return VectorDistance(&v0->x, &v1->x);
}

// ea: 0x005E9710
float VecDistanceSquared(const vector* v0, const vector* v1) {
    return VectorDistanceSquared(&v0->x, &v1->x);
}

// ea: 0x005E9730
int VecCloser(const vector* vRef, const vector* vA, const vector* vB) {
    float fDistASqrd = VectorDistanceSquared(&vA->x, &vRef->x);
    return VectorDistanceSquared(&vB->x, &vRef->x) > fDistASqrd;
}

// ea: 0x005EF680
void MathFastSinCos(float fAng, float* sin, float* cos) {
    FastSinCos((fAng * 3.1415927f) * 0.0055555557f, sin, cos);
}

// ea: 0x0049287C0
int strcmp(const char* lhsa, const char* rhsa) {
    while (*lhsa && *rhsa) {
        char x = *lhsa++;
        char y = *rhsa++;
        if (x < y) return -1;
        if (x > y) return 1;
    }
    return 0;
}

// ============================================================================
// string::Block implementation
// ============================================================================

// ea: 0x0049263A0
/*static*/ char* string::Block::GetBuff(string::Block* block) {
    return (char*)(block + 1);
}

// ea: 0x004926B90 (inline)
void string::Block::IncrementCount(void) {
    ++this->mRefCount;
}

// ea: 0x004926730
void string::Block::DecrementCount(void) {
    if (--this->mRefCount <= 0) {
        mem_free(this);
        ++gNumStringsFreed;
    }
}

// ea: 0x004926510
string::Block::Block(unsigned short blockSize, unsigned short strLen, const char* txt) {
    mRefCount = 1;
    mLength = strLen;
    mBuff = GetBuff(this);
    mBlockSize = blockSize;

    if (txt) {
        char* dst = mBuff;
        const char* src = txt;
        while ((*dst++ = *src++) != '\0')
            ;
    } else {
        mBuff[0] = '\0';
    }
}

// ea: 0x004926630
void string::Block::Append(const char* txt, unsigned short strLen) {
    if (!txt)
        txt = "(null)";

    char* dst = GetBuff(this) + mLength;
    const char* src = txt;
    while ((*dst++ = *src++) != '\0')
        ;

    mLength += strLen;
}

// ============================================================================
// string::AllocBlock — static allocation helpers
// ============================================================================

// ea: 0x004926DA0 (inline)
int string::GetBlockSize(int hint) {
    int size = hint + 13;
    return (size + 3) & ~3;
}

// ea: 0x004926D20
static void string_AllocBlock_internal(ae_pair<void*, int>* result, int hint) {
    int finalSize = result->second + 12;
    finalSize = string::GetBlockSize(finalSize);
    ++gNumStringsAlloc;
    int usableSize = finalSize - 12;
    void* mem = mem_alloc(finalSize, 4);
    result->first = mem;
    result->second = usableSize;
}

// ea: 0x004926DC0
string::Block* string::AllocBlock(const char* txt, unsigned int txtLen,
                                   const char* sizeHint, unsigned int sizeHintLen) {
    if (!sizeHint)
        return NULL;

    int s = sizeHintLen ? (int)(sizeHintLen + 1) : (int)(txtLen + 1);

    ae_pair<void*, int> r;
    r.second = s;
    string_AllocBlock_internal(&r, (int)txtLen);

    string::Block* blk = (string::Block*)r.first;
    if (blk) {
        new (blk) Block((unsigned short)r.second, (unsigned short)txtLen, sizeHint);
    }
    return blk;
}

// ============================================================================
// string constructors
// ============================================================================

string::string() {
    mBlock = NULL;
}

string::string(EUndefined) {
    mBlock = NULL;
}

string::string(Block* b) {
    mBlock = b;
}

string::string(const char* txt) {
    unsigned int len = Broc::length(txt);
    mBlock = AllocBlock(txt, len, txt, len);
}

string::string(const char* txt, int) {
    unsigned int len = Broc::length(txt);
    mBlock = AllocBlock(txt, len, txt, len);
}

string::string(const string& rhs) {
    mBlock = NULL;
    if (rhs.mBlock) {
        mBlock = rhs.mBlock;
        mBlock->IncrementCount();
    }
}

string::string(const string& rhs, int) {
    mBlock = rhs.mBlock;
    if (mBlock) {
        mBlock->IncrementCount();
    }
}

string::string(unsigned int val) {
    char buf[16];
    sprintf(buf, "%u", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(int val) {
    char buf[16];
    sprintf(buf, "%d", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(float val) {
    char buf[32];
    sprintf(buf, "%g", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(const bint& val) {
    char buf[16];
    sprintf(buf, "%d", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(const bfloat& val) {
    char buf[32];
    sprintf(buf, "%g", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(const bunsigned& val) {
    char buf[16];
    sprintf(buf, "%u", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
}

string::string(const bbool& val) {
    const char* s = val.mVal ? "true" : "false";
    unsigned int len = Broc::length(s);
    mBlock = AllocBlock(s, len, s, len);
}

string::~string() {
    if (mBlock)
        mBlock->DecrementCount();
}

// ============================================================================
// string accessors
// ============================================================================

const char* string::c_str() const {
    if (!mBlock)
        return "";
    return mBlock->mBuff;
}

int string::length() const {
    if (!mBlock)
        return 0;
    return mBlock->mLength;
}

char* string::GetBuff() {
    if (!mBlock)
        return NULL;
    return mBlock->mBuff;
}

// ============================================================================
// string assignment operators
// ============================================================================

string& string::operator=(const string& rhs) {
    Block* oldBlock = mBlock;
    mBlock = rhs.mBlock;
    if (mBlock)
        mBlock->IncrementCount();
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(const char* txt) {
    Block* oldBlock = mBlock;
    if (txt) {
        unsigned int len = Broc::length(txt);
        mBlock = AllocBlock(txt, len, txt, len);
    } else {
        mBlock = NULL;
    }
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(char c) {
    Block* oldBlock = mBlock;
    char buf[2] = { c, '\0' };
    mBlock = AllocBlock(buf, 1, buf, 1);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(int val) {
    Block* oldBlock = mBlock;
    char buf[16];
    sprintf(buf, "%d", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(float val) {
    Block* oldBlock = mBlock;
    char buf[32];
    sprintf(buf, "%g", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(const vector& v) {
    Block* oldBlock = mBlock;
    char buf[64];
    sprintf(buf, "(%g %g %g)", v.x, v.y, v.z);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, buf, len);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

// ============================================================================
// string concatenation
// ============================================================================

string& string::operator+=(char c) {
    char buf[2] = { c, '\0' };
    Append(buf, 1);
    return *this;
}

string& string::operator+=(const char* txt) {
    if (txt) {
        unsigned int len = Broc::length(txt);
        Append(txt, len);
    }
    return *this;
}

string& string::operator+=(const string& rhs) {
    if (rhs.mBlock) {
        Append(rhs.mBlock->mBuff, rhs.mBlock->mLength);
    }
    return *this;
}

string& string::operator+=(int val) {
    char buf[16];
    sprintf(buf, "%d", val);
    Append(buf, Broc::length(buf));
    return *this;
}

string& string::operator+=(unsigned int val) {
    char buf[16];
    sprintf(buf, "%u", val);
    Append(buf, Broc::length(buf));
    return *this;
}

string& string::operator+=(float val) {
    char buf[32];
    sprintf(buf, "%g", val);
    Append(buf, Broc::length(buf));
    return *this;
}

// ============================================================================
// string operations
// ============================================================================

void string::Append(const char* txt, unsigned int len) {
    if (!txt)
        txt = "(null)";

    if (!mBlock) {
        mBlock = AllocBlock(txt, len, txt, len);
    } else if (mBlock->mRefCount > 1) {
        // Copy-on-write
        unsigned int oldLen = mBlock->mLength;
        unsigned int newLen = oldLen + len;
        char* oldBuf = mBlock->mBuff;
        Block* oldBlock = mBlock;

        mBlock = AllocBlock(NULL, newLen, (const char*)(uintptr_t)newLen, newLen);
        if (mBlock) {
            if (oldLen > 0)
                memcpy(mBlock->mBuff, oldBuf, oldLen);
            memcpy(mBlock->mBuff + oldLen, txt, len);
            mBlock->mBuff[newLen] = '\0';
            mBlock->mLength = (unsigned short)newLen;
        }
        oldBlock->DecrementCount();
    } else {
        unsigned int oldLen = mBlock->mLength;
        if (oldLen + len + 1 <= mBlock->mBlockSize) {
            memcpy(mBlock->mBuff + oldLen, txt, len);
            mBlock->mBuff[oldLen + len] = '\0';
            mBlock->mLength = (unsigned short)(oldLen + len);
        } else {
            unsigned int newLen = oldLen + len;
            Block* oldBlock = mBlock;
            mBlock = AllocBlock(NULL, newLen, (const char*)(uintptr_t)newLen, newLen);
            if (mBlock) {
                memcpy(mBlock->mBuff, oldBlock->mBuff, oldLen);
                memcpy(mBlock->mBuff + oldLen, txt, len);
                mBlock->mBuff[newLen] = '\0';
                mBlock->mLength = (unsigned short)newLen;
            }
            oldBlock->DecrementCount();
        }
    }
}

void string::clear() {
    if (mBlock) {
        if (mBlock->mRefCount > 1) {
            mBlock->DecrementCount();
            mBlock = NULL;
        } else {
            mBlock->mBuff[0] = '\0';
            mBlock->mLength = 0;
        }
    }
}

int string::find(const char* txt, unsigned int start) const {
    if (!mBlock || !txt)
        return -1;
    const char* buf = mBlock->mBuff;
    unsigned int bufLen = mBlock->mLength;
    unsigned int txtLen = Broc::length(txt);
    if (start >= bufLen || txtLen == 0)
        return -1;
    if (txtLen > bufLen - start)
        return -1;

    for (unsigned int i = start; i <= bufLen - txtLen; ++i) {
        if (memcmp(buf + i, txt, txtLen) == 0)
            return (int)i;
    }
    return -1;
}

int string::find(unsigned int start, char c) const {
    if (!mBlock)
        return -1;
    const char* buf = mBlock->mBuff;
    unsigned int bufLen = mBlock->mLength;
    if (start >= bufLen)
        return -1;

    for (unsigned int i = start; i < bufLen; ++i) {
        if (buf[i] == c)
            return (int)i;
    }
    return -1;
}

int string::rfind(const char* txt) const {
    if (!mBlock || !txt)
        return -1;
    const char* buf = mBlock->mBuff;
    unsigned int bufLen = mBlock->mLength;
    unsigned int txtLen = Broc::length(txt);
    if (txtLen == 0 || txtLen > bufLen)
        return -1;

    for (int i = (int)(bufLen - txtLen); i >= 0; --i) {
        if (memcmp(buf + i, txt, txtLen) == 0)
            return i;
    }
    return -1;
}

string& string::remove_leading(const char* chars) {
    if (!mBlock || !chars || !*chars)
        return *this;

    if (mBlock->mRefCount > 1) {
        Block* old = mBlock;
        unsigned int len = mBlock->mLength;
        mBlock = AllocBlock(mBlock->mBuff, len, mBlock->mBuff, len);
        old->DecrementCount();
    }

    char* buf = mBlock->mBuff;
    unsigned int charsLen = Broc::length(chars);
    while (mBlock->mLength > 0) {
        bool found = false;
        for (unsigned int i = 0; i < charsLen; ++i) {
            if (buf[0] == chars[i]) {
                memmove(buf, buf + 1, mBlock->mLength);
                mBlock->mLength--;
                buf[mBlock->mLength] = '\0';
                found = true;
                break;
            }
        }
        if (!found)
            break;
    }
    return *this;
}

string& string::remove_trailing(const char* chars) {
    if (!mBlock || !chars || !*chars)
        return *this;

    if (mBlock->mRefCount > 1) {
        Block* old = mBlock;
        unsigned int len = mBlock->mLength;
        mBlock = AllocBlock(mBlock->mBuff, len, mBlock->mBuff, len);
        old->DecrementCount();
    }

    char* buf = mBlock->mBuff;
    unsigned int charsLen = Broc::length(chars);
    while (mBlock->mLength > 0) {
        bool found = false;
        for (unsigned int i = 0; i < charsLen; ++i) {
            if (buf[mBlock->mLength - 1] == chars[i]) {
                mBlock->mLength--;
                buf[mBlock->mLength] = '\0';
                found = true;
                break;
            }
        }
        if (!found)
            break;
    }
    return *this;
}

void string::set_char(unsigned int idx, char c) {
    if (!mBlock || idx >= mBlock->mLength)
        return;

    if (mBlock->mRefCount > 1) {
        Block* old = mBlock;
        unsigned int len = mBlock->mLength;
        mBlock = AllocBlock(mBlock->mBuff, len, mBlock->mBuff, len);
        old->DecrementCount();
    }
    mBlock->mBuff[idx] = c;
}

string string::substr(unsigned int start, unsigned int count) const {
    string result(UNDEFINED);
    if (!mBlock || start >= mBlock->mLength)
        return result;

    unsigned int maxLen = mBlock->mLength - start;
    if (count > maxLen)
        count = maxLen;

    if (count > 0) {
        char* sub = mBlock->mBuff + start;
        result.mBlock = AllocBlock(sub, count, sub, count);
    }
    return result;
}

// ============================================================================
// Broc string comparison operators
// ============================================================================

bool operator==(const string& lhs, const char* rhs) {
    if (!rhs)
        return false;
    unsigned int lenB = Broc::length(rhs);
    unsigned int lenA = (lhs.mBlock) ? lhs.mBlock->mLength : 0;
    if (lenA != lenB)
        return false;
    const char* a = lhs.c_str();
    if (!a)
        return false;
    const char* b = rhs;
    while (*a) {
        if (*a != *b)
            return false;
        ++a; ++b;
    }
    return true;
}

bool operator==(const string& lhs, const string& rhs) {
    unsigned int lenA = (lhs.mBlock) ? lhs.mBlock->mLength : 0;
    unsigned int lenB = (rhs.mBlock) ? rhs.mBlock->mLength : 0;
    if (lenA != lenB)
        return false;
    if (lenA == 0)
        return true;
    return memcmp(lhs.mBlock->mBuff, rhs.mBlock->mBuff, lenA) == 0;
}

string operator+(const string& lhs, const string& rhs) {
    string result(lhs);
    result += rhs;
    return result;
}

string operator+(const string& lhs, const char* rhs) {
    string result(lhs);
    result += rhs;
    return result;
}

// ============================================================================
// HashStr functions
// ============================================================================

HashStr string_hash(const char* str) {
    unsigned int hash = 5381;
    if (str) {
        while (*str)
            hash = ((hash << 5) + hash) + (unsigned char)*str++;
    }
    HashStr result;
    result.mVal = hash;
    return result;
}

HashStr string_hash(const string& str) {
    return string_hash(str.c_str());
}

bool operator==(HashStr lhs, const string& rhs) {
    HashStr rhsHash = string_hash(rhs);
    return lhs.mVal == rhsHash.mVal;
}

bool operator==(const string& lhs, HashStr rhs) {
    return operator==(rhs, lhs);
}

bool operator!=(HashStr lhs, const string& rhs) {
    return !operator==(lhs, rhs);
}

bool operator!=(const string& lhs, HashStr rhs) {
    return !operator==(lhs, rhs);
}

// ============================================================================
// Stubs — ExtendedEntity
// ============================================================================

ExtendedEntity::ExtendedEntity() {
    mCount = 0;
    mCapacity = 0;
    mKVPairs = NULL;
}

ExtendedEntity::ExtendedEntity(const ExtendedEntity&) {
    mCount = 0;
    mCapacity = 0;
    mKVPairs = NULL;
}

ExtendedEntity::~ExtendedEntity() {}

ExtendedEntity* ExtendedEntity::GetExtendedEntity(unsigned int) { return NULL; }
void* ExtendedEntity::CreateExtendedEntity(const char**, int) { return NULL; }
void ExtendedEntity::DeleteExtendedEntity(void*) {}
bool ExtendedEntity::MatchExtendedEntityKey(void*, int, const char*) { return false; }
void* ExtendedEntity::CopyExtendedEntity(const void*) { return NULL; }
void ExtendedEntity::InitScript(BrocExports&) {}
bool ExtendedEntity::IsDefined(unsigned int) const { return false; }
void ExtendedEntity::SetUndefined(unsigned int) {}
unsigned int* ExtendedEntity::SetVal(unsigned int, const string&) { return NULL; }
unsigned int* ExtendedEntity::InternalSet(unsigned int, unsigned int) { return NULL; }
const unsigned int* ExtendedEntity::InternalGet(unsigned int) const { return NULL; }
ExtendedEntity::InitFunc* ExtendedEntity::GetInit(unsigned int) { return NULL; }
ExtendedEntity::CopyFunc* ExtendedEntity::GetCopier(unsigned int) { return NULL; }
ExtendedEntity::EqualsFunc* ExtendedEntity::GetEquals(unsigned int) { return NULL; }
ExtendedEntity::DestructFunc* ExtendedEntity::GetDestructor(unsigned int) { return NULL; }

// ============================================================================
// Stubs — wait / thread
// ============================================================================

Broc::entity Broc::gEntityUndef;

void ThreadExecute(void*) {}
void wait_accurate(float) {}
void wait(float) {}
void wait_frame(int) {}
void waittill(entity, HashStr) {}
void waittill_timeout(entity, HashStr, float) {}
void waittillmatch(entity, HashStr, HashStr, HashStr, HashStr) {}
void waittillor(entity, HashStr, HashStr, HashStr, HashStr) {}
void waittill(entity, const char*) {}
void waittill_loaded(unsigned int) {}
void waittill_unloaded(unsigned int) {}
void endon(entity, const char*) {}
void thread_sleep_time(void) {}
void thread_sleep_frames(void) {}
void thread_sleep_until_notify(void) {}
void thread_debug_wait_msg(int) {}
void thread_debug_wait_msg(float) {}
void thread_debug_wait_until(HashStr, HashStr, HashStr, HashStr) {}

// ============================================================================
// Stubs — entity
// ============================================================================

void entity::UndefineEEField(unsigned int) {}

// ============================================================================
// Stubs — path nodes
// ============================================================================

void GetNodeArray(const string&, const string&, void*) {}
void GetAllNodes(void*) {}
void GetVehicleNodeArray(const string&, const string&, void*) {}
void GetAllVehicleNodes(void*) {}

// ============================================================================
// Stubs — EEHelper / EEDefault templates
// ============================================================================

namespace EEHelper {
    template <typename T> unsigned int Initialize(const char*) { return 0; }
    template <typename T> bool Equals(unsigned int, const char*) { return false; }
    template <typename T> unsigned int Copy(unsigned int val) { return val; }

    template unsigned int Initialize<float>(const char*);
    template unsigned int Initialize<int>(const char*);
    template unsigned int Initialize<string>(const char*);
    template bool Equals<int>(unsigned int, const char*);
    template bool Equals<float>(unsigned int, const char*);
    template bool Equals<string>(unsigned int, const char*);
    template unsigned int Copy<string>(unsigned int);
}

namespace EEDefault {
    unsigned int Initialize(const char*) { return 0; }
    unsigned int Copy(unsigned int val) { return val; }
    void Destruct(unsigned int&) {}
    bool Equals(unsigned int, const char*) { return false; }
}

} // namespace Broc

// ============================================================================
// COD3 Broc Engine Core — Broc.o port
// Reconstructed from codmp_xboxr.xbe (release build, /O2)
// Source: c:\cod\code\script\include\strings.inl (assert strings confirm)
// ============================================================================

#include "engine/broc_types.h"
#include "game/AeThreadFunctor.h"
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <new>

// ============================================================================
// External dependencies
// ============================================================================
// IDA's Broc string routines call the global BrocSys allocator.  Keep these
// declarations bound to that existing implementation so pool-owned blocks
// are released through the matching ownership check.
extern void* mem_alloc(unsigned int size, unsigned int align);
extern void mem_free(void* ptr);
extern const char defaultFileName[];

extern int gNumStringsAlloc;
extern int gNumStringsFreed;
extern const float VectorDistance(const float* const v1, const float* const v2);
extern const float VectorDistanceSquared(const float* const p1,
                                         const float* const p2);
extern void FastSinCos(float radians, float* psin, float* pcos);
extern void vectoangles(const float* const vec, float* const angles);
extern void AngleVectors(const float* const angles, float* const forward,
                         float* const right, float* const up);
extern void AnglesToUp(const float* const angles, float* const up);
extern void AnglesToRight(const float* const angles, float* const right);
extern void AnglesToForward(const float* const angles, float* const forward);
extern int irand(int min, int max);
extern float flrand(float min, float max);

extern "C" int __fpclass(float);

namespace AeAssert {
enum ECoderId : int;
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
}

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

// IDA globals @ 0x10F0540 / 0x10F0544.
float gThreadSleepTime = 0.0f;
int gThreadSleepFrames = 0;
unsigned int gThreadSleepEntity = 0;
int gThreadSleepNotify2 = 0;
int gThreadSleepNotify1 = 0;
int gThreadSleepNotify3 = 0;
bool gThreadWaitForAll = false;
float gTimeOut = 0.0f;
int gThreadSleepNotify4 = 0;
TPakInfo gThreadSleepPakfile = (TPakInfo)0;

// ea: 0x00929060. IDA forwards hash registration through BrocAPI.
void RegisterHashString(int hash, const char* text)
{
    Broc::gBrocAPI.mRegisterHashString(hash, text);
}

// ea: 0x00929570. IDA forwards hash-to-text conversion through BrocAPI.
const char* ToStr(int hash)
{
    return Broc::gBrocAPI.mHashToStr(hash);
}

// ============================================================================
// Broc utility functions
// ============================================================================
namespace Broc {

// IDA global (codmp_xboxr.xbe.c:956493; ea: 0x010F0568).
BrocAPI gBrocAPI = {};
ExtendedEntity ExtendedEntity::nullEnt;
ExtendedEntity::GetFunctionsFunc ExtendedEntity::sGetFunctions = nullptr;

int MathsRandomInt(int iMax)
{
    if (iMax <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../script/include\\maths.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "iMax > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("RandomInt range must be positive integer.\n"))
            __debugbreak();
    }
    return irand(0, iMax);
}

float MathsRandomFloat(float fMax)
{
    return flrand(0.0f, fMax);
}

int MathsRandomIntRange(int iMin, int iMax)
{
    return iMax > iMin ? irand(iMin, iMax) : irand(iMax, iMin);
}

float MathsRandomFloatRange(float fMin, float fMax)
{
    return fMin < fMax ? flrand(fMin, fMax) : flrand(fMax, fMin);
}

// ea: 0x0094B880. IDA forwards to the runtime's typed random callback.
float RandomFloat(float fMax)
{
    return gBrocAPI.mMathsRandomFloat(fMax);
}

// ea: 0x0094C9F0. IDA forwards the player stat index to the runtime.
__int16 Code_GetPlayerStat(Broc::entity player, int index)
{
    return gBrocAPI.mGetPlayerStat(player.GetHandle(), (unsigned int)index);
}

// Runtime export used by _mp_common::GetWinningTeam.
int Code_GetPlayerTotalScore(Broc::entity player)
{
    return gBrocAPI.mGetPlayerTotalScore(player.GetHandle());
}

// ea: 0x00928B40. IDA forwards to the runtime's typed random-range callback.
float RandomFloatRange(float fMin, float fMax)
{
    return gBrocAPI.mMathsRandomFloatRange(fMin, fMax);
}

float MathsLog(float fVal)
{
    if (fVal <= 0.0f)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "../script/include\\maths.h";
        AeAssert::gCurrentLine = 101;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                   "value passed into natural log function out of range ( value <= 0.0f )"))
            __debugbreak();
    }
    return logf(fVal);
}

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

vector& vector::operator+=(const vector& rhs)
{
    x = rhs.x + x;
    y = rhs.y + y;
    z = rhs.z + z;
    return *this;
}

bool vector::operator==(const vector& rhs)
{
    return x == rhs.x && y == rhs.y && z == rhs.z;
}

vector operator+(const vector& lhs, const vector& rhs)
{
    vector result;
    result.x = lhs.x + rhs.x;
    result.y = lhs.y + rhs.y;
    result.z = lhs.z + rhs.z;
    return result;
}

vector operator-(const vector& lhs, const vector& rhs)
{
    vector result;
    result.x = lhs.x - rhs.x;
    result.y = lhs.y - rhs.y;
    result.z = lhs.z - rhs.z;
    return result;
}

vector operator/(const vector& lhs, float rhs)
{
    const float inverse = 1.0f / rhs;
    vector result;
    result.x = lhs.x * inverse;
    result.y = lhs.y * inverse;
    result.z = lhs.z * inverse;
    return result;
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

// ea: 0x005EE110
void normalize(vector* v) {
    float len = sqrtf(v->x * v->x + v->y * v->y + v->z * v->z);
    if (len != 0.0f)
    {
        v->x = (1.0f / len) * v->x;
        float z = (1.0f / len) * v->z;
        v->y = (1.0f / len) * v->y;
        v->z = z;
    }
}

// ea: 0x005EE1A0
void VecNormalize(vector* vecOut, const vector* vecIn) {
    *vecOut = *vecIn;
    normalize(vecOut);
}

// ea: 0x005E9780
void VecToAngles(vector* vecOut, const vector* vecIn) {
    vectoangles(&vecIn->x, &vecOut->x);
}

// ea: 0x005E97A0
void VecAnglesToVectors(const vector* angles, vector* forward,
                        vector* right, vector* up) {
    AngleVectors(&angles->x, &forward->x, &right->x, &up->x);
}

// ea: 0x005E97B0
hudelem::hudelem()
    : ___u0(0xFFFFFFFFu)
{
}

// ea: 0x005E97C0
hudelem::hudelem(unsigned int v)
    : ___u0(v)
{
}

// ea: 0x005E97E0
unsigned int hudelem::GetIndex() const
{
    return ___u0;
}

// ea: 0x00986BC0
bool hudelem::IsDefined() const
{
    return ___u0 != 0xFFFFFFFFu;
}

// ea: 0x00948420
void hudelem::SetUndefined()
{
    ___u0 = 0xFFFFFFFFu;
}

// ea: 0x005E9800
TPathnodeHandle pathnode::GetHandle() const
{
    return (TPathnodeHandle)___u0;
}

// ea: 0x005E9810
bool pathnode::IsDefined() const
{
    return ___u0 != (unsigned int)INVALID_PATHNODE_HANDLE
        && ___u0 != 0xFFFFFFFFu;
}

// ea: 0x005E9830
TVehiclenodeHandle vehiclenode::GetHandle() const
{
    return (TVehiclenodeHandle)___u0;
}

// ea: 0x005E9840
bool vehiclenode::IsDefined() const
{
    return ___u0 != (unsigned int)INVALID_VEHICLENODE_HANDLE;
}

// ea: 0x005EF530
void VecAnglesToUp(vector* vecOut, const vector* angles) {
    ::AnglesToUp(&angles->x, &vecOut->x);
}

// ea: 0x005EF550
void VecAnglesToRight(vector* vecOut, const vector* angles) {
    ::AnglesToRight(&angles->x, &vecOut->x);
}

// ea: 0x005EF570
void VecAnglesToForward(vector* vecOut, const vector* angles) {
    ::AnglesToForward(&angles->x, &vecOut->x);
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
    return (hint + 32) & ~31;
}

// ea: 0x004926D20
static void string_AllocBlock_internal(ae_pair<void*, int>* result, int size) {
    int finalSize = string::GetBlockSize(size + 12);
    ++gNumStringsAlloc;
    void* mem = mem_alloc(finalSize, 4);
    result->first = mem;
    result->second = finalSize - 12;
}

// ea: 0x004926DC0
string::Block* string::AllocBlock(const char* txt, unsigned int txtLen,
                                   unsigned int sizeHint) {
    if (!txt)
        return NULL;

    int s = sizeHint ? (int)(sizeHint + 1) : (int)(txtLen + 1);

    ae_pair<void*, int> r;
    string_AllocBlock_internal(&r, s);

    string::Block* blk = (string::Block*)r.first;
    if (blk) {
        new (blk) Block((unsigned short)r.second, (unsigned short)txtLen, txt);
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
    mBlock = AllocBlock(txt, len, 0);
}

string::string(const char* txt, int) {
    unsigned int len = Broc::length(txt);
    mBlock = AllocBlock(txt, len, 0);
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
    mBlock = AllocBlock(buf, len, 0);
}

string::string(int val) {
    char buf[16];
    sprintf(buf, "%d", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
}

string::string(float val) {
    char buf[32];
    sprintf(buf, "%g", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
}

string::string(const bint& val) {
    char buf[16];
    sprintf(buf, "%d", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
}

string::string(const bfloat& val) {
    char buf[32];
    sprintf(buf, "%g", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
}

string::string(const bunsigned& val) {
    char buf[16];
    sprintf(buf, "%u", val.mVal);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
}

string::string(const bbool& val) {
    const char* s = val.mVal ? "true" : "false";
    unsigned int len = Broc::length(s);
    mBlock = AllocBlock(s, len, 0);
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
        return defaultFileName;
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
        mBlock = AllocBlock(txt, len, 0);
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
    mBlock = AllocBlock(buf, 1, 0);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(int val) {
    Block* oldBlock = mBlock;
    char buf[16];
    sprintf(buf, "%d", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(float val) {
    Block* oldBlock = mBlock;
    char buf[32];
    sprintf(buf, "%g", val);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
    if (oldBlock)
        oldBlock->DecrementCount();
    return *this;
}

string& string::operator=(const vector& v) {
    Block* oldBlock = mBlock;
    char buf[64];
    sprintf(buf, "(%g %g %g)", v.x, v.y, v.z);
    unsigned int len = Broc::length(buf);
    mBlock = AllocBlock(buf, len, 0);
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
    if (!txt || len == 0)
        return;

    if (!mBlock) {
        mBlock = AllocBlock(txt, len, 0);
    } else if (mBlock->mRefCount > 1) {
        // Copy-on-write
        unsigned int oldLen = mBlock->mLength;
        unsigned int newLen = oldLen + len;
        char* oldBuf = mBlock->mBuff;
        Block* oldBlock = mBlock;

        mBlock = AllocBlock(oldBuf, oldLen, newLen);
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
        if (oldLen + len + 1 < mBlock->mBlockSize) {
            memcpy(mBlock->mBuff + oldLen, txt, len);
            mBlock->mBuff[oldLen + len] = '\0';
            mBlock->mLength = (unsigned short)(oldLen + len);
        } else {
            unsigned int newLen = oldLen + len;
            Block* oldBlock = mBlock;
            mBlock = AllocBlock(oldBlock->mBuff, oldLen, newLen);
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
        mBlock = AllocBlock(mBlock->mBuff, len, 0);
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
        mBlock = AllocBlock(mBlock->mBuff, len, 0);
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

string& string::remove_surrounding_whitespace() {
    remove_leading(" \n\t\r");
    remove_trailing(" \n\t\r");
    return *this;
}

void string::set_char(unsigned int idx, char c) {
    if (!mBlock || idx >= mBlock->mLength)
        return;

    if (mBlock->mRefCount > 1) {
        Block* old = mBlock;
        unsigned int len = mBlock->mLength;
        mBlock = AllocBlock(mBlock->mBuff, len, 0);
        old->DecrementCount();
    }
    mBlock->mBuff[idx] = c;
}

string string::substr(unsigned int start, unsigned int count) const {
    string result(UNDEFINED);
    if (!mBlock || start >= mBlock->mLength || count == 0)
        return result;

    if (count + start > mBlock->mLength)
        count = mBlock->mLength - start;

    const unsigned int end = start + count;
    char* buffer = Block::GetBuff(mBlock);
    const char saved = buffer[end];
    buffer[end] = '\0';
    result.mBlock = AllocBlock(&buffer[start], count, 0);
    buffer[end] = saved;
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

// operator==(char*, string) - ea: 0x967D90
bool operator==(const char* lhs, const string& rhs) {
    return operator==(rhs, lhs);
}

bool operator!=(const string& lhs, const char* rhs) {
    return !operator==(lhs, rhs);
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

// ea: 0x00928710. IDA hashes through the runtime callback into the result.
HashStr* string_hash(HashStr* result, const char* str)
{
    result->mVal = gBrocAPI.mStringHash(str);
    return result;
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

// ea: 0x0092A380. IDA profiling events are omitted; allocation, typed copies,
// and unused-slot initialization follow the decompiled body.
ExtendedEntity::ExtendedEntity(const ExtendedEntity& rhs)
{
    mCount = rhs.mCount;
    mCapacity = rhs.mCapacity;
    mKVPairs = static_cast<KVPair*>(mem_alloc(8 * mCapacity, 4));
    for (unsigned int i = 0; i < mCount; ++i)
    {
        mKVPairs[i].key = rhs.mKVPairs[i].key;
        CopyFunc* copier = GetCopier(mKVPairs[i].key);
        mKVPairs[i].val = copier(rhs.mKVPairs[i].val);
    }
    for (unsigned int j = mCount; j < mCapacity; ++j)
    {
        mKVPairs[j].key = 0;
        mKVPairs[j].val = 0;
    }
}

// ea: 0x0092A5D0. Profiling events are omitted; typed destruction and storage
// release follow the IDA body.
ExtendedEntity::~ExtendedEntity()
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        GetDestructor(mKVPairs[i].key)(mKVPairs[i].val);
        mKVPairs[i].key = 0;
        mKVPairs[i].val = 0;
    }
    mem_free(mKVPairs);
    mKVPairs = nullptr;
}

// ea: 0x00929980. IDA forwards directly to BrocAPI::mGetExtendedEntity.
ExtendedEntity* ExtendedEntity::GetExtendedEntity(unsigned int handle)
{
    return static_cast<ExtendedEntity*>(gBrocAPI.mGetExtendedEntity(handle));
}
// ea: 0x00929AC0. IDA profiling events are omitted; allocation, key hashing,
// callback lookup, validation, and initialization follow the decompiled body.
void* ExtendedEntity::CreateExtendedEntity(const char** kvPairs, int nPairs)
{
    ExtendedEntity* ee = new (std::nothrow) ExtendedEntity();

    for (int i = 0; i < nPairs; ++i)
    {
        const char* keystr = kvPairs[2 * i];
        const char* valstr = kvPairs[2 * i + 1];
        const unsigned int key = string_hash(keystr).mVal;
        InitFunc* init = nullptr;
        DestructFunc* dtor = nullptr;

        if (sGetFunctions == nullptr
            && gBrocAPI.mAssert(
                   "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                   157,
                   "sGetFunctions not initialized!"))
            __debugbreak();

        sGetFunctions(key, &init, nullptr, nullptr, &dtor);
        if (init == nullptr && dtor != nullptr
            && gBrocAPI.mAssert(
                   "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                   160,
                   "Key value can not be set in Graydiant.  Press Debug and look at \"keystr\""))
            __debugbreak();

        if (init != nullptr)
            ee->InternalSet(key, init(valstr));
    }

    return ee;
}
// ea: 0x00929DB0; IDA invokes the scalar deleting destructor with delete flag.
void ExtendedEntity::DeleteExtendedEntity(void* mem)
{
    delete static_cast<ExtendedEntity*>(mem);
}
// ea: 0x00929F60. Profiling events are omitted; lookup and typed comparison
// follow the IDA body exactly.
bool ExtendedEntity::MatchExtendedEntityKey(void* mem, int key, const char* text)
{
    ExtendedEntity* ptr = static_cast<ExtendedEntity*>(mem);
    const unsigned int* value = ptr->InternalGet(static_cast<unsigned int>(key));
    if (value == nullptr)
        return false;
    return ptr->GetEquals(static_cast<unsigned int>(key))(*value, text);
}
// ea: 0x0092A140. IDA allocates one ExtendedEntity and invokes the copy
// constructor; profiling bookkeeping is omitted with the other Broc wrappers.
void* ExtendedEntity::CopyExtendedEntity(const void* source)
{
    return new (std::nothrow) ExtendedEntity(
        *static_cast<const ExtendedEntity*>(source));
}
// ea: 0x0092A2F0. IDA installs the four ExtendedEntity callbacks directly.
void ExtendedEntity::InitScript(BrocExports& exports)
{
    exports.mCreateExtendedEntity = ExtendedEntity::CreateExtendedEntity;
    exports.mDeleteExtendedEntity = ExtendedEntity::DeleteExtendedEntity;
    exports.mMatchExtendedEntityKey = ExtendedEntity::MatchExtendedEntityKey;
    exports.mCopyExtendedEntity = ExtendedEntity::CopyExtendedEntity;
}
// ea: 0x0092A7A0. IDA's profiling enter/leave events use BrocAPI members not
// present in the current partial declaration; the key scan is unchanged.
bool ExtendedEntity::IsDefined(unsigned int key) const
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        if (mKVPairs[i].key == key)
            return true;
    }
    return false;
}
// ea: 0x0092A990. Profiling enter/leave events are omitted for the same
// partial-BrocAPI reason documented above IsDefined.
void ExtendedEntity::SetUndefined(unsigned int key)
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        if (mKVPairs[i].key != key)
            continue;

        GetDestructor(mKVPairs[i].key)(mKVPairs[i].val);
        if (i < --mCount)
        {
            mKVPairs[i].key = mKVPairs[mCount].key;
            mKVPairs[i].val = mKVPairs[mCount].val;
        }
        mKVPairs[mCount].key = 0;
        mKVPairs[mCount].val = 0;
        return;
    }
}
// ea: 0x0092AC40. IDA placement-copies the string into the raw slot and
// delegates ownership/replacement to InternalSet.
unsigned int* ExtendedEntity::SetVal(unsigned int key, const string& val)
{
    unsigned int raw = 0;
    new (&raw) string(val, 0);
    return InternalSet(key, raw);
}
// ea: 0x0092ACF0. IDA profiling events are omitted; null-entity handling,
// typed replacement, growth, and slot initialization follow the decompiled
// implementation.
unsigned int* ExtendedEntity::InternalSet(unsigned int key, unsigned int val)
{
    if (this == &ExtendedEntity::nullEnt)
    {
        if (gBrocAPI.mThreadGetId() == gBrocAPI.mBrocExports.mMainThreadHandle
            && gBrocAPI.mError(
                   "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                   344,
                   "deref of null entity in the main level thread is not allowed"))
            __debugbreak();
        if (gBrocAPI.mKillThread)
            gBrocAPI.mKillThreadExec();
    }

    for (unsigned int i = 0; i < mCount; ++i)
    {
        if (mKVPairs[i].key != key)
            continue;
        GetDestructor(mKVPairs[i].key)(mKVPairs[i].val);
        mKVPairs[i].val = val;
        return &mKVPairs[i].val;
    }

    if (mCount == mCapacity)
    {
        mCapacity += 8;
        KVPair* oldKVPairs = mKVPairs;
        mKVPairs = static_cast<KVPair*>(mem_alloc(8 * mCapacity, 4));
        memcpy(mKVPairs, oldKVPairs, 8 * mCount);
        for (unsigned int j = mCount; j < mCapacity; ++j)
        {
            mKVPairs[j].key = 0;
            mKVPairs[j].val = 0;
        }
        mem_free(oldKVPairs);
    }

    mKVPairs[mCount].key = key;
    mKVPairs[mCount].val = val;
    return &mKVPairs[mCount++].val;
}
// ea: 0x0092B0E0. IDA profiling events are omitted as documented above.
const unsigned int* ExtendedEntity::InternalGet(unsigned int key) const
{
    for (unsigned int i = 0; i < mCount; ++i)
    {
        if (mKVPairs[i].key == key)
            return &mKVPairs[i].val;
    }
    return nullptr;
}
// The IDA bodies also emit profiling events through fields not represented in
// the current partial BrocAPI declaration; callback selection and exact
// EEDefault fallbacks are preserved here.
ExtendedEntity::InitFunc* ExtendedEntity::GetInit(unsigned int key)
{
    InitFunc* init = nullptr;
    if (sGetFunctions != nullptr)
    {
        sGetFunctions(key, &init, nullptr, nullptr, nullptr);
        if (init != nullptr)
            return init;
    }
    return EEDefault::Initialize;
}

ExtendedEntity::CopyFunc* ExtendedEntity::GetCopier(unsigned int key)
{
    CopyFunc* copy = nullptr;
    if (sGetFunctions != nullptr)
    {
        sGetFunctions(key, nullptr, &copy, nullptr, nullptr);
        if (copy != nullptr)
            return copy;
    }
    return EEDefault::Copy;
}

ExtendedEntity::EqualsFunc* ExtendedEntity::GetEquals(unsigned int key)
{
    EqualsFunc* equals = nullptr;
    if (sGetFunctions != nullptr)
    {
        sGetFunctions(key, nullptr, nullptr, &equals, nullptr);
        if (equals != nullptr)
            return equals;
    }
    return EEDefault::Equals;
}

ExtendedEntity::DestructFunc* ExtendedEntity::GetDestructor(unsigned int key)
{
    DestructFunc* dtor = nullptr;
    if (sGetFunctions != nullptr)
    {
        sGetFunctions(key, nullptr, nullptr, nullptr, &dtor);
        if (dtor != nullptr)
            return dtor;
    }
    return EEDefault::Destruct;
}

// ============================================================================
// Stubs — wait / thread
// ============================================================================

Broc::entity Broc::gEntityUndef;

// ea: 0x009289A0. IDA invokes the thread functor's virtual CallFunction.
void ThreadExecute(AeThreadFunctor* thread)
{
    thread->CallFunction();
}
// ea: 0x009289D0. IDA validates the time, records it, and enters the sleep
// hook before honoring the kill-thread flag.
void wait_accurate(float t)
{
    if (IS_NAN(t)
        && gBrocAPI.mError(
               "c:\\cod\\code\\script\\include\\threads.inl",
               68,
               "Cannot wait for undefined time"))
        __debugbreak();
    thread_debug_wait_msg(t);
    gThreadSleepTime = t;
    thread_sleep_time();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00928A70. IDA validates the time, randomizes the interval, and sleeps.
void wait(float t)
{
    if (IS_NAN(t)
        && gBrocAPI.mError(
               "c:\\cod\\code\\script\\include\\threads.inl",
               83,
               "Cannot wait for undefined time"))
        __debugbreak();
    const float ta = RandomFloatRange(t, 1.05f * t);
    thread_debug_wait_msg(ta);
    gThreadSleepTime = ta;
    thread_sleep_time();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00928B70. IDA stores the frame count and enters the frame-sleep hook.
void wait_frame(int numFrames)
{
    thread_debug_wait_msg(numFrames);
    gThreadSleepFrames = numFrames;
    thread_sleep_frames();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00928BC0. IDA sets the notify state and sleeps on a non-null entity.
void waittill(entity ent, HashStr labelHash)
{
    HashStr notify2 = {0};
    HashStr notify3 = {0};
    HashStr notify4 = {0};
    thread_debug_wait_until(labelHash, notify2, notify3, notify4);
    if (ent.GetHandle() == 0
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\include\\threads.inl",
               110,
               "null entity passed into waittill- thread will be killed"))
        __debugbreak();
    if (ent.GetHandle() != 0)
    {
        gThreadSleepEntity = ent.GetHandle();
        gThreadSleepPakfile = (TPakInfo)0;
        gThreadSleepNotify1 = labelHash.mVal;
        gThreadSleepNotify2 = 0;
        gThreadSleepNotify3 = 0;
        gThreadSleepNotify4 = 0;
        gThreadWaitForAll = false;
        gTimeOut = 0.0f;
        thread_sleep_until_notify();
        if (gBrocAPI.mKillThread)
            gBrocAPI.mKillThreadExec();
    }
}
// ea: 0x00928CF0. IDA uses the same notify state with a timeout value.
void waittill_timeout(entity ent, HashStr labelHash, float t)
{
    HashStr notify2 = {0};
    HashStr notify3 = {0};
    HashStr notify4 = {0};
    thread_debug_wait_until(labelHash, notify2, notify3, notify4);
    if (ent.GetHandle() == 0
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\include\\threads.inl",
               132,
               "null entity passed into waittill- thread will be killed"))
        __debugbreak();
    if (ent.GetHandle() != 0)
    {
        gThreadSleepEntity = ent.GetHandle();
        gThreadSleepPakfile = (TPakInfo)0;
        gThreadSleepNotify1 = labelHash.mVal;
        gThreadSleepNotify2 = 0;
        gThreadSleepNotify3 = 0;
        gThreadSleepNotify4 = 0;
        gThreadWaitForAll = false;
        gTimeOut = t;
        thread_sleep_until_notify();
        if (gBrocAPI.mKillThread)
            gBrocAPI.mKillThreadExec();
    }
}
// ea: 0x00928E20. IDA waits for all four labels.
void waittillmatch(entity ent, HashStr label1, HashStr label2,
                   HashStr label3, HashStr label4)
{
    thread_debug_wait_until(label1, label2, label3, label4);
    if (ent.GetHandle() == 0
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\include\\threads.inl",
               153,
               "null entity passed into waittillmatch- thread will be killed"))
        __debugbreak();
    gThreadSleepEntity = ent.GetHandle();
    gThreadSleepPakfile = (TPakInfo)0;
    gThreadSleepNotify1 = label1.mVal;
    gThreadSleepNotify2 = label2.mVal;
    gThreadSleepNotify3 = label3.mVal;
    gThreadSleepNotify4 = label4.mVal;
    gThreadWaitForAll = true;
    gTimeOut = 0.0f;
    thread_sleep_until_notify();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00928F10. IDA waits for any of the four labels.
void waittillor(entity ent, HashStr label1, HashStr label2,
                HashStr label3, HashStr label4)
{
    thread_debug_wait_until(label1, label2, label3, label4);
    if (ent.GetHandle() == 0
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\include\\threads.inl",
               172,
               "null entity passed into waittillmatch- thread will be killed"))
        __debugbreak();
    gThreadSleepEntity = ent.GetHandle();
    gThreadSleepPakfile = (TPakInfo)0;
    gThreadSleepNotify1 = label1.mVal;
    gThreadSleepNotify2 = label2.mVal;
    gThreadSleepNotify3 = label3.mVal;
    gThreadSleepNotify4 = label4.mVal;
    gThreadWaitForAll = false;
    gTimeOut = 0.0f;
    thread_sleep_until_notify();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00929000. IDA hashes/registers the text and forwards to the hash form.
void waittill(entity ent, const char* label)
{
    HashStr h;
    string_hash(&h, label);
    RegisterHashString(h.mVal, label);
    waittill(ent, h);
}
// ea: 0x00929090. IDA records the pak handle and waits for a load notify.
void waittill_loaded(TPakInfo info)
{
    gThreadSleepPakfile = info;
    gThreadSleepNotify1 = 1;
    thread_sleep_until_notify();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x009290E0. The unload path differs only in its notify selector.
void waittill_unloaded(TPakInfo info)
{
    gThreadSleepPakfile = info;
    gThreadSleepNotify1 = 0;
    thread_sleep_until_notify();
    if (gBrocAPI.mKillThread)
        gBrocAPI.mKillThreadExec();
}
// ea: 0x00929130. IDA hashes/registers the label before terminating on it.
void endon(entity ent, const char* label)
{
    HashStr h;
    string_hash(&h, label);
    RegisterHashString(h.mVal, label);
    gBrocAPI.mThreadTerminateOnNotify(ent.GetHandle(), h.mVal);
}
void thread_sleep_time(void) {}
void thread_sleep_frames(void) {}
void thread_sleep_until_notify(void) {}
void thread_debug_wait_msg(int) {}
// ea: 0x00929320. IDA formats the wait duration and emits a thread notice.
void thread_debug_wait_msg(float time)
{
    char tmpBuf[64];
    sprintf(tmpBuf, "going to sleep for %3.2f seconds", time);
    gBrocAPI.mThreadDebugNotice(tmpBuf);
}
// ea: 0x00929370. IDA formats the non-zero notify labels in order.
void thread_debug_wait_until(HashStr notify1, HashStr notify2,
                             HashStr notify3, HashStr notify4)
{
    char tmpBuf[1024];
    if (notify2.mVal == 0)
        sprintf(tmpBuf, "sleeping until %s", ToStr(notify1.mVal));
    else if (notify3.mVal == 0)
        sprintf(tmpBuf, "sleeping until %s,%s",
                ToStr(notify1.mVal), ToStr(notify2.mVal));
    else if (notify4.mVal == 0)
        sprintf(tmpBuf, "sleeping until %s,%s,%s",
                ToStr(notify1.mVal), ToStr(notify2.mVal),
                ToStr(notify3.mVal));
    else
        sprintf(tmpBuf, "sleeping until %s,%s,%s,%s",
                ToStr(notify1.mVal), ToStr(notify2.mVal),
                ToStr(notify3.mVal), ToStr(notify4.mVal));
    gBrocAPI.mThreadDebugNotice(tmpBuf);
}

// ============================================================================
// Stubs — entity
// ============================================================================

// ea: 0x0092BA60. IDA forwards the field removal through the entity handle.
void entity::UndefineEEField(unsigned int member)
{
    ExtendedEntity* ee = ExtendedEntity::GetExtendedEntity(GetHandle());
    if (ee != nullptr)
        ee->SetUndefined(member);
}

// ============================================================================
// Stubs — path nodes
// ============================================================================

// ea: 0x0092BAB0. IDA fills pathnode handles returned by the runtime.
void GetNodeArray(const string& val, const string& key,
                  dyn_array<pathnode>* nodearr)
{
    nodearr->clear();
    int intarr[1024];
    const int size = gBrocAPI.mGetNode(&val, &key, intarr, 1024);
    if ((size < 0 || size == 1024)
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\gen\\broc.cpp", 83,
               "getnodearray- there may be more node than the current capacity"))
        __debugbreak();
    nodearr->reserve((unsigned int)(size + nodearr->mSize));
    for (int i = 0; i < size; ++i)
        nodearr->push_back(pathnode(intarr[i]));
}

// ea: 0x0092BC10. IDA queries all path nodes with blank filters.
void GetAllNodes(dyn_array<pathnode>* nodearr)
{
    nodearr->clear();
    string blank((string::Block*)nullptr);
    int intarr[1024];
    const int size = gBrocAPI.mGetNode(&blank, &blank, intarr, 1024);
    if (size == 1024
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\gen\\broc.cpp", 96,
               "getnodearray- there may be more node than the current capacity"))
        __debugbreak();
    if (size > 0)
    {
        nodearr->reserve((unsigned int)(size + nodearr->mSize));
        for (int i = 0; i < size; ++i)
            nodearr->push_back(pathnode(intarr[i]));
    }
}

// ea: 0x0092BD90. IDA fills vehicle-node handles returned by the runtime.
void GetVehicleNodeArray(const string& val, const string& key,
                         dyn_array<vehiclenode>* nodearr)
{
    nodearr->clear();
    int intarr[1024];
    const int size = gBrocAPI.mGetVehicleNode(&val, &key, intarr, 1024);
    if (size == 1024
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\gen\\broc.cpp", 112,
               "getnodearray- there may be more node than the current capacity"))
        __debugbreak();
    if (size > 0)
    {
        nodearr->reserve((unsigned int)(size + nodearr->mSize));
        for (int i = 0; i < size; ++i)
            nodearr->push_back(vehiclenode(intarr[i]));
    }
}

// ea: 0x0092BEE0. IDA queries all vehicle nodes into pathnode handles.
void GetAllVehicleNodes(dyn_array<pathnode>* nodearr)
{
    nodearr->clear();
    string blank1((string::Block*)nullptr);
    string blank2((string::Block*)nullptr);
    int intarr[1024];
    const int size = gBrocAPI.mGetVehicleNode(&blank2, &blank1, intarr, 1024);
    if (size == 1024
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\gen\\broc.cpp", 129,
               "getnodearray- there may be more node than the current capacity"))
        __debugbreak();
    nodearr->reserve((unsigned int)(size + nodearr->mSize));
    for (int i = 0; i < size; ++i)
        nodearr->push_back(pathnode(intarr[i]));
}

// ea: 0x0092C0B0. IDA queries all vehicle nodes into vehiclenode handles.
void GetAllVehicleNodes(dyn_array<vehiclenode>* nodearr)
{
    nodearr->clear();
    string blank1((string::Block*)nullptr);
    string blank2((string::Block*)nullptr);
    int intarr[1024];
    const int size = gBrocAPI.mGetVehicleNode(&blank2, &blank1, intarr, 1024);
    if (size == 1024
        && gBrocAPI.mAssert(
               "c:\\cod\\code\\script\\gen\\broc.cpp", 142,
               "GetVehicleNodeArray- there may be more node than the current capacity"))
        __debugbreak();
    if (size > 0)
    {
        nodearr->reserve((unsigned int)(size + nodearr->mSize));
        for (int i = 0; i < size; ++i)
            nodearr->push_back(vehiclenode(intarr[i]));
    }
}

// ============================================================================
// EEHelper / EEDefault templates
// ============================================================================

namespace EEHelper {
    template <typename T> unsigned int Initialize(const char*) { return 0; }
    template <typename T>
    bool Equals(typename EqualsArg<T>::type, const char*) { return false; }
    template <typename T> unsigned int Copy(unsigned int val) { return val; }

    // ea: 0x009295A0; IDA calls BrocAPI::mAtoF and returns the raw float bits.
    template <> unsigned int Initialize<float>(const char* text)
    {
        const float value = gBrocAPI.mAtoF(text);
        unsigned int bits = 0;
        memcpy(&bits, &value, sizeof(bits));
        return bits;
    }

    // ea: 0x009295E0; IDA calls BrocAPI::mAtoI.
    template <> unsigned int Initialize<int>(const char* text)
    {
        return static_cast<unsigned int>(gBrocAPI.mAtoI(text));
    }

    // ea: 0x00929620; the string object is constructed in the returned raw
    // four-byte ExtendedEntity slot, matching IDA's placement-new body.
    template <> unsigned int Initialize<string>(const char* text)
    {
        unsigned int raw = 0;
        new (&raw) string(text, 0);
        return raw;
    }

    // ea: 0x009296C0.
    template <> bool Equals<int>(unsigned int lhs, const char* text)
    {
        return static_cast<unsigned int>(atoi(text)) == lhs;
    }

    // ea: 0x00929700.
    template <> bool Equals<float>(float lhs, const char* text)
    {
        return lhs == static_cast<float>(atof(text));
    }

    // ea: 0x00929760.
    template <> bool Equals<string>(unsigned int lhs, const char* text)
    {
        return operator==(*reinterpret_cast<const string*>(&lhs), text);
    }

    // ea: 0x00929790; copy the raw string object into the returned slot.
    template <> unsigned int Copy<string>(unsigned int data)
    {
        unsigned int raw = 0;
        new (&raw) string(*reinterpret_cast<const string*>(&data), 0);
        return raw;
    }
}

namespace EEDefault {
    // ea: 0x00929840; IDA source line global is 103 (+1 in the call).
    unsigned int Initialize(const char*)
    {
        if (gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                104, "No initializer found!"))
            __debugbreak();
        return 0;
    }

    // ea: 0x00929890; IDA source line global is 109 (+1 in the call).
    unsigned int Copy(unsigned int)
    {
        if (gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                110, "No copier found!"))
            __debugbreak();
        return 0;
    }

    // ea: 0x009298E0; IDA source line global is 115 (+1 in the call).
    void Destruct(unsigned int&)
    {
        if (gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                116, "No destructor found!"))
            __debugbreak();
    }

    // ea: 0x00929930; IDA source line global is 120 (+1 in the call).
    bool Equals(unsigned int, const char*)
    {
        if (gBrocAPI.mWarning(
                "c:\\cod\\code\\script\\include\\extendedentity.cpp",
                121, "No equality function found!"))
            __debugbreak();
        return false;
    }
}

} // namespace Broc

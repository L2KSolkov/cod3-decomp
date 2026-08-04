// ============================================================================
// COD3 Engine Types — Broc/BrocSys string, entity, hashing, and extended entity
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA (32-bit; 64-bit builds use conditional guards).
// ============================================================================

#pragma once

#include <stdint.h>

// Size assertions are 32-bit only (4-byte pointers)
#if defined(_WIN32) && !defined(_WIN64)
#define COD3_STATIC_ASSERT_32BIT(expr, msg) static_assert(expr, msg)
#elif UINTPTR_MAX == 0xFFFFFFFF
#define COD3_STATIC_ASSERT_32BIT(expr, msg) static_assert(expr, msg)
#else
#define COD3_STATIC_ASSERT_32BIT(expr, msg)
#endif

// ============================================================================
// HashStr — hashed string identifier (4 bytes)
// ============================================================================
struct HashStr {
    unsigned int mVal;  // +0x00
};
COD3_STATIC_ASSERT_32BIT(sizeof(HashStr) == 4, "HashStr size mismatch");

// ============================================================================
// HashString — global hashed string (4 bytes) — verified against IDA
// ============================================================================
struct HashString {
    unsigned int mHash;  // +0x00
};
COD3_STATIC_ASSERT_32BIT(sizeof(HashString) == 4, "HashString size mismatch");

// ============================================================================
// Broc namespace
// ============================================================================
namespace Broc {

// ============================================================================
// Broc::EUndefined — tag type for undefined string construction
// ============================================================================
enum EUndefined { UNDEFINED };

// ============================================================================
// Broc::vector — 3D vector (12 bytes)
// ============================================================================
struct vector {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
};
COD3_STATIC_ASSERT_32BIT(sizeof(vector) == 12, "Broc::vector size mismatch");

// ============================================================================
// Broc::entity — script entity handle (4 bytes)
// ============================================================================
struct entity {
    unsigned int ___u0;  // +0x00

    void UndefineEEField(unsigned int key);
};
COD3_STATIC_ASSERT_32BIT(sizeof(entity) == 4, "Broc::entity size mismatch");

// ============================================================================
// Broc::string — reference-counted (COW) string (4 bytes)
// Points to a Block allocated immediately before the string data.
// Block layout:
//   +0x00: mBuff (char*)   — pointer to data (Block + 1)
//   +0x04: mBlockSize (u16) — allocated size
//   +0x06: mLength (u16)    — string length
//   +0x08: mRefCount (i16)  — reference count
// ============================================================================
struct string {
    struct Block {
        char*          mBuff;       // +0x00
        unsigned short mBlockSize;  // +0x04
        unsigned short mLength;     // +0x06
        short          mRefCount;   // +0x08

        Block(unsigned short blockSize, unsigned short strLen, const char* txt);
        void IncrementCount(void);
        void DecrementCount(void);
        void Append(const char* txt, unsigned short strLen);
        static char* GetBuff(Block* block);
    };
    Block* mBlock;  // +0x00

    // --- lifecycle ---
    string();
    string(EUndefined);
    string(Block* b);
    string(const char* txt);
    string(const char* txt, int __formal);
    string(const string& rhs);
    string(const string& rhs, int __formal);
    string(unsigned int val);
    string(int val);
    string(float val);
    string(const struct bint& val);
    string(const struct bfloat& val);
    string(const struct bunsigned& val);
    string(const struct bbool& val);
    ~string();

    // --- assignment ---
    string& operator=(const string& rhs);
    string& operator=(const char* txt);
    string& operator=(char c);
    string& operator=(int val);
    string& operator=(float val);
    string& operator=(const vector& v);

    // --- concatenation ---
    string& operator+=(char c);
    string& operator+=(const char* txt);
    string& operator+=(const string& rhs);
    string& operator+=(int val);
    string& operator+=(unsigned int val);
    string& operator+=(float val);

    // --- access ---
    const char* c_str() const;
    char* GetBuff();

    // --- operations ---
    void  clear();
    int   find(const char* txt, unsigned int start) const;
    int   find(unsigned int start, char c) const;
    int   rfind(const char* txt) const;
    string& remove_leading(const char* chars);
    string& remove_trailing(const char* chars);
    void  set_char(unsigned int idx, char c);
    string substr(unsigned int start, unsigned int count) const;

    // --- static ---
    static Block* AllocBlock(const char* txt, unsigned int txtLen,
                             const char* sizeHint, unsigned int sizeHintLen);
    static int GetBlockSize(int hint);

private:
    void Append(const char* txt, unsigned int len);
};
COD3_STATIC_ASSERT_32BIT(sizeof(string) == 4, "Broc::string size mismatch");
COD3_STATIC_ASSERT_32BIT(sizeof(string::Block) == 12, "Broc::string::Block size mismatch");

// ============================================================================
// Broc utility functions
// ============================================================================
bool         IsAlpha(char ca);
char         ToLower(char ca);
unsigned int length(const char* txt);
int          strcmp(const char* lhs, const char* rhs);
HashStr      string_hash(const char* str);
HashStr      string_hash(const string& str);

// ============================================================================
// Broc string operators
// ============================================================================
bool   operator==(const string& lhs, const char* rhs);
bool   operator==(const string& lhs, const string& rhs);
bool   operator==(HashStr lhs, const string& rhs);
bool   operator==(const string& lhs, HashStr rhs);
bool   operator!=(HashStr lhs, const string& rhs);
bool   operator!=(const string& lhs, HashStr rhs);
string operator+(const string& lhs, const string& rhs);
string operator+(const string& lhs, const char* rhs);

// ============================================================================
// Broc::ExtendedEntity — variable storage for script entities (12 bytes)
// ============================================================================
struct ExtendedEntity {
    struct KVPair {
        unsigned int key;  // +0x00
        unsigned int val;  // +0x04
    };

    unsigned int mCount;     // +0x00
    unsigned int mCapacity;  // +0x04
    KVPair*      mKVPairs;   // +0x08

    ExtendedEntity();
    ExtendedEntity(const ExtendedEntity& other);
    ~ExtendedEntity();

    static ExtendedEntity* GetExtendedEntity(unsigned int handle);
    static void*           CreateExtendedEntity(const char** fields, int fieldCount);
    static void            DeleteExtendedEntity(void* mem);
    static bool            MatchExtendedEntityKey(void* mem, int fieldCount, const char* keyName);
    static void*           CopyExtendedEntity(const void* src);
    static void            InitScript(struct BrocExports& exports);

    bool          IsDefined(unsigned int key) const;
    void          SetUndefined(unsigned int key);
    unsigned int* SetVal(unsigned int key, const string& val);
    const unsigned int* InternalGet(unsigned int key) const;
    unsigned int* InternalSet(unsigned int key, unsigned int val);

    typedef unsigned int (InitFunc)(const char* key);
    typedef unsigned int (CopyFunc)(unsigned int val);
    typedef bool (EqualsFunc)(unsigned int val, const char* str);
    typedef void (DestructFunc)(unsigned int& val);

    static InitFunc*     GetInit(unsigned int typeId);
    static CopyFunc*     GetCopier(unsigned int typeId);
    static EqualsFunc*   GetEquals(unsigned int typeId);
    static DestructFunc* GetDestructor(unsigned int typeId);
};
COD3_STATIC_ASSERT_32BIT(sizeof(ExtendedEntity) == 12, "Broc::ExtendedEntity size mismatch");
COD3_STATIC_ASSERT_32BIT(sizeof(ExtendedEntity::KVPair) == 8, "Broc::ExtendedEntity::KVPair size mismatch");

// ============================================================================
// Broc::pathnode / Broc::vehiclenode — node handles (4 bytes each)
// ============================================================================
struct pathnode {
    unsigned int ___u0;
};
COD3_STATIC_ASSERT_32BIT(sizeof(pathnode) == 4, "Broc::pathnode size mismatch");

struct vehiclenode {
    unsigned int ___u0;
};
COD3_STATIC_ASSERT_32BIT(sizeof(vehiclenode) == 4, "Broc::vehiclenode size mismatch");

// ============================================================================
// EEHelper / EEDefault — template helpers for ExtendedEntity field types
// ============================================================================
namespace EEHelper {
    template <typename T> unsigned int Initialize(const char* key);
    template <typename T> bool Equals(unsigned int val, const char* str);
    template <typename T> unsigned int Copy(unsigned int val);
}

namespace EEDefault {
    unsigned int Initialize(const char*);
    unsigned int Copy(unsigned int val);
    void         Destruct(unsigned int& val);
    bool         Equals(unsigned int val, const char* str);
}

// ============================================================================
// PathNodes namespace — AI path node handles
// (moved to global scope below namespace Broc)
// ============================================================================

// ============================================================================
// Broc wait / thread functions
// ============================================================================
void ThreadExecute(void* functor);
void wait_accurate(float seconds);
void wait(float seconds);
void wait_frame(int frames);
void waittill(entity ent, HashStr signal);
void waittill_timeout(entity ent, HashStr signal, float timeout);
void waittillmatch(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittillor(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittill(entity ent, const char* signal);
void waittill_loaded(unsigned int pakInfo);
void waittill_unloaded(unsigned int pakInfo);
void endon(entity ent, const char* event);
void thread_sleep_time(void);
void thread_sleep_frames(void);
void thread_sleep_until_notify(void);
void thread_debug_wait_msg(int x);
void thread_debug_wait_msg(float x);
void thread_debug_wait_until(HashStr a, HashStr b, HashStr c, HashStr d);

// ============================================================================
// Broc path node functions
// ============================================================================
void GetNodeArray(const string& name1, const string& name2, void* result);
void GetAllNodes(void* result);
void GetVehicleNodeArray(const string& name1, const string& name2, void* result);
void GetAllVehicleNodes(void* result);

// ============================================================================
// Boxed types used by Broc::string constructors
// ============================================================================
struct bint {
    int mVal;
};
COD3_STATIC_ASSERT_32BIT(sizeof(bint) == 4, "bint size mismatch");

struct bfloat {
    float mVal;
};
COD3_STATIC_ASSERT_32BIT(sizeof(bfloat) == 4, "bfloat size mismatch");

struct bunsigned {
    unsigned int mVal;
};
COD3_STATIC_ASSERT_32BIT(sizeof(bunsigned) == 4, "bunsigned size mismatch");

struct bbool {
    bool mVal;
};
COD3_STATIC_ASSERT_32BIT(sizeof(bbool) == 1, "bbool size mismatch");

} // namespace Broc

// ============================================================================
// PathNodes namespace — AI path node handles (global scope, verified against IDA)
// ============================================================================
namespace PathNodes {
struct NodeHandle {
    uint16_t mValue;  // +0x00
};
} // namespace PathNodes
COD3_STATIC_ASSERT_32BIT(sizeof(PathNodes::NodeHandle) == 2, "PathNodes::NodeHandle size mismatch");

// ============================================================================
// InplaceString — in-place char* (4 bytes)
// ============================================================================
struct InplaceString {
    char* mStr;  // +0x00
};
COD3_STATIC_ASSERT_32BIT(sizeof(InplaceString) == 4, "InplaceString size mismatch");

#undef COD3_STATIC_ASSERT_32BIT

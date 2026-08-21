// ============================================================================
// COD3 Engine Types — Broc/BrocSys string, entity, hashing, and extended entity
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA (32-bit; 64-bit builds use conditional guards).
// ============================================================================

#pragma once

#include <stdint.h>

class AeThreadFunctor;
enum TPakInfo : int;

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

namespace Broc {
class string;
struct bint;
}

// ============================================================================
// HashString — global hashed string (4 bytes) — verified against IDA
// ============================================================================
class HashString {
public:
    unsigned int mHash;  // +0x00
    HashString() : mHash(0) {}
    HashString(Broc::string& str);  // ea: 0x004C1450
    HashString(const char* str);    // ??0HashString@@QAE@PBD@Z (g.o 0x4A9B60)
    HashString(int hash);           // ??0HashString@@QAE@H@Z (g.o 0x4A9B90)
    static unsigned int CalcHash(const char* str);  // ea: 0x004C1540
    static unsigned int NullHash();  // ea: 0x8990B0 (g.o inline; returns 0)
    unsigned int GetHash() const;          // ?GetHash@HashString@@QBEIXZ (g.o 0x4A9BC0)
    static bool Compare(const HashString& lhs, const HashString& rhs);  // ?Compare@HashString@@SA_NABV1@0@Z (g.o 0x4A9BD0)
};
COD3_STATIC_ASSERT_32BIT(sizeof(HashString) == 4, "HashString size mismatch");

bool operator==(const HashString& lhs, const HashString& rhs);  // ??8@YA_NABVHashString@@0@Z (g.o 0x4A9BF0)
bool operator!=(const HashString& lhs, const HashString& rhs);  // ??9@YA_NABVHashString@@0@Z (g.o 0x4A9C10)
bool operator==(const HashString& lhs, unsigned int rhs);       // ??8@YA_NABVHashString@@I@Z (g.o 0x4A9C30)
bool operator!=(const HashString& lhs, unsigned int rhs);       // ??9@YA_NABVHashString@@I@Z (g.o 0x4A9C50)

// ============================================================================
// EEndRoundCondition — round-end reason (global enum, verified values in bin).
// ============================================================================
enum EEndRoundCondition {
    kEndRoundNone = 0,
    kEndRoundTimeLimit = 1,
    kEndRoundScoreLimit = 2,
    kEndRoundLastManStanding = 3,
    kEndRoundNoPlayers = 4,
};

// ============================================================================
// EHitLocation â€” damage location (hitLocation_t). Values verified against the
// GetPhysBoneID jump table (physics.o 0x6F2F30): switch covers cases 0..18
// with 19 cases, HITLOC_NUM == 0x13. PDB member names kept exactly.
// ============================================================================
enum hitLocation_t {
    HITLOC_NONE = 0,
    HITLOC_HELMET,
    HITLOC_HEAD,
    HITLOC_NECK,
    HITLOC_TORSO_UPR,
    HITLOC_TORSO_LWR,
    HITLOC_R_ARM_UPR,
    HITLOC_L_ARM_UPR,
    HITLOC_R_ARM_LWR,
    HITLOC_L_ARM_LWR,
    HITLOC_R_HAND,
    HITLOC_L_HAND,
    HITLOC_R_LEG_UPR,
    HITLOC_L_LEG_UPR,
    HITLOC_R_LEG_LWR,
    HITLOC_L_LEG_LWR,
    HITLOC_R_FOOT,
    HITLOC_L_FOOT,
    HITLOC_GUN,
    HITLOC_NUM = 0x13,
};

typedef hitLocation_t EHitLocation;  // alias kept for early ports

enum {
    INVALID_PAK_INFO = 0,
};

// ============================================================================
// Broc namespace
// ============================================================================
namespace Broc {

// ============================================================================
// Broc::EUndefined — tag type for undefined string construction
// ============================================================================
enum EUndefined { UNDEFINED };

// ============================================================================
// Broc::dyn_array<T> - dynamic array (12 bytes: mElements/mCapacity/mSize).
// ============================================================================
template <typename T>
class dyn_array {
public:
    T*           mElements;   // +0x00
    unsigned int mCapacity;   // +0x04
    unsigned int mSize;       // +0x08

    dyn_array() : mElements(NULL), mCapacity(0), mSize(0) {}  // ea: 0x93305B
    ~dyn_array();  // ea: 0x933129
    void destroy_all();  // scr.o 0x5EF400 (Broc::dyn_array<Broc::string>)

    // ea: 0x005EDF00 / 0x005EDF10 (dyn_array<Broc::entity>)
    T* begin() { return mElements; }
    T* end() { return &mElements[mSize]; }

    void reserve(unsigned int newCapacity) {
        if (newCapacity > mCapacity) {
            T* ne = new T[newCapacity];
            for (unsigned int k = 0; k < mSize; k++)
                ne[k] = mElements[k];
            delete[] mElements;
            mElements = ne;
            mCapacity = newCapacity;
        }
    }

    void resize(unsigned int newSize, unsigned int newCapacity) {
        if (newCapacity > mCapacity)
            reserve(newCapacity);
        mSize = newSize;
    }

    void clear() { resize(0, 0); }

    void push_back(const T& elt) {
        if (mSize >= mCapacity) {
            unsigned int newCap = mCapacity ? mCapacity * 2 : 4;
            T* ne = new T[newCap];
            for (unsigned int k = 0; k < mSize; k++)
                ne[k] = mElements[k];
            delete[] mElements;
            mElements = ne;
            mCapacity = newCap;
        }
        mElements[mSize++] = elt;
    }

    T& operator[](unsigned int idx) { return mElements[idx]; }
    const T& operator[](unsigned int idx) const { return mElements[idx]; }
};

template <typename T> int size(const dyn_array<T>& ar) { return (int)ar.mSize; }
template <typename T> void push(dyn_array<T>& ar, const T& elt);

// ea: 0x005EDF90 (Broc::min_val<unsigned int>)
template <typename T>
T min_val(const T* lhs, const T* rhs)
{
    T result = *lhs;
    if (*lhs >= *rhs)
        return *rhs;
    return result;
}

// ============================================================================
// Broc::vector — 3D vector (12 bytes)
// ============================================================================
struct vector {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08

    vector() : x(0.0f), y(0.0f), z(0.0f) {}
    vector(float ix, float iy) : x(ix), y(iy), z(0.0f) {}
    vector(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
    vector& operator+=(const vector& rhs);
    vector& operator/=(float rhs)
    {
        x = x * (1.0f / rhs);
        const float zValue = (1.0f / rhs) * z;
        y = (1.0f / rhs) * y;
        z = zValue;
        return *this;
    }
    bool operator==(const vector& rhs);
    bool IsDefined() const;  // ?IsDefined@vector@Broc@@QBE_NXZ (core.o 0x4B5330)
    float operator[](int i) const { return (&x)[i]; }
    void Set(float X, float Y, float Z);  // ?Set@vector@Broc@@QAEXMMM@Z (g.o 0x4A5DE0)
};
COD3_STATIC_ASSERT_32BIT(sizeof(vector) == 12, "Broc::vector size mismatch");

// ============================================================================
// Broc::entity — script entity handle (4 bytes)
// ============================================================================
class entity {
public:
    unsigned int ___u0;  // +0x00

    // IDA local proxy types used by Broc entity field accessors.
    struct __unnamed {
        struct origin_struct {
            unsigned int mHandle;  // +0x00
            const Broc::vector* Get(Broc::vector* result) const;
        };
        struct target_struct {
            unsigned int mHandle;  // +0x00
            const Broc::string* Get(Broc::string* result) const;
        };
        struct targetname_struct {
            unsigned int mHandle;  // +0x00
            const Broc::string* Get(Broc::string* result) const;
        };
        struct maxhealth_struct {
            unsigned int mHandle;  // +0x00
            const Broc::bint* Get(Broc::bint* result) const;
        };
        struct health_struct {
            unsigned int mHandle;  // +0x00
            const Broc::bint* Get(Broc::bint* result) const;
            const int& operator=(const int& rhs);
        };
        struct rank_struct {
            unsigned int mHandle;  // +0x00
            __int16 Get() const;
            const __int16& operator=(const __int16& rhs);
        };
        struct playerState_struct {
            unsigned int mHandle;  // +0x00
            const Broc::bint* Get(Broc::bint* result) const;
            const int& operator=(const int& rhs);
        };
        struct spectatorClient_struct {
            unsigned int mHandle;  // +0x00
            const int& operator=(const int& rhs);
        };
        struct playerClass_struct {
            unsigned int mHandle;  // +0x00
            __int16 Get() const;
            const __int16& operator=(const __int16& rhs);
        };
        struct nextPlayerClass_struct {
            unsigned int mHandle;  // +0x00
            __int16 Get() const;
            const __int16& operator=(const __int16& rhs);
        };
        struct angles_struct {
            unsigned int mHandle;  // +0x00
            const Broc::vector* Get(Broc::vector* result) const;
            const Broc::vector* operator=(const Broc::vector* rhs);
        };
        struct team_struct {
            unsigned int mHandle;  // +0x00
            const Broc::string* Get(Broc::string* result) const;
        };
    };
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::origin_struct) == 4,
                            "origin_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::target_struct) == 4,
                            "target_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::targetname_struct) == 4,
                            "targetname_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::maxhealth_struct) == 4,
                            "maxhealth_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::health_struct) == 4,
                            "health_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::rank_struct) == 4,
                            "rank_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::playerState_struct) == 4,
                            "playerState_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::spectatorClient_struct) == 4,
                            "spectatorClient_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::playerClass_struct) == 4,
                            "playerClass_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::nextPlayerClass_struct) == 4,
                            "nextPlayerClass_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::angles_struct) == 4,
                            "angles_struct size mismatch");
    COD3_STATIC_ASSERT_32BIT(sizeof(__unnamed::team_struct) == 4,
                            "team_struct size mismatch");

    entity(unsigned int v);  // ??0entity@Broc@@QAE@I@Z (g.o 0x4A6250)
    entity(const entity& rhs);  // ??0entity@Broc@@QAE@ABV01@@Z (g.o 0x4A6270)
    entity() : ___u0(0) {}

    void UndefineEEField(unsigned int key);
    unsigned int GetHandle() const { return ___u0; }  // ea: 0x92F170
    bool IsDefined() const { return ___u0 != 0; }     // ea: 0x92F170
    bool operator!=(const entity& rhs) const;         // ea: 0x93F640
};
COD3_STATIC_ASSERT_32BIT(sizeof(entity) == 4, "Broc::entity size mismatch");


inline bool operator==(const entity& lhs, const entity& rhs) {
    return lhs.___u0 == rhs.___u0;
}

// ============================================================================
// Broc::string — reference-counted (COW) string (4 bytes)
// Points to a Block allocated immediately before the string data.
// Block layout:
//   +0x00: mBuff (char*)   — pointer to data (Block + 1)
//   +0x04: mBlockSize (u16) — allocated size
//   +0x06: mLength (u16)    — string length
//   +0x08: mRefCount (i16)  — reference count
// ============================================================================
class string {
public:
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
        char* GetBuff();  // ?GetBuff@Block@string@Broc@@QAEPADXZ (g.o 0x4A9C70)
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

    int length() const;
    bool is_empty() const;  // ?is_empty@string@Broc@@QBE_NXZ

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
    char operator[](unsigned int idx);  // ??Astring@Broc@@QAEDI@Z (g.o 0x4A9C80)
    bool IsDefined() const;             // ?IsDefined@string@Broc@@QBE_NXZ (g.o 0x4A9CD0)
    void SetUndefined();                // ?SetUndefined@string@Broc@@QAEXXZ (g.o 0x4A9CE0)

    // --- operations ---
    void  clear();
    int   find(const char* txt, unsigned int start) const;
    int   find(unsigned int start, char c) const;
    int   rfind(const char* txt) const;
    string& remove_leading(const char* chars);
    string& remove_trailing(const char* chars);
    string& remove_surrounding_whitespace();
    void  set_char(unsigned int idx, char c);
    string substr(unsigned int start, unsigned int count) const;

    // --- static ---
    static Block* AllocBlock(const char* txt, unsigned int txtLen,
                             unsigned int sizeHint);
    static int GetBlockSize(int hint);

private:
    void Append(const char* txt, unsigned int len);
};
COD3_STATIC_ASSERT_32BIT(sizeof(string) == 4, "Broc::string size mismatch");
COD3_STATIC_ASSERT_32BIT(sizeof(string::Block) == 12, "Broc::string::Block size mismatch");

// Broc::collResult — script trace result (IDA type 5174)
struct collResult {
    float mFraction;     // +0x00
    Broc::vector mPosition;  // +0x04
    Broc::entity mEnt;   // +0x10
    Broc::vector mNormal;  // +0x14
    Broc::string mSurfaceType;  // +0x20
};

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
bool   operator!=(const string& lhs, const char* rhs);
bool   operator!=(const string& lhs, const string& rhs);
bool   operator!=(HashStr lhs, const string& rhs);
bool   operator!=(const string& lhs, HashStr rhs);
string operator+(const string& lhs, const string& rhs);
string operator+(const string& lhs, const char* rhs);
string operator+(const string& lhs, float rhs);  // ea: 0x934830

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

    static ExtendedEntity nullEnt; // IDA global @ 0x10F197C

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

    // IDA local type: mp_level_wad supplies these callback implementations.
    typedef void (__cdecl *GetFunctionsFunc)(unsigned int key,
                                              InitFunc** init,
                                              CopyFunc** copy,
                                              EqualsFunc** equals,
                                              DestructFunc** dtor);
    static GetFunctionsFunc sGetFunctions;

    static InitFunc*     GetInit(unsigned int typeId);
    static CopyFunc*     GetCopier(unsigned int typeId);
    static EqualsFunc*   GetEquals(unsigned int typeId);
    static DestructFunc* GetDestructor(unsigned int typeId);

    // GetRef<T> / GetVal<T> - typed accessors (mp_util_wad.o COMDATs).
    template <typename T> T& GetRef(unsigned int key) {
        return *(T*)InternalGet(key);
    }
    template <typename T> const T* GetVal(void* result, unsigned int key) const {
        const unsigned int* p = InternalGet(key);
        if (p == NULL)
            return NULL;
        *(T*)result = *(const T*)p;
        return (const T*)result;
    }
};
COD3_STATIC_ASSERT_32BIT(sizeof(ExtendedEntity) == 12, "Broc::ExtendedEntity size mismatch");
COD3_STATIC_ASSERT_32BIT(sizeof(ExtendedEntity::KVPair) == 8, "Broc::ExtendedEntity::KVPair size mismatch");

// ============================================================================
// Broc::pathnode / Broc::vehiclenode — node handles (4 bytes each)
// ============================================================================
enum TPathnodeHandle : int {
    INVALID_PATHNODE_HANDLE = 0,
};

enum TVehiclenodeHandle : int {
    INVALID_VEHICLENODE_HANDLE = -1,
};

class pathnode {
public:
    unsigned int ___u0;

    pathnode() : ___u0(0) {}
    explicit pathnode(int value) : ___u0((unsigned int)value) {}
    TPathnodeHandle GetHandle() const;
    bool IsDefined() const;
};
// Binary mangle uses class tag V for pathnode (PAVpathnode@Broc@@).
COD3_STATIC_ASSERT_32BIT(sizeof(pathnode) == 4, "Broc::pathnode size mismatch");

// Binary mangle uses class tag V for vehiclenode (ABVvehiclenode@Broc@@).
class vehiclenode {
public:
    unsigned int ___u0;

    vehiclenode() : ___u0(0) {}
    explicit vehiclenode(int value) : ___u0((unsigned int)value) {}
    TVehiclenodeHandle GetHandle() const;
    bool IsDefined() const;
};
COD3_STATIC_ASSERT_32BIT(sizeof(vehiclenode) == 4, "Broc::vehiclenode size mismatch");

// ============================================================================
// Broc::hudelem - HUD element handle (4 bytes).
// ============================================================================
class hudelem {
public:
    unsigned int ___u0;

    hudelem();
    hudelem(unsigned int v);
    unsigned int GetIndex() const;

    void SetUndefined();             // ea: 0x92F7D0
    void x(int v);                   // hudelem.x property
    void y(int v);                   // hudelem.y property
    void sort(float v);              // hudelem.sort property
    void alpha(unsigned char v);     // hudelem.alpha property
};
COD3_STATIC_ASSERT_32BIT(sizeof(hudelem) == 4, "Broc::hudelem size mismatch");

// ============================================================================
// EEHelper / EEDefault — template helpers for ExtendedEntity field types
// ============================================================================
namespace EEHelper {
    template <typename T> struct EqualsArg {
        typedef unsigned int type;
    };
    template <> struct EqualsArg<float> {
        typedef float type;
    };

    template <typename T> unsigned int Initialize(const char* key);
    template <typename T>
    bool Equals(typename EqualsArg<T>::type val, const char* str);
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
void ThreadExecute(AeThreadFunctor* functor);
void wait_accurate(float seconds);
void wait(float seconds);
void wait_frame(int frames);
void waittill(entity ent, HashStr signal);
void waittill(entity ent, HashStr signal, entity* output);
void waittill_timeout(entity ent, HashStr signal, float timeout);
void waittillmatch(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittillor(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittill(entity ent, const char* signal);
void waittill_loaded(TPakInfo pakInfo);
void waittill_unloaded(TPakInfo pakInfo);
void endon(entity ent, const char* event);
void endon(entity ent, HashStr label);
void thread_sleep_time(void);
void thread_sleep_frames(void);
void thread_sleep_until_notify(void);
void thread_debug_wait_msg(int x);
void thread_debug_wait_msg(float x);
void thread_debug_wait_until(HashStr a, HashStr b, HashStr c, HashStr d);

// ============================================================================
// Broc path node functions
// ============================================================================
void GetNodeArray(const string& name1, const string& name2,
                  dyn_array<pathnode>* result);
void GetAllNodes(dyn_array<pathnode>* result);
void GetVehicleNodeArray(const string& name1, const string& name2,
                         dyn_array<vehiclenode>* result);
void GetAllVehicleNodes(dyn_array<pathnode>* result);
void GetAllVehicleNodes(dyn_array<vehiclenode>* result);

// ============================================================================
// Boxed types used by Broc::string constructors
// ============================================================================
struct bint {
    int mVal;

    bint() : mVal(0) {}
    bint(int v) : mVal(v) {}
    // ea: 0x936EF0
    bint(unsigned int v) : mVal(v) {}
    int operator=(int v) { mVal = v; return v; }
    unsigned int operator=(unsigned int v) { mVal = v; return v; }
    operator int() const { AssertDefined(); return mVal; }
    // ea: 0x936F20
    bool operator==(int rhs) const { return rhs == mVal; }
    bool operator!=(int rhs) const;  // ea: 0x93BD20
    int operator++(int) { AssertDefined(); return ++mVal; }
    int operator++() { AssertDefined(); return mVal++; }
    int operator--(int) { AssertDefined(); return --mVal; }
    int operator+=(int v) { AssertDefined(); mVal += v; return mVal; }
    bint& operator-=(int v) { AssertDefined(); mVal -= v; return *this; }
    void AssertDefined() const {}  // ea: 0x934790
};
COD3_STATIC_ASSERT_32BIT(sizeof(bint) == 4, "bint size mismatch");

struct bfloat {
    float mVal;

    bfloat() : mVal(0.0f) {}
    bfloat(float v) : mVal(v) {}
    // ea: 0x935840
    bfloat(int v) : mVal((float)v) {}
    // ea: 0x93AE50
    bfloat(const bint& rhs) : mVal((float)rhs.mVal) {}
    double operator=(float v) { mVal = v; return v; }
    double operator=(int v) { mVal = (float)v; return mVal; }
    operator float() const { return mVal; }
    double operator+=(float v) { AssertDefined(); mVal += v; return mVal; }
    bfloat& operator-=(float v) { mVal -= v; return *this; }
    void AssertDefined() const {}  // ea: 0x93B250
};
COD3_STATIC_ASSERT_32BIT(sizeof(bfloat) == 4, "bfloat size mismatch");

struct bunsigned {
    unsigned int mVal;
};
COD3_STATIC_ASSERT_32BIT(sizeof(bunsigned) == 4, "bunsigned size mismatch");

struct bbool {
    bool mVal;

    bbool() : mVal(false) {}
    bbool(bool v) : mVal(v) {}
    double operator=(bool v) { mVal = v; return (double)v; }
    operator bool() const { return mVal; }
};
COD3_STATIC_ASSERT_32BIT(sizeof(bbool) == 1, "bbool size mismatch");

// Boxed-type comparison operators (mp_util_wad.o inline COMDATs).
bbool operator<(bint lhs, bint rhs);
bool operator>(bint lhs, bint rhs);
bool operator==(bint lhs, bint rhs);
bool operator!=(bint lhs, bint rhs);
bool operator<(bfloat lhs, bfloat rhs);
bool operator>(bfloat lhs, bfloat rhs);
bool operator==(bfloat lhs, bfloat rhs);
bool operator!=(bfloat lhs, bfloat rhs);
bbool operator<(bfloat lhs, float rhs);
bbool operator>(bfloat lhs, float rhs);
bfloat operator*(float lhs, bfloat rhs);
bfloat operator*(bfloat lhs, int rhs);
bbool operator>(bint lhs, int rhs);
bbool operator<(int lhs, bfloat rhs);
bint operator+(bint lhs, bint rhs);
bint operator+(bint lhs, int rhs);
bint operator+(int lhs, bint rhs);
bint operator-(bint lhs, bint rhs);
bint operator*(bint lhs, bint rhs);
bint operator*(int lhs, bint rhs);
bint operator*(bint lhs, int rhs);
bfloat operator*(bfloat lhs, float rhs);
bfloat operator*(bfloat lhs, bfloat rhs);
bool operator<(bint lhs, int rhs);

} // namespace Broc

// Global boxed operators emitted by mp_util_wad.o.
struct bfloat {
    float mVal;
    explicit bfloat(float v) : mVal(v) {}
    bfloat(long double v);
    operator float() const { return mVal; }
};
COD3_STATIC_ASSERT_32BIT(sizeof(bfloat) == 4, "global bfloat size mismatch");

struct bint {
    int mVal;
    explicit bint(int v) : mVal(v) {}
    bint(const bfloat& rhs);
};
bint operator+(bint lhs, bint rhs);
bint operator*(bint lhs, int rhs);
bfloat operator*(int lhs, bfloat rhs);
struct bbool {
    bool mVal;
    explicit bbool(bool v) : mVal(v) {}
    bool operator==(bool rhs) const;
};
Broc::bbool operator>(Broc::bfloat lhs, int rhs);
Broc::bbool operator<(float lhs, Broc::bfloat rhs);
Broc::bfloat operator*(Broc::bfloat lhs, Broc::bfloat rhs);
Broc::bfloat operator+(float lhs, Broc::bfloat rhs);
Broc::vector AnglesToForward(const Broc::vector& angles);
float DistanceSquared(const Broc::vector& a, const Broc::vector& b);

// ============================================================================
// Broc free helpers used by the mp_util_wad accessor layer.
// ============================================================================
namespace Broc {
bool IsDefined(const Broc::entity& e);              // ea: 0x92F130
bool IsDefined(const Broc::vector& v);              // ea: 0x92F150
bool IsDefined(const Broc::string& s);              // ea: 0x92F6F0
inline bool IsDefined(const Broc::hudelem& h) { return h.___u0 != 0; }
template <typename T> bool IsDefined(const T& t);   // boxed-type IsDefined

extern Broc::entity gEntityUndef;

// ============================================================================
// BrocExports - the Broc script runtime export table (456 bytes, 114 members).
// Verified against IDA struct BrocExports (size 0x1C8).
// ============================================================================
struct BrocExports {
    void (*mValidateApiSize)(int, int);                       // +0x00
    void* (*mCreateExtendedEntity)(const char**, int);        // +0x04
    void (*mDeleteExtendedEntity)(void*);                     // +0x08
    bool (*mMatchExtendedEntityKey)(void*, const int, const char*);  // +0x0C
    void* (*mCopyExtendedEntity)(const void*);                // +0x10
    void (*mSetPNodeField_string)(unsigned int, unsigned int, Broc::string);  // +0x14
    Broc::string* (*mGetPNodeField_string)(Broc::string*, unsigned int, unsigned int);  // +0x18
    void (*mSetPNodeField_int)(unsigned int, unsigned int, int);  // +0x1C
    int (*mGetPNodeField_int)(unsigned int, unsigned int);    // +0x20
    void (*mSetPNodeField_float)(unsigned int, unsigned int, float);  // +0x24
    float (*mGetPNodeField_float)(unsigned int, unsigned int);  // +0x28
    void (*mSetPNodeField_pathnode)(unsigned int, unsigned int, Broc::pathnode);  // +0x2C
    Broc::pathnode* (*mGetPNodeField_pathnode)(Broc::pathnode*, unsigned int, unsigned int);  // +0x30
    void (*mSetVNodeField_string)(unsigned int, unsigned int, Broc::string);  // +0x34
    Broc::string* (*mGetVNodeField_string)(Broc::string*, unsigned int, unsigned int);  // +0x38
    void (*mSetVNodeField_int)(unsigned int, unsigned int, int);  // +0x3C
    int (*mGetVNodeField_int)(unsigned int, unsigned int);    // +0x40
    void (*mSetVNodeField_float)(unsigned int, unsigned int, float);  // +0x44
    float (*mGetVNodeField_float)(unsigned int, unsigned int);  // +0x48
    void (*mSetVNodeField_vehiclenode)(unsigned int, unsigned int, Broc::vehiclenode);  // +0x4C
    Broc::vehiclenode* (*mGetVNodeField_vehiclenode)(Broc::vehiclenode*, unsigned int, unsigned int);  // +0x50
    unsigned int (*mAnimGetBroValue)(unsigned int, unsigned int);  // +0x54
    void (*mAnimIndexResolver)(const int, const int, const int, const int);  // +0x58
    bool (*mAnimIndexValidate)();                             // +0x5C
    bool (*mGetNextAnimtree)(const int, char*, const int);    // +0x60
    void (*mAnimCleanup)();                                   // +0x64
    void (*mAnimInitialize)();                                // +0x68
    unsigned int (*mAnimResolver)(const char*, const char*);  // +0x6C
    const char* (*mAnimNameResolver)(unsigned int);           // +0x70
    void (*mRegisterDebugStrings)();                          // +0x74
    void* (*mGetScriptExplodedMap)();                         // +0x78
    void (*mSetScriptExplodedMap)(void*);                     // +0x7C
    unsigned int (*mSpawnScriptThread)(unsigned int, bool, Broc::entity, Broc::entity, Broc::entity, float, float, float, Broc::vector*);  // +0x80
    bool (*mScriptThreadExists)(unsigned int);                // +0x84
    void (*mThreadExecute)(AeThreadFunctor*);                 // +0x88
    void (*mScriptedInit)(const Broc::entity, void*);         // +0x8C
    void (*mAnimDebug)(const Broc::entity);                   // +0x90
    void (*mShutdown)();                                      // +0x94
    void (*mCallbackPlayerJoin)(const Broc::entity, const unsigned int, const int);  // +0x98
    void (*mCallbackPlayerEnter)(const Broc::entity, int);    // +0x9C
    void (*mCallbackPlayerLeave)(const Broc::entity);         // +0xA0
    void (*mCallbackPainFlinch)(const Broc::entity, int);     // +0xA4
    void (*mCallbackPlayerDamage)(const Broc::entity, const Broc::entity, const Broc::entity, const Broc::vector*, const Broc::vector*, int, int, int, int);  // +0xA8
    void (*mCallbackPlayerKilled)(const Broc::entity, const Broc::entity, const Broc::entity, int, int, int);  // +0xAC
    void (*mCallbackPlayerAssist)(const Broc::entity);        // +0xB0
    void (*mCallbackPlayerRespawnRequest)(const Broc::entity, int);  // +0xB4
    void (*mCallbackPlayerSpawn)(const Broc::entity, int);    // +0xB8
    void (*mCallbackPlayerRevive)(const Broc::entity, const Broc::entity);  // +0xBC
    void (*mCallbackPlayerTeamChange)(const Broc::entity, const int, const int);  // +0xC0
    int (*mCallbackCanTeamChange)(const Broc::entity, const int);  // +0xC4
    void (*mCallbackPlayerClassChange)(const Broc::entity, const unsigned int);  // +0xC8
    void (*mCallbackVehicleKilled)(const Broc::entity, const Broc::entity, const Broc::entity, int, int, int);  // +0xCC
    void (*mCallbackVehicleMantled)(const Broc::entity, const Broc::entity);  // +0xD0
    void (*mCallbackSpotted)(const Broc::entity, const Broc::entity);  // +0xD4
    void (*mCallbackMineFailed)(const Broc::entity);          // +0xD8
    void (*mCallbackReviveFailed)(const Broc::entity);        // +0xDC
    void (*mCallbackCallForMedic)(const Broc::entity);        // +0xE0
    void (*mCallbackPunishedForTeamKill)(const Broc::entity, int);  // +0xE4
    void (*mCallbackSpawnButtonPressed)(const Broc::entity);  // +0xE8
    void (*mCallbackHealthRegenRecovering)(const Broc::entity);  // +0xEC
    void (*mCallbackRoundOver)(int, const Broc::string);      // +0xF0
    void (*mCallbackNextRound)();                             // +0xF4
    void (*mCallbackRestartMap)();                            // +0xF8
    void (*mCallbackQuitGame)();                              // +0xFC
    void (*mCallbackHostOptionsChanged)(int);                 // +0x100
    void (*mCallbackGameScore)(int, int);                     // +0x104
    void (*mCallbackGameState)(int, int, int, int, int, int, int, int, int, int, int, int, int);  // +0x108
    void (*mCallbackGameStateCTF)(const Broc::vector, const Broc::vector, const Broc::entity, const Broc::vector, const Broc::vector, const Broc::entity);  // +0x10C
    void (*mCallbackGameStateHQ)(const unsigned int, const Broc::vector, const Broc::vector, const unsigned int, int, int);  // +0x110
    void (*mCallbackGameStateSCF)(const int, const Broc::vector, const Broc::vector, const Broc::entity);  // +0x114
    void (*mCallbackGameStateDOM)(const int, const int, const int, const int, const int);  // +0x118
    void (*mCallbackGameStateSD)(const Broc::entity, const Broc::entity, const int, const Broc::vector*, const Broc::vector*, const int);  // +0x11C
    void (*mCallbackDropItem)(const int, const int, const Broc::vector, const Broc::vector, const Broc::vector);  // +0x120
    void (*mCallbackPickupScriptItem)(const int, const Broc::entity, const int);  // +0x124
    void (*mCallbackDropFlag)(const Broc::entity);            // +0x128
    void (*mCallbackPickupItem)(const Broc::entity, const Broc::entity);  // +0x12C
    void (*mCallbackAreaCaptured)(const int, const int);      // +0x130
    void (*mCallbackZonesLoaded)();                           // +0x134
    void (*mCallbackSDHostBombRequest)(const Broc::entity, const int);  // +0x138
    void (*mCallbackSDBombExplosion)();                       // +0x13C
    void (*mCallbackSDBombOperation)(const Broc::entity, const int);  // +0x140
    void (*mCallbackSDBombOperationEvent)(const Broc::entity, const int, const int);  // +0x144
    void (*mCallbackHostDisconnected)();                      // +0x148
    void (*mCallbackHostMigrated)();                          // +0x14C
    void (*mCallbackLocalPlayerKicked)();                     // +0x150
    void (*mCallbackStopFollowing)();                         // +0x154
    void (*mCallbackFireArtillery)(const Broc::entity, const Broc::vector);  // +0x158
    void (*mCallbackFireArtilleryShell)(const Broc::entity);  // +0x15C
    void (*mCallbackDenyArtillery)(const Broc::entity);       // +0x160
    int (*mCallbackGetTeamWeapon)(const char*, const unsigned int);  // +0x164
    int (*mCallbackGetGrenadeCount)(const unsigned int, const unsigned int);  // +0x168
    int (*mCallbackGetClipCount)(const unsigned int, const unsigned int, const int);  // +0x16C
    int (*mCallbackGetSlotClipCount)(const char*, const unsigned int, const unsigned int, const int);  // +0x170
    void (*mCallbackGiveAmmoPack)(const Broc::entity, const unsigned int);  // +0x174
    int (*mCallbackCanPickupAmmoPack)(const Broc::entity);    // +0x178
    int (*mCallbackGetFlagBeingContested)(const Broc::entity);  // +0x17C
    void (*mCallbackPickupKit)(const Broc::entity, const unsigned int);  // +0x180
    int (*mCallbackGetTeamCapturingHQPercent)(const Broc::entity);  // +0x184
    int (*mCallbackGetTeamDestroyingHQPercent)();             // +0x188
    int (*mCallbackGetHQCaptureStatus)();                     // +0x18C
    int (*mCallbackGetFlagCount)();                           // +0x190
    int (*mCallbackGetTeamControllingFlag)(const unsigned int);  // +0x194
    int (*mCallbackGetFlagBeingCaptured)();                   // +0x198
    int (*mCallbackGetTeamCapturingFlag)();                   // +0x19C
    int (*mCallbackGetCapturingFlagPercent)();                // +0x1A0
    int (*mCallbackGetHQPercent)();                           // +0x1A4
    int (*mCallbackGetFlagBreatherTime)();                    // +0x1A8
    void (*mCallbackPlayerTotalScore)(int, int);              // +0x1AC
    void (*mCallbackSetLevelAudio)(const char*, const char*, const char*, int, int);  // +0x1B0
    int (*mCallbackShowFlagHint)();                           // +0x1B4
    void (*mCallbackDebugRender)();                           // +0x1B8
    void (*mInit)();                                          // +0x1BC
    void (*mRegisterFunction)(const char*, unsigned int (*)(Broc::entity));  // +0x1C0
    unsigned int mMainThreadHandle;                           // +0x1C4
};
COD3_STATIC_ASSERT_32BIT(sizeof(BrocExports) == 0x1C8, "BrocExports size mismatch");

// ============================================================================
// BrocAPI - the Broc scripting runtime interface (4924 bytes, 1118 members).
// Only the members used by ported code are declared; the rest is padding.
// ============================================================================

struct BrocAPI {
    void (*mPrint)(const char*);                          // +0x000
    void (*mPrintLn)(const char*);                        // +0x004
    char _pad08[0x34 - 0x08];                             // +0x008
    bool mKillThread;                                     // +0x034
    unsigned char _pad35[0x38 - 0x35];                    // +0x035
    char _pad38[0x44 - 0x38];                             // +0x038
    void (*mThreadBackupStack)(unsigned int);             // +0x044
    void (*mThreadGetDebugInfo)(Broc::string*, Broc::string*, Broc::string*); // +0x048
    void (*mThreadSleepInternal)(float);                  // +0x04C
    void (*mThreadSleepFrames)(int);                      // +0x050
    void (*mThreadSleepUntilNotify)(unsigned int, TPakInfo, int, int, int, int, bool, float); // +0x054
    void (*mThreadTerminateOnNotify)(unsigned int, const int); // +0x058
    char _pad5C[0x68 - 0x5C];                             // +0x05C
    void (*mThreadDebugNotice)(const char*);              // +0x068
    unsigned int (*mThreadGetId)();                       // +0x06C
    char _pad70[0x74 - 0x70];                             // +0x070
    void (*mEntNotify)(unsigned int, unsigned int);       // +0x074
    char _pad78[0x8C - 0x78];                             // +0x078
    void (*mAddEventHandler)(unsigned int, unsigned int, unsigned int); // +0x08C
    char _pad90[0x94 - 0x90];                             // +0x090
    unsigned int (*mGetEnt)(const Broc::string*, int, unsigned int*, int, int);  // +0x094
    unsigned int (*mGetEntByNum)(int);                    // +0x098
    char _pad9C[0xA4 - 0x9C];                             // +0x09C
    int (*mGetNode)(const Broc::string*, const Broc::string*, int*, int); // +0x0A4
    char _padA8[0xAC - 0xA8];                             // +0x0A8
    int (*mGetVehicleNode)(const Broc::string*, const Broc::string*, int*, int); // +0x0AC
    char _padB0[0xB8 - 0xB0];                            // +0x0B0
    int (*mEffectEventPlay)(unsigned int, const Broc::string*, int,
                            bool, bool);                 // +0x0B8
    int (*mEffectEventPlayNonEnt)(const Broc::string*, const Broc::vector*,
                                  const Broc::vector*, bool, unsigned int,
                                  int);                   // +0x0BC
    int (*mEffectEventPlayDir)(unsigned int, const Broc::string*,
                               const Broc::vector*, int, bool); // +0x0C0
    int (*mEffectEventQueue)(unsigned int, const Broc::string*, int,
                             bool, bool);                 // +0x0C4
    int (*mEffectEventQueueDialog)(unsigned int, const Broc::string*, int,
                                   bool);                  // +0x0C8
    void (*mEffectEventPlayQueued)(unsigned int);          // +0x0CC
    bool (*mEffectEventIsStillPlaying)(unsigned int);      // +0x0D0
    void (*mEffectEventStop)(unsigned int);                // +0x0D4
    void (*mEffectEventFastForward)(unsigned int, float);  // +0x0D8
    int (*mEffectEventWeaponPlay)(unsigned int, unsigned int); // +0x0DC
    char _padE0[0xF8 - 0xE0];                              // +0x0E0
    int (*mDialogPlay)(unsigned int, const Broc::string*, int,
                       bool);                              // +0x0F8
    char _padFC[0x118 - 0xFC];                            // +0x0FC
    int (*mEntityIsAlive)(unsigned int);                  // +0x118
    int (*mEntityExists)(unsigned int);                   // +0x11C
    int (*mEntityIsPlayer)(unsigned int);                 // +0x120
    int (*mEntityIsAI)(unsigned int);                     // +0x124
    int (*mEntityIsSentient)(unsigned int);               // +0x128
    int (*mEntityIsVehicle)(unsigned int);                // +0x12C
    int (*mEntityIsVehicleTank)(unsigned int);            // +0x130
    int (*mEntityIsWounded)(unsigned int);                // +0x134
    int (*mIsPathNodeDefined)(unsigned int);              // +0x138
    int (*mIsVehicleNodeDefined)(unsigned int);           // +0x13C
    int (*mMathsRandomInt)(int);                          // +0x140
    char _pad144[0x148 - 0x144];                          // +0x144
    int (*mMathsRandomIntRange)(int, int);                // +0x148
    float (*mMathsRandomFloatRange)(float, float);         // +0x14C
    char _pad150[0x184 - 0x150];                          // +0x150
    float (*mVecDistance)(const Broc::vector*, const Broc::vector*);  // +0x184
    float (*mVecDistanceSquared)(const Broc::vector*, const Broc::vector*); // +0x188
    char _pad18C[0x1A0 - 0x18C];                          // +0x18C
    void (*mVecToAngles)(Broc::vector*, const Broc::vector*);  // +0x1A0
    char _pad1A4[0x1AC - 0x1A4];                          // +0x1A4
    void (*mVecAnglesToForward)(Broc::vector*, const Broc::vector*); // +0x1AC
    char _pad1B0[0x24C - 0x1B0];                          // +0x1B0
    void (*mCVarGetString)(Broc::string*, const char*);   // +0x24C
    int (*mCVarGetInt)(const char*);                      // +0x250
    float (*mCVarGetFloat)(const char*);                  // +0x254
    void (*mCVarSetString)(const char*, const char*);     // +0x258
    void (*mCVarSetInt)(const char*, int);                // +0x25C
    void (*mCVarSetFloat)(const char*, float);            // +0x260
    unsigned int (*mSpawn)(const Broc::string*, const Broc::vector*,
                           TPakInfo);                    // +0x264
    char _pad268[0x290 - 0x268];                          // +0x268
    unsigned int (*mSoundPlay)(const Broc::string*, float); // +0x290
    char _pad294[0x2A0 - 0x294];                          // +0x294
    void (*mSoundCrossFade)(unsigned int, unsigned int, float); // +0x2A0
    char _pad2A4[0x2C0 - 0x2A4];                          // +0x2A4
    void (*mReverbSetParams)(const Broc::string*, bool);  // +0x2C0
    char _pad2C4[0x2EC - 0x2C4];                          // +0x2C4
    void (*mPlayerRespawn)(unsigned int, const Broc::vector*,
                           const Broc::vector*, const Broc::string*); // +0x2EC
    void (*mPlayerSpawn)(unsigned int, const Broc::vector*,
                         const Broc::vector*, bool);      // +0x2F0
    void (*mSetPlayerAlive)(unsigned int, int);            // +0x2F4
    void (*mSetRespawnMaxTime)(unsigned int, int);         // +0x2F8
    const char* (*mGetPlayerName)(unsigned int);           // +0x2FC
    void (*mRequestRespawn)(unsigned int);                 // +0x300
    void (*mFinishDamage)(unsigned int, unsigned int, unsigned int,
                          const Broc::vector&, const Broc::vector&, int,
                          int, int, int);                  // +0x304
    void (*mRoundOver)(int, const Broc::string*);           // +0x308
    bool (*mIsLocalPlayer)(unsigned int);                  // +0x30C
    bool (*mIsInVehicle1)(unsigned int);                   // +0x310
    bool (*mIsInVehicle2)(unsigned int, unsigned int);     // +0x314
    unsigned int (*mGetPlayerInSeat)(unsigned int, unsigned int); // +0x318
    void (*mGetOutOfVehicle)(unsigned int);                // +0x31C
    void (*mObituary)(unsigned int, unsigned int, const Broc::string&, int,
                      bool);                               // +0x320
    void (*mClearPlayerStats)();                            // +0x324
    void (*mIncPlayerStat)(unsigned int, unsigned int, short); // +0x328
    char _pad32C[0x33C - 0x32C];                           // +0x32C
    void (*mIncTeamScore)(const Broc::string&, int);      // +0x33C
    int (*mGetTeamScore)(const Broc::string&);            // +0x340
    char _pad344[0x370 - 0x344];                          // +0x344
    void (*mSendGameState)(unsigned int, int, int, int, int,
                           bool, bool, bool, int, int, int, bool, int, int); // +0x370
    void (*mSendGameStateHQ)(unsigned int, unsigned int, const Broc::vector&,
                             const Broc::vector&, unsigned int, bool, bool); // +0x374
    void (*mSendGameStateCTF)(unsigned int, const Broc::vector&,
                              const Broc::vector&, unsigned int,
                              const Broc::vector&, const Broc::vector&,
                              unsigned int);                // +0x378
    void (*mSendGameStateSCF)(unsigned int, int, const Broc::vector&,
                              const Broc::vector&, unsigned int); // +0x37C
    void (*mSendGameStateDOM)(unsigned int, int, int, int, int, int); // +0x380
    void (*mSendGameStateSD)(unsigned int, unsigned int, unsigned int, bool,
                             const Broc::vector&, const Broc::vector&, int); // +0x384
    void (*mSendGameScore)(int, int);                       // +0x388
    void (*mEnterGame)();                                   // +0x38C
    void (*mDebugOut)(const char*);                         // +0x390
    bool (*mControllerErrorMessageUp)();                    // +0x394
    void (*mForceControllerErrorMessageDown)();             // +0x398
    void (*mDropItem1)(int, int, const Broc::vector*, const Broc::vector*); // +0x39C
    void (*mDropItem2)(int, int, const Broc::vector*, const Broc::vector*, const Broc::vector*); // +0x3A0
    void (*mHostDropItem1)(int, int, const Broc::vector*, const Broc::vector*); // +0x3A4
    void (*mHostDropItem2)(int, int, const Broc::vector*, const Broc::vector*, const Broc::vector*); // +0x3A8
    void (*mPickupItem)(int, unsigned int);                  // +0x3AC
    void (*mAreaCaptured)(int, unsigned int, unsigned int);  // +0x3B0
    void (*mSendHostBombRequest)(unsigned int, bool);        // +0x3B4
    void (*mSendBombExplosion)(unsigned int);                // +0x3B8
    void (*mSendBombOperation)(unsigned int, bool);          // +0x3BC
    void (*mSendBombOperationEvent)(unsigned int, bool, bool); // +0x3C0
    unsigned int (*mGetSpotterEntity)(unsigned int);         // +0x3C4
    void (*mClearSpottingFromOccupants)(unsigned int);       // +0x3C8
    void (*mGetWeaponName)(unsigned int, Broc::string*);  // +0x3CC
    char _pad3D0[0x528 - 0x3D0];                          // +0x3D0
    unsigned int (*mGetTime)();                           // +0x528
    char _pad52C[0x6D8 - 0x52C];                          // +0x52C
    void (*mDelete)(unsigned int);                        // +0x6D8
    char _pad6DC[0x6E8 - 0x6DC];                          // +0x6DC
    void (*mSetModel)(unsigned int, const Broc::string*, TPakInfo); // +0x6E8
    void (*mSetModelIndex)(unsigned int, int);            // +0x6EC
    float (*mGetNormalHealth)(unsigned int);              // +0x6F0
    void (*mSetNormalHealth)(unsigned int, float);        // +0x6F4
    void (*mDoDamage)(unsigned int, float, const Broc::vector*, hitLocation_t); // +0x6F8
    void (*mSetTakeDamage)(unsigned int, int);            // +0x6FC
    char _pad700[0x9E8 - 0x700];                          // +0x700
    void (*mMoveTo)(unsigned int, const Broc::vector*, float, float,
                    float);                              // +0x9E8
    char _pad9EC[0x9FC - 0x9EC];                          // +0x9EC
    void (*mRotateTo)(unsigned int, const Broc::vector&, float, float, float); // +0x9FC
    char _padA00[0xA8C - 0xA00];                          // +0xA00
    int (*mOpenMenu)(const Broc::string*, int);            // +0xA8C
    int (*mIsMenuOpen)(const Broc::string*, int);          // +0xA90
    int (*mOpenMenuNoMouse)(unsigned int, const Broc::string*); // +0xA94
    void (*mCloseMenu1)(unsigned int);                    // +0xA98
    void (*mCloseMenu2)(const Broc::string*, int);        // +0xA9C
    void (*mCloseAllMenus)(int);                           // +0xAA0
    void (*mSetSpectateState)(int, int);                   // +0xAA4
    void (*mSetSpectateSeconds)(int, int);                 // +0xAA8
    void (*mSetSpectateMedic)(int, int);                   // +0xAAC
    void (*mSetSpectateTeamKill)(int, unsigned int, int);  // +0xAB0
    char _padAB4[0xBD0 - 0xAB4];                          // +0xAB4
    void* (*mPoolAlloc)(unsigned int);                    // +0xBD0
    void (*mPoolFree)(void*);                              // +0xBD4
    char _padBD8[0xBDC - 0xBD8];                          // +0xBD8
    bool (*mAssert)(const char* file, int line, const char* msg);  // +0xBDC
    bool (*mWarning)(const char* file, int line, const char* msg);  // +0xBE0
    bool (*mError)(const char* file, int line, const char* msg);  // +0xBE4
    struct BrocExports mBrocExports;                      // +0xBE8
    float (*mAtoF)(const char*);                           // +0xDB0
    int (*mAtoI)(const char*);                             // +0xDB4
    unsigned int (*mStringHash)(const char*);              // +0xDB8
    void* (*mGetExtendedEntity)(unsigned int);             // +0xDBC
    char _padBrocExports[0xE00 - (0xBE8 + 0x1C8 + 0x10)]; // +0xDC0
    void (*mRegisterHashString)(int, const char*);         // +0xE00
    const char* (*mHashToStr)(int);                        // +0xE04
    char _padE08[0xF1C - 0xE08];                           // +0xE08
    Broc::vector* (*m_entity_get_origin)(Broc::vector*, unsigned int);  // +0xF1C
    void (*m_entity_set_origin)(unsigned int, Broc::vector);  // +0xF20
    Broc::string* (*m_entity_get_model)(Broc::string*, unsigned int);  // +0xF24
    void (*m_entity_set_model)(unsigned int, Broc::string);  // +0xF28
    char _padF2C[0xF4C - 0xF2C];                          // +0xF2C
    Broc::string* (*m_entity_get_target)(Broc::string*, unsigned int);  // +0xF4C
    void (*m_entity_set_target)(unsigned int, Broc::string);  // +0xF50
    Broc::string* (*m_entity_get_targetname)(Broc::string*, unsigned int);  // +0xF54
    void (*m_entity_set_targetname)(unsigned int, Broc::string);  // +0xF58
    char _padF5C[0xF94 - 0xF5C];                          // +0xF5C
    int (*m_entity_get_health)(unsigned int);             // +0xF94
    void (*m_entity_set_health)(unsigned int, int);       // +0xF98
    char _padF9C[0xFA4 - 0xF9C];                          // +0xF9C
    Broc::vector* (*m_entity_get_angles)(Broc::vector*, unsigned int);  // +0xFA4
    void (*m_entity_set_angles)(unsigned int, Broc::vector);  // +0xFA8
    char _padFAC[0xFB4 - 0xFAC];                          // +0xFAC
    Broc::vector* (*m_entity_get_rotate)(Broc::vector*, unsigned int);  // +0xFB4
    void (*m_entity_set_rotate)(unsigned int, Broc::vector);  // +0xFB8
    char _padFBC[0xFDC - 0xFBC];                          // +0xFBC
    int (*m_entity_get_key)(unsigned int);                // +0xFDC
    void (*m_entity_set_key)(unsigned int, int);          // +0xFE0
    char _padFE4[0x1034 - 0xFE4];                         // +0xFE4
    int (*m_entity_get_maxhealth)(unsigned int);          // +0x1034
    void (*m_entity_set_maxhealth)(unsigned int, int);    // +0x1038
    char _pad103C[0x1054 - 0x103C];                       // +0x103C
    int (*m_entity_get_takedamage)(unsigned int);         // +0x1054
    void (*m_entity_set_takedamage)(unsigned int, int);   // +0x1058
    char _pad105C[0x1224 - 0x105C];                       // +0x105C
    Broc::string* (*m_entity_get_sentient_team)(Broc::string*, unsigned int);  // +0x1224
    void (*m_entity_set_sentient_team)(unsigned int, Broc::string);  // +0x1228
    char _pad122C[0x12CC - 0x122C];                       // +0x122C
    int (*m_entity_get_player_spectatorClient)(unsigned int);  // +0x12CC
    void (*m_entity_set_player_spectatorClient)(unsigned int, int);  // +0x12D0
    __int16 (*m_entity_get_player_ctf_has_flag)(unsigned int);  // +0x12D4
    void (*m_entity_set_player_ctf_has_flag)(unsigned int, __int16);  // +0x12D8
    Broc::vector* (*m_entity_get_player_viewangles)(Broc::vector*, unsigned int);  // +0x12DC
    void (*m_entity_set_player_viewangles)(unsigned int, Broc::vector);  // +0x12E0
    __int16 (*m_entity_get_persistent_player_rank)(unsigned int);  // +0x12E4
    void (*m_entity_set_persistent_player_rank)(unsigned int, __int16);  // +0x12E8
    __int16 (*m_entity_get_persistent_player_playerClass)(unsigned int);  // +0x12EC
    void (*m_entity_set_persistent_player_playerClass)(unsigned int, __int16);  // +0x12F0
    __int16 (*m_entity_get_persistent_player_nextPlayerClass)(unsigned int);  // +0x12F4
    void (*m_entity_set_persistent_player_nextPlayerClass)(unsigned int, __int16);  // +0x12F8
    int (*m_entity_get_persistent_player_playerState)(unsigned int);  // +0x12FC
    void (*m_entity_set_persistent_player_playerState)(unsigned int, int);  // +0x1300
    char _pad1304[0x1338 - 0x1304];                       // +0x1304
    void (*mKillThreadExec)();                            // +0x1338
};
static_assert(sizeof(BrocAPI) == 0x133C, "BrocAPI size mismatch");

extern BrocAPI gBrocAPI;  // ?gBrocAPI@@3UBrocAPI@@A @0x10F0568

// Broc runtime entry points used by ported script functions.
void wait(float seconds);
void wait_accurate(float seconds);
unsigned int thread_create(bool createHandle, const char* file, int line,
                           const char* func, void* functor);
float RandomFloatRange(float fMin, float fMax);
int RandomInt(int iMax);
int RandomIntRange(int iMin, int iMax);
void wait(float seconds);
void wait_accurate(float seconds);
void GetEntArray(const Broc::string* name, unsigned int key,
                 Broc::dyn_array<Broc::entity>* out, unsigned int flags);
Broc::entity* GetEnt(Broc::entity* result, const Broc::string* val, HashStr key,
                     unsigned int flags);
void Delete(const Broc::entity& e);
void Delete(Broc::entity* e);  // pointer convenience wrapper
Broc::entity* Spawn(Broc::entity* result, const Broc::string* classname,
                    const Broc::vector* origin, int pakInfo);
Broc::entity* Spawn(Broc::entity* result, const Broc::string* classname,
                    const Broc::vector* origin, const Broc::vector* mins,
                    const Broc::vector* maxs, int iSpawnFlags, int pakInfo);
void LinkTo(Broc::entity* e, Broc::entity* pe);
void SetModel(Broc::entity* e, const Broc::string* model, int whichPak);
void MoveTo(Broc::entity* e, const Broc::vector* vPos, float time,
            float accTime, float decTime);
int EffectEventPlay(Broc::entity* e, const Broc::string* script);
int EffectEventPlay(Broc::entity* e, const Broc::string* script,
                    HashStr notifyHash, bool stoppable);
int EffectEventPlay(const Broc::string& script, const Broc::vector& pos,
                    const Broc::vector& facing);
void SoundCrossFade(unsigned int handle1, unsigned int handle2, float time);
void GetLocalPlayerArray(Broc::dyn_array<Broc::entity>* out);
void AddEventHandler(const Broc::entity& e, HashStr label, HashStr func);
void AddEventHandler(Broc::entity* e, unsigned int label, unsigned int func);
void RemoveEventHandler(Broc::entity* e, unsigned int label, unsigned int func);
HashStr string_hash(const char* str);
HashStr* string_hash(HashStr* result, const char* str);
HashStr* string_hash(HashStr* result, const Broc::string* str);
unsigned int SoundPlay(const Broc::string& name, float volume);
unsigned int SoundPlay(const Broc::string* name, float volume);
void ReverbSetParams(const Broc::string& name, bool immediate);
void ReverbSetParams(const Broc::string* name, bool immediate);
int GetCvarInt(const char* cvar);
void iprintlnbold(const Broc::string& s);
void Code_SetTeamGame(bool teamGame);
void Code_SetShowScore(bool showScore);
void Code_DisplayScoreBoard(bool show, int time);
void Code_ClearPlayerStats();
void Code_ClearTeamScores();
void Code_SetupLevelSpecificVariables();
Broc::string* GetCvar(Broc::string* result, const char* cvar);
void SetCvar(const char* cvar, const char* value);
void SetCvar(const char* cvar, int value);
Broc::vector* entity_origin(Broc::entity* e, Broc::vector* result);
Broc::vector* vector_scale(Broc::vector* result, const Broc::vector* a, float s);
Broc::vector* vector_add(Broc::vector* result, const Broc::vector* a, const Broc::vector* b);
float vector_get(const Broc::vector* v, int i);
void vector_set(Broc::vector* v, int i, float f);
void AnglesToRight(Broc::vector* result, const Broc::vector* angles);
void AnglesToUp(Broc::vector* result, const Broc::vector* angles);
Broc::vector* VectorToAngles(Broc::vector* result, const Broc::vector* vec);
vector operator+(const vector& a, const vector& b);
vector operator-(const vector& a, const vector& b);
vector operator*(const vector& a, float s);
float Distance(const Broc::vector* a, const Broc::vector* b);
float VectorLength(const Broc::vector* v);
float VectorDot(const Broc::vector* a, const Broc::vector* b);
void VectorNormalize(Broc::vector* result, const Broc::vector* v);
int VecCloser(const Broc::vector* a, const Broc::vector* b, const Broc::vector* c);
int IsPlayer(const Broc::entity& e);             // ea: 0x93BE30
int IsAlive(const Broc::entity* e);              // ea: 0x92F320
int IsVehicle(const Broc::entity* e);            // ea: 0x92F350
int IsSentient(const Broc::entity* e);           // ea: 0x92F380
int IsTouching(const Broc::entity* e, const Broc::entity* other);
bool IsLocalHost();                              // ea: 0x92F6B0
bool IsVehicleFlipped(const Broc::entity* e);
int GetPlayerIndex(Broc::entity ent);            // ea: 0x92F4A0
int UseButtonPressed(Broc::entity e);            // ea: 0x92F4D0
float Length(const Broc::vector* v);
bint* GetTime(bint* result);                     // gBrocAPI.mGetTime
void GetPlayerArray(dyn_array<entity>* entarr);  // ea: 0x92F440
void notify(const entity* ent, HashStr label);   // ea: 0x92F500
void notify(const entity* ent, const char* label);
void iprintln(const char* msg);
void iprintln(const char* a, const char* sep, const char* b);
void SetTutorialText(int hash, int viewport);
void SetTutorialTextAllPlayers(int hash);
void SetActionHint(int hash, int viewport);
void SetSpectateState(int state, int viewport);
void SetSpectateTeamKill(int team_kill, Broc::entity* e, int viewport);
void SetSpectateMedic(int medic, int viewport);
void SetSpectateSeconds(int seconds, int viewport);
void SetTakeDamage(const Broc::entity& e, int damage);
void RotateTo(const Broc::entity& e, const Broc::vector& angles,
              float totalTime, float accTime, float decTime);
void RotateTo(Broc::entity* e, const Broc::vector* angles, float totalTime,
              float accTime, float decTime);
void GiveWeapon(Broc::entity* e, const Broc::string* pszWeaponName);
void TakeWeapon(Broc::entity* e, const Broc::string* pszWeaponName);
void SetWeaponSlotAmmo(Broc::entity* e, const Broc::string* sSlot, int iSetAmmo);
void SetWeaponSlotClipAmmo(Broc::entity* e, const Broc::string* sSlot,
                           int iSetClipAmmo);
int GetWeaponIndex(const Broc::string* team);
int GetFullClipAmmoCount(Broc::entity* e, const Broc::string* slot);
int GetMaxAmmo(Broc::entity* e, const Broc::string* slot);
void SetAiType(Broc::entity* e, const Broc::string* modelName, int whichPak);
void SetViewModel(Broc::entity* e, const Broc::string* modelName);
void Code_SetSpecialRecharge(int start, int length, int playerClass,
                             int playerIndex);
void ShellShock(Broc::entity* e, const Broc::string* shock, float fVal);
void EnableNanoForces(bool onOff);
unsigned int CreateNanoForce(const Broc::string* id,
                             const Broc::vector* direction,
                             const Broc::vector* param2);
void RadiusDamage(const Broc::vector* origin, float range, float max_damage,
                  float min_damage, int damageType);
void RadiusDamageFromEnt(Broc::entity* which, const Broc::vector* origin,
                         float range, float max_damage, float min_damage,
                         int damageType);
void SetMaxVehicles(int vehicles);
void FireTurret(Broc::entity* e);
Broc::vector* GetOrigin(Broc::vector* result, Broc::entity* e);
void Earthquake(float scale, float duration, const Broc::vector* source,
                float radius, int player_index);
void Rumble(const Broc::string* lowFreqNotes, float lowFreqDuraton,
            const Broc::string& highFreqNotes, float highFreqDuration,
            int player_index);
void SwitchToWeapon(Broc::entity* e, const Broc::string* weapon);
void TakeAllWeapons(Broc::entity* e);
void DoDamage(const Broc::entity& e, float damage, const Broc::vector& vecIn, hitLocation_t hitLoc);
float RandomFloat(float fMax);
void MusicStop();
void SoundStop(unsigned int handle);
void ObjectiveAdd(int iObjective, const Broc::string* state,
                  const Broc::string* pszString, const Broc::vector vPos,
                  const char* display);
void ObjectiveAdd(int iObjective, const Broc::string* state,
                  const Broc::string* pszString, const Broc::vector vPos,
                  float height, int clientIndex);
void ObjectiveDelete(int iObjective, int clientIndex);
void ObjectiveRing(int iObjective, int clientIndex);
void Launch(Broc::entity* e, const Broc::vector* velocity);
void Code_DebugRenderText(const char* text, int x, int y);
void Code_FinishDamage(Broc::entity player, Broc::entity inflictor,
                       Broc::entity attacker, const Broc::vector* dir,
                       const Broc::vector* position, int damage, int mod,
                       int weapon, int hitLoc);
int Code_GetPlayerTotalScore(Broc::entity player);
int Code_GetPlayerStat(Broc::entity player, int index);
void Code_IncPlayerStat(Broc::entity player, unsigned int index, __int16 value);
int Code_GetTeamScore(const Broc::string& team);
void Code_IncTeamScore(const Broc::string& team, int ammount);
bool Code_PositionWouldTelefrag(const Broc::vector* position);
void Code_ChangePlayerTeam(Broc::entity player, const Broc::string* team,
                           bool autoBalance);
bool Code_IsLocalPlayer(Broc::entity player);
const char* Code_GetPlayerName(Broc::entity player);
void Code_DebugOut(const char* strOut);
void Code_NextRound(bool allowChange);
void Code_RoundOver(int condition, const Broc::string* winner);
bool Code_GetTeamGame();
bool Code_IsHost();
bool Code_IsRankedGame();
void Code_ScreenFadeToBlack(unsigned int time, int viewport);
void Code_ScreenFadeUp(unsigned int time, int viewport);
void Code_QuitGame();
void Code_EnterGame();
void Code_SendGameStateCTF(Broc::entity player, const Broc::vector* allied_flag,
                           const Broc::vector* allied_angles,
                           Broc::entity allied_flag_holder,
                           const Broc::vector* axis_flag,
                           const Broc::vector* axis_angles,
                           Broc::entity axis_flag_holder);
void Code_SetCompassVisibilty(int teamid, bool visible);
void Code_PickupItem(int netID, Broc::entity player);
void Code_AreaCaptured(int netID, int team, int hostOnly);
void Code_HostDropItem(int itemType, int netID, const Broc::vector* position,
                       const Broc::vector* angles, const Broc::vector* velocity);
void Code_SendGameStateSCF(Broc::entity player, int defendingTeam,
                           const Broc::vector* flag,
                           const Broc::vector* flagAngles,
                           Broc::entity flag_holder);
void Code_SendGameStateHQ(Broc::entity player, int stage,
                          const Broc::vector* vA, const Broc::vector* vB,
                          int triggerIndex, bool alliesDefending,
                          bool pointAIsHQ);
void Code_SendGameStateDOM(Broc::entity player, int flag0, int flag1,
                           int flag2, int flag3, int flag4);
void SoundFadeOut(unsigned int handle, float time);
void Code_DebugRenderBox(const Broc::vector* min, const Broc::vector* max,
                         const Broc::vector* color, float alpha);
void Code_DebugRenderSphere(const Broc::vector* point, float radius,
                            const Broc::vector* color, float alpha);
void Code_DebugRenderEntityBBox(Broc::entity e, const Broc::vector* color,
                                float alpha);
void Code_RespawnVehicle(Broc::entity* e);
void Code_BroadcastVehicleRespawn(Broc::entity vehicle);
void Code_GetPlayerInSeat(Broc::entity* result, Broc::entity vehicle, int seat);
void Code_PlayerSpawn(Broc::entity player, const Broc::vector* origin,
                      const Broc::vector* angles, bool stopPhysics);
void Code_PlayerRespawn(Broc::entity player, const Broc::vector* origin,
                        const Broc::vector* angles, const Broc::string* team);
void Code_RequestRespawn(int playerID);
void Code_SetPlayerAlive(Broc::entity player, int health);
void Code_SetRespawnMaxTime(Broc::entity player, int time);
void Code_GetOutOfVehicle(Broc::entity player);
bool Code_IsInVehicle(Broc::entity player);
void Code_Obituary(Broc::entity target, Broc::entity attacker,
                   const Broc::string* weapon, int mod, bool teamGame);
void Code_SendGameState(Broc::entity player, int currentTime, int timeLimit,
                        int scoreLimit, int roundLimit, bool friendlyFire,
                        bool lastManStanding, bool teamBalance, int respawnTime,
                        int alliesScore, int axisScore, bool roundStarted,
                        int roundOver, int roundCount);
void Code_SendInitialGameState(Broc::entity player);
void Code_SendVehicleStates(Broc::entity player);
bool Code_NextRoundMapChanges();
void Code_SettleMapVote();
void Code_SettleGameModeVote();
void Code_SetShowTime(float gameTime);
void Code_ClearSpottingFromOccupants(Broc::entity ent);
Broc::entity Code_GetSpotterEntity(Broc::entity ent);
int Code_IsMenuOpen(const Broc::string* menu, int viewport);
void Code_ForceControllerErrorMessageDown();
void Code_GetWeaponName(int weaponIndex, Broc::string& weapon);
int OpenMenu(const Broc::string* str, int viewport);
void CloseMenu(const Broc::string& str, int viewport);
void CloseAllMenus(int viewport);
int DialogPlay(const Broc::entity& e, const Broc::string& script);
int DialogPlay(Broc::entity e, const Broc::string* script);
void EffectEventStopEmitting(unsigned int effectId);
void GetWeaponSlotWeapon(Broc::entity e, const Broc::string* slot, Broc::string* result);
int GetWeaponSlotAmmo(Broc::entity e, const Broc::string* slot);
int GetWeaponSlotClipAmmo(Broc::entity e, const Broc::string* slot);
Broc::hudelem* NewHudElem(Broc::hudelem* result, int panelType);
void Destroy(Broc::hudelem* hud);
void SetShader(Broc::hudelem* hud, const Broc::string* shader, int w, int h);
void ScaleOverTime(Broc::hudelem* hud, float time, int w, int h);
template <typename... Args> void println(const char* fmt, const Args&... args);
template <typename T> int size(const Broc::dyn_array<T>& ar);
template <typename T> void push(Broc::dyn_array<T>& ar, const T& elt);
template <typename T> void push(Broc::dyn_array<T>& ar, const T* elt);
}

bool IS_NAN(float x);  // global (defined in Broc.cpp)
void RegisterHashString(int hash, const char* text); // ea: 0x929060
const char* ToStr(int hash); // ea: 0x929570

// Forward (defined in game/actor_types.h)
struct sentient_s;

// ============================================================================
// PathNodes namespace — AI path node handles (global scope, verified against IDA)
// ============================================================================
namespace PathNodes {
struct PathNode;
struct PathLink;
struct PathLinkInfo;
struct NodeVariableSaver;
struct PathNodeTree;
enum ENodeType : int32_t {
    NODE_BADNODE = 0x0,
    NODE_PATHNODE = 0x1,
    NODE_COVER_STAND = 0x2,
    NODE_COVER_CROUCH = 0x3,
    NODE_COVER_PRONE = 0x4,
    NODE_COVER_RIGHT = 0x5,
    NODE_COVER_LEFT = 0x6,
    NODE_COVER_WIDE_RIGHT = 0x7,
    NODE_COVER_WIDE_LEFT = 0x8,
    NODE_CONCEALMENT_STAND = 0x9,
    NODE_CONCEALMENT_CROUCH = 0xA,
    NODE_CONCEALMENT_PRONE = 0xB,
    NODE_STACK = 0xC,
    NODE_REACQUIRE = 0xD,
    NODE_BALCONY = 0xE,
    NODE_SCRIPTED = 0xF,
    NODE_NEGOTIATION_BEGIN = 0x10,
    NODE_NEGOTIATION_END = 0x11,
    NODE_AMBUSH = 0x12,
    NODE_NUMTYPES = 0x13,
};

class NodeHandle {
public:
    uint16_t mValue;  // +0x00

    NodeHandle();                            // ??0NodeHandle@PathNodes@@QAE@XZ (g.o 0x4A75A0)
    NodeHandle(int value);                   // ??0NodeHandle@PathNodes@@QAE@H@Z (g.o 0x4A7630)
    unsigned short GetZone() const;          // ?GetZone@NodeHandle@PathNodes@@QBEGXZ (game2.o 0x517180)
    unsigned short GetValue() const;         // ?GetValue@NodeHandle@PathNodes@@QBEGXZ (game2.o 0x517190)
    unsigned short GetZoneIndex() const;     // ?GetZoneIndex@NodeHandle@PathNodes@@QBEGXZ (g.o 0x4A7650)
    static NodeHandle NullHandle();          // ?NullHandle@NodeHandle@PathNodes@@SA?AV12@XZ (g.o 0x4A7660)
    bool IsAssigned() const;                 // ?IsAssigned@NodeHandle@PathNodes@@QBE_NXZ (g.o 0x4A7670)
    bool operator==(const NodeHandle& rhs) const;  // ??8NodeHandle@PathNodes@@QBE_NABV01@@Z (g.o 0x4A7690)
    operator bool() const;                   // ??BNodeHandle@PathNodes@@QBE_NXZ (g.o 0x4A76B0)
    const PathNode* operator*() const;  // ??DNodeHandle@PathNodes@@QBEPBUPathNode@1@XZ (mp_actors.o)
    PathNode* operator*();             // ??DNodeHandle@PathNodes@@QAEPAUPathNode@1@XZ (mp_actors.o)
    const PathNode* operator->() const;  // ??CNodeHandle@PathNodes@@QBEPBUPathNode@1@XZ (mp_actors.o)
    PathNode* operator->();             // ??CNodeHandle@PathNodes@@QAEPAUPathNode@1@XZ (mp_actors.o)
};

// ============================================================================
// PathNodeDynamic - per-node runtime state (0x24 bytes) - IDA verified
// ============================================================================
struct PathNodeDynamic {
    sentient_s* mOwner;        // +0x00
    int   mFreeTime;           // +0x04
    int   mValidTime[3];       // +0x08
    int   mSafeTime[3];        // +0x14
    int16_t mLinkCount;        // +0x20
    char  mOverlapCount;       // +0x22
    char  mFlags;              // +0x23
};
static_assert(sizeof(PathNodeDynamic) == 0x24,
              "PathNodeDynamic size mismatch");

// ============================================================================
// PathNodeConstant - per-node static data (0x48 bytes) - IDA verified
// ============================================================================
struct PathNodeConstant {
    ENodeType mType;                       // +0x00
    uint16_t mSpawnFlags;                  // +0x04
    Broc::string mTargetName;              // +0x08
    Broc::string mScriptNoteWorthy;        // +0x0C
    Broc::string mTarget;                  // +0x10
    Broc::string mOnGoalCallback;          // +0x14
    Broc::string mReserveName;             // +0x18
    Broc::string mAnimScript;              // +0x1C
    unsigned int (__cdecl* mAnimScriptFunc)(void*);  // +0x20
    float mOrigin[3];                      // +0x24
    float mAngle;                          // +0x30
    float mRadius;                         // +0x34
    NodeHandle mOverlapNode[2];            // +0x38
    int16_t mChainId;                      // +0x3C
    int16_t mChainDepth;                   // +0x3E
    NodeHandle mChainParent;               // +0x40
    int16_t mTotalLinkCount;               // +0x42
    PathLink* mLinks;                      // +0x44
};
static_assert(sizeof(PathNodeConstant) == 0x48,
              "PathNodeConstant size mismatch");

// ============================================================================
// PathNodeTransient - per-search state (0x14 bytes) - IDA verified
// ============================================================================
struct PathNodeTransient {
    int   mSearchFrame;        // +0x00
    PathNode* mParent;         // +0x04
    float mCost;               // +0x08
    float mHeuristic;          // +0x0C
    int   mPriorityQueueIndex; // +0x10
};
static_assert(sizeof(PathNodeTransient) == 0x14,
              "PathNodeTransient size mismatch");

// ============================================================================
// PathLink - node link (0x0C bytes) - IDA verified
// ============================================================================
struct PathLink {
    NodeHandle mNodeHandle;     // +0x00
    uint8_t mDisconnectCount;   // +0x02
    uint8_t mNegotiationLink;   // +0x03
    uint8_t mBadPlaceCount[4];  // +0x04
    float  mDist;               // +0x08
};
static_assert(sizeof(PathLink) == 0x0C, "PathLink size mismatch");

// ============================================================================
// PathSort - sorted node search result (8 bytes) - IDA verified
// ============================================================================
struct PathSort {
    PathNode* pNode;   // +0x00
    float fMetric;     // +0x04
};
static_assert(sizeof(PathSort) == 8, "PathSort size mismatch");

// ============================================================================
// PathNodeTree - spatial tree node (0x10 bytes) - IDA verified
// ============================================================================
struct PathNodeTree {
    int   axis;         // +0x00
    float dist;         // +0x04
    union {
        struct {
            PathNodeTree* left;   // +0x08
            PathNodeTree* right;  // +0x0C
        } children;
        struct {
            int count;           // +0x08
            PathNode** nodes;    // +0x0C
        } leaf;
    } u;                // +0x08
};
static_assert(sizeof(PathNodeTree) == 0x10, "PathNodeTree size mismatch");

// ============================================================================
// TOC1 - level path table of contents (0x34 bytes) - IDA verified
// ============================================================================
struct TOC1 {
    int mVersion;          // +0x00
    int mNodeCount;        // +0x04
    int mChainNodeCount;   // +0x08
    int mLinkCount;        // +0x0C
    int mVariableCount;    // +0x10
    PathNode* mNodes;      // +0x14
    NodeHandle* mChainNodes;  // +0x18
    PathLink* mLinks;      // +0x1C
    NodeVariableSaver* mVariables;  // +0x20
    PathLinkInfo* mLinkPool;  // +0x24
    PathNodeTree* mTree;   // +0x28
    char* mStrings;        // +0x2C
    PathNode** mTreeNodes; // +0x30

    TOC1();  // ??0TOC1@PathNodes@@QAE@XZ (mp_actors.o)
};
static_assert(sizeof(TOC1) == 0x34, "TOC1 size mismatch");

// ============================================================================
// TOC2 - level vis table of contents (0x0C bytes) - IDA verified
// ============================================================================
struct TOC2 {
    int mVersion;         // +0x00
    int mVisSize;         // +0x04
    uint8_t* mVisData;    // +0x08

    TOC2();  // ??0TOC2@PathNodes@@QAE@XZ (mp_actors.o)
};
static_assert(sizeof(TOC2) == 0x0C, "TOC2 size mismatch");

// ============================================================================
// NodeVariableSaver / PathLinkInfo - checkpoint save structures
// ============================================================================
struct NodeVariableSaver {
    NodeHandle mNode;   // +0x00
    char mKey[64];      // +0x02
    char mValue[128];   // +0x42
};
static_assert(sizeof(NodeVariableSaver) == 0xC2,
              "NodeVariableSaver size mismatch");

struct PathLinkInfo {
    NodeHandle from;    // +0x00
    NodeHandle to;      // +0x02
    uint16_t prev;      // +0x04
    uint16_t next;      // +0x06
};
static_assert(sizeof(PathLinkInfo) == 8, "PathLinkInfo size mismatch");

// ============================================================================
// PathNode - AI path node (0x84 bytes) - verified against IDA
// ============================================================================
struct PathNode {
    NodeHandle mHandle;         // +0x00
    PathNodeDynamic mDynamic;   // +0x04
    PathNodeConstant mConstant; // +0x28
    PathNodeTransient mTransient;  // +0x70
};
} // namespace PathNodes
COD3_STATIC_ASSERT_32BIT(sizeof(PathNodes::NodeHandle) == 2, "PathNodes::NodeHandle size mismatch");
static_assert(sizeof(PathNodes::PathNode) == 0x84, "PathNode size mismatch");

// ============================================================================
// InplaceString — in-place char* (4 bytes)
// ============================================================================
class InplaceString {
public:
    char* mStr;  // +0x00

    bool empty() const;  // ?empty@InplaceString@@QBE_NXZ (streamer.o 0x6631D0)
    const char* c_str() const;            // ?c_str@InplaceString@@QBEPBDXZ
    operator const char*() const;         // ??BInplaceString@@QBEPBDXZ
    bool operator<(const char* rhs) const;    // ??MInplaceString@@QBE_NPBD@Z
    bool operator==(const char* rhs) const;   // ??8InplaceString@@QBE_NPBD@Z
    bool operator!=(const char* rhs) const;   // ??9InplaceString@@QBE_NPBD@Z
};
COD3_STATIC_ASSERT_32BIT(sizeof(InplaceString) == 4, "InplaceString size mismatch");

#undef COD3_STATIC_ASSERT_32BIT

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

namespace Broc {
class string;
}

// ============================================================================
// HashString — global hashed string (4 bytes) — verified against IDA
// ============================================================================
class HashString {
public:
    unsigned int mHash;  // +0x00
    HashString() : mHash(0) {}
    HashString(Broc::string& str);  // ea: 0x004C1450
    static unsigned int CalcHash(const char* str);  // ea: 0x004C1540
};
COD3_STATIC_ASSERT_32BIT(sizeof(HashString) == 4, "HashString size mismatch");

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
enum EHitLocation {
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
struct dyn_array {
    T*           mElements;   // +0x00
    unsigned int mCapacity;   // +0x04
    unsigned int mSize;       // +0x08

    dyn_array() : mElements(NULL), mCapacity(0), mSize(0) {}  // ea: 0x93305B
    ~dyn_array();  // ea: 0x933129

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

// ============================================================================
// Broc::vector — 3D vector (12 bytes)
// ============================================================================
struct vector {
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08

    vector() : x(0.0f), y(0.0f), z(0.0f) {}
    vector(float ix, float iy, float iz) : x(ix), y(iy), z(iz) {}
    float operator[](int i) const { return (&x)[i]; }
};
COD3_STATIC_ASSERT_32BIT(sizeof(vector) == 12, "Broc::vector size mismatch");

// ============================================================================
// Broc::entity — script entity handle (4 bytes)
// ============================================================================
class entity {
public:
    unsigned int ___u0;  // +0x00

    void UndefineEEField(unsigned int key);
    unsigned int GetHandle() const { return ___u0; }  // ea: 0x92F170
    bool IsDefined() const { return ___u0 != 0; }     // ea: 0x92F170
};
COD3_STATIC_ASSERT_32BIT(sizeof(entity) == 4, "Broc::entity size mismatch");

inline bool operator==(const entity& lhs, const entity& rhs) {
    return lhs.___u0 == rhs.___u0;
}
inline bool operator!=(const entity& lhs, const entity& rhs) {
    return lhs.___u0 != rhs.___u0;
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
struct pathnode {
    unsigned int ___u0;
};
COD3_STATIC_ASSERT_32BIT(sizeof(pathnode) == 4, "Broc::pathnode size mismatch");

struct vehiclenode {
    unsigned int ___u0;
};
COD3_STATIC_ASSERT_32BIT(sizeof(vehiclenode) == 4, "Broc::vehiclenode size mismatch");

// ============================================================================
// Broc::hudelem - HUD element handle (4 bytes).
// ============================================================================
struct hudelem {
    unsigned int ___u0;

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
void waittill(entity ent, HashStr signal, entity* output);
void waittill_timeout(entity ent, HashStr signal, float timeout);
void waittillmatch(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittillor(entity ent, HashStr s1, HashStr s2, HashStr s3, HashStr s4);
void waittill(entity ent, const char* signal);
void waittill_loaded(unsigned int pakInfo);
void waittill_unloaded(unsigned int pakInfo);
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
void GetNodeArray(const string& name1, const string& name2, void* result);
void GetAllNodes(void* result);
void GetVehicleNodeArray(const string& name1, const string& name2, void* result);
void GetAllVehicleNodes(void* result);

// ============================================================================
// Boxed types used by Broc::string constructors
// ============================================================================
struct bint {
    int mVal;

    bint() : mVal(0) {}
    bint(int v) : mVal(v) {}
    bint& operator=(int v) { mVal = v; return *this; }
    operator int() const { AssertDefined(); return mVal; }
    int operator++(int) { AssertDefined(); return ++mVal; }
    int operator++() { AssertDefined(); return ++mVal; }
    int operator--(int) { AssertDefined(); return --mVal; }
    bint& operator+=(int v) { AssertDefined(); mVal += v; return *this; }
    bint& operator-=(int v) { AssertDefined(); mVal -= v; return *this; }
    void AssertDefined() const {}  // ea: 0x934790
};
COD3_STATIC_ASSERT_32BIT(sizeof(bint) == 4, "bint size mismatch");

struct bfloat {
    float mVal;

    bfloat() : mVal(0.0f) {}
    bfloat(float v) : mVal(v) {}
    bfloat& operator=(float v) { mVal = v; return *this; }
    operator float() const { return mVal; }
    bfloat& operator+=(float v) { mVal += v; return *this; }
    bfloat& operator-=(float v) { mVal -= v; return *this; }
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
    bbool& operator=(bool v) { mVal = v; return *this; }
    operator bool() const { return mVal; }
};
COD3_STATIC_ASSERT_32BIT(sizeof(bbool) == 1, "bbool size mismatch");

// Boxed-type comparison operators (mp_util_wad.o inline COMDATs).
bool operator<(bint lhs, bint rhs);
bool operator>(bint lhs, bint rhs);
bool operator==(bint lhs, bint rhs);
bool operator!=(bint lhs, bint rhs);
bool operator<(bfloat lhs, bfloat rhs);
bool operator>(bfloat lhs, bfloat rhs);
bool operator==(bfloat lhs, bfloat rhs);
bool operator!=(bfloat lhs, bfloat rhs);
bool operator<(bfloat lhs, float rhs);
bool operator>(bfloat lhs, float rhs);
bint operator+(bint lhs, bint rhs);
bint operator-(bint lhs, bint rhs);
bint operator*(bint lhs, bint rhs);
bint operator*(int lhs, bint rhs);
bint operator*(bint lhs, int rhs);
bfloat operator*(bfloat lhs, float rhs);
bfloat operator*(bfloat lhs, bfloat rhs);
bool operator<(bint lhs, int rhs);
bool operator>(bint lhs, int rhs);

} // namespace Broc

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
    void (*mThreadExecute)(void*);                            // +0x88
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
    char _pad08[0x94 - 0x08];                             // +0x008
    unsigned int (*mGetEnt)(const Broc::string*, int, unsigned int*, int, int);  // +0x094
    unsigned int (*mGetEntByNum)(int);                    // +0x098
    char _pad9C[0x184 - 0x9C];                            // +0x09C
    float (*mVecDistance)(const Broc::vector*, const Broc::vector*);  // +0x184
    char _pad188[0x1A0 - 0x188];                          // +0x188
    void (*mVecToAngles)(Broc::vector*, const Broc::vector*);  // +0x1A0
    char _pad1A4[0x6D8 - 0x1A4];                          // +0x1A4
    void (*mDelete)(unsigned int);                        // +0x6D8
    char _pad6DC[0xBDC - 0x6DC];                          // +0x6DC
    bool (*mAssert)(const char* file, int line, const char* msg);  // +0xBDC
    bool (*mWarning)(const char* file, int line, const char* msg);  // +0xBE0
    bool (*mError)(const char* file, int line, const char* msg);  // +0xBE4
    struct BrocExports mBrocExports;                      // +0xBE8
    char _padBrocExports[0xF1C - (0xBE8 + 0x1C8)];        // +0xDB0
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
    char _pad1304[0x133C - 0x1304];                       // +0x1304 (total 0x133C = 4924)
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
void Delete(Broc::entity* e);
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
int EffectEventPlay(const Broc::string* script, const Broc::vector* pos,
                    const Broc::vector* facing);
void SoundCrossFade(unsigned int handle1, unsigned int handle2, float time);
void GetLocalPlayerArray(Broc::dyn_array<Broc::entity>* out);
void AnglesToForward(Broc::vector* result, const Broc::vector* angles);
void AddEventHandler(Broc::entity* e, unsigned int label, unsigned int func);
void RemoveEventHandler(Broc::entity* e, unsigned int label, unsigned int func);
HashStr string_hash(const char* str);
HashStr* string_hash(HashStr* result, const char* str);
HashStr* string_hash(HashStr* result, const Broc::string* str);
unsigned int SoundPlay(const Broc::string& name, float volume);
void ReverbSetParams(const Broc::string& name, bool immediate);
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
float DistanceSquared(const Broc::vector* a, const Broc::vector* b);
float VectorLength(const Broc::vector* v);
float VectorDot(const Broc::vector* a, const Broc::vector* b);
void VectorNormalize(Broc::vector* result, const Broc::vector* v);
int VecCloser(const Broc::vector* a, const Broc::vector* b, const Broc::vector* c);
bool IsPlayer(const Broc::entity* e);            // ea: 0x92F2F0
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
void SetTakeDamage(Broc::entity* e, int damage);
void RotateTo(Broc::entity* e, const Broc::vector* angles, float time);
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
void DoDamage(Broc::entity* e, float damage, const Broc::vector* vecIn, int hitLoc);
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
void Code_IncPlayerStat(Broc::entity player, int index, int value);
int Code_GetTeamScore(const Broc::string* team);
void Code_IncTeamScore(const Broc::string* team, int ammount);
bool Code_PositionWouldTelefrag(const Broc::vector* position);
void Code_ChangePlayerTeam(Broc::entity player, const Broc::string* team,
                           bool autoBalance);
bool Code_IsLocalPlayer(Broc::entity player);
char* Code_GetPlayerName(Broc::entity player);
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
                      const Broc::vector* angles, int stopPhysics);
void Code_PlayerRespawn(Broc::entity player, const Broc::vector* origin,
                        const Broc::vector* angles, const Broc::string* team);
void Code_RequestRespawn(int playerID);
void Code_SetPlayerAlive(Broc::entity player, int health);
void Code_SetRespawnMaxTime(Broc::entity player, int time);
void Code_GetOutOfVehicle(Broc::entity player);
bool Code_IsInVehicle(Broc::entity player);
void Code_Obituary(Broc::entity target, Broc::entity attacker,
                   const Broc::string* weapon, int mod, int teamGame);
void Code_SendGameState(Broc::entity player, float currentTime, int timeLimit,
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
void Code_GetSpotterEntity(Broc::entity* result, Broc::entity ent);
int Code_IsMenuOpen(const Broc::string* menu, int viewport);
void Code_ForceControllerErrorMessageDown();
void Code_GetWeaponName(int weaponIndex, Broc::string* weapon);
void OpenMenu(const Broc::string* str, int viewport);
void CloseMenu(const Broc::string* str, int viewport);
void CloseAllMenus(int viewport);
int DialogPlay(Broc::entity e, const Broc::string* script);
int EffectEventStopEmitting(int effectId);
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

// ============================================================================
// PathNodes namespace — AI path node handles (global scope, verified against IDA)
// ============================================================================
namespace PathNodes {
struct PathNode;  // forward (defined below)

class NodeHandle {
public:
    uint16_t mValue;  // +0x00

    const PathNode* operator*() const;  // ??DNodeHandle@PathNodes@@QBEPBUPathNode@1@XZ (mp_actors.o)
};

// ============================================================================
// PathNode - AI path node (0x84 bytes) - verified against IDA
// ============================================================================
struct PathNode {
    NodeHandle mHandle;    // +0x00
    uint8_t    _pad4[0x24];   // +0x04 (PathNodeDynamic)
    struct Constant {
        uint8_t  _pad0[0x24];   // +0x00 (mType .. mAnimScriptFunc)
        float    mOrigin[3];    // +0x24
        uint8_t  _pad30[0x18];  // +0x30 (mAngle .. mLinks)
    } mConstant;               // +0x28 (mOrigin at +0x4C)
    uint8_t    _pad70[0x14];   // +0x70 (PathNodeTransient)
};
} // namespace PathNodes
COD3_STATIC_ASSERT_32BIT(sizeof(PathNodes::NodeHandle) == 2, "PathNodes::NodeHandle size mismatch");
static_assert(sizeof(PathNodes::PathNode) == 0x84, "PathNode size mismatch");

// ============================================================================
// InplaceString — in-place char* (4 bytes)
// ============================================================================
struct InplaceString {
    char* mStr;  // +0x00

    bool empty() const;  // ?empty@InplaceString@@QBE_NXZ (streamer.o 0x6631D0)
};
COD3_STATIC_ASSERT_32BIT(sizeof(InplaceString) == 4, "InplaceString size mismatch");

#undef COD3_STATIC_ASSERT_32BIT

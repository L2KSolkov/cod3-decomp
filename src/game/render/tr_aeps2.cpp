// ============================================================================
// tr_aeps2.cpp - render.o aps error check + model manager dtors
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "aeps/apsError.h"
#include "core/ae_array.h"
#include "core/fourcc.h"

#include <new>
#include <string.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1, CD = 2, JRS = 3, JSV = 10 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
bool Warning(const char* fmtstring, ...);
bool Error(const char* fmtstring, ...);
}

enum TPakId { kPakTypeLevel = 0, kPakTypeNone = -1 };
#define PAK_ID_INVALID ((TPakId)-1)

extern void ValidatePakId(TPakId pakId);  // streamer.o (pakmanager.cpp)
extern void GetPakPrerequisites(
    TPakId pakId,
    ae_sized_array<TPakId, 32>* prereqs);  // ?GetPakPrerequisites@@YAXW4TPakId@@PAV?$ae_sized_array@W4TPakId@@$0CA@@@@Z
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);

// InplaceString (ae in-place char*; class tag per VInplaceString mangling)
class InplaceString {
public:
    char* mStr;  // +0x00
};

template <typename T>
struct InplaceVector {
    unsigned int mSize;  // +0x00
    T* mList;            // +0x04

    T& operator[](unsigned int i)
    {
        if (i >= mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
            if (i >= mSize)
                i = 0;
        }
        return mList[i];
    }
};

template <typename T>
class IVPointer {
public:
    T* mValue;        // +0x00
    unsigned int mPakId;  // +0x04
};

template <typename T>
struct AeType {};

template <typename K, typename V>
class InplaceTree {
public:
    struct Element {
        InplaceString mKey;  // +0x00
        V mVal;              // +0x04
    };
    unsigned int mSize;      // +0x00
    Element* m_array;        // +0x04

    bool IsUsed(unsigned int index) const
    {
        return index < mSize && m_array[index].mKey.mStr != nullptr;
    }

    template <typename KeyT>
    V* Find(const KeyT& key) const
    {
        unsigned int v3 = 0;
        if (mSize == 0)
            return nullptr;
        while (1)
        {
            if (!IsUsed(v3))
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\inplace/InplaceTree.h";
                AeAssert::gCurrentLine = 101;
                AeAssert::gCurrentExpr = "IsUsed(index)";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("index must be used"))
                    __debugbreak();
            }
            if (_stricmp(m_array[v3].mKey.mStr, key) == 0)
                break;
            if (_stricmp(m_array[v3].mKey.mStr, key) >= 0)
                v3 = 2 * v3 + 1;
            else
                v3 = 2 * v3 + 2;
            if (v3 >= mSize || !IsUsed(v3) || v3 >= mSize)
                return nullptr;
        }
        return &m_array[v3].mVal;
    }
};

// InplaceAssetBank<T, Tree> (ae/inplace/InplaceAssetBank.h)
int FourCC::GetVal() const
{
    return (int)mVal;
}

bool operator==(FourCC lhs, FourCC rhs)
{
    return lhs.mVal == rhs.mVal;
}
struct PtrFixupTable {
    void Fixup(const void* pBase);  // ?Fixup@PtrFixupTable@@QAEXPBX@Z (filesystem/inplace.cpp)
};

template <typename T, typename Tree>
class InplaceAssetBank {
public:
    typedef T ElementType;
    FourCC mFileId;                       // +0x00
    float mVersion;                       // +0x04
    Tree mTree;                           // +0x08
    InplaceVector<T const*> mPtrs;        // +0x10
    PtrFixupTable* mPtrFixupTable;        // +0x18

    void Fixup();                         // ?Fixup@?$InplaceAssetBank@...@@QAEXXZ
    unsigned int Size() const;            // ?Size@...@@QBEIXZ
    T* operator[](unsigned int i);        // ??A@...@@QAEPAV...@@I@Z
    template <typename KeyT>
    bool FindIndex(const KeyT& key, unsigned int* res) const;  // ??$FindIndex@...@@QBE_NABQ...DPAI@Z
};

template <typename T, typename Tree>
void InplaceAssetBank<T, Tree>::Fixup()
{
    if ((unsigned int)mPtrFixupTable >= 0x10000000)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 122;
        AeAssert::gCurrentExpr = "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    PtrFixupTable* v2 = (PtrFixupTable*)((char*)this
                                         + (unsigned int)mPtrFixupTable);
    mPtrFixupTable = v2;
    v2->Fixup(this);
}

template <typename T, typename Tree>
unsigned int InplaceAssetBank<T, Tree>::Size() const
{
    return mPtrs.mSize;
}

template <typename T, typename Tree>
T* InplaceAssetBank<T, Tree>::operator[](unsigned int i)
{
    if (i >= mPtrs.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 199;
        AeAssert::gCurrentExpr = "i<mPtrs.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bounds check"))
            __debugbreak();
    }
    return (T*)mPtrs.mList[i];
}

template <typename T, typename Tree>
template <typename KeyT>
bool InplaceAssetBank<T, Tree>::FindIndex(const KeyT& key,
                                          unsigned int* res) const
{
    unsigned int* v3 = mTree.Find(key);
    if (v3 == nullptr)
        return false;
    *res = *v3;
    return true;
}

class XModel;
class XModelParts;
class XModelPartsBank
    : public InplaceAssetBank<XModelParts,
                              InplaceTree<InplaceString, unsigned int>> {
};
class XModelBank
    : public InplaceAssetBank<XModel,
                              InplaceTree<InplaceString, unsigned int>> {
};

// ============================================================================
// apsCheckErrors - ea: 0x006C71C0
// ============================================================================
int apsCheckErrors()
{
    apsError* inst = apsSingleton<apsError>::InstancePtr();
    if (inst == nullptr
        && _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsUtil.h", 109,
                     "sInstancePtr", "singleton not initialised"))
    {
        __debugbreak();
    }

    int mNumErrors = inst->GetNumErrors();
    int i = 0;
    int nErrors = mNumErrors;
    const char* mMessage = nullptr;
    if (mNumErrors > 0)
    {
        while (1)
        {
            apsError* v3 = apsSingleton<apsError>::InstancePtr();
            if (v3 == nullptr
                && _tlAssert("c:\\cod\\code\\tl\\aeps\\include\\apsUtil.h", 109,
                             "sInstancePtr", "singleton not initialised"))
            {
                __debugbreak();
            }
            apsError::ErrorMessage& err = v3->GetError(i);
            mMessage = err.GetMessage();
            int v6 = (int)err.GetType() - 2;
            AeAssert::gCurrentExpr = nullptr;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\aeps_support.cpp";
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            if (v6 != 0)
                break;
            AeAssert::gCurrentLine = 318;
            if (AeAssert::Error(mMessage))
                __debugbreak();
cont:
            ++i;
            if (i >= nErrors)
            {
                mNumErrors = nErrors;
                goto done;
            }
        }
        AeAssert::gCurrentLine = 314;
        if (!AeAssert::IsIgnored() && AeAssert::Warning(mMessage))
            __debugbreak();
        goto cont;
    }
done:
    apsError* finalInst = apsSingleton<apsError>::InstancePtr();
    if (finalInst != nullptr)
        finalInst->Clear();
    else
        apsSingleton<apsError>::InstancePtr()->Clear();
    return mNumErrors;
}

// ============================================================================
// InplaceAssetBankSet<T> + XModelManager / XModelPartsManager (render.o)
// ============================================================================
class AssetBankSet {
public:
    AssetBankSet();             // ??0AssetBankSet@@QAE@XZ (streamer.o)
    virtual ~AssetBankSet();    // ??1AssetBankSet@@UAE@XZ (streamer.o)
};

template <typename T>
class InplaceAssetBankSet : public AssetBankSet {
public:
    InplaceAssetBankSet();                 // ??0?$InplaceAssetBankSet@VXModelBank@@@@QAE@XZ
    virtual ~InplaceAssetBankSet();        // ??1?$InplaceAssetBankSet@VXModelBank@@@@UAE@XZ
    void AddBank(TPakId pakId, T* bank);   // ?AddBank@?$InplaceAssetBankSet@VXModelBank@@@@QAEXW4TPakId@@PAVXModelBank@@@Z
    template <typename KeyT, typename ValueT>
    ValueT Find(TPakId pakId, KeyT key, AeType<ValueT>,
                TPakId* foundPakId) const;  // ??$Find@PBDV?$IVPointer@VXModel@@@@@?$InplaceAssetBankSet@VXModelBank@@@@QBE?AV?$IVPointer@VXModel@@@@W4TPakId@@PBDV?$AeType@V?$IVPointer@VXModel@@@@@@PAW42@@Z
protected:
    virtual void OnBankUnloaded(T& bank);  // ?OnBankUnloaded@?$InplaceAssetBankSet@VXModelBank@@@@MAEXAAVXModelBank@@@Z
    virtual void UnloadBank(TPakId pakId); // ?UnloadBank@?$InplaceAssetBankSet@VXModelBank@@@@MAEXW4TPakId@@@Z
    T* mBankArray[99];                     // ae_array<T*,99> mBankArray
};

template <typename T>
template <typename KeyT, typename ValueT>
ValueT InplaceAssetBankSet<T>::Find(TPakId pakId, KeyT key, AeType<ValueT>,
                                    TPakId* foundPakId) const
{
    ValueT result;
    if (pakId == PAK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 121;
        AeAssert::gCurrentExpr = "pakId != PAK_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad pak id"))
            __debugbreak();
        result.mValue = nullptr;
        result.mPakId = (unsigned int)PAK_ID_INVALID;
        return result;
    }
    unsigned int v7 = 0;
    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    GetPakPrerequisites(pakId, &prereqs);
    if (prereqs.m_size <= 0)
    {
        result.mValue = nullptr;
        result.mPakId = (unsigned int)PAK_ID_INVALID;
        return result;
    }
    while (1)
    {
        if (v7 >= 32)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        TPakId v8 = prereqs.m_elements[v7];
        if (v8 != PAK_ID_INVALID)
        {
            int idx = (int)prereqs.m_elements[v7];
            if (idx > 0x62)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 31;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _SIZE";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            T* v9 = mBankArray[idx];
            if (v9 != nullptr)
            {
                unsigned int* v10 = v9->mTree.Find(key);
                if (v10 != nullptr)
                {
                    unsigned int v11 = *v10;
                    if (foundPakId != nullptr)
                        *foundPakId = v8;
                    T* bank = v9;
                    result.mPakId = (unsigned int)v8;
                    result.mValue = (typename T::ElementType*)bank->operator[](v11);
                    return result;
                }
            }
        }
        if (++v7 >= (unsigned int)prereqs.m_size)
            break;
    }
    result.mValue = nullptr;
    result.mPakId = (unsigned int)PAK_ID_INVALID;
    return result;
}

template <typename T>
InplaceAssetBankSet<T>::InplaceAssetBankSet()
{
    for (unsigned int i = 0; i < 99; ++i)
        mBankArray[i] = nullptr;
}

template <typename T>
InplaceAssetBankSet<T>::~InplaceAssetBankSet()
{
}

template <typename T>
void InplaceAssetBankSet<T>::AddBank(TPakId pakId, T* bank)
{
    if (mBankArray[(int)pakId] != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 109;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("We already have a bank for this pak id!"))
        {
            __debugbreak();
        }
    }
    mBankArray[(int)pakId] = bank;
}

template <typename T>
void InplaceAssetBankSet<T>::UnloadBank(TPakId pakId)
{
    if (mBankArray[(int)pakId] != nullptr)
    {
        OnBankUnloaded(*mBankArray[(int)pakId]);
        mBankArray[(int)pakId] = nullptr;
    }
}

template <typename T>
void InplaceAssetBankSet<T>::OnBankUnloaded(T& bank)
{
    (void)bank;
}

template class InplaceAssetBankSet<XModelBank>;
template class InplaceAssetBankSet<XModelPartsBank>;

// XModel / XModelLod / XModelParts views (sv_stubs.h layouts)
class XModelParts {
public:
    uint8_t _pad[0x40];
};
struct XModelLod {
    float dist;           // +0x00
    InplaceString filename;  // +0x04
    XModelParts* xmodelParts;  // +0x08
};
class XModel {
public:
    uint8_t _pad0[0x20];
    XModelParts* parts;   // +0x20
    XModelLod* lod[5];    // +0x24
    uint8_t _pad28[0x48 - 0x28];
    InplaceString name;   // +0x48
};

namespace AeStringSupport {
void CStrToAeStr(char* dst, int* const len, int capacity,
                 const char* src);  // ?CStrToAeStr@AeStringSupport@@YAXPADPAHHPBD@Z
void GetFileName(char* dst, int* const len, const char* src, int a4,
                 bool truncExt);  // ?GetFileName@AeStringSupport@@YAXPADPAHPBDH_N@Z
void AeStrCopy(char* dst, int* const len, int capacity, const char* src,
               int a5);  // ?AeStrCopy@AeStringSupport@@YAXPADPAHHPBDH@Z
}

class XModelManager : public InplaceAssetBankSet<XModelBank> {
private:
    XModelManager();             // ??0XModelManager@@AAE@XZ
    virtual ~XModelManager();    // ??1XModelManager@@EAE@XZ
    void PostProcess(XModelBank* xmodelBank, TPakId pak_id);  // ?PostProcess@XModelManager@@AAEXPAVXModelBank@@W4TPakId@@@Z
public:
    static XModelManager* sInst;  // ?sInst@XModelManager@@2PAV1@A (sv_globals.cpp)
    static void CreateInst();      // ?CreateInst@XModelManager@@SAXXZ
    static void DeleteInst();      // ?DeleteInst@XModelManager@@SAXXZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // ?DecodeBank@XModelManager@@QAEXPBDPAEHW4TPakId@@@Z
    IVPointer<XModel> GetXModel(TPakId pak_id,
                                const char* name);  // ?GetXModel@XModelManager@@QAE?AV?$IVPointer@VXModel@@@@W4TPakId@@PBD@Z
};

class XModelPartsManager : public InplaceAssetBankSet<XModelPartsBank> {
private:
    XModelPartsManager();        // ??0XModelPartsManager@@AAE@XZ
    virtual ~XModelPartsManager();  // ??1XModelPartsManager@@EAE@XZ
    void PostProcess(XModelPartsBank* xmpBank, TPakId pak_id);  // ?PostProcess@XModelPartsManager@@AAEXPAVXModelPartsBank@@W4TPakId@@@Z
public:
    static XModelPartsManager* sInst;  // ?sInst@XModelPartsManager@@2PAV1@A (sv_globals.cpp)
    static void CreateInst();           // ?CreateInst@XModelPartsManager@@SAXXZ
    static void DeleteInst();           // ?DeleteInst@XModelPartsManager@@SAXXZ
    void DecodeBank(const char* name, unsigned char* data, int size,
                    TPakId pak_id);  // ?DecodeBank@XModelPartsManager@@QAEXPBDPAEHW4TPakId@@@Z
    IVPointer<XModelParts> GetXModelParts(TPakId pak_id,
                                          const char* name);  // ?GetXModelParts@XModelPartsManager@@QAE?AV?$IVPointer@VXModelParts@@@@W4TPakId@@PBD@Z
    void AssignHashName(XModelParts* xmp);  // ?AssignHashName@XModelPartsManager@@QAEXPAVXModelParts@@@Z
};

XModelManager::XModelManager()
{
}

XModelManager::~XModelManager()
{
}

// ea: 0x004B4E40
void XModelManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    XModelManager* result = static_cast<XModelManager*>(
        mem_heap_malloc_ctx(0x190u, 4, "core",
                            "c:\\cod\\code\\game\\XModelManager.h", 36));
    if (result != nullptr)
    {
        result = new (result) XModelManager();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
}

// ea: 0x004B4F40
void XModelManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

XModelPartsManager::XModelPartsManager()
{
}

XModelPartsManager::~XModelPartsManager()
{
}

// ea: 0x004B4FD0
void XModelPartsManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.h";
        AeAssert::gCurrentLine = 59;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    XModelPartsManager* result = static_cast<XModelPartsManager*>(
        mem_heap_malloc_ctx(0x190u, 4, "core",
                            "c:\\cod\\code\\game\\XModelManager.h", 59));
    if (result != nullptr)
    {
        result = new (result) XModelPartsManager();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
}

// ea: 0x004B50D0
void XModelPartsManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelManager.h";
        AeAssert::gCurrentLine = 59;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete sInst;
    sInst = nullptr;
}

// ea: 0x006D5FD0
IVPointer<XModel> XModelManager::GetXModel(TPakId pak_id, const char* name)
{
    char nm[128];
    char oBuff[128];
    char dstBuff[128];

    int nameLen = 0;
    AeStringSupport::CStrToAeStr(nm, &nameLen, 127, name);
    nm[127] = 0;
    dstBuff[0] = 0;
    AeStringSupport::GetFileName(dstBuff, &nameLen, nm, nameLen, true);
    AeStringSupport::AeStrCopy(oBuff, &nameLen, 127, dstBuff, nameLen);
    oBuff[127] = 0;
    memcpy(nm, oBuff, sizeof(nm));

    IVPointer<XModel> xm = Find<char*, IVPointer<XModel>>(
        pak_id, nm, AeType<IVPointer<XModel>>(), nullptr);
    TPakId mPakId = (TPakId)xm.mPakId;
    ValidatePakId((TPakId)mPakId);
    XModel* mValue = xm.mValue;
    if (xm.mValue != nullptr)
    {
        XModelLod** lod = xm.mValue->lod;
        for (char lodIdx = 5; lodIdx != 0; --lodIdx)
        {
            ValidatePakId(mPakId);
            if (*lod != nullptr)
            {
                ValidatePakId(mPakId);
                XModelPartsManager::sInst->AssignHashName(
                    (*lod)->xmodelParts);
            }
            ++lod;
        }
    }
    IVPointer<XModel> result;
    result.mPakId = (unsigned int)mPakId;
    result.mValue = mValue;
    return result;
}

extern TPakId PakManager_GetDebugPakId();
IVPointer<XModel> gDefaultXmodel = { nullptr, PAK_ID_INVALID };

// ea: 0x006DBBB0
void XModelManager::DecodeBank(const char* name, unsigned char* data,
                               int size, TPakId pak_id)
{
    (void)name;
    (void)size;
    XModelBank* xmodelBank = reinterpret_cast<XModelBank*>(data);
    xmodelBank->Fixup();
    PostProcess(xmodelBank, pak_id);
    AddBank(pak_id, xmodelBank);
    if (pak_id == PakManager_GetDebugPakId())
    {
        IVPointer<XModel> result;
        result = Find<const char*, IVPointer<XModel>>(
            pak_id, "noxmp", AeType<IVPointer<XModel>>(), nullptr);
        gDefaultXmodel = result;
    }
}

// ea: 0x006D6120
void XModelPartsManager::DecodeBank(const char* name, unsigned char* data,
                                    int size, TPakId pak_id)
{
    (void)name;
    (void)size;
    XModelPartsBank* xmpBank = reinterpret_cast<XModelPartsBank*>(data);
    xmpBank->Fixup();
    PostProcess(xmpBank, pak_id);
    AddBank(pak_id, xmpBank);
}

// ea: 0x006D60F0
IVPointer<XModelParts> XModelPartsManager::GetXModelParts(TPakId pak_id,
                                                          const char* name)
{
    return Find<char const*, IVPointer<XModelParts>>(
        pak_id, name, AeType<IVPointer<XModelParts>>(), nullptr);
}

// ea: 0x006D7680
IVPointer<XModel> RE_RegisterModel(const char* name, TPakId pakId, int a3)
{
    (void)a3;
    return XModelManager::sInst->GetXModel(pakId, name);
}

// ea: 0x006D9730
void XModelManager::PostProcess(XModelBank* xmodelBank, TPakId pak_id)
{
    unsigned int mSize = xmodelBank->mPtrs.mSize;
    unsigned int v5 = 0;
    int i = 0;
    if (mSize == 0)
        return;
    while (1)
    {
        if (v5 >= mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
            AeAssert::gCurrentLine = 199;
            AeAssert::gCurrentExpr = "i<mPtrs.size()";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("bounds check"))
                __debugbreak();
        }
        bool v6 = v5 < xmodelBank->mPtrs.mSize;
        if (v5 >= xmodelBank->mPtrs.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
            AeAssert::gCurrentLine = 81;
            AeAssert::gCurrentExpr = "index < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
                __debugbreak();
            v6 = v5 < xmodelBank->mPtrs.mSize;
        }
        if (!v6)
            v5 = 0;
        XModel* v7 = (XModel*)xmodelBank->mPtrs.mList[v5];
        bool found = false;
        XModelLod** lod = v7->lod;
        for (int j = 5; j != 0; --j)
        {
            if (*lod != nullptr)
            {
                const char* mStr = (*lod)->filename.mStr;
                found = true;
                IVPointer<XModelParts> result =
                    XModelPartsManager::sInst->Find<char const*,
                                                     IVPointer<XModelParts>>(
                        pak_id, mStr, AeType<IVPointer<XModelParts>>(),
                        nullptr);
                ValidatePakId((TPakId)result.mPakId);
                (*lod)->xmodelParts = result.mValue;
                if (v7->parts == nullptr)
                    v7->parts = (*lod)->xmodelParts;
                if ((*lod)->xmodelParts == nullptr)
                {
                    AeAssert::gCurrentAuthor = AeAssert::JRS;
                    AeAssert::gCurrentFile =
                        "c:\\cod\\code\\game\\XModelManager.cpp";
                    AeAssert::gCurrentLine = 91;
                    AeAssert::gCurrentExpr = "model->lod[lod]->xmodelParts";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert(
                            "Failed to find XModelParts '%s' for XModel '%s'.",
                            (*lod)->filename.mStr, v7->name.mStr))
                        __debugbreak();
                }
            }
            ++lod;
        }
        if (!found)
        {
            AeAssert::gCurrentAuthor = AeAssert::JRS;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\XModelManager.cpp";
            AeAssert::gCurrentLine = 95;
            AeAssert::gCurrentExpr = "found";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("XModel %s has no parts.", v7->name.mStr))
                __debugbreak();
        }
        mSize = xmodelBank->mPtrs.mSize;
        if (++i >= (int)mSize)
            break;
        v5 = (unsigned int)i;
    }
}

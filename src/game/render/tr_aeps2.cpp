// ============================================================================
// tr_aeps2.cpp - render.o aps error check + model manager dtors
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "aeps/apsError.h"

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
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
class XModelBank;
class XModelPartsBank;

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
protected:
    virtual void OnBankUnloaded(T& bank);  // ?OnBankUnloaded@?$InplaceAssetBankSet@VXModelBank@@@@MAEXAAVXModelBank@@@Z
    virtual void UnloadBank(TPakId pakId); // ?UnloadBank@?$InplaceAssetBankSet@VXModelBank@@@@MAEXW4TPakId@@@Z
    T* mBankArray[99];                     // ae_array<T*,99> mBankArray
};

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

class XModelManager : public InplaceAssetBankSet<XModelBank> {
private:
    XModelManager();             // ??0XModelManager@@AAE@XZ
    virtual ~XModelManager();    // ??1XModelManager@@EAE@XZ
};

class XModelPartsManager : public InplaceAssetBankSet<XModelPartsBank> {
private:
    XModelPartsManager();        // ??0XModelPartsManager@@AAE@XZ
    virtual ~XModelPartsManager();  // ??1XModelPartsManager@@EAE@XZ
};

XModelManager::XModelManager()
{
}

XModelManager::~XModelManager()
{
}

XModelPartsManager::XModelPartsManager()
{
}

XModelPartsManager::~XModelPartsManager()
{
}

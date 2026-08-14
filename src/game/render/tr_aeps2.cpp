// ============================================================================
// tr_aeps2.cpp - render.o aps error check + model manager dtors
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "aeps/apsError.h"

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Warning(const char* fmtstring, ...);
bool Error(const char* fmtstring, ...);
}

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
// XModelManager / XModelPartsManager protected virtual dtors
// ============================================================================
class AssetBankSet {
public:
    virtual ~AssetBankSet();  // ??1AssetBankSet@@UAE@XZ (streamer.o)
};

class XModelManager : public AssetBankSet {
private:
    virtual ~XModelManager();  // ??1XModelManager@@EAE@XZ
};

class XModelPartsManager : public AssetBankSet {
private:
    virtual ~XModelPartsManager();  // ??1XModelPartsManager@@EAE@XZ
};

XModelManager::~XModelManager()
{
}

XModelPartsManager::~XModelPartsManager()
{
}

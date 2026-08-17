// ============================================================================
// loading_menu.cpp - LoadingMenu (shell.o InGameMenus.cpp family)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/cvar_types.h"

#include <stdlib.h>
#include <string.h>
#include <new>

extern unsigned int AeHash(const char* str);        // core/ae_hash.cpp (binary _AeHash 0x7BF220)
extern void* mem_heap_malloc(unsigned int size);    // core.o
extern void mem_heap_free(void* ptr);               // core.o
extern void* FEManager_GetIGMS(void* self, int client);  // g_entity_misc.cpp
extern nglTexture* GetTextureData(const char* name, int image_type,
                                  const char* fromPak);  // core.o
extern int irand(int min, int max);                 // game.o

// Minimal STBManager view (full definition lives in game/core/core_systems.h,
// which conflicts with sv_stubs.h; same pattern as stringed_hooks.cpp).
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // ?GetSTBString@STBManager@@QAEPBDPBD@Z (core.o)
    const char* GetSTBString(unsigned int hash);  // ?GetSTBString@STBManager@@QAEPBDI@Z (core.o)
};

extern FEManager g_femanager;
extern int currCl;                                  // ?currCl@@3HA @ 0xF1579C
extern vmCvar_t cg_widescreen;                      // cg.o @ 0xF5CC88

namespace View {
void SetNumViewports(int num);      // ?SetNumViewports@View@@YAXH@Z (cg.o 0x69B0B0)
void UpdateNumViewports();          // ?UpdateNumViewports@View@@YAXXZ (cg.o 0x69B100)
}

// mp.o / mp_shell.o symbols (not ported yet; link tolerates the unresolveds)
struct sServerCreateParams {
    char mRandomMapList[64];        // +0x00
    char mName[24];                 // +0x40
    unsigned char mMapID;           // +0x58
    unsigned char mGameType;        // +0x59
    unsigned char mGameSubType;     // +0x5A
    unsigned char mMaxPlayers;      // +0x5B
    unsigned char mTeamBalancing;   // +0x5C
    unsigned char mFriendlyFire;    // +0x5D
    unsigned char mPrivateSlots;    // +0x5E
    unsigned char mTimeLimit;       // +0x5F
    unsigned char mScoreLimit;      // +0x60
    unsigned char mRoundLimit;      // +0x61
    unsigned char mSwapEnds;        // +0x62
    unsigned char mRespawnTime;     // +0x63
    bool mDontRotate;               // +0x64
    bool mDoChangeMap;              // +0x65
    unsigned char mEnableAARVote;   // +0x66
    unsigned char mEnablePenaltyVote;  // +0x67
    unsigned char mMapRotation;     // +0x68
};
static_assert(sizeof(sServerCreateParams) == 105,
              "sServerCreateParams size mismatch");

class MPUIInterface {
public:
    static sServerCreateParams mServerParams;  // ?mServerParams@MPUIInterface@@1UsServerCreateParams@@A (mp.o)
};

bool MI_IsAvailableMap(unsigned char id);        // ?MI_IsAvailableMap@@YA_ND@Z (mp_shell.o)
const char* MI_GetMapTitle(unsigned char id);    // ?MI_GetMapTitle@@YAPADD@Z (mp_shell.o)
const char* MI_GetMapLocation(unsigned char id); // ?MI_GetMapLocation@@YAPADD@Z (mp_shell.o)
char MI_GetMapIndexbyID(unsigned char ID);       // ?MI_GetMapIndexbyID@@YADD@Z (mp_shell.o)

// ============================================================================
// LoadingMenu data (shell.o InGameMenus.cpp data; VAs from IDA)
// ============================================================================
const char* szLoadingScreenBackgroundArt[9] = {
    "ls_bkg", "ls_bkg_line_01", "ls_bkg_detail_01", "ls_bkg_detail_03",
    "ls_bkg_detail_05", "LS_text_tip_title", "LS_text_map_01",
    "i_MP_LS_image_03.tga", "i_MP_LS_image_08.tga",
};
const char* szLoadingScreenTitleText[2] = {
    "LS_text_screen_title", "LS_text_tip_description",
};
const char* szLoadingScreenText[1] = {
    "LS_text_tip_description",
};
const char* szLoadingScreenMapText[2] = {
    "LS_text_map_01", "i_MP_LS_image_03.tga",
};

const char* LoadingMenu::szMapImageFiles[] = {
    "i_MP_LS_image_03.tga", "i_MP_LS_image_08.tga", "i_MP_LS_image_06.tga",
    "i_MP_LS_image_04.tga", "i_MP_LS_image_08.tga", "i_MP_LS_image_08.tga",
    "i_MP_LS_image_generic_01a.tga", "english", "french", "german", "italian",
    "spanish", "british", "russian", "polish", "korean", "taiwanese",
    "japanese", "chinese", "thai", "leet",
};

// Tip hash tables. The release build computes these in static initializers via
// HashString::CalcHash (which forwards to AeHash); values match the runtime
// contents of the binary's arrays.
unsigned int szGeneralTipTitlesHash[86] = {
    AeHash("MPLOADING_TIP_TITLE_GENERAL_01"), AeHash("MPLOADING_TIP_TITLE_GENERAL_02"), AeHash("MPLOADING_TIP_TITLE_GENERAL_03"), AeHash("MPLOADING_TIP_TITLE_GENERAL_04"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_05"), AeHash("MPLOADING_TIP_TITLE_GENERAL_06"), AeHash("MPLOADING_TIP_TITLE_GENERAL_07"), AeHash("MPLOADING_TIP_TITLE_GENERAL_08"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_09"), AeHash("MPLOADING_TIP_TITLE_GENERAL_10"), AeHash("MPLOADING_TIP_TITLE_GENERAL_12"), AeHash("MPLOADING_TIP_TITLE_GENERAL_13"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_14"), AeHash("MPLOADING_TIP_TITLE_GENERAL_15"), AeHash("MPLOADING_TIP_TITLE_GENERAL_16"), AeHash("MPLOADING_TIP_TITLE_GENERAL_17"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_18"), AeHash("MPLOADING_TIP_TITLE_GENERAL_20"), AeHash("MPLOADING_TIP_TITLE_GENERAL_21"), AeHash("MPLOADING_TIP_TITLE_GENERAL_22"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_23"), AeHash("MPLOADING_TIP_TITLE_GENERAL_24"), AeHash("MPLOADING_TIP_TITLE_GENERAL_25"), AeHash("MPLOADING_TIP_TITLE_GENERAL_27"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_28"), AeHash("MPLOADING_TIP_TITLE_GENERAL_29"), AeHash("MPLOADING_TIP_TITLE_GENERAL_30"), AeHash("MPLOADING_TIP_TITLE_GENERAL_32"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_33"), AeHash("MPLOADING_TIP_TITLE_GENERAL_34"), AeHash("MPLOADING_TIP_TITLE_GENERAL_36"), AeHash("MPLOADING_TIP_TITLE_GENERAL_37"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_38"), AeHash("MPLOADING_TIP_TITLE_GENERAL_39"), AeHash("MPLOADING_TIP_TITLE_GENERAL_40"), AeHash("MPLOADING_TIP_TITLE_GENERAL_41"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_42"), AeHash("MPLOADING_TIP_TITLE_GENERAL_43"), AeHash("MPLOADING_TIP_TITLE_GENERAL_44"), AeHash("MPLOADING_TIP_TITLE_GENERAL_46"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_47"), AeHash("MPLOADING_TIP_TITLE_GENERAL_48"), AeHash("MPLOADING_TIP_TITLE_GENERAL_49"), AeHash("MPLOADING_TIP_TITLE_GENERAL_50"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_51"), AeHash("MPLOADING_TIP_TITLE_GENERAL_52"), AeHash("MPLOADING_TIP_TITLE_GENERAL_53"), AeHash("MPLOADING_TIP_TITLE_GENERAL_54"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_55"), AeHash("MPLOADING_TIP_TITLE_GENERAL_56"), AeHash("MPLOADING_TIP_TITLE_GENERAL_57"), AeHash("MPLOADING_TIP_TITLE_GENERAL_61"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_63"), AeHash("MPLOADING_TIP_TITLE_GENERAL_66"), AeHash("MPLOADING_TIP_TITLE_GENERAL_68"), AeHash("MPLOADING_TIP_TITLE_GENERAL_69"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_70"), AeHash("MPLOADING_TIP_TITLE_GENERAL_71"), AeHash("MPLOADING_TIP_TITLE_GENERAL_73"), AeHash("MPLOADING_TIP_TITLE_GENERAL_74"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_75"), AeHash("MPLOADING_TIP_TITLE_GENERAL_76"), AeHash("MPLOADING_TIP_TITLE_GENERAL_78"), AeHash("MPLOADING_TIP_TITLE_GENERAL_79"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_80"), AeHash("MPLOADING_TIP_TITLE_GENERAL_81"), AeHash("MPLOADING_TIP_TITLE_GENERAL_82"), AeHash("MPLOADING_TIP_TITLE_GENERAL_83"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_84"), AeHash("MPLOADING_TIP_TITLE_GENERAL_85"), AeHash("MPLOADING_TIP_TITLE_GENERAL_86"), AeHash("MPLOADING_TIP_TITLE_GENERAL_87"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_88"), AeHash("MPLOADING_TIP_TITLE_GENERAL_89"), AeHash("MPLOADING_TIP_TITLE_GENERAL_90"), AeHash("MPLOADING_TIP_TITLE_GENERAL_91"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_92"), AeHash("MPLOADING_TIP_TITLE_GENERAL_93"), AeHash("MPLOADING_TIP_TITLE_GENERAL_94"), AeHash("MPLOADING_TIP_TITLE_GENERAL_95"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_96"), AeHash("MPLOADING_TIP_TITLE_GENERAL_97"), AeHash("MPLOADING_TIP_TITLE_GENERAL_98"), AeHash("MPLOADING_TIP_TITLE_GENERAL_99"),
    AeHash("MPLOADING_TIP_TITLE_GENERAL_100"), AeHash("MPLOADING_TIP_TITLE_GENERAL_101"),
};

unsigned int szWarTipTitlesHash[5] = {
    AeHash("MPLOADING_TIP_TITLE_WAR_01"), AeHash("MPLOADING_TIP_TITLE_WAR_02"), AeHash("MPLOADING_TIP_TITLE_WAR_03"), AeHash("MPLOADING_TIP_TITLE_WAR_04"),
    AeHash("MPLOADING_TIP_TITLE_WAR_05"),
};

unsigned int szCTFTipTitlesHash[1] = {
    AeHash("MPLOADING_TIP_TITLE_CTF_01"),
};

unsigned int szSCFTipTitlesHash[1] = {
    AeHash("MPLOADING_TIP_TITLE_SCF_01"),
};

unsigned int szHQTipTitlesHash[5] = {
    AeHash("MPLOADING_TIP_TITLE_HQ_01"), AeHash("MPLOADING_TIP_TITLE_HQ_02"), AeHash("MPLOADING_TIP_TITLE_HQ_03"), AeHash("MPLOADING_TIP_TITLE_HQ_04"),
    AeHash("MPLOADING_TIP_TITLE_HQ_05"),
};

unsigned int szTDMTipTitlesHash[1] = {
    AeHash("MPLOADING_TIP_TITLE_TDM_01"),
};

unsigned int szDMTipTitlesHash[1] = {
    AeHash("MPLOADING_TIP_TITLE_DM_01"),
};

unsigned int szGeneralTipsHash[86] = {
    AeHash("MPLOADING_TIP_GENERAL_01"), AeHash("MPLOADING_TIP_GENERAL_02"), AeHash("MPLOADING_TIP_GENERAL_03"), AeHash("MPLOADING_TIP_GENERAL_04"),
    AeHash("MPLOADING_TIP_GENERAL_05"), AeHash("MPLOADING_TIP_GENERAL_06"), AeHash("MPLOADING_TIP_GENERAL_07"), AeHash("MPLOADING_TIP_GENERAL_08"),
    AeHash("MPLOADING_TIP_GENERAL_09"), AeHash("MPLOADING_TIP_GENERAL_10"), AeHash("MPLOADING_TIP_GENERAL_12"), AeHash("MPLOADING_TIP_GENERAL_13"),
    AeHash("MPLOADING_TIP_GENERAL_14"), AeHash("MPLOADING_TIP_GENERAL_15"), AeHash("MPLOADING_TIP_GENERAL_16"), AeHash("MPLOADING_TIP_GENERAL_17"),
    AeHash("MPLOADING_TIP_GENERAL_18"), AeHash("MPLOADING_TIP_GENERAL_20"), AeHash("MPLOADING_TIP_GENERAL_21"), AeHash("MPLOADING_TIP_GENERAL_22"),
    AeHash("MPLOADING_TIP_GENERAL_23"), AeHash("MPLOADING_TIP_GENERAL_24"), AeHash("MPLOADING_TIP_GENERAL_25"), AeHash("MPLOADING_TIP_GENERAL_27"),
    AeHash("MPLOADING_TIP_GENERAL_28"), AeHash("MPLOADING_TIP_GENERAL_29"), AeHash("MPLOADING_TIP_GENERAL_30"), AeHash("MPLOADING_TIP_GENERAL_32"),
    AeHash("MPLOADING_TIP_GENERAL_33"), AeHash("MPLOADING_TIP_GENERAL_34"), AeHash("MPLOADING_TIP_GENERAL_36"), AeHash("MPLOADING_TIP_GENERAL_37"),
    AeHash("MPLOADING_TIP_GENERAL_38"), AeHash("MPLOADING_TIP_GENERAL_39"), AeHash("MPLOADING_TIP_GENERAL_40"), AeHash("MPLOADING_TIP_GENERAL_41"),
    AeHash("MPLOADING_TIP_GENERAL_42"), AeHash("MPLOADING_TIP_GENERAL_43"), AeHash("MPLOADING_TIP_GENERAL_44"), AeHash("MPLOADING_TIP_GENERAL_46"),
    AeHash("MPLOADING_TIP_GENERAL_47"), AeHash("MPLOADING_TIP_GENERAL_48"), AeHash("MPLOADING_TIP_GENERAL_49"), AeHash("MPLOADING_TIP_GENERAL_50"),
    AeHash("MPLOADING_TIP_GENERAL_51"), AeHash("MPLOADING_TIP_GENERAL_52"), AeHash("MPLOADING_TIP_GENERAL_53"), AeHash("MPLOADING_TIP_GENERAL_54"),
    AeHash("MPLOADING_TIP_GENERAL_55"), AeHash("MPLOADING_TIP_GENERAL_56"), AeHash("MPLOADING_TIP_GENERAL_57"), AeHash("MPLOADING_TIP_GENERAL_61"),
    AeHash("MPLOADING_TIP_GENERAL_63"), AeHash("MPLOADING_TIP_GENERAL_66"), AeHash("MPLOADING_TIP_GENERAL_68"), AeHash("MPLOADING_TIP_GENERAL_69"),
    AeHash("MPLOADING_TIP_GENERAL_70"), AeHash("MPLOADING_TIP_GENERAL_71"), AeHash("MPLOADING_TIP_GENERAL_73"), AeHash("MPLOADING_TIP_GENERAL_74"),
    AeHash("MPLOADING_TIP_GENERAL_75"), AeHash("MPLOADING_TIP_GENERAL_76"), AeHash("MPLOADING_TIP_GENERAL_78"), AeHash("MPLOADING_TIP_GENERAL_79"),
    AeHash("MPLOADING_TIP_GENERAL_80"), AeHash("MPLOADING_TIP_GENERAL_81"), AeHash("MPLOADING_TIP_GENERAL_82"), AeHash("MPLOADING_TIP_GENERAL_83"),
    AeHash("MPLOADING_TIP_GENERAL_84"), AeHash("MPLOADING_TIP_GENERAL_85"), AeHash("MPLOADING_TIP_GENERAL_86"), AeHash("MPLOADING_TIP_GENERAL_87"),
    AeHash("MPLOADING_TIP_GENERAL_88"), AeHash("MPLOADING_TIP_GENERAL_89"), AeHash("MPLOADING_TIP_GENERAL_90"), AeHash("MPLOADING_TIP_GENERAL_91"),
    AeHash("MPLOADING_TIP_GENERAL_92"), AeHash("MPLOADING_TIP_GENERAL_93"), AeHash("MPLOADING_TIP_GENERAL_94"), AeHash("MPLOADING_TIP_GENERAL_95"),
    AeHash("MPLOADING_TIP_GENERAL_96"), AeHash("MPLOADING_TIP_GENERAL_97"), AeHash("MPLOADING_TIP_GENERAL_98"), AeHash("MPLOADING_TIP_GENERAL_99"),
    AeHash("MPLOADING_TIP_GENERAL_100"), AeHash("MPLOADING_TIP_GENERAL_101"),
};

unsigned int szWarTipsHash[5] = {
    AeHash("MPLOADING_TIP_WAR_01"), AeHash("MPLOADING_TIP_WAR_02"), AeHash("MPLOADING_TIP_WAR_03"), AeHash("MPLOADING_TIP_WAR_04"),
    AeHash("MPLOADING_TIP_WAR_05"),
};

unsigned int szCTFTipsHash[1] = {
    AeHash("MPLOADING_TIP_CTF_01"),
};

unsigned int szSCFTipsHash[1] = {
    AeHash("MPLOADING_TIP_SCF_01"),
};

unsigned int szHQTipsHash[5] = {
    AeHash("MPLOADING_TIP_HQ_01"), AeHash("MPLOADING_TIP_HQ_02"), AeHash("MPLOADING_TIP_HQ_03"), AeHash("MPLOADING_TIP_HQ_04"),
    AeHash("MPLOADING_TIP_HQ_05"),
};

unsigned int szTDMTipsHash[1] = {
    AeHash("MPLOADING_TIP_TDM_01"),
};

unsigned int szDMTipsHash[1] = {
    AeHash("MPLOADING_TIP_DM_01"),
};

// ============================================================================
// LoadingMenu
// ============================================================================

// ea: 0x00592E90
LoadingMenu::LoadingMenu(FEMenuSystem* s)
    : FEMenu(s, 0, 320, 240, 8, 0)
{
    mTipEntry = nullptr;
    mWidescreen = false;
    m_pLoadingBar = nullptr;
    m_fLoadingBarLeft = 0.0f;
    m_fLoadingBarRight = 0.0f;
    m_fLoadingBarTop = 0.0f;
    m_fLoadingBarBottom = 0.0f;

    mTipArrays[6].mCount = 86;
    mTipArrays[6].mArray[0] = szGeneralTipTitlesHash;
    mTipArrays[0].mArray[0] = szWarTipTitlesHash;
    mTipArrays[1].mArray[0] = szCTFTipTitlesHash;
    mTipArrays[2].mArray[0] = szSCFTipTitlesHash;
    mTipArrays[3].mArray[0] = szHQTipTitlesHash;
    mTipArrays[4].mArray[0] = szTDMTipTitlesHash;
    mTipArrays[5].mArray[0] = szDMTipTitlesHash;
    mTipArrays[6].mArray[1] = szGeneralTipsHash;
    mTipArrays[0].mArray[1] = szWarTipsHash;
    mTipArrays[1].mArray[1] = szCTFTipsHash;
    mTipArrays[2].mArray[1] = szSCFTipsHash;
    mTipArrays[3].mArray[1] = szHQTipsHash;
    mTipArrays[4].mArray[1] = szTDMTipsHash;
    mTipArrays[5].mArray[1] = szDMTipsHash;

    mTipArrays[1].mCount = 1;
    mTipArrays[2].mCount = 1;
    mTipArrays[4].mCount = 1;
    mTipArrays[5].mCount = 1;
    mTipArrays[0].mCount = 5;
    mTipArrays[3].mCount = 5;

    mBackgroundArt.m_elements[0] = nullptr;
    mBackgroundArt.m_elements[1] = nullptr;
    mBackgroundArt.m_elements[2] = nullptr;
    mBackgroundArt.m_elements[3] = nullptr;
    mBackgroundArt.m_elements[4] = nullptr;
    mBackgroundArt.m_elements[5] = nullptr;
    mBackgroundArt.m_elements[6] = nullptr;
    mBackgroundArt.m_elements[7] = nullptr;
    mBackgroundArt.m_elements[8] = nullptr;
    mTitleText.m_elements[0] = nullptr;
    mTitleText.m_elements[1] = nullptr;
    mText.m_elements[0] = nullptr;
    mMapText.m_elements[0] = nullptr;
    mMapText.m_elements[1] = nullptr;
    default_color_scheme = 5;
}

// ea: 0x00592FF0
LoadingMenu::~LoadingMenu()
{
    if (mTipEntry != nullptr)
        delete mTipEntry;
    mTipEntry = nullptr;
    if (panel != nullptr)
    {
        panel->~PanelFile();
        mem_heap_free(panel);
    }
    panel = nullptr;
}

// ea: 0x00574430
LoadingMenu* LoadingMenu::Me()
{
    return (LoadingMenu*)((InGameMenuSystem*)FEManager_GetIGMS(&g_femanager,
                                                              currCl))
        ->menus[4];
}

// ea: 0x00574450
void LoadingMenu::Update(float time_inc)
{
    FEMenu::Update(time_inc);
}

// ea: 0x00574460
void LoadingMenu::UpdateLoading(float percentDone)
{
    if (percentDone > mPercentDone)
        mPercentDone = percentDone;
}

// ea: 0x00574410
void LoadingMenu::Select(int entry_num)
{
    (void)entry_num;
}

// ea: 0x00574420
void LoadingMenu::OnCross(int c)
{
    (void)c;
}

// ea: 0x005743F0
void LoadingMenu::OnDeactivate(FEMenu* m)
{
    (void)m;
    g_femanager.mIGMS[0]->Update(0.0f);
    View::UpdateNumViewports();
}

// ea: 0x0057FFC0
void LoadingMenu::Draw()
{
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
    if (mTipEntry != nullptr)
        mTipEntry->Draw();
    if (m_pLoadingBar != nullptr)
        m_pLoadingBar->Mask(mPercentDone, RIGHT_MASK, 1.0f);
}

// ea: 0x00580230
void LoadingMenu::UpdateWidescreen(bool widescreen)
{
    if (mWidescreen == widescreen)
        return;
    if (mTipEntry != nullptr)
        mTipEntry->UpdateForWidescreen(widescreen);
    if (panel != nullptr)
    {
        mWidescreen = widescreen;
        panel->UpdateWidescreen(widescreen, 320.0f);
    }
    if (m_pLoadingBar != nullptr)
        m_pLoadingBar->SetXYInitialToCurrentPos();
}

// ea: 0x00580010
void LoadingMenu::PickTip()
{
    int mGameType = 6;
    if ((float)rand() * 0.000030517578f > 0.80000001f)
    {
        mGameType = MPUIInterface::mServerParams.mGameType;
        if (mGameType >= 6)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
            AeAssert::gCurrentLine = 479;
            AeAssert::gCurrentExpr =
                "arrayIndex >= 0 && arrayIndex < GAME_TYPE_LIMIT";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("game type index invalid"))
                __debugbreak();
        }
    }
    if (mTipArrays[mGameType].mCount <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 482;
        AeAssert::gCurrentExpr = "mTipArrays[arrayIndex].mCount > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("No tips available"))
            __debugbreak();
    }
    int tip = irand(0, mTipArrays[mGameType].mCount);
    if (mTipArrays[mGameType].mCount <= (unsigned int)tip)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 486;
        AeAssert::gCurrentExpr = "mTipArrays[arrayIndex].mCount > tip";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid tip index"))
            __debugbreak();
    }
    mTitleText.m_elements[1]->SetText(mTipArrays[mGameType].mArray[0][tip]);
    const char* tipString =
        STBManager::sInst->GetSTBString(mTipArrays[mGameType].mArray[1][tip]);
    if (strncmp(tipString, "STRING MISSING", 15) == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 494;
        AeAssert::gCurrentExpr = "strcmp(tipString, \"STRING MISSING\") != 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad Tip string index"))
            __debugbreak();
    }
    mTipEntry->SetTextBoxNoLocalize(Broc::string(tipString),
                                    mWidescreen ? 390 : 520, -1.0f);
}

// ea: 0x00596FB0
void LoadingMenu::OnActivate()
{
    FEMenu::OnActivate();
    View::SetNumViewports(1);
    if (mWidescreen != (cg_widescreen.integer != 0))
        UpdateWidescreen(cg_widescreen.integer != 0);
    mPercentDone = 0.0f;
    if (!MI_IsAvailableMap(MPUIInterface::mServerParams.mMapID))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 401;
        AeAssert::gCurrentExpr =
            "MI_IsAvailableMap(MPUIInterface::GetServerParams()->mMapID)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("LoadingMenu: Invalid map index"))
            __debugbreak();
    }
    char szMapTitle[32];
    char szMapLoc[32];
    strcpy(szMapTitle, STBManager::sInst->GetSTBString(
                           MI_GetMapTitle(MPUIInterface::mServerParams.mMapID)));
    strcpy(szMapLoc, STBManager::sInst->GetSTBString(
                         MI_GetMapLocation(MPUIInterface::mServerParams.mMapID)));
    mMapText.m_elements[0]->SetTextNoLocalize(szMapTitle);
    mMapText.m_elements[1]->SetTextNoLocalize(szMapLoc);
    int mapIndex = MI_GetMapIndexbyID(MPUIInterface::mServerParams.mMapID);
    PanelQuad* Pointer = panel->GetPointer("LS_image_01");
    Pointer->SetTexture(
        GetTextureData(szMapImageFiles[mapIndex], 0, "mp_loadingscreen"));
    PickTip();
}

// ea: 0x00593080
void LoadingMenu::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    FEMenu::Cleanup();
    for (int i = 0; i < 9; ++i)
        mBackgroundArt.m_elements[i] = nullptr;
    for (int j = 0; j < 2; ++j)
        mTitleText.m_elements[j] = nullptr;
    mText.m_elements[0] = nullptr;
    for (int k = 0; k < 2; ++k)
        mMapText.m_elements[k] = nullptr;
    if (mTipEntry != nullptr)
        delete mTipEntry;
    mTipEntry = nullptr;
    mWidescreen = false;
    panel = nullptr;
}

// ea: 0x00597120
void LoadingMenu::SetPanelFile(PanelFile* pf)
{
    panel = pf;
    for (int i = 0; i < 9; ++i)
    {
        if (mBackgroundArt.m_elements[i] != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
            AeAssert::gCurrentLine = 511;
            AeAssert::gCurrentExpr = "0 == mBackgroundArt[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Why is the array not null?"))
                __debugbreak();
        }
        mBackgroundArt.m_elements[i] =
            panel->GetPointer(szLoadingScreenBackgroundArt[i]);
        if (mBackgroundArt.m_elements[i] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
            AeAssert::gCurrentLine = 514;
            AeAssert::gCurrentExpr = "mBackgroundArt[i]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Not found!"))
                __debugbreak();
        }
    }
    for (int j = 0; j < 2; ++j)
    {
        mTitleText.m_elements[j] =
            panel->GetTextPointer(szLoadingScreenTitleText[j]);
        if (mTitleText.m_elements[j] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
            AeAssert::gCurrentLine = 520;
            AeAssert::gCurrentExpr = "mTitleText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Could not get loading screen title text!"))
                __debugbreak();
        }
        mTitleText.m_elements[j]->SetShown(true);
    }
    mTitleText.m_elements[0]->SetText("MPLOADING_LOADING");
    FEText* TextPointer = panel->GetTextPointer(szLoadingScreenText[0]);
    mText.m_elements[0] = TextPointer;
    if (TextPointer == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 529;
        AeAssert::gCurrentExpr = "mText[i]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Could not get loading screen text!"))
            __debugbreak();
    }
    mText.m_elements[0]->SetShown(true);
    for (int k = 0; k < 2; ++k)
    {
        mMapText.m_elements[k] =
            panel->GetTextPointer(szLoadingScreenMapText[k]);
        if (mMapText.m_elements[k] == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
            AeAssert::gCurrentLine = 536;
            AeAssert::gCurrentExpr = "mMapText[i]";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Could not get loading screen MAP text!"))
                __debugbreak();
        }
        mMapText.m_elements[k]->SetShown(true);
    }
    if (mText.m_elements[0] == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\InGameMenus.cpp";
        AeAssert::gCurrentLine = 540;
        AeAssert::gCurrentExpr = "mText[ls_text_tip_description]";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Could not locate the text description widget in the "
                "loading screen."))
            __debugbreak();
    }
    mTipEntry = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (mTipEntry != nullptr)
    {
        FEText* tipBase = mText.m_elements[0];
        color32 col = tipBase->GetColor();
        new (mTipEntry) FEMultiLineText(
            tipBase->GetFont(), tipBase->GetY(), 0.0f, 0,
            (panel_layer)tipBase->GetScaleX(), 16.0f, 64, (int)col.i, col);
    }
    mTipEntry->SetNumLines(10);
    PanelQuad* Pointer = panel->GetPointer("loading_bar_use");
    m_pLoadingBar = Pointer;
    if (Pointer != nullptr)
    {
        m_fLoadingBarLeft = m_pLoadingBar->GetMin().x;
        m_fLoadingBarRight = m_pLoadingBar->GetMax().x;
        m_fLoadingBarTop = m_pLoadingBar->GetMin().y;
        m_fLoadingBarBottom = m_pLoadingBar->GetMax().y;
    }
}

LoadingMenu* LoadingMenu_ctor(void* mem, FEMenuSystem* s)
{
    return new (mem) LoadingMenu(s);
}

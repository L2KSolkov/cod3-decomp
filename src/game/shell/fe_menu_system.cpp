// ============================================================================
// fe_menu_system.cpp - FrontEndMenuSystem + InGameMenuSystem (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "ngl/nglFont.h"
#include "ngl/nglTexture.h"
#include "game/platform_xbox/XboxLive.h"

#include <string.h>
#include <stdio.h>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char defaultFileName[];  // ?defaultFileName
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);  // core.o
extern vmCvar_t cg_widescreen;         // ?cg_widescreen@@3UvmCvar_t@@A
float blur_amount = 0.1f;              // ?blur_amount @ 0xDF4290
float blur_amount_0 = 0.1f;            // @ 0xDF4294
float blur_amount_1 = 0.1f;            // @ 0xDF4298
enum FULLSCREENBLUR_STATE {
    FULLSCREENBLUR_OFF = 0,
    FULLSCREENBLUR_START = 1,
    FULLSCREENBLUR_RUNNING = 2,
    FULLSCREENBLUR_FINISHED = 3,
    FULLSCREENBLUR_THISFRAMEONLY = 4,
};
extern FULLSCREENBLUR_STATE g_doFullScreenBlur[];
extern float g_fullScreenBlurAmount[]; // ?g_fullScreenBlurAmount@@3PAMA
extern bool gSkipFrontEnd;             // ?gSkipFrontEnd@@3_NA
extern int unk_F6A28C[];               // @ 0xF6A28C (per-client controller)
extern nglTexture* nglDefaultTex;      // ?nglDefaultTex@@3PAUnglTexture@@A

namespace View {
extern void SetViewportClipping(int clientIndex);  // cg.o
}

// STBManager minimal view (same pattern as other shell files)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);  // core.o
};

// MusicMgr minimal view
struct MusicMgr {
public:
    static MusicMgr* sInst;  // ?sInst@MusicMgr@@2PAV1@A @ 0xF4EBE4
    void Play(const char* name);  // ?Play@MusicMgr@@QAEXPBD@Z
};

// MPLiveEngine minimal view (mp.o)
class MPLiveEngine;
class MPLiveEngine {
public:
    int internalState;      // +0x04
    int lastLoginCode;      // +0x08
    unsigned int actualPort;  // +0x0C
    static MPLiveEngine* GetHandle();  // ?GetHandle@MPLiveEngine@@SAPAV1@XZ
};

// MPUIInterface::GetReturnMenu
class MPUIInterface {
public:
    static const int GetReturnMenu();  // ?GetReturnMenu@MPUIInterface@@SA?BHXZ
};

// GetTextureData (ngl/frontend helper)
extern nglTexture* GetTextureData(const char* name, int image_type,
                                  const char* fromPak);  // ?GetTextureData@@YAPAUnglTexture@@PBDH0@Z

// Menu classes owned by other objects (extern ctors / statics; unresolved
// until those objects land). Manglings from IDA.
class CreateSessionMenu;
class CreateLanSessionMenu;
class CreateSessionAdvancedMenu;
class CreateLanSessionAdvancedMenu;
class FindSessionMenu;
class FindLanSessionMenu;
class InitialLoadingMenu;
class InstantActionMenu;
class MPMainMenuXBox;
class PlayLanMenu;
class PlayOnlineMenu;
class PressStartMenu;
class SessionDetailsMenu;
class SessionListMenu;
class SessionLanListMenu;
class JoinGameMenu;
class OverlayMenu;
class MultilineOverlayMenu;
class VKMenu;
class MPOptionsGameplayMenu;
class MPOptionsControlsMenu;
class MPOptionsScreenMenu;
class MPOptionsSoundMenu;
class MPOptionsPreferencesMenu;
class OptionsStickMenu;
class OptionsButtonMenu;
class GammaScreenMenu;
class MPProfileMainMenu;
class MPProfileEditMenu;
class XboxLiveOptionsMenu;
class MemCardCheckMenu;
class PauseMenu;
class WeaponSelectMenu;
class GameSettingsEdit;
class GameSettingsView;
class XBoxLiveIngameOptionsCOD3;
class VoteMapMenu;
class VoteGameTypeMenu;
class InGameOptionsMenu;
class InGameScoreBoard;
class InGameSwitchSides;
class SpectateMenu;
class InGameOverlay;
class HotJoinMenu;
class InGameLiveOptionsMenu;
class DialogMenuDisplay;

extern CreateSessionMenu* CreateSessionMenu_ctor(void* mem,
                                                 FEMenuSystem* s);
extern CreateLanSessionMenu* CreateLanSessionMenu_ctor(void* mem,
                                                       FEMenuSystem* s);
extern CreateSessionAdvancedMenu* CreateSessionAdvancedMenu_ctor(
    void* mem, FEMenuSystem* s);
extern CreateLanSessionAdvancedMenu* CreateLanSessionAdvancedMenu_ctor(
    void* mem, FEMenuSystem* s);
extern FindSessionMenu* FindSessionMenu_ctor(void* mem, FEMenuSystem* s);
extern FindLanSessionMenu* FindLanSessionMenu_ctor(void* mem,
                                                   FEMenuSystem* s);
extern InitialLoadingMenu* InitialLoadingMenu_ctor(void* mem,
                                                   FEMenuSystem* s);
extern InstantActionMenu* InstantActionMenu_ctor(void* mem, FEMenuSystem* s);
extern MPMainMenuXBox* MPMainMenuXBox_ctor(void* mem, FEMenuSystem* s);
extern PlayLanMenu* PlayLanMenu_ctor(void* mem, FEMenuSystem* s);
extern PlayOnlineMenu* PlayOnlineMenu_ctor(void* mem, FEMenuSystem* s);
extern PressStartMenu* PressStartMenu_ctor(void* mem, FEMenuSystem* s);
extern SessionDetailsMenu* SessionDetailsMenu_ctor(void* mem,
                                                   FEMenuSystem* s);
extern SessionListMenu* SessionListMenu_ctor(void* mem, FEMenuSystem* s);
extern SessionLanListMenu* SessionLanListMenu_ctor(void* mem,
                                                   FEMenuSystem* s);
extern JoinGameMenu* JoinGameMenu_ctor(void* mem, FEMenuSystem* s);
extern OverlayMenu* OverlayMenu_ctor(void* mem, FEMenuSystem* s,
                                     int numEntries);
extern MultilineOverlayMenu* MultilineOverlayMenu_ctor(void* mem,
                                                       FEMenuSystem* s);
extern VKMenu* VKMenu_ctor(void* mem, FEMenuSystem* s);
extern MPOptionsGameplayMenu* MPOptionsGameplayMenu_ctor(void* mem,
                                                         FEMenuSystem* s);
extern MPOptionsControlsMenu* MPOptionsControlsMenu_ctor(void* mem,
                                                         FEMenuSystem* s);
extern MPOptionsScreenMenu* MPOptionsScreenMenu_ctor(void* mem,
                                                     FEMenuSystem* s);
extern MPOptionsSoundMenu* MPOptionsSoundMenu_ctor(void* mem,
                                                   FEMenuSystem* s);
extern MPOptionsPreferencesMenu* MPOptionsPreferencesMenu_ctor(void* mem,
                                                               FEMenuSystem* s);
extern OptionsStickMenu* OptionsStickMenu_ctor(void* mem, FEMenuSystem* s);
extern OptionsButtonMenu* OptionsButtonMenu_ctor(void* mem, FEMenuSystem* s);
extern GammaScreenMenu* GammaScreenMenu_ctor(void* mem, FEMenuSystem* s);
extern MemCardCheckMenu* MemCardCheckMenu_ctor(void* mem, FEMenuSystem* s);
extern MPProfileMainMenu* MPProfileMainMenu_ctor(void* mem, FEMenuSystem* s);
extern MPProfileEditMenu* MPProfileEditMenu_ctor(void* mem, FEMenuSystem* s);
extern XboxLiveOptionsMenu* XboxLiveOptionsMenu_ctor(void* mem,
                                                     FEMenuSystem* s);
extern PauseMenu* PauseMenu_ctor(void* mem, FEMenuSystem* s);
extern WeaponSelectMenu* WeaponSelectMenu_ctor(void* mem, FEMenuSystem* s);
extern LoadingMenu* LoadingMenu_ctor(void* mem, FEMenuSystem* s);
extern GameSettingsEdit* GameSettingsEdit_ctor(void* mem, FEMenuSystem* s);
extern GameSettingsView* GameSettingsView_ctor(void* mem, FEMenuSystem* s);
extern XBoxLiveIngameOptionsCOD3* XBoxLiveIngameOptionsCOD3_ctor(
    void* mem, FEMenuSystem* s);
extern VoteMapMenu* VoteMapMenu_ctor(void* mem, FEMenuSystem* s);
extern VoteGameTypeMenu* VoteGameTypeMenu_ctor(void* mem, FEMenuSystem* s);
extern InGameOptionsMenu* InGameOptionsMenu_ctor(void* mem, FEMenuSystem* s);
extern InGameScoreBoard* InGameScoreBoard_ctor(void* mem, FEMenuSystem* s);
extern InGameSwitchSides* InGameSwitchSides_ctor(void* mem, FEMenuSystem* s);
extern SpectateMenu* SpectateMenu_ctor(void* mem, FEMenuSystem* s);
extern InGameOverlay* InGameOverlay_ctor(void* mem, FEMenuSystem* s);
extern HotJoinMenu* HotJoinMenu_ctor(void* mem, FEMenuSystem* s);
extern InGameLiveOptionsMenu* InGameLiveOptionsMenu_ctor(void* mem,
                                                         FEMenuSystem* s);

extern GameSettingsEdit* GameSettingsEdit_Me(int version);
extern GameSettingsView* GameSettingsView_Me(int version);
extern PauseMenu* PauseMenu_Me(int version);
extern WeaponSelectMenu* WeaponSelectMenu_Me(int version);
extern InGameOptionsMenu* InGameOptionsMenu_Me(int version);
extern SpectateMenu* SpectateMenu_Me(int version);
extern DialogMenuDisplay* DialogMenuDisplay_Me(int viewport);
extern InGameOverlay* InGameOverlay_Me(int version);
extern HotJoinMenu* HotJoinMenu_Me(int version);
extern InGameScoreBoard* InGameScoreBoard_Me(int version);
extern InGameSwitchSides* InGameSwitchSides_Me(int version);
extern XBoxLiveIngameOptionsCOD3* XBoxLiveIngameOptionsCOD3_Me(int client);

// ============================================================================
// FrontEndMenuSystem statics
// ============================================================================
PanelFile* FrontEndMenuSystem::panelFile = nullptr;
FEText*    FrontEndMenuSystem::loginText = nullptr;

// ============================================================================
// FrontEndMenuSystem
// ============================================================================

// ea: 0x00595900
FrontEndMenuSystem::FrontEndMenuSystem()
    : FEMenuSystem(31, FONT_GARAMOND)
{
    mPreviousWidescreen = 0;
    FEMenuSystem::Add((FEMenu*)CreateSessionMenu_ctor(
        mem_heap_malloc(16, 0x158u), this));
    FEMenuSystem::Add((FEMenu*)CreateLanSessionMenu_ctor(
        mem_heap_malloc(16, 0x138u), this));
    FEMenuSystem::Add((FEMenu*)CreateSessionAdvancedMenu_ctor(
        mem_heap_malloc(16, 0x120u), this));
    FEMenuSystem::Add((FEMenu*)CreateLanSessionAdvancedMenu_ctor(
        mem_heap_malloc(16, 0x120u), this));
    FEMenuSystem::Add((FEMenu*)FindSessionMenu_ctor(
        mem_heap_malloc(16, 0x10Cu), this));
    FEMenuSystem::Add((FEMenu*)FindLanSessionMenu_ctor(
        mem_heap_malloc(16, 0x104u), this));
    FEMenuSystem::Add((FEMenu*)InitialLoadingMenu_ctor(
        mem_heap_malloc(16, 0x50u), this));
    FEMenuSystem::Add((FEMenu*)InstantActionMenu_ctor(
        mem_heap_malloc(16, 0x4Cu), this));
    FEMenuSystem::Add((FEMenu*)MPMainMenuXBox_ctor(
        mem_heap_malloc(16, 0x160u), this));
    FEMenuSystem::Add((FEMenu*)PlayLanMenu_ctor(
        mem_heap_malloc(16, 0x144u), this));
    FEMenuSystem::Add((FEMenu*)PlayOnlineMenu_ctor(
        mem_heap_malloc(16, 0x60u), this));
    FEMenuSystem::Add((FEMenu*)PressStartMenu_ctor(
        mem_heap_malloc(16, 0x4Cu), this));
    FEMenuSystem::Add((FEMenu*)SessionDetailsMenu_ctor(
        mem_heap_malloc(16, 0x54u), this));
    FEMenuSystem::Add((FEMenu*)SessionListMenu_ctor(
        mem_heap_malloc(16, 0x1B4u), this));
    FEMenuSystem::Add((FEMenu*)SessionLanListMenu_ctor(
        mem_heap_malloc(16, 0x1B4u), this));
    FEMenuSystem::Add((FEMenu*)JoinGameMenu_ctor(
        mem_heap_malloc(16, 0x50u), this));
    FEMenuSystem::Add((FEMenu*)OverlayMenu_ctor(
        mem_heap_malloc(16, 0x148u), this, 1));
    FEMenuSystem::Add((FEMenu*)MultilineOverlayMenu_ctor(
        mem_heap_malloc(16, 0x78u), this));
    FEMenuSystem::Add((FEMenu*)VKMenu_ctor(
        mem_heap_malloc(16, 0x1C8u), this));
    FEMenuSystem::Add((FEMenu*)MPOptionsGameplayMenu_ctor(
        mem_heap_malloc(16, 0x64u), this));
    FEMenuSystem::Add((FEMenu*)MPOptionsControlsMenu_ctor(
        mem_heap_malloc(16, 0x64u), this));
    FEMenuSystem::Add((FEMenu*)MPOptionsScreenMenu_ctor(
        mem_heap_malloc(16, 0x64u), this));
    FEMenuSystem::Add((FEMenu*)MPOptionsSoundMenu_ctor(
        mem_heap_malloc(16, 0x70u), this));
    FEMenuSystem::Add((FEMenu*)MPOptionsPreferencesMenu_ctor(
        mem_heap_malloc(16, 0xA4u), this));
    FEMenuSystem::Add((FEMenu*)OptionsStickMenu_ctor(
        mem_heap_malloc(16, 0x90u), this));
    FEMenuSystem::Add((FEMenu*)OptionsButtonMenu_ctor(
        mem_heap_malloc(16, 0x18Cu), this));
    FEMenuSystem::Add((FEMenu*)GammaScreenMenu_ctor(
        mem_heap_malloc(16, 0x68u), this));
    FEMenuSystem::Add((FEMenu*)MPProfileMainMenu_ctor(
        mem_heap_malloc(16, 0x8Cu), this));
    FEMenuSystem::Add((FEMenu*)MemCardCheckMenu_ctor(
        mem_heap_malloc(16, 0x58u), this));
    FEMenuSystem::Add((FEMenu*)MPProfileEditMenu_ctor(
        mem_heap_malloc(16, 0x110u), this));
    FEMenuSystem::Add((FEMenu*)XboxLiveOptionsMenu_ctor(
        mem_heap_malloc(16, 0x188u), this));
    if (count != 31)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\FrontEndMenuSystem.cpp";
        AeAssert::gCurrentLine = 474;
        AeAssert::gCurrentExpr = "count == NUM_MENUS";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "Mismatch on total number of menu indexes and total number "
                "of menus added."))
            __debugbreak();
    }
    nglInitQuad((nglQuad*)back_top);
    nglInitQuad((nglQuad*)back_bottom);
    nglSetQuadRect((nglQuad*)back_top, 0.0f, 0.0f, 640.0f, 320.0f);
    nglSetQuadRect((nglQuad*)back_bottom, 0.0f, 320.0f, 640.0f, 480.0f);
    textures_are_good = false;
    need_to_play_movies = true;
    drawYButton = false;
    lastLoginCode = 0;
    lastLiveState = 0;
    yButtonText = nullptr;
    UpdateButtonDown();
    for (int v1 = 0; v1 < count; ++v1)
        menus[v1]->Init();
    nglSetQuadZ((nglQuad*)back_top, 300.0f);
    nglSetQuadZ((nglQuad*)back_bottom, 300.0f);
}

// ea: 0x0057EAA0
FrontEndMenuSystem::~FrontEndMenuSystem()
{
    if (movie_manager::theMovie != nullptr)
        movie_manager::movie_done(true);
    g_femanager.menuMovieRunning = false;
    if (yButtonText != nullptr)
        delete yButtonText;
    FEMenuSystem::~FEMenuSystem();
}

// ea: 0x005721C0
void FrontEndMenuSystem::SetInitialMenu()
{
}

// ea: 0x005721D0
void FrontEndMenuSystem::CheckIfSignedInXBox()
{
    MPLiveEngine* Handle = MPLiveEngine::GetHandle();
    if (loginText != nullptr)
    {
        int internalState = Handle->internalState;
        if (internalState != lastLiveState
            || Handle->lastLoginCode != lastLoginCode)
        {
            lastLiveState = internalState;
            int lastLoginCode = Handle->lastLoginCode;
            this->lastLoginCode = lastLoginCode;
            if (internalState != 0)
            {
                int v5 = internalState - 1;
                if (v5 != 0)
                {
                    if (v5 == 1)
                    {
                        MPLiveEngine* v6 = MPLiveEngine::GetHandle();
                        LivePlayer* LocalPlayer =
                            LiveWrapper::theWrapper->GetLocalPlayer(
                                v6->actualPort);
                        char narrowName[16];
                        sprintf(narrowName, "%S", LocalPlayer->gamertag);
                        const char* STBString =
                            STBManager::sInst->GetSTBString(
                                "MPFRONTEND_SIGNED_IN_AS");
                        Broc::string loginString =
                            Broc::string(STBString) + narrowName;
                        loginText->SetTextNoLocalize(
                            loginString.mBlock != nullptr
                                ? (const char*)&loginString.mBlock[1]
                                : defaultFileName);
                    }
                }
                else
                {
                    loginText->SetText("MPFRONTEND_SIGNING_ON");
                }
            }
            else
            {
                if (lastLoginCode != -2146103168)
                {
                    if (lastLoginCode == -2146103166)
                    {
                        loginText->SetText(
                            "MPFRONTEND_PASSCODE_NEEDED");
                        return;
                    }
                    if (lastLoginCode != 0)
                    {
                        loginText->SetText(
                            "MPFRONTEND_SIGNIN_FAILED");
                        return;
                    }
                }
                loginText->SetText("MPFRONTEND_NOT_SIGNED_IN");
            }
        }
    }
}

// ea: 0x00572370
void FrontEndMenuSystem::CheckIfSignedIn()
{
    CheckIfSignedInXBox();
}

// ea: 0x00572380
void FrontEndMenuSystem::PlayBGMusic()
{
    MusicMgr::sInst->Play("music_mainmenu");
}

// ea: 0x005723A0
void FrontEndMenuSystem::PanelFileUnloaded(PanelFile* pf)
{
    (void)pf;
    panelFile = nullptr;
    loginText = nullptr;
}

// ea: 0x0057EB10
void FrontEndMenuSystem::Draw()
{
    if (panelFile != nullptr)
        panelFile->Draw();
    if (background >= 0)
    {
        drawHelpbar = false;
        menus[background]->Draw();
        drawHelpbar = true;
    }
    if (m_active >= 0)
    {
        FEMenu* v5 = menus[m_active];
        if (v5 != nullptr)
            v5->Draw();
    }
}

// ea: 0x0057EB60
void FrontEndMenuSystem::Update(float time_inc)
{
    int integer = cg_widescreen.integer;
    if (mPreviousWidescreen != cg_widescreen.integer)
    {
        mPreviousWidescreen = cg_widescreen.integer;
        bool ws = integer != 0;
        for (int i = 0; i < 31; ++i)
            menus[i]->UpdateWidescreen(ws);
        if (currCl != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
            AeAssert::gCurrentLine = 147;
            AeAssert::gCurrentExpr = "client >= 0 && client < 1";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Invalid client index for dms"))
                __debugbreak();
        }
        DialogMenuDisplay* mDisplay =
            g_femanager.mDMS[0]->mDisplay;
        mDisplay->UpdateWidescreen(ws);
    }
    CheckIfSignedInXBox();
    if (!need_to_play_movies || gSkipFrontEnd)
    {
        if (m_active >= 0)
        {
            FEMenu* v10 = menus[m_active];
            if (v10 != nullptr)
            {
                v10->Update(time_inc);
                UpdateButtonPresses();
            }
        }
        if (!textures_are_good)
        {
            nglTexture* TextureData =
                GetTextureData("main_back_top", 0, "mp_frontEnd");
            nglTexture* v12 =
                GetTextureData("main_back_bottom", 0, "mp_frontEnd");
            if (TextureData != nglDefaultTex && v12 != nglDefaultTex)
            {
                nglSetQuadTex((nglQuad*)back_top, TextureData);
                nglSetQuadTex((nglQuad*)back_bottom, v12);
                textures_are_good = true;
            }
        }
        g_femanager.legalMoviesFinished = true;
    }
    else
    {
        MusicMgr::sInst->Play("music_mainmenu");
        g_femanager.legalMoviesFinished = true;
        if (cls.state != CA_LOADING)
        {
            int ReturnMenu = MPUIInterface::GetReturnMenu();
            MakeActive(ReturnMenu);
            need_to_play_movies = false;
            return;
        }
    }
    need_to_play_movies = false;
}

// ea: 0x0057ED10
void FrontEndMenuSystem::SetPanelFile(PanelFile* pf)
{
    panelFile = pf;
    if (pf != nullptr)
    {
        loginText = pf->GetTextPointer("login_text");
        if (loginText != nullptr)
        {
            loginText->SetText("MPFRONTEND_NOT_SIGNED_IN");
            loginText->SetNoFlash(color32(0xFFD6C8EAu));
        }
    }
}

// ============================================================================
// InGameMenuSystem
// ============================================================================

// ea: 0x005960B0
InGameMenuSystem::InGameMenuSystem(int client)
    : FEMenuSystem(16, FONT_GARAMOND)
{
    mClient = client;
    mPreviousWidescreen = false;
    mHotJoinPort = -1;
    FEMenuSystem::Add((FEMenu*)PauseMenu_ctor(
        mem_heap_malloc(16, 0x6Cu), this));
    FEMenuSystem::Add((FEMenu*)WeaponSelectMenu_ctor(
        mem_heap_malloc(16, 0x1F0u), this));
    FEMenuSystem::Add((FEMenu*)GameSettingsEdit_ctor(
        mem_heap_malloc(16, 0x12Cu), this));
    FEMenuSystem::Add((FEMenu*)GameSettingsView_ctor(
        mem_heap_malloc(16, 0x11Cu), this));
    FEMenuSystem::Add((FEMenu*)LoadingMenu_ctor(
        mem_heap_malloc(16, 0xF8u), this));
    FEMenuSystem::Add((FEMenu*)XBoxLiveIngameOptionsCOD3_ctor(
        mem_heap_malloc(16, 0x164u), this));
    FEMenuSystem::Add((FEMenu*)VoteMapMenu_ctor(
        mem_heap_malloc(16, 0x54u), this));
    FEMenuSystem::Add((FEMenu*)VoteGameTypeMenu_ctor(
        mem_heap_malloc(16, 0x54u), this));
    FEMenuSystem::Add((FEMenu*)InGameOptionsMenu_ctor(
        mem_heap_malloc(16, 0x11Cu), this));
    FEMenuSystem::Add((FEMenu*)MultilineOverlayMenu_ctor(
        mem_heap_malloc(16, 0x6Cu), this));
    FEMenuSystem::Add((FEMenu*)InGameScoreBoard_ctor(
        mem_heap_malloc(16, 0x3E0u), this));
    FEMenuSystem::Add((FEMenu*)InGameSwitchSides_ctor(
        mem_heap_malloc(16, 0x120u), this));
    FEMenuSystem::Add((FEMenu*)SpectateMenu_ctor(
        mem_heap_malloc(16, 0x70u), this));
    FEMenuSystem::Add((FEMenu*)InGameOverlay_ctor(
        mem_heap_malloc(16, 0x12Cu), this));
    FEMenuSystem::Add((FEMenu*)HotJoinMenu_ctor(
        mem_heap_malloc(16, 0x58u), this));
    FEMenuSystem::Add((FEMenu*)InGameLiveOptionsMenu_ctor(
        mem_heap_malloc(16, 0x60u), this));
    UpdateButtonDown();
    for (int i = 0; i < count; ++i)
        menus[i]->Init();
}

// ea: 0x005B8CD0
InGameMenuSystem::~InGameMenuSystem()
{
    FEMenuSystem::~FEMenuSystem();
}

// ea: 0x00572FD0
void InGameMenuSystem::ActivateMenu(int menu)
{
    int mClient = this->mClient;
    is_active = true;
    GamePause::SetGamePaused(mClient, true);
    if (m_active != 12)
    {
        g_doFullScreenBlur[mClient] = FULLSCREENBLUR_THISFRAMEONLY;
        g_fullScreenBlurAmount[mClient] = blur_amount;
    }
    MakeActive(menu, -1);
}

// ea: 0x00573030
void InGameMenuSystem::ActivateHotJoinMenu()
{
    is_active = true;
    MakeActive(14);
    int v3 = unk_F6A28C[802 * mClient];
    controller* v4 = controller::inst();
    for (int i = controller::LEFTBUTTON; i < 16; ++i)
        v4->button_pressed_clear(v3, (controller::ButtonIndex)i);
}

// ea: 0x00573070
void InGameMenuSystem::ActivatePauseMenu()
{
    int mClient = this->mClient;
    is_active = true;
    GamePause::SetGamePaused(mClient, true);
    if (m_active != 12)
    {
        g_doFullScreenBlur[mClient] = FULLSCREENBLUR_THISFRAMEONLY;
        g_fullScreenBlurAmount[mClient] = blur_amount_0;
    }
    MakeActive(0, -1);
}

// ea: 0x005730C0
void InGameMenuSystem::Draw()
{
    View::SetViewportClipping(mClient);
    if (background >= 0)
    {
        drawHelpbar = false;
        menus[background]->Draw();
        drawHelpbar = true;
    }
    if (m_active >= 0)
    {
        FEMenu* v5 = menus[m_active];
        if (v5 != nullptr)
            v5->Draw();
    }
}

// ea: 0x00573110
void InGameMenuSystem::Update(float time_inc)
{
    if (m_active >= 0)
    {
        FEMenu* v10 = menus[m_active];
        if (v10 != nullptr)
        {
            v10->Update(time_inc);
            UpdateButtonPresses();
        }
    }
    if (mPreviousWidescreen != (cg_widescreen.integer != 0))
    {
        UpdateWidescreen(cg_widescreen.integer != 0);
        mPreviousWidescreen = cg_widescreen.integer != 0;
    }
    if (m_active != 12)
    {
        g_doFullScreenBlur[mClient] = FULLSCREENBLUR_THISFRAMEONLY;
        g_fullScreenBlurAmount[mClient] = blur_amount_1;
    }
}

// ea: 0x005731A0
bool InGameMenuSystem::GetAnalogPressed(int button, int* p_controller)
{
    int v3 = unk_F6A28C[802 * currCl];
    int x = 0;
    int y = 0;
    controller* v4 = controller::inst();
    v4->stick_value(v3, controller::LEFTSTICK, x, y);
    controller* v5 = controller::inst();
    if (v5->button_pressed(v3, controller::UPBUTTON))
        y = -128;
    controller* v6 = controller::inst();
    if (v6->button_pressed(v3, controller::DOWNBUTTON))
        y = 128;
    controller* v7 = controller::inst();
    if (v7->button_pressed(v3, controller::LEFTBUTTON))
        x = -128;
    controller* v8 = controller::inst();
    bool v9 = v8->button_pressed(v3, controller::RIGHTBUTTON);
    int v10 = 128;
    if (!v9)
        v10 = x;
    bool result = false;
    switch (button)
    {
    case 4:
        result = y < -64;
        break;
    case 8:
        result = y >= 64;
        break;
    case 16:
        result = v10 < -64;
        break;
    case 32:
        result = v10 >= 64;
        break;
    default:
        result = false;
        break;
    }
    if (result && p_controller != nullptr)
        *p_controller = v3;
    return result;
}

// ea: 0x005732D0
void InGameMenuSystem::UpdateSplitScreen()
{
    for (int i = 0; i < count; ++i)
        menus[i]->UpdateSplitScreen();
}

// ea: 0x00573300
bool InGameMenuSystem::GetButtonPressed(int button, int* p_controller)
{
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * currCl];
    int v5 = unk_F6A28C[802 * currCl];
    controller* v3 = controller::inst();
    return v3->button_pressed(v5, (controller::ButtonIndex)button);
}

// ea: 0x00573350
int InGameMenuSystem::GetStickValueX(int stick, int* p_controller)
{
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * currCl];
    int v5 = unk_F6A28C[802 * currCl];
    controller* v3 = controller::inst();
    return v3->stick_value_x(v5, (controller::StickIndex)stick);
}

// ea: 0x005733A0
int InGameMenuSystem::GetStickValueY(int stick, int* p_controller)
{
    if (p_controller != nullptr)
        *p_controller = unk_F6A28C[802 * currCl];
    int v5 = unk_F6A28C[802 * currCl];
    controller* v3 = controller::inst();
    return v3->stick_value_y(v5, (controller::StickIndex)stick);
}

// ea: 0x005733F0
int InGameMenuSystem::GetCurrentClient()
{
    return mClient;
}

// ea: 0x00573400
int InGameMenuSystem::GetCurrentClientController()
{
    return unk_F6A28C[802 * mClient];
}

// ea: 0x00573410
int InGameMenuSystem::GetClientFromController(int c)
{
    (void)c;
    return mClient;
}

// ea: 0x00573420
void InGameMenuSystem::CheckForNoMenus()
{
    if (GetActiveMenu() <= -1)
    {
        int mClient = this->mClient;
        is_active = false;
        GamePause::SetGamePaused(mClient, false);
    }
}

// ea: 0x00573450
void InGameMenuSystem::UpdateWidescreen(bool widescreen)
{
    for (int i = 0; i < 16; ++i)
        menus[i]->UpdateWidescreen(widescreen);
}

// ea: 0x0057F330
void InGameMenuSystem::NewMenuActive()
{
    if (GetActiveMenu() <= -1)
    {
        int mClient = this->mClient;
        is_active = false;
        GamePause::SetGamePaused(mClient, false);
    }
}

// ea: 0x0057F360
bool InGameMenuSystem::GetPanelFileUsers(
    const char* name, ae_sized_array<PanelFileUser*, 12>& array)
{
    int old_size = array.m_size;
    if (_stricmp(name, "MP_PM_GS_edit.PANEL") == 0)
    {
        array.push_back((PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[2]);
    }
    else
    {
        if (_stricmp(name, "MP_PM_GS_view.PANEL") != 0)
        {
            if (strcmp(name, "MP_PM_mainmenu.PANEL") == 0)
            {
                array.push_back(g_femanager.GetIGMS(mClient)->menus[0]);
            }
            else
            {
                if (strcmp(name, "MP_SS_PM_options.PANEL") == 0)
                {
                    InGameMenuSystem* igms =
                        g_femanager.GetIGMS(mClient);
                    array.push_back(igms->menus[0]);
                    array.push_back(igms->menus[1]);
                    array.push_back(igms->menus[11]);
                    return old_size != array.m_size;
                }
                if (strcmp(name, "MP_SS_PM_background.PANEL") == 0)
                {
                    array.push_back((PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[0]);
                    return old_size != array.m_size;
                }
                if (_stricmp(name, "MP_class_select.panel") == 0)
                {
                    array.push_back(
                        (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[1]);
                    return old_size != array.m_size;
                }
                if (strcmp(name, "MP_PM_controller.PANEL") == 0)
                {
                    array.push_back(
                        (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[8]);
                    return old_size != array.m_size;
                }
                PanelFileUser* v6;
                if (strcmp(name, "MP_SS_PM_options_edit.PANEL") == 0)
                {
                    array.push_back(
                        (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[8]);
                    v6 = (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[2];
                }
                else
                {
                    if (strcmp(name, "MP_SS_PM_options_view.PANEL") == 0)
                    {
                        array.push_back(
                            (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[3]);
                        return old_size != array.m_size;
                    }
                    if (strcmp(name, "MP_spectator.panel") == 0)
                    {
                        array.push_back(
                            (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[12]);
                        return old_size != array.m_size;
                    }
                    if (strcmp(name, "SP_small_textbox_ingame.PANEL") == 0)
                    {
                        array.push_back(
                            (PanelFileUser*)DialogMenuDisplay::Me(mClient));
                        v6 = (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[13];
                    }
                    else if (strcmp(name, "MP_SS_PM_textbox.PANEL") == 0)
                    {
                        array.push_back(
                            (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[14]);
                        v6 = (PanelFileUser*)DialogMenuDisplay::Me(mClient);
                    }
                    else if (_stricmp(name,
                                      "MP_ingame_scoreboard.panel") == 0)
                    {
                        v6 = (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[10];
                    }
                    else
                    {
                        const char* v7 = name;
                        if (_stricmp(name,
                                     "MP_PM_sideselection.PANEL") == 0)
                        {
                            v6 = (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[11];
                        }
                        else
                        {
                            if (_stricmp(v7, "MP_PM_xblive.PANEL") != 0)
                                return old_size != array.m_size;
                            v6 = (PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[5];
                        }
                    }
                }
                array.push_back(v6);
                return old_size != array.m_size;
            }
        }
        else
        {
            array.push_back((PanelFileUser*)g_femanager.GetIGMS(mClient)->menus[3]);
        }
    }
    return old_size != array.m_size;
}

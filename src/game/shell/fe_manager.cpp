// ============================================================================
// fe_manager.cpp - FEManager (shell.o FEManager.cpp)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/platform_xbox/MemoryUnitManager.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_scene.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/nglFont.h"
#include "ngl/nglTexture.h"

#include <string.h>
#include <intrin.h>

extern FEManager g_femanager;          // ?g_femanager@@3UFEManager@@A
extern int currCl;                     // ?currCl@@3HA @ 0xF1579C
extern const char defaultFileName[];  // ?defaultFileName
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);  // core.o
extern ELanguage gLanguage;            // ?gLanguage@@3W4ELanguage@@A
extern nglScene* nglBuildScene;        // ?nglBuildScene@@3PAUnglScene@@A
bool onlyOnce = false;                 // ?onlyOnce @ 0xF3A4C8
extern void codNflUpdate();            // g_entity_misc.cpp
extern const char* GetLanguageId(ELanguage l);  // ?GetLanguageId@@YAPBDW4ELanguage@@@Z
extern void GetXboxLanguage();         // fe_util.cpp
extern nglTexture* nglGetTexture(const tlFixedString& FileName);  // ngl_texture.cpp
extern void* mem_heap_malloc(unsigned int size);  // core.o (1-arg overload)
extern bool gMPLoadingUnthreaded;      // ?gMPLoadingUnthreaded@@3_NA
namespace AeStringSupport {
extern void Concat(char* dst, int* dstLen, int dstCapacity,
                   const char* src);  // core.o
}

// STBManager minimal view
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);  // core.o
};

// SpinnerDrawFrame (spinner_lens.cpp)
extern void SpinnerDrawFrame(bool bEndFrame);

namespace View {
extern int GetNumViewports();  // ?GetNumViewports@View@@YAHXZ
}

// IDA's FEManager object is the full 0x3F4-byte type.  Defining the global
// with that type keeps currCl from being placed inside its font fields.
FEManager g_femanager;

void FEManager_InitDialogMenuSystem(void* self)
{
    static_cast<FEManager*>(self)->InitDialogMenuSystem();
}

void FEManager_InitIGO(void* self)
{
    static_cast<FEManager*>(self)->InitIGO();
}

void FEManager_LoadInGameMenus(void* self)
{
    static_cast<FEManager*>(self)->LoadInGameMenus();
}

namespace LocalClient {
extern int FirstLocalClientIndex();  // ?FirstLocalClientIndex@LocalClient@@YAHXZ
extern int PortToClient(int port);   // ?PortToClient@LocalClient@@YAHH@Z
}

// Menu class externs (mp.o / game_xbox.o; unresolved until those land)
class InstantActionMenu;
class MPMainMenuXBox;
class CreateSessionMenu;
class CreateLanSessionMenu;
class FindSessionMenu;
class FindLanSessionMenu;
class InitialLoadingMenu;
class PlayLanMenu;
class PlayOnlineMenu;
class PressStartMenu;
class SessionDetailsMenu;
class SessionListMenu;
class SessionLanListMenu;
class OverlayMenu;
class MultilineFrontendOverlayMenu;
class MultilineIngameOverlayMenu;
class LoadingMenu;
class JoinGameMenu;
class CreateSessionAdvancedMenu;
class CreateLanSessionAdvancedMenu;
class XboxLiveOptionsMenu;
class MPOptionsGameplayMenu;
class MPOptionsControlsMenu;
class MPOptionsScreenMenu;
class MPOptionsSoundMenu;
class MPOptionsPreferencesMenu;
class MPProfileMainMenu;
class MPProfileEditMenu;
class VKMenu;

extern InstantActionMenu* InstantActionMenu_Me();
extern MPMainMenuXBox* MPMainMenuXBox_Me();
extern CreateSessionMenu* CreateSessionMenu_Me();
extern CreateLanSessionMenu* CreateLanSessionMenu_Me();
extern FindSessionMenu* FindSessionMenu_Me();
extern FindLanSessionMenu* FindLanSessionMenu_Me();
extern InitialLoadingMenu* InitialLoadingMenu_Me();
extern PlayLanMenu* PlayLanMenu_Me();
extern PlayOnlineMenu* PlayOnlineMenu_Me();
extern PressStartMenu* PressStartMenu_Me();
extern SessionDetailsMenu* SessionDetailsMenu_Me();
extern SessionListMenu* SessionListMenu_Me();
extern SessionLanListMenu* SessionLanListMenu_Me();
extern OverlayMenu* OverlayMenu_Me(int version);
extern MultilineFrontendOverlayMenu* MultilineFrontendOverlayMenu_Me();
extern MultilineIngameOverlayMenu* MultilineIngameOverlayMenu_Me();
extern LoadingMenu* LoadingMenu_Me();
extern JoinGameMenu* JoinGameMenu_Me();
extern CreateSessionAdvancedMenu* CreateSessionAdvancedMenu_Me();
extern CreateLanSessionAdvancedMenu* CreateLanSessionAdvancedMenu_Me();
extern XboxLiveOptionsMenu* XboxLiveOptionsMenu_Me();
extern MPOptionsGameplayMenu* MPOptionsGameplayMenu_Me();
extern MPOptionsControlsMenu* MPOptionsControlsMenu_Me();
extern MPOptionsScreenMenu* MPOptionsScreenMenu_Me();
extern MPOptionsSoundMenu* MPOptionsSoundMenu_Me();
extern MPOptionsPreferencesMenu* MPOptionsPreferencesMenu_Me();
extern MPProfileMainMenu* MPProfileMainMenu_Me();
extern MPProfileEditMenu* MPProfileEditMenu_Me();
extern AARMenuSystem* AARMenuSystem_ctor(void* mem);
extern DialogMenuSystem* DialogMenuSystem_ctor(void* mem, int client);

// ============================================================================
// FEManager
// ============================================================================

// ea: 0x00593C90
FEManager::FEManager()
{
    fems = nullptr;
    mDMS[0] = nullptr;
    mIGMS[0] = nullptr;
    start_on = false;
    default_pq = nullptr;
    pause_menu_timer = 0.0f;
    mAARS = nullptr;
    IGO_active = false;
    saveTime = 0.0f;
    forceMovieExit = false;
    debug_mode = true;
    menuMovieRunning = false;
    legalMoviesFinished = false;
    skipAllLegalMovies = false;
    skipAllMovies = false;
    skipFE = false;
    mDontDrawHud = false;
    renderMovieOnly = false;
    enablePause = true;
    loadLevel[0] = 0;
    inGame = false;
    for (int v2 = 0; v2 < 4; ++v2)
    {
        fonts[v2] = nullptr;
        fontsLoaded[v2] = false;
    }
    ControllerDisconnectedMenu* v4 =
        (ControllerDisconnectedMenu*)mem_heap_malloc(16, 0x50u);
    if (v4 != nullptr)
    {
        ControllerDisconnected = new (v4) ControllerDisconnectedMenu();
    }
    else
    {
        ControllerDisconnected = nullptr;
    }
    mNumPanels = 0;
}

// ea: 0x0056F040
FEManager::~FEManager()
{
    if (fems != nullptr)
        delete fems;
    if (mDMS[0] != nullptr)
        delete mDMS[0];
    if (mIGMS[0] != nullptr)
        delete mIGMS[0];
    if (IGO != nullptr)
        delete IGO;
    if (mAARS != nullptr)
        delete mAARS;
    mAARS = nullptr;
    if (ControllerDisconnected != nullptr)
        delete ControllerDisconnected;
    if (default_pq != nullptr)
        delete default_pq;
}

// ea: 0x0056F0C0
void FEManager::LoadFonts()
{
}

// ea: 0x0056F0D0
void FEManager::ReleaseFonts()
{
}

// ea: 0x0056F0E0
void FEManager::ReleaseFont(font_index f)
{
    (void)f;
}

// ea: 0x0057D720
void FEManager::LoadFont(font_index i)
{
    tlFixedString FileName((const char*)font_name_array[i].mBuff);
    nglGetTexture(FileName);
    tlFixedString v3((const char*)font_name_array[i].mBuff);
    fonts[i] = nglGetFont(v3);
}

// ea: 0x0056FA70
nglFont* FEManager::GetFont(font_index f)
{
    if (f != FONT_BIG && f != FONT_NORMAL)
        return fonts[f];
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
    AeAssert::gCurrentLine = 1303;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
        __debugbreak();
    return nullptr;
}

// ea: 0x00585490
nglFont* FEManager::GetFont(font_index f, float scale)
{
    (void)scale;
    if (f == FONT_NORMAL)
    {
        font_index Font = FindFont("i_helvetica_bold", false);
        nglFont* result = GetFont(Font);
        if (result != nullptr)
            return result;
    }
    font_index v6 = FindFont("i_helvetica_bold", false);
    return GetFont(v6);
}

// ea: 0x00585260
void FEManager::UpdateButtonFontForLanguage()
{
    if (!onlyOnce)
    {
        onlyOnce = true;
        if (font_name_array[1].mLength < 0x1Eu)
        {
            font_name_array[1].mBuff[font_name_array[1].mLength++] = 95;
            font_name_array[1].mBuff[font_name_array[1].mLength] = 0;
        }
        const char* LanguageId = GetLanguageId(gLanguage);
        int dstLen = font_name_array[1].mLength;
        AeStringSupport::Concat((char*)font_name_array[1].mBuff, &dstLen,
                                31, LanguageId);
        font_name_array[1].mLength = (unsigned char)dstLen;
    }
}

// ea: 0x00585450
void FEManager::ReleaseFrontEnd()
{
    if (fems != nullptr)
        delete fems;
    ProfileManager* mProfileManager = this->mProfileManager;
    fems = nullptr;
    if (mProfileManager != nullptr)
    {
        mProfileManager->~ProfileManager();
        mem_heap_free(mProfileManager);
    }
    mProfileManager = nullptr;
}

// ea: 0x005854D0
void FEManager::GetPanelFileUsers(
    const char* name, ae_sized_array<PanelFileUser*, 12>& array)
{
    const char* v3 = name;
    bool PanelFileUsers = mAARS->GetPanelFileUsers(v3, array);
    bool v8 = mIGMS[0]->GetPanelFileUsers(v3, array);
    if ((v8 | PanelFileUsers) == 0)
    {
        if (strcmp(v3, "instant_action.panel") == 0)
        {
            array.push_back((PanelFileUser*)fems->menus[7]);
            return;
        }
        if (strcmp(v3, "MP_pre_mainmenu.PANEL") == 0)
        {
            array.push_back((PanelFileUser*)fems->menus[8]);
            return;
        }
        if (strcmp(v3, "MP_creategame.PANEL") == 0)
        {
            array.push_back((PanelFileUser*)fems->menus[0]);
            return;
        }
        if (strcmp(v3, "MP_Xbox_system_link_CG.PANEL") == 0)
        {
                array.push_back((PanelFileUser*)fems->menus[1]);
            return;
        }
        if (strcmp(v3, "MP_SS_creatematch.PANEL") != 0)
        {
            if (strcmp(v3, "MP_findgame_optimatch.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[4]);
                return;
            }
            if (strcmp(v3, "MP_Xbox_system_link_FG.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[5]);
                return;
            }
            if (strcmp(v3, "initial_loading_screen.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[6]);
                return;
            }
            if (strcmp(v3, "MP_Xbox_system_link.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[9]);
                return;
            }
            if (strcmp(v3, "MP_mainmenu.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[10]);
                return;
            }
            if (strcmp(v3, "press_start.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[11]);
                return;
            }
            if (strcmp(v3, "session_details.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[12]);
                return;
            }
            if (strcmp(v3, "MP_gamelist.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[13]);
                return;
            }
            if (_stricmp(v3, "MP_Xbox_system_link_GL.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[14]);
                return;
            }
            if (strcmp(v3, "SP_small_textbox_frontend.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[16]);
                return;
            }
            if (strcmp(v3, "multiline_frontend_overlay.panel") == 0)
            {
                array.push_back(
                    (PanelFileUser*)fems->menus[17]);
                return;
            }
            if (strcmp(v3, "multiline_ingame_overlay.panel") == 0)
            {
                array.push_back(
                    (PanelFileUser*)mIGMS[currCl]->menus[9]);
                return;
            }
            if (strcmp(v3, "hud_menu.panel") == 0
                || strcmp(v3, "hud_mp.panel") == 0
                || strcmp(v3, "hud_icons.panel") == 0)
            {
                array.push_back((PanelFileUser*)IGO);
                return;
            }
            if (strcmp(v3, "mp_loadingscreen.panel") == 0)
            {
                array.push_back((PanelFileUser*)mIGMS[currCl]->menus[4]);
                return;
            }
            if (strcmp(v3, "createprofile_menu.panel") == 0)
            {
                array.push_back(g_femanager.fems->menus[18]);
                return;
            }
            if (strcmp(v3, "error_controller_disconnected.panel") == 0)
            {
                array.push_back((PanelFileUser*)ControllerDisconnected);
                return;
            }
            if (strcmp(v3, "MP_Xbox_liveoptions.PANEL") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[30]);
                return;
            }
            if (strcmp(v3, "join_game.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[15]);
            }
            else if (strcmp(v3, "login_status.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems);
            }
            else if (strcmp(v3, "MP_creategame_advanced.PANEL") == 0)
            {
                array.push_back(
                    (PanelFileUser*)fems->menus[2]);
            }
            else if (strcmp(v3, "MP_Xbox_system_link_CGA.PANEL") == 0)
            {
                array.push_back(
                    (PanelFileUser*)fems->menus[3]);
            }
            else if (strcmp(v3, "SP_OP_gameplay.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[19]);
            }
            else if (strcmp(v3, "SP_OP_con_main.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[20]);
            }
            else if (strcmp(v3, "SP_OP_screen.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[21]);
            }
            else if (strcmp(v3, "SP_OP_sound.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[22]);
            }
            else if (strcmp(v3, "SP_OP_pref_QM.panel") == 0)
            {
                array.push_back(
                    (PanelFileUser*)fems->menus[23]);
            }
            else if (strcmp(v3, "SP_OP_con_stick_XB.panel") == 0)
            {
                array.push_back(g_femanager.fems->menus[24]);
            }
            else if (strcmp(v3, "SP_OP_con_button_XB.panel") == 0)
            {
                array.push_back(g_femanager.fems->menus[25]);
            }
            else if (strcmp(v3, "SP_OP_screen_gamma.panel") == 0)
            {
                array.push_back(g_femanager.fems->menus[26]);
            }
            else if (strcmp(v3, "SP_OP_mainmenu.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[27]);
            }
            else if (strcmp(v3, "SP_OP_editprofile.panel") == 0)
            {
                array.push_back((PanelFileUser*)fems->menus[29]);
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
                AeAssert::gCurrentLine = 1843;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid Panel File"))
                    __debugbreak();
            }
        }
    }
}

// ea: 0x005958A0
void FEManager::InitDialogMenuSystem()
{
    mDMS[0] = DialogMenuSystem_ctor(mem_heap_malloc(0x44u), 0);
}

// ea: 0x0059ACD0
void FEManager::LoadFrontEnd()
{
    mProfileManager = nullptr;
    ProfileManager* v2 = (ProfileManager*)mem_heap_malloc(0x6Cu);
    mProfileManager = v2 != nullptr ? new (v2) ProfileManager() : nullptr;
    fems = (FEMenuSystem*)mem_heap_malloc(16, 0x104u);
    if (fems != nullptr)
        new (fems) FrontEndMenuSystem();
    else
        fems = nullptr;
}

// ea: 0x0059AD70
void FEManager::LoadInGameMenus()
{
    InGameMenuSystem* v2 = (InGameMenuSystem*)mem_heap_malloc(16, 0x38u);
    if (v2 != nullptr)
        mIGMS[0] = new (v2) InGameMenuSystem(0);
    else
        mIGMS[0] = nullptr;
    AARMenuSystem* v4 = (AARMenuSystem*)mem_heap_malloc(16, 0x30u);
    if (v4 != nullptr)
        mAARS = AARMenuSystem_ctor(v4);
    else
        mAARS = nullptr;
}

// ea: 0x0059D0C0
void FEManager::InitIGO()
{
    IGO = (IGOFrontEnd*)mem_heap_malloc(0xA8u);
    if (IGO != nullptr)
        new (IGO) IGOFrontEnd();
    else
        IGO = nullptr;
}

// ea: 0x004DDB70
InGameMenuSystem* FEManager::GetIGMS(int client)
{
    if (client != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
        AeAssert::gCurrentLine = 148;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index for dms"))
            __debugbreak();
    }
    return mIGMS[client];
}

// ea: 0x004DDAF0
DialogMenuSystem* FEManager::GetDMS(int client)
{
    if (client != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.h";
        AeAssert::gCurrentLine = 147;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid client index for dms"))
            __debugbreak();
    }
    return mDMS[client];
}

// ea: 0x005AEF60 (shell.o)
bool FEManager::DMSMenusActiveAnyClient()
{
    for (DialogMenuSystem** menu = mDMS;
         *menu == nullptr || !(*menu)->IsSystemActive();
         ++menu)
    {
        return false;
    }
    return true;
}

// ea: 0x0057D6D0
void FEManager::UpdateLoadingMenu(float percentDone)
{
    InGameMenuSystem* igms = g_femanager.GetIGMS(currCl);
    int v2 = *(int*)&igms->menus[1];
    if (percentDone > *(float*)(v2 + 132))
        *(float*)(v2 + 132) = percentDone;
    MultiplayerMgr::sInst->Step(0, !gMPLoadingUnthreaded, true);
}

// ea: 0x0057D8A0
void FEManager::DrawDiscError()
{
    if (STBManager::sInst != nullptr && g_femanager.fonts[0] != nullptr)
    {
        const char* STBString = STBManager::sInst->GetSTBString(
            "GAME_DAMAGED_DISC_XBOX_LINEA");
        const char* v2 = STBManager::sInst->GetSTBString(
            "GAME_DAMAGED_DISC_XBOX_LINEB");
        unsigned int y;
        unsigned int x;
        nglGetStringDimensions(g_femanager.fonts[0], &x, &y, STBString);
        x = 640 - x;
        float v4 = (float)x;
        x = (unsigned int)(v4 * 0.5f);
        nglQuad loadImage;
        nglInitQuad(&loadImage);
        nglSetQuadRect(&loadImage, 0.0f, 0.0f, 640.0f, 480.0f);
        nglSetQuadColor(&loadImage, 0);
        nglSetQuadZ(&loadImage, 300.0f);
        for (int i = 2; i != 0; --i)
        {
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(0xF3u);
            nglSetZWriteEnable(false);
            nglListAddQuad(&loadImage);
            nglListEndScene();
            nglPresent();
        }
        nglWaitForRendering();
        for (;;)
        {
            nglSetClearFlags(3u);
            nglListAddQuad(&loadImage);
            nglListAddString(g_femanager.fonts[0], (float)x, 200.0f, 0.0f,
                             0xE0FFFFFF, 0.5f, 0.5f, STBString);
            nglListAddString(g_femanager.fonts[0], (float)x, 230.0f, 0.0f,
                             0xE0FFFFFF, 0.5f, 0.5f, v2);
            nglPresent();
        }
    }
    DrawDebugDiscError();
}

// ea: 0x0056F0F0
void FEManager::UpdateFrontEnd(float time_inc)
{
    if (mDMS[0] != nullptr && DMSMenusActiveAnyClient())
    {
        mDMS[0]->Update(time_inc);
        if (fems != nullptr && fems->GetActiveMenu() >= 0
            && fems->GetActiveMenu() < fems->size)
        {
            fems->menus[fems->GetActiveMenu()]->InputLock(true);
        }
    }
    else
    {
        if (fems != nullptr && fems->GetActiveMenu() >= 0
            && fems->GetActiveMenu() < fems->size)
        {
            fems->menus[fems->GetActiveMenu()]->InputLock(false);
        }
        renderMovieOnly = false;
    }
    if (fems != nullptr)
        fems->Update(time_inc);
}

// ea: 0x0056F1B0
void FEManager::UpdateAARMenus(float time_inc)
{
    if (mAARS != nullptr && mAARS->IsSystemActive()
        && currCl == LocalClient::FirstLocalClientIndex())
    {
        controller* v4 = controller::inst();
        unsigned int v5 = (unsigned int)LocalClient::PortToClient(
            v4->locked_port);
        unsigned int v6 = v5;
        if (v5 <= 1 && mDMS[v5] != nullptr && mDMS[v5]->IsSystemActive())
            mDMS[v6]->Update(time_inc);
        else
            mAARS->Update(time_inc);
    }
}

// ea: 0x0056F230
void FEManager::UpdateInGameMenus(float time_inc)
{
    DialogMenuSystem* v3 = mDMS[currCl];
    if (v3 != nullptr && v3->IsSystemActive())
    {
        mDMS[currCl]->Update(time_inc);
    }
    else
    {
        renderMovieOnly = false;
        mIGMS[currCl]->Update(time_inc);
    }
}

// ea: 0x0056F280
void FEManager::DrawIGO(int client)
{
    InGameMenuSystem* v3 = mIGMS[currCl];
    int v4 = client;
    if (v3 == nullptr || !v3->IsSystemActive())
    {
        IGO->DrawHint(v4);
    }
    else
    {
        v4 = client;
        if (mIGMS[client]->GetActiveMenu() == 12)
            IGO->DrawHint(v4);
    }
    nglListBeginScene(NGLSCENE_DEFAULTS);
    nglSetClearFlags(0);
    nglSetZTestEnable(false);
    nglSetZWriteEnable(false);
    DialogMenuSystem* v5 = mDMS[v4];
    if (v5 == nullptr
        || (v5->IsSystemActive() == 0
            && (mIGMS[v4] == nullptr || mIGMS[v4]->IsSystemActive() == 0
                || View::GetNumViewports() != 1)))
    {
        IGO->Draw(v4);
    }
    nglListEndScene();
}

// ea: 0x0056F330
void FEManager::Draw3DWorldSpace()
{
    if (g_femanager.default_pq != nullptr)
        g_femanager.default_pq->SetShown(false);
    IGOFrontEnd* IGO = this->IGO;
    int v3 = currCl;
    IGOHeadIcons* v4 = IGO->mHeadIcons[currCl];
    if (v4 != nullptr)
    {
        v4->Draw();
        v3 = currCl;
    }
    IGOItemIcons* v5 = IGO->mItemIcons[v3];
    if (v5 != nullptr)
        v5->Draw();
}

// ea: 0x0056F3B0
void FEManager::DrawLoadingDots()
{
    SpinnerDrawFrame(false);
}

// ea: 0x0056F440 (Xbox disc-error; structural port)
void FEManager::DrawDebugDiscError()
{
    GetXboxLanguage();
    // XFONT_OpenDefaultFont / D3DDevice_GetRenderTarget2 / XFONT_TextOut are
    // Xbox-only (structural stub per plan).
}

// ea: 0x0056F5F0
void FEManager::PlayFadeInOranScreen()
{
    nglQuad black;
    nglInitQuad(&black);
    nglSetQuadRect(&black, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetQuadColor(&black, 0xFF000000);
    nglSetQuadZ(&black, 300.0f);
    const char* STBString =
        STBManager::sInst->GetSTBString("INGAME_TWO_YEARS");
    FEText* v3 = (FEText*)mem_heap_malloc(0x70u);
    if (v3 != nullptr)
        v3 = new (v3) FEText(FONT_GARAMOND, STBString, 0.0f, 0.0f, 0,
                             PANEL_LAYER_IGO, 1.7f, 16, 0,
                             color32(0xFFD2E8EAu));
    float width = v3->GetWidth(nullptr);
    float v11 = (640.0f - width) * 0.5f;
    v3->SetPos(v11, 460.0f);
    v3->SetAlpha(0);
    unsigned long long v6 = __rdtsc();
    int v8 = (int)v6;
    float totalTime = 0.0f;
    int v7 = 0;
    float visibility = 0.0f;
    for (;;)
    {
        while (1)
        {
            v3->SetAlpha(visibility);
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(0xF3u);
            nglSetZWriteEnable(false);
            nglListAddQuad(&black);
            nglListEndScene();
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(0);
            v3->Draw();
            nglListEndScene();
            nglPresent();
            codNflUpdate();
            unsigned long long v14 = __rdtsc();
            v8 = (int)v14;
            double v10 = (double)(v14 - (unsigned long long)v8)
                             * 0.0000013636364 * 0.001
                         + totalTime;
            totalTime = (float)v10;
            if (totalTime >= 2.0f)
                break;
            visibility = totalTime * 0.5f;
        }
        if (totalTime < 3.0f)
        {
            v7 = 1065353216;
            visibility = 1.0f;
            continue;
        }
        if (totalTime >= 5.0f)
            break;
        visibility = 1.0f - (totalTime - 3.0f) * 0.5f;
    }
}

// ea: 0x0056F820
void FEManager::DrawFrontEnd()
{
    if (nglBuildScene != nullptr)
    {
        if (fems != nullptr && fems->IsSystemActive() && fontsLoaded[0])
        {
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(0);
            nglSetZTestEnable(false);
            nglSetZWriteEnable(false);
            fems->Draw();
            nglListEndScene();
            DialogMenuSystem* v3 = mDMS[0];
            if (v3 != nullptr && v3->IsSystemActive())
                mDMS[0]->Draw();
            else
                renderMovieOnly = false;
        }
    }
}

// ea: 0x0056F8A0
void FEManager::DrawInGameMenus()
{
    if (nglBuildScene != nullptr && fontsLoaded[0])
    {
        InGameMenuSystem* v2 = mIGMS[currCl];
        if (v2 != nullptr && v2->IsSystemActive())
        {
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(3u);
            nglSetZTestEnable(false);
            nglSetZWriteEnable(false);
            mIGMS[currCl]->Draw();
            DialogMenuSystem* v3 = mDMS[currCl];
            if (v3 != nullptr && v3->IsSystemActive())
                mDMS[currCl]->Draw();
            nglListEndScene();
            nglListBeginScene(NGLSCENE_DEFAULTS);
            nglSetClearFlags(3u);
            nglSetZWriteEnable(true);
            nglSetZTestEnable(true);
            mIGMS[currCl]->Draw3D();
            nglListEndScene();
            renderMovieOnly = false;
        }
    }
}

// ea: 0x0056F980
void FEManager::DrawAARMenus()
{
    if (nglBuildScene != nullptr && fontsLoaded[0]
        && mAARS->IsSystemActive())
    {
        nglListBeginScene(NGLSCENE_DEFAULTS);
        nglSetClearFlags(3u);
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        mAARS->Draw();
        controller* v2 = controller::inst();
        unsigned int v3 = (unsigned int)LocalClient::PortToClient(
            v2->locked_port);
        unsigned int v4 = v3;
        if (v3 <= 1)
        {
            DialogMenuSystem* v5 = mDMS[v3];
            if (v5 != nullptr && v5->IsSystemActive())
                mDMS[v4]->Draw();
        }
        nglListEndScene();
    }
}

// ea: 0x0056FA20
void FEManager::PrepareLoadingMenus()
{
}

// ea: 0x0056FA30
void FEManager::ReleaseInGameMenus()
{
    if (mIGMS[0] != nullptr)
        delete mIGMS[0];
    mIGMS[0] = nullptr;
    if (mAARS != nullptr)
        delete mAARS;
    mAARS = nullptr;
}

// ea: 0x0056FAE0
void FEManager::UpdateSplitScreen()
{
    IGO->UpdateSplitScreen();
    mAARS->UpdateSplitScreen();
    mDMS[0]->UpdateSplitScreen();
    mIGMS[0]->UpdateSplitScreen();
}

// ea: 0x0057D770
void FEManager::Draw3DScreenSpace()
{
    IGOCompassWidget* v2 = IGO->compassWidget[currCl];
    if (v2 != nullptr && v2->IsShown())
        IGO->compassWidget[currCl]->Draw3DObjectiveLocations();
}

// ea: 0x0057D7A0
void FEManager::DrawLoadingScreen(int alpha)
{
    const char* STBString =
        STBManager::sInst->GetSTBString("INGAME_LOADING");
    unsigned int y;
    unsigned int x;
    nglGetStringDimensions(g_femanager.fonts[2], &x, &y, STBString);
    x = 640 - x;
    float v4 = (float)x;
    x = (unsigned int)(v4 * 0.5f);
    nglQuad loadImage;
    nglInitQuad(&loadImage);
    nglSetQuadRect(&loadImage, 0.0f, 0.0f, 640.0f, 480.0f);
    nglSetQuadColor(&loadImage, (unsigned int)alpha << 24);
    nglSetQuadZ(&loadImage, 300.0f);
    nglSetQuadBlend(&loadImage, 0x64CF8600u);
    nglSetClearFlags(0xF3u);
    nglListAddQuad(&loadImage);
    nglListAddString(g_femanager.fonts[2], (float)x, 220.0f, 0.0f,
                     0xE0FFFFFF, 1.0f, 1.0f, STBString);
    nglPresent();
}

// ea: 0x0057DA30
void FEManager::DrawControllerError()
{
    if (ControllerDisconnected != nullptr)
    {
        nglListBeginScene(NGLSCENE_DEFAULTS);
        nglSetClearFlags(0xF3u);
        nglSetZTestEnable(false);
        nglSetZWriteEnable(false);
        ControllerDisconnected->Draw();
        nglListEndScene();
        nglPresent();
    }
}

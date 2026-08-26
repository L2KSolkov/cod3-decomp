// ============================================================================
// g_hudelem.cpp - HUD element pool (g.o: g_hudelem.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

// ea: 0x0044B0F0
game_hudelem_s* HudElem_Alloc()
{
    unsigned int v1 = 0;
    while (g_hudelems[v1].elem.type != HE_TYPE_FREE)
    {
        ++v1;
        if (v1 >= 16)
            return nullptr;
    }
    game_hudelem_s* v3 = &g_hudelems[v1];
    HudElem_SetDefaults(v3);
    return v3;
}

// ea: 0x0044B120
void HudElem_UpdateClient(Client* client, int iClientNum)
{
    // The release client update entry point is an intentional null-sub.  Keep
    // the exported call ABI while making the no-state-change contract explicit.
    (void)client;
    (void)iClientNum;
}

// ea: 0x0044CDE0
void Scr_ConstructMessageString(int /*iValue*/, char* /*pszBuffer*/, int /*iSize*/, conMsgType_t /*iType*/)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_main.cpp";
    AeAssert::gCurrentLine = 456;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("ma dead code"))
        __debugbreak();
}

// ea: 0x0044CE30
void Scr_LocalizationError(int iParm, const char* pszErrorMessage)
{
    Scr_ParamError(iParm, pszErrorMessage);
}

// ea: 0x0044FF20
void G_SetEntityScriptVariable(const char* key, const char* value, Entity* ent)
{
    // This multiplayer export is a release no-op; script fields are handled by
    // the Broc field callbacks in g_scr.cpp.
    (void)key;
    (void)value;
    (void)ent;
}

// ea: 0x004504C0
void Scr_FreeHudElem(game_hudelem_s* hud)
{
    // The release script path owns HUD lifetime through HudElem_Free.
    (void)hud;
}

// ea: 0x004504D0
void Scr_AddHudElem(game_hudelem_s* hud)
{
    // The release script path does not maintain a second HUD list.
    (void)hud;
}

// ea: 0x00457610
void HudElem_Free(game_hudelem_s* hud)
{
    if (hud == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_hudelem.cpp";
        AeAssert::gCurrentLine = 129;
        AeAssert::gCurrentExpr = "hud";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((hud - g_hudelems) >= 0x10)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_hudelem.cpp";
        AeAssert::gCurrentLine = 130;
        AeAssert::gCurrentExpr = "hud - g_hudelems >= 0 && hud - g_hudelems < (sizeof(g_hudelems) / sizeof(g_hudelems[0]))";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (hud->elem.type <= HE_TYPE_FREE || hud->elem.type >= HE_TYPE_COUNT)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_hudelem.cpp";
        AeAssert::gCurrentLine = 131;
        AeAssert::gCurrentExpr = "hud->elem.type > HE_TYPE_FREE && hud->elem.type < HE_TYPE_COUNT";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", hud->elem.type))
            __debugbreak();
    }
    hud->elem.type = HE_TYPE_FREE;
}

// ea: 0x00457720
void HudElem_DestroyAll()
{
    game_hudelem_s* v0 = g_hudelems;
    for (int i = 16; i != 0; --i)
    {
        if (v0->elem.type != HE_TYPE_FREE)
            HudElem_Free(v0);
        ++v0;
    }
    memset(g_hudelems, 0, sizeof(g_hudelems));
}

// ============================================================================
// cg_messages.cpp - center prints, game messages, obituary (cg.o)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/cvar_types.h"
#include "game/game_types.h"

#include <string.h>

extern int currCl;
enum msgLocErrType_t {
    LOCMSG_NOERR = 0,
    LOCMSG_ERR = 1,
};
extern const char* SEH_LocalizeTextMessage(const char* pszInputBuffer,
                                           const char* pszMessageType,
                                           msgLocErrType_t errType);
// ea: 0x006C2D60
const char* SEH_LocalizeTextMessage(const char* pszMessage,
                                    const char* pszMsgType)
{
    return SEH_LocalizeTextMessage(pszMessage, pszMsgType, LOCMSG_NOERR);
}
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern unsigned int SEH_ReadCharFromString(const char** ppsText,
                                           int* pbIsTrailingPunctuation);
enum print_msg_type_t;
extern void CL_ConsolePrint(print_msg_type_t type, const char* txt,
                            int duration, int linewidth, int flags);
extern void CL_DeathMessagePrint(print_msg_type_t type,
                                 const char* pszAttackerName,
                                 float* vAttackerColor,
                                 const char* pszVictimName,
                                 float* vVictimColor,
                                 const char* pszIconShader, float fIconWidth,
                                 float fIconHeight, float* vIconColor,
                                 int iDuration);
extern void CG_DrawScoreboard_GetTeamColor(int iTeam, float* vColor);
extern bool Entity_IsLocalPlayer(const Entity* ent);
class MultiplayerMgr {
public:
    const char* GetPlayerName(const Entity* player) const;
};
// ea: 0x007401E0
const char* MultiplayerMgr_GetPlayerName(void* mgr, const Entity* player)
{
    return static_cast<const MultiplayerMgr*>(mgr)->GetPlayerName(player);
}
extern void* MultiplayerMgr_sInst;
extern weaponFileInfo_t* BG_GetInfoForWeapon(int weapon);

struct sentient_s {
    int eTeam;  // +0x00
};

extern vmCvar_t cg_gameMessageWidth;    // 0x00F5F158
extern vmCvar_t cg_gameBoldMessageWidth; // 0x00F60408

float gGameMessageFadeOutTime;  // 0x00F61704
Broc::string gGameMessage;             // ?gGameMessage@@3Vstring@Broc@@A (cg.o @ 0xF62930)
float MAXSCREENMESSAGETIME_1 = 3.0f;  // ?MAXSCREENMESSAGETIME_1@@3MA

// Per-client center-print state (stride 6320 bytes)
static char    sCenterText[4][6320];
static int     sCenterTime[4 * 1580];
static int     sCenterPriority[4 * 1580];
static int     sCenterCharWidth[4 * 1580];
static int     sCenterLines[4 * 1580];

// ea: 0x00687DA0
void CG_PriorityCenterPrint(const char* str, float y, int charWidth,
                            int priority)
{
    int v4 = 0;
    int cl = currCl;
    int stateIndex = 1580 * cl;
    if (sCenterTime[stateIndex] == 0
        || priority >= sCenterPriority[stateIndex])
    {
        const char* v5 =
            SEH_LocalizeTextMessage(str, "Center Print");
        Q_strncpyz(sCenterText[cl], v5, 512);
        sCenterPriority[stateIndex] = priority;
        char* s = sCenterText[cl];
        int v7 = 0;
        while (*s != 0)
        {
            unsigned int CharFromString =
                SEH_ReadCharFromString((const char**)&s, nullptr);
            if (CharFromString == 10)
                goto LABEL_10;
            if (++v7 < 75)
            {
                if (v4 == 0)
                    continue;
            }
            else
            {
                v4 = 1;
            }
            if (CharFromString == 32)
            {
                *(s - 1) = 10;
            LABEL_10:
                v4 = 0;
                v7 = 0;
            }
        }
        sCenterTime[stateIndex] = cgGlobal.time;
        sCenterPriority[stateIndex] = priority;
        sCenterCharWidth[stateIndex] = charWidth;
        sCenterLines[stateIndex] = 1;
        s = sCenterText[cl];
        while (*s != 0)
        {
            unsigned int v10 =
                SEH_ReadCharFromString((const char**)&s, nullptr);
            if (v10 == 10)
            {
                ++sCenterLines[stateIndex];
            }
            else if (v10 == 92 && *s == 110)
            {
                ++sCenterLines[stateIndex];
                ++s;
            }
        }
    }
}

// ea: 0x00687F10
void CG_CenterPrint(const char* str, int y, int charWidth)
{
    CG_PriorityCenterPrint(str, (float)y, charWidth, 0);
}

// ea: 0x0068B4F0
void CG_ObjMessage(const char* msg)
{
    if (gGameMessageFadeOutTime <= 0.0f
        || !(gGameMessage == msg))
    {
        gGameMessageFadeOutTime = MAXSCREENMESSAGETIME_1;
        gGameMessage = msg;
    }
}

// ea: 0x0068B540
void CG_GameMessage(const char* msg, int flags)
{
    CL_ConsolePrint((print_msg_type_t)3 /* PMSG_GAME */, msg, 0,
                    cg_gameMessageWidth.integer, flags);
}

// ea: 0x0068B560
void CG_BoldGameMessage(const char* msg)
{
    CL_ConsolePrint((print_msg_type_t)4 /* PMSG_BOLDGAME */, msg, 0,
                    cg_gameBoldMessageWidth.integer, 0);
}

// ea: 0x00697260
void CG_Obituary(Entity* target, Entity* attacker, int parm, bool teamGame)
{
    unsigned int v4 = 0;
    const char* szKillIcon;
    float vIconColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float vAttackerColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float vVictimColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    char attackerName[35];
    char targetName[36];
    float fIconWidth = 1.7f;

    attackerName[0] = 0;
    targetName[0] = 0;
    if (target == nullptr)
    {
        CG_ASSERT("target", "c:\\cod\\code\\game\\cg_event.cpp", 964);
    }
    if (target->sentient == nullptr)
    {
        CG_ASSERT("target->sentient", "c:\\cod\\code\\game\\cg_event.cpp",
                  965);
    }
    if ((parm & 0x80u) == 0)
    {
        weaponFileInfo_t* InfoForWeapon =
            (weaponFileInfo_t*)BG_GetInfoForWeapon(parm);
        if (InfoForWeapon->szKillIcon[0] != 0)
        {
            szKillIcon = InfoForWeapon->szKillIcon;
            if (InfoForWeapon->bWideKillIcon != 0)
                fIconWidth = 3.4000001f;
        }
        else
        {
            szKillIcon = "hint_death_died";
        }
    }
    else
    {
        v4 = parm & 0xFFFFFF7F;
        szKillIcon = "hint_death_died";
    }
    switch (v4)
    {
    case 0xBu:
    case 0x1Eu:
        szKillIcon = "hint_death_melee";
        goto LABEL_24;
    case 0xCu:
        szKillIcon = "hint_death_headshot";
        goto LABEL_24;
    case 0x11u:
    case 0x12u:
        szKillIcon = "hint_death_artillery";
        goto LABEL_24;
    case 0x13u:
    case 0x1Au:
        szKillIcon = "hint_death_died";
        goto LABEL_24;
    case 0x14u:
    case 0x15u:
        szKillIcon = "hint_death_tank";
        goto LABEL_24;
    case 0x16u:
        szKillIcon = "hint_death_jeep";
        goto LABEL_24;
    case 0x18u:
        szKillIcon = "hint_death_falling";
        goto LABEL_24;
    case 0x19u:
    case 0x1Fu:
        szKillIcon = "hint_death_suicide";
    LABEL_24:
        fIconWidth = 1.7f;
        break;
    default:
        break;
    }
    const char* PlayerName =
        MultiplayerMgr_GetPlayerName(MultiplayerMgr_sInst, target);
    Q_strncpyz(targetName, PlayerName, 32);
    strcat(targetName, "^7");
    if (cgGlobal.teamGame)
    {
        CG_DrawScoreboard_GetTeamColor(target->sentient->eTeam,
                                       vVictimColor);
    }
    else
    {
        if (Entity_IsLocalPlayer(target))
        {
            vVictimColor[0] = 0.390625f;  // 1048576000
            vVictimColor[1] = 1.0f;
        }
        else
        {
            vVictimColor[0] = 1.0f;
            vVictimColor[1] = 0.390625f;
        }
        vVictimColor[2] = 0.390625f;
    }
    if (attacker != nullptr && attacker->client != nullptr
        && target != attacker)
    {
        const char* v10 =
            MultiplayerMgr_GetPlayerName(MultiplayerMgr_sInst, attacker);
        Q_strncpyz(attackerName, v10, 32);
        if (cgGlobal.teamGame)
        {
            CG_DrawScoreboard_GetTeamColor(attacker->sentient->eTeam,
                                           vAttackerColor);
        }
        else
        {
            if (Entity_IsLocalPlayer(attacker))
            {
                vAttackerColor[0] = 0.390625f;
                vAttackerColor[1] = 1.0f;
            }
            else
            {
                vAttackerColor[0] = 1.0f;
                vAttackerColor[1] = 0.390625f;
            }
            vAttackerColor[2] = 0.390625f;
        }
    }
    CL_DeathMessagePrint((print_msg_type_t)3 /* PMSG_GAME */, attackerName,
                         vAttackerColor,
                         targetName, vVictimColor, szKillIcon,
                         fIconWidth * 1.25f, 2.125f, vIconColor, 0);
}

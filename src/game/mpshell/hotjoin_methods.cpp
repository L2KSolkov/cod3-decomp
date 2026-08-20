// ============================================================================
// hotjoin_methods.cpp - HotJoinMenu methods owned by mp_shell.o
// Bodies match the IDA release decompilation.
// ============================================================================

#include "game/mpshell/session_menus.h"

namespace View {
void UpdateNumViewports();
}

// ea: 0x005AF970
HotJoinMenu* HotJoinMenu::Me(int version)
{
    return static_cast<HotJoinMenu*>(g_femanager.GetIGMS(version)->menus[14]);
}

SpectateMenu* SpectateMenu::Me(int version)
{
    return static_cast<SpectateMenu*>(g_femanager.GetIGMS(version)->menus[12]);
}

WeaponSelectMenu* WeaponSelectMenu::Me(int version)
{
    return static_cast<WeaponSelectMenu*>(g_femanager.GetIGMS(version)->menus[1]);
}

InGameScoreBoard* InGameScoreBoard::Me(int version)
{
    return static_cast<InGameScoreBoard*>(g_femanager.GetIGMS(version)->menus[10]);
}

InGameSwitchSides* InGameSwitchSides::Me(int version)
{
    return static_cast<InGameSwitchSides*>(g_femanager.GetIGMS(version)->menus[11]);
}

// ea: 0x00792580
void HotJoinMenu::Close(bool joined)
{
    system->ReturnToPreviousMenu(-1);
    g_femanager.GetDMS(mVersion)->MakeActive(-1);
    if (!joined)
    {
        dword_F6A290[802 * mVersion] = 0;
        View::UpdateNumViewports();
    }
    ClearAllButtons();
}

// ea: 0x00792620
void HotJoinMenu::DisplayError(const char* error_msg)
{
    entries[0]->SetShown(false);
    SetHigh(1, true);
    panel->GetTextPointer("text_body")->SetText(error_msg);
}

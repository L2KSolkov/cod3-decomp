// ============================================================================
// menu_misc.cpp - FEMultiMenu + ControllerDisconnectedMenu
// (shell.o FEMultiMenu.cpp / ControllerDisconnectedMenu.cpp families)
// ============================================================================

#include "game/shell/shell_types.h"

#include <stdio.h>

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);             // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C

// Minimal STBManager view (full definition in game/core/core_systems.h,
// which conflicts with sv_stubs.h; same pattern as stringed_hooks.cpp).
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
};

// ============================================================================
// FEMultiMenu
// ============================================================================

// ea: 0x005921B0
FEMultiMenu::FEMultiMenu(FEMenuSystem* s, int num, int flg)
    : FEMenu(s, num, 0, 0, 1, flg)
{
    // The binary's ctor is the FEMenu base init inlined with navigation
    // sound left disabled (FEMenu ctor enables it).
    enableNavigationSound = false;
}

// ea: 0x00570950
void FEMultiMenu::ButtonHeldAction()
{
    if ((signed char)flags < 0)
    {
        switch (button_held_down)
        {
        case 4:
            Up();
            break;
        case 8:
            Down();
            break;
        case 0x10:
            Left();
            break;
        case 0x20:
            Right();
            break;
        default:
            return;
        }
    }
}

// ============================================================================
// ControllerDisconnectedMenu
// ============================================================================

// ea: 0x005931C0
ControllerDisconnectedMenu::ControllerDisconnectedMenu()
    : FEMenu()
{
    default_color_scheme = 0;
    text = nullptr;
}

// ea: 0x00574480
void ControllerDisconnectedMenu::OnActivate()
{
    FEMenu::OnActivate();
}

// ea: 0x00574490
void ControllerDisconnectedMenu::SetErrorMessage()
{
}

// ea: 0x00580290
void ControllerDisconnectedMenu::Draw()
{
    if (text == nullptr)
        return;

    const char* msg;
    char newString[512];
    if (controller::inst()->is_locked)
    {
        const char* part1 = STBManager::sInst->GetSTBString(
            "CGAME_XBOX_CONTROLLER_DISCONNECTED1");
        const char* part3 = STBManager::sInst->GetSTBString(
            "CGAME_XBOX_CONTROLLER_DISCONNECTED2");
        sprintf(newString, "%s %d %s", part1,
                controller::inst()->locked_port + 1, part3);
        msg = newString;
    }
    else
    {
        msg = STBManager::sInst->GetSTBString(
            "CGAME_XBOX_CONTROLLER_DISCONNECTED");
    }

    Broc::string v11(msg);
    Broc::string str = FEMultiLineText::ReplaceEndlines(v11);
    text->SetTextBoxNoLocalize(str, 260, -1.0f);
    text->Draw();
    if (panel != nullptr)
        panel->Draw();
    FEMenu::Draw();
}

// ea: 0x00586D20
void ControllerDisconnectedMenu::SetPanelFile(PanelFile* pf)
{
    if (text != nullptr)
    {
        text->~FEMultiLineText();
        mem_heap_free(text);
    }
    panel = pf;

    FEText* TextPointer = panel->GetTextPointer("Resume");
    text = (FEMultiLineText*)mem_heap_malloc(0xA8);
    if (text != nullptr)
    {
        color32 col;
        col.i = TextPointer->GetColor().i;
        new (text) FEMultiLineText(
            TextPointer->GetFont(), TextPointer->GetY(), 0.0f, 0,
            (panel_layer)TextPointer->GetScaleX(), 0.0f, 0, 0, col);
    }
    text->SetNumLines(8);
    text->SetNoFlash(color32(-1));
    text->SetLineSpacing(20);
}

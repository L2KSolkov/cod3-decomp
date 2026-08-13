// ============================================================================
// fe_menu.cpp - FEMenuEntry + FEMenu (shell.o FEMenu.cpp family)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/sv/sv_stubs.h"
#include "core/math_types.h"

#include <string.h>

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern const char* const defaultFileName;  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o

extern FEManager g_femanager;
extern FEMenuColorScheme color_schemes[];  // 0xDF3AE0
extern int currCl;                       // ?currCl@@3HA @ 0xF1579C
extern int dword_F6A28C[];               // @ 0xF6A28C

// Widget entry subclasses (shell.o, ported later)
class FEComboBox : public FEMenuEntry {
public:
    FEComboBox(FEMenu* parent, short maxOptions, FEText* text,
               FEText* label, PanelQuad* leftArrow,
               PanelQuad* rightArrow);  // ??0FEComboBox@@QAE@PAVFEMenu@@FPAVFEText@@1PAVPanelQuad@@2@Z
};
class FESlider : public FEMenuEntry {
public:
    FESlider(FEMenu* parent, PanelQuad* bar, FEText* label,
             FEText* barText);  // ??0FESlider@@QAE@PAVFEMenu@@PAVPanelQuad@@PAVFEText@@2@Z
};
class FEDoubleEntry : public FEMenuEntry {
public:
    FEDoubleEntry(FEMenu* parent, FEText* label,
                  FEText* text);  // ??0FEDoubleEntry@@QAE@PAVFEMenu@@PAVFEText@@1@Z
};
class FEMenuListBox : public FEMenuEntry {
public:
    FEMenuListBox(FEText* t, FEMenu* m,
                  int numLines);  // ??0FEMenuListBox@@QAE@PAVFEText@@PAVFEMenu@@H@Z
};

// ============================================================================
// FEMenuEntry
// ============================================================================

// ea: 0x0056FBC0
void FEMenuEntry::CommonConstructor(FEText* t, FEMenu* m)
{
    text = t;
    menu = m;
    color_scheme_index = m->GetDefaultColorScheme();
    highlight = false;
    disabled = false;
    up = -1;
    down = -1;
    left = -1;
    right = -1;
    if (t != nullptr)
        t->AddedToMenu(true);
    AdjustColor();
}

// ea: 0x005B1C90 (inline COMDAT)
FEMenuEntry::FEMenuEntry(FEText* t, FEMenu* m, bool delete_me)
{
    CommonConstructor(t, m);
    must_delete_text = delete_me;
}

// ea: 0x00585C70
FEMenuEntry::FEMenuEntry(const char* text, FEMenu* m, bool floating,
                         font_index ft, int nlines)
{
    font_index font = ft;
    if (ft == FONT_NORMAL)
        font = m->GetSystem()->font;
    FEText* v9;
    if (nlines == 1)
    {
        FEText* v8 = (FEText*)mem_heap_malloc(0x70u);
        v9 = v8 != nullptr
                 ? new (v8) FEText(font, text, 0.0f, 0.0f, 0,
                                   PANEL_LAYER_PAUSE_MENU, 1.0f, 0, 0,
                                   color32(0))
                 : nullptr;
    }
    else
    {
        FEMultiLineText* v10 = (FEMultiLineText*)mem_heap_malloc(0xA8u);
        v9 = v10 != nullptr
                 ? new (v10) FEMultiLineText(font, 0.0f, 0.0f, 0,
                                             PANEL_LAYER_PAUSE_MENU, 1.0f, 0,
                                             0, color32(0))
                 : nullptr;
        if (v9 != nullptr)
        {
            ((FEMultiLineText*)v9)->SetNumLines(nlines);
            v9->SetTextNoLocalize(text);
        }
    }
    CommonConstructor(v9, m);
    must_delete_text = true;
    if (v9 != nullptr)
        v9->AddedToMenu(false);
}

// ea: 0x005AE460 (inline COMDAT)
FEMenuEntry::~FEMenuEntry()
{
    if (must_delete_text && text != nullptr)
        delete text;
}

// ea: 0x0056FC20
void FEMenuEntry::CopyFrom(FEText* copy_this)
{
    text->CopyFrom(copy_this);
}

// ea: 0x0056FC30
void FEMenuEntry::SetText(FEText* copy_this)
{
    text = copy_this;
}

// ea: 0x0056FC40
void FEMenuEntry::Highlight(bool h, bool anim)
{
    highlight = h;
    AdjustColor();
    if (highlight)
        OnHighlight(anim);
}

// ea: 0x0056FC70
void FEMenuEntry::Disable(bool d)
{
    disabled = d;
    AdjustColor();
}

// ea: 0x0056FC90
color32 FEMenuEntry::WithAlpha(color32 c, int alpha)
{
    c.c.a = (unsigned char)alpha;
    return c;
}

// ea: 0x0056FCB0
void FEMenuEntry::AdjustColor()
{
    AdjustColor(text);
}

// ea: 0x0056FCC0
void FEMenuEntry::AdjustColor(FEText* text_to_adjust)
{
    if (color_scheme_index == 19)
        return;
    FEText* v4 = text_to_adjust;
    color32 color_unselect;
    unsigned int i;
    unsigned int v11;
    unsigned int v12;
    bool no_flash;
    if (color_scheme_index == 18)
    {
        no_flash = true;
        i = WithAlpha(v4->GetColor(), 140).i;
        color_unselect.i = i;
        v11 = WithAlpha(v4->GetColor(), 255).i;
        v12 = v11;
    }
    else
    {
        no_flash = color_schemes[color_scheme_index].flash == 0;
        i = color_schemes[color_scheme_index].unselect.i;
        v11 = color_schemes[color_scheme_index].high1.i;
        v12 = color_schemes[color_scheme_index].high2.i;
        color_unselect.i = i;
    }
    if (highlight)
    {
        int flags = menu->flags;
        if ((flags & 8) != 0 && disabled)
        {
            goto LABEL_8;
        }
        if ((flags & 0x10) == 0 && !no_flash)
        {
            v4->SetFlash(color32(v11), color32(v12), 1.0f);
            return;
        }
        if ((flags & 0x20) == 0)
        {
            v4->SetNoFlash(color32(v11));
            return;
        }
    }
    else
    {
        if (v4 == nullptr)
            return;
        if (disabled)
        {
        LABEL_8:
            v4->SetNoFlash(WithAlpha(
                color32(i), (unsigned char)(color_unselect.c.a * 0.2f)));
            return;
        }
    }
    v4->SetNoFlash(color32(i));
}

// ea: 0x0056FE70
void FEMenuEntry::MoveForSplitScreen(int viewport, int old_viewport)
{
    text->MoveForSplitScreen(viewport, old_viewport);
}

// ============================================================================
// FEMenu
// ============================================================================

// ea: 0x0057DA80
FEMenu::FEMenu()
{
    entries = nullptr;
    system = nullptr;
    center_x = 0;
    center_y = 0;
    y_distance = 0;
    half_height = 0;
    first_vis_entry = 0;
    highlighted = -1;
    highlightedDefault = -1;
    num_entries = 0;
    max_vis_entries = 1;
    flags = 0;
    sound = nullptr;
    lockInput = false;
    enableNavigationSound = false;
    mReturnMenu = -1;
    helpbar = nullptr;
    helpbar1 = nullptr;
    helpbar2 = nullptr;
    helpbar3 = nullptr;
    panel = nullptr;
}

// ea: 0x0057DAE0
FEMenu::FEMenu(FEMenuSystem* menuSystem, int num, int x, int y, short mve,
               short flg)
{
    center_y = y;
    system = menuSystem;
    center_x = x;
    highlighted = -1;
    highlightedDefault = -1;
    entries = nullptr;
    y_distance = 28;
    half_height = 0;
    first_vis_entry = 0;
    num_entries = num;
    max_vis_entries = mve;
    flags = (int16_t)(flg & 0xFDFF);
    sound = nullptr;
    mReturnMenu = -1;
    lockInput = false;
    enableNavigationSound = true;
    helpbar = nullptr;
    helpbar1 = nullptr;
    helpbar2 = nullptr;
    helpbar3 = nullptr;
    panel = nullptr;
    entries = (FEMenuEntry**)mem_heap_malloc(4 * num);
    for (int i = 0; i < num; ++i)
        entries[i] = nullptr;
    if (system != nullptr)
        default_color_scheme = system->GetDefaultColorScheme();
    else
        default_color_scheme = 10;
}

// ea: 0x00592150
FEMenu::~FEMenu()
{
    Cleanup();
    if (panel != nullptr)
    {
        delete panel;
    }
    mem_heap_free(entries);
    if (helpbar1 != nullptr)
        delete helpbar1;
    if (helpbar2 != nullptr)
        delete helpbar2;
    if (helpbar3 != nullptr)
        delete helpbar3;
}

// ea: 0x0058DF20
void FEMenu::Cleanup()
{
    for (int i = 0; i < num_entries; ++i)
    {
        if (entries[i] != nullptr)
            delete entries[i];
        entries[i] = nullptr;
    }
    unsigned int mVal = *(unsigned int*)&sound;
    unsigned int v5 = mVal & 0xFFF;
    if (v5 < 0x200
        && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v5].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v5].mObject != nullptr)
    {
        unsigned int v6 = mVal & 0xFFF;
        SoundDevice::Sound* mObject = nullptr;
        if (v6 < 0x200
            && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v6].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v6].mObject;
        SoundDevice::sInst->ReleaseSound(mObject);
    }
    sound = nullptr;
    if (helpbar1 != nullptr)
        delete helpbar1;
    if (helpbar2 != nullptr)
        delete helpbar2;
    if (helpbar3 != nullptr)
        delete helpbar3;
    helpbar1 = nullptr;
    helpbar2 = nullptr;
    helpbar3 = nullptr;
    helpbar = nullptr;
    panel = nullptr;
}

// ea: 0x0056FE80
void FEMenu::ConnectEntries(short index)
{
    FEMenuEntry* v3 = entries[index];
    v3->up = (short)(index - 1);
    v3->down = (short)(((flags & 2) != 0) - 1);
    if (index != 0 && entries[index - 1] != nullptr)
    {
        entries[index - 1]->down = index;
        entries[index] = v3;
    }
    else
    {
        if ((flags & 2) != 0)
        {
            entries[0]->up = num_entries;
            entries[index] = v3;
        }
        else
        {
            entries[index] = v3;
        }
    }
}

// ea: 0x0056FF00
void FEMenu::Down()
{
    if (lockInput)
        return;
    short highlighted = this->highlighted;
    if (highlighted < 0 || highlighted >= num_entries)
        return;
    FEMenuEntry* v3 = entries[highlighted];
    int v4 = v3->OnDown();
    if ((flags & 8) != 0)
        goto LABEL_24;
    for (; v4 >= 0; v4 = entries[v4]->OnDown())
    {
        if (v4 >= num_entries)
            break;
        if (!entries[v4]->GetDisable())
            break;
        if (v4 == highlighted)
            break;
    }
    if (v4 == highlighted)
        return;
LABEL_24:
    if ((flags & 1) != 0)
    {
        if (highlighted == max_vis_entries + first_vis_entry - 1)
        {
            int v8 = (first_vis_entry + 1) % num_entries;
            if ((flags & 0x40) != 0)
            {
                if (entries[v8]->GetDisable())
                {
                    do
                    {
                        if (v8 == highlighted)
                            break;
                        v8 = (v8 + 1) % num_entries;
                    }
                    while (entries[v8]->GetDisable());
                }
            }
            SetVis(v8);
        }
    }
    if (v4 >= 0)
    {
        if (enableNavigationSound)
            PlayNavigationSound();
        SetHigh(v4, true);
    }
}

// ea: 0x00570020
void FEMenu::Up()
{
    if (lockInput)
        return;
    short highlighted = this->highlighted;
    if (highlighted < 0 || highlighted >= num_entries)
        return;
    FEMenuEntry* v3 = entries[highlighted];
    int v4 = v3->OnUp();
    if ((flags & 8) != 0)
        goto LABEL_24;
    for (; v4 >= 0; v4 = entries[v4]->OnUp())
    {
        if (v4 >= num_entries)
            break;
        if (!entries[v4]->GetDisable())
            break;
        if (v4 == highlighted)
            break;
    }
    if (v4 == highlighted)
        return;
LABEL_24:
    if ((flags & 1) != 0)
    {
        if (highlighted == first_vis_entry)
        {
            int v8 = (first_vis_entry + num_entries - 1) % num_entries;
            if ((flags & 0x40) != 0)
            {
                if (entries[(first_vis_entry + num_entries - 1) % num_entries]
                        ->GetDisable())
                {
                    do
                    {
                        if (v8 == highlighted)
                            break;
                        v8 = (num_entries + v8 - 1) % num_entries;
                    }
                    while (entries[v8]->GetDisable());
                }
            }
            SetVis(v8);
        }
    }
    if (v4 >= 0)
    {
        if (enableNavigationSound)
            PlayNavigationSound();
        SetHigh(v4, true);
    }
}

// ea: 0x00570140
void FEMenu::Left()
{
    if (lockInput)
        return;
    short highlighted = this->highlighted;
    if (highlighted < 0 || highlighted >= num_entries)
        return;
    FEMenuEntry* v3 = entries[highlighted];
    int v4 = v3->OnLeft();
    if ((flags & 8) != 0)
        goto LABEL_10;
    for (; v4 >= 0; v4 = entries[v4]->OnLeft())
    {
        if (!entries[v4]->GetDisable())
            break;
        if (v4 == highlighted)
            break;
    }
    if (v4 == highlighted)
        return;
LABEL_10:
    if (v4 >= 0)
    {
        if (enableNavigationSound)
            PlayNavigationSound();
        SetHigh(v4, true);
    }
}

// ea: 0x005701E0
void FEMenu::Right()
{
    if (lockInput)
        return;
    short highlighted = this->highlighted;
    if (highlighted < 0 || highlighted >= num_entries)
        return;
    FEMenuEntry* v3 = entries[highlighted];
    int v4 = v3->OnRight();
    if ((flags & 8) != 0)
        goto LABEL_10;
    for (; v4 >= 0; v4 = entries[v4]->OnRight())
    {
        if (!entries[v4]->GetDisable())
            break;
        if (v4 == highlighted)
            break;
    }
    if (v4 == highlighted)
        return;
LABEL_10:
    if (v4 >= 0)
    {
        if (enableNavigationSound)
            PlayNavigationSound();
        SetHigh(v4, true);
    }
}

// ea: 0x00570280
void FEMenu::SetVis(int first)
{
    first_vis_entry = (short)first;
    if (max_vis_entries <= 0)
        return;
    int v3 = first;
    int v4 = 0;
    while (1)
    {
        if (v3 < num_entries)
        {
            if ((flags & 0x40) == 0)
                goto LABEL_8;
            if (!entries[v3]->GetDisable())
                goto LABEL_8;
        }
        if (++v3 >= num_entries)
        {
            if ((flags & 2) == 0)
                return;
            v3 = 0;
        }
        continue;
    LABEL_8:
        int v7;
        if ((flags & 0x400) != 0)
            v7 = v4 * y_distance;
        else
        {
            int half_height = (flags & 0x800) != 0 ? 2 * this->half_height
                                                   : this->half_height;
            v7 = v4 * y_distance - half_height;
        }
        FEMenuEntry* v10 = entries[v3];
        if (v10 != nullptr)
            v10->SetPos((float)center_x, (float)(center_y + v7));
        ++v4;
        ++v3;
        if (v4 >= max_vis_entries)
            return;
    }
}

// ea: 0x00570350
void FEMenu::SetVerticalJust(bool top, bool bottom)
{
    flags = (int16_t)(flags & 0xF3FF);
    if (top)
        flags = (int16_t)(flags | 0x400);
    else if (bottom)
        flags = (int16_t)(flags | 0x800);
}

// ea: 0x00570390
void FEMenu::SetScaleThroughout(float sc)
{
    for (int i = 0; i < num_entries; ++i)
        entries[i]->SetScale(sc);
}

// ea: 0x005703C0
void FEMenu::SetZThroughout(float z, panel_layer layer)
{
    for (int i = 0; i < num_entries; ++i)
        entries[i]->SetZ(z, layer);
}

// ea: 0x00570400
void FEMenu::Init()
{
    int v2 = num_entries;
    short max_vis_entries = this->max_vis_entries;
    if (v2 > max_vis_entries)
        flags = (int16_t)(flags | 1);
    int v7;
    if ((flags & 1) != 0)
    {
        v2 = max_vis_entries;
        v7 = v2 - 1;
    }
    else if ((flags & 0x40) == 0)
    {
        v2 = v2;
        v7 = v2 - 1;
    }
    else
    {
        int v4 = 0;
        for (int v5 = 0; v5 < num_entries; ++v5)
        {
            if (!entries[v5]->GetDisable())
                ++v4;
        }
        v7 = v4 - 1;
    }
    half_height = y_distance * v7 / 2;
    if (num_entries >= 1)
        SetHigh(0, false);
    flags = (int16_t)(flags | 0x200);
    if ((flags & 1) != 0)
    {
        SetVis(0);
    }
    else
    {
        int i = 0;
        for (int v9 = 0; v9 < num_entries; ++v9)
        {
            FEMenuEntry* v11 = entries[v9];
            if (v11 != nullptr
                && ((flags & 0x40) == 0 || !v11->GetDisable()))
            {
                int v12;
                if ((flags & 0x400) != 0)
                    v12 = center_y + i * y_distance;
                else
                {
                    if ((flags & 0x800) != 0)
                        v12 = center_y + i * y_distance - 2 * half_height;
                    else
                        v12 = center_y + i * y_distance - half_height;
                }
                v11->SetPos((float)center_x, (float)v12);
                ++i;
            }
        }
    }
}

// ea: 0x00570550
void FEMenu::Draw()
{
    if (system != nullptr && system->drawHelpbar)
    {
        if (helpbar != nullptr)
            helpbar->Draw();
        if (helpbar1 != nullptr)
            helpbar1->Draw();
        if (helpbar2 != nullptr)
            helpbar2->Draw();
        if (helpbar3 != nullptr)
            helpbar3->Draw();
    }
    if ((flags & 1) != 0)
    {
        int first_vis_entry = this->first_vis_entry;
        int v11 = 0;
        if (max_vis_entries <= 0)
            return;
        while (1)
        {
            if (first_vis_entry < num_entries)
            {
                if ((flags & 0x40) == 0)
                    goto LABEL_26;
                if (!entries[first_vis_entry]->GetDisable())
                    goto LABEL_26;
            }
            if (++first_vis_entry >= num_entries)
            {
                if ((flags & 2) == 0)
                    return;
                first_vis_entry = 0;
            }
            continue;
        LABEL_26:
            entries[first_vis_entry]->Draw();
            ++v11;
            ++first_vis_entry;
            if (v11 >= max_vis_entries)
                return;
        }
    }
    for (int i = 0; i < num_entries; ++i)
    {
        if (entries[i] != nullptr
            && ((flags & 0x40) == 0 || !entries[i]->GetDisable()))
            entries[i]->Draw();
    }
}

// ea: 0x00570660
void FEMenu::Update(float time_inc)
{
    if ((char)flags < 0 && button_held_down != -1)
    {
        button_held_timer -= time_inc;
        if (button_held_timer <= 0.0f)
        {
            ButtonHeldAction();
            button_held_timer = 0.2f;
            if ((flags & 0x100) != 0)
                button_held_timer = 0.020000001f;
        }
    }
    for (int i = 0; i < num_entries; ++i)
    {
        if (entries[i] != nullptr)
            entries[i]->Update(time_inc);
    }
}

// ea: 0x00570700
void FEMenu::HighlightDefault()
{
    int i = 0;
    for (; i < num_entries; ++i)
    {
        if (entries[i] == nullptr)
            break;
        if (!entries[i]->GetDisable())
            break;
    }
    if (i < num_entries)
        SetHigh(i, false);
}

// ea: 0x00570750
void FEMenu::OnActivate()
{
    if (highlighted == -1)
        highlighted = highlightedDefault;
    if (highlighted == -1 || entries[highlighted] == nullptr
        || entries[highlighted]->GetDisable())
    {
        HighlightDefault();
    }
    else
    {
        SetHigh(highlighted, false);
    }
    button_held_down = -1;
    if ((flags & 1) == 0)
    {
        if ((flags & 0x40) != 0)
            Init();
    }
    else
    {
        SetVis(highlighted);
    }
}

// ea: 0x005707D0
void FEMenu::OnCross(int c)
{
    if (highlighted >= 0)
    {
        FEMenuEntry* v3 = entries[highlighted];
        if (!v3->GetDisable())
            Select(highlighted, c);
    }
}

// ea: 0x00570810
void FEMenu::OnAnyButtonPress(int c, int b)
{
    if ((char)flags < 0 && (b == 4 || b == 8 || b == 16 || b == 32))
    {
        button_held_down = (char)b;
        button_held_timer = 0.30000001f;
    }
}

// ea: 0x00570850
void FEMenu::OnButtonRelease(int c, int b)
{
    if ((char)flags < 0 && (b == 4 || b == 8 || b == 16 || b == 32)
        && b == button_held_down)
        button_held_down = -1;
}

// ea: 0x00570890
void FEMenu::ButtonHeldAction()
{
    if ((char)flags < 0)
    {
        if (button_held_down == 4)
            Up();
        else if (button_held_down == 8)
            Down();
    }
}

// ea: 0x005708C0
void FEMenu::ClearAllButtons()
{
    int locked_port = dword_F6A28C[802 * currCl];
    if (controller::inst()->is_locked)
        locked_port = controller::inst()->locked_port;
    controller* v2 = controller::inst();
    for (int i = controller::LEFTBUTTON; i < 16; ++i)
        v2->button_pressed_clear(locked_port, (controller::ButtonIndex)i);
}

// ea: 0x00570910
void FEMenu::ClearButton(controller::ButtonIndex a_eButton)
{
    int locked_port = dword_F6A28C[802 * currCl];
    if (controller::inst()->is_locked)
        locked_port = controller::inst()->locked_port;
    controller* v3 = controller::inst();
    v3->button_pressed_clear(locked_port, a_eButton);
}

// ea: 0x0057DBA0
void FEMenu::AddEntry(int index, FEText* t, bool delete_me)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 376;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEMenuEntry* v5 = (FEMenuEntry*)mem_heap_malloc(0x18u);
    FEMenuEntry* v6;
    if (v5 != nullptr)
    {
        v6 = new (v5) FEMenuEntry(t, this, delete_me);
    }
    else
    {
        v6 = nullptr;
    }
    entries[index] = v6;
    ConnectEntries((short)index);
}

// ea: 0x00585DA0
void FEMenu::AddEntry(int index, const char* text)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 366;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEMenuEntry* v4 = (FEMenuEntry*)mem_heap_malloc(0x18u);
    FEMenuEntry* v5;
    if (v4 != nullptr)
    {
        v5 = new (v4) FEMenuEntry(text, this, false, FONT_NORMAL, 1);
    }
    else
    {
        v5 = nullptr;
    }
    entries[index] = v5;
    ConnectEntries((short)index);
}

// ea: 0x0057DC70
void FEMenu::SetHigh(int index, bool anim)
{
    if (lockInput)
        return;
    for (short i = 0; i < num_entries; ++i)
    {
        if (entries[i] != nullptr)
        {
            if (i == index)
            {
                entries[i]->Highlight(true, true);
                FEText* text = entries[i]->text;
                if (text != nullptr && text->flash_info != nullptr)
                {
                    text->flash_info->flash_timer = 0.0f;
                    text->flash_info->flash_intensity = 0.0f;
                }
            }
            else
            {
                entries[i]->Highlight(false, true);
            }
            entries[i]->AdjustColor();
        }
    }
    highlighted = (short)index;
}

// ea: 0x0057DF20
void FEMenu::UpdateWidescreen(bool widescreen)
{
    if (panel != nullptr)
        panel->UpdateWidescreen(widescreen, 320.0f);
    if (helpbar1 != nullptr)
        helpbar1->UpdateForWidescreen(widescreen);
    if (helpbar2 != nullptr)
        helpbar2->UpdateForWidescreen(widescreen);
    if (helpbar3 != nullptr)
        helpbar3->UpdateForWidescreen(widescreen);
    for (int i = 0; i < num_entries; ++i)
    {
        if (entries[i] != nullptr)
            entries[i]->UpdateWidescreen(widescreen);
    }
}

// ea: 0x0057DD00
void FEMenu::PlayNavigationSound()
{
    math::Position3 v7;
    math::Dir3 v8;
    memset(&v7, 0, sizeof(v7));
    memset(&v8, 0, sizeof(v8));
    *(unsigned int*)&sound =
        SoundDevice::sInst
            ->PlaySound("UI_Select", DbLinkedHandle<EntityHandleDb, Entity>(),
                        true, false, v7, v8, -1.0f, -1.0f, -1.0f, -1.0f)
            .mHandle.mVal;
}

// ea: 0x0058E0B0
void FEMenu::PlayNavigationSoundWait()
{
    unsigned int mVal = *(unsigned int*)&sound;
    unsigned int v4 = mVal & 0xFFF;
    if (v4 < 0x200
        && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v4].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v4].mObject != nullptr)
    {
        unsigned int v5 = mVal & 0xFFF;
        SoundDevice::Sound* mObject = nullptr;
        if (v5 < 0x200
            && (mVal >> 12) == SoundDevice::SoundHandleDb::sInst.mElements[v5].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v5].mObject;
        if (mObject->IsPlaying())
            return;
    }
    math::Position3 v7;
    math::Dir3 v8;
    memset(&v7, 0, sizeof(v7));
    memset(&v8, 0, sizeof(v8));
    *(unsigned int*)&sound =
        SoundDevice::sInst
            ->PlaySound("UI_Select", DbLinkedHandle<EntityHandleDb, Entity>(),
                        true, false, v7, v8, -1.0f, -1.0f, -1.0f, -1.0f)
            .mHandle.mVal;
}

// ea: 0x00585F30
FEComboBox* FEMenu::AddComboBox(int index, short numOptions, FEText* text,
                                FEText* label, PanelQuad* leftArrow,
                                PanelQuad* rightArrow)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 396;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEComboBox* v8 = (FEComboBox*)mem_heap_malloc(0x60u);
    FEComboBox* v9;
    if (v8 != nullptr)
        v9 = new (v8) FEComboBox(this, numOptions, text, label, leftArrow,
                                 rightArrow);
    else
        v9 = nullptr;
    entries[index] = v9;
    ConnectEntries((short)index);
    return v8;
}

// ea: 0x00585E60
FEComboBox* FEMenu::AddComboBox(int index, short numOptions, FEText* text,
                                PanelQuad* leftArrow, PanelQuad* rightArrow)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 387;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEComboBox* v7 = (FEComboBox*)mem_heap_malloc(0x60u);
    FEComboBox* v8;
    if (v7 != nullptr)
        v8 = new (v7) FEComboBox(this, numOptions, text, nullptr, leftArrow,
                                 rightArrow);
    else
        v8 = nullptr;
    entries[index] = v8;
    ConnectEntries((short)index);
    return v8;
}

// ea: 0x005860D0
FESlider* FEMenu::AddSlider(int index, FEText* barText, FEText* label)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 414;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FESlider* v5 = (FESlider*)mem_heap_malloc(0x38u);
    FESlider* v6;
    if (v5 != nullptr)
        v6 = new (v5) FESlider(this, nullptr, label, barText);
    else
        v6 = nullptr;
    entries[index] = v6;
    ConnectEntries((short)index);
    return v6;
}

// ea: 0x00586000
FESlider* FEMenu::AddSlider(int index, PanelQuad* bar, FEText* label)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 405;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FESlider* v5 = (FESlider*)mem_heap_malloc(0x38u);
    FESlider* v6;
    if (v5 != nullptr)
        v6 = new (v5) FESlider(this, bar, label, nullptr);
    else
        v6 = nullptr;
    entries[index] = v6;
    ConnectEntries((short)index);
    return v6;
}

// ea: 0x005861A0
FEDoubleEntry* FEMenu::AddDoubleEntry(int index, FEText* barText,
                                      FEText* label)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 423;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEDoubleEntry* v5 = (FEDoubleEntry*)mem_heap_malloc(0x1Cu);
    FEDoubleEntry* v6;
    if (v5 != nullptr)
        v6 = new (v5) FEDoubleEntry(this, label, barText);
    else
        v6 = nullptr;
    entries[index] = v6;
    ConnectEntries((short)index);
    return v6;
}

// ea: 0x0058DFF0
FEMenuListBox* FEMenu::AddListBoxEntry(int index, FEText* t, int numLines)
{
    if (index < 0 || index >= num_entries)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 432;
        AeAssert::gCurrentExpr = "index >= 0 && index < num_entries";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    FEMenuListBox* v5 = (FEMenuListBox*)mem_heap_malloc(0x5Cu);
    FEMenuListBox* result;
    if (v5 != nullptr)
        result = new (v5) FEMenuListBox(t, this, numLines);
    else
        result = nullptr;
    entries[index] = result;
    return result;
}

// ============================================================================
// FEMenuSystem
// ============================================================================

// ea: 0x00570A10
FEMenuSystem::~FEMenuSystem()
{
    if (menus != nullptr)
    {
        for (int i = 0; i < count; ++i)
        {
            if (menus[i] != nullptr)
                delete menus[i];
            menus[i] = nullptr;
        }
        mem_heap_free(menus);
        menus = nullptr;
    }
}

// ea: 0x00570A30
void FEMenuSystem::Add(FEMenu* m)
{
    if (count >= size)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1099;
        AeAssert::gCurrentExpr = "count < size";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    menus[count++] = m;
}

// ea: 0x00570AA0
void FEMenuSystem::InitAll()
{
    UpdateButtonDown();
    for (int i = 0; i < count; ++i)
        menus[i]->Init();
}

// ea: 0x00570AD0
void FEMenuSystem::ReturnToPreviousMenu(int fallback)
{
    int m_active = this->m_active;
    if (m_active != -1)
    {
        int mReturnMenu = menus[m_active]->mReturnMenu;
        menus[m_active]->mReturnMenu = -1;
        if (mReturnMenu != -1)
        {
            MakeActive(mReturnMenu, menus[mReturnMenu]->mReturnMenu);
            return;
        }
    }
    MakeActive(fallback, -1);
}

// ea: 0x00570B20
void FEMenuSystem::MakeActive(int index)
{
    MakeActive(index, -1);
}

// ea: 0x0057DDF0
void FEMenuSystem::MakeActive(int index, int return_to_menu)
{
    if (index >= size)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1174;
        AeAssert::gCurrentExpr = "index < size";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index == -1)
        background = -1;
    int m_active = this->m_active;
    if (m_active != -1)
    {
        FEMenu* v8 = index < 0 ? nullptr : menus[index];
        menus[m_active]->OnDeactivate(v8);
    }
    if (index != -1)
        menus[index]->mReturnMenu = return_to_menu;
    int v10 = this->m_active;
    this->m_active = index;
    if (index >= 0)
    {
        menus[index]->OnActivate(v10);
        if ((menus[index]->flags & 0x1000) == 0)
        {
            math::Position3 v13;
            math::Dir3 v14;
            memset(&v13, 0, sizeof(v13));
            memset(&v14, 0, sizeof(v14));
            SoundDevice::sInst->PlaySound(
                "UI_Highlight", DbLinkedHandle<EntityHandleDb, Entity>(),
                true, false, v13, v14, -1.0f, -1.0f, -1.0f, -1.0f);
        }
    }
    UpdateButtonDown();
    NewMenuActive();
}

// ea: 0x00570B80
void FEMenuSystem::MakeActiveAndReturn(int index, int return_to)
{
    if (index != -1)
        menus[index]->SetHigh(-1, true);
    MakeActive(index, return_to);
}

// ea: 0x00570B40
void FEMenuSystem::MakeActiveAndReturn(int index)
{
    if (index != -1)
        menus[index]->SetHigh(-1, true);
    MakeActive(index, m_active);
}

// ea: 0x00570BC0
bool FEMenuSystem::IsMenuActive(int menu)
{
    return m_active == menu;
}

// ea: 0x00570BE0
void FEMenuSystem::ClearReturnMenu(int menu)
{
    if (menu != -1)
        menus[menu]->mReturnMenu = -1;
}

// ea: 0x00570C00
void FEMenuSystem::AddOverlay(int index)
{
    if (m_active < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1231;
        AeAssert::gCurrentExpr = "m_active >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1232;
        AeAssert::gCurrentExpr = "index >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (background != -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1233;
        AeAssert::gCurrentExpr = "background == -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    background = m_active;
    m_active = index;
    menus[index]->OnActivate();
}

// ea: 0x00570D00
void FEMenuSystem::RemoveOverlay()
{
    if (m_active < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1241;
        AeAssert::gCurrentExpr = "m_active >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (background < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1242;
        AeAssert::gCurrentExpr = "background >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (background >= 0)
    {
        menus[m_active]->OnDeactivate(menus[background]);
        m_active = background;
        background = -1;
    }
}

// ea: 0x00570DC0
int FEMenuSystem::CurrentOverlay()
{
    if (background != -1)
        return m_active;
    return -1;
}

// ea: 0x00570DD0
void FEMenuSystem::Update(float time_inc)
{
    if (m_active >= 0 && menus[m_active] != nullptr)
    {
        menus[m_active]->Update(time_inc);
        UpdateButtonPresses();
    }
}

// ea: 0x00570E10
void FEMenuSystem::UpdateSplitScreen()
{
    for (int i = 0; i < count; ++i)
        menus[i]->UpdateSplitScreen();
}

// ea: 0x00570E40
void FEMenuSystem::UpdateButtonDown()
{
    int i_controller = 0;
    for (int i = 4; i <= 32; i *= 2)
    {
        int v3 = i_controller;
        if (GetAnalogPressed(i, &i_controller))
            button_down_flags[v3] |= (int16_t)i;
        else
            button_down_flags[v3] &= (int16_t)~i;
    }
}

// ea: 0x00570E90
void FEMenuSystem::Draw()
{
    if (background >= 0)
    {
        drawHelpbar = false;
        menus[background]->Draw();
        drawHelpbar = true;
    }
    if (m_active >= 0 && menus[m_active] != nullptr)
        menus[m_active]->Draw();
}

// ea: 0x00570ED0
void FEMenuSystem::Draw3D()
{
    if (background >= 0)
    {
        drawHelpbar = false;
        menus[background]->Draw3D();
        drawHelpbar = true;
    }
    if (m_active >= 0 && menus[m_active] != nullptr)
        menus[m_active]->Draw3D();
}

// ea: 0x00570F10
void FEMenuSystem::OnButtonPress(int button, int controller)
{
    GetClientFromController(controller);
    int m_active = this->m_active;
    if (m_active < 0)
        return;
    FEMenu* v5 = menus[m_active];
    v5->OnAnyButtonPress(controller, button);
    if (button > 128)
    {
        if (button > 2048)
        {
            if (button == 4096)
            {
                menus[m_active]->OnL2(controller);
                return;
            }
            if (button == 0x2000)
            {
                menus[m_active]->OnR2(controller);
                return;
            }
        }
        else
        {
            switch (button)
            {
            case 2048:
                menus[m_active]->OnR1(controller);
                return;
            case 256:
                menus[m_active]->OnSquare(controller);
                return;
            case 512:
                menus[m_active]->OnTrueTriangle(controller);
                menus[m_active]->OnCircle(controller);
                return;
            case 1024:
                menus[m_active]->OnL1(controller);
                return;
            default:
                break;
            }
        }
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
        AeAssert::gCurrentLine = 1530;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    else if (button == 128)
    {
        menus[m_active]->OnTrueCircle(controller);
        menus[m_active]->OnTriangle(controller);
    }
    else
    {
        switch (button)
        {
        case 1:
            menus[m_active]->OnSelect(controller);
            break;
        case 2:
            menus[m_active]->OnStart(controller);
            break;
        case 4:
            menus[m_active]->OnUp(controller);
            break;
        case 8:
            menus[m_active]->OnDown(controller);
            break;
        case 16:
            menus[m_active]->OnLeft(controller);
            break;
        case 32:
            menus[m_active]->OnRight(controller);
            break;
        case 64:
            menus[m_active]->OnCross(controller);
            break;
        default:
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenu.cpp";
            AeAssert::gCurrentLine = 1530;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
            break;
        }
    }
}

// ea: 0x005711A0
void FEMenuSystem::OnButtonRelease(int button, int controller)
{
    if (m_active >= 0 && menus[m_active] != nullptr)
        menus[m_active]->OnButtonRelease(controller, button);
}

// ea: 0x005711D0
bool FEMenuSystem::GetAnalogPressed(int button, int* p_controller)
{
    int x_controller = 0;
    int y_controller = 0;
    int v5 = GetStickValueX(controller::LEFTSTICK, &x_controller);
    int v6 = GetStickValueY(controller::LEFTSTICK, &y_controller);
    if (GetButtonPressed(controller::UPBUTTON, &y_controller))
        v6 = -128;
    if (GetButtonPressed(controller::DOWNBUTTON, &y_controller))
        v6 = 128;
    if (GetButtonPressed(controller::LEFTBUTTON, &x_controller))
        v5 = -128;
    if (GetButtonPressed(controller::RIGHTBUTTON, &x_controller))
        v5 = 128;
    switch (button)
    {
    case 4:
        return v6 < -64;
    case 8:
        return v6 > 64;
    case 16:
        return v5 < -64;
    case 32:
        return v5 > 64;
    default:
        return false;
    }
}

// ea: 0x00571300
int FEMenuSystem::GetCurrentClient()
{
    return 0;
}

// ea: 0x00571310
int FEMenuSystem::GetCurrentClientController()
{
    return dword_F6A28C[0];
}

// ea: 0x00571320
int FEMenuSystem::GetClientFromController(int c)
{
    return GetCurrentClient();
}

// ea: 0x00571330
bool FEMenuSystem::GetButtonPressed(controller::ButtonIndex button,
                                    int* p_controller)
{
    controller* v3 = controller::inst();
    return v3->button_pressed(button, p_controller);
}

// ea: 0x00571350
bool FEMenuSystem::GetButtonReleased(controller::ButtonIndex button,
                                     int* p_controller)
{
    controller* v3 = controller::inst();
    return v3->button_released(button, p_controller);
}

// ea: 0x00571370
int FEMenuSystem::GetActiveMenu()
{
    return m_active;
}

// ea: 0x00571380
void FEMenuSystem::SetActiveMenu(int menu)
{
    m_active = menu;
}

// ea: 0x00571390
int FEMenuSystem::GetStickValueX(controller::StickIndex stick,
                                 int* p_controller)
{
    return controller::inst()->stick_value_x(stick, p_controller);
}

// ea: 0x005713B0
int FEMenuSystem::GetStickValueY(controller::StickIndex stick,
                                 int* p_controller)
{
    return controller::inst()->stick_value_y(stick, p_controller);
}

// ea: 0x0057DFB0
void FEMenuSystem::UpdateButtonPresses()
{
    int v2 = 1;
    while (1)
    {
        int controllerPort = GetCurrentClientController();
        if (v2 >= 4 && v2 <= 32)
        {
            if (GetAnalogPressed(v2, &controllerPort)
                && (button_down_flags[controllerPort] & v2) == 0)
            {
                OnButtonPress(v2, controllerPort);
                button_down_flags[controllerPort] |= (int16_t)v2;
                return;
            }
            if (!GetAnalogPressed(v2, &controllerPort)
                && (button_down_flags[controllerPort] & v2) != 0)
            {
                OnButtonRelease(v2, controllerPort);
                button_down_flags[controllerPort] &= (int16_t)~v2;
                return;
            }
        }
        else
        {
            controller::ButtonIndex v3 = mapButton(v2);
            if (GetButtonPressed(v3, &controllerPort))
            {
                controller::inst()->button_pressed_clear(
                    controllerPort, mapButton(v2));
                OnButtonPress(v2, controllerPort);
                return;
            }
            if (GetButtonReleased(v3, &controllerPort))
            {
                controller::inst()->button_released_clear(
                    controllerPort, mapButton(v2));
                OnButtonRelease(v2, controllerPort);
                return;
            }
        }
        v2 *= 2;
        if (v2 >= 0x4000)
            return;
    }
}

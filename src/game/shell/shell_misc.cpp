// ============================================================================
// shell_misc.cpp - small shell.o free functions
// (DecodePanel / FEManagerSetFont / GetPlayersTank)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/client_types.h"

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C
extern FEManager g_femanager;
extern const char* const defaultFileName;

class PakFile;

// ============================================================================
// FEManager methods from FEManager.cpp (IDA 0x58DD60 and 0x58DE00).
// ============================================================================

void FEManager::SetFont(nglFont* f, const char* font_filename)
{
    char tmp[32];
    strncpy(tmp, font_filename, 0x1Fu);
    tmp[31] = 0;
    font_index Font = FindFont(tmp, true);
    if (Font == FONT_NORMAL)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
        AeAssert::gCurrentLine = 376;
        AeAssert::gCurrentExpr = "fi != INVALID_FONT";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid font index found"))
            __debugbreak();
    }
    else
    {
        fonts[Font] = f;
        fontsLoaded[Font] = f != nullptr;
    }
}

void FEManager::HandlePanelFilePointer(const char* name, PanelFile* pf,
                                       TPakId pakId)
{
    ae_sized_array<PanelFileUser*, 12> users;
    GetPanelFileUsers(name, users);
    for (int i = 0; i < users.m_size; ++i)
    {
        if (i >= 0xC)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        users.m_elements[i]->SetPanelFile(pf);
    }
    if (mNumPanels >= 100)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEManager.cpp";
        AeAssert::gCurrentLine = 1526;
        AeAssert::gCurrentExpr = "mNumPanels < 100";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    int panelIndex = mNumPanels;
    if (panelIndex < 100)
    {
        mPanelArray[panelIndex].mPakId = pakId;
        mPanelArray[mNumPanels++].mPanelFile = pf;
    }
}

// ea: 0x00593F00
void DecodePanel(const char* name, unsigned char* data, int size, TPakId pakId,
                 PakFile* /*unused*/)
{
    PanelFile* v4 = (PanelFile*)mem_heap_malloc(0x60u);
    PanelFile* v5;
    if (v4 != nullptr)
    {
        v4->pquads.mElements = nullptr;
        v4->pquads.mCapacity = 0;
        v4->pquads.mSize = 0;
        v4->ptext.mElements = nullptr;
        v4->ptext.mCapacity = 0;
        v4->ptext.mSize = 0;
        v4->indexHidden = -1;
        v4->hideText = false;
        v5 = v4;
    }
    else
    {
        v5 = nullptr;
    }
    v5->Load(name, data, size);
    g_femanager.HandlePanelFilePointer(name, v5, pakId);
}

// ea: 0x0058DF00
void FEManagerSetFont(nglFont* f, const char* font_filename)
{
    g_femanager.SetFont(f, font_filename);
}

// ea: 0x00588920 (IGOWidget.cpp)
DbLinkedHandle<EntityHandleDb, Entity> GetPlayersTank()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr && Player->client != nullptr
        && Player->client->pers.playerState == 3
        && (Player->client->ps.eFlags & 0x100000) != 0)
    {
        if (EntityHandleDb::sInst.GetObject(Player->r.mOwner.mHandle.mVal)
            == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\IGOWidget.cpp";
            AeAssert::gCurrentLine = 1849;
            AeAssert::gCurrentExpr = "*GetPlayer()->r.mOwner";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vehicle"))
                __debugbreak();
        }
        DbLinkedHandle<EntityHandleDb, Entity> result;
        result.mHandle.mVal = Player->r.mOwner.mHandle.mVal;
        return result;
    }
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle.mVal = 0;
    return result;
}

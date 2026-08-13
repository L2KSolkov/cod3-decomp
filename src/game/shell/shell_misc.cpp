// ============================================================================
// shell_misc.cpp - small shell.o free functions
// (DecodePanel / FEManagerSetFont / GetPlayersTank)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/client_types.h"

extern void* mem_heap_malloc(unsigned int size);  // core.o
extern int currCl;                                // ?currCl@@3HA @ 0xF1579C
extern FEManager g_femanager;

class PakFile;

// ============================================================================
// FEManager::SetFont / HandlePanelFilePointer are declared in sv_stubs.h but
// defined in the FEManager batch; /FORCE tolerates the unresolveds for now.
// ============================================================================

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

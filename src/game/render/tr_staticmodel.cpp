// ============================================================================
// tr_staticmodel.cpp - render.o static-model cell insertion (tr_staticmodel.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <stdint.h>

// AeAssert (game.o)
namespace AeAssert {
enum ECoderId { COD3 = 0, ARO = 1 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

class StaticModel;
class PoolAllocator;

// trStaticModelList_t (8 bytes)
struct trStaticModelList_t {
    StaticModel* model;          // +0x00
    trStaticModelList_t* next;   // +0x04
    static PoolAllocator* sAllocator;  // ?sAllocator@trStaticModelList_t@@2PAVPoolAllocator@@A @ 0xF7443C
};

class PoolAllocator {
public:
    void* Allocate(unsigned int s, bool forceHeapAlloc);  // ?Allocate@PoolAllocator@@QAEPAXI_N@Z
};

// BspCell view (staticModels +0x34)
struct BspCell {
    uint8_t _pad[0x34];
    trStaticModelList_t* staticModels;  // +0x34
};

// BspTree view (mCells +0x18)
class BspTree {
public:
    uint8_t _pad[0x18];
    unsigned int mCellsSize;   // +0x18
    BspCell* mCellsList;       // +0x1C
};

// world_t view (bspTree +0x100)
struct world_t {
    uint8_t _pad[0x100];
    BspTree* bspTree;          // +0x100
};

PoolAllocator* trStaticModelList_t::sAllocator;

// ============================================================================
// R_AddModelToCell - ea: 0x006C6CE0
// ============================================================================
void R_AddModelToCell(world_t* world, StaticModel* psm, int cellNum)
{
    if (psm == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
        AeAssert::gCurrentLine = 26;
        AeAssert::gCurrentExpr = "psm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (cellNum < 0 || cellNum >= (int)world->bspTree->mCellsSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\tr_staticmodel.cpp";
        AeAssert::gCurrentLine = 27;
        AeAssert::gCurrentExpr =
            "cellNum >= 0 && cellNum < world->bspTree->mCells.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BspCell* v3 = &world->bspTree->mCellsList[cellNum];
    StaticModel** p_model = &v3->staticModels->model;
    if (p_model == nullptr || *p_model != psm)
    {
        trStaticModelList_t* v5 = (trStaticModelList_t*)
            trStaticModelList_t::sAllocator->Allocate(8u, false);
        v5->model = psm;
        v5->next = v3->staticModels;
        v3->staticModels = v5;
    }
}

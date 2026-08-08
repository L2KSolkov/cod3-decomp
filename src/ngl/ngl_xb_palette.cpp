// ============================================================================
// ngl_xb_palette.cpp - NGL Xbox palette implementation (9 funcs).
// Source: src/xbox/ngl_xb_palette.cpp (ngl_xboxr)
// Verified against IDA (ngl_xboxr:ngl_xb_palette.o).
// ============================================================================

#include "ngl/nglPalette.h"

// ============================================================================
// Cross-object externs
// ============================================================================
extern void* tlMemAlloc(unsigned int Size, unsigned int Align, unsigned int Flags);
extern void tlMemFree(void* Ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);
extern void nglDxUnbindTexStages(void);

// ngl_dx_core.o (data, not yet ported)
extern _D3DCAPS8 nglDxCaps;

// ============================================================================
// Globals (data)
// ============================================================================
nglPalette** nglPrevPal = NULL;

// ============================================================================
// nglDxInitPalette - ea: 0x853040
// ============================================================================
void nglDxInitPalette() {
    nglPrevPal = (nglPalette**)tlMemAlloc(16 * nglDxCaps.MaxSimultaneousTextures, 8u, 0x1000000);
    for (unsigned int result = 0; result < nglDxCaps.MaxSimultaneousTextures; ++result)
        nglPrevPal[result] = NULL;
}

// ============================================================================
// nglDxUnbindPalettes - ea: 0x853090
// ============================================================================
void nglDxUnbindPalettes() {
    for (unsigned int i = 0; i < nglDxCaps.MaxSimultaneousTextures; ++i) {
        D3DDevice_SetPalette(i, NULL);
        nglPrevPal[i] = NULL;
    }
}

// ============================================================================
// nglCreatePalette - ea: 0x8530C0
// ============================================================================
nglPalette* nglCreatePalette(nglPaletteFormat PalFormat, unsigned int NEntries,
                             void* CreateInPlaceAddress) {
    nglPalette* v3 = (nglPalette*)tlMemAlloc(0x10u, 8u, 0x1000000);
    v3->Data = NULL;
    v3->NEntries = NEntries;
    v3->CreatedInPlace = 0;
    v3->DXPalette = NULL;

    if (PalFormat != NGLPAL_A8R8G8B8 && _tlAssert("src/xbox/ngl_xb_palette.cpp", 40,
            "PalFormat == NGLPAL_A8R8G8B8", "XBox only supports NGLPAL_A8R8G8B8 !"))
        __debugbreak();

    _D3DPALETTESIZE v4 = D3DPALETTE_256;
    switch (NEntries) {
    case 0x20:
        v4 = D3DPALETTE_32;
        break;
    case 0x40:
        v4 = D3DPALETTE_64;
        break;
    case 0x80:
        v4 = D3DPALETTE_128;
        break;
    case 0x100:
        v4 = D3DPALETTE_256;
        break;
    default:
        if (_tlAssert("src/xbox/ngl_xb_palette.cpp", 50, "false",
                      "nglCreatePalette: Invalid #entries !"))
            __debugbreak();
        break;
    }

    void* v5;
    if (CreateInPlaceAddress != NULL) {
        v3->CreatedInPlace = 1;
        v5 = CreateInPlaceAddress;
    } else {
        v5 = tlMemAlloc(4 * NEntries, 0x40u, 0x1030000);
        v3->CreatedInPlace = 0;
    }
    v3->Data = (unsigned int*)v5;

    D3DPalette* v6 = (D3DPalette*)tlMemAlloc(0xCu, 8u, 0x1000000);
    v3->DXPalette = v6;
    XGSetPaletteHeader(v4, v6, NULL);
    D3DResource_Register((D3DResource*)v6, v3->Data);
    *(unsigned int*)v3->DXPalette = 0x30001;
    return v3;
}

// ============================================================================
// nglDestroyPalette - ea: 0x8532D0
// ============================================================================
void nglDestroyPalette(nglPalette* Palette) {
    D3DResource_BlockUntilNotBusy((D3DResource*)Palette->DXPalette);
    nglDxUnbindTexStages();
    if (!Palette->CreatedInPlace) {
        tlMemFree(Palette->Data);
        D3DPalette* DXPalette = Palette->DXPalette;
        Palette->Data = NULL;
        tlMemFree(DXPalette);
        Palette->DXPalette = NULL;
        if (!Palette->CreatedInPlace)
            tlMemFree(Palette);
    }
}

// ============================================================================
// nglPalette::Lock - ea: 0x853320
// ============================================================================
void nglPalette::Lock() {
    Data = (unsigned int*)D3DPalette_Lock2(DXPalette, 0);
}

// ============================================================================
// nglPalette::Unlock - ea: 0x853340
// ============================================================================
void nglPalette::Unlock() {
}

// ============================================================================
// nglPalette::SetEntry32 - ea: 0x853350
// ============================================================================
void nglPalette::SetEntry32(unsigned int Index, unsigned int Color) {
    Data[Index] = Color;
}

// ============================================================================
// nglPalette::GetEntry32 - ea: 0x853370
// ============================================================================
unsigned int nglPalette::GetEntry32(unsigned int Index) {
    return Data[Index];
}

// ============================================================================
// nglBindPalette - ea: 0x853380
// ============================================================================
void nglBindPalette(unsigned int Stage, nglPalette* Palette) {
    if (Palette != nglPrevPal[Stage]) {
        D3DDevice_SetPalette(Stage, Palette->DXPalette);
        nglPrevPal[Stage] = Palette;
    }
}

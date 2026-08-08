// ============================================================================
// nglPalette - NGL palette wrapper (16 bytes, verified against IDA).
// Source: c:\cod\code\tl\ngl\include\nglPalette.h
// Data: nglPrevPal (ngl_xb_palette.o).
// ============================================================================

#ifndef COD3_NGL_NGL_PALETTE_H
#define COD3_NGL_NGL_PALETTE_H

#include "d3d8.h"

enum nglPaletteFormat {
    NGLPAL_A8R8G8B8 = 0,
};

struct nglPalette {
    unsigned int* Data;            // +0x00
    unsigned int  NEntries;        // +0x04
    int           CreatedInPlace;  // +0x08
    D3DPalette*   DXPalette;       // +0x0C

    void Lock();               // ea: 0x853320
    void Unlock();             // ea: 0x853340
    void SetEntry32(unsigned int Index, unsigned int Color);  // ea: 0x853350
    unsigned int GetEntry32(unsigned int Index);              // ea: 0x853370
};
static_assert(sizeof(nglPalette) == 0x10, "nglPalette size mismatch");

// ngl_xb_palette.o (data, defined in ngl_xb_palette.cpp)
extern nglPalette** nglPrevPal;

// ngl_xb_palette.o (functions)
void nglDxInitPalette();
void nglDxUnbindPalettes();
nglPalette* nglCreatePalette(nglPaletteFormat PalFormat, unsigned int NEntries,
                             void* CreateInPlaceAddress);
void nglDestroyPalette(nglPalette* Palette);
void nglBindPalette(unsigned int Stage, nglPalette* Palette);

#endif // COD3_NGL_NGL_PALETTE_H

// ============================================================================
// COD3 UI / Front-End Types â€” PanelFile, PanelAnimObject, FEText, FEMenu
// Reconstructed from IDA local types (PDB symbol data).
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

// Full FE type set is available; sv_stubs.h keeps its minimal FEMenuSystem
// view out when this header has been included.
#define COD3_FULL_FE_TYPES

#include "core/math_types.h"
#include "engine/broc_types.h"
#include "game/game_types.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stddef.h>
#include <stdint.h>

extern const char* const defaultFileName;  // 0xCD67AE

class nglTexture;
class nglFont;
class PanelMaterial;

// Minimal controller view (full impl in input/controller.cpp; manglings match)
class controller {
public:
    enum ButtonIndex {
        LEFTBUTTON = 0, DOWNBUTTON = 1, RIGHTBUTTON = 2, UPBUTTON = 3,
        SQUARE = 4, X = 5, CIRCLE = 6, TRIANGLE = 7, R1 = 8, L1 = 9,
        R2 = 10, L2 = 11, R3 = 12, L3 = 13, START = 14, SELECT = 15,
    };
    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };
    static controller* inst();          // ?inst@controller@@SAPAV1@XZ
    static int num_controllers;         // ?num_controllers@controller@@2HA
    void poll();                        // ?poll@controller@@QAEXXZ
    bool any_button_pressed(int* p_controller);  // ?any_button_pressed@controller@@QAE_NPAH@Z
    void button_pressed_clear_all();    // ?button_pressed_clear_all@controller@@QAEXXZ
    bool button_pressed_clear(int index, ButtonIndex btn);  // controller.o
    bool button_released_clear(int index, ButtonIndex btn); // controller.o
    bool button_pressed(ButtonIndex btn, int* p_controller);  // controller.o
    bool button_released(ButtonIndex btn, int* p_controller); // controller.o
    int  stick_value_x(StickIndex stick, int* p_controller);  // controller.o
    int  stick_value_y(StickIndex stick, int* p_controller);  // controller.o
    int  locked_port;
    bool is_locked;
};

// ============================================================================
// color32 â€” 32-bit RGBA color (4 bytes) â€” verified against IDA
// Class-tagged (V) to match the binary's mangling (??AVcolor32@@);
// anonymous union keeps the packed uint + byte component views.
// ============================================================================
class color32 {
public:
    union {
        unsigned int i;
        struct {
            uint8_t b;  // +0x00
            uint8_t g;  // +0x01
            uint8_t r;  // +0x02
            uint8_t a;  // +0x03
        } c;
    };

    color32() {}
    // ??0color32@@QAE@I@Z (anim.o 0x53A320)
    color32(unsigned int ic) { i = ic; }
};
static_assert(sizeof(color32) == 4, "color32 size mismatch");

// ============================================================================
// ae_vector<T> â€” dynamic array (12 bytes) â€” verified against IDA
// ============================================================================
template <typename T>
struct ae_vector {
    T*  mElements;  // +0x00
    int mCapacity;  // +0x04
    int mSize;      // +0x08
};
static_assert(sizeof(ae_vector<char>) == 0x0C, "ae_vector size mismatch");

// ============================================================================
// font_index â€” font selection enum (from IDA, all values verified)
// ============================================================================
#ifndef COD3_FONT_INDEX_DEFINED
#define COD3_FONT_INDEX_DEFINED
enum font_index {
    FONT_GARAMOND = 0,
    FONT_GEMFONTONE = 1,
    FONT_BUTTON = 2,
    FONT_ARIAL14 = 3,
    FONT_BIG = 4,
    FONT_NORMAL = 5,
    FONT_IMPACT = 6,
    FONT_HELVETICA_BOLD = 7,
    NUM_FONTS = 8,
    INVALID_FONT = 9,
};
#endif

// ============================================================================
// FEMENUCMD â€” menu command bit flags (from IDA, all values verified)
// ============================================================================
enum FEMENUCMD {
    FEMENUCMD_SELECT = 1,
    FEMENUCMD_START = 2,
    FEMENUCMD_UP = 4,
    FEMENUCMD_DOWN = 8,
    FEMENUCMD_LEFT = 16,
    FEMENUCMD_RIGHT = 32,
    FEMENUCMD_CROSS = 64,
    FEMENUCMD_TRIANGLE = 128,
    FEMENUCMD_SQUARE = 256,
    FEMENUCMD_CIRCLE = 512,
    FEMENUCMD_L1 = 1024,
    FEMENUCMD_R1 = 2048,
    FEMENUCMD_L2 = 4096,
    FEMENUCMD_R2 = 8192,
    FEMENUCMD_END = 16384,
};

// Forward declarations (class tags match the original binary's manglings)
class FEText;
class FEMultiLineText;
class FEMenu;
class FEMenuSystem;
class PanelFile;
class FEMenuEntry;
struct UIListBox;
struct OverlayMenu;
struct DialogMenuSystem;
struct DialogMenu;
class PanelQuad;
class FEText;

// panel_layer - quad layer enum
enum panel_layer {
    PANEL_LAYER_BACKGROUND = 0,
    PANEL_LAYER_1 = 1,
    PANEL_LAYER_PAUSE_MENU = 2,
    PANEL_LAYER_2 = 2,
    PANEL_LAYER_3 = 3,
    PANEL_LAYER_4 = 4,
    PANEL_LAYER_5 = 5,
    PANEL_LAYER_6 = 6,
    PANEL_LAYER_7 = 7,
    PANEL_LAYER_COMPASS_ICONS = 7,
    PANEL_LAYER_8 = 8,
    PANEL_LAYER_IGO = 8,
    PANEL_LAYER_TOTAL = 9,
};

enum mask_type {
    NO_MASK = 0,
    LEFT_MASK = 1,
    RIGHT_MASK = 2,
    BOTTOM_MASK = 3,
    TOP_MASK = 4,
};

// ============================================================================
// ae_array<T,N> - fixed-size array (elements only; bounds asserts in callers)
// ============================================================================
template <typename T, int CAPACITY>
struct ae_array {
    T m_elements[CAPACITY];  // +0x00

    T& operator[](int idx) { return m_elements[idx]; }
    const T& operator[](int idx) const { return m_elements[idx]; }
};

// ============================================================================
// PanelFileUser â€” panel file user base (4 bytes, vtable only)
// Size: 0x04 (4 bytes) â€” verified against IDA
// ============================================================================
class PanelFileUser {
public:
    virtual void SetPanelFile(PanelFile* pf) {}        // slot 0 0x5AEA10
    virtual void PanelFileUnloaded(PanelFile* pf) {}   // slot 1 0x5AEA20
    virtual void UpdateWidescreen(bool widescreen) {}  // slot 2 0x5AEA30
};
static_assert(sizeof(PanelFileUser) == 4, "PanelFileUser size mismatch");

// ============================================================================
// PanelAnimObject â€” animated panel element base (20 bytes)
// Size: 0x14 (20 bytes) â€” verified against IDA
// ============================================================================
class PanelAnimObject {
public:
    virtual ~PanelAnimObject() {}            // +0x00 (vfptr)
    float visibility;                        // +0x04
    float z_value;                           // +0x08
    float fade_timer;                        // +0x0C
    char  flags;                             // +0x10
    uint8_t _pad11[3];                       // +0x11

    // Vtable layout matches the binary (verified against ??_7PanelAnimObject).
    virtual void Draw() = 0;                    // +0x04 pure (__purecall)
    virtual void Update(float time_inc);        // +0x08 shell.o 0x56B6E0
    virtual void CopyFrom(const PanelAnimObject* pao);  // +0x0C 0x56B770
    virtual void StartAnim(bool on)             // +0x10 inline 0x5B1A90
    {
        flags = on ? (char)(flags | 2) : (char)(flags & 0xFD);
    }
    virtual void ResetXform()                   // +0x14 inline 0x5B1AC0
    {
        flags = (char)(flags & ~1);
    }
protected:
    virtual void Animate(math::Mat43*, float) = 0;  // +0x18 pure
public:
    virtual void SetWidescreenAlign(short wa);       // +0x1C 0x56B7A0
    virtual void SetZvalue(float z, panel_layer layer);  // +0x20 0x56B7D0
    virtual void StartFade(bool start, bool fade_in,
                           float time);              // +0x24 0x56B820
    virtual void SetZvalueAbs(float z)               // +0x28 inline 0x5AD5B0
    {
        z_value = z;
    }
    virtual float GetZvalue()                        // +0x2C inline 0x5AD5D0
    {
        return z_value;
    }
    virtual float GetY()                             // +0x30 inline 0x5AD5E0
    {
        return 0.0f;
    }
    virtual bool IsFading()                          // +0x34 inline 0x5B1AD0
    {
        return (flags & 0x10) != 0 || (flags & 0x20) != 0;
    }
    virtual bool IsShown()                           // +0x38 inline 0x5B1AE0
    {
        return (flags & 1) != 0;
    }
    virtual void SetShown(bool shown)                // +0x3C inline 0x5B1AF0
    {
        flags = shown ? (char)(flags | 1) : (char)(flags & 0xFE);
    }
    virtual void SetColor(color32 c) = 0;            // +0x40 pure
    virtual color32 GetColor() = 0;                  // +0x44 pure
};
static_assert(sizeof(PanelAnimObject) == 0x14, "PanelAnimObject size mismatch");
static_assert(offsetof(PanelAnimObject, visibility) == 0x04, "PanelAnimObject::visibility offset mismatch");

// ============================================================================
// PanelQuad â€” panel quad (72 bytes)
// Size: 0x48 (72 bytes) â€” verified against IDA
// ============================================================================
class PanelQuadSection;

// ============================================================================
// PQArcMaskingInfo - quad arc-mask data (52 bytes) - verified against IDA
// ============================================================================
struct PQArcMaskingInfo {
    float arc_center_x;       // +0x00
    float arc_center_y;       // +0x04
    float arc_center_u;       // +0x08
    float arc_center_v;       // +0x0C
    float arc_max_angle;      // +0x10
    float arc_start_angle;    // +0x14
    float arc_radius_x;       // +0x18
    float arc_radius_y;       // +0x1C
    float arc_radius_u;       // +0x20
    float arc_radius_v;       // +0x24
    float arc_start_ang_offset;  // +0x28
    int   arc_start_position;    // +0x2C
    bool  arc_clockwise;         // +0x30
    bool  arc_no_offset;         // +0x31
};
static_assert(sizeof(PQArcMaskingInfo) == 0x34,
              "PQArcMaskingInfo size mismatch");

class PanelQuad : public PanelAnimObject {
public:
    Broc::vector   center_point;              // +0x14
    ae_vector<PanelQuadSection*> pqs;         // +0x20 (12 bytes)
    PQArcMaskingInfo* am_info;                // +0x2C
    float          rotation;                  // +0x30
    float          sc_x;                      // +0x34
    float          sc_y;                      // +0x38
    unsigned int   quadMapFlags;              // +0x3C
    unsigned int   quadBlendModeType;         // +0x40
    Broc::string   name;                      // +0x44

    PanelQuad();  // ??0PanelQuad@@QAE@XZ (shell.o 0x58B780)
    PanelQuad(char* name);  // ??0PanelQuad@@QAE@PAD@Z (shell.o 0x58B840)

    // Vftable order verified against ??_7PanelQuad (55 slots).
    virtual ~PanelQuad();                     // slot 0 0x590F70
    virtual void Draw();                      // slot 1 0x579BC0
protected:
    virtual void Animate(math::Mat43* mat, float vis);  // slot 6 0x57AB70
public:
    virtual void SetZvalueAbs(float z);       // slot 10 0x57A530
    virtual float GetY() { return center_point.y; }  // slot 12 inline 0x5B64B0
    virtual void SetColor(color32 c);         // slot 16 0x57A1F0
    virtual color32 GetColor();               // slot 17 0x5B6620
    virtual void CopyFrom(const PanelQuad* pq);  // slot 18 0x58B930
    virtual void InstanceFrom(const PanelQuad* pq);  // slot 19 0x5799E0
    virtual void Init(Broc::vector* xy, color32* col, panel_layer lay,
                      float z, const char* filename);  // slot 20 0x58BAC0
    virtual void Load(PanelMaterial* mats, unsigned char* buffer, int& index,
                      const math::Mat43* parent_matrix);  // slot 21 0x58BC10
    virtual void Rotate(float rx, float ry, float r,
                        bool absolute);      // slot 22 0x579D40
    virtual void Rotate(float r, bool absolute)  // slot 23 inline 0x5B6350
    {
        Rotate(center_point.x, center_point.y, r, absolute);
    }
    virtual void Scale(float sx, float sy, float scx, float scy,
                       bool absolute);       // slot 24 0x579E70
    virtual void Scale(float x, float y, float s, bool absolute)  // slot 25 inline 0x5B63E0
    {
        Scale(x, y, s, s, absolute);
    }
    virtual void Scale(float sx, float sy, bool absolute)  // slot 26 inline 0x5B63B0
    {
        Scale(center_point.x, center_point.y, sx, sy, absolute);
    }
    virtual void Scale(float s, bool absolute)  // slot 27 inline 0x5B6380
    {
        Scale(center_point.x, center_point.y, s, s, absolute);
    }
    virtual void ScaleAbsoluteCenter(float scx,
                                     float scy);  // slot 28 0x57A050
    virtual void ScaleAbsoluteCenter(float s)  // slot 29 inline 0x5B6410
    {
        ScaleAbsoluteCenter(s, s);
    }
    virtual void SetTexture(nglTexture* tex);  // slot 30 0x57A170
    virtual void SetMaterialFlags(unsigned int mapflags);  // slot 31 0x56AB90
    virtual void SetColorNA(color32 c);        // slot 32 0x57A2B0
    virtual void SetAlpha(int pqsIdx, int vertIdx,
                          float alpha);        // slot 33 0x5B65A0
    virtual void SetAlpha(float alpha);        // slot 34 0x57A370
    virtual void SetVisibility(float alpha);   // slot 35 0x57A450
    virtual void SetSectionUV(int index, float* u,
                              float* v);       // slot 36 0x5B64D0
    virtual void SetPos(float x1, float y1, float x2,
                        float y2);            // slot 37 0x56ABA0
    virtual void SetPos(float* x, float* y);  // slot 38 0x583DF0
    virtual void SetCenterPos(float cx, float cy)  // slot 39 inline 0x5B6430
    {
        Shift(cx - center_point.x, cy - center_point.y);
    }
    virtual void ResetToInitialXY();          // slot 40 0x584250
    virtual void GetPos(float* x, float* y);  // slot 41 0x57A5C0
    virtual void GetCenterPos(float& cx, float& cy)  // slot 42 inline 0x5B6470
    {
        cx = center_point.x;
        cy = center_point.y;
    }
    virtual float GetCenterX() { return center_point.x; }  // slot 43 inline 0x5B6490
    virtual float GetCenterY() { return center_point.y; }  // slot 44 inline 0x5B64A0
    virtual nglTexture* GetTexture();          // slot 45 0x5B6530
    virtual float GetRotation() { return rotation; }  // slot 46 inline 0x5B64C0
    virtual color32 GetColor(int pqsIdx,
                             int vertIdx);    // slot 47 0x5B66C0
    virtual float GetWidth()                  // slot 48 inline 0x5B6710
    {
        return GetMax().x - GetMin().x;
    }
    virtual float GetInitialWidth()           // slot 49 inline 0x5B6750
    {
        return GetInitialMax().x - GetInitialMin().x;
    }
    virtual float GetHeight()                 // slot 50 inline 0x5B6790
    {
        return GetMax().y - GetMin().y;
    }
    virtual float GetInitialHeight()          // slot 51 inline 0x5B67D0
    {
        return GetInitialMax().y - GetInitialMin().y;
    }
    virtual void Shift(float off_x, float off_y);  // slot 52 0x57A6A0
    virtual void ShiftXYInitial(float off_x,
                                float off_y);      // slot 53 0x57A7E0
    virtual void SetXYInitialToCurrentPos();       // slot 54 0x57A8A0

    static PanelQuad* Clone(PanelQuad* pPQ);  // ?Clone@PanelQuad@@SAPAV1@PAV1@@Z 0x58C450
    void Mask(float percent, mask_type maskType,
              float uv_width);  // ?Mask@PanelQuad@@QAEXMW4mask_type@@M@Z 0x579C40
    void SetBlend(unsigned int type);          // 0x56AB80
    void FattenMeForPS2();                     // 0x56AC00 (empty)
    void FattenMeForGC();                      // 0x56AC10 (empty)
    void FormatHUDForSplitScreen(int viewport, int old_viewport,
                                 int justification, float just_width,
                                 float just_height);  // 0x56AC20
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x57A940
    void MoveForSplitScreen(int viewport, int old_viewport);    // 0x57A9C0
    void FattenMeForWidescreen(bool widescreen, float x);       // 0x57AA40
    Broc::vector GetMax();        // shell.o 0x57AC40
    Broc::vector GetMin();        // shell.o 0x57AF20
    Broc::vector GetInitialMax(); // shell.o 0x57B200
    Broc::vector GetInitialMin(); // shell.o 0x57B490
};
static_assert(sizeof(PanelQuad) == 0x48, "PanelQuad size mismatch");
static_assert(offsetof(PanelQuad, center_point) == 0x14, "PanelQuad::center_point offset mismatch");
static_assert(offsetof(PanelQuad, pqs) == 0x20, "PanelQuad::pqs offset mismatch");

// ============================================================================
// PanelMaterial â€” per-quad material (16 bytes) â€” verified against IDA
// (ReadPanelMaterial 0x57BEE0: texture +0, filename +4, color +8, hasmap +0xC)
// ============================================================================
class PanelMaterial {
public:
    nglTexture* texture;         // +0x00
    const char* filename;        // +0x04
    color32     color;           // +0x08
    bool        hasmap;          // +0x0C
    bool        bilinearfilter;  // +0x0D
    bool        wrapu;           // +0x0E
    bool        wrapv;           // +0x0F

    PanelMaterial() : texture(nullptr), filename(nullptr), hasmap(false),
                      bilinearfilter(false), wrapu(false), wrapv(false)
    {
        color.i = 0;
    }
};
static_assert(sizeof(PanelMaterial) == 0x10, "PanelMaterial size mismatch");

// ============================================================================
// PanelQuadSection â€” quad section (104 bytes) â€” verified against IDA
// ============================================================================
class PanelQuadSection {
public:
    struct PQVert {
        float        X;      // +0x00
        float        Y;      // +0x04
        float        U;      // +0x08
        float        V;      // +0x0C
        unsigned int Color;  // +0x10
    };
    struct QuadData {
        PQVert Verts[4];  // +0x00 (80 bytes)
        float  Z;         // +0x50
        nglTexture* Tex;  // +0x54
    };

    short    x_initial[4];  // +0x00
    short    y_initial[4];  // +0x08
    QuadData quad;          // +0x10 (88 bytes)

    PanelQuadSection();            // shell.o 0x5696D0
    Broc::vector GetMax();            // shell.o 0x569C60
    Broc::vector GetMin();            // shell.o 0x569D00
    Broc::vector GetInitialMax();     // shell.o 0x569DA0
    Broc::vector GetInitialMin();     // shell.o 0x569E50
    Broc::vector GetMaxUV();          // shell.o 0x569F00
    Broc::vector GetMinUV();          // shell.o 0x569FB0
    void SetInitialXY(Broc::vector* tmp_initial);  // shell.o 0x5696F0
    void SetXYInitialToCurrentPos();  // shell.o 0x569720
    void Rotate(float rotate_x, float rotate_y,
                float rotation);      // shell.o 0x569750
    void Scale(float sx, float sy, float scx,
               float scy);            // shell.o 0x5697E0
    void SetUV(Broc::vector* uv);                  // shell.o 0x569B30
    void SetUV(float* u, float* v);                // shell.o 0x569AF0
    void SetPos(Broc::vector* xy);                 // shell.o 0x569BB0
    void SetPos(float* x, float* y);               // shell.o 0x569B70
    void ResetToInitialXY();                       // shell.o 0x569BF0
    void Shift(float off_x, float off_y);          // shell.o 0x56A050
    void ShiftXYInitial(float off_x,
                        float off_y);              // shell.o 0x56A0E0
    void Fatten(float fatten_width,
                float about_x);                    // shell.o 0x56A280
    void Fatten(float fatten_width);               // shell.o 0x579930
    void AddPQSection(Broc::vector* xy, Broc::vector* uv, color32* col,
                      float z);                    // shell.o 0x579550
    void SetColorVert(int i, color32 c);           // shell.o 0x569AB0
    void SetColorNAVert(int i, color32 c);         // shell.o 0x579800
    void SetAlphaVert(int i, float alpha);         // shell.o 0x579840
    void CopyFrom(PanelQuadSection* pSrc);         // shell.o 0x56A810
    void ScaleAbsoluteCenter(float sx, float sy, float scx,
                             float scy);           // shell.o 0x5698B0
    color32 GetColor(int index);                   // shell.o 0x579900
    void Animate(math::Mat43* xform, float z_value,
                 bool xform_was_set);              // shell.o 0x56A130
    void Draw(unsigned int type, unsigned int mapflags);  // shell.o 0x56A2F0
    void SetVisibility(int i, float vis);          // shell.o 0x5798A0
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x56A3B0
    void MoveForSplitScreen(int viewport, int old_viewport);    // 0x56A690
    void Mask(float mask, mask_type type, float uv_width,
              float scale);  // ?Mask@PanelQuadSection@@QAEXMW4mask_type@@MM@Z 0x579640
};
static_assert(sizeof(PanelQuadSection::PQVert) == 20,
              "PQVert size mismatch");
static_assert(sizeof(PanelQuadSection) == 104,
              "PanelQuadSection size mismatch");

// ============================================================================
// FloatingPQ â€” screen-projected quad (84 bytes) â€” verified against IDA
// ============================================================================
struct FloatingPQ : PanelQuad {
    Broc::vector location_3d;  // +0x48

    FloatingPQ(char* n);          // shell.o 0x591070
    virtual void UpdateInScene(); // shell.o 0x56AD40
};
static_assert(sizeof(FloatingPQ) == 84, "FloatingPQ size mismatch");

// ============================================================================
// PanelFile â€” panel definition file (96 bytes)
// Size: 0x60 (96 bytes) â€” verified against IDA
// ============================================================================
class PanelFile {
public:
    ae_vector<PanelQuad*> pquads;      // +0x00 (12 bytes)
    ae_vector<FEText*>    ptext;       // +0x0C (12 bytes)
    int       indexHidden;             // +0x18
    bool      hideText;                // +0x1C
    uint8_t   _pad1D[3];               // +0x1D
    char      mName[64];               // +0x20

    // shell.o members (?Clone@PanelFile@@QAEPAV1@XZ etc.)
    PanelFile();  // ??0PanelFile@@QAE@XZ (shell.o 0x5843D0)
    PanelFile* Clone();
    PanelQuad* GetPointer(const char* search_name);
    FEText* GetTextPointer(const char* search_name);
    void Draw();
    void UpdateSplitScreen(int viewport, int old_viewport);
    void UpdateWidescreen(bool widescreen, float about_x);
    void Update(float time_inc);                 // shell.o 0x57BA40
    void MoveSplitScreen(int viewport, int old_viewport);  // 0x57BCB0
    void PreMashFixup();                         // shell.o 0x56ADC0
    void PostUnmashFixup(panel_layer layer);     // shell.o 0x57B730
    void ReadPanelMaterial(PanelMaterial& mat, unsigned char* buffer,
                           int& index);          // shell.o 0x57BEE0
    int FindPanelQuadByPointer(PanelQuad* the_pointer);  // 0x57BF80
    int FindFETextByPointer(FEText* the_pointer);        // 0x57BFC0
    bool Load(const char* filename, unsigned char* buffer,
              int buffer_size);                  // shell.o 0x593A00
    PanelAnimObject* FindAnimObject(const char* search_name);  // 0x5948C0
    ~PanelFile();  // ??1PanelFile@@QAE@XZ
    void LoadPanelGeom(unsigned char* buffer, int& index,
                       const math::Mat43* parent_matrix);  // 0x5910E0
    void LoadPanelObject(unsigned char* buffer, int& index,
                         const math::Mat43* parent_matrix,
                         const char* name, short widescreen_align);  // 0x58C770
    void LoadPanelText(unsigned char* buffer, int& index,
                       const math::Mat43* parent_matrix,
                       const char* name, short widescreen_align);  // 0x58C8E0
    void LoadPanelText2(unsigned char* buffer, int& index,
                        const math::Mat43* parent_matrix,
                        const char* name, short widescreen_align);  // 0x58CCF0
    void HideQuad(const char* name);              // 0x59ABC0
    void SetQuadVisible(const char* name, bool visible);  // 0x59AC10
private:
    void Cleanup();  // ??0PanelFile... Cleanup@PanelFile@@AAEXXZ (0x58C4C0)
};
static_assert(sizeof(PanelFile) == 0x60, "PanelFile size mismatch");
static_assert(offsetof(PanelFile, pquads) == 0x00, "PanelFile::pquads offset mismatch");
static_assert(offsetof(PanelFile, ptext) == 0x0C, "PanelFile::ptext offset mismatch");
static_assert(offsetof(PanelFile, mName) == 0x20, "PanelFile::mName offset mismatch");

// ============================================================================
// FETextFlashInfo â€” text flash animation state (20 bytes)
// Size: 0x14 (20 bytes) â€” verified against IDA
// ============================================================================
struct FETextFlashInfo {
    color32 flash_color;       // +0x00
    float   flash_timer;       // +0x04
    float   flash_intensity;   // +0x08
    float   flash_period;      // +0x0C
    bool    reset;             // +0x10
    uint8_t _pad11[3];         // +0x11

    FETextFlashInfo(color32 col, float period);  // shell.o 0x56B940
    void Update(float time_inc);                 // shell.o 0x56B970
    color32 GetColor(color32 normal_color);      // shell.o 0x56B9B0
    void Reset();                                // shell.o 0x56BA70
};
static_assert(sizeof(FETextFlashInfo) == 0x14, "FETextFlashInfo size mismatch");

// ============================================================================
// FEText â€” text element (112 bytes)
// Size: 0x70 (112 bytes) â€” verified against IDA
// ============================================================================
class FEText : public PanelAnimObject {
public:
    FETextFlashInfo* flash_info;            // +0x14
    font_index       font;                  // +0x18
    Broc::string     text;                  // +0x1C
    Broc::vector     xy_initial;            // +0x20
    Broc::vector     xy;                    // +0x2C
    Broc::vector     scale;                 // +0x38
    Broc::vector     scale_init;            // +0x44
    Broc::vector     scale_unselected;      // +0x50
    color32          color1;                // +0x5C
    color32          color_unselected;      // +0x60
    Broc::string     name;                  // +0x64
    int              panel_text_index;      // +0x68
    int16_t          flags;                 // +0x6C
    uint8_t          _pad6E[2];             // +0x6E

    // shell.o inline COMDATs (verified manglings)
    virtual void Draw() { Draw(false); }  // ?Draw@FEText@@UAEXXZ (0x5AD830)
    virtual void SetColor(color32 c) { SetNoFlash(c); }  // 0x5ADC20
    virtual void SetTextNoLocalize(const char* s) { text = s; }
    virtual void SetPos(float x, float y) { xy.x = x; xy.y = y; }
    virtual void SetY(float y) { xy.y = y; }
    virtual void SetAlpha(float a) { (void)a; }
    virtual void SetColorMenuItem(color32 normal, color32 selected)
    {
        color1 = normal;
        color_unselected = selected;
    }
    virtual float GetScaleX() const { return scale.x; }
    virtual color32 GetColor() { return color1; }
    virtual color32 GetUnselectedColor() { return color_unselected; }
    virtual float GetX() { return xy.x; }
    virtual float GetY() { return xy.y; }
    virtual bool GetFlag(int f) { return (flags & f) != 0; }
    virtual bool IsOnMenu() { return GetFlag(2); }       // 0x5AD860
    virtual void AddedToMenu(bool add) { SetFlag(2, add); }  // 0x5AD840
    virtual bool IsMultiLineObject() { return false; }    // 0x5AD870
    virtual Broc::string GetName() { return name; }      // 0x5ADC50
    virtual void SetName(const char* n) { name = n; }    // 0x5ADAA0
    virtual Broc::string GetText() { return text; }      // 0x5ADC80
    virtual void SetPanelTextIndex(int the_index)        // 0x5ADC10
    {
        panel_text_index = the_index;
    }
    virtual void SetFont(font_index f) { font = f; }     // 0x5AD8B0
    virtual void SetEvenNumberSpacing(bool on)           // 0x5AD8C0
    {
        flags = (int16_t)(on ? (flags | 4) : (flags & ~4));
    }
    virtual void SetScale(float s)                       // 0x5AD8E0
    {
        scale.x = s;
        scale.y = s;
        scale_unselected.x = s;
        scale_unselected.y = s;
    }
    virtual void SetScale(float sx, float sy)            // 0x5AD950
    {
        scale.x = sx;
        scale.y = sy;
        scale_unselected.x = sx;
        scale_unselected.y = sy;
        scale_unselected.z = 0.0f;
    }
    virtual void SetScaleInit(float sx, float sy)        // 0x5ADA50
    {
        scale_init.x = sx;
        scale_init.y = sy;
        scale_init.z = 0.0f;
    }
    virtual void Shift(float offx, float offy)           // 0x5ADAB0
    {
        SetPos(GetX() + offx, GetY() + offy);
    }
    virtual void ShiftXYInitial(Broc::vector offset)     // 0x5ADAF0
    {
        xy_initial.x += offset.x;
        xy_initial.y += offset.y;
        xy_initial.z += offset.z;
    }
    virtual void SetX(float posX) { xy.x = posX; }       // 0x5ADBA0
    virtual void SetScaleMenuItem(float s_selected,      // 0x5AD9D0
                                  float s_unselected)
    {
        scale.x = s_selected;
        scale.y = s_selected;
        scale.z = 0.0f;
        scale_unselected.x = s_unselected;
        scale_unselected.y = s_unselected;
        scale_unselected.z = 0.0f;
        scale_init.z = s_unselected;
    }
    virtual void SetLineSpacing(int) {}                  // 0x5ADE10 (empty)
    virtual void UpdateInScene(bool) {}                  // 0x5ADE00 (empty)
    virtual void SetLocation3D(Broc::vector) {}          // 0x5ADE30 (empty)
    virtual void SetBehaviorNF(float, float) {}          // 0x5ADE40 (empty)
    virtual void SetBehavior(bool) {}                    // 0x5ADE50 (empty)
    virtual int GetLineNum() { return 1; }               // 0x5ADD80
    virtual void AddFont(int, font_index) {}             // delegate (FEMultiLineText)
    virtual float GetScaleY() const { return scale.y; }
    virtual float GetScaleInitX() const { return scale_init.x; }
    virtual float GetScaleInitY() const { return scale_init.y; }
    virtual bool GetEvenNumberSpacing() const
    {
        return (flags & 4) != 0;
    }
    virtual int GetFlags() { return flags; }
    virtual int GetHJustify() { return flags & 0x30; }
    virtual int GetVJustify() { return flags & 0xC0; }
    virtual Broc::vector GetLocation3D()
    {
        Broc::vector v;
        v.x = v.y = v.z = 0.0f;
        return v;
    }
    virtual void SetFlag(int f, bool on)
    {
        if (on)
            flags = (int16_t)(flags | f);
        else
            flags = (int16_t)(flags & ~f);
    }

    FEText() {}  // inline default (FE subclass ctors)
    FEText(font_index f, const char* s, float x, float y, int z,
           panel_layer layer, float scale, int hJustify, int vJustify,
           color32 col);                    // shell.o 0x56BA80
    virtual ~FEText();                      // shell.o 0x56BCE0
    virtual FEText* Clone();                // shell.o 0x56BD50
    virtual void CopyFrom(FEText* fet);     // shell.o 0x56BDC0
    virtual void Update(float time_inc);    // shell.o 0x56BEF0
    virtual void UpdateForWidescreen(bool widescreen);  // shell.o 0x56BFF0
    virtual void UpdateForWidescreen(bool widescreen, int viewport);
                                            // shell.o 0x56C120
    virtual void MoveForSplitScreen(int viewport, int old_viewport);
                                            // shell.o 0x56C250
    virtual void UpdateForHUDSplitScreen(int viewport, int old_viewport,
                                         int client, float widescreen,
                                         float split);  // shell.o 0x56C380
    virtual void UpdateForSplitScreen(int viewport, int old_viewport);
                                            // shell.o 0x56C4F0
    virtual void SetHJustify(int h);        // shell.o 0x56C7F0
    virtual void SetVJustify(int v);        // shell.o 0x56C830
    virtual void SetText(const char* s);    // shell.o 0x56C870
    virtual void SetText(unsigned int hash);// shell.o 0x56C900
    virtual void SetNoFlash(color32 c);     // shell.o 0x56C930
    virtual void SetFlash(color32 c1, color32 c2, float period);
                                            // shell.o 0x56C950
    virtual void SetNoColor();              // shell.o 0x56C9D0
    static void CreateNGLColorCode(color32 c, char* dest);  // shell.o 0x56C9E0
    virtual void Draw(bool localize);       // shell.o 0x57C390
    virtual float GetWidth(const float* p); // shell.o 0x57C6E0
    virtual float GetHeight(const float* p);// shell.o 0x57C910
protected:
    virtual void Animate(math::Mat43* m, float time);  // shell.o 0x56CA30
    virtual void AdjustForJustification(float& x, float& y, float z);
                                            // shell.o 0x56CB60
public:
    font_index GetFont();
};
static_assert(sizeof(FEText) == 0x70, "FEText size mismatch");
static_assert(offsetof(FEText, font) == 0x18, "FEText::font offset mismatch");
static_assert(offsetof(FEText, text) == 0x1C, "FEText::text offset mismatch");
static_assert(offsetof(FEText, xy) == 0x2C, "FEText::xy offset mismatch");
static_assert(offsetof(FEText, scale) == 0x38, "FEText::scale offset mismatch");
static_assert(offsetof(FEText, color1) == 0x5C, "FEText::color1 offset mismatch");
static_assert(offsetof(FEText, panel_text_index) == 0x68, "FEText::panel_text_index offset mismatch");
static_assert(offsetof(FEText, flags) == 0x6C, "FEText::flags offset mismatch");

// ============================================================================
// FEMenuEntry â€” selectable menu entry (24 bytes)
// Size: 0x18 (24 bytes) â€” verified against IDA
// ============================================================================
class FEMenuEntry {
public:
    FEMenu*    menu;                     // +0x04
    int16_t    up;                       // +0x08
    int16_t    down;                     // +0x0A
    int16_t    left;                     // +0x0C
    int16_t    right;                    // +0x0E
    FEText*    text;                     // +0x10
    char       color_scheme_index;       // +0x14
    bool       highlight;                // +0x15
    bool       disabled;                 // +0x16
    bool       must_delete_text;         // +0x17

    // Vftable order verified against ??_7FEMenuEntry (52 slots).
    virtual ~FEMenuEntry();                     // 0x5AE460
protected:
    virtual void OnHighlight(bool anim) {}      // slot 1 (0x5AE860, empty)
public:
    virtual void Load() {}                      // slot 2 (0x5AE490, empty)
    virtual short OnUp() { return up; }         // slot 3 0x5AE4F0
    virtual short OnDown() { return down; }     // slot 4 0x5AE500
    virtual short OnLeft() { return left; }     // slot 5 0x5AE510
    virtual short OnRight() { return right; }   // slot 6 0x5AE520
    virtual void Draw()                         // slot 7 0x5AE530
    {
        if (text != nullptr)
            text->Draw(highlight);
    }
    virtual void Update(float time_inc)         // slot 8 0x5AE550
    {
        if (text != nullptr)
            text->Update(time_inc);
    }
    virtual void UpdateInScene()                // slot 9 0x5AE570
    {
        if (text != nullptr)
            text->UpdateInScene(false);
    }
    virtual void CopyFrom(FEText* fet);         // slot 10 0x56FC20
    virtual void SetText(Broc::string ref)      // slot 11 0x5B1D00
    {
        if (text != nullptr)
            text->SetText(ref.mBlock != nullptr
                               ? (const char*)&ref.mBlock[1]
                               : defaultFileName);
    }
    virtual void SetText(const char* s)         // slot 12 0x5AE5E0
    {
        if (text != nullptr)
            text->SetText(s);
    }
    virtual void SetText(FEText* fet);          // slot 13 0x56FC30
    virtual void SetShown(bool on)              // slot 14 0x5AE590
    {
        if (text != nullptr)
            text->SetShown(on);
    }
    virtual void Highlight(bool h, bool anim);  // slot 15 0x56FC40
    virtual void Disable(bool d);               // slot 16 0x56FC70
    virtual bool GetDisable()                   // slot 17 0x5AE5A0
    {
        return disabled;
    }
    virtual void StartFade(bool s, bool f,      // slot 18 0x5AE5B0
                           float t)
    {
        if (text != nullptr)
            text->StartFade(s, f, t);
    }
    virtual void SetPos(float x, float y)       // slot 19 0x5AE5C0
    {
        if (text != nullptr)
            text->SetPos(x, y);
    }
    virtual void SetTextNoLocalize(char* txt)   // slot 20 0x5AE600
    {
        if (text != nullptr)
            text->SetTextNoLocalize(txt);
    }
    virtual void SetTextNoLocalize(Broc::string str)  // slot 21 0x5B1D80
    {
        if (text != nullptr)
            text->SetTextNoLocalize(str.mBlock != nullptr
                                        ? (const char*)&str.mBlock[1]
                                        : defaultFileName);
    }
    virtual void SetLocation3D(Broc::vector loc)  // slot 22 0x5AE620
    {
        if (text != nullptr)
            text->SetLocation3D(loc);
    }
    virtual void SetHJustify(int h)             // slot 23 0x5AE660
    {
        if (text != nullptr)
            text->SetHJustify(h);
    }
    virtual void SetVJustify(int v)             // slot 24 0x5AE670
    {
        if (text != nullptr)
            text->SetVJustify(v);
    }
    virtual void SetLineSpacing(int s)          // slot 25 0x5AE680
    {
        if (text != nullptr)
            text->SetLineSpacing(s);
    }
    virtual void SetFont(font_index f)          // slot 26 0x5AE6A0
    {
        if (text != nullptr)
            text->SetFont(f);
    }
    virtual void SetBehaviorNF(float x, float y)  // slot 27 0x5AE6B0
    {
        if (text != nullptr)
            text->SetBehaviorNF(x, y);
    }
    virtual void SetBehavior(bool nfb)          // slot 28 0x5AE6D0
    {
        if (text != nullptr)
            text->SetBehavior(nfb);
    }
    virtual void SetColorSchemeIndex(char csi)  // slot 29 0x5AE6F0
    {
        color_scheme_index = csi;
        AdjustColor();
    }
    virtual void SetScale(float s, float su)    // slot 30 0x5AE720
    {
        if (text != nullptr)
            text->SetScaleMenuItem(s, su);
    }
    virtual void SetScale(float s)              // slot 31 0x5AE710
    {
        if (text != nullptr)
            text->SetScale(s);
    }
    virtual void SetZ(float z, panel_layer layer)  // slot 32 0x5AE730
    {
        if (text != nullptr)
            text->SetZvalue(z, layer);
    }
    virtual Broc::string GetText()              // slot 33 0x5AE740
    {
        return text != nullptr ? text->GetText() : Broc::string();
    }
    virtual float GetWidth()                    // slot 34 0x5AE770
    {
        return text != nullptr ? text->GetWidth(nullptr) : 0.0f;
    }
    virtual float GetX()                        // slot 35 0x5AE780
    {
        return text != nullptr ? text->GetX() : 0.0f;
    }
    virtual float GetY()                        // slot 36 0x5AE790
    {
        return text != nullptr ? text->GetY() : 0.0f;
    }
    virtual int GetLineNum()                    // slot 37 0x5AE7A0
    {
        return text != nullptr ? text->GetLineNum() : 0;
    }
    virtual float GetScaleX()                   // slot 38 0x5AE7B0
    {
        return text != nullptr ? text->GetScaleX() : 0.0f;
    }
    virtual float GetScaleY()                   // slot 39 0x5AE7C0
    {
        return text != nullptr ? text->GetScaleY() : 0.0f;
    }
    virtual char GetColorSchemeIndex()          // slot 40
    {
        return color_scheme_index;
    }
    virtual color32 GetColor()                  // slot 41
    {
        return text != nullptr ? text->GetColor() : color32(0);
    }
    virtual float GetZ()                        // slot 42
    {
        return text != nullptr ? text->GetZvalue() : 0.0f;
    }
    virtual void AddFont(int index, font_index f)  // slot 43 0x5AE810
    {
        if (text != nullptr)
            text->AddFont(index, f);
    }
    virtual color32 WithAlpha(color32 c, int alpha);  // slot 44 0x56FC90
protected:
    virtual void AdjustColor(FEText* fet);      // slot 45 0x56FCC0
public:
    virtual void AdjustColor();                 // slot 46 0x56FCB0
    virtual void MoveForSplitScreen(int viewport,
                                    int old_viewport);  // slot 47 0x56FE70
    virtual void UpdateWidescreen(bool) {}      // slot 48 0x5AE830 (empty)
    virtual void OnSelect() {}                  // slot 49 0x5AE4A0 (empty)
    virtual int GetValue() { return 0; }        // slot 50 0x5AE850
    virtual void SetValue(int) {}               // slot 51 0x5AE840 (empty)

    void CommonConstructor(FEText* text, FEMenu* menu);  // shell.o 0x56FBC0
    FEMenuEntry() : menu(nullptr), up(-1), down(-1), left(-1), right(-1),
                    text(nullptr), color_scheme_index(0), highlight(false),
                    disabled(false), must_delete_text(false) {}
    FEMenuEntry(FEText* t, FEMenu* m, bool delete_me);  // inline 0x5B1C90
    FEMenuEntry(const char* text, FEMenu* m, bool floating,
                font_index ft, int nlines);     // shell.o 0x585C70
};
static_assert(sizeof(FEMenuEntry) == 0x18, "FEMenuEntry size mismatch");
static_assert(offsetof(FEMenuEntry, text) == 0x10, "FEMenuEntry::text offset mismatch");

// ============================================================================
// FEMenuColorScheme â€” menu color scheme (16 bytes)
// Size: 0x10 (16 bytes) â€” verified against IDA
// ============================================================================
struct FEMenuColorScheme {
    color32 unselect;  // +0x00
    color32 high1;     // +0x04
    color32 high2;     // +0x08
    bool    flash;     // +0x0C
    uint8_t _pad0D[3]; // +0x0D

    static bool GetInfo(char index, color32& un, color32& h1,
                        color32& h2);   // shell.o 0x56FB10
    static bool GetInfo(char index, color32& un,
                        color32& sel);  // shell.o 0x56FB50
    static int GetSchemeFromText(Broc::string& schemeText);  // shell.o 0x56FB80
};
static_assert(sizeof(FEMenuColorScheme) == 0x10, "FEMenuColorScheme size mismatch");

// ============================================================================
// FEMenuSystem â€” menu system (44 bytes)
// Size: 0x2C (44 bytes) â€” verified against IDA
// ============================================================================
class FEMenuSystem : public PanelFileUser {
public:
    FEMenu**    menus;                 // +0x04
    font_index  font;                  // +0x08
    int         size;                  // +0x0C
    int         count;                 // +0x10
    int         background;            // +0x14
    bool        drawHelpbar;           // +0x18
    uint8_t     _pad19[3];             // +0x19
    int         m_active;              // +0x1C
    char        default_color_scheme;  // +0x20
    uint8_t     _pad21[1];             // +0x21
    int16_t     button_down_flags[4];  // +0x22 (8 bytes)
    bool        is_active;             // +0x2A
    uint8_t     _pad2B[1];             // +0x2B

    // Vftable order verified against ??_7FEMenuSystem (37 slots).
    virtual ~FEMenuSystem();                        // slot 3 0x570A10
    virtual void InitAll();                         // slot 4 0x570AA0
    virtual void Add(FEMenu* m);                    // slot 5 0x570A30
    virtual void ReturnToPreviousMenu(int fallback);  // slot 6 0x570AD0
    virtual void MakeActive(int index);             // slot 7 0x570B20
    virtual void MakeActive(int index,
                            int return_to_menu);    // slot 8 0x57DDF0
    virtual void MakeActiveAndReturn(int index,
                                     int return_to);  // slot 9 0x570B80
    virtual void MakeActiveAndReturn(int index);    // slot 10 0x570B40
    virtual bool IsMenuActive(int menu);            // slot 11 0x570BC0
    virtual void ClearReturnMenu(int menu);         // slot 12 0x570BE0
    virtual void UpdateSplitScreen();               // slot 13 0x570E10
    virtual void AddOverlay(int index);             // slot 14 0x570C00
    virtual void RemoveOverlay();                   // slot 15 0x570D00
    virtual void Update(float time_inc);            // slot 16 0x570DD0
    virtual void UpdateButtonPresses();             // slot 17 0x57DFB0
    virtual void UpdateButtonDown();                // slot 18 0x570E40
    virtual void Draw();                            // slot 19 0x570E90
    virtual void Draw3D();                          // slot 20 0x570ED0
    virtual void SetDefaultColorScheme(char csi)    // slot 21 inline 0x5AE990
    {
        default_color_scheme = csi;
    }
    virtual char GetDefaultColorScheme()            // slot 22 inline 0x5AFA10
    {
        return default_color_scheme;
    }
    virtual bool IsSystemActive()                   // slot 23 inline 0x5AFA20
    {
        return is_active;
    }
    virtual int GetActiveMenu();                    // slot 24 0x571370
    virtual void SetActiveMenu(int menu);           // slot 25 0x571380
    virtual int GetCurrentClient();                 // slot 26 0x571300
    virtual int GetCurrentClientController();       // slot 27 0x571310
protected:
    virtual void OnButtonPress(int button,
                               int controller);     // slot 28 0x570F10
    virtual void OnButtonRelease(int button,
                                 int controller);   // slot 29 0x5711A0
    virtual bool GetAnalogPressed(int button,
                                  int* p_controller);  // slot 30 0x5711D0
    virtual bool GetButtonPressed(int button,
                                  int* p_controller);  // slot 31 0x571330
    virtual bool GetButtonReleased(int button,
                                   int* p_controller);  // slot 32 0x571350
    virtual int GetStickValueX(int stick,
                               int* p_controller);  // slot 33 0x571390
    virtual int GetStickValueY(int stick,
                               int* p_controller);  // slot 34 0x5713B0
    virtual int GetClientFromController(int c);     // slot 35 0x571320
    virtual void NewMenuActive() {}                 // slot 36 inline 0x5AFA30
public:
    FEMenuSystem(int s, font_index f);  // ?FEMenuSystem@@QAE@HW4font_index@@@Z 0x57DD70
    int CurrentOverlay();                           // ?CurrentOverlay@FEMenuSystem@@QAEHXZ 0x570DC0
    void SetSystemActive(bool active);  // ?SetSystemActive@FEMenuSystem@@QAEX_N@Z (sv.o 0x51E150)
};
static_assert(sizeof(FEMenuSystem) == 0x2C, "FEMenuSystem size mismatch");
static_assert(offsetof(FEMenuSystem, menus) == 0x04, "FEMenuSystem::menus offset mismatch");
static_assert(offsetof(FEMenuSystem, m_active) == 0x1C, "FEMenuSystem::m_active offset mismatch");

// ============================================================================
// FEMenu â€” menu (76 bytes)
// Size: 0x4C (76 bytes) â€” verified against IDA
// ============================================================================

// FEMenu (shell.o FEMenu.cpp) - vftable order verified against ??_7FEMenu
class FEMenu : public PanelFileUser {
public:
    FEMenuEntry** entries;                       // +0x04
    FEMenuSystem* system;                        // +0x08
    int    center_x;                             // +0x0C
    int    center_y;                             // +0x10
    int    y_distance;                           // +0x14
    int    half_height;                          // +0x18
    float  button_held_timer;                    // +0x1C
    int16_t first_vis_entry;                     // +0x20
    int16_t highlighted;                         // +0x22
    int16_t highlightedDefault;                  // +0x24
    int16_t num_entries;                         // +0x26
    int16_t max_vis_entries;                     // +0x28
    int16_t flags;                               // +0x2A
    void*  sound;                                // +0x2C (DbLinkedHandle)
    bool   lockInput;                            // +0x30
    bool   enableNavigationSound;                // +0x31
    char   button_held_down;                     // +0x32
    char   default_color_scheme;                 // +0x33
    int    mReturnMenu;                          // +0x34
    FEText* helpbar;                             // +0x38
    FEMultiLineText* helpbar1;                   // +0x3C
    FEMultiLineText* helpbar2;                   // +0x40
    FEMultiLineText* helpbar3;                   // +0x44
    PanelFile* panel;                            // +0x48

    virtual void PanelFileUnloaded(PanelFile* pf)  // slot 1 0x5B7570
    {
        Cleanup();
    }
    virtual void UpdateWidescreen(bool widescreen);  // slot 2 0x57DF20
    virtual ~FEMenu();                              // slot 3 (??_G) 0x592150
    virtual void AddEntry(int index, FEText* t,
                          bool delete_me);          // slot 4 0x57DBA0
    virtual void AddEntry(int index,
                          const char* text);        // slot 5 0x585DA0
    virtual void EnableNavigationSound(bool enable)  // slot 6 0x5AF770
    {
        enableNavigationSound = enable;
    }
    virtual void PlayNavigationSound();             // slot 7 0x57DD00
    virtual void PlayNavigationSoundWait();         // slot 8 0x58E0B0
    virtual void InputLock(bool enable)             // slot 9 0x5AF780
    {
        lockInput = enable;
    }
    virtual class FEComboBox* AddComboBox(
        int index, int numOptions, FEText* text, FEText* label,
        PanelQuad* leftArrow, PanelQuad* rightArrow);  // slot 10 0x585F30
    virtual class FEComboBox* AddComboBox(
        int index, int numOptions, FEText* text, PanelQuad* leftArrow,
        PanelQuad* rightArrow);                      // slot 11 0x585E60
    virtual class FESlider* AddSlider(int index, FEText* barText,
                                      FEText* label);  // slot 12 0x5860D0
    virtual class FESlider* AddSlider(int index, PanelQuad* bar,
                                      FEText* label);  // slot 13 0x586000
    virtual class FEDoubleEntry* AddDoubleEntry(int index, FEText* barText,
                                                FEText* label);  // slot 14 0x5861A0
    virtual class FEMenuListBox* AddListBoxEntry(int index, FEText* t,
                                                 int numLines);  // slot 15 0x58DFF0
    virtual void OnDeactivate(FEMenu* m) {}         // slot 16 (empty inline)
    virtual void Init();                            // slot 17 0x570400
    virtual void Load()                             // slot 18 0x5AF7A0
    {
        Load(false);
    }
    virtual void Load(bool floating) {}             // slot 19 0x5AF790 (empty)
    virtual void Draw();                            // slot 20 0x570550
    virtual void Draw3D() {}                        // slot 21 (empty inline)
    virtual void UpdateInScene() {}                 // slot 22 (empty inline)
    virtual void Update(float time_inc);            // slot 23 0x570660
    virtual void HighlightDefault();                // slot 24 0x570700
    virtual void Select(int entry_num, int controller)  // slot 25 0x5AF7E0
    {
        Select(entry_num);
    }
    virtual void Select(int entry_num) {}           // slot 26 0x5AF7D0 (empty)
    virtual void OnActivate(int prev)               // slot 27 0x5AF800
    {
        OnActivate();
    }
    virtual void OnActivate();                      // slot 28 0x570750
    virtual void OnSelect(int c) {}                 // slot 29 (empty inline)
    virtual void OnSquare(int c) {}                 // slot 30 (empty inline)
    virtual void OnCircle(int c) {}                 // slot 31 (empty inline)
    virtual void OnUp(int c)                        // slot 32 0x5AE910
    {
        Up();
    }
    virtual void OnDown(int c)                      // slot 33 0x5AE920
    {
        Down();
    }
    virtual void OnLeft(int c)                      // slot 34 0x5AF820
    {
        Left();
    }
    virtual void OnRight(int c)                     // slot 35 0x5AF830
    {
        Right();
    }
    virtual void OnCross(int c);                    // slot 36 0x5707D0
    virtual void OnL1(int c) {}                     // slot 37 (empty inline)
    virtual void OnR1(int c) {}                     // slot 38 (empty inline)
    virtual void OnL2(int c) {}                     // slot 39 (empty inline)
    virtual void OnTrueCircle(int c) {}             // slot 40 (empty inline)
    virtual void OnTrueTriangle(int c) {}           // slot 41 (empty inline)
    virtual void OnStart(int c) {}                  // slot 42 (empty inline)
    virtual void OnR2(int c) {}                     // slot 43 (inline 0x5AF890)
    virtual void OnAnyButtonPress(int c, int b);    // slot 44 0x570810
    virtual void OnButtonRelease(int c, int b);     // slot 45 0x570850
    virtual void OnTriangle(int c) {}               // slot 44 (empty inline)
    virtual void UpdateSplitScreen() {}             // slot 47 (empty inline)
    virtual void SetHigh(int index, bool anim);     // slot 48 0x57DC70
    virtual void SetVis(int first);                 // slot 49 0x570280
    virtual void SetDistanceBetweenEntries(int dbe)  // slot 50 0x5AF8D0
    {
        y_distance = dbe;
    }
    virtual void SetVerticalJust(bool top, bool bottom);  // slot 51 0x570350
    virtual void SetScaleThroughout(float sc);      // slot 52 0x570390
    virtual void SetZThroughout(float z,
                                panel_layer layer);  // slot 53 0x5703C0
    virtual void SetDefaultColorScheme(char csi)    // slot 54 0x5AE930
    {
        default_color_scheme = csi;
    }
    virtual char GetDefaultColorScheme()            // slot 55 0x5AF8E0
    {
        return default_color_scheme;
    }
    virtual FEMenuSystem* GetSystem()               // slot 56 0x5AF8F0
    {
        return system;
    }
    virtual int GetFlags()                          // slot 57 0x5AF900
    {
        return flags;
    }
protected:
    virtual void Up();                              // slot 58 0x570020
    virtual void Down();                            // slot 59 0x56FF00
    virtual void Left();                            // slot 60 0x570140
    virtual void Right();                           // slot 61 0x5701E0
    virtual void ButtonHeldAction();                // slot 62 0x570890
public:
    FEMenu();                                       // 0x57DA80
    FEMenu(FEMenuSystem* menuSystem, int num, int x, int y, int mve,
           int flg);                                // 0x57DAE0 ??0FEMenu@@QAE@PAVFEMenuSystem@@HHHHH@Z
    void Cleanup();                                 // ?Cleanup@FEMenu@@QAEXXZ 0x58DF20
    bool GetFlag(int f)                             // ?GetFlag@FEMenu@@QAE_NH@Z 0x5AE940
    {
        return (flags & f) != 0;
    }
    void SetFlag(int f, bool b)                     // ?SetFlag@FEMenu@@QAEXH_N@Z 0x5AE960
    {
        flags = b ? (int16_t)(flags | f) : (int16_t)(flags & ~f);
    }
protected:
    void ClearAllButtons();                         // ?ClearAllButtons@FEMenu@@IAEXXZ 0x5708C0
    void ClearButton(controller::ButtonIndex a_eButton);  // ?ClearButton@FEMenu@@IAEXW4ButtonIndex@controller@@@Z 0x570910
    void ConnectEntries(short index);               // ?ConnectEntries@FEMenu@@IAEXF@Z 0x56FE80
};
static_assert(sizeof(FEMenu) == 0x4C, "FEMenu size mismatch");
static_assert(offsetof(FEMenu, entries) == 0x04, "FEMenu::entries offset mismatch");
static_assert(offsetof(FEMenu, panel) == 0x48, "FEMenu::panel offset mismatch");

// ============================================================================
// MultiLineString â€” multiline text storage (opaque)
// ============================================================================
struct MultiLineString;

// ============================================================================
// FEMultiLineText â€” multiline text element (168 bytes)
// Size: 0xA8 (168 bytes) â€” verified against IDA
// ============================================================================
class FEMultiLineText : public FEText {
public:
    color32 button_color;              // +0x70
    float   button_scale;              // +0x74
    float   line_spacing_init;         // +0x78
    float   line_spacing;              // +0x7C
    float   button_y_offset;           // +0x80
    int     box_width;                 // +0x84
    int     line_num;                  // +0x88
    int     line_avail_num;            // +0x8C
    MultiLineString* lines;            // +0x90
    int     scroll_box_height;         // +0x94
    int     scroll_first;              // +0x98
    int     scroll_last;               // +0x9C
    float   scroll_offset;             // +0xA0
    bool    scrollable;                // +0xA4
    bool    scroll_edge_based;         // +0xA5
    bool    cut_off_if_too_long;       // +0xA6
    uint8_t _padA7[1];                 // +0xA7

    void Draw();
    void SetTextBoxNoLocalize(const char* s, int a3, int a4);
    virtual void SetTextBox(const char* reference, int w,
                            float sc_override);  // ?SetTextBox@FEMultiLineText@@UAEXPBDHM@Z 0x56E5A0
    void UpdateForSplitScreen(int viewport, int old_viewport);
    void UpdateForWidescreen(bool widescreen);
    virtual void SetNumLines(int n);      // ?SetNumLines@FEMultiLineText@@UAEXH@Z (0x56D6D0)
    virtual void SetText(const char* s);  // ?SetText@FEMultiLineText@@UAEXPBD@Z (0x56D7E0)
    virtual void SetLineSpacing(int new_spacing);  // 0x57CFA0
    virtual void SetBoxWidth(int width)            // inline 0x5B1C30
    {
        box_width = width;
    }
    virtual void SetCutOffIfTooLong(bool coitl)    // inline 0x5B1C70
    {
        cut_off_if_too_long = coitl;
    }
    static Broc::string ReplaceEndlines(Broc::string t);  // shell.o 0x56E670
    FEMultiLineText(font_index f, float x1, float y1, int z1,
                    panel_layer layer, float s, int horizJust, int vertJust,
                    color32 col);
};
static_assert(sizeof(FEMultiLineText) == 0xA8, "FEMultiLineText size mismatch");
static_assert(offsetof(FEMultiLineText, lines) == 0x90, "FEMultiLineText::lines offset mismatch");

// ============================================================================
// LoadingMenu - in-game loading screen (shell.o InGameMenus.cpp)
// Size: 0xF8 (248 bytes) - verified against IDA
// ============================================================================
class LoadingMenu : public FEMenu {
public:
    struct TipArray {
        unsigned int  mCount;     // +0x00
        unsigned int* mArray[2];  // +0x04
    };

    ae_array<PanelQuad*, 9> mBackgroundArt;  // +0x4C
    ae_array<FEText*, 2>    mTitleText;      // +0x70
    ae_array<FEText*, 1>    mText;           // +0x78
    ae_array<FEText*, 2>    mMapText;        // +0x7C
    float                   mPercentDone;    // +0x84
    FEMultiLineText*        mTipEntry;       // +0x88
    TipArray                mTipArrays[7];   // +0x8C (84 bytes)
    bool                    mWidescreen;     // +0xE0
    PanelQuad*              m_pLoadingBar;   // +0xE4
    float                   m_fLoadingBarLeft;   // +0xE8
    float                   m_fLoadingBarRight;  // +0xEC
    float                   m_fLoadingBarTop;    // +0xF0
    float                   m_fLoadingBarBottom; // +0xF4

    virtual void SetPanelFile(PanelFile* pf);        // slot 0 0x597120
    virtual void PanelFileUnloaded(PanelFile* pf);   // slot 1 0x593080
    virtual void UpdateWidescreen(bool widescreen);  // slot 2 0x580230
    virtual ~LoadingMenu();                          // slot 3 0x592FF0
    virtual void Draw();                             // slot 20 0x57FFC0
    virtual void Update(float time_inc);             // slot 23 0x574450
    virtual void Select(int entry_num);              // slot 26 0x574410
    virtual void OnActivate();                       // slot 28 0x596FB0
    virtual void OnDeactivate(FEMenu* m);            // slot 29 0x5743F0
    virtual void OnCross(int c);                     // slot 36 0x574420

    LoadingMenu(FEMenuSystem* s);                    // 0x592E90
    static LoadingMenu* Me();                        // ?Me@LoadingMenu@@SAPAV1@XZ 0x574430
    void PickTip();                                  // 0x580010
    void UpdateLoading(float percentDone);           // 0x574460
private:
    static const char* szMapImageFiles[];            // ?szMapImageFiles@LoadingMenu@@0PAPBDA @ 0xDF3A20
};
static_assert(sizeof(LoadingMenu) == 0xF8, "LoadingMenu size mismatch");
static_assert(offsetof(LoadingMenu, mTipArrays) == 0x8C,
              "LoadingMenu::mTipArrays offset mismatch");

// ============================================================================
// FEMultiMenu - menu base that adds held-button Up/Down/Left/Right
// (shell.o FEMultiMenu.cpp) - same size as FEMenu (0x4C)
// ============================================================================
class FEMultiMenu : public FEMenu {
public:
    FEMultiMenu(FEMenuSystem* s, int num, int flg);  // 0x5921B0
protected:
    virtual void ButtonHeldAction();                 // slot 62 0x570950
};
static_assert(sizeof(FEMultiMenu) == 0x4C, "FEMultiMenu size mismatch");

// ============================================================================
// ControllerDisconnectedMenu - controller error overlay (80 bytes)
// Size: 0x50 - verified against IDA
// ============================================================================
class ControllerDisconnectedMenu : public FEMenu {
public:
    FEMultiLineText* text;           // +0x4C

    virtual void SetPanelFile(PanelFile* pf);  // slot 0 0x586D20
    virtual void Draw();                       // slot 20 0x580290
    virtual void OnActivate();                 // slot 28 0x574480

    ControllerDisconnectedMenu();              // 0x5931C0
    void SetErrorMessage();                    // 0x574490
};
static_assert(sizeof(ControllerDisconnectedMenu) == 0x50,
              "ControllerDisconnectedMenu size mismatch");
static_assert(offsetof(ControllerDisconnectedMenu, text) == 0x4C,
              "ControllerDisconnectedMenu::text offset mismatch");

// ============================================================================
// IGOWidget - in-game overlay widget base (12 bytes) - verified against IDA
// 9-slot vftable: dtor/Init/Update/Draw/IsShown/SetShown/UpdateWidescreen/
// UpdateSplitScreen/ForceToAppear (Init/Update/Draw/UpdateWidescreen pure)
// ============================================================================
class IGOWidget {
public:
    bool is_shown;      // +0x04
    bool force_appear;  // +0x05
    int  mClient;       // +0x08

    virtual ~IGOWidget() {}                        // inline COMDAT
    virtual void Init(PanelFile* panel) = 0;
    virtual void Update(float time_inc) = 0;
    virtual void Draw() = 0;
    virtual bool IsShown() { return is_shown; }    // 0x5AEAB0
    virtual void SetShown(bool s) { is_shown = s; }  // 0x5AEAC0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x) = 0;
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport) {}  // 0x5AEAE0 (empty)
    virtual void ForceToAppear() { force_appear = true; }  // 0x5AEAF0
};
static_assert(sizeof(IGOWidget) == 0xC, "IGOWidget size mismatch");
static_assert(offsetof(IGOWidget, is_shown) == 0x04,
              "IGOWidget::is_shown offset mismatch");

// ============================================================================
// IGOHealthWidget (40 bytes) - verified against IDA
// ============================================================================
class IGOHealthWidget : public IGOWidget {
public:
    PanelQuad* bar;          // +0x0C
    PanelQuad* frame;        // +0x10
    PanelQuad* cross;        // +0x14
    PanelQuad* flash;        // +0x18
    float      health;       // +0x1C
    float      last_health;  // +0x20
    bool       draw_flash;   // +0x24

    IGOHealthWidget(int client);  // 0x565EA0
    virtual void Init(PanelFile* panel);              // 0x598330
    virtual void Update(float time_inc);              // 0x5826B0
    virtual void Draw();                              // 0x565EE0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582890
};
static_assert(sizeof(IGOHealthWidget) == 0x28,
              "IGOHealthWidget size mismatch");

// ============================================================================
// IGOTankHealthWidget (32 bytes) - verified against IDA
// ============================================================================
class IGOTankHealthWidget : public IGOWidget {
public:
    PanelQuad* frame;          // +0x0C
    PanelQuad* bar;            // +0x10
    PanelQuad* armor;          // +0x14
    float      healthMaxWidth; // +0x18
    float      maxHealth;      // +0x1C

    IGOTankHealthWidget(int client);  // 0x567370
    virtual void Init(PanelFile* panel);              // 0x598AB0
    virtual void Update(float time_inc);              // 0x588A30
    virtual void Draw();                              // 0x5673B0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583170
};
static_assert(sizeof(IGOTankHealthWidget) == 0x20,
              "IGOTankHealthWidget size mismatch");

// ============================================================================
// IGOVoteWidget (16 bytes) - verified against IDA
// ============================================================================
class IGOVoteWidget : public IGOWidget {
public:
    PanelQuad* vote;  // +0x0C

    IGOVoteWidget(int client);  // 0x5677A0
    virtual void Init(PanelFile* panel);              // 0x598D70
    virtual void Update(float time_inc);              // 0x5677D0
    virtual void Draw();                              // 0x5677F0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5832C0
};
static_assert(sizeof(IGOVoteWidget) == 0x10,
              "IGOVoteWidget size mismatch");

// ============================================================================
// IGORowboatWidget (28 bytes) - verified against IDA
// ============================================================================
class IGORowboatWidget : public IGOWidget {
public:
    enum ePhase {
        PHASE_UP_ARROW = 0,
        PHASE_HALF_CIRCLE = 1,
        PHASE_FADE_OUT = 2,
        PHASE_OFF = 3,
    };

    PanelQuad* mUpArrow;    // +0x0C
    PanelQuad* mHalfCircle; // +0x10
    ePhase     mPhase;      // +0x14
    float      mTimer;      // +0x18

    IGORowboatWidget();     // 0x568FA0
    virtual void Init(PanelFile* panel);              // 0x59A980
    virtual void Update(float time_inc);              // 0x568FD0
    virtual void Draw();                              // 0x569180
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583D60
};
static_assert(sizeof(IGORowboatWidget) == 0x1C,
              "IGORowboatWidget size mismatch");

// ============================================================================
// IGOStanceWidget (56 bytes) - verified against IDA
// ============================================================================
class IGOStanceWidget : public IGOWidget {
public:
    PanelQuad* icons[3][2];    // +0x0C (24 bytes)
    PanelQuad* flash;          // +0x24
    int        cur_stance;     // +0x28
    int        last_change_time;  // +0x2C
    int        last_stance;    // +0x30
    bool       draw_flash;     // +0x34

    IGOStanceWidget(int client);  // 0x565CC0
    virtual void Init(PanelFile* panel);              // 0x598250
    virtual void Update(float time_inc);              // 0x565D00
    virtual void Draw();                              // 0x582550
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582650
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5775D0
};
static_assert(sizeof(IGOStanceWidget) == 0x38,
              "IGOStanceWidget size mismatch");

// ============================================================================
// IGORankWidget (32 bytes) - verified against IDA
// ============================================================================
class IGORankWidget : public IGOWidget {
public:
    PanelQuad* friendlyRanks[3];  // +0x0C
    int        rank;              // +0x18
    int        timeForNormalSize; // +0x1C

    IGORankWidget(int client);  // 0x567630
    virtual void Init(PanelFile* panel);              // 0x598D00
    virtual void Update(float time_inc);              // 0x567660
    virtual void Draw();                              // 0x567760
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583280
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577AB0
};
static_assert(sizeof(IGORankWidget) == 0x20,
              "IGORankWidget size mismatch");

// ============================================================================
// IGOWeaponNameWidget (28 bytes) - verified against IDA
// ============================================================================
class IGOWeaponNameWidget : public IGOWidget {
public:
    FEText*    name;              // +0x0C
    PanelQuad* background;        // +0x10
    int        last_weapon_index; // +0x14
    bool       dont_draw;         // +0x18

    IGOWeaponNameWidget(int client);  // 0x590B00
    virtual void Init(PanelFile* panel);              // 0x566F90
    virtual void Update(float time_inc);              // 0x567040
    virtual void Draw();                              // 0x5672E0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582E90
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577980
};
static_assert(sizeof(IGOWeaponNameWidget) == 0x1C,
              "IGOWeaponNameWidget size mismatch");

// ============================================================================
// IGOAmmoWidget (40 bytes) - verified against IDA
// ============================================================================
class IGOAmmoWidget : public IGOWidget {
public:
    PanelQuad* frame;     // +0x0C
    FEText*    clipAmmo;  // +0x10
    FEText*    totalAmmo; // +0x14
    int        clip_val;  // +0x18
    int        ammo_val;  // +0x1C
    bool       dont_draw; // +0x20
    float      draw_time; // +0x24

    IGOAmmoWidget(int client);  // 0x566530
    virtual void Init(PanelFile* panel);              // 0x598660
    virtual void Update(float time_inc);              // 0x588510
    virtual void Draw();                              // 0x566560
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582D30
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577840
};
static_assert(sizeof(IGOAmmoWidget) == 0x28,
              "IGOAmmoWidget size mismatch");

// ============================================================================
// IGOActionHintWidget (32 bytes) - verified against IDA
// ============================================================================
class IGOActionHintWidget : public IGOWidget {
public:
    FEText* text;            // +0x0C
    bool    dont_draw;       // +0x10
    float   lastAlpha;       // +0x14
    bool    isFadingDown;    // +0x18
    int     startHintTime;   // +0x1C

    IGOActionHintWidget(int client);  // 0x5779C0
    virtual void Init(PanelFile* panel);              // 0x567310
    virtual void Update(float time_inc);              // 0x582EC0
    virtual void Draw();                              // 0x567320
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x567340
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x567360
};
static_assert(sizeof(IGOActionHintWidget) == 0x20,
              "IGOActionHintWidget size mismatch");

// ============================================================================
// IGOHQProgressBarWidget (40 bytes) - verified against IDA
// ============================================================================
class IGOHQProgressBarWidget : public IGOWidget {
public:
    PanelQuad* loading_bar_bkg_01;  // +0x0C
    PanelQuad* loading_bar_bkg_02;  // +0x10
    PanelQuad* loading_bar_bkg_03;  // +0x14
    PanelQuad* loading_bar_white;   // +0x18
    PanelQuad* loading_bar_red;     // +0x1C
    PanelQuad* m_pRadioIcon;        // +0x20
    bool       m_Draw;              // +0x24

    IGOHQProgressBarWidget(int client);  // 0x565F50
    virtual ~IGOHQProgressBarWidget();   // 0x565F90
    virtual void Init(PanelFile* panel);              // 0x598410
    virtual void Update(float time_inc);              // 0x5828F0
    virtual void Draw();                              // 0x566020
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582B70
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x582BD0
};
static_assert(sizeof(IGOHQProgressBarWidget) == 0x28,
              "IGOHQProgressBarWidget size mismatch");

// ============================================================================
// IGOInGameScoreWidget (40 bytes) - verified against IDA
// ============================================================================
class IGOInGameScoreWidget : public IGOWidget {
public:
    PanelQuad*   m_pAlliesFlagIcon;  // +0x0C
    PanelQuad*   m_pAxisFlagIcon;    // +0x10
    FEText*      m_pAlliesScoreText; // +0x14
    FEText*      m_pAxisScoreText;   // +0x18
    unsigned int m_AlliesScore;      // +0x1C
    unsigned int m_AxisScore;        // +0x20
    bool         m_Draw;             // +0x24

    IGOInGameScoreWidget(int client);  // 0x566340
    virtual ~IGOInGameScoreWidget();   // 0x566380
    virtual void Init(PanelFile* panel);              // 0x598540
    virtual void Update(float time_inc);              // 0x566400
    virtual void Draw();                              // 0x5664F0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582D00
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5777D0
};
static_assert(sizeof(IGOInGameScoreWidget) == 0x28,
              "IGOInGameScoreWidget size mismatch");

// ============================================================================
// IGORaiseFlagWidget (24 bytes) - verified against IDA
// ============================================================================
class IGORaiseFlagWidget : public IGOWidget {
public:
    PanelQuad* raise_flag_icon;  // +0x0C
    FEText*    multiplyer_text;  // +0x10
    int        player_count;     // +0x14

    IGORaiseFlagWidget(int client);  // 0x566070
    virtual ~IGORaiseFlagWidget();   // 0x5660A0
    virtual void Init(PanelFile* panel);              // 0x5984D0
    virtual void Update(float time_inc);              // 0x566100
    virtual void Draw();                              // 0x5661D0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582C40
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577660
};
static_assert(sizeof(IGORaiseFlagWidget) == 0x18,
              "IGORaiseFlagWidget size mismatch");

// ============================================================================
// IGOTimerWidget (36 bytes) - verified against IDA
// ============================================================================
class IGOTimerWidget : public IGOWidget {
public:
    static float m_TimeLimit;  // ?m_TimeLimit@IGOTimerWidget@@0MA @ 0xF30D58
    static float m_StartTime;  // ?m_StartTime@IGOTimerWidget@@0MA @ 0xF30D54

    FEText*       m_pTimer;     // +0x0C
    float         m_DeltaTime;  // +0x10
    unsigned int  m_hour;       // +0x14
    unsigned int  m_min;        // +0x18
    int           m_sec;        // +0x1C
    bool          m_TimerActive;// +0x20
    bool          m_Draw;       // +0x21

    IGOTimerWidget(int client);  // 0x5661F0
    virtual ~IGOTimerWidget();   // 0x566230
    virtual void Init(PanelFile* panel);              // 0x582C70
    virtual void Update(float time_inc);              // 0x5776A0
    virtual void Draw();                              // 0x566280
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5662A0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5662C0
private:
    void setTimerValues();                            // 0x5662F0
};
static_assert(sizeof(IGOTimerWidget) == 0x24,
              "IGOTimerWidget size mismatch");

// ============================================================================
// IGOGrenadeWidget (68 bytes) - verified against IDA
// ============================================================================
class IGOGrenadeWidget : public IGOWidget {
public:
    PanelQuad* grenadeUS;        // +0x0C
    PanelQuad* grenadeGerman;    // +0x10
    PanelQuad* grenadeSmokeL;    // +0x14
    PanelQuad* grenadeSmokeR;    // +0x18
    PanelQuad* grenadeSticky;    // +0x1C
    PanelQuad* rifleGrenade;     // +0x20
    PanelQuad* apMine;           // +0x24
    FEText*    ammoLeft;         // +0x28
    FEText*    ammoRight;        // +0x2C
    int        ammo_left_val;    // +0x30
    int        ammo_right_val;   // +0x34
    bool       showLeft;         // +0x38
    bool       showRight;        // +0x39
    float      left_draw_time;   // +0x3C
    float      right_draw_time;  // +0x40

    IGOGrenadeWidget(int client);  // 0x566590
    virtual ~IGOGrenadeWidget();   // 0x5665F0
    virtual void Init(PanelFile* panel);              // 0x598730
    virtual void Update(float time_inc);              // 0x5666B0
    virtual void Draw();                              // 0x566BC0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582D70
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5778A0
};
static_assert(sizeof(IGOGrenadeWidget) == 0x44,
              "IGOGrenadeWidget size mismatch");

// ============================================================================
// IGOSpecialWeaponWidget (40 bytes) - verified against IDA
// ============================================================================
class IGOSpecialWeaponWidget : public IGOWidget {
public:
    PanelQuad* artillery;         // +0x0C
    PanelQuad* health;            // +0x10
    PanelQuad* ammo;              // +0x14
    float      percent;           // +0x18
    bool       hadAmmo;           // +0x1C
    int        timeForNormalSize; // +0x20
    float      scale;             // +0x24

    IGOSpecialWeaponWidget(int client);  // 0x567820
    virtual void Init(PanelFile* panel);              // 0x598DE0
    virtual void Update(float time_inc);              // 0x567860
    virtual void Draw();                              // 0x5832D0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5834B0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577B40
};
static_assert(sizeof(IGOSpecialWeaponWidget) == 0x28,
              "IGOSpecialWeaponWidget size mismatch");

// ============================================================================
// IGOTankLoadingWidget (24 bytes) - verified against IDA
// ============================================================================
class IGOTankLoadingWidget : public IGOWidget {
public:
    PanelQuad* on;    // +0x0C
    PanelQuad* off;   // +0x10
    bool       is_on; // +0x14

    IGOTankLoadingWidget(int client);  // 0x567420
    virtual void Init(PanelFile* panel);              // 0x598B00
    virtual void Update(float time_inc);              // 0x588AF0
    virtual void Draw();                              // 0x567450
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5831B0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577A70
};
static_assert(sizeof(IGOTankLoadingWidget) == 0x18,
              "IGOTankLoadingWidget size mismatch");

// ============================================================================
// IGOTankReticleWidget (76 bytes) - verified against IDA
// ============================================================================
class IGOTankReticleWidget : public IGOWidget {
public:
    PanelQuad* reticle;      // +0x0C
    PanelQuad* tic[4];       // +0x10
    float      ticXPosition[4];  // +0x20
    float      ticYPosition[4];  // +0x30
    int        ticCount;     // +0x40
    int        currentTic;   // +0x44
    float      currentAlpha; // +0x48

    IGOTankReticleWidget(int client);  // 0x567500
    virtual void Init(PanelFile* panel);              // 0x598B50
    virtual void Update(float time_inc);              // 0x588B40
    virtual void Draw();                              // 0x567530
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5831E0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x583230
};
static_assert(sizeof(IGOTankReticleWidget) == 0x4C,
              "IGOTankReticleWidget size mismatch");

// ============================================================================
// IGOHintWidget (1132 bytes) - verified against IDA
// ============================================================================
class IGOHintWidget : public IGOWidget {
public:
    PanelQuad* icons[138];         // +0x0C
    FEMultiLineText* text;         // +0x234
    int        current_icon;       // +0x238
    int        current_icon_nudge[138];  // +0x23C
    int        last_icon;          // +0x464
    bool       dont_draw;          // +0x468
    bool       wide_weapon;        // +0x469

    IGOHintWidget(int client);  // 0x568AF0
    virtual void Init(PanelFile* panel);              // 0x59A2E0
    virtual void Update(float time_inc);              // 0x578AD0
    virtual void Draw();                              // 0x568B50
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583C70
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5792B0
    void SetWeaponsPQs(PanelFile* panel,
                       PanelFile* panel2);            // 0x59A5E0
};
static_assert(sizeof(IGOHintWidget) == 0x46C,
              "IGOHintWidget size mismatch");

// ============================================================================
// IGOItemIcons (28 bytes) - verified against IDA
// ============================================================================
class IGOItemIcons : public IGOWidget {
public:
    struct ItemIcon {
        PanelQuad* icon;    // +0x00
        uint8_t    height;  // +0x04
        uint8_t    alpha;   // +0x05
    };

    ItemIcon mItemIcons[2];  // +0x0C

    IGOItemIcons(int client);  // 0x5691B0
    virtual ~IGOItemIcons();   // 0x5691D0
    virtual void Init(PanelFile* panel);              // 0x59AA40
    virtual void Update(float time_inc);              // 0x5691E0
    virtual void Draw();                              // 0x569530
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583D90
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x583DC0
private:
    void Draw(Entity* pEnt, const math::Position3& playerPosition,
              int iconIndex);                         // 0x5691F0
};
static_assert(sizeof(IGOItemIcons) == 0x1C,
              "IGOItemIcons size mismatch");

// ============================================================================
// IGOHeadIcons (268 bytes) - verified against IDA
// ============================================================================
class IGOHeadIcons : public IGOWidget {
public:
    struct HeadIcon {
        PanelQuad* icon;    // +0x00
        uint8_t    height;  // +0x04
        uint8_t    alpha;   // +0x05
    };
    struct HeadIconsPlayer {
        bool  show;            // +0x00
        bool  showVehicleIcon; // +0x01
        float alpha;           // +0x04
        int   index;           // +0x08
    };

    HeadIcon        mHeadIcons[8];      // +0x0C
    HeadIconsPlayer mPlayers[16];       // +0x4C

    IGOHeadIcons(int client);  // 0x568F70
    virtual ~IGOHeadIcons();   // 0x568F90
    virtual void Init(PanelFile* panel);              // 0x59A8A0
    virtual void Update(float time_inc);              // 0x58AC90
    virtual void Draw();                              // 0x58B070
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583D00
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x583D30
};
static_assert(sizeof(IGOHeadIcons) == 0x10C,
              "IGOHeadIcons size mismatch");

// ============================================================================
// FEMenuListBoxItem â€" list-box data row (28 bytes) â€" verified against IDA
// ============================================================================
class FEMenuListBoxItem {
public:
    unsigned int mIndex;         // +0x00
    Broc::string mText;          // +0x04
    int          mData;          // +0x08
    unsigned int mSubItemCount;  // +0x0C
    Broc::string mSubItems[3];   // +0x10

    FEMenuListBoxItem(unsigned int index, const Broc::string& text,
                      int data);  // inline COMDAT 0x5B2DF0-ish
    ~FEMenuListBoxItem();         // ?dtor 0x5B2E10-ish
    const Broc::string& GetSubItem(unsigned int index);  // shell.o 0x571D90
    const unsigned int AddSubItem(const Broc::string& text);  // shell.o 0x571D60
};
static_assert(sizeof(FEMenuListBoxItem) == 28,
              "FEMenuListBoxItem size mismatch");

// ============================================================================
// FE widget entry subclasses (shell.o FEComboBox.cpp / FESlider.cpp /
// FEDoubleEntry.cpp / FEMenuListBox.cpp) - all inherit FEMenuEntry
// ============================================================================
class FEComboBox : public FEMenuEntry {
public:
    short    mCurrOption;     // +0x18
    short    mCachedOption;   // +0x1A
    short    mNumOptions;     // +0x1C
    short    mMaxOptions;     // +0x1E
    Broc::string* mOptionStrings;  // +0x20
    Handle   mSound;          // +0x24
    bool     mEnableSound;    // +0x28
    uint8_t  _pad29[3];       // +0x29
    FEText*  mLabel;          // +0x2C
    struct Fader {
        PanelQuad* mQuad;
        float mAlpha;
        float mAlphaTo;
        float mTime;
        float mAlphaDelta;
        bool  mFading;
        uint8_t _pad[3];
    } mScrollBarLeftFader;    // +0x30
    Fader mScrollBarRightFader;  // +0x48

    FEComboBox(FEMenu* parent, short maxOptions, FEText* text,
               FEText* label, PanelQuad* leftArrow,
               PanelQuad* rightArrow);  // 0x57E0E0
    virtual ~FEComboBox();              // 0x58E180
    virtual short OnLeft();             // 0x586260
    virtual short OnRight();            // 0x586310
    virtual void Draw();                // 0x5714F0
    virtual void Update(float time_inc);// 0x571810
    virtual void SetShown(bool on);     // vtable slot 14
    virtual void Highlight(bool h, bool anim);  // 0x571A80
    virtual void AdjustColor();         // 0x571B00
    virtual void MoveForSplitScreen(int viewport, int old_viewport);  // 0x571AC0
    virtual void SetValue(int value);   // slot 49
    virtual int GetValue();             // slot 50

    void PlayNavigationSound();         // 0x57E230
    void SetWidgets(FEText* copy_this, FEText* label, PanelQuad* leftArrow,
                    PanelQuad* rightArrow);  // 0x571520
    void AddOption(Broc::string optionString);       // 0x571570
    void AddOptionNoLocalize(Broc::string optionString);  // 0x571950
    void ClearOptions();                // 0x571660
    void SetOption(int index, Broc::string optionString);  // 0x571700
    void SetCurrOption(short option);   // 0x571A30
};
static_assert(sizeof(FEComboBox) == 0x60, "FEComboBox size mismatch");

class FESlider : public FEMenuEntry {
public:
    int   mValue;            // +0x18
    int   mMin;              // +0x1C
    int   mMax;              // +0x20
    Handle mSound;           // +0x24
    bool  mEnableSound;      // +0x28
    uint8_t _pad29[3];       // +0x29
    FEText* mBarText;        // +0x2C
    PanelQuad* mBar;         // +0x30
    bool  mNumeric;          // +0x34
    uint8_t _pad35[3];       // +0x35

    FESlider(FEMenu* parent, PanelQuad* bar, FEText* label,
             FEText* barText);          // 0x57E2B0
    virtual ~FESlider();                // 0x58E270
    virtual short OnLeft();             // 0x5863C0
    virtual short OnRight();            // 0x5863F0
    virtual void Draw();                // 0x571B30
    virtual void Update(float time_inc);// 0x571B70
    virtual void SetShown(bool on);     // vtable slot 14
    virtual void Highlight(bool h, bool anim);  // 0x571BD0
    virtual void MoveForSplitScreen(int viewport, int old_viewport);  // 0x57E440
    virtual void UpdateWidescreen(bool widescreen);  // 0x571C10
    virtual void SetValue(int value);   // slot 49
    virtual int GetValue();             // slot 50

    void PlayNavigationSound();         // 0x57E3C0
    void SetRange(int min, int max);    // 0x571BB0
    void AdjustBar();                   // 0x57E350
    void SetWidgets(PanelQuad* bar, FEText* label, FEText* barText);  // 0x57E470
};
static_assert(sizeof(FESlider) == 0x38, "FESlider size mismatch");

class FEDoubleEntry : public FEMenuEntry {
public:
    FEText* mLabel;          // +0x18

    FEDoubleEntry(FEMenu* parent, FEText* label, FEText* text);  // 0x57E4B0
    virtual ~FEDoubleEntry();           // 0x571C30
    virtual void SetShown(bool on);     // inline 0x5B3690
    virtual void Draw();                // 0x571C50
    virtual void Update(float time_inc);// 0x571C80
    virtual void Highlight(bool h, bool anim);  // 0x571CB0
    virtual void AdjustColor();         // 0x571D30
    virtual void MoveForSplitScreen(int viewport, int old_viewport);  // 0x571CF0
};
static_assert(sizeof(FEDoubleEntry) == 0x1C, "FEDoubleEntry size mismatch");

class FEMenuListBox : public FEMenuEntry {
public:
    ae_vector<FEMenuListBoxItem*> mItems;  // +0x18
    float  mColumnWidths[4];               // +0x24
    bool   mHasHeadings;                   // +0x34
    uint8_t _pad35[3];                     // +0x35
    Broc::string mColumnHeadings[4];       // +0x38
    unsigned int mNumLines;                // +0x48
    unsigned int mTopLine;                 // +0x4C
    unsigned int mSelectedLine;            // +0x50
    float  mRowHeight;                     // +0x54
    float  mHeadingSpacing;                // +0x58

    FEMenuListBox(FEText* t, FEMenu* m, int numLines);  // inline 0x5A5D30-ish
    virtual ~FEMenuListBox();
    virtual short OnUp();               // 0x57E8F0
    virtual short OnDown();             // 0x57E970
    virtual void Draw();                // 0x57E5B0

    const unsigned int AddItem(const Broc::string& itemText,
                               int itemData);  // 0x58E330
    const unsigned int AddSubItem(unsigned int itemIndex,
                                  const Broc::string& subItemText);  // 0x57E520
    void SetCurrentSelection(int iCurrentSelection);  // 0x57EA00
    const int GetCurrentSelection();     // 0x57EA60
    const int GetCurrentSelectionData(); // 0x57EA80
    void SetColumnWidth(unsigned int column, float width);  // 0x571DF0
    void SetColumnHeading(unsigned int column,
                          const char* heading);  // 0x571E60
    void Sort(unsigned int column);      // 0x571ED0 (empty)
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x571EE0
    void Clear();                        // 0x586420
};
static_assert(sizeof(FEMenuListBox) == 0x5C, "FEMenuListBox size mismatch");

// ============================================================================
// UIListBox - 172 bytes (verified against IDA; shell.o owns the impl)
// ============================================================================
class UIListBox {
public:
    enum EScrollQuads {
        kScrollBarArrowUp = 0,
        kScrollBarArrowDown = 1,
        kScrollBarIndicator = 2,
        kScrollBarTrack = 3,
        kScrollBarDetail1 = 4,
        kScrollBarDetail2 = 5,
    };

    class UIListBoxRow;

    class UIListBoxData {
    public:
        Broc::string mText;   // +0x00
        int          mState;  // +0x04

        UIListBoxData();      // 0x5AEBB0
        void SetState(int state);  // 0x5AEBE0
        void SetText(const char* text);  // 0x5AEC70
        const char* GetText();  // 0x5B2580
    };
    static_assert(sizeof(UIListBoxData) == 8, "UIListBoxData size mismatch");

    class UIListBoxDataRow {
    public:
        enum { kTypeNone = 0, kTypeText = 1, kTypeQuad = 2 };

        ae_vector<UIListBoxData> mColumns;  // +0x00 (12 bytes)
        int  mColumnCount;                  // +0x0C
        bool mEnabled;                      // +0x10

        UIListBoxDataRow();  // 0x5B6160
        void ClearItem();    // 0x5B2DB0
        void SetColumnCount(int columns);  // 0x5B6180
        void SetItemState(int column, int state);  // 0x5B2BF0
        void SetText(int column, const char* text);  // 0x5B2C80
        const char* GetText(int column);  // 0x5B2D10
    };
    static_assert(sizeof(UIListBoxDataRow) == 20,
                  "UIListBoxDataRow size mismatch");

    class UIListBoxItem {
    public:
        enum { kTypeNone = 0, kTypeText = 1, kTypeQuad = 2 };
        friend class UIListBoxRow;

        int          mType;       // +0x00
        int          mState;      // +0x04
        int          mStateCount; // +0x08
        ae_vector<PanelAnimObject*> mObjects;  // +0x0C (12 bytes)

        UIListBoxItem();  // 0x5B4D50
        void SetState(int state);       // 0x5B1E30
        void SetStateCount(int count);  // 0x5B4DC0
        void SetText(const char* text); // 0x5813A0
        void SetSelected(bool selected, bool flashing);  // 0x581460
        void SetColor(color32 unselectedColor,
                      color32 selectedcolor);  // 0x581580
        void SetEnabled(bool enabled);  // 0x581650
        void ClearText();               // 0x5B4EC0
        void RemoveItems();             // 0x5B2070
        color32 GetColor();             // 0x5B22B0
        color32 GetUnselectedColor();   // 0x5B23D0
        float GetY();                   // 0x5B21A0
    private:
        void SetObject(PanelAnimObject* object, int state);  // 0x5B24E0
    };
    static_assert(sizeof(UIListBoxItem) == 24, "UIListBoxItem size mismatch");

    class UIListBoxRow {
    public:
        ae_vector<UIListBoxItem> mColumns;  // +0x00 (12 bytes)
        int  mColumnCount;                  // +0x0C

        UIListBoxRow();  // 0x5B6E30
        void ClearItem();                    // 0x5B5390
        void RemoveItems();                  // 0x5B2630
        void Draw();                         // 0x5B26D0
        void Update(float time_delta);       // 0x5B2870
        void SetColumnCount(int columns);    // 0x5B7580
        void SetColumnStateCount(int column, int count);  // 0x5B5130
        void SetItem(int column, FEText* text, int state);   // 0x5B51C0
        void SetItem(int column, PanelQuad* quad, int state);  // 0x5B5260
        void SetText(int column, const char* text);  // 0x5B5300
        void SetItemState(int column, int state);    // 0x5B25A0
        void SetColumnColor(int column, color32 unselectedColor,
                            color32 selectedcolor);  // 0x5B5450
        void SetSelected(int column, bool selected,
                         bool flashing);             // 0x5B54E0
        void SetEnabled(bool enabled);               // 0x5B5570
        float GetY(int column);                      // 0x5B2A20
        color32 GetColumnSelectedColor(int column);  // 0x5B2AB0
        color32 GetColumnUnselectedColor(int column);// 0x5B2B40
    };
    static_assert(sizeof(UIListBoxRow) == 16, "UIListBoxRow size mismatch");

    // Members
    ae_vector<UIListBoxRow> mItemRows;        // +0x04
    ae_vector<UIListBoxDataRow> mDataRows;    // +0x10
    bool mSelectedFlashing;                   // +0x1C
    ae_vector<color32> mSelectedRowOriginalColor;      // +0x20
    ae_vector<bool> mSelectedRowColorChangeColumns;    // +0x2C
    PanelQuad* mScrollBarQuads[6];            // +0x38
    int mLastRowContainingData;               // +0x50
    int mItemRowsCount;                       // +0x54
    int mItemColumnsCount;                    // +0x58
    int mDataRowsCount;                       // +0x5C
    int mTopLine;                             // +0x60
    int mSelectedLine;                        // +0x64
    int mScrollBarTopY;                       // +0x68
    int mScrollBarBottomY;                    // +0x6C
    float mScrollBarYInc;                     // +0x70
    bool mIsWrapping;                         // +0x74
    bool mBlockRefresh;                       // +0x75
    int mIncrementBy;                         // +0x78
    struct PanelQuadFader {
        PanelQuad* mQuad;        // +0x00
        float mAlpha;            // +0x04
        float mAlphaTo;          // +0x08
        float mTime;             // +0x0C
        float mAlphaDelta;       // +0x10
        bool  mFading;           // +0x14
        uint8_t _pad[3];         // +0x15
    } mScrollBarUpFader;         // +0x7C
    PanelQuadFader mScrollBarDownFader;  // +0x94

    UIListBox(int visibleRows, int visibleColumns, int maxDataRows,
              bool bIsWrapping);  // 0x59BB70
    virtual ~UIListBox();         // 0x5B9BB0
    virtual void Clear();         // 0x58FFC0
    virtual void ClearRow(int row);  // 0x590050
protected:
    virtual void SelectLine(int selection, int top_line);  // 0x5904A0
public:
    virtual void SelectLine(int selection);  // 0x5903F0
    virtual short OnUp(int c);      // 0x590290
    virtual short OnDown(int c);    // 0x590340
    virtual void Update(float time_delta);  // 0x581750
    virtual void Draw();            // 0x581910
    virtual void Refresh();         // 0x590520

    void RemoveAllItems();          // 0x5816B0
    void SetItemState(int row, int column, int state);  // 0x581990
    void PageDown(int numPageRows);  // 0x581B00
    void PageUp(int numPageRows);    // 0x581BC0
    void SetAllColumnsSelectable(bool selectable);  // 0x581D40
    void SetRowEnabled(int row, bool enabled);      // 0x5878D0
    void SetColumnStateCount(int column, int count);  // 0x587960
    void SetItem(int row, int column, FEText* text, int state);  // 0x587BA0
    void SetItem(int row, int column, PanelQuad* quad, int state); // 0x587CB0
    void SetText(int row, int column, const char* text);  // 0x587DC0
    void SetScrollBarQuad(EScrollQuads index, PanelQuad* quad);  // 0x577050
    void SetScrollBarFromPanelFile(PanelFile* pf);  // 0x598120
    void FormatForSplitScreen(int viewport, int old_viewport);  // 0x5770B0
    void FormatForWidescreen(bool widescreen, float about_x);   // 0x5770C0
    void ResizeDataRows(int rowCount);  // 0x590150
protected:
    void MoveUpTo(int newSelectedLine);  // 0x576F70
    void MoveDownTo(int newSelectedLine);// 0x576FE0
    short MoveUp(int c);                 // 0x587F00
    short MoveDown(int c);               // 0x587FF0
    void DeselectRow();                  // 0x5880E0
    void SelectRow();                    // 0x588180
    void UpdateScrollBar();              // 0x581C70
    void PlayNavigationSound();          // 0x581A90
};
static_assert(sizeof(UIListBox) == 0xAC, "UIListBox size mismatch");

// ============================================================================
// _XUID - 12 bytes (verified against IDA / xlive.h)
// ============================================================================
#ifndef XUID_TYPE_DEFINED
#define XUID_TYPE_DEFINED
#pragma pack(push, 4)
struct _XUID {
    union {
        unsigned long long qwValue;
        struct {
            unsigned long dwUserID;  // +0x00
            unsigned long dwTeamID;  // +0x04
        };
    };
    unsigned long dwUserFlags;  // +0x08
};
#pragma pack(pop)
static_assert(sizeof(_XUID) == 0xC, "_XUID size mismatch");
#endif

// ============================================================================
// UIHighlightListBox - 224 bytes (verified against IDA)
// ============================================================================
class UIHighlightListBox : public UIListBox {
public:
    PanelQuad* mHighlightQuad;   // +0xAC
    ae_vector<bool> mHighlights; // +0xB0
    ae_vector<color32> mHighlightedRowOriginalSelectedColor;   // +0xBC
    ae_vector<color32> mHighlightedRowOriginalUnselectedColor; // +0xC8
    color32 mHighlightedSelectedTextColor;   // +0xD4
    color32 mHighlightedUnselectedTextColor; // +0xD8
    int     mHighlightedRow;                 // +0xDC

    UIHighlightListBox(int visibleRows, int visibleColumns,
                       int maxDataRows, bool bIsWrapping);  // 0x59BF60
    virtual void Clear();             // 0x5909B0
    virtual void Refresh();           // 0x5909D0

    void ClearHighlights();         // 0x581DC0
    void SetEntryColor(int y, int x, color32 colorUnLit,
                       color32 colorLit);  // 0x588390
protected:
    void UpdateHighlight();         // 0x581E40
    void SaveHighlightRowColor();   // 0x581F90
    void RestoreHighlightRowColor();// 0x588220
    void ColorHighlightRow();       // 0x5882F0
};
static_assert(sizeof(UIHighlightListBox) == 0xE0,
              "UIHighlightListBox size mismatch");

// ============================================================================
// UIPlayerListBox - 264 bytes (verified against IDA)
// ============================================================================
class UIPlayerListBox : public UIHighlightListBox {
public:
    int mPlayerRow;          // +0xE0
    _XUID myXUID;            // +0xE4
    ae_vector<int> mPlayerIDs;   // +0xF0
    ae_vector<_XUID> mPlayerXUIDs;  // +0xFC

    UIPlayerListBox(int visibleRows, int visibleColumns,
                    int maxDataRows, bool bIsWrapping);  // 0x59C020
    virtual void Clear();        // 0x590A00
    virtual void ClearRow(int row);  // 0x590AB0

    void SetPlayerID(int row, int id);      // 0x588400
    void SetPlayerXUID(int row, _XUID id);  // 0x588480
protected:
    void CheckIfLocalPlayer(int row);  // 0x5820C0
};
static_assert(sizeof(UIPlayerListBox) == 0x108,
              "UIPlayerListBox size mismatch");

// ============================================================================
// IGOVoipList (184 bytes) - verified against IDA
// ============================================================================
class IGOVoipList : public IGOWidget {
public:
    UIListBox mListBox;  // +0x0C

    IGOVoipList(int client);  // 0x59C480
    virtual ~IGOVoipList();   // 0x59AB00
    virtual void Init(PanelFile* panel);              // 0x59AB50
    virtual void Update(float time_inc);              // 0x58B680
    virtual void Draw();                              // 0x5696B0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x579530
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x579540
};
static_assert(sizeof(IGOVoipList) == 0xB8,
              "IGOVoipList size mismatch");

// ============================================================================
// IGOJeepMapWidget (80 bytes) - verified against IDA
// ============================================================================
class IGOJeepMapWidget : public IGOWidget {
public:
    static const float mapTopRight[3][2];   // @ 0xDF38C4
    static const float mapWideHeight[3][2]; // @ 0xDF38DC
    static const char* mLevelMapName[2];    // @ 0xCEF344

    PanelQuad* icon;         // +0x0C
    float mapSizeX[3];       // +0x10
    float mapSizeY[3];       // +0x1C
    float u[4];              // +0x28
    float v[4];              // +0x38
    float hudRange;          // +0x48
    bool  mTextureSetted;    // +0x4C

    IGOJeepMapWidget(int client);  // 0x5689B0
    virtual void Init(PanelFile* panel);              // 0x59A250
    virtual void Update(float time_inc);              // 0x5789A0
    virtual void Draw();                              // 0x568AD0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583C40
    void SetLevelMap(int levelIndex, TPakId pakId);   // 0x5689E0
private:
    void WithinMap(float x, float y, float& scaleX,
                   float& scaleY);                    // 0x568A90
};
static_assert(sizeof(IGOJeepMapWidget) == 0x50,
              "IGOJeepMapWidget size mismatch");

// ============================================================================
// IGOWarStatusWidget (164 bytes) - verified against IDA
// ============================================================================
class IGOWarStatusWidget : public IGOWidget {
public:
    float  iconWidth;     // +0x0C
    float  iconHeight;    // +0x10
    float  neutralWidth;  // +0x14
    float  neutralHeight; // +0x18
    float  centerX;       // +0x1C
    float  centerY;       // +0x20
    float  zoomPct;       // +0x24
    int    lastFlag;      // +0x28
    ae_array<PanelQuad*, 5> m_pObjectiveFrameUS;    // +0x2C
    ae_array<PanelQuad*, 5> m_pObjectiveFrameGerman;// +0x40
    ae_array<PanelQuad*, 5> m_pObjectiveGerman;     // +0x54
    ae_array<PanelQuad*, 5> m_pObjectiveUS;         // +0x68
    ae_array<PanelQuad*, 5> m_pIconGerman;          // +0x7C
    ae_array<PanelQuad*, 5> m_pIconUS;              // +0x90

    IGOWarStatusWidget(int client);  // 0x5679F0
    virtual ~IGOWarStatusWidget();   // 0x577B90
    virtual void Init(PanelFile* panel);              // 0x598E70
    virtual void Update(float time_inc);              // 0x567A90
    virtual void Draw();                              // 0x588C90
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583640
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x577FC0
    void DrawFlag(int iFlag, int iIndexAdjustedFlag, int iContestedFlag,
                  int numFlags, int myTeam, int notMyTeam,
                  float capturePct);                  // 0x5834F0
};
static_assert(sizeof(IGOWarStatusWidget) == 0xA4,
              "IGOWarStatusWidget size mismatch");

// ============================================================================
// IGOGrenadeCookWidget (52 bytes) - verified against IDA
// ============================================================================
class IGOGrenadeCookWidget : public IGOWidget {
public:
    PanelQuad* grenadeTime[6];  // +0x0C
    PanelQuad* grenadeRing;     // +0x24
    float      fuseRemaining;   // +0x28
    float      fuseTotal;       // +0x2C
    bool       crossHair;       // +0x30
    bool       dont_draw;       // +0x31

    IGOGrenadeCookWidget(int client);  // 0x566C50
    virtual ~IGOGrenadeCookWidget();   // 0x566CE0
    virtual void Init(PanelFile* panel);              // 0x5988D0
    virtual void Update(float time_inc);              // 0x566D70
    virtual void Draw();                              // 0x566F50
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x582DF0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x582E40
    void SetFuse(float total, float remain);          // 0x566D50
};
static_assert(sizeof(IGOGrenadeCookWidget) == 0x34,
              "IGOGrenadeCookWidget size mismatch");

// ============================================================================
// IGOTankIconWidget (156 bytes) - verified against IDA
// ============================================================================
class IGOTankIconWidget : public IGOWidget {
public:
    PanelQuad* base[4];                  // +0x0C
    PanelQuad* turret[4];                // +0x1C
    PanelQuad* occupants[4][3][2];       // +0x2C (96 bytes)
    int        mVehicleType;             // +0x8C
    float      mCompassWidth;            // +0x90
    float      mLastBaseAngles;          // +0x94
    float      mLastTurretAngles;        // +0x98

    IGOTankIconWidget(int client);  // 0x567AA0
    virtual void Init(PanelFile* panel);              // 0x599D10
    virtual void Update(float time_inc);              // 0x588DF0
    virtual void Draw();                              // 0x567BE0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583890
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x5782A0
    void Rotate(PanelQuad* quad, float angle);        // 0x567B00
    int  GetVehicleIndex(scr_vehicle_t* vehicle) const;  // 0x567CC0
};
static_assert(sizeof(IGOTankIconWidget) == 0x9C,
              "IGOTankIconWidget size mismatch");

// ============================================================================
// IGOGrenadeIndicator (76 bytes) - verified against IDA
// ============================================================================
class IGOGrenadeIndicator : public IGOWidget {
public:
    DbLinkedHandle<EntityHandleDb, Entity> mActiveGrenadeList[10];  // +0x0C
    float      mArrowOffset;       // +0x34
    PanelQuad* mMineIcon;          // +0x38
    PanelQuad* mGrenadeIcon;       // +0x3C
    PanelQuad* mGrenadeArrow;      // +0x40
    PanelQuad* mGrenadeHold;       // +0x44
    PanelQuad* mCurrentGrenadeIcon;// +0x48

    IGOGrenadeIndicator(int client);  // 0x579310
    virtual ~IGOGrenadeIndicator();   // 0x568BA0
    virtual void Init(PanelFile* panel);              // 0x59A780
    virtual void Update(float time_inc);              // 0x58A9A0
    virtual void Draw();                              // 0x58AAA0
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x583CD0
    bool CanBePickUp();                               // 0x568F50
    void SetDefaultIcon();                            // 0x568F60
    void DrawGrenade(const Entity* grenade);          // 0x5793A0
    void AddActiveGrenade(const Entity* grenade);     // 0x58ABE0
private:
    bool ValidHudGrenade(const Entity* grenade, float splashRadius,
                         float (&grenadeOffset)[3],
                         float& grenadeDistanceSquared) const;  // 0x568C30
    float CalcGrenadeAlpha(float grenadeDistanceSquared,
                           float splashInnerRadius,
                           float splashOutterRadius) const;     // 0x568D70
    void DrawGrenadeIcon(float sinYaw, float cosYaw,
                         float alpha) const;                    // 0x568DE0
    void DrawGrenadeArrow(float yaw, float sinYaw, float cosYaw,
                          float alpha) const;                   // 0x568E80
};
static_assert(sizeof(IGOGrenadeIndicator) == 0x4C,
              "IGOGrenadeIndicator size mismatch");

// ============================================================================
// hud_type - in-game HUD mode (verified against IDA enum)
// ============================================================================
enum hud_type {
    HUD_TYPE_NORMAL = 0,
    HUD_TYPE_LIBERATOR_BOMBER = 1,
    HUD_TYPE_LIBERATOR_GROUND = 2,
    HUD_TYPE_TUNISIA = 3,
    HUD_TYPE_SPECTATE = 4,
    HUD_TYPE_INTERMISSION = 5,
    HUD_TYPE_NUM = 6,
};

// ============================================================================
// IGOMapObject - compass map marker base (16 bytes) - verified against IDA
// ============================================================================
class IGOMapObject {
public:
    float x;      // +0x00
    float y;      // +0x04
    float alpha;  // +0x08
    bool  draw;   // +0x0C
};
static_assert(sizeof(IGOMapObject) == 0x10, "IGOMapObject size mismatch");

// ============================================================================
// IGOCompassWidget (5256 bytes) - verified against IDA
// ============================================================================
class IGOCompassWidget : public IGOWidget {
public:
    struct WorldIcon {
        PanelQuad* icon;    // +0x00
        uint8_t    height;  // +0x04
        uint8_t    alpha;   // +0x05
    };

    struct IGOFriendly : IGOMapObject {
        int   last_update;// +0x10
        float last_yaw;   // +0x14
        float last_pos[2];// +0x18
        int   flags;      // +0x20
    };
    struct IGOEnemy : IGOFriendly {
        int   last_shot_time;  // +0x24
    };
    struct IGOObjective : IGOMapObject {
        float ring_alpha; // +0x10
        float ring_scale; // +0x14
        bool  draw_ring;  // +0x18
        bool  up;         // +0x19
        bool  down;       // +0x1A
        int   state;      // +0x1C
        int   worldState; // +0x20
    };

    int16_t  m_hideCompassStarActive;   // +0x0C
    int16_t  m_hideCompassStarIndex;    // +0x0E
    int      m_hideUpdatedText;         // +0x10
    int      m_hideUpdatedTextIndex;    // +0x14
    bool     mDrawVehMap;               // +0x18
    PanelQuad* objectiveIcons[27];      // +0x1C
    WorldIcon worldIcons[27];           // +0x88
    PanelQuad* compass;                 // +0x160
    PanelQuad* pointer;                 // +0x164
    PanelQuad* frame;                   // +0x168
    float    compass_speed;             // +0x16C
    float    compass_yaw;               // +0x170
    bool     DrawObjectivesOnly;        // +0x174
    IGOFriendly friendlies[32];  // +0x178 (1152 bytes)
    IGOEnemy    gEnemies[32];    // +0x5F8 (1280 bytes)
    IGOObjective objectives[17]; // +0x1200 (612 bytes)
    IGOFriendly enemyTanks[25];  // +0xAF8 (900 bytes)
    IGOFriendly tanks[25];       // +0xE7C (900 bytes)
    float global_alpha;   // +0x1464
    float draw_time;      // +0x1468
    int   mViewport;      // +0x146C
    float last_player_pos[3];   // +0x1470
    float last_player_angles[3];// +0x147C

    IGOCompassWidget(int client);  // 0x567D00
    virtual ~IGOCompassWidget();   // 0x567CF0
    virtual void Init(PanelFile* panel) {}  // nullsub_220 (concrete override)
    void Init(PanelFile* panel,
              bool bIconPanel);           // 0x599FA0 (non-virtual)
    virtual void Update(float time_inc);              // 0x58ADB0
    virtual void Draw();                              // 0x58B100
    virtual void UpdateWidescreen(bool widescreen,
                                  float about_x);     // 0x5840B0
    virtual void UpdateSplitScreen(int viewport,
                                   int old_viewport); // 0x58A050
    void Draw3DObjective(int index);                  // 0x567D00
    void Draw3DObjectiveLocations();                  // 0x590BF0
    void SetHideCompassStar(int active, int index);      // 0x568850
    int  IsCompassStarHidden(int index);              // 0x568870
    void SetHideUpdatedText(int active, int objectiveIndex);  // 0x5688A0
    int  IsUpdatedTextHidden(int index);              // 0x5688C0
    static int ObjectiveStateIndexFromString(const char* name);  // 0x5688F0
    void UpdateCompassRotation();                     // 0x578580
    void CheckpointRestart();                         // 0x578800
private:
    void UpdateCompassDial();                         // 0x567FE0
    void UpdateCompassFrame();                        // 0x5680D0
    void DrawEnemies();                               // 0x568250
    void DrawFriendlies();                            // 0x568340
    void DrawTanks();                                 // 0x568440
    void DrawObjectives();                            // 0x568540
    void CalculateRing(IGOMapObject* mo, float ring_time);  // 0x568660
    void UpdateVehcile();                             // 0x5689A0
    void CalculateMapObjectABS(IGOMapObject* mo, float dist,
                               float yaw, float ring_time,
                               bool is_objective);    // 0x578820
    void CalculateMapObject(IGOMapObject* mo, float dist,
                            float yaw, float ring_time,
                            bool is_objective);       // 0x583AA0
    void UpdateFriendlies();                          // 0x5890A0
    void UpdateEnemies();                             // 0x5895F0
    void UpdateObjectives();                          // 0x589880
    void UpdateTanks();                               // 0x589AD0
    void DrawVehcile();                               // 0x589F40
    void UpdateObjectivesABS();                       // 0x58A020
    void UpdateEnemiesABS();                          // 0x58A230
    void UpdateTanksABS();                            // 0x58A4F0
};
static_assert(sizeof(IGOCompassWidget) == 0x1488,
              "IGOCompassWidget size mismatch");

// ============================================================================
// IGOFrontEnd - in-game overlay front end (168 bytes) - verified against IDA
// ============================================================================
class IGOFrontEnd : public PanelFileUser {
public:
    IGOCompassWidget*  compassWidget[1];       // +0x04
    IGOJeepMapWidget*  jeepMapWidget[1];       // +0x08
    IGOStanceWidget*   stanceWidget[1];        // +0x0C
    IGOHealthWidget*   healthWidget[1];        // +0x10
    IGOAmmoWidget*     ammoWidget[1];          // +0x14
    IGOWeaponNameWidget* weaponNameWidget[1];  // +0x18
    IGOHintWidget*     hintWidget[1];          // +0x1C
    IGOTankHealthWidget* tankHealthWidget;     // +0x20
    IGOTankLoadingWidget* tankLoadingWidget[1];// +0x24
    IGOGrenadeWidget*  grenadeWidget[1];       // +0x28
    IGOGrenadeCookWidget* grenadeCookWidget[1];// +0x2C
    IGOTankIconWidget* tankIconWidget[1];      // +0x30
    FEMultiLineText*   hintText[1];            // +0x34
    IGOTankReticleWidget* mTankReticleWidget[1];  // +0x38
    IGOGrenadeIndicator* mGrenadeIndicator[1]; // +0x3C
    IGOVoipList*       mVoipList[1];           // +0x40
    int                actionHintText[1];     // +0x44
    IGOTimerWidget*    mTimerWidget[1];        // +0x48
    IGOInGameScoreWidget* mGameScoreWidget[1]; // +0x4C
    IGOHeadIcons*      mHeadIcons[1];          // +0x50
    IGOItemIcons*      mItemIcons[1];          // +0x54
    IGORankWidget*     mRankWidget[1];         // +0x58
    IGOVoteWidget*     mVoteWidget[1];         // +0x5C
    IGOSpecialWeaponWidget* mSpecialWeaponWidget[1];  // +0x60
    IGOHQProgressBarWidget* mHQProgressBarWidget[1];  // +0x64
    IGORaiseFlagWidget* mRaiseFlagWidget[1];   // +0x68
    IGOWarStatusWidget* mWarStatusWidget[1];   // +0x6C
    IGOActionHintWidget* actionHintWidget[1];  // +0x70
    PanelFile*         panel;                  // +0x74
    PanelFile*         iconsPanel;             // +0x78
    PanelFile*         mpPanel;                // +0x7C
    PanelFile*         spJeepMapPanel;         // +0x80
    char*              activate_key;           // +0x84
    char*              run_key;                // +0x88
    char*              speed_key;              // +0x8C
    bool               key_bindings_set;       // +0x90
    int                previous_widescreen;    // +0x94
    int                previous_splitscreen;   // +0x98
    float              hintTimer[1];           // +0x9C
    int                actionHintTimer[1];     // +0xA0
    hud_type           current_type[1];        // +0xA4

    virtual ~IGOFrontEnd();                    // slot 3 0x564F70
    virtual void Update(float time_inc);        // slot 4 0x5821F0
    virtual void UpdateInScene(float time_inc); // slot 5 0x565140
    virtual void Draw(int client);              // slot 6 0x565350
    virtual void DrawHint(int viewport);        // slot 7 0x565300

    IGOFrontEnd();                              // 0x59CA80
    virtual void SetPanelFile(PanelFile* pf);   // slot 0 0x59C0F0
    virtual void PanelFileUnloaded(PanelFile* pf);  // slot 1 0x565110
    virtual void UpdateWidescreen(bool widescreen); // slot 2 0x577200
    void SetTutorialText(int ref, int viewport);// 0x565180
    void SetActionHint(int ref, int viewport);  // 0x565260
    void Draw3DWorldSpace();                    // 0x565660
    void Draw3DScreenSpace();                   // 0x5771D0
    void UpdateSplitScreen();                   // 0x565690
    void ResetWidgets();                        // 0x565920
    void TurnOffMostWidgets();                  // 0x565A40
    void SetForLiberatorBomber();               // 0x565B50
    void SetForLiberatorGround();               // 0x565B60
    void SetForTunisia();                       // 0x565BF0
    void SetHUDType(hud_type ht, int viewport); // 0x565C40
    void FindKeyBindings();                     // 0x577420
    void SetFuse(float total, float remain,
                 int client);                   // 0x5775A0
    void AddActiveGrenade(const Entity* grenade);  // 0x590AF0
    void UpdateAfterWeaponsLoaded();            // 0x59C460
    const char* GetLMGKey();                    // 0x56DE50
};
static_assert(sizeof(IGOFrontEnd) == 0xA8, "IGOFrontEnd size mismatch");

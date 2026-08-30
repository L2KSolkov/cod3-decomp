// ============================================================================
// COD3 Color type — RGBA float color (16 bytes)
// Referenced as `Color` (global namespace) with constructor at ea:0x448920.
// Size: 0x10 (16 bytes)
// ============================================================================

#pragma once

class Color;

// Color32 (IDA type; class V-tag; 4 bytes)
class Color32 {
public:
    union {
        struct {
            unsigned char b;  // +0x00
            unsigned char g;  // +0x01
            unsigned char r;  // +0x02
            unsigned char a;  // +0x03
        } c;
        unsigned int i;
    };

    Color32(unsigned char _r, unsigned char _g, unsigned char _b,
            unsigned char _a);
    // ea: 0x00663200
    Color32(unsigned int packed);
    // ea: 0x00663220
    unsigned int to_ulong() const;
    Color to_color();
};
static_assert(sizeof(Color32) == 4, "Color32 size mismatch");

class Color {
public:
    float r;  // +0x00
    float g;  // +0x04
    float b;  // +0x08
    float a;  // +0x0C

    // Release constructor is an intentional no-op; callers provide the
    // components explicitly or overwrite the object before use.
    // ea: 0x00448920
    Color() {}
    // ea: 0x00448930
    Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    // ea: 0x00448970
    Color(const Color& other) : r(other.r), g(other.g), b(other.b), a(other.a) {}
    // ea: 0x004489A0
    Color& operator=(const Color& other) {
        r = other.r; g = other.g; b = other.b; a = other.a;
        return *this;
    }
    float get_red() const;    // ?get_red@Color@@QBEMXZ (render.o 0x6E5A30)
    float get_green() const;  // ?get_green@Color@@QBEMXZ (render.o 0x6E5A40)
    float get_blue() const;   // ?get_blue@Color@@QBEMXZ (render.o 0x6E5A50)
    float get_alpha() const;  // ?get_alpha@Color@@QBEMXZ (render.o 0x6E5A60)
    Color operator*(const Color& c) const;  // ??DColor@@QBE?AV0@ABV0@@Z (render.o 0x6E5D50)
    void clamp();                // ?clamp@Color@@QAEXXZ (render.o 0x6E5CB0)
    Color32 to_color32() const;  // ?to_color32@Color@@QBE?AVColor32@@XZ (render.o 0x6E5A70)
};
static_assert(sizeof(Color) == 0x10, "Color size mismatch");

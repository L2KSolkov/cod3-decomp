// ============================================================================
// COD3 Color type — RGBA float color (16 bytes)
// Referenced as `Color` (global namespace) with constructor at ea:0x448920.
// Size: 0x10 (16 bytes)
// ============================================================================

#pragma once

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
};
static_assert(sizeof(Color32) == 4, "Color32 size mismatch");

class Color {
public:
    float r;  // +0x00
    float g;  // +0x04
    float b;  // +0x08
    float a;  // +0x0C

    Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    Color(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    Color(const Color& other) : r(other.r), g(other.g), b(other.b), a(other.a) {}
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

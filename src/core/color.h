// ============================================================================
// COD3 Color type — RGBA float color (16 bytes)
// Referenced as `Color` (global namespace) with constructor at ea:0x448920.
// Size: 0x10 (16 bytes)
// ============================================================================

#pragma once

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
};
static_assert(sizeof(Color) == 0x10, "Color size mismatch");

// ============================================================================
// COD3 Math Types — geometry primitives used throughout the engine
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "core/sse_portable.h"
#include <math.h>

namespace math {

unsigned int mathInit();
long double Abs(float a);

// Forward declarations
class Dir3;
class Position3;
class Vector4;
class Quaternion;
class Mat33;
class Mat44;
class Mat43;
class TranMat43;
class DiagMat33;

// ============================================================================
// DiagMat33 - diagonal 3x3 scale (16 bytes: packed x/y/z scale + pad w).
// Size: 0x10 - verified against IDA (math::DiagMat33).
// ============================================================================
class DiagMat33 {
public:
    __m128 v;  // SSE-packed: x, y, z, w

    DiagMat33();                     // ??0DiagMat33@math@@QAE@XZ (anim.o 0x539DC0)
    DiagMat33(const DiagMat33& m);   // ??0DiagMat33@math@@QAE@ABV01@@Z (anim.o 0x539F60)
    DiagMat33(float _x, float _y, float _z);  // ??0DiagMat33@math@@QAE@MMM@Z (render.o 0x6E6510)
    const DiagMat33& operator=(const DiagMat33& m);  // ??4DiagMat33@math@@QAEABV01@ABV01@@Z (render.o 0x6E66E0)
    Dir3 GetX() const;               // ?GetX@DiagMat33@math@@QBE?AVDir3@2@XZ (anim.o 0x539DD0)
    Dir3 GetY() const;               // anim.o 0x539E10
    Dir3 GetZ() const;               // anim.o 0x539E60
};
static_assert(sizeof(DiagMat33) == 0x10, "DiagMat33 size mismatch");

// ============================================================================
// Dir3 — normalized direction vector (3 float + pad w, total 16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// Note: declared `class` (not struct) to match the original binary's MSVC
// mangling (`?AVDir3` / `ABV...Dir3...`), which uses V for class types.
// ============================================================================
class Dir3 {
public:
    __m128 v;  // SSE-packed: x, y, z, w
    struct Packed;

    Dir3();                       // ??0Dir3@math@@QAE@XZ (g.o 0x4A55C0)
    Dir3(__m128 _v) : v(_v) {}
    Dir3(float _x, float _y, float _z);  // ??0Dir3@math@@QAE@MMM@Z (g.o 0x4A55D0)
    Dir3(float _x);               // ??0Dir3@math@@QAE@M@Z (g.o 0x4A5640)
    Dir3(const Dir3& other) : v(other.v) {}  // implicit copy (aggregate support)
    Dir3(const Position3& _v);    // ??0Dir3@math@@QAE@ABVPosition3@1@@Z
    Dir3(const Vector4& _v);      // ??0Dir3@math@@QAE@ABVVector4@1@@Z
    const Dir3& operator=(const Dir3::Packed& _p); // game.o 0x006022C0
    const Dir3& operator=(const Position3& _v);  // ??4Dir3@math@@QAEABV01@ABVPosition3@1@@Z
    const Dir3& operator=(const Vector4& _v);    // game2.o 0x004EAF50
    double GetX() const;          // ?GetX@Dir3@math@@QBEMXZ
    double GetY() const;          // ?GetY@Dir3@math@@QBEMXZ
    float GetZ() const;           // ?GetZ@Dir3@math@@QBEMXZ (g.o 0x4A56A0)
    void SetX(float _x);          // ?SetX@Dir3@math@@QAEXM@Z
    void SetY(float _y);          // ?SetY@Dir3@math@@QAEXM@Z
    void SetZ(float _z);          // ?SetZ@Dir3@math@@QAEXM@Z (g.o 0x4A56F0)
    float& operator[](unsigned int i);        // ??ADir3@math@@QAEAAMI@Z
    const float& operator[](unsigned int i) const;  // ??ADir3@math@@QBEABMI@Z

    // apsMath.o (non-inline): row-vector * 3x3 matrix. Unresolved here.
    const math::Dir3& operator*=(const math::Mat33& m);
    const math::Dir3& operator+=(float _v);  // ??YDir3@math@@QAEABV01@M@Z (render.o 0x6E6490)
    const math::Dir3& operator-=(float _v);  // ??ZDir3@math@@QAEABV01@M@Z (render.o 0x6E64D0)
    const math::Dir3& operator+=(const math::Dir3& _v); // game2.o 0x004EB190
    const math::Dir3& operator*=(float _v);              // game2.o 0x004EB1D0

    // cg.o inline COMDAT (??ZDir3@math@@QAEABV01@ABVPosition3@1@@Z)
    const math::Dir3& operator-=(const math::Position3& v);
    const math::Dir3& operator-=(const math::Dir3& v);  // game.o 0x0065B400
    const math::Dir3& operator/=(float _v);  // ??_0Dir3@math@@QAEABV01@M@Z (g.o 0x4A6CF0)

    // Constant layout (for compile-time initialization)
    struct Constant {
        float x, y, z, w;
    };

    // Packed layout (3 floats, 12 bytes — for network/disk)
    struct Packed {
        float x, y, z;
        void Set(const Dir3& v);  // game.o 0x00602050
        const Packed& operator=(const Dir3& v);  // game.o 0x006020E0
        float GetX() const;  // ?GetX@Packed@Dir3@math@@QBEMXZ (render.o 0x6E5EC0)
        float GetY() const;  // ?GetY@Packed@Dir3@math@@QBEMXZ (render.o 0x6E5ED0)
        float GetZ() const;  // ?GetZ@Packed@Dir3@math@@QBEMXZ (render.o 0x6E5EE0)
    };
    Dir3(const Dir3::Packed& _p);  // ??0Dir3@math@@QAE@ABUPacked@01@@Z
};
static_assert(sizeof(Dir3) == 0x10, "Dir3 size mismatch");
static_assert(sizeof(Dir3::Constant) == 0x10, "Dir3::Constant size mismatch");
static_assert(sizeof(Dir3::Packed) == 0x0C, "Dir3::Packed size mismatch");

// ============================================================================
// Position3 — point/translation vector (3 float + pad w, total 16 bytes)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
class Position3 {
public:
    __m128 v;  // SSE-packed: x, y, z, w

    Position3() {}
    Position3(__m128 _v) : v(_v) {}
    Position3(float _x, float _y, float _z);  // ??0Position3@math@@QAE@MMM@Z (g.o 0x4A57A0)
    Position3(float _x);           // ??0Position3@math@@QAE@M@Z (g.o 0x4A5830)
    Position3(const Position3& other) : v(other.v) {}  // implicit copy
    Vector4 val34() const;          // ?val34@Position3@math@@QBE?AVVector4@2@XZ (scr.o 0x5E9BB0)
    Position3(const Dir3& _v);    // ??0Position3@math@@QAE@ABVDir3@1@@Z
    const Position3& operator=(const Dir3& _v); // game.o 0x00602330
    const Position3& operator*=(const Mat43& _m);  // ??XPosition3@math@@QAEABV01@ABVMat43@1@@Z (render.o 0x6E67E0)
    const Position3& operator+=(const Dir3& _v);       // ??YPosition3@math@@QAEABV01@ABVDir3@1@@Z (g.o 0x4A6D30)
    const Position3& operator-=(const Dir3& _v);       // game.o 0x0065B440
    const Position3& operator+=(const Position3& _v);  // ??YPosition3@math@@QAEABV01@ABV01@@Z (g.o 0x4A6D70)
    const Position3& operator*=(float _v);             // ??XPosition3@math@@QAEABV01@M@Z (g.o 0x4A6DB0)
    const Position3& operator/=(float _v);             // ??_0Position3@math@@QAEABV01@M@Z (g.o 0x4A6DF0)

    float GetX() const;           // ?GetX@Position3@math@@QBEMXZ (g.o 0x4A58B0)
    float GetY() const;           // ?GetY@Position3@math@@QBEMXZ (g.o 0x4A58D0)
    float GetZ() const;           // ?GetZ@Position3@math@@QBEMXZ (g.o 0x4A5930)
    float GetW() const;           // ?GetW@Position3@math@@QBEMXZ (game.o 0x00602170)
    void SetX(float _x);          // ?SetX@Position3@math@@QAEXM@Z (g.o 0x4A59B0)
    void SetY(float _y);          // ?SetY@Position3@math@@QAEXM@Z (g.o 0x4A59F0)
    void SetZ(float _z);          // ?SetZ@Position3@math@@QAEXM@Z (g.o 0x4A5A30)
    float& operator[](unsigned int i);        // ??APosition3@math@@QAEAAMI@Z (g.o 0x4A5A70)
    const float& operator[](unsigned int i) const;  // ??APosition3@math@@QBEABMI@Z (g.o 0x4A5A80)

    // ??0Position3@math@@QAE@ABVVector4@1@@Z (anim.o 0x539D60)
    Position3(const Vector4& v);

    // ??4Position3@math@@QAEABV01@ABVVector4@1@@Z (anim.o; defined in nal.cpp)
    const Position3& operator=(const Vector4& v);

    struct Constant {
        float x, y, z, w;
    };

    struct Packed {
        float x, y, z;
        Packed(const Position3& v);  // game.o 0x00602180
        void Set(const Position3& v);  // game2.o 0x004EACD0
        float GetX() const;            // game2.o 0x004EAD60
        float GetY() const;            // game2.o 0x004EAD70
        float GetZ() const;            // game2.o 0x004EAD80
        const Packed& operator=(const Position3& v);  // game2.o 0x004EAD90
        Packed();                       // game2.o 0x004EAE20
    };
    Position3(const Position3::Packed& _p);  // ??0Position3@math@@QAE@ABUPacked@01@@Z
    const Position3& operator=(const Position3::Packed& _p);  // ??4Position3@math@@QAEABV01@ABUPacked@01@@Z
};
static_assert(sizeof(Position3) == 0x10, "Position3 size mismatch");
static_assert(sizeof(Position3::Constant) == 0x10, "Position3::Constant size mismatch");
static_assert(sizeof(Position3::Packed) == 0x0C, "Position3::Packed size mismatch");

// ea: 0x00687920
inline const math::Dir3& Dir3::operator-=(const math::Position3& v)
{
    this->v = _mm_sub_ps(this->v, v.v);
    return *this;
}

float Cos(float radians);  // ?Cos@math@@YAMM@Z (render.o 0x6E7120)
float Min(float a, float b);  // ?Min@math@@YAMMM@Z (ngl_debug.o 0x835540)
float Sin(float radians);  // ?Sin@math@@YAMM@Z (ngl_debug.o 0x835800)

// g.o free-function vector math (0x4A5FB0-0x4A65E0)
Dir3 operator-(const Dir3& _v);  // ??Gmath@@YA?AVDir3@0@ABV10@@Z
Vector4 RepeatX(const Vector4& _v);  // ?RepeatX@math@@YA?AVVector4@1@ABV21@@Z (game2.o 0x004EAFB0)
Vector4 RepeatY(const Vector4& _v);  // ?RepeatY@math@@YA?AVVector4@1@ABV21@@Z (game2.o 0x004EAFF0)
Vector4 RepeatZ(const Vector4& _v);  // ?RepeatZ@math@@YA?AVVector4@1@ABV21@@Z (game2.o 0x004EB030)
Vector4 RepeatW(const Vector4& _v);  // ?RepeatW@math@@YA?AVVector4@1@ABV21@@Z (game2.o 0x004EB070)
Dir3 operator+(const Dir3& _a, const Dir3& _b); // ??Hmath@@YA?AVDir3@0@ABV10@0@Z (game2.o 0x004EB0B0)
Dir3 operator+(const Dir3& _a, float _b);        // game.o 0x0065B1C0
Dir3 operator-(const Dir3& _a, const Dir3& _b); // ??Gmath@@YA?AVDir3@0@ABV10@0@Z (game2.o 0x004EB0F0)
float Dot(const Dir3& _a, const Dir3& _b);       // ?Dot@math@@YAMABVDir3@1@0@Z (game2.o 0x004EB130)
Position3 operator-(const Position3& _v);
Vector4 operator-(const Vector4& _v);
float Length(const Dir3& _v);  // ?Length@math@@YAMABVDir3@1@@Z
float AbsSquared(const Dir3& _v);
float AbsSquared(const Position3& _v);
float Abs(const Dir3& _v);
float Abs(const Position3& _v);
Vector4 AbsValue(const Vector4& _v);
Dir3 AbsValue(const Dir3& _v);                                  // game.o 0x0065B030
Vector4 Ceil(const Vector4& _v);
bool operator==(const Dir3& _a, const Dir3& _b);                // game.o 0x0065B0E0
bool operator==(const Position3& _a, const Position3& _b);
bool operator!=(const Position3& _a, const Position3& _b);
Dir3 operator+(const Dir3& _a, const Position3& _b);
Position3 operator+(const Position3& _a, const Dir3& _b);
Position3 operator+(const Position3& _a, const Position3& _b);  // ??Hmath@@YA?AVPosition3@0@ABV10@0@Z (g.o 0x4A6630)
Vector4 operator+(const Vector4& _a, const Vector4& _b);        // ??Hmath@@YA?AVVector4@0@ABV10@0@Z (g.o 0x4A6670)
Vector4 operator+(const Position3& _a, const Vector4& _b);     // game.o 0x0065B200
Vector4 operator-(const Vector4& _a, const Vector4& _b);        // game.o 0x0065B250
Vector4 operator-(const Position3& _a, const Vector4& _b);     // game.o 0x0065B2B0
Dir3 Normalize(const Dir3& _v);                                  // ?Normalize@math@@YA?AVDir3@1@ABV21@@Z (game2.o 0x004EB2D0)
Vector4 Vector4_Zero();                                          // ?Vector4_Zero@math@@YA?AVVector4@1@XZ (game2.o 0x004EB350)
Dir3 operator-(const Dir3& _a, const Position3& _b);            // ??Gmath@@YA?AVDir3@0@ABV10@ABVPosition3@0@@Z (g.o 0x4A66B0)
Position3 operator-(const Position3& _a, const Dir3& _b);       // ??Gmath@@YA?AVPosition3@0@ABV10@ABVDir3@0@@Z (g.o 0x4A66F0)
Position3 operator-(const Position3& _a, const Position3& _b);  // ??Gmath@@YA?AVPosition3@0@ABV10@0@Z (g.o 0x4A6730)
Position3 operator/(const Position3& _a, float _b);             // game.o 0x0065B330
Dir3 operator/(const Dir3& _a, float _b);                       // ??Kmath@@YA?AVDir3@0@ABV10@M@Z (g.o 0x4A6770)
Dir3 operator*(const Dir3& _a, float _b);                       // ??Dmath@@YA?AVDir3@0@ABV10@M@Z (g.o 0x4A6830)
Dir3 operator*(float _a, const Dir3& _b);                       // ??Dmath@@YA?AVDir3@0@MABV10@@Z (g.o 0x4A6870)
Position3 operator*(const Position3& _a, float _b);             // ??Dmath@@YA?AVPosition3@0@ABV10@M@Z (g.o 0x4A6910)
Position3 operator*(float _a, const Position3& _b);             // ??Dmath@@YA?AVPosition3@0@MABV10@@Z (g.o 0x4A6950)
float operator*(const Dir3& _a, const Dir3& _b);                // ??Dmath@@YAMABVDir3@0@0@Z (g.o 0x4A69A0)
float operator*(const Dir3& _a, const Position3& _b);           // ??Dmath@@YAMABVDir3@0@ABVPosition3@0@@Z (g.o 0x4A6A00)
float operator*(const Position3& _a, const Dir3& _b);           // ??Dmath@@YAMABVPosition3@0@ABVDir3@0@@Z (g.o 0x4A6B10)
float operator*(const Position3& _a, const Position3& _b);      // game.o 0x0065B3A0
Dir3 UnitNegDirX();                                             // game.o 0x0065B480
Dir3 UnitNegDirY();                                             // game.o 0x0065B4C0
Dir3 UnitNegDirZ();                                             // game.o 0x0065B500
Vector4 Mul(const Vector4& _a, const Vector4& _b);              // ?Mul@math@@YA?AVVector4@1@ABV21@0@Z (g.o 0x4A6BC0)
Vector4 Mul(const Vector4& _a, float _b);                       // nal_generic.o 0x00868A80
Vector4 Vector4_Half();                                         // ?Vector4_Half@math@@YA?AVVector4@1@XZ (ngl_debug.o 0x835570)
Vector4 Sin(const Vector4& radians);                            // ?Sin@math@@YA?AVVector4@1@ABV21@@Z (ngl_debug.o 0x835720)
Dir3 Cross(const Dir3& _a, const Dir3& _b);                     // ?Cross@math@@YA?AVDir3@1@ABV21@0@Z (g.o 0x4A6C00)
Position3 Min(const Position3& _a, const Position3& _b);        // ?Min@math@@YA?AVPosition3@1@ABV21@0@Z (g.o 0x4A6C70)
Position3 Max(const Position3& _a, const Position3& _b);        // ?Max@math@@YA?AVPosition3@1@ABV21@0@Z (g.o 0x4A6CB0)
Dir3 DeclareUnit(const Dir3& _v);                               // ?DeclareUnit@math@@YA?AVDir3@1@ABV21@@Z (g.o 0x4A6E70)
Dir3 Unitize(const Dir3& _v);                                   // ?Unitize@math@@YA?AVDir3@1@ABV21@@Z (g.o 0x4A6EA0)
Dir3 Dir3_Zero();                                               // ?Dir3_Zero@math@@YA?AVDir3@1@XZ (g.o 0x4A6F20)
Position3 Position3_Zero();                                     // ?Position3_Zero@math@@YA?AVPosition3@1@XZ (g.o 0x4A6F60)

// ============================================================================
// Vector4 — 4-component float vector (16 bytes)
// Size: 0x10 (16 bytes)
// ============================================================================
class Vector4 {
public:
    __m128 v;

    Vector4(float _x, float _y, float _z, float _w);  // ??0Vector4@math@@QAE@MMMM@Z (g.o 0x4A5AA0)
    Vector4(float _x);           // ??0Vector4@math@@QAE@M@Z (g.o 0x4A5B10)
    float GetX() const;           // ?GetX@Vector4@math@@QBEMXZ (g.o 0x4A5B80)
    float GetY() const;           // ?GetY@Vector4@math@@QBEMXZ (g.o 0x4A5BA0)
    float GetZ() const;           // ?GetZ@Vector4@math@@QBEMXZ
    float GetW() const;           // ?GetW@Vector4@math@@QBEMXZ
    void SetX(float _x);          // ?SetX@Vector4@math@@QAEXM@Z
    void SetY(float _y);          // ?SetY@Vector4@math@@QAEXM@Z
    void SetZ(float _z);          // ?SetZ@Vector4@math@@QAEXM@Z
    void SetW(float _w);          // ?SetW@Vector4@math@@QAEXM@Z

    struct Packed {
        float x, y, z, w;
        float GetX() const;  // ?GetX@Packed@Vector4@math@@QBEMXZ (render.o 0x6E6020)
        float GetY() const;  // ?GetY@Packed@Vector4@math@@QBEMXZ (render.o 0x6E6030)
        float GetZ() const;  // ?GetZ@Packed@Vector4@math@@QBEMXZ (render.o 0x6E6040)
        float GetW() const;  // ?GetW@Packed@Vector4@math@@QBEMXZ (render.o 0x6E6050)
        void SetX(float _x);  // ?SetX@Packed@Vector4@math@@QAEXM@Z (render.o 0x6E5FA0)
        void SetY(float _y);  // ?SetY@Packed@Vector4@math@@QAEXM@Z (render.o 0x6E5FC0)
        void SetZ(float _z);  // ?SetZ@Packed@Vector4@math@@QAEXM@Z (render.o 0x6E5FE0)
        void SetW(float _w);  // ?SetW@Packed@Vector4@math@@QAEXM@Z (render.o 0x6E6000)
        void Set(const Vector4& v);  // ?Set@Packed@Vector4@math@@QAEXABV23@@Z (render.o 0x6E5EF0)
        const Packed& operator=(const Vector4& v);  // ??4Packed@Vector4@math@@QAEABU012@ABV12@@Z (render.o 0x6E6060)
    };

    struct Constant {
        float x, y, z, w;
    };

    Vector4() {}
    Vector4(__m128 _v) : v(_v) {}
    Vector4(const Vector4& other) : v(other.v) {}  // implicit copy
    Vector4(const Vector4::Constant& _c);  // ??0Vector4@math@@QAE@ABUConstant@01@@Z (g.o 0x4A5F80)
    Vector4(const Position3& _v);           // ??0Vector4@math@@QAE@ABVPosition3@1@@Z (core.o 0x4DB7B0)
    Vector4(const Dir3& _v);  // ??0Vector4@math@@QAE@ABVDir3@1@@Z
    Vector4(const Quaternion& _v);  // ??0Vector4@math@@QAE@ABVQuaternion@1@@Z (game2.o 0x00516B70)
    Vector4(const Dir3& _v, float _w);  // ??0Vector4@math@@QAE@ABVDir3@1@M@Z (render.o 0x6E6110)
    Vector4(const Vector4::Packed& _p); // ??0Vector4@math@@QAE@ABUPacked@01@@Z (render.o 0x6E6160)
    const Vector4& operator=(const Vector4::Packed& _p);  // ??4Vector4@math@@QAEABV01@ABUPacked@01@@Z (render.o 0x6E6220)
    const Vector4& operator=(const Position3& _v);  // ??4Vector4@math@@QAEABV01@ABVPosition3@1@@Z (render.o 0x6E61D0)
    const Vector4& operator=(const Dir3& _v);      // game2.o 0x004EAF70

    float operator[](int i) const { return v.m128_f32[i]; }
    float& operator[](int i) { return v.m128_f32[i]; }

    const Vector4& operator*=(const Mat44& m);  // ??XVector4@math@@QAEABV01@ABVMat44@1@@Z (streamer.o)
    const Vector4& operator+=(const Vector4& _v);  // game2.o 0x004EB210
    const Vector4& operator*=(float _v);           // game2.o 0x004EB250
    const Vector4& operator/=(float _v);           // game2.o 0x004EB290
    const Vector4& operator-=(const Vector4& _v);  // ??ZVector4@math@@QAEABV01@ABV01@@Z (g.o 0x4A6E30)
};
static_assert(sizeof(Vector4) == 0x10, "Vector4 size mismatch");

// ea: 0x005E9BB0
inline math::Vector4 math::Position3::val34() const
{
    __m128 one = _mm_set_ss(1.0f);
    __m128 z = _mm_shuffle_ps(one, v, _MM_SHUFFLE(2, 2, 0, 0));
    math::Vector4 result;
    result.v = _mm_shuffle_ps(v, z, _MM_SHUFFLE(0, 3, 1, 0));
    return result;
}

// ============================================================================
// Mat43 — 4x3 affine transform matrix (64 bytes)
//   3 rotation axes (Dir3 each, 16 bytes) + 1 translation (Position3, 16 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
class Mat43 {
public:
    Mat43() {}
    Mat43(const Dir3& _x, const Dir3& _y, const Dir3& _z,
          const Position3& _w);            // ??0Mat43@math@@QAE@ABVDir3@1@00ABVPosition3@1@@Z (core.o 0x4DBB90)

    // ??0Mat43@math@@QAE@ABVDiagMat33@1@@Z (anim.o 0x539FC0)
    Mat43(const DiagMat33& m);
    Mat43(const TranMat43& m);  // ??0Mat43@math@@QAE@ABVTranMat43@1@@Z (ngl_debug.o 0x835670)

    // ??4Mat43@math@@QAEABV01@ABVDiagMat33@1@@Z (anim.o; defined in nal.cpp)
    const Mat43& operator=(const DiagMat33& m);
    const Mat43& operator*=(const TranMat43& m);  // ??XMat43@math@@QAEABV01@ABVTranMat43@1@@Z (render.o 0x6E7040)
    const Mat43& operator=(const Mat43& _m);           // ??4Mat43@math@@QAEABV01@ABV01@@Z (g.o 0x4A74A0)
    const Dir3& GetX() const;      // ?GetX@Mat43@math@@QBEABVDir3@2@XZ (g.o 0x4A7070)
    const Dir3& GetY() const;      // ?GetY@Mat43@math@@QBEABVDir3@2@XZ (g.o 0x4A7080)
    const Dir3& GetZ() const;      // ?GetZ@Mat43@math@@QBEABVDir3@2@XZ (g.o 0x4A7090)
    const Position3& GetW() const; // ?GetW@Mat43@math@@QBEABVPosition3@2@XZ (g.o 0x4A70A0)
    Dir3& GetX();                  // ?GetX@Mat43@math@@QAEAAVDir3@2@XZ (g.o 0x4A70B0)
    Dir3& GetY();                  // ?GetY@Mat43@math@@QAEAAVDir3@2@XZ (g.o 0x4A70C0)
    Dir3& GetZ();                  // ?GetZ@Mat43@math@@QAEAAVDir3@2@XZ (g.o 0x4A70D0)
    Position3& GetW();             // ?GetW@Mat43@math@@QAEAAVPosition3@2@XZ (g.o 0x4A70E0)
    void SetX(const Dir3& _x);     // ?SetX@Mat43@math@@QAEXABVDir3@2@@Z (core.o 0x4DBBC30)
    void SetY(const Dir3& _y);     // ?SetY@Mat43@math@@QAEXABVDir3@2@@Z (core.o 0x4DBBC60)
    void SetZ(const Dir3& _z);     // ?SetZ@Mat43@math@@QAEXABVDir3@2@@Z (core.o 0x4DBBC90)
    void SetW(const Position3& _w); // ?SetW@Mat43@math@@QAEXABVPosition3@2@@Z (core.o 0x4DBBCC0)
    Dir3      x;  // +0x00 — right axis
    Dir3      y;  // +0x10 — forward axis
    Dir3      z;  // +0x20 — up axis
    Position3 w;  // +0x30 — translation

    // Compressed form (48 bytes: 3 * 12-byte Packed dirs + 12-byte Packed pos)
    struct Packed {
        Dir3::Packed      x;
        Dir3::Packed      y;
        Dir3::Packed      z;
        Position3::Packed w;
    };

    Mat43(const Mat43::Packed& _p);                    // ??0Mat43@math@@QAE@ABUPacked@01@@Z (g.o 0x4A7200)
    Mat43(const Mat33& _m, const Position3& _p);       // ??0Mat43@math@@QAE@ABVMat33@1@ABVPosition3@1@@Z (g.o 0x4A7360)
};
static_assert(sizeof(Mat43) == 0x40, "Mat43 size mismatch");
static_assert(sizeof(Mat43::Packed) == 0x30, "Mat43::Packed size mismatch");

// ============================================================================
// Mat33 — 3x3 rotation matrix (48 bytes = 3 * Dir3)
// Size: 0x30 (48 bytes)
// Note: declared `class` (not struct) to match the original binary's MSVC
// mangling (`ABVMat33` / `?AVMat33`), which uses V for class types.
// Members are x/y/z (Dir3 rows) per the original source.
// ============================================================================
class Mat33 {
public:
    Dir3 x;  // +0x00
    Dir3 y;  // +0x10
    Dir3 z;  // +0x20

    Mat33() {}
    Mat33(const Mat33& _m);        // ??0Mat33@math@@QAE@ABV01@@Z (g.o 0x4A7100)
    Mat33(const Dir3& _x, const Dir3& _y, const Dir3& _z);  // ??0Mat33@math@@QAE@ABVDir3@1@00@Z (g.o 0x4A6FB0)
    Mat33(const Quaternion& _q);   // ??0Mat33@math@@QAE@ABVQuaternion@1@@Z (game2.o 0x00516FD0)
    const Mat33& operator=(const Mat33& _m);  // ??4Mat33@math@@QAEABV01@ABV01@@Z (g.o 0x4A7180)
    const Mat33& operator*=(const Mat33& _m);  // ??XMat33@math@@QAEABV01@ABV01@@Z (g.o 0x4A7F60)
    const Dir3& GetX() const;  // ?GetX@Mat33@math@@QBEABVDir3@2@XZ (g.o 0x4A7030)
    const Dir3& GetY() const;  // ?GetY@Mat33@math@@QBEABVDir3@2@XZ (g.o 0x4A7040)
    const Dir3& GetZ() const;  // ?GetZ@Mat33@math@@QBEABVDir3@2@XZ (g.o 0x4A7050)
    Dir3& GetX();              // ?GetX@Mat33@math@@QAEAAVDir3@2@XZ (game2.o 0x004EB3D0)
    Dir3& GetY();              // ?GetY@Mat33@math@@QAEAAVDir3@2@XZ (game2.o 0x004EB3E0)
    Dir3& GetZ();              // ?GetZ@Mat33@math@@QAEAAVDir3@2@XZ (game2.o 0x004EB3F0)
    Dir3& operator[](unsigned int i); // ??AMat33@math@@QAEAAVDir3@1@I@Z (game2.o 0x004EB420)
    void SetX(const Dir3& _x);  // ?SetX@Mat33@math@@QAEXABVDir3@2@@Z (ngl_debug.o 0x8355A0)
    void SetY(const Dir3& _y);  // ?SetY@Mat33@math@@QAEXABVDir3@2@@Z (ngl_debug.o 0x8355C0)
    void SetZ(const Dir3& _z);  // ?SetZ@Mat33@math@@QAEXABVDir3@2@@Z (ngl_debug.o 0x8355F0)
};
static_assert(sizeof(Mat33) == 0x30, "Mat33 size mismatch");

// g.o matrix free functions (0x4A75B0-0x4A82C0)
Position3 Mul(const Position3& _v, const Mat43& _m);  // ?Mul@math@@YA?AVPosition3@1@ABV21@ABVMat43@1@@Z (g.o 0x4A75B0)
Dir3 Mul(const Dir3& _v, const Mat44& _m);            // ?Mul@math@@YA?AVDir3@1@ABV21@ABVMat44@1@@Z (game2.o 0x004EB580)
Dir3 operator*(const Dir3& _v, const Mat44& _m);      // ??Dmath@@YA?AVDir3@0@ABV10@ABVMat44@0@@Z (game2.o 0x004EB5F0)
Vector4 Mul(const Vector4& _v, const Mat44& _m);      // ?Mul@math@@YA?AVVector4@1@ABV21@ABVMat44@1@@Z (game2.o 0x004EB660)
Vector4 operator*(const Vector4& _v, const Mat44& _m); // ??Dmath@@YA?AVVector4@0@ABV10@ABVMat44@0@@Z (game2.o 0x004EB6F0)
Position3 operator*(const Position3& _v, const Mat43& _m);  // ??Dmath@@YA?AVPosition3@0@ABV10@ABVMat43@0@@Z (g.o 0x4A76E0)
Position3 Mul(const Position3& _v, const DiagMat33& _m);  // ?Mul@math@@YA?AVPosition3@1@ABV21@ABVDiagMat33@1@@Z (ngl_debug.o 0x8356C0)
Position3 operator*(const Position3& _v, const DiagMat33& _m);  // ??Dmath@@YA?AVPosition3@0@ABV10@ABVDiagMat33@0@@Z (ngl_debug.o 0x8356F0)
Position3 operator/(const Position3& _v, const Mat43& _m);  // ??Kmath@@YA?AVPosition3@0@ABV10@ABVMat43@0@@Z (g.o 0x4A82C0)
Mat33 Mul(const Mat33& _a, const Mat33& _b);  // ?Mul@math@@YA?AVMat33@1@ABV21@0@Z (g.o 0x4A77C0)
Mat33 Mul(const Mat33& _a, const DiagMat33& _b);  // phys_constraint_solver_multithreaded.o 0x891B40
Mat33 operator*(const Mat33& _a, const DiagMat33& _b);  // phys_constraint_solver_multithreaded.o 0x891BE0
Mat43 Mul(const Mat43& _a, const Mat43& _b);  // ?Mul@math@@YA?AVMat43@1@ABV21@0@Z (g.o 0x4A7BC0)
Mat43 operator*(const Mat43& _a, const Mat43& _b);  // ??Dmath@@YA?AVMat43@0@ABV10@0@Z (g.o 0x4A7D90)
Mat43 operator/(const Mat43& _a, const Mat43& _b);  // ??Kmath@@YA?AVMat43@0@ABV10@0@Z (cdl_common.o 0x81D260)
Mat43 Inv(const Mat43& _m);                     // ?Inv@math@@YA?AVMat43@1@ABV21@@Z (g.o 0x4A80A0)
Vector4 Cos(const Vector4& radians, const Vector4& frequency);  // ?Cos@math@@YA?AVVector4@1@ABV21@0@Z (g.o 0x4A83C0)
bool Compare_all_lt(const Position3& _a, const Position3& _b);  // ?Compare_all_lt@math@@YA_NABVPosition3@1@0@Z (g.o 0x4A84D0)
bool Compare_all_ge(const Vector4& _a, const Vector4& _b);       // game.o 0x0065B540
bool Compare_all_gt(const Vector4& _a, const Vector4& _b);       // game.o 0x0065B590
bool Compare_any_le(const Position3& _a, const Position3& _b);  // game.o 0x0065B5E0
bool Compare_any_ge(const Position3& _a, const Position3& _b);  // game.o 0x0065B630

// ============================================================================
// Mat44 — 4x4 matrix (64 bytes = 4 * Vector4)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
// class tag per binary mangling (?PAVMat44 in render.o/cg.o)
class Mat44 {
public:
    Vector4 x;  // +0x00
    Vector4 y;  // +0x10
    Vector4 z;  // +0x20
    Vector4 w;  // +0x30

    Mat44() {}
    Mat44(const Vector4& _x, const Vector4& _y, const Vector4& _z,
          const Vector4& _w);  // ??0Mat44@math@@QAE@ABVVector4@1@000@Z
    Mat44(const Mat33& m);     // ??0Mat44@math@@QAE@ABVMat33@1@@Z

    Vector4& GetX();  // ?GetX@Mat44@math@@QAEAAVVector4@2@XZ
    Vector4& GetY();  // ?GetY@Mat44@math@@QAEAAVVector4@2@XZ
    Vector4& GetZ();  // ?GetZ@Mat44@math@@QAEAAVVector4@2@XZ
    Vector4& GetW();  // ?GetW@Mat44@math@@QAEAAVVector4@2@XZ
    void SetX(const Vector4& _x);  // ?SetX@Mat44@math@@QAEXABVVector4@2@@Z (render.o 0x6E65F0)
    void SetY(const Vector4& _y);  // ?SetY@Mat44@math@@QAEXABVVector4@2@@Z (render.o 0x6E6630)
    void SetZ(const Vector4& _z);  // ?SetZ@Mat44@math@@QAEXABVVector4@2@@Z (render.o 0x6E6670)
    void SetW(const Vector4& _w);  // ?SetW@Mat44@math@@QAEXABVVector4@2@@Z (game2.o 0x004EB470)
    Vector4& operator[](unsigned int i); // ??AMat44@math@@QAEAAVVector4@1@I@Z (game2.o 0x004EB4A0)
    const Vector4& operator[](unsigned int i) const; // ??AMat44@math@@QBEABVVector4@1@I@Z (game2.o 0x004EB4C0)
    const Mat44& operator=(const Mat44& _m);  // ??4Mat44@math@@QAEABV01@ABV01@@Z (core.o 0x4DBD50)
    const Mat44& operator*=(const Mat44& _m); // ??XMat44@math@@QAEABV01@ABV01@@Z (game2.o 0x004EBC30)
};
static_assert(sizeof(Mat44) == 0x40, "Mat44 size mismatch");

// streamer.o SSE math COMDATs (defined in game/streamer/pakmanager.cpp)
float    LengthSquared(const Position3& v);                       // ?LengthSquared@math@@YAMABVPosition3@1@@Z
Vector4  operator+(const Vector4& a, const Position3& b);         // ??Hmath@@YA?AVVector4@0@ABV10@ABVPosition3@0@@Z
Vector4  operator+(const Dir3& a, const Vector4& b);              // ??Hmath@@YA?AVVector4@0@ABVDir3@0@ABV10@@Z (render.o 0x6E6290)
Vector4  operator/(const Vector4& a, float b);                    // ??Kmath@@YA?AVVector4@0@ABV10@M@Z
Vector4  operator*(const Vector4& a, float b);                    // ??Dmath@@YA?AVVector4@0@ABV10@M@Z (render.o 0x6E62E0)
float    operator*(const Vector4& a, const Vector4& b);           // ??Dmath@@YAMABVVector4@0@0@Z (render.o 0x6E6320)
float    operator*(const Dir3& a, const Vector4& b);              // ??Dmath@@YAMABVDir3@0@ABVVector4@0@@Z (render.o 0x6E6390)
float    operator*(const Position3& a, const Vector4& b);         // ??Dmath@@YAMABVPosition3@0@ABVVector4@0@@Z (render.o 0x6E6410)
Position3 Mul(const Position3& v, const Mat33& m);                // ?Mul@math@@YA?AVPosition3@1@ABV21@ABVMat33@1@@Z (render.o 0x6E6700)
Position3 operator*(const Position3& v, const Mat33& m);          // ??Dmath@@YA?AVPosition3@0@ABV10@ABVMat33@0@@Z (render.o 0x6E6770)
Vector4  Mul(const Vector4& v, const Mat43& m);                   // ?Mul@math@@YA?AVVector4@1@ABV21@ABVMat43@1@@Z (render.o 0x6E6860)
Vector4  operator*(const Vector4& v, const Mat43& m);             // ??Dmath@@YA?AVVector4@0@ABV10@ABVMat43@0@@Z (render.o 0x6E6900)
Vector4  Mul(const Position3& v, const Mat44& m);                 // ?Mul@math@@YA?AVVector4@1@ABVPosition3@1@ABVMat44@1@@Z
Vector4  operator*(const Position3& v, const Mat44& m);           // ??Dmath@@YA?AVVector4@0@ABVPosition3@0@ABVMat44@0@@Z
Mat44    Mul(const Mat44& a, const Mat33& b);                     // ?Mul@math@@YA?AVMat44@1@ABV21@ABVMat33@1@@Z
Mat44    Mul(const Mat44& a, const Mat44& b);                     // ?Mul@math@@YA?AVMat44@1@ABV21@0@Z (game2.o 0x004EB7D0)
Mat44    operator*(const Mat44& a, const Mat44& b);               // ??Dmath@@YA?AVMat44@0@ABV10@0@Z (game2.o 0x004EBA00)
Mat43    Mul(const Mat43& a, const TranMat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABV21@ABUTranMat43@1@@Z (render.o 0x6E69F0)
Mat43    Mul(const DiagMat33& a, const Mat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABVDiagMat33@1@ABV21@@Z (render.o 0x6E6AE0)
Mat43    Mul(const TranMat43& a, const Mat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABUTranMat43@1@ABV21@@Z (render.o 0x6E6C90)
Mat43    operator*(const DiagMat33& a, const Mat43& b);           // ??Dmath@@YA?AVMat43@0@ABVDiagMat33@0@ABV10@@Z (render.o 0x6E6E10)
Mat43    operator*(const TranMat43& a, const Mat43& b);           // ??Dmath@@YA?AVMat43@0@ABUTranMat43@0@ABV10@@Z (render.o 0x6E6F20)
float    ASinUpper(float y);                                      // ?ASinUpper@math@@YAMM@Z (core.o 0x4DBDF0)
float    ACosUpper(float x);                                      // ?ACosUpper@math@@YAMM@Z (core.o 0x4DBF00)
Mat33    AxisSinCosToRotMat(const Dir3& v, float s, float c);     // ?AxisSinCosToRotMat@math@@YA?AVMat33@1@ABVDir3@1@MM@Z
Mat33    AxisAngleToRotMat(const Dir3& axis, float angle);        // ?AxisAngleToRotMat@math@@YA?AVMat33@1@ABVDir3@1@M@Z
Vector4  Vector4_One();                                           // ?Vector4_One@math@@YA?AVVector4@1@XZ (sv.o 0x51E110)

// ============================================================================
// TranMat43 — translation-only matrix (16 bytes, single Position3)
// Size: 0x10 (16 bytes) — verified against IDA
// ============================================================================
class TranMat43 {
public:
    __m128 v;
    TranMat43() {}
    TranMat43(float _x, float _y, float _z);  // ??0TranMat43@math@@QAE@MMM@Z (render.o 0x6E6580)
    TranMat43(const Position3& _p);            // ??0TranMat43@math@@QAE@ABVPosition3@1@@Z (ngl_debug.o 0x835620)
    Position3 GetW() const;                    // ?GetW@TranMat43@math@@QBE?AVPosition3@2@XZ (ngl_debug.o 0x835640)
};
static_assert(sizeof(TranMat43) == 0x10, "TranMat43 size mismatch");

// Quaternion - 4-float quaternion (16 bytes; `class` tag to match the
// binary's V-tag mangling in free-function signatures)
class Quaternion {
public:
    union {
        __m128 v;  // +0x00 (IDA local type: math::Quaternion::v)
        struct {
            float x;  // +0x00
            float y;  // +0x04
            float z;  // +0x08
            float w;  // +0x0C
        };
    };

    Quaternion();                                // ??0Quaternion@math@@QAE@XZ (g.o 0x4A8530)
    Quaternion(float _x, float _y, float _z, float _w);  // ??0Quaternion@math@@QAE@MMMM@Z (g.o 0x4A8540)

    // ??AQuaternion@math@@QAEAAMI@Z (anim.o 0x53AD30)
    float& operator[](unsigned int i);
};
static_assert(sizeof(Quaternion) == 0x10, "Quaternion size mismatch");

// anim.o math free functions (defined out-of-line in nal.cpp so the exact
// mangled symbols are emitted there)
Quaternion Unitize(const Quaternion& q);  // ?Unitize@math@@YA?AVQuaternion@1@ABV21@@Z (0x53AF10)
Vector4    UnitDirW();                    // ?UnitDirW@math@@YA?AVVector4@1@XZ (0x539D80)
Dir3       UnitDirX();                    // ?UnitDirX@math@@YA?AVDir3@1@XZ (core.o 0x4DBA00)
Dir3       UnitDirY();                    // ?UnitDirY@math@@YA?AVDir3@1@XZ (core.o 0x4DBA40)
Dir3       UnitDirZ();                    // ?UnitDirZ@math@@YA?AVDir3@1@XZ (core.o 0x4DBAA0)
DiagMat33  IdentityMat33();               // ?IdentityMat33@math@@YA?AVDiagMat33@1@XZ (0x55F240)
Dir3       Mul(const Dir3& v, const Mat33& m);  // ?Mul@math@@YA?AVDir3@1@ABV21@ABVMat33@1@@Z (0x53A2B0)
Dir3       operator*(const Dir3& v, const Mat33& m);  // ??Dmath@@YA?AVDir3@0@ABV10@ABVMat33@0@@Z (0x53A340)
Quaternion Mul(const Quaternion& a, const Quaternion& b);  // ?Mul@math@@YA?AVQuaternion@1@ABV21@0@Z (0x53AD40)
Quaternion operator*(const Quaternion& a, const Quaternion& b);  // ??Dmath@@YA?AVQuaternion@0@ABV10@0@Z (0x53ADF0)
float      LengthSquared(const Quaternion& q);  // ?LengthSquared@math@@YAMABVQuaternion@1@@Z (0x53AEA0)
float      Dot(const Quaternion& a, const Quaternion& b);  // ?Dot@math@@YAMABVQuaternion@1@0@Z (game2.o 0x00516C00)
Quaternion Slerp(float t, const Quaternion& a, const Quaternion& b);  // ?Slerp@math@@YA?AVQuaternion@1@MABV21@0@Z (game2.o 0x00516CB0)

// g.o quaternion free functions (0x4A85B0-0x4A8620)
Quaternion operator*(const Quaternion& _a, float _b);  // ??Dmath@@YA?AVQuaternion@0@ABV10@M@Z (g.o 0x4A85B0)
Quaternion DeclareUnit(const Quaternion& _q);          // ?DeclareUnit@math@@YA?AVQuaternion@1@ABV21@@Z (g.o 0x4A85F0)
Quaternion GetQuaternion(const Mat33& rot);            // ?GetQuaternion@math@@YA?AVQuaternion@1@ABVMat33@1@@Z (g.o 0x4A8620)

// ea: 0x00516B70
inline math::Vector4::Vector4(const math::Quaternion& _v)
{
    v = _v.v;
}

// ea: 0x00516C00
inline float math::Dot(const math::Quaternion& _a,
                       const math::Quaternion& _b)
{
    __m128 product = _mm_mul_ps(_a.v, _b.v);
    return product.m128_f32[0]
           + (_mm_shuffle_ps(product, product, 85).m128_f32[0]
              + (_mm_shuffle_ps(product, product, 170).m128_f32[0]
                 + _mm_shuffle_ps(product, product, 255).m128_f32[0]));
}

// ea: 0x00516CB0
inline math::Quaternion math::Slerp(float t,
                                    const math::Quaternion& _a,
                                    const math::Quaternion& _b)
{
    __m128 product = _mm_mul_ps(_a.v, _b.v);
    float dot = product.m128_f32[0]
                + (_mm_shuffle_ps(product, product, 85).m128_f32[0]
                   + (_mm_shuffle_ps(product, product, 170).m128_f32[0]
                      + _mm_shuffle_ps(product, product, 255).m128_f32[0]));
    float adjustedDot = dot;
    __m128 weights;
    if (dot >= 0.0f)
    {
        weights = _mm_setr_ps(1.0f - t, t, 1.0f, 0.0f);
    }
    else
    {
        adjustedDot = 0.0f - dot;
        weights = _mm_setr_ps(1.0f - t, 0.0f - t, 1.0f, 0.0f);
    }

    if (adjustedDot < 0.99999899f)
    {
        float theta;
        if (adjustedDot >= 0.5f)
        {
            float s = sqrtf((1.0f - adjustedDot) * 0.5f);
            float s2 = s * s;
            float s3 = s2 * s;
            float s5 = s3 * s2;
            theta = ((((s5 * s2) * 0.1079625f)
                      + (s5 * 0.15000001f))
                     + (s3 * 0.33333331f))
                    + (s * 2.0f);
        }
        else
        {
            float d2 = adjustedDot * adjustedDot;
            float d3 = d2 * adjustedDot;
            float d4 = d2 * d2;
            theta = ((((d4 * d2) * -0.053981241f)
                      - (d4 * 0.075000003f))
                     - (d3 * 0.1666667f))
                    - adjustedDot + 1.570796f;
        }

        __m128 angle = _mm_mul_ps(weights, _mm_set1_ps(theta));
        __m128 angle2 = _mm_mul_ps(angle, angle);
        __m128 angle3 = _mm_mul_ps(angle2, angle);
        __m128 angle5 = _mm_mul_ps(angle2, angle3);
        const __m128 sinCoefs =
            _mm_setr_ps(-0.16666667f, 0.0083333338f, -0.00019841269f, 0.0f);
        __m128 sine = _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(
                    angle,
                    _mm_mul_ps(_mm_mul_ps(angle2, angle5),
                               _mm_shuffle_ps(sinCoefs, sinCoefs, 170))),
                _mm_mul_ps(angle5, _mm_shuffle_ps(sinCoefs, sinCoefs, 85))),
            _mm_mul_ps(angle3, _mm_shuffle_ps(sinCoefs, sinCoefs, 0)));
        float denominator = _mm_shuffle_ps(sine, sine, 170).m128_f32[0];
        weights = _mm_div_ps(sine, _mm_set1_ps(denominator));
    }

    math::Quaternion result;
    result.v = _mm_add_ps(
        _mm_mul_ps(_a.v, _mm_shuffle_ps(weights, weights, 0)),
        _mm_mul_ps(_b.v, _mm_shuffle_ps(weights, weights, 85)));
    return result;
}

// ea: 0x00516FD0
inline math::Mat33::Mat33(const math::Quaternion& _q)
{
    __m128 doubled = _mm_add_ps(_q.v, _q.v);
    __m128 product = _mm_mul_ps(
        _mm_shuffle_ps(doubled, doubled, 255), _q.v);
    const __m128 signMask =
        _mm_setr_ps(-0.0f, -0.0f, -0.0f, -0.0f);
    __m128 negated = _mm_xor_ps(signMask, product);
    float diagonal = _mm_shuffle_ps(product, product, 255).m128_f32[0]
                     - 1.0f;
    __m128 term;

    term = _mm_setr_ps(
        diagonal,
        _mm_shuffle_ps(negated, negated, 170).m128_f32[0],
        _mm_shuffle_ps(product, product, 85).m128_f32[0],
        0.0f);
    x.v = _mm_add_ps(
        _mm_mul_ps(doubled, _mm_shuffle_ps(_q.v, _q.v, 0)), term);

    term = _mm_setr_ps(
        _mm_shuffle_ps(product, product, 170).m128_f32[0],
        diagonal,
        negated.m128_f32[0],
        0.0f);
    y.v = _mm_add_ps(
        _mm_mul_ps(doubled, _mm_shuffle_ps(_q.v, _q.v, 85)), term);

    term = _mm_setr_ps(
        _mm_shuffle_ps(negated, negated, 85).m128_f32[0],
        product.m128_f32[0],
        diagonal,
        0.0f);
    z.v = _mm_add_ps(
        _mm_mul_ps(doubled, _mm_shuffle_ps(_q.v, _q.v, 170)), term);
}


// ============================================================================
// com_math.h helpers (inline COMDATs; g.o / scr.o / streamer.o)
//   SinCos<3,0,3,0>          - ea: 0x4AE920
//   FastSinCos               - ea: 0x4B01F0
//   AnglesToForward(float*)  - ea: 0x4B02A0
//   AnglesToForward(Pos3)    - ea: 0x4B03A0
//   AnglesToUp               - ea: 0x4B0440
//   AnglesToRight            - ea: 0x5EE650
// Function-only AeAssert contract (no enum here: other headers declare it).
// ============================================================================

// Per-lane sin/cos of radians. Lane i computes sin when arg i != 0 (phase
// +3pi/2) and cos when arg i == 0. Only <3,0,3,0> exists in the binary.
template <int A, int B, int C, int D>
inline Vector4 SinCos(const Vector4& radians)
{
    const __m128 sign_mask = _mm_set1_ps(-0.0f);
    const __m128 shift = _mm_setr_ps(A ? 4.7123880f : 0.0f,
                                     B ? 4.7123880f : 0.0f,
                                     C ? 4.7123880f : 0.0f,
                                     D ? 4.7123880f : 0.0f);
    // Folded range reduction: t = -|x + shift| / (2pi), y = |frac(t)-0.5|-0.25.
    __m128 t = _mm_mul_ps(
        _mm_xor_ps(sign_mask,
                   _mm_andnot_ps(sign_mask, _mm_add_ps(radians.v, shift))),
        _mm_set1_ps(0.15915494f));
    __m128 y = _mm_sub_ps(
        _mm_andnot_ps(
            sign_mask,
            _mm_sub_ps(_mm_sub_ps(_mm_add_ps(_mm_sub_ps(t, _mm_set1_ps(12582912.0f)),
                                             _mm_set1_ps(12582912.0f)),
                                  t),
                       _mm_set1_ps(0.5f))),
        _mm_set1_ps(0.25f));
    __m128 y2 = _mm_mul_ps(y, y);
    __m128 y3 = _mm_mul_ps(y, y2);
    __m128 y4 = _mm_mul_ps(y2, y2);
    __m128 y5 = _mm_mul_ps(y, y4);
    __m128 y7 = _mm_mul_ps(y3, y4);
    __m128 y9 = _mm_mul_ps(y5, y4);
    Vector4 result;
    result.v = _mm_add_ps(
        _mm_add_ps(
            _mm_add_ps(
                _mm_add_ps(_mm_mul_ps(y9, _mm_set1_ps(39.710659f)),
                           _mm_mul_ps(y7, _mm_set1_ps(-76.574959f))),
                _mm_mul_ps(y5, _mm_set1_ps(81.602226f))),
            _mm_mul_ps(y3, _mm_set1_ps(-41.341675f))),
        _mm_mul_ps(y, _mm_set1_ps(6.2831850f)));
    return result;
}

} // namespace math

namespace AeAssert {
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

// ea: 0x4B01F0
inline void FastSinCos(float radians, float* psin, float* pcos)
{
    math::Vector4 in;
    in.v = _mm_setr_ps(radians, radians, radians, 0.0f);
    math::Vector4 r = math::SinCos<3, 0, 3, 0>(in);
    *psin = r.v.m128_f32[0];
    *pcos = r.v.m128_f32[1];
}

// ea: 0x4B02A0
inline void AnglesToForward(const float* const angles,
                            float* const forward)
{
    if (forward == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sy, cy, sp, cp;
    FastSinCos(angles[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles[0] * 0.017453292f, &sp, &cp);
    forward[0] = cp * cy;
    forward[1] = cp * sy;
    forward[2] = -sp;
}

// ea: 0x4B03A0
inline void AnglesToForward(const math::Position3& angles,
                            math::Dir3& forward)
{
    float sy, cy, sp, cp;
    FastSinCos(angles.v.m128_f32[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles.v.m128_f32[0] * 0.017453292f, &sp, &cp);
    forward.v.m128_f32[0] = cp * cy;
    forward.v.m128_f32[1] = cp * sy;
    forward.v.m128_f32[2] = -sp;
}

// ea: 0x4B0440
inline void AnglesToUp(const float* const angles, float* const up)
{
    if (up == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sp, cp, sr, cr;
    FastSinCos(angles[0] * 0.017453292f, &sp, &cp);
    FastSinCos(angles[2] * 0.017453292f, &sr, &cr);
    up[0] = cr * sp;
    up[1] = -sr;
    up[2] = cr * cp;
}

// ea: 0x5EE650
inline void AnglesToRight(const float* const angles,
                          float* const right)
{
    if (right == nullptr)
    {
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float sy, cy, sr, cr;
    FastSinCos(angles[1] * 0.017453292f, &sy, &cy);
    FastSinCos(angles[2] * 0.017453292f, &sr, &cr);
    right[0] = cr * sy;
    right[1] = -cr * cy;
    right[2] = -sr;
}

// ea: 0x51A630 (game2.o COMDAT) - c:\cod\code\game\com_math.h:682
// Binary assert: "beg <= end" with message "Beg must be less than end."
template <typename T>
inline T ClampRange(const T& in, const T& beg, const T& end)
{
    if (end < beg)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Beg must be less than end."))
            __debugbreak();
    }
    if (beg > in)
        return beg;
    if (in <= end)
        return in;
    return end;
}

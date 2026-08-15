// ============================================================================
// COD3 Math Types — geometry primitives used throughout the engine
// Reconstructed from IDA local types (PDB symbol data).
// All sizes verified against IDA.
// ============================================================================

#pragma once

#include "core/sse_portable.h"
#include <math.h>

namespace math {

// Forward declarations
class Dir3;
class Position3;
class Vector4;
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

    Dir3();                       // ??0Dir3@math@@QAE@XZ (g.o 0x4A55C0)
    Dir3(__m128 _v) : v(_v) {}
    Dir3(float _x, float _y, float _z);  // ??0Dir3@math@@QAE@MMM@Z (g.o 0x4A55D0)
    Dir3(float _x);               // ??0Dir3@math@@QAE@M@Z (g.o 0x4A5640)
    Dir3(const Dir3& other) : v(other.v) {}  // implicit copy (aggregate support)
    Dir3(const Position3& _v);    // ??0Dir3@math@@QAE@ABVPosition3@1@@Z
    Dir3(const Vector4& _v);      // ??0Dir3@math@@QAE@ABVVector4@1@@Z
    const Dir3& operator=(const Position3& _v);  // ??4Dir3@math@@QAEABV01@ABVPosition3@1@@Z
    float GetX() const;           // ?GetX@Dir3@math@@QBEMXZ
    float GetY() const;           // ?GetY@Dir3@math@@QBEMXZ
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

    // cg.o inline COMDAT (??ZDir3@math@@QAEABV01@ABVPosition3@1@@Z)
    const math::Dir3& operator-=(const math::Position3& v);

    // Constant layout (for compile-time initialization)
    struct Constant {
        float x, y, z, w;
    };

    // Packed layout (3 floats, 12 bytes — for network/disk)
    struct Packed {
        float x, y, z;
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
    Position3(const Dir3& _v);    // ??0Position3@math@@QAE@ABVDir3@1@@Z
    const Position3& operator*=(const Mat43& _m);  // ??XPosition3@math@@QAEABV01@ABVMat43@1@@Z (render.o 0x6E67E0)

    float GetX() const;           // ?GetX@Position3@math@@QBEMXZ (g.o 0x4A58B0)
    float GetY() const;           // ?GetY@Position3@math@@QBEMXZ (g.o 0x4A58D0)
    float GetZ() const;           // ?GetZ@Position3@math@@QBEMXZ (g.o 0x4A5930)
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

// g.o free-function vector math (0x4A5FB0-0x4A65E0)
Dir3 operator-(const Dir3& _v);  // ??Gmath@@YA?AVDir3@0@ABV10@@Z
Position3 operator-(const Position3& _v);
Vector4 operator-(const Vector4& _v);
float Length(const Dir3& _v);  // ?Length@math@@YAMABVDir3@1@@Z
float AbsSquared(const Dir3& _v);
float AbsSquared(const Position3& _v);
float Abs(const Dir3& _v);
float Abs(const Position3& _v);
Vector4 AbsValue(const Vector4& _v);
Vector4 Ceil(const Vector4& _v);
bool operator==(const Position3& _a, const Position3& _b);
bool operator!=(const Position3& _a, const Position3& _b);
Dir3 operator+(const Dir3& _a, const Position3& _b);
Position3 operator+(const Position3& _a, const Dir3& _b);
Position3 operator+(const Position3& _a, const Position3& _b);  // ??Hmath@@YA?AVPosition3@0@ABV10@0@Z (g.o 0x4A6630)
Vector4 operator+(const Vector4& _a, const Vector4& _b);        // ??Hmath@@YA?AVVector4@0@ABV10@0@Z (g.o 0x4A6670)
Dir3 operator-(const Dir3& _a, const Position3& _b);            // ??Gmath@@YA?AVDir3@0@ABV10@ABVPosition3@0@@Z (g.o 0x4A66B0)
Position3 operator-(const Position3& _a, const Dir3& _b);       // ??Gmath@@YA?AVPosition3@0@ABV10@ABVDir3@0@@Z (g.o 0x4A66F0)
Position3 operator-(const Position3& _a, const Position3& _b);  // ??Gmath@@YA?AVPosition3@0@ABV10@0@Z (g.o 0x4A6730)
Dir3 operator/(const Dir3& _a, float _b);                       // ??Kmath@@YA?AVDir3@0@ABV10@M@Z (g.o 0x4A6770)

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
    Vector4(const Dir3& _v);  // ??0Vector4@math@@QAE@ABVDir3@1@@Z
    Vector4(const Dir3& _v, float _w);  // ??0Vector4@math@@QAE@ABVDir3@1@M@Z (render.o 0x6E6110)
    Vector4(const Vector4::Packed& _p); // ??0Vector4@math@@QAE@ABUPacked@01@@Z (render.o 0x6E6160)
    const Vector4& operator=(const Vector4::Packed& _p);  // ??4Vector4@math@@QAEABV01@ABUPacked@01@@Z (render.o 0x6E6220)
    const Vector4& operator=(const Position3& _v);  // ??4Vector4@math@@QAEABV01@ABVPosition3@1@@Z (render.o 0x6E61D0)

    float operator[](int i) const { return v.m128_f32[i]; }
    float& operator[](int i) { return v.m128_f32[i]; }

    const Vector4& operator*=(const Mat44& m);  // ??XVector4@math@@QAEABV01@ABVMat44@1@@Z (streamer.o)
};
static_assert(sizeof(Vector4) == 0x10, "Vector4 size mismatch");

// ============================================================================
// Mat43 — 4x3 affine transform matrix (64 bytes)
//   3 rotation axes (Dir3 each, 16 bytes) + 1 translation (Position3, 16 bytes)
// Size: 0x40 (64 bytes) — verified against IDA
// ============================================================================
class Mat43 {
public:
    Mat43() {}

    // ??0Mat43@math@@QAE@ABVDiagMat33@1@@Z (anim.o 0x539FC0)
    Mat43(const DiagMat33& m);

    // ??4Mat43@math@@QAEABV01@ABVDiagMat33@1@@Z (anim.o; defined in nal.cpp)
    const Mat43& operator=(const DiagMat33& m);
    const Mat43& operator*=(const TranMat43& m);  // ??XMat43@math@@QAEABV01@ABVTranMat43@1@@Z (render.o 0x6E7040)
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
};
static_assert(sizeof(Mat33) == 0x30, "Mat33 size mismatch");

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
Mat43    Mul(const Mat43& a, const TranMat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABV21@ABUTranMat43@1@@Z (render.o 0x6E69F0)
Mat43    Mul(const DiagMat33& a, const Mat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABVDiagMat33@1@ABV21@@Z (render.o 0x6E6AE0)
Mat43    Mul(const TranMat43& a, const Mat43& b);                 // ?Mul@math@@YA?AVMat43@1@ABUTranMat43@1@ABV21@@Z (render.o 0x6E6C90)
Mat43    operator*(const DiagMat33& a, const Mat43& b);           // ??Dmath@@YA?AVMat43@0@ABVDiagMat33@0@ABV10@@Z (render.o 0x6E6E10)
Mat43    operator*(const TranMat43& a, const Mat43& b);           // ??Dmath@@YA?AVMat43@0@ABUTranMat43@0@ABV10@@Z (render.o 0x6E6F20)
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
};
static_assert(sizeof(TranMat43) == 0x10, "TranMat43 size mismatch");

// Quaternion - 4-float quaternion (16 bytes; `class` tag to match the
// binary's V-tag mangling in free-function signatures)
class Quaternion {
public:
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
    float w;  // +0x0C

    // ??AQuaternion@math@@QAEAAMI@Z (anim.o 0x53AD30)
    float& operator[](unsigned int i);
};
static_assert(sizeof(Quaternion) == 0x10, "Quaternion size mismatch");

// anim.o math free functions (defined out-of-line in nal.cpp so the exact
// mangled symbols are emitted there)
Quaternion Unitize(const Quaternion& q);  // ?Unitize@math@@YA?AVQuaternion@1@ABV21@@Z (0x53AF10)
Vector4    UnitDirW();                    // ?UnitDirW@math@@YA?AVVector4@1@XZ (0x539D80)
DiagMat33  IdentityMat33();               // ?IdentityMat33@math@@YA?AVDiagMat33@1@XZ (0x55F240)
Dir3       Mul(const Dir3& v, const Mat33& m);  // ?Mul@math@@YA?AVDir3@1@ABV21@ABVMat33@1@@Z (0x53A2B0)
Dir3       operator*(const Dir3& v, const Mat33& m);  // ??Dmath@@YA?AVDir3@0@ABV10@ABVMat33@0@@Z (0x53A340)
Quaternion Mul(const Quaternion& a, const Quaternion& b);  // ?Mul@math@@YA?AVQuaternion@1@ABV21@0@Z (0x53AD40)
Quaternion operator*(const Quaternion& a, const Quaternion& b);  // ??Dmath@@YA?AVQuaternion@0@ABV10@0@Z (0x53ADF0)
float      LengthSquared(const Quaternion& q);  // ?LengthSquared@math@@YAMABVQuaternion@1@@Z (0x53AEA0)


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

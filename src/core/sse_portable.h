// ============================================================================
// Portable SSE emulation — provides __m128 on non-x86 platforms
// On x86/x64: uses real SSE intrinsics. On ARM: uses plain float[4].
// ============================================================================

#pragma once

#ifdef _MSC_VER
    #include <xmmintrin.h>
    #include <stdint.h>
#elif defined(__x86_64__)
    #include <xmmintrin.h>
#elif defined(__i386__)
    #include <xmmintrin.h>
#else
    // ARM / other: define __m128 as a plain struct
    #include <math.h>
    #include <string.h>
    #include <stdint.h>

    struct __m128 {
        float m128_f32[4];

        __m128() { m128_f32[0] = m128_f32[1] = m128_f32[2] = m128_f32[3] = 0.0f; }
        __m128(float x, float y, float z, float w) {
            m128_f32[0] = x; m128_f32[1] = y; m128_f32[2] = z; m128_f32[3] = w;
        }
    };

    // Minimal SSE intrinsic emulations used by math_types.h storage only
    // (Actual SSE math functions should not be called on ARM — they're stubbed)

    #define _mm_setzero_ps() __m128()
    #define _mm_set_ps(w,z,y,x) __m128(x,y,z,w)
    #define _mm_set_ss(x) __m128(x,0,0,0)

    inline __m128 _mm_add_ps(__m128 a, __m128 b) {
        return __m128(a.m128_f32[0]+b.m128_f32[0], a.m128_f32[1]+b.m128_f32[1],
                      a.m128_f32[2]+b.m128_f32[2], a.m128_f32[3]+b.m128_f32[3]);
    }
    inline __m128 _mm_sub_ps(__m128 a, __m128 b) {
        return __m128(a.m128_f32[0]-b.m128_f32[0], a.m128_f32[1]-b.m128_f32[1],
                      a.m128_f32[2]-b.m128_f32[2], a.m128_f32[3]-b.m128_f32[3]);
    }
    inline __m128 _mm_mul_ps(__m128 a, __m128 b) {
        return __m128(a.m128_f32[0]*b.m128_f32[0], a.m128_f32[1]*b.m128_f32[1],
                      a.m128_f32[2]*b.m128_f32[2], a.m128_f32[3]*b.m128_f32[3]);
    }
    inline __m128 _mm_min_ps(__m128 a, __m128 b) {
        return __m128(fminf(a.m128_f32[0],b.m128_f32[0]), fminf(a.m128_f32[1],b.m128_f32[1]),
                      fminf(a.m128_f32[2],b.m128_f32[2]), fminf(a.m128_f32[3],b.m128_f32[3]));
    }
    inline __m128 _mm_max_ps(__m128 a, __m128 b) {
        return __m128(fmaxf(a.m128_f32[0],b.m128_f32[0]), fmaxf(a.m128_f32[1],b.m128_f32[1]),
                      fmaxf(a.m128_f32[2],b.m128_f32[2]), fmaxf(a.m128_f32[3],b.m128_f32[3]));
    }
    inline __m128 _mm_xor_ps(__m128 a, __m128 b) {
        uint32_t* ua = (uint32_t*)&a;
        uint32_t* ub = (uint32_t*)&b;
        __m128 r;
        uint32_t* ur = (uint32_t*)&r;
        ur[0] = ua[0] ^ ub[0]; ur[1] = ua[1] ^ ub[1];
        ur[2] = ua[2] ^ ub[2]; ur[3] = ua[3] ^ ub[3];
        return r;
    }
    // shuffle emulation (basic forms only)
    #define _MM_SHUFFLE(z,y,x,w) (((z)<<6)|((y)<<4)|((x)<<2)|(w))
    inline __m128 _mm_shuffle_ps(__m128 a, __m128 b, int imm) {
        int sel[4] = { imm&3, (imm>>2)&3, (imm>>4)&3, (imm>>6)&3 };
        float va[4] = { a.m128_f32[0],a.m128_f32[1],a.m128_f32[2],a.m128_f32[3] };
        float vb[4] = { b.m128_f32[0],b.m128_f32[1],b.m128_f32[2],b.m128_f32[3] };
        return __m128(
            (sel[0]<4) ? va[sel[0]] : vb[sel[0]-4],
            (sel[1]<4) ? va[sel[1]] : vb[sel[1]-4],
            (sel[2]<4) ? va[sel[2]] : vb[sel[2]-4],
            (sel[3]<4) ? va[sel[3]] : vb[sel[3]-4]);
    }
    inline __m128 _mm_cmplt_ps(__m128 a, __m128 b) {
        // returns all-1s bits where a < b
        uint32_t ra[4];
        for (int i = 0; i < 4; i++)
            ra[i] = (a.m128_f32[i] < b.m128_f32[i]) ? 0xFFFFFFFFu : 0u;
        __m128 r;
        memcpy(&r, ra, 16);
        return r;
    }
    inline int _mm_movemask_ps(__m128 a) {
        uint32_t* u = (uint32_t*)&a;
        return ((u[0]>>31)&1) | ((u[1]>>31)&2) | ((u[2]>>31)&4) | ((u[3]>>31)&8);
    }
#endif

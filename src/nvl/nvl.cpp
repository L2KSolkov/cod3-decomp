// NVL - NGL Video Library (AFMV movie playback).

#include <cstdint>
#include <cstring>
#include <mmintrin.h>
#include "../ngl/nglTexture.h"

struct IDirectSoundBuffer;

enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
enum nflRequestID : unsigned { NFL_REQUEST_ID_INVALID = 0xFFFFFFFFu };
enum nflStreamID : unsigned {
    NFL_STREAM_ID_INVALID = 0xFFFFFFFFu,
    NFL_STREAM_ID_DEFAULT = 0,
};
enum nflPriority : unsigned {
    NFL_PRIORITY_INVALID = 0xFFFFFFFFu,
    NFL_PRIORITY_LOWEST = 0,
    NFL_PRIORITY_LOW = 1,
    NFL_PRIORITY_NORMAL = 2,
    NFL_PRIORITY_HIGH = 3,
    NFL_PRIORITY_HIGHEST = 4,
};
enum nflRequestType : unsigned {
    NFL_REQUEST_TYPE_INVALID = 0xFFFFFFFFu,
    NFL_REQUEST_TYPE_READ = 0,
    NFL_REQUEST_TYPE_WRITE = 1,
};
enum nflRequestState : unsigned {
    NFL_REQUEST_STATE_INVALID = 0xFFFFFFFFu,
    NFL_REQUEST_STATE_COMPLETED = 0,
    NFL_REQUEST_STATE_CANCELED = 1,
    NFL_REQUEST_STATE_TIMEOUT = 2,
    NFL_REQUEST_STATE_ERROR = 3,
    NFL_REQUEST_STATE_ACTIVE = 4,
};

enum nvlFrameState : int {
    NVL_FRAME_ERROR = -1,
    NVL_FRAME_READY = 0,
    NVL_FRAME_STREAMING = 1,
    NVL_FRAME_DECODING = 2,
    NVL_FRAME_LAST = 3,
    NVL_FRAME_NONE = 4,
};
enum nvlResult : int {
    NVL_RESULT_ERROR = -1,
    NVL_RESULT_OK = 0,
};
enum nvlMovieState : int {
    NVL_STATE_ERROR = -1,
    NVL_STATE_PAUSE = 0,
    NVL_STATE_PLAY = 1,
    NVL_STATE_LOOP = 3,
    NVL_STATE_RESET_PAUSE = 0x10,
    NVL_STATE_RESET_PLAY = 0x11,
    NVL_STATE_RESET_LOOP = 0x13,
};
enum nvlPhase : int {
    NVL_PHASE_FIRST = 0,
    NVL_PHASE_EMPTY = 1,
    NVL_PHASE_DECODED = 2,
    NVL_PHASE_RENDERING = 3,
};

struct nflRequestParams {
    nflFileID fileID;
    nflStreamID streamID;
    void (*callback)(nflRequestState, nflRequestID, void*);
    nflRequestType type;
    nflPriority priority;
    unsigned fileOffset;
    void* buffer;
    unsigned dataSize;
    unsigned timeout;
    void* userData;
};
static_assert(sizeof(nflRequestParams) == 40, "NVL NFL request layout mismatch");

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void tlMemFree(void* ptr);
extern bool _tlAssert(const char* file, int line, const char* expr, const char* message);
extern void tlFatal(const char* format, ...);
extern void tlPrintf(const char* format, ...);
extern nflRequestID nflAddRequest(const nflRequestParams* params);
extern unsigned nflReadFile(nflFileID fileID, unsigned fileOffset, void* buffer, unsigned dataSize);
extern nglTexture* nglCreateTexture(unsigned flags, unsigned format, int width, int height,
                                    int depth, int levels);
extern void nglDestroyTexture(nglTexture* texture);
extern void nglSetEndOfVBlankCallback(void (*fn)(void*), void* data);
extern void MacroBlockIdctCopy(short* mb, unsigned char* dest, int stride);
extern void MacroBlockIdctAdd(int last, short* mb, unsigned char* dest, int stride);

struct afmv_motion_t {
    unsigned char* ref[2][3];
    unsigned char** ref2[2];
    int pmv[2][2];
    int f_code[2];
};
static_assert(sizeof(afmv_motion_t) == 56, "AFMV motion layout mismatch");

struct afmv_mbtab { unsigned char modes; unsigned char len; };
struct afmv_dmvt { signed char dmv; unsigned char len; };
static const afmv_mbtab afmv_mb_i[2] = {{17, 2}, {1, 1}};
static const afmv_mbtab afmv_mb_p[32] = {
    {17,6},{18,5},{26,5},{1,5},{8,3},{8,3},{8,3},{8,3},
    {2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
    {10,1},{10,1},{10,1},{10,1},{10,1},{10,1},{10,1},{10,1},
    {10,1},{10,1},{10,1},{10,1},{10,1},{10,1},{10,1},{10,1}
};
static const afmv_mbtab afmv_mb_b[64] = {
    {0,0},{17,6},{22,6},{26,6},{30,5},{30,5},{1,5},{1,5},
    {8,4},{8,4},{8,4},{8,4},{10,4},{10,4},{10,4},{10,4},
    {4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},{4,3},
    {6,3},{6,3},{6,3},{6,3},{6,3},{6,3},{6,3},{6,3},
    {12,2},{12,2},{12,2},{12,2},{12,2},{12,2},{12,2},{12,2},
    {12,2},{12,2},{12,2},{12,2},{12,2},{12,2},{12,2},{12,2},
    {14,2},{14,2},{14,2},{14,2},{14,2},{14,2},{14,2},{14,2},
    {14,2},{14,2},{14,2},{14,2},{14,2},{14,2},{14,2},{14,2}
};
static const afmv_dmvt afmv_dmv_2[4] = {{0,1},{0,1},{1,2},{-1,2}};
static const int afmv_alternate_scale[32] = {
    0,1,2,3,4,5,6,7,8,10,12,14,16,18,20,22,
    24,28,32,36,40,44,48,52,56,64,72,80,88,96,104,112
};

// AFMV VLC data copied from the IDA data segment at 0x00D418D8-0x00D41B6F.
struct afmv_vlc2 { unsigned char value; unsigned char len; };
struct afmv_dcttab { unsigned char run; unsigned char level; unsigned char len; };

static const afmv_dcttab afmv_dct_16[32] = {
    {129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},
    {129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},{129,0,0},
    {2,18,0},{2,17,0},{2,16,0},{2,15,0},{7,3,0},{17,2,0},{16,2,0},{15,2,0},
    {14,2,0},{13,2,0},{12,2,0},{32,1,0},{31,1,0},{30,1,0},{29,1,0},{28,1,0}
};
static const afmv_dcttab afmv_dct_15[48] = {
    {1,40,15},{1,39,15},{1,38,15},{1,37,15},{1,36,15},{1,35,15},{1,34,15},{1,33,15},
    {1,32,15},{2,14,15},{2,13,15},{2,12,15},{2,11,15},{2,10,15},{2,9,15},{2,8,15},
    {1,31,14},{1,31,14},{1,30,14},{1,30,14},{1,29,14},{1,29,14},{1,28,14},{1,28,14},
    {1,27,14},{1,27,14},{1,26,14},{1,26,14},{1,25,14},{1,25,14},{1,24,14},{1,24,14},
    {1,23,14},{1,23,14},{1,22,14},{1,22,14},{1,21,14},{1,21,14},{1,20,14},{1,20,14},
    {1,19,14},{1,19,14},{1,18,14},{1,18,14},{1,17,14},{1,17,14},{1,16,14},{1,16,14}
};
static const afmv_dcttab afmv_dct_13[48] = {
    {11,2,13},{10,2,13},{6,3,13},{4,4,13},{3,5,13},{2,7,13},{2,6,13},{1,15,13},
    {1,14,13},{1,13,13},{1,12,13},{27,1,13},{26,1,13},{25,1,13},{24,1,13},{23,1,13},
    {1,11,12},{1,11,12},{9,2,12},{9,2,12},{5,3,12},{5,3,12},{1,10,12},{1,10,12},
    {3,4,12},{3,4,12},{8,2,12},{8,2,12},{22,1,12},{22,1,12},{21,1,12},{21,1,12},
    {1,9,12},{1,9,12},{20,1,12},{20,1,12},{19,1,12},{19,1,12},{2,5,12},{2,5,12},
    {4,3,12},{4,3,12},{1,8,12},{1,8,12},{7,2,12},{7,2,12},{18,1,12},{18,1,12}
};
static const afmv_dcttab afmv_dct_b14_10[8] = {
    {17,1,10},{6,2,10},{1,7,10},{3,3,10},{2,4,10},{16,1,10},{15,1,10},{5,2,10}
};
static const afmv_dcttab afmv_dct_b14_8[36] = {
    {65,0,6},{65,0,6},{65,0,6},{65,0,6},{3,2,7},{3,2,7},{10,1,7},{10,1,7},
    {1,4,7},{1,4,7},{9,1,7},{9,1,7},{8,1,6},{8,1,6},{8,1,6},{8,1,6},
    {7,1,6},{7,1,6},{7,1,6},{7,1,6},{2,2,6},{2,2,6},{2,2,6},{2,2,6},
    {6,1,6},{6,1,6},{6,1,6},{6,1,6},{14,1,8},{1,6,8},{13,1,8},{12,1,8},
    {4,2,8},{2,3,8},{1,5,8},{11,1,8}
};
static const afmv_dcttab afmv_dct_b14ac_5[27] = {
    {1,3,5},{5,1,5},{4,1,5},{1,2,4},{1,2,4},{3,1,4},{3,1,4},{2,1,3},{2,1,3},
    {2,1,3},{2,1,3},{129,0,2},{129,0,2},{129,0,2},{129,0,2},{129,0,2},{129,0,2},
    {129,0,2},{129,0,2},{1,1,2},{1,1,2},{1,1,2},{1,1,2},{1,1,2},{1,1,2},{1,1,2}
};
static const afmv_dcttab afmv_dct_b14dc_5[27] = {
    {1,3,5},{5,1,5},{4,1,5},{1,2,4},{1,2,4},{3,1,4},{3,1,4},{2,1,3},{2,1,3},
    {2,1,3},{2,1,3},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},
    {1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1},{1,1,1}
};
static const afmv_dcttab afmv_dct_b15_10[8] = {
    {6,2,9},{6,2,9},{15,1,9},{15,1,9},{3,4,10},{17,1,10},{16,1,9},{16,1,9}
};
static const afmv_dcttab afmv_dct_b15_8[36] = {
    {65,0,6},{65,0,6},{65,0,6},{65,0,6},{8,1,7},{8,1,7},{9,1,7},{9,1,7},
    {7,1,7},{7,1,7},{3,2,7},{3,2,7},{1,7,6},{1,7,6},{1,7,6},{1,7,6},
    {1,6,6},{1,6,6},{1,6,6},{1,6,6},{5,1,6},{5,1,6},{5,1,6},{5,1,6},
    {6,1,6},{6,1,6},{6,1,6},{6,1,6},{2,5,8},{12,1,8},{1,11,8},{1,10,8},
    {14,1,8},{13,1,8},{4,2,8},{2,4,8}
};

struct afmv_mc_t {
    void (*put[8])(unsigned char*, const unsigned char*, int, int);
    void (*avg[8])(unsigned char*, const unsigned char*, int, int);
};
static const __m64 afmv_last_bit = { 0x0101010101010101ULL };

static __m64 afmv_from_bits(std::uint64_t bits) {
    __m64 value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
static std::uint64_t afmv_to_bits(__m64 value) {
    std::uint64_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}
static __m64 afmv_pavgb(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a);
    const std::uint64_t bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i)
        result |= static_cast<std::uint64_t>((((av >> (i * 8)) & 0xFFu) +
                                               ((bv >> (i * 8)) & 0xFFu) + 1u) >> 1)
            << (i * 8);
    return afmv_from_bits(result);
}
static __m64 afmv_psubusb(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a);
    const std::uint64_t bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i) {
        const unsigned aByte = static_cast<unsigned>((av >> (i * 8)) & 0xFFu);
        const unsigned bByte = static_cast<unsigned>((bv >> (i * 8)) & 0xFFu);
        result |= static_cast<std::uint64_t>(aByte > bByte ? aByte - bByte : 0u) << (i * 8);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_pand(__m64 a, __m64 b) { return afmv_from_bits(afmv_to_bits(a) & afmv_to_bits(b)); }
static __m64 afmv_por(__m64 a, __m64 b) { return afmv_from_bits(afmv_to_bits(a) | afmv_to_bits(b)); }
static __m64 afmv_pxor(__m64 a, __m64 b) { return afmv_from_bits(afmv_to_bits(a) ^ afmv_to_bits(b)); }
#define _m_pavgb afmv_pavgb
#define _m_psubusb afmv_psubusb
#define _m_pand afmv_pand
#define _m_por afmv_por
#define _m_pxor afmv_pxor

static __m64 afmv_paddd(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 2; ++i)
        result |= static_cast<std::uint64_t>(static_cast<std::uint32_t>(av >> (i * 32)) +
                                             static_cast<std::uint32_t>(bv >> (i * 32))) << (i * 32);
    return afmv_from_bits(result);
}
static __m64 afmv_psubd(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 2; ++i)
        result |= static_cast<std::uint64_t>(static_cast<std::uint32_t>(av >> (i * 32)) -
                                             static_cast<std::uint32_t>(bv >> (i * 32))) << (i * 32);
    return afmv_from_bits(result);
}
static __m64 afmv_psradi(__m64 a, unsigned count) {
    const std::uint64_t av = afmv_to_bits(a);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 2; ++i) {
        const std::int32_t lane = static_cast<std::int32_t>(static_cast<std::uint32_t>(av >> (i * 32)));
        result |= static_cast<std::uint64_t>(static_cast<std::uint32_t>(lane >> count)) << (i * 32);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_psrawi(__m64 a, unsigned count) {
    const std::uint64_t av = afmv_to_bits(a);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const std::int16_t lane = static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> (i * 16)));
        result |= static_cast<std::uint64_t>(static_cast<std::uint16_t>(lane >> count)) << (i * 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_psllwi(__m64 a, unsigned count) {
    const std::uint64_t av = afmv_to_bits(a);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= ((((av >> (i * 16)) & 0xFFFFu) << count) & 0xFFFFu) << (i * 16);
    return afmv_from_bits(result);
}
static __m64 afmv_psrlwi(__m64 a, unsigned count) {
    const std::uint64_t av = afmv_to_bits(a);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= (((av >> (i * 16)) & 0xFFFFu) >> count) << (i * 16);
    return afmv_from_bits(result);
}
static __m64 afmv_pmulhw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const std::int32_t product = static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> (i * 16))) *
                                     static_cast<std::int16_t>(static_cast<std::uint16_t>(bv >> (i * 16)));
        result |= static_cast<std::uint64_t>(static_cast<std::uint16_t>(product >> 16)) << (i * 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_paddsw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const int sum = static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> (i * 16))) +
                        static_cast<std::int16_t>(static_cast<std::uint16_t>(bv >> (i * 16)));
        const int clipped = sum < -32768 ? -32768 : sum > 32767 ? 32767 : sum;
        result |= static_cast<std::uint64_t>(static_cast<std::uint16_t>(clipped)) << (i * 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_psubsw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const int difference = static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> (i * 16))) -
                               static_cast<std::int16_t>(static_cast<std::uint16_t>(bv >> (i * 16)));
        const int clipped = difference < -32768 ? -32768 : difference > 32767 ? 32767 : difference;
        result |= static_cast<std::uint64_t>(static_cast<std::uint16_t>(clipped)) << (i * 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_paddusb(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i) {
        const unsigned sum = static_cast<unsigned>((av >> (i * 8)) & 0xFFu) +
                              static_cast<unsigned>((bv >> (i * 8)) & 0xFFu);
        result |= static_cast<std::uint64_t>(sum > 255 ? 255 : sum) << (i * 8);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_punpcklbw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        result |= ((av >> (i * 8)) & 0xFFu) << (i * 16);
        result |= ((bv >> (i * 8)) & 0xFFu) << (i * 16 + 8);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_punpckhbw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        result |= ((av >> ((i + 4) * 8)) & 0xFFu) << (i * 16);
        result |= ((bv >> ((i + 4) * 8)) & 0xFFu) << (i * 16 + 8);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_punpcklwd(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 2; ++i) {
        result |= ((av >> (i * 16)) & 0xFFFFu) << (i * 32);
        result |= ((bv >> (i * 16)) & 0xFFFFu) << (i * 32 + 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_punpckldq(__m64 a, __m64 b) {
    return afmv_from_bits((afmv_to_bits(a) & 0xFFFFFFFFu) |
                          ((afmv_to_bits(b) & 0xFFFFFFFFu) << 32));
}
static __m64 afmv_pshufw(__m64 a, unsigned imm) {
    const std::uint64_t av = afmv_to_bits(a);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i)
        result |= ((av >> (((imm >> (i * 2)) & 3u) * 16)) & 0xFFFFu) << (i * 16);
    return afmv_from_bits(result);
}
static __m64 afmv_pmaddwd(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 2; ++i) {
        const unsigned word = i * 2;
        const std::int64_t sum = static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> (word * 16))) *
                                     static_cast<std::int16_t>(static_cast<std::uint16_t>(bv >> (word * 16))) +
                                 static_cast<std::int16_t>(static_cast<std::uint16_t>(av >> ((word + 1) * 16))) *
                                     static_cast<std::int16_t>(static_cast<std::uint16_t>(bv >> ((word + 1) * 16)));
        result |= static_cast<std::uint64_t>(static_cast<std::uint32_t>(sum)) << (i * 32);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_packssdw(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 4; ++i) {
        const std::int64_t raw = static_cast<std::int32_t>(static_cast<std::uint32_t>((i < 2 ? av : bv) >> ((i & 1) * 32)));
        const int clipped = raw < -32768 ? -32768 : raw > 32767 ? 32767 : static_cast<int>(raw);
        result |= static_cast<std::uint64_t>(static_cast<std::uint16_t>(clipped)) << (i * 16);
    }
    return afmv_from_bits(result);
}
static __m64 afmv_packuswb(__m64 a, __m64 b) {
    const std::uint64_t av = afmv_to_bits(a), bv = afmv_to_bits(b);
    std::uint64_t result = 0;
    for (unsigned i = 0; i < 8; ++i) {
        const std::int32_t raw = static_cast<std::int16_t>(static_cast<std::uint16_t>((i < 4 ? av : bv) >> ((i & 3) * 16)));
        const unsigned clipped = raw < 0 ? 0u : raw > 255 ? 255u : static_cast<unsigned>(raw);
        result |= static_cast<std::uint64_t>(clipped) << (i * 8);
    }
    return afmv_from_bits(result);
}
#define _m_paddd afmv_paddd
#define _m_psubd afmv_psubd
#define _m_psradi afmv_psradi
#define _m_psrawi afmv_psrawi
#define _m_psllwi afmv_psllwi
#define _m_psrlwi afmv_psrlwi
#define _m_pmulhw afmv_pmulhw
#define _m_paddsw afmv_paddsw
#define _m_psubsw afmv_psubsw
#define _m_paddusb afmv_paddusb
#define _m_punpcklbw afmv_punpcklbw
#define _m_punpckhbw afmv_punpckhbw
#define _m_punpcklwd afmv_punpcklwd
#define _m_punpckldq afmv_punpckldq
#define _m_pshufw afmv_pshufw
#define _m_pmaddwd afmv_pmaddwd
#define _m_packssdw afmv_packssdw
#define _m_packuswb afmv_packuswb

static const std::int16_t one_corr[4] = {1, 1, 1, 1};
static const std::int16_t round_inv_row[4] = {16384, 0, 16384, 0};
static const std::int16_t round_inv_col[4] = {32, 32, 32, 32};
static const std::int16_t round_inv_corr[4] = {31, 31, 31, 31};
static const std::int16_t tg_1_16[4] = {13036, 13036, 13036, 13036};
static const std::int16_t tg_2_16[4] = {27146, 27146, 27146, 27146};
static const std::int16_t tg_3_16[4] = {-21746, -21746, -21746, -21746};
static const std::int16_t cos_4_16[4] = {-19195, -19195, -19195, -19195};
static const std::int16_t tab_i_04[32] = {
    16384,21407,16384,8867,16384,8867,-16384,-21407,
    16384,-8867,16384,-21407,-16384,21407,16384,-8867,
    22725,19266,19266,-4520,12873,4520,-22725,-12873,
    12873,-22725,4520,-12873,4520,19266,19266,-22725};
static const std::int16_t tab_i_17[32] = {
    22725,29692,22725,12299,22725,12299,-22725,-29692,
    22725,-12299,22725,-29692,-22725,29692,22725,-12299,
    31521,26722,26722,-6270,17855,6270,-31521,-17855,
    17855,-31521,6270,-17855,6270,26722,26722,-31521};
static const std::int16_t tab_i_26[32] = {
    21407,27969,21407,11585,21407,11585,-21407,-27969,
    21407,-11585,21407,-27969,-21407,27969,21407,-11585,
    29692,25172,25172,-5906,16819,5906,-29692,-16819,
    16819,-29692,5906,-16819,5906,25172,25172,-29692};
static const std::int16_t tab_i_35[32] = {
    19266,25172,19266,10426,19266,10426,-19266,-25172,
    19266,-10426,19266,-25172,-19266,25172,19266,-10426,
    26722,22654,22654,-5315,15137,5315,-26722,-15137,
    15137,-26722,5315,-15137,5315,22654,22654,-26722};

static __m64 afmv_load64(const unsigned char* p) {
    __m64 value;
    std::memcpy(&value, p, sizeof(value));
    return value;
}
static void afmv_store64(unsigned char* p, __m64 value) {
    std::memcpy(p, &value, sizeof(value));
}
static void afmv_mc_avg_O16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 right = _m_pavgb(afmv_load64(ref + 8), afmv_load64(dest + 8));
        afmv_store64(dest, _m_pavgb(afmv_load64(ref), afmv_load64(dest)));
        ref += stride;
        afmv_store64(dest + 8, right);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_O8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 value = _m_pavgb(afmv_load64(ref), afmv_load64(dest));
        ref += stride;
        afmv_store64(dest, value);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_O16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const std::uint64_t right = *reinterpret_cast<const std::uint64_t*>(ref + 8);
        const std::uint64_t left = *reinterpret_cast<const std::uint64_t*>(ref);
        *reinterpret_cast<std::uint64_t*>(dest) = left;
        ref += stride;
        *reinterpret_cast<std::uint64_t*>(dest + 8) = right;
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_O8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const std::uint64_t value = *reinterpret_cast<const std::uint64_t*>(ref);
        ref += stride;
        *reinterpret_cast<std::uint64_t*>(dest) = value;
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_X16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 right = _m_pavgb(_m_pavgb(afmv_load64(ref + 8), afmv_load64(ref + 9)), afmv_load64(dest + 8));
        afmv_store64(dest, _m_pavgb(_m_pavgb(afmv_load64(ref), afmv_load64(ref + 1)), afmv_load64(dest)));
        ref += stride;
        afmv_store64(dest + 8, right);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_X8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 value = _m_pavgb(_m_pavgb(afmv_load64(ref), afmv_load64(ref + 1)), afmv_load64(dest));
        ref += stride;
        afmv_store64(dest, value);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_X16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 right = _m_pavgb(afmv_load64(ref + 1), afmv_load64(ref + 9));
        afmv_store64(dest, _m_pavgb(afmv_load64(ref), afmv_load64(ref + 1)));
        ref += stride;
        afmv_store64(dest + 8, right);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_X8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 value = _m_pavgb(afmv_load64(ref), afmv_load64(ref + 1));
        ref += stride;
        afmv_store64(dest, value);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_Y16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 right = _m_pavgb(_m_pavgb(afmv_load64(ref + 8), afmv_load64(ref + stride + 8)), afmv_load64(dest + 8));
        afmv_store64(dest, _m_pavgb(_m_pavgb(afmv_load64(ref), afmv_load64(ref + stride)), afmv_load64(dest)));
        ref += stride;
        afmv_store64(dest + 8, right);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_Y8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 value = _m_pavgb(_m_pavgb(afmv_load64(ref), afmv_load64(ref + stride)), afmv_load64(dest));
        ref += stride;
        afmv_store64(dest, value);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_Y16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 right = _m_pavgb(afmv_load64(ref + 8), afmv_load64(ref + stride + 8));
        afmv_store64(dest, _m_pavgb(afmv_load64(ref), afmv_load64(ref + stride)));
        ref += stride;
        afmv_store64(dest + 8, right);
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_Y8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        const __m64 value = _m_pavgb(afmv_load64(ref), afmv_load64(ref + stride));
        ref += stride;
        afmv_store64(dest, value);
        dest += stride;
        --height;
    } while (height != 0);
}
static __m64 afmv_mc_bilinear(__m64 a, __m64 b, __m64 c, __m64 d) {
    const __m64 ab = _m_pavgb(a, b);
    const __m64 cd = _m_pavgb(c, d);
    return _m_psubusb(_m_pavgb(ab, cd),
        _m_pand(_m_pand(_m_por(_m_pxor(c, d), _m_pxor(a, b)), _m_pxor(ab, cd)), afmv_last_bit));
}
static void afmv_mc_avg_XY16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        afmv_store64(dest, _m_pavgb(afmv_mc_bilinear(afmv_load64(ref), afmv_load64(ref + 1),
                                                       afmv_load64(ref + stride), afmv_load64(ref + stride + 1)),
                                     afmv_load64(dest)));
        afmv_store64(dest + 8, _m_pavgb(afmv_mc_bilinear(afmv_load64(ref + 1), afmv_load64(ref + 9),
                                                          afmv_load64(ref + stride + 8), afmv_load64(ref + stride + 9)),
                                        afmv_load64(dest + 8)));
        ref += stride;
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_avg_XY8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        afmv_store64(dest, _m_pavgb(afmv_mc_bilinear(afmv_load64(ref), afmv_load64(ref + 1),
                                                       afmv_load64(ref + stride), afmv_load64(ref + stride + 1)),
                                     afmv_load64(dest)));
        ref += stride;
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_XY16(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        afmv_store64(dest, afmv_mc_bilinear(afmv_load64(ref), afmv_load64(ref + 1),
                                             afmv_load64(ref + stride), afmv_load64(ref + stride + 1)));
        afmv_store64(dest + 8, afmv_mc_bilinear(afmv_load64(ref + 1), afmv_load64(ref + 9),
                                                 afmv_load64(ref + stride + 8), afmv_load64(ref + stride + 9)));
        ref += stride;
        dest += stride;
        --height;
    } while (height != 0);
}
static void afmv_mc_copy_XY8(unsigned char* dest, const unsigned char* ref, int stride, int height) {
    do {
        afmv_store64(dest, afmv_mc_bilinear(afmv_load64(ref), afmv_load64(ref + 1),
                                             afmv_load64(ref + stride), afmv_load64(ref + stride + 1)));
        ref += stride;
        dest += stride;
        --height;
    } while (height != 0);
}
afmv_mc_t afmv_mc = {
    {afmv_mc_copy_O16, afmv_mc_copy_X16, afmv_mc_copy_Y16, afmv_mc_copy_XY16,
     afmv_mc_copy_O8, afmv_mc_copy_X8, afmv_mc_copy_Y8, afmv_mc_copy_XY8},
    {afmv_mc_avg_O16, afmv_mc_avg_X16, afmv_mc_avg_Y16, afmv_mc_avg_XY16,
     afmv_mc_avg_O8, afmv_mc_avg_X8, afmv_mc_avg_Y8, afmv_mc_avg_XY8}
};

static const afmv_vlc2 afmv_mv_4[8] = {
    {3,6},{2,4},{1,3},{1,3},{0,2},{0,2},{0,2},{0,2}
};
static const afmv_vlc2 afmv_mv_10[48] = {
    {0,10},{0,10},{0,10},{0,10},{0,10},{0,10},{0,10},{0,10},
    {0,10},{0,10},{0,10},{0,10},{15,10},{14,10},{13,10},{12,10},
    {11,10},{10,10},{9,9},{9,9},{8,9},{8,9},{7,9},{7,9},
    {6,7},{6,7},{6,7},{6,7},{6,7},{6,7},{6,7},{6,7},
    {5,7},{5,7},{5,7},{5,7},{5,7},{5,7},{5,7},{5,7},
    {4,7},{4,7},{4,7},{4,7},{4,7},{4,7},{4,7},{4,7}
};
static const afmv_vlc2 afmv_dc_lum[31] = {
    {1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},
    {2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
    {0,3},{0,3},{0,3},{0,3},{3,3},{3,3},{3,3},{3,3},
    {4,3},{4,3},{4,3},{4,3},{5,4},{5,4},{6,5}
};
static const afmv_vlc2 afmv_dc_chroma[31] = {
    {0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},{0,2},
    {1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},{1,2},
    {2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},{2,2},
    {3,3},{3,3},{3,3},{3,3},{4,4},{4,4},{5,5}
};
static const afmv_vlc2 afmv_dc_lum_escape[16] = {
    {7,6},{7,6},{7,6},{7,6},{7,6},{7,6},{7,6},{7,6},
    {8,7},{8,7},{8,7},{8,7},{9,8},{9,8},{10,9},{11,9}
};
static const afmv_vlc2 afmv_dc_chroma_escape[32] = {
    {6,5},{6,5},{6,5},{6,5},{6,5},{6,5},{6,5},{6,5},
    {6,5},{6,5},{6,5},{6,5},{6,5},{6,5},{6,5},{6,5},
    {7,6},{7,6},{7,6},{7,6},{7,6},{7,6},{7,6},{7,6},
    {8,7},{8,7},{8,7},{8,7},{9,8},{9,8},{10,9},{11,9}
};
static const unsigned char afmv_cbp_large[256] = {
    0x05,0x07,0x05,0x07,0x05,0x07,0x05,0x07,0x04,0x07,0x04,0x07,0x04,0x07,0x04,0x07,
    0x04,0x07,0x04,0x07,0x04,0x07,0x04,0x07,0x00,0x01,0x00,0x01,0x01,0x02,0xFF,0x02,
    0x11,0x07,0x12,0x07,0x14,0x07,0x18,0x07,0x21,0x07,0x22,0x07,0x24,0x07,0x28,0x07,
    0x3F,0x06,0x3F,0x06,0x30,0x06,0x30,0x06,0x09,0x06,0x09,0x06,0x06,0x06,0x06,0x06,
    0x1F,0x05,0x1F,0x05,0x1F,0x05,0x1F,0x05,0x10,0x05,0x10,0x05,0x10,0x05,0x10,0x05,
    0x2F,0x05,0x2F,0x05,0x2F,0x05,0x2F,0x05,0x20,0x05,0x20,0x05,0x20,0x05,0x20,0x05,
    0x07,0x05,0x07,0x05,0x07,0x05,0x07,0x05,0x0B,0x05,0x0B,0x05,0x0B,0x05,0x0B,0x05,
    0x0D,0x05,0x0D,0x05,0x0D,0x05,0x0D,0x05,0x0E,0x05,0x0E,0x05,0x0E,0x05,0x0E,0x05,
    0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x0A,0x05,0x0A,0x05,0x0A,0x05,0x0A,0x05,
    0x03,0x05,0x03,0x05,0x03,0x05,0x03,0x05,0x0C,0x05,0x0C,0x05,0x0C,0x05,0x0C,0x05,
    0x01,0x04,0x01,0x04,0x01,0x04,0x01,0x04,0x01,0x04,0x01,0x04,0x01,0x04,0x01,0x04,
    0x02,0x04,0x02,0x04,0x02,0x04,0x02,0x04,0x02,0x04,0x02,0x04,0x02,0x04,0x02,0x04,
    0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,
    0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,0x08,0x04,
    0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,
    0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03,0x0F,0x03
};
static const unsigned char afmv_cbp_small[128] = {
    0x00,0x00,0x00,0x09,0x39,0x09,0x36,0x09,0x37,0x09,0x3B,0x09,0x3D,0x09,0x3E,0x09,
    0x17,0x08,0x17,0x08,0x1B,0x08,0x1B,0x08,0x1D,0x08,0x1D,0x08,0x1E,0x08,0x1E,0x08,
    0x27,0x08,0x27,0x08,0x2B,0x08,0x2B,0x08,0x2D,0x08,0x2D,0x08,0x2E,0x08,0x2E,0x08,
    0x19,0x08,0x19,0x08,0x16,0x08,0x16,0x08,0x29,0x08,0x29,0x08,0x26,0x08,0x26,0x08,
    0x35,0x08,0x35,0x08,0x3A,0x08,0x3A,0x08,0x33,0x08,0x33,0x08,0x3C,0x08,0x3C,0x08,
    0x15,0x08,0x15,0x08,0x1A,0x08,0x1A,0x08,0x13,0x08,0x13,0x08,0x1C,0x08,0x1C,0x08,
    0x25,0x08,0x25,0x08,0x2A,0x08,0x2A,0x08,0x23,0x08,0x23,0x08,0x2C,0x08,0x2C,0x08,
    0x31,0x08,0x31,0x08,0x32,0x08,0x32,0x08,0x34,0x08,0x34,0x08,0x38,0x08,0x38,0x08
};

class nvlMovieBase;
class nvlMovie;
void nvl_RequestCallback(nflRequestState reason, nflRequestID requestID, nvlMovieBase* userData);

class nvlMovieBase {
public:
    nvlMovieBase();
    virtual ~nvlMovieBase();
    virtual nvlResult InitMovie();
    virtual nvlFrameState DecodeFrame();
    virtual void Reset();

    static char* GetVersion();
    static void SetConvertNTSCtoPAL(bool convertPal);
    nflFileID GetFileID();
    nflFileID ReleaseMovie();
    nvlMovieState SetMovieState(nvlMovieState newState);
    nvlMovieState GetMovieState();
    void SetUserDataCallback(void (*callback)(unsigned char*, unsigned, bool, void*), void* user);
    int GetNumFrames();
    int GetFrameNumber();
    nglTexture* GetTexture();
    nvlResult SetAudioTrack(int audioTrackID);
    bool IsBufferLoading();
    bool BufferCheckBytes(int thisDataSize, bool update);
    bool IsBufferFull();
    void RequestCallback(nflRequestState reason, nflRequestID callreqID);

protected:
    void CheckNextNFLRequest();

    int mWidth;
    int mHeight;
    int mWorkBufferSize;
    int mImageBufferSize;
    int mStreamBufferSize;
    unsigned char* mDataBuffer;
    unsigned char* mImageBuffer;
    unsigned char* mIntBuffer[4];
    nglTexture* mSwapTexture[2];
    nvlPhase mMoviePhase;
    int mTotalFrames;
    int mFrameNumber;
    nvlMovieState mMovieState;
    int mAudiotrackID;
    float mConvertRate;
    void (*mUserDataCallback)(unsigned char*, unsigned, bool, void*);
    void* mUserData;
    int mPackOffset;
    nflFileID mFileID;
    nflRequestID mRequestID[4];
    bool mBufferValid[4];
    int mAssetFlags;
    int mMovieFlags;
    bool mBackBuffer;
    int mFileSize;
    int mFileOffset;
    int mReadBuffer;
    int mPlayBuffer;
    int mPackBuffer;
    int mFirstBuffer;
    int mLastBuffer;
    int mLastBufferSize;
    unsigned char mQuantizerMatrix[2][64];
    unsigned char* mBufYUV[3][3];
    unsigned char* mBufRGB;
    unsigned char* mDecodePnt;
    unsigned char* mParserPnt;
    static bool mConvertPal;
};
static_assert(sizeof(nvlMovieBase) == 336, "NVL base layout mismatch");

bool nvlMovieBase::mConvertPal = false;

class nvlAFMVMovie : public nvlMovieBase {
public:
    nvlAFMVMovie();
    ~nvlAFMVMovie() override;
    nvlResult InitMovie() override;
    nvlFrameState DecodeFrame() override;
    nvlResult ParseHeader(nflFileID fileID, bool backBuffer, int offset, int formal);
    void PrecalcScaler(int index);
    unsigned GetMBModes();
    void GetQuantScale();
    int GetMotionDiff(int fCode);
    int SignExtendVector(int vector, char fCode);
    int GetDMV();
    int GetCBP();
    unsigned GetLuminanceDiff();
    unsigned GetChromaDiff();
    void IntraDCT(int cc, unsigned char* dst, int stride);
    void NonIntraDCT(int formal, unsigned char* dst, int stride);

protected:
    void ProcessUserDataChunk(bool firstChunk);
    void GetIntraCoefB14(const unsigned short* table);
    void GetIntraCoefB15(const unsigned short* table);
    int GetNonIntraCoef(const unsigned short* table);
    void DoMotionFrame(afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int));
    void DoMotionField(afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int));
    void DoMotionDualP(afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int));
    void DoMotionSame(afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int));
    void DoMotionCopy(afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int));
    int DecodeSlice();

    bool mAudioIsValid;
    unsigned mShifter;
    int mBitCount;
    short* mDCTblock;
    unsigned short* mCurrentQuantizer[2];
    unsigned short mQuantizerPrescale[2][32][64];
    unsigned char mProfileLevel;
    unsigned mPictureType;
    afmv_motion_t mForwVector;
    afmv_motion_t mBackVector;
    void (*mMotionParser[5])(nvlMovie*, afmv_motion_t*, void (*const)(unsigned char*, const unsigned char*, int, int));
    short mDcDctPred[3];
    int mHorzOffset;
    int mStride;
    int mUVStride;
    int mSliceStride;
    int mSliceUVStride;
    int mLimitX;
    int mLimitY;
    int mLimitY8;
    int mLimitY16;
    int mVertOffset;
    unsigned char* mDest[3];
    unsigned char* temp_rgb;
    unsigned mBufIndex;
    unsigned mFrameReady;
    unsigned mIntraDcPrecision;
    unsigned mFramePredFrameDct;
    unsigned mIntraVlcFormat;
    const unsigned char* mScanMatrix;
    char mScaled[2];
    char mQScaleType;

public:
    static unsigned char* mAudioData;
    static int mAudioOffset;
    static unsigned mAudioStartOffset;
};

class nvlMovie : public nvlAFMVMovie {
public:
    void ProcessAudioChunk();
    void StartAudioPlayback();
    void StopAudioPlayback();
    ~nvlMovie() override;
    static IDirectSoundBuffer* mAudioBuffer;
};

static_assert(sizeof(nvlAFMVMovie) == 8784, "AFMV movie layout mismatch");
static_assert(sizeof(nvlMovie) == 8784, "NVL movie layout mismatch");

unsigned char* nvlLockTexture(nglTexture* texture, bool backBuffer) {
    D3DLOCKED_RECT rect = {};
    D3DTexture_LockRect(reinterpret_cast<D3DTexture*>(texture->Texture), 0, &rect, nullptr,
                        backBuffer ? 0x40u : 0u);
    return static_cast<unsigned char*>(rect.pBits);
}

void nvlUnlockTexture(nglTexture*, bool) {}

IDirectSoundBuffer* nvlMovie::mAudioBuffer = nullptr;
unsigned char* nvlAFMVMovie::mAudioData = nullptr;
int nvlAFMVMovie::mAudioOffset = 0;
unsigned nvlAFMVMovie::mAudioStartOffset = 0;
static unsigned char nvlVersionByte = 0;

char* nvlMovieBase::GetVersion() { return reinterpret_cast<char*>(&nvlVersionByte); }
nflFileID nvlMovieBase::GetFileID() { return mFileID; }

nflFileID nvlMovieBase::ReleaseMovie() {
    nglSetEndOfVBlankCallback(nullptr, nullptr);
    if (!mBackBuffer) {
        if ((mAssetFlags & 1) != 0) {
            for (int i = 0; i < 2; ++i) {
                nglDestroyTexture(mSwapTexture[i]);
                mSwapTexture[i] = nullptr;
            }
        }
        mAssetFlags >>= 1;
    }
    if ((mAssetFlags & 1) != 0) {
        tlMemFree(mImageBuffer);
        mImageBuffer = nullptr;
    }
    const int imageFlag = mAssetFlags >> 1;
    mAssetFlags >>= 1;
    if ((imageFlag & 1) != 0) {
        for (int i = 0; i < 4; ++i) {
            tlMemFree(mIntBuffer[i]);
            mIntBuffer[i] = nullptr;
        }
    }
    const int dataFlag = mAssetFlags >> 1;
    mAssetFlags >>= 1;
    if ((dataFlag & 1) != 0) {
        tlMemFree(mDataBuffer);
        mDataBuffer = nullptr;
    }
    const nflFileID result = mFileID;
    mAssetFlags = 0;
    return result;
}

nvlMovieState nvlMovieBase::SetMovieState(nvlMovieState newState) { mMovieState = newState; return newState; }
nvlMovieState nvlMovieBase::GetMovieState() { return mMovieState; }
void nvlMovieBase::SetUserDataCallback(void (*callback)(unsigned char*, unsigned, bool, void*), void* user) {
    mUserDataCallback = callback;
    mUserData = user;
}
int nvlMovieBase::GetNumFrames() { return mTotalFrames; }
int nvlMovieBase::GetFrameNumber() { return mFrameNumber; }
nglTexture* nvlMovieBase::GetTexture() { return mSwapTexture[0]; }
nvlFrameState nvlMovieBase::DecodeFrame() { return NVL_FRAME_ERROR; }
void nvlMovieBase::SetConvertNTSCtoPAL(bool convertPal) { mConvertPal = convertPal; }
nvlResult nvlMovieBase::SetAudioTrack(int audioTrackID) { mAudiotrackID = audioTrackID; return NVL_RESULT_OK; }
bool nvlMovieBase::IsBufferLoading() {
    return mRequestID[0] != NFL_REQUEST_ID_INVALID || mRequestID[1] != NFL_REQUEST_ID_INVALID ||
           mRequestID[2] != NFL_REQUEST_ID_INVALID || mRequestID[3] != NFL_REQUEST_ID_INVALID;
}

void nvlMovieBase::Reset() {
    mFirstBuffer = 15;
    mLastBuffer = 15;
    mDecodePnt = mIntBuffer[0];
    mParserPnt = mIntBuffer[0];
    mFrameNumber = 0;
    mConvertRate = 0.0f;
    mMoviePhase = NVL_PHASE_FIRST;
    mReadBuffer = 0;
    mPlayBuffer = 3;
    mPackBuffer = 0;
    mLastBufferSize = 0;
    mFileOffset = 0;
    for (int i = 0; i < 4; ++i) {
        mRequestID[i] = NFL_REQUEST_ID_INVALID;
        mBufferValid[i] = false;
    }
}

void nvlMovieBase::CheckNextNFLRequest() {
    nflRequestParams request = {};
    request.fileID = NFL_FILE_ID_INVALID;
    request.streamID = NFL_STREAM_ID_DEFAULT;
    request.callback = nullptr;
    request.type = NFL_REQUEST_TYPE_INVALID;
    request.priority = NFL_PRIORITY_NORMAL;
    if (mReadBuffer == mPlayBuffer)
        return;
    do {
        const int readBuffer = mReadBuffer;
        if (mRequestID[readBuffer] != NFL_REQUEST_ID_INVALID)
            break;
        const int fileOffset = mFileOffset;
        request.fileID = mFileID;
        request.streamID = NFL_STREAM_ID_DEFAULT;
        request.callback = reinterpret_cast<void (*)(nflRequestState, nflRequestID, void*)>(nvl_RequestCallback);
        request.type = NFL_REQUEST_TYPE_READ;
        request.priority = NFL_PRIORITY_NORMAL;
        request.fileOffset = static_cast<unsigned>(fileOffset + mPackOffset);
        request.buffer = mIntBuffer[readBuffer];
        request.timeout = 0;
        request.userData = this;
        if (fileOffset == 0)
            mFirstBuffer = readBuffer;
        const unsigned streamBufferSize = static_cast<unsigned>(mStreamBufferSize);
        const unsigned remaining = static_cast<unsigned>(mFileSize - fileOffset);
        if (remaining <= streamBufferSize) {
            mLastBufferSize = static_cast<int>(remaining);
            mLastBuffer = readBuffer;
            mFileOffset = 0;
            request.dataSize = (remaining + 2047u) & 0xFFFFF800u;
        } else {
            request.dataSize = streamBufferSize;
            mFileOffset = static_cast<int>(streamBufferSize) + fileOffset;
        }
        mRequestID[readBuffer] = nflAddRequest(&request);
        mReadBuffer = (readBuffer + 1) & 3;
    } while (mReadBuffer != mPlayBuffer);
}

nvlMovieBase::nvlMovieBase() {
    mDataBuffer = nullptr;
    for (int i = 0; i < 4; ++i) mIntBuffer[i] = nullptr;
    for (int i = 0; i < 2; ++i) mSwapTexture[i] = nullptr;
    mBackBuffer = false;
    mAssetFlags = 0;
    mHeight = 0;
    mWidth = 0;
    mImageBufferSize = 0;
    mStreamBufferSize = 0;
    mWorkBufferSize = 0;
    mAudiotrackID = 0;
    mUserDataCallback = nullptr;
    mUserData = nullptr;
    Reset();
}
nvlMovieBase::~nvlMovieBase() { ReleaseMovie(); }

nvlResult nvlMovieBase::InitMovie() {
    if (mAssetFlags != 0 && _tlAssert("src/nvl_base.cpp", 70, "!mAssetFlags",
                                      "NVL: This movie was already started"))
        __debugbreak();
    mAssetFlags = 2;
    if (mDataBuffer == nullptr) {
        mDataBuffer = static_cast<unsigned char*>(tlMemAlloc(static_cast<unsigned>(mWorkBufferSize), 0x40, 0));
        mAssetFlags |= 1;
    }
    mAssetFlags *= 2;
    if (mIntBuffer[0] == nullptr) {
        for (int i = 0; i < 4; ++i)
            mIntBuffer[i] = static_cast<unsigned char*>(tlMemAlloc(static_cast<unsigned>(mStreamBufferSize), 0x40, 0));
        mAssetFlags |= 1;
    }
    mAssetFlags *= 2;
    if (mImageBuffer == nullptr && mImageBufferSize != 0) {
        mImageBuffer = static_cast<unsigned char*>(tlMemAlloc(static_cast<unsigned>(mImageBufferSize), 0x80, 0));
        mAssetFlags |= 1;
    }
    if (!mBackBuffer) {
        mAssetFlags *= 2;
        if (mSwapTexture[0] == nullptr) {
            for (int i = 0; i < 2; ++i)
                mSwapTexture[i] = nglCreateTexture(0, 0x12, mWidth, mHeight, 0, 1);
            mAssetFlags |= 1;
        }
    }
    Reset();
    mMovieState = NVL_STATE_PLAY;
    for (int i = 0; i < 4; ++i) mRequestID[i] = NFL_REQUEST_ID_INVALID;
    CheckNextNFLRequest();
    unsigned char* data = mDataBuffer;
    if (mMovieFlags == 1) {
        int widthBytes = 16 * mWidth;
        mBufYUV[0][0] = data;
        unsigned char* next = data + widthBytes;
        widthBytes >>= 2;
        mBufYUV[0][1] = next;
        next += widthBytes;
        mBufYUV[0][2] = next;
        data = next + widthBytes;
    } else if (mMovieFlags == 3 || mMovieFlags == 7) {
        const int width = mWidth;
        const int planeSize = width * mHeight;
        mBufYUV[0][0] = data;
        data += planeSize;
        mBufYUV[0][1] = data;
        data += planeSize >> 2;
        mBufYUV[0][2] = data;
        data += planeSize >> 2;
        mBufYUV[1][0] = data;
        data += planeSize;
        mBufYUV[1][1] = data;
        data += planeSize >> 2;
        mBufYUV[1][2] = data;
        data += planeSize >> 2;
        const int sliceWidth = width * 16;
        mBufYUV[2][0] = data;
        data += sliceWidth;
        mBufYUV[2][1] = data;
        data += sliceWidth >> 2;
        mBufYUV[2][2] = data;
        mBufRGB = data + (sliceWidth >> 2);
        return NVL_RESULT_OK;
    }
    mBufRGB = data;
    return NVL_RESULT_OK;
}

bool nvlMovieBase::BufferCheckBytes(int thisDataSize, bool update) {
    const int oldPlayBuffer = mPlayBuffer;
    if (oldPlayBuffer != mPackBuffer) {
        mBufferValid[oldPlayBuffer] = false;
        mPlayBuffer = mPackBuffer;
        CheckNextNFLRequest();
    }
    const int playBuffer = mPlayBuffer;
    if (!mBufferValid[playBuffer]) return false;
    const int lastBufferSize = playBuffer == mLastBuffer ? mLastBufferSize : mStreamBufferSize;
    unsigned char* parser = mParserPnt;
    const int bytesToEnd = static_cast<int>(&mIntBuffer[playBuffer][lastBufferSize] - parser);
    if (thisDataSize > bytesToEnd) {
        const int nextBuffer = (playBuffer + 1) & 3;
        if (!mBufferValid[nextBuffer]) return false;
        if (playBuffer == mLastBuffer) {
            mBufferValid[playBuffer] = false;
            mPlayBuffer = (mPlayBuffer + 1) & 3;
            mPackBuffer = mPlayBuffer;
            mDecodePnt = mIntBuffer[mPlayBuffer];
            mParserPnt = mIntBuffer[mPlayBuffer];
            mLastBuffer = 15;
            CheckNextNFLRequest();
            if (update) {
                mParserPnt += thisDataSize;
                return true;
            }
        } else if (!update) {
            std::memcpy(mIntBuffer[playBuffer], parser, static_cast<size_t>(bytesToEnd));
            std::memcpy(mIntBuffer[mPlayBuffer] + bytesToEnd,
                        mIntBuffer[(mPlayBuffer + 1) & 3],
                        static_cast<size_t>(thisDataSize - bytesToEnd));
            mDecodePnt = mIntBuffer[mPlayBuffer];
            return true;
        } else {
            const int carry = thisDataSize - bytesToEnd;
            mPackBuffer = nextBuffer;
            const unsigned alignedEnd = (static_cast<unsigned>(bytesToEnd) + 31u) & ~31u;
            unsigned char* alignedParser = reinterpret_cast<unsigned char*>(
                reinterpret_cast<uintptr_t>(parser) & ~static_cast<uintptr_t>(31));
            const unsigned alignedCarry = (static_cast<unsigned>(carry) + 31u) & ~31u;
            std::memmove(alignedParser - alignedCarry, alignedParser, alignedEnd);
            std::memcpy(alignedParser + alignedEnd - alignedCarry,
                        mIntBuffer[mPackBuffer], alignedCarry);
            mDecodePnt = parser - alignedCarry;
            mParserPnt = parser + (mIntBuffer[mPackBuffer] - mIntBuffer[mPlayBuffer])
                         - mStreamBufferSize + thisDataSize;
        }
    } else {
        mDecodePnt = parser;
        if (update) {
            mParserPnt = parser + thisDataSize;
            return true;
        }
    }
    return true;
}

bool nvlMovieBase::IsBufferFull() {
    bool allValid = true;
    if (mMovieState != NVL_STATE_ERROR) {
        if (!mBufferValid[0]) allValid = mPlayBuffer == 0;
        if (!mBufferValid[1] && mPlayBuffer != 1) allValid = false;
        if (!mBufferValid[2] && mPlayBuffer != 2) allValid = false;
        if (mBufferValid[3] || mPlayBuffer == 3) {
            if (!allValid) CheckNextNFLRequest();
            return allValid;
        }
        CheckNextNFLRequest();
        return false;
    }
    return true;
}

void nvlMovieBase::RequestCallback(nflRequestState reason, nflRequestID callreqID) {
    int index = 0;
    while (index < 4 && mRequestID[index] != callreqID) ++index;
    if (index < 4) {
        if (reason != NFL_REQUEST_STATE_COMPLETED) {
            mMovieState = NVL_STATE_ERROR;
            tlFatal("NVL error: NFL reported error reading file\n");
        } else {
            mRequestID[index] = NFL_REQUEST_ID_INVALID;
            mBufferValid[index] = true;
            CheckNextNFLRequest();
        }
    } else if (_tlAssert("src/nvl_base.cpp", 490, "0",
                         "NVL: NFL callback with an invalid ID!\n")) {
        __debugbreak();
    }
}

void nvl_RequestCallback(nflRequestState reason, nflRequestID requestID, nvlMovieBase* userData) {
    if (reason != NFL_REQUEST_STATE_CANCELED) userData->RequestCallback(reason, requestID);
}

nvlAFMVMovie::nvlAFMVMovie() : mDCTblock(nullptr) {
    mDCTblock = static_cast<short*>(tlMemAlloc(0x80, 0x40, 0));
    if (mDCTblock == nullptr && _tlAssert("src/nvl_afmv.cpp", 63, "mDCTblock",
                                          "NVL: Could not allocate memory for decoder DCT block"))
        __debugbreak();
    std::memset(mDCTblock, 0, 0x80);
    mScaled[1] = 0;
    mScaled[0] = 0;
    mAudioOffset = 0;
    mAudioStartOffset = 0;
    mAudioIsValid = false;
}
nvlAFMVMovie::~nvlAFMVMovie() { tlMemFree(mDCTblock); }
nvlResult nvlAFMVMovie::InitMovie() {
    mUVStride = mWidth >> 1;
    mSliceStride = 16 * mWidth;
    mStride = mWidth;
    mSliceUVStride = (16 * mWidth) >> 2;
    mLimitX = 2 * mWidth - 32;
    mLimitY16 = 2 * (mHeight - 16);
    mBufIndex = 0;
    mLimitY8 = 2 * mHeight - 16;
    mLimitY = mHeight - 16;
    return nvlMovieBase::InitMovie();
}

void nvlAFMVMovie::PrecalcScaler(int index) {
    if (mScaled[index] == mQScaleType)
        return;
    mScaled[index] = mQScaleType;
    for (int scale = 0; scale < 32; ++scale) {
        const int factor = mQScaleType != 0 ? afmv_alternate_scale[scale] : 2 * scale;
        for (int i = 0; i < 64; ++i)
            mQuantizerPrescale[index][scale][i] = static_cast<unsigned short>(factor * mQuantizerMatrix[index][i]);
    }
}

unsigned nvlAFMVMovie::GetMBModes() {
    unsigned result;
    switch (mPictureType) {
    case 1: {
        const afmv_mbtab& tab = afmv_mb_i[mShifter >> 31];
        mShifter <<= tab.len;
        mBitCount += tab.len;
        result = tab.modes;
        if (mFramePredFrameDct != 0)
            return result;
        const unsigned old = mShifter;
        mShifter <<= 1;
        ++mBitCount;
        return result | 32 * (old >> 31);
    }
    case 2: {
        const afmv_mbtab& tab = afmv_mb_p[(mShifter >> 27) & 31];
        mShifter <<= tab.len;
        mBitCount += tab.len;
        int modes = tab.modes;
        if (mFramePredFrameDct != 0)
            return (modes & 8) != 0 ? static_cast<unsigned>(modes | 0x88) : static_cast<unsigned>(modes | 8);
        if ((modes & 8) != 0) {
            const unsigned old = mShifter;
            mShifter <<= 2;
            modes |= (old >> 24) & 0xC0;
            mBitCount += 2;
        }
        if ((modes & 3) != 0) {
            const unsigned old = mShifter;
            mShifter <<= 1;
            modes |= 32 * (old >> 31);
            ++mBitCount;
        }
        return static_cast<unsigned>(modes | 8);
    }
    case 3: {
        const afmv_mbtab& tab = afmv_mb_b[(mShifter >> 26) & 63];
        mShifter <<= tab.len;
        mBitCount += tab.len;
        result = tab.modes;
        if (mFramePredFrameDct != 0)
            return result | 0x80;
        result |= (mShifter >> 24) & 0xC0;
        mShifter <<= 2;
        mBitCount += 2;
        if ((result & 3) != 0) {
            const unsigned old = mShifter;
            mShifter <<= 1;
            ++mBitCount;
            result |= 32 * (old >> 31);
        }
        return result;
    }
    default:
        return 0;
    }
}

void nvlAFMVMovie::GetQuantScale() {
    const unsigned scale = mShifter >> 27;
    mShifter <<= 5;
    mBitCount += 5;
    mCurrentQuantizer[0] = mQuantizerPrescale[0][scale];
    mCurrentQuantizer[1] = mQuantizerPrescale[1][scale];
}

int nvlAFMVMovie::SignExtendVector(int vector, char fCode) {
    return vector << (27 - fCode) >> (27 - fCode);
}

int nvlAFMVMovie::GetDMV() {
    const afmv_dmvt& tab = afmv_dmv_2[mShifter >> 30];
    mShifter <<= tab.len;
    mBitCount += tab.len;
    return tab.dmv;
}

int nvlAFMVMovie::GetMotionDiff(int fCode) {
    const unsigned shifter = mShifter;
    if ((shifter & 0x80000000u) != 0) {
        mShifter = shifter << 1;
        ++mBitCount;
        return 0;
    }

    const int oldBitCount = mBitCount;
    if (shifter < 0x0C000000u) {
        const afmv_vlc2& tab = afmv_mv_10[shifter >> 22];
        int value = (static_cast<int>(tab.value) << fCode) + 1;
        mBitCount = oldBitCount + tab.len + 1;
        const uint64_t shifted = 2ull * (static_cast<uint64_t>(shifter << tab.len));
        const unsigned sign = static_cast<unsigned>(shifted >> 32);
        mShifter = static_cast<unsigned>(shifted);
        if (fCode != 0) {
            if (mBitCount > 0) {
                const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
                mDecodePnt += 2;
                mShifter |= bits << mBitCount;
                mBitCount -= 16;
            }
            const unsigned beforeSign = mShifter;
            value += beforeSign >> (32 - fCode);
            mBitCount += fCode;
            mShifter = beforeSign << fCode;
        }
        return (value ^ static_cast<int>(sign)) - static_cast<int>(sign);
    }

    const afmv_vlc2& tab = afmv_mv_4[shifter >> 28];
    int value = (static_cast<int>(tab.value) << fCode) + 1;
    mBitCount = oldBitCount + tab.len + fCode + 1;
    const unsigned shifted = shifter << tab.len;
    const unsigned doubled = shifted << 1;
    mShifter = doubled;
    if (fCode != 0)
        value += doubled >> (32 - fCode);
    mShifter = doubled << fCode;
    const int sign = static_cast<int>(shifted >> 31);
    return (value ^ sign) - sign;
}

int nvlAFMVMovie::GetCBP() {
    if (mBitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= bits << mBitCount;
        mBitCount -= 16;
    }
    const unsigned index = mShifter < 0x20000000u ? (mShifter >> 23) : (mShifter >> 25);
    const unsigned char* table = mShifter < 0x20000000u ? afmv_cbp_small : afmv_cbp_large;
    const unsigned offset = index << 1;
    mShifter <<= table[offset + 1];
    mBitCount += table[offset + 1];
    return table[offset];
}

unsigned nvlAFMVMovie::GetLuminanceDiff() {
    const unsigned shifter = mShifter;
    if (shifter < 0xF8000000u) {
        const afmv_vlc2& tab = afmv_dc_lum[shifter >> 27];
        if (tab.value != 0) {
            mBitCount += tab.value + tab.len;
            const unsigned shifted = shifter << tab.len;
            const unsigned sign = static_cast<unsigned>(static_cast<int32_t>(~shifted) >> 31) >> (32 - tab.value);
            const unsigned result = ((shifted >> (32 - tab.value)) - sign) << mIntraDcPrecision;
            mShifter = shifted << tab.value;
            return result;
        }
        mBitCount += 3;
        mShifter = shifter << 3;
        return 0;
    }

    const afmv_vlc2& tab = afmv_dc_lum_escape[(shifter >> 23) - 0x1F0];
    mShifter = shifter << tab.len;
    mBitCount += tab.len;
    if (mBitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= bits << mBitCount;
        mBitCount -= 16;
    }
    const unsigned sign = static_cast<unsigned>(static_cast<int32_t>(~mShifter) >> 31) >> (32 - tab.value);
    const unsigned result = ((mShifter >> (32 - tab.value)) - sign) << mIntraDcPrecision;
    mShifter <<= tab.value;
    mBitCount += tab.value;
    return result;
}

unsigned nvlAFMVMovie::GetChromaDiff() {
    const unsigned shifter = mShifter;
    if (shifter < 0xF8000000u) {
        const afmv_vlc2& tab = afmv_dc_chroma[shifter >> 27];
        if (tab.value != 0) {
            mBitCount += tab.value + tab.len;
            const unsigned shifted = shifter << tab.len;
            const unsigned sign = static_cast<unsigned>(static_cast<int32_t>(~shifted) >> 31) >> (32 - tab.value);
            const unsigned result = ((shifted >> (32 - tab.value)) - sign) << mIntraDcPrecision;
            mShifter = shifted << tab.value;
            return result;
        }
        mBitCount += 2;
        mShifter = shifter << 2;
        return 0;
    }

    const afmv_vlc2& tab = afmv_dc_chroma_escape[(shifter >> 22) - 0x3E0];
    mShifter = shifter << tab.len;
    mBitCount += tab.len;
    if (mBitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= bits << mBitCount;
        mBitCount -= 16;
    }
    const unsigned sign = static_cast<unsigned>(static_cast<int32_t>(~mShifter) >> 31) >> (32 - tab.value);
    const unsigned result = ((mShifter >> (32 - tab.value)) - sign) << mIntraDcPrecision;
    mShifter <<= tab.value;
    mBitCount += tab.value;
    return result;
}

void nvlAFMVMovie::IntraDCT(int cc, unsigned char* dst, int stride) {
    if (mBitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= bits << mBitCount;
        mBitCount -= 16;
    }
    mDcDctPred[cc] += static_cast<short>(cc != 0 ? GetChromaDiff() : GetLuminanceDiff());
    mDCTblock[0] = mDcDctPred[cc];
    if (mIntraVlcFormat != 0)
        GetIntraCoefB15(mCurrentQuantizer[0]);
    else
        GetIntraCoefB14(mCurrentQuantizer[0]);
    MacroBlockIdctCopy(mDCTblock, dst, stride);
}

void nvlAFMVMovie::NonIntraDCT(int, unsigned char* dst, int stride) {
    MacroBlockIdctAdd(GetNonIntraCoef(mCurrentQuantizer[1]), mDCTblock, dst, stride);
}

nvlFrameState nvlAFMVMovie::DecodeFrame() { return NVL_FRAME_ERROR; }
nvlResult nvlAFMVMovie::ParseHeader(nflFileID fileID, bool backBuffer, int offset, int) {
    mMovieState = NVL_STATE_ERROR;
    unsigned* fileHeader = static_cast<unsigned*>(tlMemAlloc(0x800, 0x40, 0));
    if (fileHeader == nullptr && _tlAssert("src/nvl_afmv.cpp", 110, "file_header",
                                           "NVL: Could not allocate memory for header parsing"))
        __debugbreak();
    if (nflReadFile(fileID, static_cast<unsigned>(offset), fileHeader, 0x800) != 0x800)
        return NVL_RESULT_ERROR;
    if (fileHeader[0] != 1447904833u &&
        _tlAssert("src/nvl_afmv.cpp", 116,
                  "tl_le32( file_header->afmv_id ) == NVL_AFMV_ID",
                  "NVL: Invalid file format"))
        __debugbreak();
    mFileID = fileID;
    mPackOffset = offset;
    mBackBuffer = backBuffer;
    mFileSize = static_cast<int>(fileHeader[1]);
    mWidth = static_cast<int>(fileHeader[4]);
    mHeight = static_cast<int>(fileHeader[5]);
    mTotalFrames = static_cast<int>(fileHeader[6]);
    mMovieFlags = static_cast<int>(fileHeader[14]);
    std::memcpy(mQuantizerMatrix, fileHeader + 4, 0x40);
    std::memcpy(mQuantizerMatrix[1], fileHeader + 20, sizeof(mQuantizerMatrix[1]));
    tlMemFree(fileHeader);

    mStreamBufferSize = 0x40000;
    if (mFileSize < 0x100000) {
        do {
            mStreamBufferSize >>= 1;
        } while (mFileSize < 4 * mStreamBufferSize);
    }
    const int format = mMovieFlags - 1;
    mWorkBufferSize = 0;
    if (format == 0) {
        mWorkBufferSize = 24 * mWidth;
    } else if (format == 2) {
        mWorkBufferSize = 3 * mWidth * mHeight;
    } else if (format == 6) {
        mWorkBufferSize = 3 * mWidth * (mHeight + 8);
    } else {
        if (_tlAssert("src/nvl_afmv.cpp", 167, "0", "NVL: Invalid file header\n"))
            __debugbreak();
        return NVL_RESULT_ERROR;
    }
    if (mBackBuffer)
        mWorkBufferSize += mWidth << 6;
    Reset();
    return NVL_RESULT_OK;
}
void nvlAFMVMovie::ProcessUserDataChunk(bool headerData) {
    if (mUserDataCallback == nullptr)
        return;
    if (headerData) {
        mUserDataCallback(mDecodePnt + 8, 32, true, mUserData);
        mUserDataCallback(mDecodePnt + 40, *mDecodePnt - 32, false, mUserData);
    } else {
        mUserDataCallback(mDecodePnt + 8, *mDecodePnt, false, mUserData);
    }
}
void nvlAFMVMovie::GetIntraCoefB14(const unsigned short* quant_matrix) {
    unsigned int shifter = mShifter;
    int bitCount = mBitCount;
    unsigned char* decodePnt = mDecodePnt;
    unsigned char* bitPtr = decodePnt;
    short* dctDest = mDCTblock;
    const unsigned char* scan = mScanMatrix;
    const afmv_dcttab* tab = nullptr;
    unsigned short bits = 0;
    unsigned int shifted = 0;
    int pos = 0;
    int mismatch = ~*dctDest;
    int value = 0;

    if (bitCount > 0) {
        bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
        decodePnt += 2;
        bitPtr = decodePnt;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }

decode_loop:
    if (shifter >= 0x28000000u) {
        tab = &afmv_dct_b14ac_5[(shifter >> 27) - 5];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter < 0x04000000u)
        goto low_tables;
    tab = &afmv_dct_b14_8[(shifter >> 24) - 4];
    pos += tab->run;
    if (pos < 64)
        goto normal_code;
    pos = pos + ((shifter >> 20) & 0x3F) - 64;
    if (pos >= 64)
        goto done;
    {
        int escapeBitCount = bitCount + 12;
        unsigned int escapeShifter = shifter << 12;
        if (escapeBitCount > 0) {
            bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
            bitPtr = decodePnt + 2;
            escapeShifter |= static_cast<unsigned int>(bits) << escapeBitCount;
            escapeBitCount -= 16;
        }
        value = 16 * ((escapeShifter >> 20) * quant_matrix[scan[pos]] / 16);
        if (value != value)
            value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
        dctDest[scan[pos]] = static_cast<short>(value);
        shifter = escapeShifter << 12;
        mismatch ^= value;
        bitCount = escapeBitCount + 12;
    }
    goto refill;

low_tables:
    if (shifter >= 0x02000000u) {
        tab = &afmv_dct_b14_10[(shifter >> 22) - 8];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00800000u) {
        tab = &afmv_dct_13[(shifter >> 19) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00200000u) {
        tab = &afmv_dct_15[(shifter >> 17) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
    tab = &afmv_dct_16[shifter >> 16];
    shifter = (static_cast<unsigned int>(bits) << (bitCount + 16)) | (shifter << 16);
    bitPtr += 2;
    pos += tab->run;
    if (pos < 64)
        goto normal_code;
    decodePnt = bitPtr;
    goto done;

normal_code:
    shifted = shifter << tab->len;
    bitCount += tab->len + 1;
    value = static_cast<int>(((shifted >> 27) ^ (tab->level * quant_matrix[scan[pos]])) & 0xFFFFFFF0u)
        - 16 * static_cast<int>(shifted >> 31);
    if (value != value)
        value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
    dctDest[scan[pos]] = static_cast<short>(value);
    mismatch ^= value;
    shifter = 2 * shifted;

refill:
    if (bitCount > 0) {
        bits = static_cast<unsigned short>((bitPtr[0] << 8) | bitPtr[1]);
        bitPtr += 2;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }
    decodePnt = bitPtr;
    goto decode_loop;

done:
    dctDest[63] ^= static_cast<short>(mismatch & 0x10);
    mBitCount = bitCount + 2;
    mShifter = 4 * shifter;
    mDecodePnt = decodePnt;
}

void nvlAFMVMovie::GetIntraCoefB15(const unsigned short* quant_matrix) {
    unsigned int shifter = mShifter;
    int bitCount = mBitCount;
    unsigned char* decodePnt = mDecodePnt;
    unsigned char* bitPtr = decodePnt;
    short* dctDest = mDCTblock;
    const unsigned char* scan = mScanMatrix;
    const afmv_dcttab* tab = nullptr;
    unsigned short bits = 0;
    unsigned int shifted = 0;
    int pos = 0;
    int mismatch = ~*dctDest;
    int value = 0;

    if (bitCount > 0) {
        bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
        decodePnt += 2;
        bitPtr = decodePnt;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }

decode_loop:
    if (shifter >= 0x04000000u) {
        tab = &afmv_dct_b15_8[(shifter >> 24) - 4];
        pos += tab->run;
        if (pos < 64)
            goto normal_code;
        pos = pos + ((shifter >> 20) & 0x3F) - 64;
        if (pos >= 64)
            goto done;
        {
            int escapeBitCount = bitCount + 12;
            unsigned int escapeShifter = shifter << 12;
            if (escapeBitCount > 0) {
                bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
                bitPtr = decodePnt + 2;
                escapeShifter |= static_cast<unsigned int>(bits) << escapeBitCount;
                escapeBitCount -= 16;
            }
            value = 16 * ((escapeShifter >> 20) * quant_matrix[scan[pos]] / 16);
            if (value != value)
                value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
            dctDest[scan[pos]] = static_cast<short>(value);
            shifter = escapeShifter << 12;
            mismatch ^= value;
            bitCount = escapeBitCount + 12;
        }
        goto refill;
    }
    if (shifter >= 0x02000000u) {
        tab = &afmv_dct_b15_10[(shifter >> 22) - 8];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00800000u) {
        tab = &afmv_dct_13[(shifter >> 19) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00200000u) {
        tab = &afmv_dct_15[(shifter >> 17) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
    tab = &afmv_dct_16[shifter >> 16];
    shifter = (static_cast<unsigned int>(bits) << (bitCount + 16)) | (shifter << 16);
    bitPtr += 2;
    pos += tab->run;
    if (pos < 64)
        goto normal_code;
    decodePnt = bitPtr;
    goto done;

normal_code:
    shifted = shifter << tab->len;
    bitCount += tab->len + 1;
    value = static_cast<int>(((shifted >> 27) ^ (tab->level * quant_matrix[scan[pos]])) & 0xFFFFFFF0u)
        - 16 * static_cast<int>(shifted >> 31);
    if (value != value)
        value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
    dctDest[scan[pos]] = static_cast<short>(value);
    mismatch ^= value;
    shifter = 2 * shifted;

refill:
    if (bitCount > 0) {
        bits = static_cast<unsigned short>((bitPtr[0] << 8) | bitPtr[1]);
        bitPtr += 2;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }
    decodePnt = bitPtr;
    goto decode_loop;

done:
    dctDest[63] ^= static_cast<short>(mismatch & 0x10);
    mBitCount = bitCount + 4;
    mShifter = 16 * shifter;
    mDecodePnt = decodePnt;
}

int nvlAFMVMovie::GetNonIntraCoef(const unsigned short* quant_matrix) {
    unsigned int shifter = mShifter;
    int bitCount = mBitCount;
    unsigned char* decodePnt = mDecodePnt;
    unsigned char* bitPtr = decodePnt;
    short* dctDest = mDCTblock;
    const unsigned char* scan = mScanMatrix;
    const afmv_dcttab* tab = nullptr;
    unsigned short bits = 0;
    unsigned int shifted = 0;
    int pos = -1;
    int mismatch = -1;
    int value = 0;
    int j = 0;

    if (bitCount > 0) {
        bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
        decodePnt += 2;
        bitPtr = decodePnt;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }
    if (shifter >= 0x28000000u) {
        tab = &afmv_dct_b14dc_5[(shifter >> 27) - 5];
        goto entry_1;
    }

entry_2:
    if (shifter < 0x04000000u)
        goto after_inner;
    tab = &afmv_dct_b14_8[(shifter >> 24) - 4];
    pos += tab->run;
    if (pos < 64)
        goto normal_code;
    pos = pos + ((shifter >> 20) & 0x3F) - 64;
    if (pos >= 64)
        goto done;
    {
        int escapeBitCount = bitCount + 12;
        unsigned int escapeShifter = shifter << 12;
        if (escapeBitCount > 0) {
            bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
            bitPtr = decodePnt + 2;
            escapeShifter |= static_cast<unsigned int>(bits) << escapeBitCount;
            escapeBitCount -= 16;
        }
        value = 16 * (quant_matrix[scan[pos]] * (2 * ((escapeShifter >> 20) + (escapeShifter >> 31)) + 1) / 32);
        if (value != value)
            value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
        dctDest[scan[pos]] = static_cast<short>(value);
        shifter = escapeShifter << 12;
        mismatch ^= value;
        bitCount = escapeBitCount + 12;
    }
    goto inner_refill;

after_inner:
    if (shifter >= 0x02000000u) {
        tab = &afmv_dct_b14_10[(shifter >> 22) - 8];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00800000u) {
        tab = &afmv_dct_13[(shifter >> 19) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    if (shifter >= 0x00200000u) {
        tab = &afmv_dct_15[(shifter >> 17) - 16];
        pos += tab->run;
        if (pos >= 64)
            goto done;
        goto normal_code;
    }
    bits = static_cast<unsigned short>((decodePnt[0] << 8) | decodePnt[1]);
    tab = &afmv_dct_16[shifter >> 16];
    shifter = (static_cast<unsigned int>(bits) << (bitCount + 16)) | (shifter << 16);
    bitPtr += 2;
    pos += tab->run;
    if (pos < 64)
        goto normal_code;
    decodePnt = bitPtr;
    goto done;

entry_1:
    pos += tab->run;
    if (pos >= 64)
        goto done;
    goto normal_code;

normal_code:
    j = scan[pos];
    shifted = shifter << tab->len;
    bitCount += tab->len + 1;
    value = static_cast<int>(((((shifted >> 26) ^ (quant_matrix[j] * (2 * tab->level + 1))) >> 1) & 0xFFFFFFF0u))
        - 16 * static_cast<int>(shifted >> 31);
    if (value != value)
        value = (value >> 27) & 0xFFFFFFF0 ^ 0x7FF0;
    dctDest[j] = static_cast<short>(value);
    mismatch ^= value;
    shifter = 2 * shifted;

inner_refill:
    if (bitCount > 0) {
        bits = static_cast<unsigned short>((bitPtr[0] << 8) | bitPtr[1]);
        bitPtr += 2;
        shifter |= static_cast<unsigned int>(bits) << bitCount;
        bitCount -= 16;
    }
    decodePnt = bitPtr;
    if (shifter >= 0x28000000u) {
        tab = &afmv_dct_b14ac_5[(shifter >> 27) - 5];
        goto entry_1;
    }
    goto entry_2;

done:
    dctDest[63] ^= static_cast<short>(mismatch & 0x10);
    mBitCount = bitCount + 2;
    mShifter = 4 * shifter;
    mDecodePnt = decodePnt;
    return pos;
}
void nvlAFMVMovie::DoMotionFrame(
    afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int)) {
    int bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int motionX = SignExtendVector(
        motion->pmv[0][0] + GetMotionDiff(motion->f_code[0]), motion->f_code[0]);
    motion->pmv[0][0] = motionX;
    motion->pmv[1][0] = motionX;
    bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int motionY = SignExtendVector(
        motion->pmv[0][1] + GetMotionDiff(motion->f_code[1]), motion->f_code[1]);
    motion->pmv[0][1] = motionY;
    motion->pmv[1][1] = motionY;
    const int baseY = 2 * mVertOffset;
    const int baseX = 2 * mHorzOffset;
    int x = motionX + baseX;
    const int limitX = mLimitX;
    int adjustedX = motionX;
    int adjustedY = motionY;
    int y = baseY + motionY;
    if (x > limitX) {
        x = x < 0 ? 0 : limitX;
        adjustedX = x - baseX;
    }
    if (y > mLimitY16) {
        y = y < 0 ? 0 : mLimitY16;
        adjustedY = y - baseY;
    }
    motionFunc[x & 1 | (2 * (y & 1))](
        &mDest[0][mHorzOffset], &motion->ref[0][0][(x >> 1) + mStride * (y >> 1)], mStride, 16);
    const int motionXa = adjustedX / 2;
    const int uvOffset = ((mHorzOffset + motionXa) >> 1) +
        mUVStride * ((adjustedY / 2 + mVertOffset) >> 1);
    const int uvMode = motionXa & 1 | (2 * ((adjustedY / 2) & 1));
    motionFunc[uvMode + 4](&mDest[1][mHorzOffset >> 1],
                            &motion->ref[0][1][uvOffset], mUVStride, 8);
    motionFunc[uvMode + 4](&mDest[2][mHorzOffset >> 1],
                            &motion->ref[0][2][uvOffset], mUVStride, 8);
}
void nvlAFMVMovie::DoMotionField(
    afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int)) {
    int bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const unsigned field0 = mShifter >> 31;
    mShifter <<= 1;
    ++mBitCount;
    const int motionX0 = SignExtendVector(
        motion->pmv[0][0] + GetMotionDiff(motion->f_code[0]), motion->f_code[0]);
    motion->pmv[0][0] = motionX0;
    bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int motionY0 = (motion->pmv[0][1] >> 1) + GetMotionDiff(motion->f_code[1]);
    motion->pmv[0][1] = 2 * motionY0;
    const int baseX = 2 * mHorzOffset;
    int x = baseX + motionX0;
    int adjustedX = motionX0;
    int y = mVertOffset + motionY0;
    int adjustedY = motionY0;
    if (x > mLimitX) {
        x = x < 0 ? 0 : mLimitX;
        adjustedX = x - baseX;
    }
    if (y > mLimitY) {
        y = y < 0 ? 0 : mLimitY;
        adjustedY = y - mVertOffset;
    }
    motionFunc[x & 1 | (2 * (y & 1))](
        &mDest[0][mHorzOffset],
        &motion->ref[0][0][(x >> 1) + mStride * (field0 + (y & 0xFFFFFFFE))],
        2 * mStride, 8);
    const int motionXb = adjustedX / 2;
    const int motionYb = adjustedY / 2;
    const int uvMode = motionXb & 1 | (2 * (motionYb & 1));
    const int uvOffset = ((mHorzOffset + motionXb) >> 1) +
        mUVStride * (field0 + (mVertOffset >> 1) + (motionYb & 0xFFFFFFFE));
    motionFunc[uvMode + 4](&mDest[1][mHorzOffset >> 1],
                            &motion->ref[0][1][uvOffset], 2 * mUVStride, 4);
    motionFunc[uvMode + 4](&mDest[2][mHorzOffset >> 1],
                            &motion->ref[0][2][uvOffset], 2 * mUVStride, 4);

    bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const unsigned field1 = mShifter >> 31;
    mShifter <<= 1;
    ++mBitCount;
    const int motionX1 = SignExtendVector(
        motion->pmv[1][0] + GetMotionDiff(motion->f_code[0]), motion->f_code[0]);
    motion->pmv[1][0] = motionX1;
    bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int motionY1 = (motion->pmv[1][1] >> 1) + GetMotionDiff(motion->f_code[1]);
    motion->pmv[1][1] = 2 * motionY1;
    const int baseX1 = 2 * mHorzOffset;
    int x1 = baseX1 + motionX1;
    int adjustedX1 = motionX1;
    int y1 = mVertOffset + motionY1;
    int adjustedY1 = motionY1;
    if (x1 > mLimitX) {
        x1 = x1 < 0 ? 0 : mLimitX;
        adjustedX1 = x1 - baseX1;
    }
    if (y1 > mLimitY) {
        y1 = y1 < 0 ? 0 : mLimitY;
        adjustedY1 = y1 - mVertOffset;
    }
    motionFunc[x1 & 1 | (2 * (y1 & 1))](
        &mDest[0][mHorzOffset + mStride],
        &motion->ref[0][0][(x1 >> 1) + mStride * (field1 + (y1 & 0xFFFFFFFE))],
        2 * mStride, 8);
    const int motionXd = adjustedX1 / 2;
    const int motionYd = adjustedY1 / 2;
    const int uvMode1 = motionXd & 1 | (2 * (motionYd & 1));
    const int uvOffset1 = ((mHorzOffset + motionXd) >> 1) +
        mUVStride * (field1 + (mVertOffset >> 1) + (motionYd & 0xFFFFFFFE));
    motionFunc[uvMode1 + 4](&mDest[1][mUVStride + (mHorzOffset >> 1)],
                            &motion->ref[0][1][uvOffset1], 2 * mUVStride, 4);
    motionFunc[uvMode1 + 4](&mDest[2][mUVStride + (mHorzOffset >> 1)],
                            &motion->ref[0][2][uvOffset1], 2 * mUVStride, 4);
}
void nvlAFMVMovie::DoMotionDualP(
    afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int)) {
    (void)motionFunc;
    int bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int motionX = SignExtendVector(
        motion->pmv[0][0] + GetMotionDiff(motion->f_code[0]), motion->f_code[0]);
    motion->pmv[0][0] = motionX;
    motion->pmv[1][0] = motionX;
    bitCount = mBitCount;
    if (bitCount > 0) {
        const unsigned short bits = static_cast<unsigned short>((mDecodePnt[0] << 8) | mDecodePnt[1]);
        mDecodePnt += 2;
        mShifter |= static_cast<unsigned int>(bits) << bitCount;
        mBitCount = bitCount - 16;
    }
    const int dmvX = GetDMV();
    const int motionY = (motion->pmv[0][1] >> 1) + GetMotionDiff(motion->f_code[1]);
    motion->pmv[0][1] = 2 * motionY;
    motion->pmv[1][1] = 2 * motionY;
    const int dmvY = GetDMV();
    const int motionXPositive = motionX > 0;
    const int motionYPositive = motionY > 0;
    int otherX = dmvX + ((motionX + motionXPositive + 2 * motionX) >> 1);
    int offset = ((motionY + motionYPositive + 2 * motionY) >> 1) + dmvY - 1;
    const int baseX = 2 * mHorzOffset;
    int x = baseX + otherX;
    int y = offset + mVertOffset;
    if (x > mLimitX) {
        x = x < 0 ? 0 : mLimitX;
        otherX = x - baseX;
    }
    if (y > mLimitY) {
        y = y < 0 ? 0 : mLimitY;
        offset = y - mVertOffset;
    }
    afmv_mc.put[x & 1 | (2 * (y & 1))](
        &mDest[0][mHorzOffset], &motion->ref[0][0][(x >> 1) + mStride * (y | 1)],
        2 * mStride, 8);
    const int uvBase = mUVStride * ((offset / 2 | 1) + (mVertOffset >> 1));
    const int uvMode = (otherX / 2) & 1 | (2 * ((offset / 2) & 1));
    const int uvOffset = ((mHorzOffset + otherX / 2) >> 1) + uvBase;
    afmv_mc.avg[uvMode - 4](&mDest[1][mHorzOffset >> 1],
                            &motion->ref[0][1][uvOffset], 2 * mUVStride, 4);
    afmv_mc.avg[uvMode - 4](&mDest[2][mHorzOffset >> 1],
                            &motion->ref[0][2][uvOffset], 2 * mUVStride, 4);

    const int secondX = dmvX + ((motionXPositive + motionX) >> 1);
    int secondXAbs = baseX + secondX;
    int secondY = ((motionY + motionYPositive) >> 1) + dmvY + 1 + mVertOffset;
    int secondOffset = secondY - mVertOffset;
    if (secondXAbs > mLimitX) {
        secondXAbs = secondXAbs < 0 ? 0 : mLimitX;
        otherX = secondXAbs;
    }
    if (secondY > mLimitY) {
        secondY = secondY < 0 ? 0 : mLimitY;
        secondOffset = secondY - mVertOffset;
    }
    afmv_mc.put[secondXAbs & 1 | (2 * (secondY & 1))](
        &mDest[0][mHorzOffset + mStride],
        &motion->ref[0][0][(otherX >> 1) + mStride * (secondY & 0xFFFFFFFE)],
        2 * mStride, 8);
    const int secondUvMode = (secondX / 2) & 1 | (2 * ((secondOffset / 2) & 1));
    const int secondUvOffset = ((mHorzOffset + secondX / 2) >> 1) +
        mUVStride * ((secondOffset / 2 & 0xFFFFFFFE) + (mVertOffset >> 1));
    afmv_mc.avg[secondUvMode - 4](
        &mDest[1][mUVStride + (mHorzOffset >> 1)],
        &motion->ref[0][1][secondUvOffset], 2 * mUVStride, 4);
    afmv_mc.avg[secondUvMode - 4](
        &mDest[2][mUVStride + (mHorzOffset >> 1)],
        &motion->ref[0][2][secondUvOffset], 2 * mUVStride, 4);

    int averageX = baseX + motionX;
    int averageY = motionY + mVertOffset;
    if (averageX > mLimitX)
        averageX = averageX < 0 ? 0 : mLimitX;
    if (averageY > mLimitY)
        averageY = averageY < 0 ? 0 : mLimitY;
    const int averageMode = averageX & 1 | (2 * (averageY & 1));
    const int averageOffset = (averageX >> 1) + mStride * (averageY & 0xFFFFFFFE);
    afmv_mc.avg[averageMode](&mDest[0][mHorzOffset],
                             &motion->ref[0][0][averageOffset], 2 * mStride, 8);
    afmv_mc.avg[averageMode](&mDest[0][mStride + mHorzOffset],
                             &motion->ref[0][0][mStride + averageOffset], 2 * mStride, 8);
    const int averageXHalf = motionX / 2;
    const int averageUvOffset = ((mHorzOffset + averageXHalf) >> 1) +
        mUVStride * ((motionY / 2 & 0xFFFFFFFE) + (mVertOffset >> 1));
    const int averageUvMode = averageXHalf & 1 | (2 * ((motionY / 2) & 1));
    afmv_mc.avg[averageUvMode + 4](&mDest[1][mHorzOffset >> 1],
                                   &motion->ref[0][1][averageUvOffset], 2 * mUVStride, 4);
    afmv_mc.avg[averageUvMode + 4](&mDest[1][mUVStride + (mHorzOffset >> 1)],
                                   &motion->ref[0][1][mUVStride + averageUvOffset], 2 * mUVStride, 4);
    afmv_mc.avg[averageUvMode + 4](&mDest[2][mHorzOffset >> 1],
                                   &motion->ref[0][2][averageUvOffset], 2 * mUVStride, 4);
    afmv_mc.avg[averageUvMode + 4](&mDest[2][mUVStride + (mHorzOffset >> 1)],
                                   &motion->ref[0][2][mUVStride + averageUvOffset], 2 * mUVStride, 4);
}
void nvlAFMVMovie::DoMotionSame(
    afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int)) {
    const int limitX = mLimitX;
    const int horzOffset = 2 * mHorzOffset;
    const int vertOffset = 2 * mVertOffset;
    int motionX = motion->pmv[0][0];
    int motionY = motion->pmv[0][1];
    int x = horzOffset + motionX;
    int y = vertOffset + motionY;
    if (x > limitX) {
        x = x < 0 ? 0 : limitX;
        motionX = x - horzOffset;
    }
    if (y > mLimitY16) {
        y = y < 0 ? 0 : mLimitY16;
        motionY = y - vertOffset;
    }
    motionFunc[motionX & 1 | (2 * (y & 1))](
        &mDest[0][mHorzOffset],
        &motion->ref[0][0][(x >> 1) + mStride * (y >> 1)],
        mStride, 16);
    motionY /= 2;
    const int uvMode = (motionX / 2) & 1 | (2 * (motionY & 1));
    const int uvOffset = ((mHorzOffset + motionX / 2) >> 1) +
        mUVStride * ((motionY + mVertOffset) >> 1);
    motionFunc[uvMode + 4](&mDest[1][mHorzOffset >> 1],
                            &motion->ref[0][1][uvOffset], mUVStride, 8);
    motionFunc[uvMode + 4](&mDest[2][mHorzOffset >> 1],
                            &motion->ref[0][2][uvOffset], mUVStride, 8);
}
void nvlAFMVMovie::DoMotionCopy(
    afmv_motion_t* motion, void (*const* motionFunc)(unsigned char*, const unsigned char*, int, int)) {
    motion->pmv[0][0] = 0;
    motion->pmv[0][1] = 0;
    motion->pmv[1][0] = 0;
    motion->pmv[1][1] = 0;
    (*motionFunc)(&mDest[0][mHorzOffset],
                  &motion->ref[0][0][mStride * mVertOffset + mHorzOffset], mStride, 16);
    const int uvOffset = (mHorzOffset >> 1) + mUVStride * (mVertOffset >> 1);
    motionFunc[4](&mDest[1][mHorzOffset >> 1], &motion->ref[0][1][uvOffset], mUVStride, 8);
    motionFunc[4](&mDest[2][mHorzOffset >> 1], &motion->ref[0][2][uvOffset], mUVStride, 8);
}
int nvlAFMVMovie::DecodeSlice() { return 0; }

void nvlMovie::ProcessAudioChunk() {}
void nvlMovie::StartAudioPlayback() {}
void nvlMovie::StopAudioPlayback() {}
nvlMovie::~nvlMovie() = default;

bool nvlInit() { return true; }
void nvlShutdown() { tlMemFree(nvlAFMVMovie::mAudioData); }

static __m64 afmv_idct_table(const std::int16_t* table, unsigned offset) {
    return afmv_load64(reinterpret_cast<const unsigned char*>(table + offset));
}

static void afmv_idct_first_pair(__m64& first, __m64& second, const std::int16_t* table,
                                 __m64 roundRow) {
    const __m64 evenFirst = _m_pshufw(first, 136);
    const __m64 evenSecond = _m_pshufw(second, 136);
    const __m64 oddFirst = _m_pshufw(first, 221);
    const __m64 oddSecond = _m_pshufw(second, 221);
    const __m64 v7 = _m_paddd(_m_paddd(_m_pmaddwd(afmv_idct_table(table, 0), evenFirst), roundRow),
                              _m_pmaddwd(afmv_idct_table(table, 4), evenSecond));
    const __m64 v8 = _m_paddd(_m_pmaddwd(afmv_idct_table(table, 16), oddFirst),
                              _m_pmaddwd(afmv_idct_table(table, 20), oddSecond));
    const __m64 v9 = _m_paddd(_m_paddd(_m_pmaddwd(evenFirst, afmv_idct_table(table, 8)), roundRow),
                              _m_pmaddwd(evenSecond, afmv_idct_table(table, 12)));
    const __m64 v10 = _m_paddd(_m_pmaddwd(oddFirst, afmv_idct_table(table, 24)),
                               _m_pmaddwd(oddSecond, afmv_idct_table(table, 28)));
    first = _m_packssdw(_m_psradi(_m_paddd(v7, v8), 15), _m_psradi(_m_paddd(v9, v10), 15));
    second = _m_pshufw(_m_packssdw(_m_psradi(_m_psubd(v9, v10), 15),
                                   _m_psradi(_m_psubd(v7, v8), 15)), 177);
}

static void afmv_macro_block_copy(unsigned char* dest, int stride, const __m64* block) {
    for (unsigned row = 0; row < 8; ++row)
        afmv_store64(dest + row * stride, _m_packuswb(block[row * 2], block[row * 2 + 1]));
}

static void afmv_macro_block_add(unsigned char* dest, int stride, const __m64* block) {
    const __m64 zero = afmv_from_bits(0);
    for (unsigned row = 0; row < 8; ++row) {
        const __m64 current = afmv_load64(dest + row * stride);
        const __m64 low = _m_paddsw(_m_punpcklbw(current, zero), block[row * 2]);
        const __m64 high = _m_paddsw(_m_punpckhbw(current, zero), block[row * 2 + 1]);
        afmv_store64(dest + row * stride, _m_packuswb(low, high));
    }
}

static void afmv_macro_block_add_dc(unsigned char* dest, int stride, const short* block) {
    const int dc = (static_cast<int>(block[0]) + 64) >> 7;
    const __m64 dcWords = afmv_from_bits(static_cast<std::uint32_t>(dc));
    const __m64 dcWordsLo = _m_punpcklwd(dcWords, dcWords);
    const __m64 dcWordsAll = _m_punpckldq(dcWordsLo, dcWordsLo);
    const __m64 negative = _m_psubsw(afmv_from_bits(0), dcWordsAll);
    const __m64 positiveBytes = _m_packuswb(dcWordsAll, dcWordsAll);
    const __m64 negativeBytes = _m_packuswb(negative, negative);
    for (unsigned row = 0; row < 8; ++row) {
        const __m64 current = afmv_load64(dest + row * stride);
        afmv_store64(dest + row * stride,
                     _m_psubusb(_m_paddusb(current, positiveBytes), negativeBytes));
    }
}

void MacroBlockIdct(short* mb) {
    __m64* block = reinterpret_cast<__m64*>(mb);
    const __m64 roundRow = afmv_idct_table(round_inv_row, 0);
    const std::int16_t* tables[8] = {
        tab_i_04, tab_i_17, tab_i_26, tab_i_35,
        tab_i_04, tab_i_35, tab_i_26, tab_i_17
    };
    for (unsigned pair = 0; pair < 8; ++pair)
        afmv_idct_first_pair(block[pair * 2], block[pair * 2 + 1], tables[pair], roundRow);

    const __m64 one = afmv_idct_table(one_corr, 0);
    const __m64 tg1 = afmv_idct_table(tg_1_16, 0);
    const __m64 tg2 = afmv_idct_table(tg_2_16, 0);
    const __m64 tg3 = afmv_idct_table(tg_3_16, 0);
    const __m64 cos4 = afmv_idct_table(cos_4_16, 0);
    const __m64 roundCol = afmv_idct_table(round_inv_col, 0);
    const __m64 roundCorr = afmv_idct_table(round_inv_corr, 0);

    {
        const __m64 v95 = block[10];
        const __m64 v96 = block[6];
        const __m64 v97 = block[14];
        const __m64 v98 = _m_psubsw(v95, _m_paddsw(_m_pmulhw(tg3, v96), v96));
        const __m64 v99 = _m_paddsw(_m_paddsw(_m_pmulhw(v95, tg3), v95), v96);
        const __m64 v100 = _m_paddsw(_m_pmulhw(v97, tg1), block[2]);
        const __m64 v101 = _m_paddsw(_m_pmulhw(block[12], tg2), block[4]);
        const __m64 v102 = _m_psubsw(_m_pmulhw(tg1, block[2]), v97);
        const __m64 v103 = _m_psubsw(_m_pmulhw(tg2, block[4]), block[12]);
        const __m64 v104 = _m_paddsw(_m_psubsw(v102, v98), one);
        const __m64 v105 = _m_paddsw(v102, v98);
        block[14] = _m_paddsw(_m_paddsw(v99, v100), one);
        const __m64 v106 = _m_psubsw(v100, v99);
        const __m64 v107 = _m_paddsw(v106, v104);
        const __m64 v109 = _m_pmulhw(cos4, v107);
        block[6] = v105;
        const __m64 v110 = _m_psubsw(v106, v104);
        const __m64 v111 = _m_por(_m_paddsw(v107, v109), one);
        const __m64 v112 = _m_por(_m_paddsw(_m_pmulhw(cos4, v110), v110), one);
        const __m64 v113 = _m_paddsw(block[8], block[0]);
        const __m64 v114 = _m_psubsw(block[0], block[8]);
        const __m64 v115 = _m_paddsw(_m_paddsw(v113, v101), roundCol);
        const __m64 v116 = _m_paddsw(_m_paddsw(v114, v103), roundCol);
        const __m64 v117 = _m_psubsw(v114, v103);
        const __m64 v118 = _m_paddsw(_m_psubsw(v113, v101), roundCorr);
        const __m64 v120 = _m_paddsw(v117, roundCorr);
        block[0] = _m_psrawi(_m_paddsw(block[14], v115), 6);
        block[2] = _m_psrawi(_m_paddsw(v116, v111), 6);
        const __m64 v121 = _m_paddsw(block[6], v118);
        const __m64 v122 = _m_psubsw(v118, block[6]);
        block[4] = _m_psrawi(_m_paddsw(v120, v112), 6);
        const __m64 v123 = _m_psubsw(v115, block[14]);
        block[6] = _m_psrawi(v121, 6);
        block[8] = _m_psrawi(v122, 6);
        block[10] = _m_psrawi(_m_psubsw(v120, v112), 6);
        block[12] = _m_psrawi(_m_psubsw(v116, v111), 6);
        block[14] = _m_psrawi(v123, 6);
    }
    {
        const __m64 v124 = block[11];
        const __m64 v125 = block[7];
        const __m64 v126 = block[15];
        const __m64 v127 = _m_paddsw(_m_paddsw(_m_pmulhw(v124, tg3), v124), v125);
        const __m64 v128 = _m_psubsw(v124, _m_paddsw(_m_pmulhw(tg3, v125), v125));
        const __m64 v129 = _m_paddsw(_m_pmulhw(v126, tg1), block[3]);
        const __m64 v130 = _m_paddsw(_m_pmulhw(block[13], tg2), block[5]);
        const __m64 v131 = _m_psubsw(_m_pmulhw(tg1, block[3]), v126);
        const __m64 v132 = _m_psubsw(_m_pmulhw(tg2, block[5]), block[13]);
        const __m64 v133 = _m_paddsw(_m_psubsw(v131, v128), one);
        const __m64 v134 = _m_paddsw(v131, v128);
        block[15] = _m_paddsw(_m_paddsw(v127, v129), one);
        const __m64 v135 = _m_psubsw(v129, v127);
        const __m64 v136 = _m_paddsw(v135, v133);
        const __m64 v138 = _m_pmulhw(cos4, v136);
        block[7] = v134;
        const __m64 v139 = _m_psubsw(v135, v133);
        const __m64 v140 = _m_por(_m_paddsw(v136, v138), one);
        const __m64 v141 = _m_por(_m_paddsw(_m_pmulhw(cos4, v139), v139), one);
        const __m64 v142 = _m_paddsw(block[9], block[1]);
        const __m64 v143 = _m_psubsw(block[1], block[9]);
        const __m64 v144 = _m_paddsw(_m_paddsw(v142, v130), roundCol);
        const __m64 v145 = _m_paddsw(_m_paddsw(v143, v132), roundCol);
        const __m64 v146 = _m_psubsw(v143, v132);
        const __m64 v147 = _m_paddsw(_m_psubsw(v142, v130), roundCorr);
        const __m64 v149 = _m_paddsw(v146, roundCorr);
        block[1] = _m_psrawi(_m_paddsw(block[15], v144), 6);
        block[3] = _m_psrawi(_m_paddsw(v145, v140), 6);
        const __m64 v150 = _m_paddsw(block[7], v147);
        const __m64 v151 = _m_psubsw(v147, block[7]);
        block[5] = _m_psrawi(_m_paddsw(v149, v141), 6);
        const __m64 v152 = _m_psubsw(v144, block[15]);
        block[7] = _m_psrawi(v150, 6);
        block[9] = _m_psrawi(v151, 6);
        block[11] = _m_psrawi(_m_psubsw(v149, v141), 6);
        block[13] = _m_psrawi(_m_psubsw(v145, v140), 6);
        block[15] = _m_psrawi(v152, 6);
    }
}

void MacroBlockIdctCopy(short* mb, unsigned char* dest, int stride) {
    MacroBlockIdct(mb);
    afmv_macro_block_copy(dest, stride, reinterpret_cast<const __m64*>(mb));
    std::memset(mb, 0, sizeof(__m64) * 16);
}

void MacroBlockIdctAdd(int last, short* mb, unsigned char* dest, int stride) {
    if (last != 129 || (static_cast<unsigned char>(mb[0]) & 0x70) == 0x40) {
        MacroBlockIdct(mb);
        afmv_macro_block_add(dest, stride, reinterpret_cast<const __m64*>(mb));
    } else {
        afmv_macro_block_add_dc(dest, stride, mb);
    }
    std::memset(mb, 0, sizeof(__m64) * 16);
}

static const __m64 q_UVMask = { 0x00ff00ff00ff00ffULL };
static const __m64 q_UVBias = { 0x0080008000800080ULL };
static const __m64 q_Y_Bias = { 0x1010101010101010ULL };
static const __m64 q_Y_Gain = { 0x253f253f253f253fULL };
static const __m64 q_V_to_R = { 0x3312331233123312ULL };
static const __m64 q_V_to_G = { 0xe5fce5fce5fce5fcULL };
static const __m64 q_U_to_G = { 0xf37df37df37df37dULL };
static const __m64 q_U_to_B = { 0x4093409340934093ULL };
static const __m64 afmv_zero = { 0 };

static __m64 afmv_load_u32(const unsigned char* p) {
    std::uint32_t value = 0;
    std::memcpy(&value, p, sizeof(value));
    return afmv_from_bits(value);
}

static void afmv_yuv_to_rgb(const unsigned char* py, const unsigned char* pu,
                            const unsigned char* pv, __m64& red, __m64& green,
                            __m64& blue) {
    __m64 u = _m_punpcklbw(afmv_load_u32(pu), afmv_zero);
    __m64 v = _m_punpcklbw(afmv_load_u32(pv), afmv_zero);
    u = _m_psubsw(u, q_UVBias);
    v = _m_psubsw(v, q_UVBias);
    u = _m_psllwi(u, 3);
    v = _m_psllwi(v, 3);

    __m64 greenBase = _m_pmulhw(u, q_U_to_G);
    const __m64 vGreen = _m_pmulhw(v, q_V_to_G);
    red = _m_pmulhw(v, q_V_to_R);
    greenBase = _m_paddsw(greenBase, vGreen);
    blue = _m_pmulhw(u, q_U_to_B);

    const __m64 y = _m_psubusb(afmv_load64(py), q_Y_Bias);
    __m64 yEven = _m_pand(y, q_UVMask);
    __m64 yOdd = _m_psrlwi(y, 8);
    yEven = _m_psllwi(yEven, 3);
    yOdd = _m_psllwi(yOdd, 3);
    yEven = _m_pmulhw(yEven, q_Y_Gain);
    yOdd = _m_pmulhw(yOdd, q_Y_Gain);

    const __m64 redLow = _m_packuswb(_m_paddsw(red, yEven), _m_paddsw(red, yEven));
    const __m64 redHigh = _m_packuswb(_m_paddsw(red, yOdd), _m_paddsw(red, yOdd));
    const __m64 greenLow = _m_packuswb(_m_paddsw(greenBase, yEven), _m_paddsw(greenBase, yEven));
    const __m64 greenHigh = _m_packuswb(_m_paddsw(greenBase, yOdd), _m_paddsw(greenBase, yOdd));
    const __m64 blueLow = _m_packuswb(_m_paddsw(blue, yEven), _m_paddsw(blue, yEven));
    const __m64 blueHigh = _m_packuswb(_m_paddsw(blue, yOdd), _m_paddsw(blue, yOdd));
    red = _m_punpcklbw(redLow, redHigh);
    green = _m_punpcklbw(greenLow, greenHigh);
    blue = _m_punpcklbw(blueLow, blueHigh);
}

static unsigned afmv_byte(__m64 value, unsigned index) {
    return static_cast<unsigned>((afmv_to_bits(value) >> (index * 8)) & 0xFFu);
}

void afmvYUV2RGB16(unsigned char** src_yuv, unsigned char* dest_rgb, int width) {
    unsigned char* y = src_yuv[0];
    unsigned char* u = src_yuv[1];
    unsigned char* v = src_yuv[2];
    const int chromaWidth = width >> 1;
    const int blocks = width >> 3;
    for (int rowPair = 0; rowPair < 8; ++rowPair) {
        for (int block = 0; block < blocks; ++block) {
            __m64 red, green, blue;
            afmv_yuv_to_rgb(y, u, v, red, green, blue);
            for (unsigned pixel = 0; pixel < 8; ++pixel) {
                const unsigned r = afmv_byte(red, pixel) & 0xF8u;
                const unsigned g = afmv_byte(green, pixel) & 0xFCu;
                const unsigned b = afmv_byte(blue, pixel) & 0xF8u;
                const std::uint16_t packed = static_cast<std::uint16_t>((b >> 3) | (g << 3) | (r << 8));
                std::memcpy(dest_rgb + pixel * 2, &packed, sizeof(packed));
            }
            y += 8;
            u += 4;
            v += 4;
            dest_rgb += 16;
        }
        u -= chromaWidth;
        v -= chromaWidth;
        for (int block = 0; block < blocks; ++block) {
            __m64 red, green, blue;
            afmv_yuv_to_rgb(y, u, v, red, green, blue);
            for (unsigned pixel = 0; pixel < 8; ++pixel) {
                const unsigned r = afmv_byte(red, pixel) & 0xF8u;
                const unsigned g = afmv_byte(green, pixel) & 0xFCu;
                const unsigned b = afmv_byte(blue, pixel) & 0xF8u;
                const std::uint16_t packed = static_cast<std::uint16_t>((b >> 3) | (g << 3) | (r << 8));
                std::memcpy(dest_rgb + pixel * 2, &packed, sizeof(packed));
            }
            y += 8;
            u += 4;
            v += 4;
            dest_rgb += 16;
        }
    }
}

void afmvYUV2RGB32(unsigned char** src_yuv, unsigned char* dest_rgb, int width) {
    unsigned char* y = src_yuv[0];
    unsigned char* u = src_yuv[1];
    unsigned char* v = src_yuv[2];
    const int chromaWidth = width >> 1;
    const int blocks = width >> 3;
    for (int rowPair = 0; rowPair < 8; ++rowPair) {
        for (int block = 0; block < blocks; ++block) {
            __m64 red, green, blue;
            afmv_yuv_to_rgb(y, u, v, red, green, blue);
            for (unsigned pixel = 0; pixel < 8; ++pixel) {
                dest_rgb[pixel * 4 + 0] = static_cast<unsigned char>(afmv_byte(blue, pixel));
                dest_rgb[pixel * 4 + 1] = static_cast<unsigned char>(afmv_byte(green, pixel));
                dest_rgb[pixel * 4 + 2] = static_cast<unsigned char>(afmv_byte(red, pixel));
                dest_rgb[pixel * 4 + 3] = 0;
            }
            y += 8;
            u += 4;
            v += 4;
            dest_rgb += 32;
        }
        u -= chromaWidth;
        v -= chromaWidth;
        for (int block = 0; block < blocks; ++block) {
            __m64 red, green, blue;
            afmv_yuv_to_rgb(y, u, v, red, green, blue);
            for (unsigned pixel = 0; pixel < 8; ++pixel) {
                dest_rgb[pixel * 4 + 0] = static_cast<unsigned char>(afmv_byte(blue, pixel));
                dest_rgb[pixel * 4 + 1] = static_cast<unsigned char>(afmv_byte(green, pixel));
                dest_rgb[pixel * 4 + 2] = static_cast<unsigned char>(afmv_byte(red, pixel));
                dest_rgb[pixel * 4 + 3] = 0;
            }
            y += 8;
            u += 4;
            v += 4;
            dest_rgb += 32;
        }
    }
}

// ============================================================================
// NVL — NGL Video Library (FMV movie playback)
// From nvl_xboxr: nvl_base.o + nvl_xbox.o + nvl_afmv.o + afmv_dx_*.o
// ea: 0x82E100-0x833270 (47 funcs, 5 objects)
// Xbox-only AFMV codec — stubbed for Win32.
// ============================================================================

#include <cstdint>

// Forward
typedef unsigned nflFileID;
typedef unsigned nflRequestID;
typedef unsigned nflRequestState;
struct nglTexture;

enum nvlResult       { NVL_OK=0, NVL_ERR=1 };
enum nvlMovieState   { NVL_STOPPED=0, NVL_PLAYING=1, NVL_PAUSED=2 };
enum nvlFrameState   { NVL_FRAME_NONE=0, NVL_FRAME_READY=1, NVL_FRAME_END=2 };

// ============================================================================
// nvlMovie — AFMV movie decoder (Xbox hardware-accelerated, stubbed)
// ============================================================================

class nvlMovie {
public:
    void  ProcessAudioChunk() {}      // ea: 0x82E100
    void  StartAudioPlayback() {}     // ea: 0x82E1E0
    void  StopAudioPlayback() {}      // ea: 0x82E230
    ~nvlMovie() {}                    // ea: 0x82E3A0
};

bool nvlInit() { return true; }      // ea: 0x82E270
void nvlShutdown() {}                // ea: 0x82E390

// ============================================================================
// nvlMovieBase — abstract movie player interface
// ============================================================================

class nvlMovieBase {
public:
    nvlMovieBase() {}                        // ea: 0x82E730
    virtual ~nvlMovieBase() {}               // ea: 0x82E780
    virtual nvlResult     InitMovie()                 { return NVL_ERR; }  // ea: 0x82E790
    virtual nvlFrameState DecodeFrame()               { return NVL_FRAME_NONE; } // ea: 0x82E530
    virtual void          Reset()                     {} // ea: 0x82E590

    static unsigned     GetVersion()                  { return 0x100; }  // ea: 0x82E3F0
    static void         SetConvertNTSCtoPAL(bool)     {} // ea: 0x82E540
    nflFileID           GetFileID()                   { return 0; }     // ea: 0x82E400
    nflFileID           ReleaseMovie()                { return 0; }     // ea: 0x82E410
    nvlMovieState       SetMovieState(nvlMovieState s) { return s; }    // ea: 0x82E4C0
    nvlMovieState       GetMovieState()               { return NVL_STOPPED; } // ea: 0x82E4D0
    void                SetUserDataCallback(void (*cb)(void*,unsigned,bool,void*), void* ctx) {} // ea: 0x82E4E0
    int                 GetNumFrames()                { return 0; }     // ea: 0x82E500
    int                 GetFrameNumber()              { return 0; }     // ea: 0x82E510
    nglTexture*         GetTexture()                  { return nullptr; } // ea: 0x82E520
    nvlResult           SetAudioTrack(int t)          { return NVL_ERR; } // ea: 0x82E550
    bool                IsBufferLoading()             { return false; }  // ea: 0x82E560
    bool                BufferCheckBytes(int len, bool wait) { return false; } // ea: 0x82E970
    bool                IsBufferFull()                { return false; }  // ea: 0x82EB80
    void                RequestCallback(nflRequestState, nflRequestID) {} // ea: 0x82EBF0

protected:
    void CheckNextNFLRequest() {}        // ea: 0x82E610
};

// ============================================================================
// nvlAFMVMovie — AFMV format movie decoder (Xbox P3SIMD, stubbed)
// ============================================================================

struct afmv_motion_t;

typedef void (*afmv_motion_func_t)(char*, const char*, int, int);

class nvlAFMVMovie : public nvlMovieBase {
public:
    nvlAFMVMovie() {}                            // ea: 0x82ECE0
    ~nvlAFMVMovie() {}                           // ea: 0x82ED60
    nvlResult InitMovie() override { return NVL_ERR; } // ea: 0x82ED80
    nvlFrameState DecodeFrame() override { return NVL_FRAME_NONE; } // ea: 0x830FF0

    nvlResult ParseHeader(nflFileID, bool, int, int) { return NVL_ERR; } // ea: 0x82EDF0

private:
    void ProcessUserDataChunk(bool) {}           // ea: 0x82EFA0
    void GetIntraCoefB14(const unsigned short*) {} // ea: 0x82F010
    void GetIntraCoefB15(const unsigned short*) {} // ea: 0x82F2C0
    int  GetNonIntraCoef(const unsigned short*)   { return 0; } // ea: 0x82F540
    void DoMotionFrame(afmv_motion_t*, afmv_motion_func_t) {} // ea: 0x82F810
    void DoMotionField(afmv_motion_t*, afmv_motion_func_t) {} // ea: 0x82FA20
    void DoMotionDualP(afmv_motion_t*, afmv_motion_func_t) {} // ea: 0x82FE70
    void DoMotionSame(afmv_motion_t*,  afmv_motion_func_t) {} // ea: 0x830440
    void DoMotionCopy(afmv_motion_t*,  afmv_motion_func_t) {} // ea: 0x830580
    int  DecodeSlice() { return 0; }             // ea: 0x830630
};

// ============================================================================
// AFMV DCT IDCT (Xbox SIMD, stubbed)
// ============================================================================

void MacroBlockIdct(short* coeffs) {}               // ea: 0x832260
void MacroBlockIdctCopy(short* coeffs, char* dst, int stride) {} // ea: 0x832A10
void MacroBlockIdctAdd(int stride, short* coeffs, char* dst, int dstStride) {} // ea: 0x832A90

// ============================================================================
// AFMV YUV→RGB conversion (Xbox GPU, stubbed)
// ============================================================================

void afmvYUV2RGB16(char** planes, char* dst, int width) {} // ea: 0x833130
void afmvYUV2RGB32(char** planes, char* dst, int width) {} // ea: 0x833270

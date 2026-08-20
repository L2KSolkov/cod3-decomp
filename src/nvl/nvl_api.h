#pragma once

#include <cstdint>

struct nglTexture;
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
enum nvlResult : int { NVL_RESULT_ERROR = -1, NVL_RESULT_OK = 0 };
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

struct afmv_motion_t {
    unsigned char* ref[2][3];
    unsigned char** ref2[2];
    int pmv[2][2];
    int f_code[2];
};

class nvlMovieBase;
class nvlMovie;
void nvl_RequestCallback(nflRequestState reason, nflRequestID requestID,
                         nvlMovieBase* userData);

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
    void SetUserDataCallback(void (*callback)(unsigned char*, unsigned, bool,
                                               void*), void* user);
    int GetNumFrames();
    int GetFrameNumber();
    nglTexture* GetTexture();
    nvlResult SetAudioTrack(int audioTrackID);
    bool IsBufferLoading();
    bool BufferCheckBytes(int thisDataSize, bool update);
    bool IsBufferFull();
    void RequestCallback(nflRequestState reason, nflRequestID callreqID);

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

private:
    void CheckNextNFLRequest();
    static bool mConvertPal;
    friend class movie_manager;
};

class nvlAFMVMovie : public nvlMovieBase {
public:
    nvlAFMVMovie();
    ~nvlAFMVMovie() override;
    nvlResult InitMovie() override;
    nvlFrameState DecodeFrame() override;
    virtual void ProcessAudioChunk() = 0;
    virtual void StartAudioPlayback() = 0;
    virtual void StopAudioPlayback() = 0;
    nvlResult ParseHeader(nflFileID fileID, bool backBuffer, int offset,
                          int formal);

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
    void (*mMotionParser[5])(nvlMovie*, afmv_motion_t*,
                             void (*const)(unsigned char*, const unsigned char*,
                                           int, int));
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

    static unsigned char* mAudioData;
    static int mAudioOffset;
    static unsigned mAudioStartOffset;
};

class nvlMovie : public nvlAFMVMovie {
private:
    void ProcessAudioChunk() override;
    void StartAudioPlayback() override;
    void StopAudioPlayback() override;

public:
    ~nvlMovie() override;
    static IDirectSoundBuffer* mAudioBuffer;
};

static_assert(sizeof(nflRequestParams) == 40, "NVL request layout mismatch");
static_assert(sizeof(nvlMovieBase) == 336, "NVL base layout mismatch");
static_assert(sizeof(nvlAFMVMovie) == 8784, "AFMV movie layout mismatch");
static_assert(sizeof(nvlMovie) == 8784, "NVL movie layout mismatch");

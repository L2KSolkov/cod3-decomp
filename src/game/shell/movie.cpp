// ============================================================================
// movie.cpp - movie_manager + subtitle_manager (shell.o movie_manager.cpp /
// subtitle_manager.cpp)
// ============================================================================

#include "game/shell/shell_types.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_core.h"
#include "ngl/ngl_scene.h"
#include "ngl/nglFont.h"
#include "core/ae_fixed_string.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <intrin.h>

// ============================================================================
// nvl minimal view (stub bodies; real codec is nvl_xboxr, stubbed)
// ============================================================================
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
struct nglTexture;

enum nvlResult {
    NVL_RESULT_ERROR = -1,
    NVL_RESULT_OK = 0,
};
enum nvlFrameState {
    NVL_FRAME_ERROR = -1,
    NVL_FRAME_READY = 0,
    NVL_FRAME_STREAMING = 1,
    NVL_FRAME_DECODING = 2,
    NVL_FRAME_LAST = 3,
    NVL_FRAME_NONE = 4,
};

class nvlMovieBase {
public:
    virtual ~nvlMovieBase() {}                       // 0x82E780
    virtual nvlResult InitMovie() { return NVL_RESULT_ERROR; }  // 0x82E790
    virtual nvlFrameState DecodeFrame() { return NVL_FRAME_NONE; }  // 0x82E530
    virtual void Reset() {}                          // 0x82E590

    int  mWidth;            // +0x04
    int  mHeight;           // +0x08
    int  mWorkBufferSize;   // +0x0C
    int  mImageBufferSize;  // +0x10
    int  mStreamBufferSize; // +0x14
    unsigned char* mDataBuffer;  // +0x18
    unsigned char* mImageBuffer; // +0x1C
    unsigned char* mIntBuffer[4]; // +0x20

    nglTexture* GetTexture() { return nullptr; }     // 0x82E520
    nflFileID   ReleaseMovie() { return (nflFileID)0; }         // 0x82E410
    nvlResult   SetAudioTrack(int t) { (void)t; return NVL_RESULT_ERROR; }  // 0x82E550
    bool        IsBufferLoading() { return false; }  // 0x82E560
    bool        IsBufferFull() { return false; }     // 0x82EB80
};

class nvlAFMVMovie : public nvlMovieBase {
public:
    nvlAFMVMovie() {}                                // 0x82ECE0
    nvlResult ParseHeader(nflFileID id, bool back,
                          int offset, int size) { (void)id; (void)back;
        (void)offset; (void)size; return NVL_RESULT_ERROR; }  // 0x82EDF0
};

class nvlMovie : public nvlAFMVMovie {
public:
    nvlMovie() {}                                    // 0x82ED10
    virtual void ProcessAudioChunk() {}              // 0x82E100
    virtual void StartAudioPlayback() {}             // 0x82E1E0
    virtual void StopAudioPlayback() {}              // 0x82E230
};

// ============================================================================
// Externs (core.o / filesystem / ngl / game)
// ============================================================================
extern void* mem_heap_malloc(unsigned int size);     // core.o
extern void  mem_heap_free(void* ptr);               // core.o
extern FEManager g_femanager;                        // ?g_femanager@@3UFEManager@@A
extern int currCl;                                   // ?currCl@@3HA @ 0xF1579C
extern ELanguage gLanguage;                          // ?gLanguage@@3W4ELanguage@@A
extern SaveGameData gSaveGameData[4];                  // ?gSaveGameData@@3PAUSaveGameData@@A
extern const char defaultFileName[];            // ?defaultFileName
extern float sNaN;                                   // ?sNaN@@3MA
extern void AeAssert_Assert(const char* fmt, ...);

namespace LocalClient {
extern int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

// STBManager minimal view (same pattern as loading_menu.cpp)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);  // core.o
};

// nfl movie helpers (filesystem/nfl.cpp)
enum nflMediaID : unsigned {
    NFL_MEDIA_DEFAULT = 0,
    NFL_MEDIA_ID_DISC = 1,
    NFL_MEDIA_ID_HOST = 2,
    NFL_MEDIA_ID_LINK = 4,
};
extern nflMediaID gNflMediaId;  // ?gNflMediaId@@3W4nflMediaID@@A @ 0xE36B50
extern nflFileID nflOpenFileEx(nflMediaID media, const char* name,
                               unsigned int* fileSize);  // ?nflOpenFileEx@@YA?AW4nflFileID@@W4nflMediaID@@PBDPAI@Z
extern void nflCloseFile(nflFileID file);            // ?nflCloseFile@@YAXW4nflFileID@@@Z
enum nflState : unsigned;
extern nflState codNflUpdate();                       // ?codNflUpdate@@YA?AW4nflState@@XZ

// world_t minimal view (baseName +0x80; full in streamer/pakmanager.cpp)
struct world_t {
    char name[128];      // +0x00
    char baseName[128];  // +0x80
    void* bspTree;       // +0x100
    char* entityString;  // +0x104
    void* mSky;          // +0x108
};
extern world_t s_worldData;  // ?s_worldData@@3Uworld_t@@A @ 0xF74B98

// BankManager minimal view (defs in streamer/pakmanager.cpp)
struct TBankAlloc {
    unsigned long long mram_alloc1;  // +0x00
    unsigned long long mram_alloc2;  // +0x08
    bool IsEmpty() const;            // ?IsEmpty@TBankAlloc@@QBE_NXZ
};
struct mem_info {
    unsigned char* data;  // +0x00
    int            size;  // +0x04
};
class BankManager {
public:
    static BankManager* sInst;  // ?sInst@BankManager@@2PAV1@A @ 0xF592F4
    TBankAlloc Allocate(NumBanks num_banks);  // ?Allocate@BankManager@@QAE?AUTBankAlloc@@VNumBanks@@@Z
    mem_info get_alloc(const TBankAlloc& bat, int which,
                       bool mram) const;  // ?get_alloc@BankManager@@QBE?AVmem_info@@ABUTBankAlloc@@H_N@Z
    void release(TBankAlloc& alloc);  // ?release@BankManager@@QAEXAAUTBankAlloc@@@Z
};

// movie_manager / subtitle_manager statics
nvlMovie* movie_manager::theMovie = nullptr;
bool movie_manager::preloadDone = false;
bool movie_manager::ignoreLocalizedTrack = false;
float movie_manager::wait_timer = 0.0f;
float movie_manager::delay_timer = 0.0f;
nglQuad* movie_manager::theQuad = nullptr;
int movie_manager::lastTick = 0;
TBankAlloc sMovieAlloc;  // ?sMovieAlloc@@3UTBankAlloc@@A @ 0xF30E34

char* subtitle_manager::token1 = nullptr;
char* subtitle_manager::token2 = nullptr;
unsigned int subtitle_manager::m_color = 0;
float subtitle_manager::m_scale = 0.0f;
float subtitle_manager::timeRef = 0.0f;
float subtitle_manager::timeStart = 0.0f;
float subtitle_manager::timeEnd = 0.0f;
unsigned int subtitle_manager::posX1 = 0;
unsigned int subtitle_manager::posY1 = 0;
unsigned int subtitle_manager::posX2 = 0;
unsigned int subtitle_manager::posY2 = 0;
char subtitle_manager::sub_tag[45] = {0};
unsigned int subtitle_manager::index = 0;
char subtitle_manager::mPrefix[16] = {0};
char subtitle_manager::sub_text[163] = {0};
short subtitle_manager::m_token1TooLong = 0;
short subtitle_manager::m_token2TooLong = 0;
const char* seps = "|";  // ?seps@@3PBDB @ 0xDF3948 -> "|"

// ============================================================================
// movie_manager
// ============================================================================

// ea: 0x0056B430
void movie_manager::RegisterMovies(const PakInfoNode* paks,
                                   const PakHeader::Section& section)
{
    (void)paks; (void)section;
}

// ea: 0x0056B440
void movie_manager::init_manager()
{
}

// ea: 0x0056B450 (private)
bool movie_manager::force_exit()
{
    controller* v0 = controller::inst();
    v0->poll();
    if (wait_timer > delay_timer)
        return false;
    controller* v1 = controller::inst();
    if (v1->button_pressed(
            (controller::ButtonIndex)(controller::R3 | controller::RIGHTBUTTON),
            nullptr))
        g_femanager.skipAllLegalMovies = true;
    controller* v2 = controller::inst();
    if (!v2->any_button_pressed(nullptr))
        return false;
    controller* v3 = controller::inst();
    v3->button_pressed_clear_all();
    g_femanager.forceMovieExit = true;
    return true;
}

// ea: 0x0056B4C0 (private)
bool movie_manager::init_movie(const char* movie_name)
{
    unsigned int fileSize = 0;
    nflFileID v1 = nflOpenFileEx(gNflMediaId, movie_name, &fileSize);
    if (v1 == (nflFileID)-1
        || theMovie->ParseHeader(v1, true, 0, (int)fileSize)
               != NVL_RESULT_OK)
        return false;

    NumBanks v8;
    v8.ps2 = 8.0f;
    v8.ps3.main = 8.0f;
    v8.ps3.lram = 0.0f;
    v8.xbox = 8.0f;
    v8.xenon = 8.0f;
    v8.pcx = 8.0f;
    v8.gc.main = 8.0f;
    v8.gc.aram = 0.0f;
    sMovieAlloc = BankManager::sInst->Allocate(v8);
    if (sMovieAlloc.IsEmpty())
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\movie_manager.cpp";
        AeAssert::gCurrentLine = 606;
        AeAssert::gCurrentExpr = "!sMovieAlloc.IsEmpty()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("No memory for movies! :("))
            __debugbreak();
    }
    mem_info mi;
    mi = BankManager::sInst->get_alloc(sMovieAlloc, 0, true);
    unsigned char* data = mi.data;
    theMovie->mDataBuffer = mi.data;
    unsigned char* v4 = &data[theMovie->mWorkBufferSize];
    theMovie->mImageBuffer = v4;
    unsigned char* v5 = &v4[theMovie->mImageBufferSize];
    theMovie->mIntBuffer[0] = v5;
    unsigned char* v6 = &v5[theMovie->mStreamBufferSize];
    theMovie->mIntBuffer[1] = v6;
    unsigned char* v7 = &v6[theMovie->mStreamBufferSize];
    theMovie->mIntBuffer[2] = v7;
    theMovie->mIntBuffer[3] = &v7[theMovie->mStreamBufferSize];
    theMovie->InitMovie();
    return true;
}

// ea: 0x0056B640 (private)
bool movie_manager::dont_play_movies()
{
    return g_femanager.skipFE;
}

// ea: 0x0056B650
void movie_manager::set_quad_pos(float upper_left_x, float upper_left_y,
                                 float lower_right_x, float lower_right_y)
{
    nglSetQuadRect(theQuad, upper_left_x, upper_left_y, lower_right_x,
                   lower_right_y);
}

// ea: 0x0057C180
void movie_manager::movie_done(bool clear_backbuffer)
{
    (void)clear_backbuffer;
    if (!g_femanager.skipFE && theMovie != nullptr)
    {
        while (theMovie->IsBufferLoading())
            codNflUpdate();
        BankManager::sInst->release(sMovieAlloc);
        nglSetClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        nglSetClearFlags(0xF3u);
        nglPresent();
        nglPresent();
        nflFileID v0 = theMovie->ReleaseMovie();
        nflCloseFile(v0);
        subtitle_manager::token1 = nullptr;
        subtitle_manager::token2 = nullptr;
        mem_heap_free(theQuad);
        if (theMovie != nullptr)
            delete theMovie;
        theMovie = nullptr;
    }
}

// ea: 0x0057C240
void movie_manager::shut_down()
{
    if (theMovie != nullptr)
        movie_done(true);
}

// ea: 0x005843F0
void movie_manager::render()
{
    if (!g_femanager.skipFE && theMovie != nullptr)
    {
        theMovie->DecodeFrame();
        subtitle_manager::render();
        theQuad->Tex = theMovie->GetTexture();
        nglListAddQuad(theQuad);
    }
}

// ea: 0x00584430
nvlFrameState movie_manager::render2()
{
    if (g_femanager.skipFE)
        return NVL_FRAME_NONE;
    if (theMovie == nullptr)
        return NVL_FRAME_ERROR;
    nvlFrameState v1 = theMovie->DecodeFrame();
    subtitle_manager::render();
    return v1;
}

// ea: 0x0058D330
void movie_manager::frame_advance()
{
    if (!g_femanager.skipFE && theMovie != nullptr)
    {
        unsigned long long v0 = __rdtsc();
        float time_inc = (float)(v0 - (unsigned long long)lastTick)
                         * 0.0000013636364f;
        lastTick = (int)v0;
        delay_timer = delay_timer + time_inc;
        subtitle_manager::frame_advance((int)(time_inc * 1.015f));
        codNflUpdate();
    }
}

// ea: 0x005913B0
bool movie_manager::load_movie(const char* movie_name,
                               const char* sound_name)
{
    (void)sound_name;
    if (g_femanager.skipFE)
        return false;
    subtitle_manager::play_subtitle(movie_name, "MOVIE");

    char finalMovieName[516];
    strcpy(finalMovieName, "movi");
    strcat(finalMovieName, "es\\");
    strcat(finalMovieName, movie_name);
    strncat(finalMovieName, ".xbv", 7u);
    preloadDone = false;

    if (theMovie != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\movie_manager.cpp";
        AeAssert::gCurrentLine = 190;
        AeAssert::gCurrentExpr = "!theMovie";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "A previous movie was created but never destroyed!"))
            __debugbreak();
    }

    nvlMovie* v5 = (nvlMovie*)mem_heap_malloc(0x2250u);
    nvlMovie* v6;
    if (v5 != nullptr)
        v6 = new (v5) nvlMovie();
    else
        v6 = nullptr;
    theMovie = v6;
    if (v6 != nullptr)
    {
        nglQuad* v7 = (nglQuad*)mem_heap_malloc(0x60u);
        if (v7 == nullptr)
        {
            theQuad = nullptr;
            if (theMovie != nullptr)
                delete theMovie;
            theMovie = nullptr;
            preloadDone = true;
            return false;
        }
        memset(v7, 0, sizeof(nglQuad));
        theQuad = v7;
        if (init_movie(finalMovieName))
        {
            if (!ignoreLocalizedTrack && gLanguage != kLanguageEnglish)
            {
                if (gLanguage == kLanguageItalian)
                    theMovie->SetAudioTrack(1);
                else if (gLanguage == kLanguageSpanish)
                    theMovie->SetAudioTrack(2);
                else
                    theMovie->SetAudioTrack(0);
            }
            ignoreLocalizedTrack = false;
            nglInitQuad(theQuad);
            nglSetQuadBlend(theQuad, 0);
            nglSetQuadRect(theQuad, 0.0f, 0.0f, 640.0f, 480.0f);
            nglSetQuadZ(theQuad, 500.0f);
            nglSetQuadTex(theQuad, theMovie->GetTexture());
            delay_timer = 0.0f;
            while (!theMovie->IsBufferFull())
                frame_advance();
            preloadDone = true;
            return true;
        }
        if (theMovie != nullptr)
            delete theMovie;
        mem_heap_free(theQuad);
        theMovie = nullptr;
    }
    preloadDone = true;
    return false;
}

// ea: 0x00591640
void movie_manager::PakLoadCallback(float)
{
    if (!g_femanager.skipFE && theMovie != nullptr)
    {
        theMovie->DecodeFrame();
        subtitle_manager::render();
        theQuad->Tex = theMovie->GetTexture();
        nglListAddQuad(theQuad);
    }
    nglPresent();
    frame_advance();
}

// ea: 0x00591690
void movie_manager::continue_playing_wait_for_keypress()
{
    delay_timer = 0.0f;
    if (!g_femanager.skipFE && theMovie != nullptr)
    {
        math::Mat43 v4;
        memset(&v4, 0, sizeof(v4));
        v4.x.v.m128_f32[0] = 1.0f;
        v4.y.v.m128_f32[1] = 1.0f;
        v4.z.v.m128_f32[2] = 1.0f;
        v4.w.v.m128_f32[2] = 10.0f;
        v4.w.v.m128_f32[3] = 1.0f;
        bool v5 = false;
        do
        {
            nglSetPerspectiveMatrix(90.0f, 1.0f, 100.0f);
            nglSetWorldToViewMatrix(v4);
            nvlFrameState v3 = NVL_FRAME_NONE;
            bool last = false;
            if (!g_femanager.skipFE && theMovie != nullptr)
            {
                v3 = theMovie->DecodeFrame();
                subtitle_manager::render();
                last = (v3 == NVL_FRAME_LAST);
            }
            if (last || force_exit())
            {
                v5 = true;
            }
            subtitle_manager::render();
            nglPresent();
            frame_advance();
        }
        while (!v5);
        movie_done(true);
    }
}

// ea: 0x00593AF0
void movie_manager::load_and_play_movie(const char* movie_name,
                                        const char* sound_name)
{
    if (!g_femanager.skipFE)
    {
        preloadDone = false;
        if (load_movie(movie_name, sound_name) && theMovie != nullptr)
        {
            while (!theMovie->IsBufferFull())
                codNflUpdate();
            lastTick = (int)__rdtsc();
            subtitle_manager::play_subtitle(movie_name, "MOVIE");
            bool v9 = false;
            nglFrameLockType v5 = nglSetFrameLock(NGLFL_TWO);
            delay_timer = 0.0f;

            math::Mat43 v8;
            memset(&v8, 0, sizeof(v8));
            v8.x.v.m128_f32[0] = 1.0f;
            v8.y.v.m128_f32[1] = 1.0f;
            v8.z.v.m128_f32[2] = 1.0f;
            v8.w.v.m128_f32[2] = 10.0f;
            v8.w.v.m128_f32[3] = 1.0f;
            do
            {
                nglSetPerspectiveMatrix(90.0f, 1.0f, 100.0f);
                nglSetWorldToViewMatrix(v8);
                nvlFrameState v7 = NVL_FRAME_NONE;
                bool last = false;
                if (!g_femanager.skipFE && theMovie != nullptr)
                {
                    v7 = theMovie->DecodeFrame();
                    subtitle_manager::render();
                    last = (v7 == NVL_FRAME_LAST);
                }
                if (last || force_exit())
                {
                    v9 = true;
                }
                subtitle_manager::render();
                nglPresent();
                frame_advance();
            }
            while (!v9);
            movie_done(true);
            nglSetFrameLock(v5);
        }
    }
}

// ============================================================================
// subtitle_manager
// ============================================================================

// ea: 0x0056B6A0
void subtitle_manager::stop()
{
    token1 = nullptr;
    token2 = nullptr;
}

// ea: 0x0056B6B0
int subtitle_manager::GetCol(int)
{
    return (int)m_color;
}

// ea: 0x0056B6C0
char* subtitle_manager::GetLevelName()
{
    if (s_worldData.baseName[0] != 0)
        return _strupr(s_worldData.baseName);
    else
        return "MOVIE";
}

// ea: 0x0057C260
void subtitle_manager::render()
{
    int port = LocalClient::ClientToPort(currCl);
    if (gSaveGameData[port].mStubData.mSubtitles
        && (!g_femanager.inGame
            || !GamePause::IsGamePaused(currCl)
            || movie_manager::theMovie != nullptr)
        && token2 != nullptr && timeRef >= timeStart)
    {
        float y = (float)posY2;
        float x = (float)posX2;
        nglListAddString(g_femanager.fonts[2], x, y, 0.0f, m_color,
                         m_scale, m_scale, token2);
        if (token1 != nullptr)
        {
            float ya = (float)posY1;
            float xa = (float)posX1;
            nglListAddString(g_femanager.fonts[2], xa, ya, 0.0f, m_color,
                             m_scale, m_scale, token1);
        }
    }
}

// ea: 0x00584460
bool subtitle_manager::play_subtitle(const char* tag, char* prefix)
{
    char final_tag[67];
    char tmpstr[256];
    final_tag[63] = 0;
    final_tag[0] = 0;
    bool singleDialog = false;
    if (gLanguage != kLanguageEnglish)
        return false;
    if (tag != nullptr)
    {
        strncpy(sub_tag, tag, 0x2Du);
        sub_tag[44] = 0;
        char* v3 = _strupr(sub_tag);
        strncpy(sub_tag, v3, 0x2Du);
        index = 1;
        timeRef = 0.0f;
    }
    const char* v4 = prefix;
    if (prefix == nullptr)
    {
        if (s_worldData.baseName[0] != 0)
            v4 = _strupr(s_worldData.baseName);
        else
            v4 = "MOVIE";
    }
    strncpy(mPrefix, v4, 0x10u);
    unsigned int v14 = index++;
    ae_formatted_string<64, unsigned char> v16b("%s_%s_%d", v4, sub_tag,
                                                v14);
    memcpy(final_tag, &v16b, 0x40u);
    const char* STBString =
        STBManager::sInst->GetSTBString(final_tag);
    bool v6;
    if (!AeStringSupport::StrCStrEqu(final_tag, final_tag[63], STBString, -1)
        && STBString != nullptr)
    {
        v6 = singleDialog;
    }
    else
    {
        if (index != 2)
        {
            token2 = nullptr;
            token1 = nullptr;
            return false;
        }
        ae_formatted_string<64, unsigned char> v16c("%s_%s", v4, sub_tag);
        memcpy(final_tag, &v16c, 0x40u);
        STBString = STBManager::sInst->GetSTBString(final_tag);
        if (!AeStringSupport::StrCStrEqu(final_tag, final_tag[63], STBString,
                                         -1)
            || STBString == nullptr)
        {
            token2 = nullptr;
            token1 = nullptr;
            return false;
        }
        v6 = true;
    }
    strncpy(sub_text, STBString, 0xA3u);
    if (!v6)
    {
        char* v7 = strtok(sub_text, seps);
        if (v7 != nullptr)
        {
            timeStart = (float)atof(v7);
            char* v8 = strtok(nullptr, seps);
            if (v8 != nullptr)
            {
                timeEnd = (float)atof(v8);
                token1 = strtok(nullptr, seps);
                if (token1 == nullptr)
                    return false;
                goto LABEL_26;
            }
        }
        return false;
    }
    char* v9 = strtok(sub_text, seps);
    char* v10 = v9;
    if (v9 == nullptr)
        return false;
    timeStart = (float)atof(v9);
    char* v11;
    if (timeStart == 0.0f)
    {
        timeEnd = 3.0f;
        token1 = v10;
        v11 = strtok(nullptr, seps);
        goto LABEL_28;
    }
    timeEnd = timeStart;
    timeStart = 0.0f;
    token1 = strtok(nullptr, seps);
    if (token1 == nullptr)
        return false;
LABEL_26:
    v11 = strtok(nullptr, seps);
LABEL_28:
    token2 = v11;
    if (v11 == nullptr)
    {
        v11 = token1;
        token2 = token1;
        token1 = nullptr;
    }
    m_token1TooLong = 0;
    m_token2TooLong = 0;
    nglGetStringDimensions(g_femanager.fonts[2], &posX2, &posY2, m_scale,
                           m_scale, v11);
    int v12 = (int)(posX2 >> 1);
    posY2 = 420;
    if (v12 > 0x134)
    {
        sprintf(tmpstr, "TOO LONG: %s", token2);
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\subtitle_manager.cpp";
        AeAssert::gCurrentLine = 200;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(tmpstr))
            __debugbreak();
        if (v12 > 320)
            v12 = 320;
        m_token2TooLong = 1;
    }
    posX2 = 320 - v12;
    if (token1 != nullptr)
    {
        nglGetStringDimensions(g_femanager.fonts[2], &posX1, &posY1, m_scale,
                               m_scale, token1);
        int v13 = (int)(posX1 >> 1);
        if (v13 > 0x134)
        {
            sprintf(tmpstr, "TOO LONG: %s", token1);
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\subtitle_manager.cpp";
            AeAssert::gCurrentLine = 220;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(tmpstr))
                __debugbreak();
            if (v13 > 320)
                v13 = 320;
            m_token1TooLong = 1;
        }
        posY1 = 400;
        posX1 = 320 - v13;
    }
    return true;
}

// ea: 0x005848C0
void subtitle_manager::frame_advance(int time_delta)
{
    timeRef = time_delta * 0.001f + timeRef;
    if (token2 != nullptr && timeRef > timeEnd)
        play_subtitle(nullptr, mPrefix);
}

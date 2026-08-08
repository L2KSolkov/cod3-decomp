// ============================================================================
// rumble.cpp - RumbleManager (core.o RumbleManager.cpp)
// ============================================================================

#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <string.h>

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

extern float ComputeIntensity(float min_distance, float max_distance,
                              float distance);
extern void* controller_inst();
extern void controller_stop_all_rumble(void* self);
extern void controller_rumble(void* self, int i_controller_num, int i_motor,
                              float intensity);
extern int currCl;
extern int level_time;
extern void* EntityManager_sInst;
extern void* EntityManager_GetPlayer(void* inst, int idx);
extern bool GamePause_mData_mGamePaused[];
extern int cls_state;
extern int dword_F6A28C[];
extern int gSaveGameData_mVibration[];
extern int AnimHeap_sInst;
extern void* PoolAllocator_Allocate(void* allocator, unsigned int s,
                                    bool forceHeapAlloc);
extern void PoolAllocator_Release(void* allocator, void* ptr);
extern void* RumbleEffectInstance_sAllocator;

// RumbleEffect accessors
extern bool RumbleEffect_GetEnabled(const RumbleEffect* self, int rumbleID);
extern float RumbleEffect_GetDelay(const RumbleEffect* self, int rumbleID);
extern float RumbleEffect_GetIntensity(const RumbleEffect* self, int rumbleID);
extern float RumbleEffect_GetRampUpDuration(const RumbleEffect* self,
                                            int rumbleID);
extern float RumbleEffect_GetSteadyDuration(const RumbleEffect* self,
                                            int rumbleID);
extern float RumbleEffect_GetRampDownDuration(const RumbleEffect* self,
                                              int rumbleID);
extern Broc::string RumbleEffect_GetNotes(const RumbleEffect* self, int rumbleID);
extern void RumbleEffectInstance_Ctor(void* self, RumbleEffectInstanceHandle handle,
                                      float delay, float intensity,
                                      float base_intensity,
                                      float ramp_up_duration,
                                      float steady_duration,
                                      float ramp_down_duration,
                                      Broc::string rumble_notes, int looping);
extern void RumbleEffectInstance_Dtor(void* self);

// ea: 0x004BD110
RumbleEffectInstanceHandle RumbleManager::BumpHandle()
{
    RumbleEffectInstanceHandle result;
    int v2 = mNextHandle.mVal++;
    result.mVal = v2;
    return result;
}

// ea: 0x004BD130
void RumbleManager::StopMotors()
{
    controller_stop_all_rumble(controller_inst());
}

// ea: 0x004C5750
RumbleEffectInstanceHandle RumbleManager::Play(RumbleEffect* effect,
                                               float intensity)
{
    RumbleEffectInstanceHandle result;
    if (intensity < 0.0f || intensity > 1.0f)
    {
        ASSERT("intensity >= 0.0f && intensity <= 1.0f",
               "c:\\cod\\code\\game\\RumbleManager.cpp", 85);
    }
    int playerState =
        *(int*)((char*)EntityManager_GetPlayer(EntityManager_sInst, mClient) + 0x100);
    if (playerState == 4 || playerState == 3)
    {
        result.mVal = mNextHandle.mVal;
        mNextHandle.mVal = result.mVal + 1;
        for (int v8 = 0; v8 < 2; ++v8)
        {
            const RumbleEffect::RumbleData& data = effect->mRumbleDataArray[v8];
            if (data.delay > 0.0f && RumbleEffect_GetEnabled(effect, v8))
            {
                void* v9 = PoolAllocator_Allocate(RumbleEffectInstance_sAllocator,
                                                  0x30u, false);
                RumbleEffectInstance* inst = (RumbleEffectInstance*)v9;
                int looping = (data.m_flags.mVal & 2) != 0;
                Broc::string notes = RumbleEffect_GetNotes(effect, v8);
                RumbleEffectInstance_Ctor(
                    v9, result, RumbleEffect_GetDelay(effect, v8), intensity,
                    RumbleEffect_GetIntensity(effect, v8),
                    RumbleEffect_GetRampUpDuration(effect, v8),
                    RumbleEffect_GetSteadyDuration(effect, v8),
                    RumbleEffect_GetRampDownDuration(effect, v8), notes, looping);
                // push into mRumbleLists[v8]
                inst->m_dlist_node.mNext = mRumbleLists[v8].mRoot.mNext;
                inst->m_dlist_node.mPrev = mRumbleLists[v8].mRoot.mPrev;
                if (mRumbleLists[v8].mRoot.mPrev)
                    mRumbleLists[v8].mRoot.mPrev->mNext = &inst->m_dlist_node;
                mRumbleLists[v8].mRoot.mPrev = &inst->m_dlist_node;
                if (!mRumbleLists[v8].mRoot.mNext)
                    mRumbleLists[v8].mRoot.mNext = &inst->m_dlist_node;
            }
        }
        return result;
    }
    result.mVal = 0;
    return result;
}

// ea: 0x004C5980
RumbleEffectInstanceHandle RumbleManager::Play(const RumbleEffect* effect,
                                               float min_distance,
                                               float max_distance,
                                               float distance)
{
    RumbleEffectInstanceHandle result;
    float intensity = ComputeIntensity(min_distance, max_distance, distance);
    result = Play((RumbleEffect*)effect, intensity);
    return result;
}

// ea: 0x004CBAB0
char RumbleManager::IsPlaying(RumbleEffectInstanceHandle handle)
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               164);
    }
    for (int list = 0; list < 2; ++list)
    {
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].mRoot.mNext;
             n != nullptr; n = (RumbleEffectInstance*)n->m_dlist_node.mNext)
        {
            if (n->m_handle.mVal == handle.mVal)
                return 1;
        }
    }
    return 0;
}

// ea: 0x004CBBA0
float RumbleManager::TimeLeft(RumbleEffectInstanceHandle handle)
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               188);
    }
    float v8 = 0.0f;
    for (int list = 0; list < 2; ++list)
    {
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].mRoot.mNext;
             n != nullptr; n = (RumbleEffectInstance*)n->m_dlist_node.mNext)
        {
            if (n->m_handle.mVal == handle.mVal)
            {
                if (n->m_duration - n->m_cur_time > v8)
                    v8 = n->m_duration - n->m_cur_time;
            }
        }
    }
    return v8;
}

// ea: 0x004CBCB0
void RumbleManager::SetIntensity(RumbleEffectInstanceHandle handle,
                                 float intensity)
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               220);
    }
    if (intensity < 0.0f || intensity > 1.0f)
    {
        ASSERT("intensity >= 0.0f && intensity <= 1.0f",
               "c:\\cod\\code\\game\\RumbleManager.cpp", 221);
    }
    for (int list = 0; list < 2; ++list)
    {
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].mRoot.mNext;
             n != nullptr; n = (RumbleEffectInstance*)n->m_dlist_node.mNext)
        {
            if (n->m_handle.mVal == handle.mVal)
                n->m_intensity = intensity;
        }
    }
}

// ea: 0x004CBDB0
void RumbleManager::SetDistance(RumbleEffectInstanceHandle handle,
                                float min_distance, float max_distance,
                                float distance)
{
    float intensity = ComputeIntensity(min_distance, max_distance, distance);
    SetIntensity(handle, intensity);
}

// ea: 0x004CF370
void RumbleManager::Reset()
{
    for (int list = 0; list < 2; ++list)
    {
        RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].mRoot.mNext;
        while (n != nullptr)
        {
            RumbleEffectInstance* next = (RumbleEffectInstance*)n->m_dlist_node.mNext;
            RumbleEffectInstance_Dtor(n);
            PoolAllocator_Release(RumbleEffectInstance_sAllocator, n);
            n = next;
        }
        mRumbleLists[list].mRoot.mNext = nullptr;
        mRumbleLists[list].mRoot.mPrev = nullptr;
    }
    controller_stop_all_rumble(controller_inst());
}

// ea: 0x004CBDE0
void RumbleManager::FrameAdvance(float delta_time)
{
    float total_max_intensity = 0.0f;
    for (int vibrator_id = 0; vibrator_id < 2; ++vibrator_id)
    {
        float max_intensity = 0.0f;
        RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[vibrator_id].mRoot.mNext;
        while (n != nullptr)
        {
            RumbleEffectInstance* next = (RumbleEffectInstance*)n->m_dlist_node.mNext;
            float intensity = 0.0f;
            n->m_cur_time = delta_time + n->m_cur_time;
            bool looping = (n->m_flags.mVal & 2) != 0;
            if (!(n->m_cur_time < n->m_duration) && looping)
                n->m_cur_time = n->m_cur_time - n->m_duration;
            float m_cur_time = n->m_cur_time;
            if ((n->m_flags.mVal & 1) != 0)
            {
                if (m_cur_time >= n->m_duration)
                {
                    intensity = 0.0f;
                }
                else if (m_cur_time >= 0.0f && n->m_duration != 0.0f)
                {
                    const char* notes = n->m_rumble_notes.c_str();
                    int mLength = n->m_rumble_notes.length();
                    float idxf = (m_cur_time / n->m_duration) * (mLength - 1);
                    int idx = (int)idxf;
                    if (idx >= mLength - 2)
                        intensity = (notes[idx] - 97) / 25.0f;
                    else
                    {
                        float v21 = (notes[idx] - 97) / 25.0f;
                        float v22 = (notes[idx + 1] - 97) / 25.0f;
                        intensity = ((v22 - v21) * (idxf - idx)) + v21;
                    }
                }
            }
            else
            {
                if (m_cur_time >= n->m_ramp_down_end && n->m_duration != 0.0f)
                {
                    intensity = 0.0f;
                }
                else if (m_cur_time < n->m_steady_end || n->m_duration == 0.0f)
                {
                    if (m_cur_time < n->m_ramp_up_end)
                    {
                        if (m_cur_time < 0.0f)
                            intensity = 0.0f;
                        else
                            intensity = (n->m_base_intensity - 0.0f)
                                      * (m_cur_time / n->m_ramp_up_end) + 0.0f;
                    }
                    else
                    {
                        intensity = n->m_base_intensity;
                    }
                }
                else
                {
                    intensity = (0.0f - n->m_base_intensity)
                              * ((m_cur_time - n->m_steady_end)
                                 / (n->m_ramp_down_end - n->m_steady_end))
                              + n->m_base_intensity;
                }
            }
            if (intensity <= 0.0f)
            {
                // expired: remove
                RumbleEffectInstance_Dtor(n);
                PoolAllocator_Release(RumbleEffectInstance_sAllocator, n);
            }
            else if ((n->m_intensity * intensity) > max_intensity)
            {
                max_intensity = n->m_intensity * intensity;
            }
            n = next;
        }
        if (mClient != 0)
        {
            ASSERT("client >= 0 && client < 1", "c:\\cod\\code\\game\\GamePause.h", 12);
        }
        if (GamePause_mData_mGamePaused[mClient]
            || !gSaveGameData_mVibration[7156 * mClient]
            || cls_state == 5)
        {
            max_intensity = 0.0f;
        }
        total_max_intensity = max_intensity + total_max_intensity;
        if (dword_F6A28C[802 * currCl] == 0)
            controller_rumble(controller_inst(), 0, vibrator_id, max_intensity);
    }
    if (total_max_intensity > 0.0f)
    {
        if (mLastTimeNotRumbling != 0 && mLastTimeNotRumbling + 19000 < level_time)
            mDontRumbleAgainUntil = level_time + 1000;
    }
    if (total_max_intensity <= 0.000001f)
        mLastTimeNotRumbling = level_time;
}

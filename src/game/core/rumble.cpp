// ============================================================================
// rumble.cpp - RumbleManager (core.o RumbleManager.cpp)
// ============================================================================

#include "game/core/core_systems.h"

#include "game/core/core_globals.h"

#include <string.h>

// Minimal view of controller (full class in game/platform_xbox/XboxLiveMenus.h).
class controller { public:
    int locked_port;
    static controller* inst();  // ?inst@controller@@SAPAV1@XZ (controller_xbox.o)
};


namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
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
extern void controller_stop_all_rumble(void* self);

// ea: 0x004DE110 (core.o)
bool RumbleEffect::GetEnabled(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].enabled;
}

// ea: 0x004DE090
float RumbleEffect::GetDelay(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].delay;
}

// ea: 0x004DE190
float RumbleEffect::GetIntensity(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].intensity;
}

// ea: 0x004DE210
float RumbleEffect::GetRampDownDuration(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].ramp_down_duration;
}

// ea: 0x004DE290
float RumbleEffect::GetRampUpDuration(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].ramp_up_duration;
}

// ea: 0x004DE310
float RumbleEffect::GetSteadyDuration(ERumbleMotorID rumbleID) const
{
    if (rumbleID >= (ERumbleMotorID)2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";
        AeAssert::gCurrentLine = 36;
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return mRumbleDataArray[rumbleID].steady_duration;
}

// ea: 0x006BBDE0 (cg.o)
void RumbleEffect::SetLooping(ERumbleMotorID rumbleID, bool looping)
{
    mRumbleDataArray[rumbleID].m_flags.mVal |= 2u;
}

extern void controller_rumble(void* self, int i_controller_num, int i_motor,
                              float intensity);
extern int currCl;
extern int level_time;
extern void* EntityManager_sInst;
extern void* EntityManager_GetPlayer(void* inst, int idx);
void* EntityManager_GetPlayer(void* inst, int idx)
{
    (void)inst; (void)idx;
    return nullptr;
}
// Minimal view of GamePause (full class in game/sv/sv_stubs.h; mData defined
// in g_entity_misc.cpp).
struct GamePause {
    struct GamePauseData {
        bool mGamePaused[1];  // +0x00
    };
    static GamePauseData mData;  // ?mData@GamePause@@0UGamePauseData@1@A
};
extern int cls_state;
extern int dword_F6A28C[];
int gSaveGameData_mVibration[4 * 7156];  // ?gSaveGameData_mVibration (game2.o)
extern int AnimHeap_sInst;
extern void* PoolAllocator_Allocate(void* allocator, unsigned int s,
                                    bool forceHeapAlloc);
extern void PoolAllocator_Release(void* allocator, void* ptr);
extern void* RumbleEffectInstance_sAllocator;

extern Broc::string RumbleEffect_GetNotes(const RumbleEffect* self, int rumbleID);
extern void RumbleEffectInstance_Ctor(void* self, RumbleEffectInstanceHandle handle,
                                      float delay, float intensity,
                                      float base_intensity,
                                      float ramp_up_duration,
                                      float steady_duration,
                                      float ramp_down_duration,
                                      Broc::string rumble_notes, int looping);
extern void RumbleEffectInstance_Dtor(void* self);

// RumbleEffect / pool artifacts (core.o; stubs, port later)
void* PoolAllocator_Allocate(void* allocator, unsigned int s,
                             bool forceHeapAlloc)
{
    (void)allocator; (void)s; (void)forceHeapAlloc;
    return nullptr;
}
void PoolAllocator_Release(void* allocator, void* ptr)
{
    (void)allocator; (void)ptr;
}
Broc::string RumbleEffect_GetNotes(const RumbleEffect* self, int rumbleID)
{
    (void)self; (void)rumbleID;
    return Broc::string((Broc::string::Block*)nullptr);
}
void RumbleEffectInstance_Ctor(void* self, RumbleEffectInstanceHandle handle,
                               float delay, float intensity,
                               float base_intensity,
                               float ramp_up_duration,
                               float steady_duration,
                               float ramp_down_duration,
                               Broc::string rumble_notes, int looping)
{
    (void)self; (void)handle; (void)delay; (void)intensity;
    (void)base_intensity; (void)ramp_up_duration; (void)steady_duration;
    (void)ramp_down_duration; (void)rumble_notes; (void)looping;
}
void RumbleEffectInstance_Dtor(void* self)
{
    (void)self;
}

// ea: 0x004BD110
RumbleEffectInstanceHandle RumbleManager::BumpHandle()
{
    RumbleEffectInstanceHandle result;
    int v2 = mNextHandle.mVal++;
    result.mVal = v2;
    return result;
}

// ea: 0x004A9DA0
RumbleManager* RumbleManager::Inst(int instance)
{
    if (instance != 0)
    {
        ASSERT("instance >= 0 && instance < 1",
               "c:\\cod\\code\\game\\RumbleManager.h", 24);
    }
    return RumbleManagerStatics::sInstHolder.sInst[instance];
}

// ea: 0x004BD130
void RumbleManager::StopMotors()
{
    controller_stop_all_rumble(controller::inst());
}

// ea: 0x004C5750
RumbleEffectInstanceHandle RumbleManager::Play(const RumbleEffect& effect,
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
            const RumbleEffect::RumbleData& data = effect.mRumbleDataArray[v8];
            if (data.delay > 0.0f
                && effect.GetEnabled((ERumbleMotorID)v8))
            {
                void* v9 = PoolAllocator_Allocate(RumbleEffectInstance_sAllocator,
                                                  0x30u, false);
                RumbleEffectInstance* inst = (RumbleEffectInstance*)v9;
                int looping = (data.m_flags.mVal & 2) != 0;
                Broc::string notes = RumbleEffect_GetNotes(&effect, v8);
                RumbleEffectInstance_Ctor(
                    v9, result,
                    effect.GetDelay((ERumbleMotorID)v8), intensity,
                    effect.GetIntensity((ERumbleMotorID)v8),
                    effect.GetRampUpDuration((ERumbleMotorID)v8),
                    effect.GetSteadyDuration((ERumbleMotorID)v8),
                    effect.GetRampDownDuration((ERumbleMotorID)v8), notes,
                    looping);
                // push into mRumbleLists[v8]
                inst->m_dlist_node.mNext = mRumbleLists[v8].m_head;
                inst->m_dlist_node.mPrev = mRumbleLists[v8].m_tail;
                if (mRumbleLists[v8].m_tail)
                    mRumbleLists[v8].m_tail->mNext = &inst->m_dlist_node;
                mRumbleLists[v8].m_tail = &inst->m_dlist_node;
                if (!mRumbleLists[v8].m_head)
                    mRumbleLists[v8].m_head = &inst->m_dlist_node;
            }
        }
        return result;
    }
    result.mVal = 0;
    return result;
}

// ea: 0x004C5980
RumbleEffectInstanceHandle RumbleManager::Play(const RumbleEffect& effect,
                                               float min_distance,
                                               float max_distance,
                                               float distance)
{
    RumbleEffectInstanceHandle result;
    float intensity = ComputeIntensity(min_distance, max_distance, distance);
    result = Play(effect, intensity);
    return result;
}

// ea: 0x004CBAB0
bool RumbleManager::IsPlaying(RumbleEffectInstanceHandle handle) const
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               164);
    }
    for (int list = 0; list < 2; ++list)
    {
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].m_head;
            n != nullptr; n = (RumbleEffectInstance*)n->m_dlist_node.mNext)
        {
            if (n->m_handle.mVal == handle.mVal)
                return true;
        }
    }
    return false;
}

// ea: 0x004CBBA0
float RumbleManager::TimeLeft(RumbleEffectInstanceHandle handle) const
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               188);
    }
    float v8 = 0.0f;
    for (int list = 0; list < 2; ++list)
    {
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].m_head;
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
        for (RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].m_head;
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
        RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].m_head;
        while (n != nullptr)
        {
            RumbleEffectInstance* next = (RumbleEffectInstance*)n->m_dlist_node.mNext;
            RumbleEffectInstance_Dtor(n);
            PoolAllocator_Release(RumbleEffectInstance_sAllocator, n);
            n = next;
        }
        mRumbleLists[list].m_head = nullptr;
        mRumbleLists[list].m_tail = nullptr;
    }
    controller_stop_all_rumble(controller::inst());
}

// ea: 0x004CB9C0
void RumbleManager::Remove(RumbleEffectInstanceHandle handle)
{
    if (handle.mVal == 0)
    {
        ASSERT("!handle.IsNull()", "c:\\cod\\code\\game\\RumbleManager.cpp",
               135);
    }
    for (int list = 0; list < 2; ++list)
    {
        RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[list].m_head;
        while (n != nullptr)
        {
            RumbleEffectInstance* next = (RumbleEffectInstance*)n->m_dlist_node.mNext;
            if (n->m_handle.mVal == handle.mVal)
            {
                RumbleEffectInstance_Dtor(n);
                PoolAllocator_Release(RumbleEffectInstance_sAllocator, n);
                break;
            }
            n = next;
        }
    }
}

// ea: 0x004CBDE0
void RumbleManager::FrameAdvance(float delta_time)
{
    float total_max_intensity = 0.0f;
    for (int vibrator_id = 0; vibrator_id < 2; ++vibrator_id)
    {
        float max_intensity = 0.0f;
        RumbleEffectInstance* n = (RumbleEffectInstance*)mRumbleLists[vibrator_id].m_head;
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
        if (GamePause::mData.mGamePaused[mClient]
            || !gSaveGameData_mVibration[7156 * mClient]
            || cls_state == 5)
        {
            max_intensity = 0.0f;
        }
        total_max_intensity = max_intensity + total_max_intensity;
        if (dword_F6A28C[802 * currCl] == 0)
            controller_rumble(controller::inst(), 0, vibrator_id, max_intensity);
    }
    if (total_max_intensity > 0.0f)
    {
        if (mLastTimeNotRumbling != 0 && mLastTimeNotRumbling + 19000 < level_time)
            mDontRumbleAgainUntil = level_time + 1000;
    }
    if (total_max_intensity <= 0.000001f)
        mLastTimeNotRumbling = level_time;
}

// ============================================================================
// RumbleEffect setters (g.o 0x4A8A70-0x4A8FB0; emitted here because
// RumbleEffect is a core_systems.h type)
// ============================================================================

#define RUMBLE_RANGE_ASSERT(line_no)                                       \
    do {                                                                   \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                         \
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";    \
        AeAssert::gCurrentLine = (line_no);                                \
        AeAssert::gCurrentExpr = "( rumbleID >= kRumbleMin && rumbleID <= kRumbleMax )"; \
        if (!AeAssert::IsIgnored()                                         \
            && AeAssert::Assert("value not in enum range"))                \
            __debugbreak();                                                \
    } while (0)

#define RUMBLE_VALUE_ASSERT(expr_str, line_no, msg)                        \
    do {                                                                   \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                         \
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\RumbleEffect.h";    \
        AeAssert::gCurrentLine = (line_no);                                \
        AeAssert::gCurrentExpr = (expr_str);                               \
        if (!AeAssert::IsIgnored()                                         \
            && AeAssert::Assert((msg)))                                    \
            __debugbreak();                                                \
    } while (0)

// ea: 0x004A8A70
void RumbleEffect::SetDelay(ERumbleMotorID rumbleID, float new_delay)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(89);
    if (new_delay < 0.0f)
        RUMBLE_VALUE_ASSERT("new_delay >= 0.0f", 90,
                            "Please add a descriptive string");
    mRumbleDataArray[rumbleID].delay = new_delay;
}

// ea: 0x004A8B60
void RumbleEffect::SetEnabled(ERumbleMotorID rumbleID, bool new_enabled)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(96);
    mRumbleDataArray[rumbleID].enabled = new_enabled;
}

// ea: 0x004A8BE0
void RumbleEffect::SetIntensity(ERumbleMotorID rumbleID, float new_intensity)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(102);
    if (new_intensity < 0.0f || new_intensity > 1.0f)
        RUMBLE_VALUE_ASSERT("new_intensity >= 0.0f && new_intensity <= 1.0f",
                            103, "Please add a descriptive string");
    mRumbleDataArray[rumbleID].intensity = new_intensity;
}

// ea: 0x004A8CE0
void RumbleEffect::SetRampDownDuration(ERumbleMotorID rumbleID,
                                       float new_duration)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(109);
    if (new_duration < 0.0f)
        RUMBLE_VALUE_ASSERT("new_duration >= 0.0f", 110,
                            "Please add a descriptive string");
    mRumbleDataArray[rumbleID].ramp_down_duration = new_duration;
}

// ea: 0x004A8DD0
void RumbleEffect::SetRampUpDuration(ERumbleMotorID rumbleID,
                                     float new_duration)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(116);
    if (new_duration < 0.0f)
        RUMBLE_VALUE_ASSERT("new_duration >= 0.0f", 117,
                            "Please add a descriptive string");
    mRumbleDataArray[rumbleID].ramp_up_duration = new_duration;
}

// ea: 0x004A8EC0
void RumbleEffect::SetSteadyDuration(ERumbleMotorID rumbleID,
                                     float new_duration)
{
    if (rumbleID >= (ERumbleMotorID)2)
        RUMBLE_RANGE_ASSERT(123);
    if (new_duration < 0.0f)
        RUMBLE_VALUE_ASSERT("new_duration >= 0.0f", 124,
                            "Please add a descriptive string");
    mRumbleDataArray[rumbleID].steady_duration = new_duration;
}

// ea: 0x004A8FB0
void RumbleEffect::Initialize()
{
    for (int i = 0; i < 2; ++i)
    {
        mRumbleDataArray[i].enabled = true;
        mRumbleDataArray[i].delay = 0.0f;
        mRumbleDataArray[i].intensity = 1.0f;
        mRumbleDataArray[i].ramp_up_duration = 0.0f;
        mRumbleDataArray[i].steady_duration = 1.0f;
        mRumbleDataArray[i].ramp_down_duration = 0.0f;
    }
}

#undef RUMBLE_RANGE_ASSERT
#undef RUMBLE_VALUE_ASSERT

// Bitmask<unsigned int> accessors (g.o 0x4ACD50-0x4ACDF0; emitted here
// because Bitmask is a core_systems.h type)
template class Bitmask<unsigned int>;

// BitSet<1344> word accessors (g.o 0x4AE350 / 0x4AE360)
template class BitSet<1344>;

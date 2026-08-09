// ============================================================================
// g_debugthread.cpp - DebugThread message helpers + Task/HealthRegenTask
// (game2.o). Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// DebugThread message globals (game2.o data)
// ============================================================================
extern float gDebugThread_MessageRGB[3];
extern float gDebugThread_MessageScale;
extern float gDebugThread_MessageXpos;
extern char* gDebugThread_Message;
extern int gDebugThread_MessageTicks;
extern float gDebugThread_MessageYpos;
extern int gDebugThread_MessageAlphaMin;

// ============================================================================
// DebugThread::DisplayMessage - ea: 0x4F4620
// ============================================================================
char* DebugThread::DisplayMessage(char* msg, int xpos, int ypos, float r,
                                  float g, float b, float scale,
                                  float alphaMin)
{
    gDebugThread_MessageRGB[0] = r;
    gDebugThread_MessageRGB[1] = g;
    gDebugThread_MessageRGB[2] = b;
    gDebugThread_MessageScale = scale;
    gDebugThread_MessageXpos = (float)xpos;
    gDebugThread_Message = msg;
    gDebugThread_MessageTicks = 300;
    gDebugThread_MessageYpos = (float)ypos;
    gDebugThread_MessageAlphaMin = (int)alphaMin;
    return msg;
}

// ============================================================================
// DebugThread::Update - ea: 0x4F46A0
// ============================================================================
void DebugThread::Update()
{
}

extern void* Task_vftable;  // ??_7Task@@6B@

// ea: 0x4F9940
Task::Task(DbLinkedHandle<EntityHandleDb, Entity> handle, unsigned int idTask)
{
    __vftable = (void*)&Task_vftable;
    memset(_dlist, 0, 8);
    mTaskId = idTask;
    mEntityHandle = handle;
    mTaskHandle.mVal = 0;
    mFlags = 1;
}

// ea: 0x4F9970
Task::Task(DbLinkedHandle<EntityHandleDb, Entity> handle, int idTask)
{
    __vftable = (void*)&Task_vftable;
    memset(_dlist, 0, 8);
    mTaskId = (unsigned int)idTask;
    mEntityHandle = handle;
    mTaskHandle.mVal = 0;
    mFlags = 1;
}

struct HealthRegenTask : Task {
    float mDamageDelay;             // +0x1C
    float mRechargeRate;            // +0x20
    float mTimeSinceDamage;         // +0x24
    float mHealthDelta;             // +0x28
    bool mVeryHurt;                 // +0x2C
    float mTimeSinceVeryHurt;       // +0x30
    float mTimeSinceRecoverySound;  // +0x34

    HealthRegenTask(DbLinkedHandle<EntityHandleDb, Entity> h, float damageDelay,
                    float rechargeRate);
    void Update(Entity* e, float deltaT);  // ea: 0x4F9AC0
};
static_assert(sizeof(HealthRegenTask) == 0x38, "HealthRegenTask size mismatch");

extern void* HealthRegenTask_vftable;  // ??_7HealthRegenTask@@6B@
extern cvar_t* HealthRegenTask_sDamageDelay;
extern cvar_t* HealthRegenTask_sRechargeRate;
extern cvar_t* Cvar_Get(const char* var_name, const char* var_value, int flags);

// ea: 0x4F99A0
HealthRegenTask::HealthRegenTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                                 float damageDelay, float rechargeRate)
    : Task(h, 1213351758)
{
    __vftable = (void*)&HealthRegenTask_vftable;
    mTimeSinceDamage = 0.0f;
    mHealthDelta = 0.0f;
    mVeryHurt = false;
    mTimeSinceVeryHurt = 0.0f;
    mTimeSinceRecoverySound = 0.0f;
    if (HealthRegenTask_sDamageDelay == nullptr)
    {
        HealthRegenTask_sDamageDelay = Cvar_Get(
            "hud_healthOverlay_regenPauseTime", "5000", 256);
        HealthRegenTask_sRechargeRate = Cvar_Get(
            "health_recharge_rate", "30", 256);
    }
    float v5 = damageDelay;
    if (damageDelay == -1.0f)
        v5 = HealthRegenTask_sDamageDelay->value * 0.001f;
    mDamageDelay = v5;
    if (rechargeRate == -1.0f)
        mRechargeRate = HealthRegenTask_sRechargeRate->value;
    else
        mRechargeRate = rechargeRate;
}

// ============================================================================
// HealthRegenTask::Update - ea: 0x4F9AC0
// ============================================================================
extern vmCvar_t hud_healthOverlay_pulseStart;
extern vmCvar_t g_player_maxhealth;
extern HashString sDamageStr_0;

void HealthRegenTask::Update(Entity* e, float deltaT)
{
    if ((mFlags & 4) == 0 && e->health > 0)
    {
        if (e->mNotifySet != nullptr
            && EntityNotifySet_GetNotify(e->mNotifySet, sDamageStr_0.mHash)
                != nullptr)
        {
            mTimeSinceDamage = mDamageDelay;
            mTimeSinceVeryHurt = 0.0f;
            mTimeSinceRecoverySound = 0.0f;
        }
        bool mVeryHurt = this->mVeryHurt;
        if (hud_healthOverlay_pulseStart.value
            <= (e->health / g_player_maxhealth.value))
        {
            mTimeSinceVeryHurt = mTimeSinceVeryHurt + deltaT;
            mTimeSinceRecoverySound = mTimeSinceRecoverySound + deltaT;
        }
        else
        {
            this->mVeryHurt = true;
        }
        if (!mVeryHurt)
        {
            mTimeSinceVeryHurt = 0.0f;
            mTimeSinceRecoverySound = 0.0f;
        }
        if (mTimeSinceDamage > 0.0f)
            mTimeSinceDamage = mTimeSinceDamage - deltaT;
        if (mTimeSinceDamage <= 0.0f && e->health < g_player_maxhealth.integer)
            mHealthDelta = (mRechargeRate * deltaT) + mHealthDelta;
        if (mHealthDelta > 1.0f)
        {
            int v8 = (int)mHealthDelta;
            mHealthDelta = mHealthDelta - (float)(int)mHealthDelta;
            int integer = (int)mHealthDelta + e->health;
            if (integer >= g_player_maxhealth.integer)
                integer = g_player_maxhealth.integer;
            e->health = integer;
            if (integer == g_player_maxhealth.integer)
                this->mVeryHurt = false;
            if (mTimeSinceRecoverySound > mDamageDelay)
            {
                mTimeSinceRecoverySound = 0.0f;
                if (gpBrocAPI->mBrocExports.mCallbackHealthRegenRecovering
                    != nullptr)
                {
                    Broc::entity ent{e->mHandle.mHandle.mVal};
                    gpBrocAPI->mBrocExports.mCallbackHealthRegenRecovering(ent);
                }
            }
            if (e->client != nullptr)
            {
                // Decrement the client's damage-scaled health display fields.
                for (int i = 342; i < 358; i += 4)
                {
                    float* base = &e->client->ps.origin.v.m128_f32[i - 1];
                    base[0] = (base[0] - v8 <= 0) ? 0 : base[0] - v8;
                    base[1] = (base[1] - v8 <= 0) ? 0 : base[1] - v8;
                    base[2] = (base[2] - v8 <= 0) ? 0 : base[2] - v8;
                    base[3] = (base[3] - v8 <= 0) ? 0 : base[3] - v8;
                }
            }
        }
    }
}

// ============================================================================
// AnimationPlayer note handler + play method (game2.o)
// ============================================================================
struct AnimNoteHandler {
    DbLinkedHandle<EntityHandleDb, Entity> mEntHandle;  // +0x00
    void* mNotify;       // +0x04
    int mNotifyIndex;    // +0x08
};

class AnimationPlayer {
public:
    struct nalPlayMethod {
        void** __vftable;       // +0x00
        AnimNoteHandler* mNoteHandler;  // +0x04

        nalPlayMethod();       // ea: 0x4FA500
        void SetNoteHandlerEntityHandle(
            DbLinkedHandle<EntityHandleDb, Entity> handle);  // ea: 0x4F5D10
        void Release();  // nalPlayMethod::Release (thunk)
    };

    struct nalAnimCallback {
        void** __vftable;       // +0x00
        virtual bool Invoke(AnimationPlayer* player);  // ea: 0x4F5F10
    };
};

extern void* nalPlayMethod_vftable;   // ??_7nalPlayMethod@AnimationPlayer@@6B@
extern void* mem_heap_malloc(unsigned int size);

// ea: 0x4FA500
AnimationPlayer::nalPlayMethod::nalPlayMethod()
{
    __vftable = (void**)&nalPlayMethod_vftable;
    mNoteHandler = nullptr;
    AnimNoteHandler* v2 = (AnimNoteHandler*)mem_heap_malloc(0xC);
    if (v2 != nullptr)
    {
        v2->mEntHandle.mHandle.mVal = 0;
        v2->mNotify = nullptr;
        v2->mNotifyIndex = -1;
        mNoteHandler = v2;
    }
    else
    {
        mNoteHandler = nullptr;
    }
}

// ea: 0x4F5D10
void AnimationPlayer::nalPlayMethod::SetNoteHandlerEntityHandle(
    DbLinkedHandle<EntityHandleDb, Entity> handle)
{
    AnimNoteHandler* mNoteHandler = this->mNoteHandler;
    if (mNoteHandler != nullptr)
        mNoteHandler->mEntHandle = handle;
}

// ea: 0x4F5F10
bool AnimationPlayer::nalAnimCallback::Invoke(AnimationPlayer* player)
{
    (void)player;
    return true;
}

// ============================================================================
// TestFPS::TestFPS - ea: 0x4FEC20
// ============================================================================
TestFPS::TestFPS()
{
    memset(_pad, 0, sizeof(_pad));
    mStats_size = 0;
    mCells_size = 0;
    mTesting = false;
    mCellIndex = 0;
    mCurrentAngle = 0;
    mCellX = 0;
    mCellY = 0;
    mZoneIndex = 0;
    mDeltaAngle = 45;
    mDelta = 0.0f;
    mDeltaInverse = 0.0f;
    mCurrentPositionIndex = 0;
    mPlayerHandle.mVal = 0;
    mFile = nullptr;
}

struct PerformanceStats {
    int   mFrame;       // +0x00
    int   mDrawn;       // +0x04
    float mPos[3];      // +0x08
    int   mNodes;       // +0x14
    float mDrawMs;      // +0x18
    float mFrameMs;     // +0x1C
    float mTotalMs;     // +0x20
    int   mPolys;       // +0x24
    int   mVertices;    // +0x28
};
static_assert(sizeof(PerformanceStats) == 0x2C,
              "PerformanceStats size mismatch");

extern bool gUseNfl;  // ?gUseNfl (game2.o)
extern char buffer[0x4000];  // ?buffer (game2.o)
extern char temp[0x400];     // ?temp (game2.o)

// ea: 0x4FEC80
void TestFPS::OutputStats()
{
    int m_size = 0;
    if (mStats_size != 0)
    {
        int length = 0;
        char filename[256];
        GetFilename(filename);
        if (_stricmp(filename, mLastFile) == 0)
        {
            if (mFile == nullptr)
            {
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
        }
        else
        {
            if (mFile != nullptr)
            {
                gUseNfl = false;
                fclose((FILE*)mFile);
                mFile = nullptr;
            }
            gUseNfl = false;
            mFile = fopen(filename, "w");
            gUseNfl = true;
            strcpy(mLastFile, filename);
        }
        memset(buffer, 0, 0x4000u);
        buffer[0] = 0;
        temp[0] = 0;
        if (mFile != nullptr)
        {
            int count = mStats_size;
            for (int i = 0; i < count; ++i)
            {
                PerformanceStats* stats =
                    reinterpret_cast<PerformanceStats*>(_pad);
                PerformanceStats& s = stats[i];
                sprintf(temp,
                        "%i, %i, %g %g %g, %i, %g, %g, %g, %i, %i, \n",
                        s.mFrame, s.mDrawn, s.mPos[0], s.mPos[1], s.mPos[2],
                        s.mNodes, s.mDrawMs, s.mFrameMs, s.mTotalMs,
                        s.mPolys, s.mVertices);
                length += (int)strlen(temp);
                strcat(buffer, temp);
                if (length > 14336)
                {
                    gUseNfl = false;
                    fwrite(buffer, 1, length, (FILE*)mFile);
                    memset(buffer, 0, 0x4000u);
                    gUseNfl = true;
                    buffer[0] = 0;
                    length = 0;
                }
            }
            gUseNfl = false;
            fwrite(buffer, 1, length, (FILE*)mFile);
            gUseNfl = true;
        }
        else
        {
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning("Couldn't open %s for writing", filename))
                __debugbreak();
        }
        fflush((FILE*)mFile);
        mStats_size = 0;
    }
    (void)m_size;
}

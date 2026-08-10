// ============================================================================
// g_debugthread.cpp - DebugThread message helpers + Task/HealthRegenTask
// (game2.o). Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "game/logic/g_inspector.h"
#include "core/tlFixedString.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int nslSourceID;  // nsl.cpp stub surface

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

// SoundDevice::Sound - 0x3C, verified against IDA local type
struct SoundDeviceSound {
    unsigned int mSource;        // +0x00 (nslSourceID)
    unsigned int mWave;          // +0x04 (nslWaveID)
    bool mPaused;                // +0x08
    bool mAutoRelease;           // +0x09
    float mPitch;                // +0x0C
    float mVolume;               // +0x10
    float mMinRange;             // +0x14
    float mMaxRange;             // +0x18
    float mGroupVolume;          // +0x1C
    unsigned int mEntHandle;     // +0x20 (DbLinkedHandle mVal)
    unsigned int mHandle;        // +0x24 (DbLinkedHandle mVal)
    const void* mPoPtr;          // +0x28
    unsigned int mDialogNotify;  // +0x2C (HashString)
    float mDebugPos[3];          // +0x30
};
static_assert(sizeof(SoundDeviceSound) == 0x3C,
              "SoundDeviceSound size mismatch");

extern DbLinkedHandle<EntityHandleDb, Entity> g_SoundOnlyPlay;  // ?g_SoundOnlyPlay@@3V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@A (game2.o)
extern vmCvar_t sound_disableAllOtherSounds;   // ?sound_disableAllOtherSounds@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t sound_showSoundStatForEntity;  // ?sound_showSoundStatForEntity@@3UvmCvar_t@@A (game2.o)
extern vmCvar_t g_debugProneCheck;             // ?g_debugProneCheck@@3UvmCvar_t@@A (g.o)
extern vmCvar_t g_debugProneCheckDepthCheck;   // ?g_debugProneCheckDepthCheck@@3UvmCvar_t@@A (g.o)
extern const char* nslGetSourceName(nslSourceID sid);   // ?nslGetSourceName@@YAPBDW4nslSourceID@@@Z (nslSource.o)
extern float nslGetSourceParam(nslSourceID sid, int index, float defaultValue);  // ?nslGetSourceParam@@YAMW4nslSourceID@@HM@Z
extern void nslGetSourcePosition(nslSourceID sid, float* position);  // ?nslGetSourcePosition@@YAXW4nslSourceID@@QAM@Z
extern bool SoundDevice_Sound_IsFinished(const SoundDeviceSound* self);  // ?IsFinished@Sound@SoundDevice@@QBE_NXZ

#define NSL_SOURCE_ID_INVALID ((nslSourceID)-1)

// ============================================================================
// DebugThread::DisplayEntitySound - ea: 0x4F8AB0
// ============================================================================
void DebugThread::DisplayEntitySound(const math::Position3* entityPos,
                                     int xpos, int ypos, int yinc, float scale)
{
    if (sound_disableAllOtherSounds.integer == 1)
        g_SoundOnlyPlay.mHandle.mVal = m_entityHandle.mHandle.mVal;
    else
        g_SoundOnlyPlay.mHandle.mVal = (unsigned int)-1;
    if (sound_showSoundStatForEntity.integer != 1)
        return;
    char tmpstr[128];
    for (unsigned int i = 0; i < 512; ++i)
    {
        SoundDeviceSound* v11 =
            (SoundDeviceSound*)((char*)SoundDevice::sInst + i * 0x3C);
        if (v11->mEntHandle != m_entityHandle.mHandle.mVal
            || v11->mSource == NSL_SOURCE_ID_INVALID
            || SoundDevice_Sound_IsFinished(v11))
        {
            continue;
        }
        float minVal = v11->mMinRange;
        nslSourceID id = v11->mSource;
        float maxVal = v11->mMaxRange;
        const char* SourceName = nslGetSourceName(id);
        int v13 = yinc + ypos;
        sprintf(tmpstr, "Sound Name: %s ", SourceName);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v13, scale);
        double SourceParam = nslGetSourceParam(id, 0, -1.0f);
        int v15 = yinc + v13;
        sprintf(tmpstr, "Sound Vol: %f ", SourceParam);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v15, scale);
        int v16 = yinc + v15;
        sprintf(tmpstr, "Min: %f  Max: %f", minVal, maxVal);
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v16, scale);
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        float camPos = Player->r.currentOrigin.v.m128_f32[0];
        float v23 = Player->r.currentOrigin.v.m128_f32[1];
        float v24 = Player->r.currentOrigin.v.m128_f32[2];
        float soundPos[3];
        nslGetSourcePosition(id, soundPos);
        float vDelta = camPos - soundPos[0];
        float v29 = v23 - soundPos[1];
        float v30 = v24 - soundPos[2];
        int v17 = yinc + v16;
        sprintf(tmpstr,
                "Distance from source (2d-top down view): %f ",
                sqrtf(v30 * v30 + v29 * v29 + vDelta * vDelta));
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, v17, scale);
        ypos = yinc + v17;
        sprintf(tmpstr,
                "Distance from source (3d): %f ",
                sqrtf((v23 - soundPos[1]) * (v23 - soundPos[1])
                      + (camPos - soundPos[0]) * (camPos - soundPos[0])));
        g_inspectorManager.m_currentRgba[0] = 0.0f;
        g_inspectorManager.m_currentRgba[1] = 0.8f;
        g_inspectorManager.m_currentRgba[2] = 1.0f;
        g_inspectorManager.m_currentRgba[3] = 1.0f;
        g_inspectorManager.Print(tmpstr, xpos, ypos, scale);
        float vUp[3] = { 0.0f, 0.0f, 1.0f };
        float srcZ = soundPos[2];
        soundPos[2] = srcZ + 25.0f;
        if (g_debugProneCheck.integer != 0)
        {
            G_DebugCircle2Ex(soundPos, minVal, vUp, colorGreen,
                             g_debugProneCheckDepthCheck.integer, 1);
        }
        soundPos[2] = srcZ + 30.0f;
        if (g_debugProneCheck.integer != 0)
        {
            G_DebugCircle2Ex(soundPos, maxVal, vUp, colorRed,
                             g_debugProneCheckDepthCheck.integer, 1);
        }
    }
}

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
        ~nalPlayMethod();      // ea: 0x50BB30
        void Advance(void* state, float delta);  // ea: 0x50BB80
        void* CreateInstance(void* anim, void* skeleton);  // ea: 0x504C60
        void SetNoteHandlerEntityHandle(
            DbLinkedHandle<EntityHandleDb, Entity> handle);  // ea: 0x4F5D10
        void Release();  // nalPlayMethod::Release (thunk)
    };

    struct nalAnimCallback {
        void** __vftable;       // +0x00
        virtual bool Invoke(AnimationPlayer* player);  // ea: 0x4F5F10
    };

    void Advance(float delta);  // ea: 0x4FA550
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
// MetaNalBaseAnim - meta-animation wrapper (0x44, IDA verified)
// ============================================================================
struct MetaAnimData {
    void** __vftable;  // +0x00
    // vtable slots:
    //   [0] GetAnimName() -> const tlFixedString*
    //   [1] IsAnimLooping() -> int
    //   [2] IsAnimTrajRelative() -> int
    //   [3] GetAnimDuration() -> float
    //   [4] GetSkeleton() -> nalBaseSkeleton*
    //   [5] DelayCreate(nalAnimClass**, int)
    //   [6] IsDelayCreate() -> int
    //   [7] CreateAnimInst(...)
};

struct nalBaseSkeleton;
struct nalInstanceClass;

struct MetaNalBaseAnim {
    void** __vftable;           // +0x00
    tlFixedString Name;         // +0x04 (32 bytes)
    unsigned char _pad24[0x30 - 0x24];  // +0x24 (nalAnimClass extra)
    void* Skeleton;             // +0x30
    unsigned int Flags;         // +0x34
    float Duration;             // +0x38
    unsigned char _pad3C[4];    // +0x3C (nalAnimClass instance ptr area)
    MetaAnimData* mData;        // +0x40

    MetaNalBaseAnim();                       // ea: 0x4F5F20
    void Create(MetaAnimData* theMetaAnimData);  // ea: 0x4F5F50
    void DelayCreate(void** animArray, int numAnims);  // ea: 0x4F5FE0
    void DelayCreate(void* anim);            // ea: 0x4FAE20
    int IsDelayCreate();                     // ea: 0x4F6010
    void* CreateAnimInst(nalBaseSkeleton* theSkel);  // ea: 0x4F6020
};
static_assert(sizeof(MetaNalBaseAnim) == 0x44, "MetaNalBaseAnim size mismatch");

extern void* MetaNalBaseAnim_vftable;  // ??_7MetaNalBaseAnim@@6B@

typedef void* (*GetAnimNameFn)(void* self);
typedef int (*IsAnimLoopingFn)(void* self);
typedef int (*IsAnimTrajRelativeFn)(void* self);
typedef float (*GetAnimDurationFn)(void* self);
typedef void* (*GetSkeletonFn)(void* self);
typedef void (*DelayCreateFn)(void* self, void** animArray, int numAnims);
typedef int (*IsDelayCreateFn2)(void* self);
typedef void* (*CreateAnimInstFn)(void* self, void* theSkel, void* metaAnim);

// ea: 0x4F5F20
MetaNalBaseAnim::MetaNalBaseAnim()
{
    memset(&Name, 0, sizeof(Name));
    __vftable = (void**)&MetaNalBaseAnim_vftable;
    mData = nullptr;
}

// ea: 0x4F5F50
void MetaNalBaseAnim::Create(MetaAnimData* theMetaAnimData)
{
    mData = theMetaAnimData;
    Flags = 0;
    void** vt = (void**)theMetaAnimData->__vftable;
    if (((IsAnimLoopingFn)vt[1])(theMetaAnimData) != 0)
        Flags |= 1u;
    if (((IsAnimTrajRelativeFn)mData->__vftable[2])(mData) != 0)
        Flags |= 2u;
    Duration = ((GetAnimDurationFn)mData->__vftable[3])(mData);
    Skeleton = ((GetSkeletonFn)mData->__vftable[4])(mData);
    const unsigned int* v5 = (const unsigned int*)
        ((GetAnimNameFn)mData->__vftable[0])(mData);
    Name.hash = v5[0];
    memcpy(Name.str, v5 + 1, 28);
}

// ea: 0x4F5FE0
void MetaNalBaseAnim::DelayCreate(void** animArray, int numAnims)
{
    ((DelayCreateFn)mData->__vftable[5])(mData, animArray, numAnims);
    Create(mData);
}

// ea: 0x4FAE20
void MetaNalBaseAnim::DelayCreate(void* anim)
{
    void* animArray = anim;
    ((DelayCreateFn)mData->__vftable[5])(mData, &animArray, 1);
    Create(mData);
}

// ea: 0x4F6010
int MetaNalBaseAnim::IsDelayCreate()
{
    return ((IsDelayCreateFn2)mData->__vftable[6])(mData);
}

// ea: 0x4F6020
void* MetaNalBaseAnim::CreateAnimInst(nalBaseSkeleton* theSkel)
{
    return ((CreateAnimInstFn)mData->__vftable[7])(mData, theSkel, this);
}

// ============================================================================
// TaskHandler - task dispatch handler (0x30, IDA verified)
// ============================================================================
struct DListNode {
    DListNode* m_next;  // +0x00
    DListNode* m_prev;  // +0x04
};

struct DList {
    DListNode m_end;     // +0x00
    DListNode* m_head;   // +0x08
    DListNode** m_tail;  // +0x0C
    int m_size;          // +0x10
};

struct QuickTaskDeactivation {
    DListNode node;  // next/prev
    unsigned int mEntHandle;  // +0x08
};

struct TaskHandlerImpl {
    void* m_dlist[2];    // +0x00
    unsigned int mTaskId;  // +0x08
    unsigned int mFlags;   // +0x0C
    DList mTaskList;       // +0x10
    DList mQuickDeactivationList;  // +0x20

    TaskHandlerImpl(unsigned int task_id, unsigned int flags);
    ~TaskHandlerImpl();
    void QuickDeactivation(DbLinkedHandle<EntityHandleDb, Entity> h);
    void DeactivateAll();
    Task* GetTaskForEntity(DbLinkedHandle<EntityHandleDb, Entity> h);
    void Update(float deltaT, void* ftor);
};

struct TaskSysImpl2 {
    TaskHandlerImpl* mTaskHandlers[32];  // +0x00
    int m_size;                          // +0x80
    DList mPostQueue;                    // +0x84
    void* mHandleDb[8];                  // +0x98 (HandleDb<Task,32,...>)
    static TaskSysImpl2* sInst;          // ?sInst@TaskSys@@0V1@A
};

extern TaskSysImpl2* TaskSysImpl2_sInst;
extern void ae_sized_array_push_back_handler(TaskSysImpl2* self,
                                             TaskHandlerImpl* const* elt);
extern void* mem_heap_malloc_sz(unsigned int size);
extern void HandleDb_ReleaseTaskHandle(void* self, Handle h);

// ea: 0x4FFB20
TaskHandlerImpl::TaskHandlerImpl(unsigned int task_id, unsigned int flags)
{
    mFlags = flags;
    mTaskId = task_id;
    m_dlist[0] = nullptr;
    m_dlist[1] = nullptr;
    mTaskList.m_end.m_next = nullptr;
    mTaskList.m_end.m_prev = nullptr;
    mTaskList.m_head = &mTaskList.m_end;
    mTaskList.m_tail = &mTaskList.m_head;
    mTaskList.m_size = 0;
    mQuickDeactivationList.m_end.m_next = nullptr;
    mQuickDeactivationList.m_end.m_prev = nullptr;
    mQuickDeactivationList.m_head = &mQuickDeactivationList.m_end;
    mQuickDeactivationList.m_tail = &mQuickDeactivationList.m_head;
    mQuickDeactivationList.m_size = 0;
    TaskHandlerImpl* self = this;
    ae_sized_array_push_back_handler(TaskSysImpl2_sInst, &self);
}

// ea: 0x50BAC0
TaskHandlerImpl::~TaskHandlerImpl()
{
    // Delete owned quick-deactivation records (each is a heap block).
    DListNode* q = mQuickDeactivationList.m_head;
    while (q != nullptr && q != &mQuickDeactivationList.m_end)
    {
        DListNode* next = q->m_next;
        QuickTaskDeactivation* rec = (QuickTaskDeactivation*)q;
        mem_heap_free(rec);
        q = next;
    }
    // Delete owned task objects (dlist node is embedded in Task).
    DListNode* t = mTaskList.m_head;
    while (t != nullptr && t != &mTaskList.m_end)
    {
        DListNode* next = t->m_next;
        Task* task = (Task*)((char*)t - 0x4);
        task->~Task();
        mem_heap_free(task);
        t = next;
    }
}

// ea: 0x4FFB80
void TaskHandlerImpl::QuickDeactivation(
    DbLinkedHandle<EntityHandleDb, Entity> h)
{
    QuickTaskDeactivation* v3 =
        (QuickTaskDeactivation*)mem_heap_malloc_sz(0xC);
    if (v3 != nullptr)
    {
        v3->node.m_next = nullptr;
        v3->node.m_prev = nullptr;
        v3->mEntHandle = h.mHandle.mVal;
    }
    DListNode* tail = *mQuickDeactivationList.m_tail;
    v3->node.m_next = &mQuickDeactivationList.m_end;
    v3->node.m_prev = tail;
    tail->m_next = &v3->node;
    *mQuickDeactivationList.m_tail = &v3->node;
    ++mQuickDeactivationList.m_size;
}

// ============================================================================
// TaskHandler::DeactivateAll - ea: 0x504B80
// ============================================================================
void TaskHandlerImpl::DeactivateAll()
{
    mFlags |= 8u;
    DListNode* m_head = mTaskList.m_head;
    DListNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head != &mTaskList.m_end && m_next != nullptr)
    {
        do
        {
            // Task dlist node is embedded in Task; mFlags at +0x18
            unsigned int* flags = (unsigned int*)((char*)m_head + 0x14);
            *flags |= 4u;
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
}

// ============================================================================
// TaskHandler::GetTaskForEntity - ea: 0x504BC0
// ============================================================================
Task* TaskHandlerImpl::GetTaskForEntity(
    DbLinkedHandle<EntityHandleDb, Entity> h)
{
    DListNode* m_head = mTaskList.m_head;
    DListNode* m_next = m_head != nullptr ? m_head->m_next : nullptr;
    if (m_head == &mTaskList.m_end || m_next == nullptr)
        return nullptr;
    while (*(unsigned int*)((char*)m_head + 0x14) != h.mHandle.mVal)
    {
        m_head = m_next;
        m_next = m_next->m_next;
        if (m_next == nullptr)
            return nullptr;
    }
    return (Task*)((char*)m_head - 0x4);
}

// ============================================================================
// TaskHandler::Update - ea: 0x504990
// ============================================================================
extern Entity* EntityHandleDb_GetObject(unsigned int val);

void TaskHandlerImpl::Update(float deltaT, void* ftor)
{
    // Apply quick-deactivation records: mark those entities' tasks.
    DListNode* q = mQuickDeactivationList.m_head;
    while (q != nullptr && q != &mQuickDeactivationList.m_end)
    {
        QuickTaskDeactivation* rec = (QuickTaskDeactivation*)q;
        unsigned int entVal = rec->mEntHandle;
        DListNode* next = q->m_next;
        Entity* ent = EntityHandleDb_GetObject(entVal);
        if (ent != nullptr)
            ent->mFlags |= 4u;
        mem_heap_free(rec);
        q = next;
    }
    mQuickDeactivationList.m_head = &mQuickDeactivationList.m_end;
    mQuickDeactivationList.m_size = 0;

    // Run each task's Update.
    DListNode* t = mTaskList.m_head;
    while (t != nullptr && t != &mTaskList.m_end)
    {
        Task* task = (Task*)((char*)t - 0x4);
        DListNode* next = t->m_next;
        if (ftor != nullptr)
        {
            // TaskFunctor path: fn(task, entity)
            ((void(*)(Task*, void*))ftor)(task, nullptr);
        }
        else
        {
            // Task::Update(Entity*, float)
            typedef void (*UpdateFn)(Task*, Entity*, float);
            void** vt = *(void***)task;
            UpdateFn fn = (UpdateFn)vt[4];  // vtable slot 4 = Update
            Entity* e = EntityHandleDb_GetObject(
                task->mEntityHandle.mHandle.mVal);
            fn(task, e, deltaT);
        }
        t = next;
    }
}

// ============================================================================
// TaskSys::ReleaseTask - ea: 0x504970
// ============================================================================
void TaskSys_ReleaseTask(Task* t)
{
    if (t->mTaskHandle.mVal != 0)
        HandleDb_ReleaseTaskHandle(&TaskSysImpl2_sInst->mHandleDb,
                                   t->mTaskHandle);
}

// ============================================================================
// TaskSys::Update - ea: 0x50B8E0
// ============================================================================
extern TaskHandlerImpl* HealthRegenTask_sHandler;
extern TaskHandlerImpl* AnimNotifyTask_sHandler;
extern TaskHandlerImpl* EntityDeathTask_sHandler;
extern void TaskHandler_Update(TaskHandlerImpl* self, float deltaT,
                               void* ftor);
extern TaskHandlerImpl* TaskSys_LookupHandler(unsigned int id);

void TaskSys_Update(float deltaT)
{
    TaskHandler_Update(HealthRegenTask_sHandler, deltaT, nullptr);
    TaskHandler_Update(AnimNotifyTask_sHandler, deltaT, nullptr);
    TaskHandler_Update(EntityDeathTask_sHandler, deltaT, nullptr);
}

// ============================================================================
// TaskSys::DeactivateTask - ea: 0x50B9C0
// ============================================================================
extern Task* HandleDb_GetTask(void* self, Handle h);  // GetObject on Task HandleDb

void TaskSys_DeactivateTask(Handle taskHandle)
{
    Task* t = HandleDb_GetTask(&TaskSysImpl2_sInst->mHandleDb, taskHandle);
    if (t != nullptr)
        t->mFlags |= 4u;
}

// ============================================================================
// TaskSys::PostTaskAndAllocateHandle - ea: 0x50D810
// ============================================================================
extern void TaskSys_PostTask_glue(Task* t);
extern Handle TaskSys_CreateTaskHandle(Task* t);

Handle TaskSys_PostTaskAndAllocateHandle(Task* t)
{
    TaskSys_PostTask_glue(t);
    return TaskSys_CreateTaskHandle(t);
}

// ============================================================================
// TaskSys::GetTaskForEntity - ea: 0x50BA90
// ============================================================================
extern Task* TaskHandler_GetTaskForEntity(TaskHandlerImpl* self,
    DbLinkedHandle<EntityHandleDb, Entity> h);
extern TaskHandlerImpl* TaskSys_LookupHandler(unsigned int id);

Task* TaskSys_GetTaskForEntity(unsigned int taskId,
    DbLinkedHandle<EntityHandleDb, Entity> eh)
{
    TaskHandlerImpl* v3 = TaskSys_LookupHandler(taskId);
    if (v3 != nullptr)
        return TaskHandler_GetTaskForEntity(v3, eh);
    return nullptr;
}

// ============================================================================
// TaskSys::CreateTaskHandle - ea: 0x50BA00
// ============================================================================
extern void HandleDb_AllocateTaskHandle(void* self, Task** t);
extern void HandleDb_BindTaskObject(void* self, Handle h, Task* obj);

Handle TaskSys_CreateTaskHandle(Task* t)
{
    if (t->mTaskHandle.mVal != 0)
    {
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("handle already assigned"))
            __debugbreak();
    }
    Task* pt = t;
    HandleDb_AllocateTaskHandle(&TaskSysImpl2_sInst->mHandleDb, &pt);
    t->mTaskHandle.mVal = pt->mTaskHandle.mVal;
    HandleDb_BindTaskObject(&TaskSysImpl2_sInst->mHandleDb,
                            t->mTaskHandle, t);
    return t->mTaskHandle;
}

// ============================================================================
// TaskSys::ShutDown - ea: 0x50B920
// ============================================================================
extern void TaskSys_DeliverTasks_glue();

void TaskSys_ShutDown()
{
    TaskSys_DeliverTasks_glue();
    int count = TaskSysImpl2_sInst->m_size;
    for (int i = 0; i < count; ++i)
    {
        TaskHandlerImpl* handler = TaskSysImpl2_sInst->mTaskHandlers[i];
        if (handler != nullptr)
            handler->DeactivateAll();
    }
    TaskHandler_Update(HealthRegenTask_sHandler, 0.01f, nullptr);
    TaskHandler_Update(AnimNotifyTask_sHandler, 0.01f, nullptr);
    TaskHandler_Update(EntityDeathTask_sHandler, 0.01f, nullptr);
}

// ============================================================================
// TaskSys::DeliverTasks - ea: 0x4FF9D0
// ============================================================================
extern void TaskSys_DeliverTasks();

void TaskSys_DeliverTasks()
{
    TaskSysImpl2* sys = TaskSysImpl2_sInst;
    // Move each posted task from mPostQueue to its handler's mTaskList.
    int count = sys->mPostQueue.m_size;
    while (count > 0)
    {
        DListNode* head = sys->mPostQueue.m_head;
        Task* task = (Task*)((char*)head - 0x4);
        sys->mPostQueue.m_head = head->m_next;
        --sys->mPostQueue.m_size;
        unsigned int taskId = *(unsigned int*)((char*)head + 0x8);
        TaskHandlerImpl* handler = TaskSys_LookupHandler(taskId);
        if (handler != nullptr)
        {
            DListNode* node = (DListNode*)&task->_dlist[0];
            node->m_next = &handler->mTaskList.m_end;
            node->m_prev = *handler->mTaskList.m_tail;
            (*handler->mTaskList.m_tail)->m_next = node;
            *handler->mTaskList.m_tail = node;
            ++handler->mTaskList.m_size;
        }
        --count;
    }
}

// ============================================================================
// AnimationPlayer::nalPlayMethod dtor - ea: 0x50BB30
// ============================================================================
extern void mem_heap_free(void* ptr);

AnimationPlayer::nalPlayMethod::~nalPlayMethod()
{
    __vftable = (void**)&nalPlayMethod_vftable;
    AnimNoteHandler* mNoteHandler = this->mNoteHandler;
    if (mNoteHandler != nullptr)
    {
        // NotifyInfo block: [count][NotifyInfo...] freed as one heap block
        mem_heap_free(mNoteHandler->mNotify != nullptr
                          ? (char*)mNoteHandler->mNotify - 4
                          : nullptr);
        mNoteHandler->mNotify = nullptr;
        mem_heap_free(mNoteHandler);
    }
}

// ============================================================================
// AnimationPlayer::nalPlayMethod::Advance - ea: 0x50BB80
// ============================================================================
struct nalAnimStateView {
    void* instance;      // +0x00
    float speed;         // +0x04
    float t;             // +0x08
};

extern void AnimNoteHandler_Advance(void* self, float t);

void AnimationPlayer::nalPlayMethod::Advance(void* state, float delta)
{
    nalAnimStateView* st = (nalAnimStateView*)state;
    float v3 = (*(float*)((char*)st->instance + 0x14)
                * st->speed) * delta + st->t;
    st->t = v3;
    if (mNoteHandler != nullptr)
        AnimNoteHandler_Advance(mNoteHandler, v3);
}

// ============================================================================
// AnimationPlayer::nalPlayMethod::CreateInstance - ea: 0x504C60
// ============================================================================
extern void* nalGenericAnim_CreateInstance(void* anim, void* skeleton);
extern void AnimNoteHandler_ParseNoteTracks(void* self, void* anim);

void* AnimationPlayer::nalPlayMethod::CreateInstance(void* anim, void* skeleton)
{
    void* instance = nalGenericAnim_CreateInstance(anim, skeleton);
    if (mNoteHandler != nullptr)
        AnimNoteHandler_ParseNoteTracks(mNoteHandler, anim);
    return instance;
}

// ============================================================================
// AnimationPlayer queue state views (game2.o AnimationPlayer.cpp)
// ============================================================================
struct nalAnimStateLocal {
    void* instance;        // +0x00
    float speed;           // +0x04
    float t;               // +0x08
    void* play_method;     // +0x0C nalPlayMethod*
    void* callback;        // +0x10 nalAnimCallback*
    void* next;            // +0x14
    float weight;          // +0x18
};
struct nalPartialAnimStateLocal {
    void* instance;        // +0x00
    float alpha;           // +0x04
    float t;               // +0x08
    void* play_method;     // +0x0C
    void* callback;        // +0x10
    void* next;            // +0x14
    int type;              // +0x18
    int CreationAdvanceCount;  // +0x1C
};
struct AnimationPlayerLocal {
    void* BackgroundPose;  // +0x00 nalGenericPose*
    int AdvanceCount;      // +0x04
    int QueueSize;         // +0x08
    void* AnimStates;      // +0x0C nalAnimState*[8]
    void* PartialAnimStates;   // +0x10
    void* PartialAnimStatePool;  // +0x14
};

extern bool AnimationPlayer_nalPartialAnimState_Update(void* self,
                                                       void* player,
                                                       float delta);  // ?Update@nalPartialAnimState@AnimationPlayer@@QAE_NPAV2@M@Z (game2.o)
extern bool AnimationPlayer_nalAnimState_Update(void* self, void* player,
                                                float delta);  // ?Update@nalAnimState@AnimationPlayer@@QAE_NPAV2@M@Z
extern void AnimationPlayer_nalAnimState_Compose(void* self, void* pose,
                                                 void* tmpPose);  // ?Compose@nalAnimState@AnimationPlayer@@QAEXPAVnalGenericPose@nalGeneric@@0@Z

// ============================================================================
// AnimationPlayer::Advance - ea: 0x4FA550
// ============================================================================
void AnimationPlayer::Advance(float delta)
{
    AnimationPlayerLocal* self = (AnimationPlayerLocal*)this;
    ++self->AdvanceCount;
    void** p_PartialAnimStates = &self->PartialAnimStates;
    void* PartialAnimStates = self->PartialAnimStates;
    while (PartialAnimStates != nullptr)
    {
        nalPartialAnimStateLocal* ps =
            (nalPartialAnimStateLocal*)PartialAnimStates;
        if (ps->CreationAdvanceCount != self->AdvanceCount)
        {
            if (AnimationPlayer_nalPartialAnimState_Update(ps, this, delta))
            {
                if (*p_PartialAnimStates != PartialAnimStates)
                {
                    void* cur = *p_PartialAnimStates;
                    do
                    {
                        p_PartialAnimStates =
                            &((nalPartialAnimStateLocal*)cur)->next;
                        cur = *p_PartialAnimStates;
                    } while (cur != PartialAnimStates);
                }
                *p_PartialAnimStates = ps->next;
                if (ps->callback != nullptr)
                    (*(void(**)(void*))ps->callback)(ps->callback);
                if (ps->play_method != nullptr)
                    (*(void(**)(void*))ps->play_method)(ps->play_method);
                if (ps->instance != nullptr)
                    (*(void(**)(void*, int))ps->instance)(ps->instance, 1);
                ps->next = self->PartialAnimStatePool;
                self->PartialAnimStatePool = ps;
            }
            else
            {
                p_PartialAnimStates = &ps->next;
            }
            PartialAnimStates = *p_PartialAnimStates;
        }
    }
    int v9 = 0;
    if (self->QueueSize > 0)
    {
        for (;;)
        {
            void** v10 = &((void**)self->AnimStates)[v9];
            void* cur = *v10;
            bool v11 = AnimationPlayer_nalAnimState_Update(cur, this, delta);
            int QueueSize = self->QueueSize;
            if (v9 < QueueSize)
            {
                do
                {
                    if (*v10 == cur)
                        break;
                    ++v9;
                    ++v10;
                } while (v9 < self->QueueSize);
            }
            if (v11 && v9 < QueueSize)
                break;
            if (++v9 >= self->QueueSize)
                break;
        }
        ++v9;
    }
    int newSize = v9;
    if (v9 < self->QueueSize)
    {
        void** deltaa = &((void**)self->AnimStates)[v9];
        do
        {
            nalAnimStateLocal* st = (nalAnimStateLocal*)*deltaa;
            if (st->callback != nullptr)
                (*(void(**)(void*))st->callback)(st->callback);
            if (st->play_method != nullptr)
                (*(void(**)(void*))st->play_method)(st->play_method);
            if (st->instance != nullptr)
                (*(void(**)(void*, int))st->instance)(st->instance, 1);
            ++v9;
            ++deltaa;
        } while (v9 < self->QueueSize);
    }
    self->QueueSize = newSize;
}

// ============================================================================
// TestFPS::TestFPS - ea: 0x4FEC20
// ============================================================================
TestFPS::TestFPS()
{
    memset(_pad, 0, sizeof(_pad));
    mStats_size = 0;
    mCells_size = 0;
    mBlock = 0;
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
    float mPosition[3];   // +0x00
    int   mPositionIndex; // +0x0C
    int   mCell;          // +0x10
    int   mAngle;         // +0x14
    float mDMATime;       // +0x18
    float mSceneSubmitTime;  // +0x1C
    float mRenderTime;    // +0x20
    int   mNodeCount;     // +0x24
    int   mPolyCount;     // +0x28
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
                        s.mPositionIndex, s.mCell,
                        s.mPosition[0], s.mPosition[1], s.mPosition[2],
                        s.mAngle, s.mDMATime, s.mSceneSubmitTime,
                        s.mRenderTime, s.mNodeCount, s.mPolyCount);
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

// ============================================================================
// TestFPS::StopTest - ea: 0x5018C0
// ============================================================================
extern int gStartTime;  // ?gStartTime (game2.o)
extern void Com_Printf(const char* fmt, ...);
extern void Cvar_Set(const char* var_name, const char* value);

void TestFPS::StopTest()
{
    if (mTesting)
    {
        OutputStats();
        if (mFile != nullptr)
        {
            fclose((FILE*)mFile);
            mFile = nullptr;
        }
        mTesting = false;
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        player->client->noclip = 0;
        player->client->bFrozen = 0;
        player->client->ps.pm_flags &= ~0x4000u;
        Cvar_Set("g_performanceTest", "0");
        int v3 = Sys_Milliseconds();
        Com_Printf(
            "Performance test finished in %i hours %i minutes %i seconds.\n",
            (int)((v3 - gStartTime) * 0.001) / 60 / 60,
            (int)((v3 - gStartTime) * 0.001) / 60 % 60,
            (int)((v3 - gStartTime) * 0.001) % 60);
    }
}

// ============================================================================
// TestFPS::GatherMetrics - ea: 0x501990
// ============================================================================
struct nglPerfInfoStruct {
    float RenderMS;
    float ListSubmitMS;
    float ListSendMS;
};
struct nglSyncPerfInfoStruct {
    int NodeCount;
    int TotalPolys;
};

extern nglPerfInfoStruct nglPerfInfo;
extern nglSyncPerfInfoStruct nglSyncPerfInfo;
extern vmCvar_t bg_viewheight_standing;

void TestFPS::GatherMetrics()
{
    if (mTesting)
    {
        float x = mCurrentPosition.x;
        float y = mCurrentPosition.y;
        float z = mCurrentPosition.z;
        mBlock = 0;
        PerformanceStats stat;
        memset(&stat, 0, sizeof(stat));
        stat.mPosition[2] = bg_viewheight_standing.integer + z;
        stat.mPosition[0] = x;
        stat.mPosition[1] = y;
        stat.mPositionIndex = mCurrentPositionIndex;
        stat.mDMATime = nglPerfInfo.ListSendMS;
        stat.mCell = mCellIndex;
        stat.mAngle = mCurrentAngle;
        stat.mSceneSubmitTime = nglPerfInfo.ListSubmitMS;
        stat.mNodeCount = nglSyncPerfInfo.NodeCount;
        stat.mRenderTime = nglPerfInfo.RenderMS;
        stat.mPolyCount = nglSyncPerfInfo.TotalPolys;
        PerformanceStats* stats = reinterpret_cast<PerformanceStats*>(_pad);
        stats[mStats_size] = stat;
        ++mStats_size;
        if (mStats_size >= 1000)
            OutputStats();
    }
}

// ============================================================================
// TestFPS::PositionCamera - ea: 0x509B50
// ============================================================================
extern void TeleportPlayer(Entity* player, const float* origin,
                           const float* angles);
extern PakManager* PakManager_sInst;  // ?sInst@PakManager@@2PAV1@A
extern char tr[0x3A0];  // ?tr@@3UtrGlobals_t@@A (render.o)

static int TestFPS_tr_cell_count()
{
    void* world = *(void**)(tr + 0x290);
    if (world == nullptr)
        return 0;
    void* bspTree = *(void**)((char*)world + 0x100);
    if (bspTree == nullptr)
        return 0;
    return *(int*)((char*)bspTree + 0x1C);  // mCells.mSize
}

void TestFPS::PositionCamera(pmove_t* pmove)
{
    PakManager_sInst->FillBanks();
    if (mBlock == 0)
    {
        mBlock = 1;
        Entity* player = EntityManager::sInst->GetPlayer(currCl);
        player->client->noclip = 1;
        player->client->bFrozen = 1;
        int mSize = TestFPS_tr_cell_count();
        mCurrentAngle += mDeltaAngle;
        pmove->ps->pm_flags |= 0x4000u;
        if (mCurrentAngle >= 360)
        {
            mCurrentAngle = 0;
            NextPosition();
        }
        if (mCellIndex >= mSize)
        {
            StopTest();
        }
        else
        {
            float angles[3] = { 0.0f, (float)mCurrentAngle, 0.0f };
            float position[3] = {
                mCurrentPosition.x, mCurrentPosition.y, mCurrentPosition.z
            };
            TeleportPlayer(player, position, angles);
        }
    }
}

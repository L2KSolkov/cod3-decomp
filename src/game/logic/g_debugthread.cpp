// ============================================================================
// g_debugthread.cpp - DebugThread message helpers + Task/HealthRegenTask
// (game2.o). Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/tlFixedString.h"

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

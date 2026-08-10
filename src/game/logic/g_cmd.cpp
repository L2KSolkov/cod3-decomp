// ============================================================================
// g_cmd.cpp - game.o command buffer / command registration (cmd.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/PoolAllocator.h"

#include <stdio.h>
#include <string.h>

// ============================================================================
// Command globals (game.o data)
// ============================================================================
struct cmd_t {
    char* data;     // +0x00
    int maxsize;    // +0x04
    int cmdsize;    // +0x08
};
static cmd_t cmd_text;               // ?cmd_text@@3Ucmd_t@@A (game.o)
static unsigned char cmd_text_buf[8192];
static cmd_t sv_cmd_text;            // ?sv_cmd_text@@3Ucmd_t@@A (game.o)
static unsigned char sv_cmd_text_buf[8192];
static int cmd_argc;                 // ?cmd_argc@@3HA (game.o)
static char* cmd_argv[512];
static char cmd_tokenized[8192];
static int cmd_wait;                 // ?cmd_wait@@3HA (game.o)

extern void Cmd_ExecuteString(const char* text);   // game.o 0x61F640
extern void Cbuf_SV_Execute();                     // game.o 0x61F3B0
extern void Cmd_List_f();                          // game.o 0x60ED20
extern void Cmd_Vstr_f();                          // game.o 0x61F550
extern void Cmd_Echo_f();                          // game.o 0x61F590
extern void Cmd_Wait_f();                          // game.o 0x61F280
void SetButtonAlias();                      // game.o 0x62B1A0
void SetStickAlias();                       // game.o 0x62B320
void ClearAllPadAliases();                  // game.o 0x62B4A0
void InitPadAliasCommands();                // game.o 0x62B4E0
void Cmd_AddCommand(const char* cmd_name, void (*function)());

enum ECmdFuncType { CMD = 0, INPUT_CMD = 1 };

struct BaseCmdFuncInfo {
    int mFuncType;          // +0x00
    const char* mName;      // +0x04
    BaseCmdFuncInfo* mNext; // +0x08
    void* mFuncPtr;         // +0x0C
    static int DoesFunctionExist(BaseCmdFuncInfo* cmd);  // ?DoesFunctionExist@BaseCmdFuncInfo@@SAHPAU1@@Z
};

// ea: 0x0060E450
void Cbuf_Init()
{
    cmd_text.data = (char*)cmd_text_buf;
    cmd_text.maxsize = 0x2000;
    cmd_text.cmdsize = 0;
    sv_cmd_text.data = (char*)sv_cmd_text_buf;
    sv_cmd_text.maxsize = 0x2000;
    sv_cmd_text.cmdsize = 0;
}

// ea: 0x0060E420
int BaseCmdFuncInfo::DoesFunctionExist(BaseCmdFuncInfo* cmd)
{
    int mFuncType = cmd->mFuncType;
    return (mFuncType == CMD && cmd[1].mNext != nullptr)
        || (mFuncType == INPUT_CMD && cmd[1].mNext != nullptr);
}

// ea: 0x0060F230
void Link_Init()
{
}

// ea: 0x0060F240
void Link_Frame()
{
}

// ============================================================================
// CurveManager - ea: 0x60F1B0..0x60F200
// ============================================================================
struct RemainingTime {
    int   mTrackId;       // +0x00
    int   mEntityId;      // +0x04
    float mRemainingTime; // +0x08
};

class CurveManager {
public:
    CurveManager();            // ??0CurveManager@@QAE@XZ (game.o 0x638160)
    virtual ~CurveManager();   // ??1CurveManager@@AAE@XZ (game.o 0x61F780)
    RemainingTime mRemainingTime[50];  // +0x04 (0x0C stride)
    struct CurveDList {
        int  m_size;  // +0x00
        void* m_end;  // +0x04
        void* m_head; // +0x08
        void** m_tail;// +0x0C
    };
    CurveDList mCurveList;           // +0x25C
    CurveDList mKeyEvaluators;       // +0x26C
    CurveDList mConditionEvaluators; // +0x27C
    static CurveManager* sInst;      // ?sInst@CurveManager@@2PAV1@A @ 0xF4F430
    void Initialize();       // ?Initialize@CurveManager@@UAEXXZ
    void CleanUp();          // ?CleanUp@CurveManager@@UAEXXZ
    void DetachCurve(Curve* curve);  // ?DetachCurve@CurveManager@@QAEXPAVCurve@@@Z
    void AttachCurve(Curve* curve);  // ?AttachCurve@CurveManager@@QAEXPAVCurve@@@Z (game.o 0x629780)
    void AddKeyFunc(unsigned int type,
                    float (__cdecl* func)(unsigned int, unsigned int,
                                          unsigned int, float, float,
                                          unsigned int));  // ?AddKeyFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z (game.o 0x6297C0)
    void AddConditionFunc(unsigned int type,
                          float (__cdecl* func)(unsigned int, unsigned int,
                                                unsigned int, float, float,
                                                unsigned int));  // ?AddConditionFunc@CurveManager@@QAEXIP6AMIIIMMI@Z@Z (game.o 0x629830)
    unsigned int UInt32Lookup(unsigned char* data, unsigned int key,
                              unsigned int _default);  // ?UInt32Lookup@CurveManager@@AAEIPAEII@Z
    float EvaluateKey(unsigned int frameId, unsigned int entityHandleVal,
                      unsigned int type, unsigned int trackId,
                      float _default);  // ?EvaluateKey@CurveManager@@IAEMIIIIM@Z (game.o 0x6385E0)
    float EvaluateCondition(unsigned int frameId, unsigned int entityHandleVal,
                            unsigned int type, float min, float max,
                            unsigned int trackId,
                            float _default);  // ?EvaluateCondition@CurveManager@@IAEMIIIMMIM@Z (game.o 0x638650)
    void PostEvent(unsigned int entityHandle, unsigned int hash,
                   float value);  // ?PostEvent@CurveManager@@QAEXIIM@Z (game.o 0x6386C0)
    void ClearEntities();  // ?ClearEntities@CurveManager@@QAEXXZ (game.o 0x6466A0)
};
CurveManager* CurveManager::sInst = nullptr;

// ea: 0x0060F1B0
void CurveManager::Initialize()
{
    RemainingTime* mRemainingTime = this->mRemainingTime;
    for (int i = 50; i != 0; --i)
    {
        mRemainingTime->mTrackId = 0;
        mRemainingTime->mEntityId = 0;
        mRemainingTime->mRemainingTime = 0.0f;
        ++mRemainingTime;
    }
}

// ea: 0x0060F1E0
void CurveManager::CleanUp()
{
}

// ea: 0x0060F1F0
void CurveManager::DetachCurve(Curve* curve)
{
}

// Curve - curve runtime state (verified against IDA Curve ctor disasm)
struct CurveNode {
    CurveNode* m_next;  // +0x00
    CurveNode* m_prev;  // +0x04
};
struct CurveEvalFunc {
    CurveEvalFunc* m_next;  // +0x00
    CurveEvalFunc* m_prev;  // +0x04
    unsigned int mType;     // +0x08
    float (__cdecl* mFunc)(unsigned int, unsigned int, unsigned int,
                           float, float, unsigned int);  // +0x0C
    static PoolAllocator* sAllocator;  // ?sAllocator@CurveEvalFunc@@2PAVPoolAllocator@@A @ 0xF4EC28
};
PoolAllocator* CurveEvalFunc::sAllocator = nullptr;

struct Curve {
    CurveNode m_dlist_node;             // +0x00
    float mCurveParams[19];             // +0x08..+0x54 (15 zeroed by ctor)
    unsigned int mEntityHandle;         // +0x54
    unsigned char* mCurveData;          // +0x58
    struct EffectList {
        int         m_size;  // +0x00
        void*       m_end;   // +0x04
        void*       m_head;  // +0x08
        void**      m_tail;  // +0x0C
    } mEffectList;                      // +0x5C
    static PoolAllocator* sAllocator;   // ?sAllocator@Curve@@2PAVPoolAllocator@@A @ 0xF4EC24

    Curve();   // ??0Curve@@QAE@XZ (game.o 0x6298A0)
    ~Curve();  // ??1Curve@@QAE@XZ (game.o 0x662A00)
};
static_assert(sizeof(Curve) == 0x6C, "Curve size mismatch");
PoolAllocator* Curve::sAllocator = nullptr;

struct CurveEffectListElem {
    CurveNode    m_dlist_node;   // +0x00
    int          mInUse;         // +0x08
    unsigned int mOwner;         // +0x0C
    Handle       mSound;         // +0x10
    float        mEffectParams[2];  // +0x14
    static PoolAllocator* sAllocator;  // ?sAllocator@CurveEffectListElem@@2PAVPoolAllocator@@A @ 0xF4EC2C
};
static_assert(sizeof(CurveEffectListElem) == 0x1C,
              "CurveEffectListElem size mismatch");
PoolAllocator* CurveEffectListElem::sAllocator = nullptr;

// ea: 0x00629780
void CurveManager::AttachCurve(Curve* curve)
{
    if (curve != nullptr)
    {
        curve->m_dlist_node.m_prev =
            ((CurveNode*)this->mCurveList.m_head)->m_prev;
        CurveNode* m_head = (CurveNode*)this->mCurveList.m_head;
        curve->m_dlist_node.m_next = m_head;
        m_head->m_prev = &curve->m_dlist_node;
        this->mCurveList.m_head = &curve->m_dlist_node;
        ++this->mCurveList.m_size;
    }
}

// ea: 0x006297C0
void CurveManager::AddKeyFunc(
    unsigned int type,
    float (__cdecl* func)(unsigned int, unsigned int, unsigned int, float,
                          float, unsigned int))
{
    if (func != nullptr && type != 0)
    {
        CurveEvalFunc* v4 = (CurveEvalFunc*)CurveEvalFunc::sAllocator->Allocate(0x10, false);
        if (v4 != nullptr)
        {
            v4->m_next = nullptr;
            v4->m_prev = nullptr;
        }
        v4->mType = type;
        v4->mFunc = func;
        v4->m_prev = ((CurveEvalFunc*)this->mKeyEvaluators.m_head)->m_prev;
        CurveEvalFunc* m_head = (CurveEvalFunc*)this->mKeyEvaluators.m_head;
        v4->m_next = m_head;
        m_head->m_prev = v4;
        this->mKeyEvaluators.m_head = v4;
        ++this->mKeyEvaluators.m_size;
    }
}

// ea: 0x00629830
void CurveManager::AddConditionFunc(
    unsigned int type,
    float (__cdecl* func)(unsigned int, unsigned int, unsigned int, float,
                          float, unsigned int))
{
    if (func != nullptr && type != 0)
    {
        CurveEvalFunc* v4 = (CurveEvalFunc*)CurveEvalFunc::sAllocator->Allocate(0x10, false);
        if (v4 != nullptr)
        {
            v4->m_next = nullptr;
            v4->m_prev = nullptr;
        }
        v4->mType = type;
        v4->mFunc = func;
        v4->m_prev = ((CurveEvalFunc*)this->mConditionEvaluators.m_head)->m_prev;
        CurveEvalFunc* m_head =
            (CurveEvalFunc*)this->mConditionEvaluators.m_head;
        v4->m_next = m_head;
        m_head->m_prev = v4;
        this->mConditionEvaluators.m_head = v4;
        ++this->mConditionEvaluators.m_size;
    }
}

// Curve evaluator statics (CurveManager.cpp; file-static Eval* helpers)
extern unsigned int AeHash(const char* str);  // ae_hash.cpp
typedef float (__cdecl* CurveEvalFn)(unsigned int, unsigned int,
                                     unsigned int, float, float,
                                     unsigned int);
extern float EvalVelocity(unsigned int, unsigned int, unsigned int, float,
                          float, unsigned int);         // @ 0x637CD0
extern float EvalRandom(unsigned int, unsigned int, unsigned int, float,
                        float, unsigned int);           // @ 0x60F180
extern float EvalSpringCompressionKey(unsigned int, unsigned int,
                                      unsigned int, float, float,
                                      unsigned int);    // @ 0x637AB0
extern float EvalTime(unsigned int, unsigned int, unsigned int, float,
                      float, unsigned int);             // @ 0x6379A0
extern float EvalImpact(unsigned int, unsigned int, unsigned int, float,
                        float, unsigned int);           // @ 0x637A30
extern float EvalRepeatInterval(unsigned int, unsigned int, unsigned int,
                                float, float, unsigned int);  // @ 0x60EEF0
extern float EvalSurface(unsigned int, unsigned int, unsigned int, float,
                         float, unsigned int);          // @ 0x637B90
extern float EvalSpringCompressionCond(unsigned int, unsigned int,
                                       unsigned int, float, float,
                                       unsigned int);   // @ 0x637B20
extern float EvalThrottle(unsigned int, unsigned int, unsigned int, float,
                          float, unsigned int);         // @ 0x637F60
extern float EvalThrottleChange(unsigned int, unsigned int, unsigned int,
                                float, float, unsigned int);  // @ 0x637E50
extern float EvalBrake(unsigned int, unsigned int, unsigned int, float,
                       float, unsigned int);            // @ 0x60F1A0
extern float EvalDriver(unsigned int, unsigned int, unsigned int, float,
                        float, unsigned int);           // @ 0x638030
extern float EvalPlayer(unsigned int, unsigned int, unsigned int, float,
                        float, unsigned int);           // @ 0x6380C0
extern float EvalHealth(unsigned int, unsigned int, unsigned int, float,
                        float, unsigned int);           // @ 0x637FD0
extern void DebugCurveRender();                         // game.o 0x60EEE0
extern void DebugRender_AddRenderer(void* self, void (*fp)());  // render.o
extern void* DebugRender_sInst;   // ?sInst@DebugRender@@2V1@A @ 0xF74D20
extern unsigned int s_ImpactMessage_0;  // @ 0xF50CC0
extern void reserved_dlist_CurveEffectListElem_delete_all(
    void* self);  // ?delete_all@?$reserved_dlist@VCurveEffectListElem@@@@QAEXXZ @ 0x4284BC

// ea: 0x00638160
CurveManager::CurveManager()
{
    this->mCurveList.m_size = 0;
    this->mCurveList.m_end = nullptr;
    this->mCurveList.m_head = &this->mCurveList.m_end;
    this->mCurveList.m_tail = &this->mCurveList.m_head;
    this->mKeyEvaluators.m_size = 0;
    this->mKeyEvaluators.m_end = nullptr;
    this->mKeyEvaluators.m_head = &this->mKeyEvaluators.m_end;
    this->mKeyEvaluators.m_tail = &this->mKeyEvaluators.m_head;
    this->mConditionEvaluators.m_size = 0;
    this->mConditionEvaluators.m_end = nullptr;
    this->mConditionEvaluators.m_head = &this->mConditionEvaluators.m_end;
    this->mConditionEvaluators.m_tail = &this->mConditionEvaluators.m_head;
    this->AddKeyFunc(AeHash("VELOCITY"), EvalVelocity);
    this->AddKeyFunc(AeHash("RANDOM"), EvalRandom);
    this->AddKeyFunc(AeHash("SPRING_COMPRESSION"),
                     EvalSpringCompressionKey);
    this->AddKeyFunc(AeHash("TIME"), EvalTime);
    this->AddKeyFunc(AeHash("IMPACT"), EvalImpact);
    this->AddConditionFunc(AeHash("REPEAT_INTERVAL"), EvalRepeatInterval);
    this->AddConditionFunc(AeHash("NONE"), EvalSurface);
    this->AddConditionFunc(AeHash("ASPHALT"), EvalSurface);
    this->AddConditionFunc(AeHash("BARK"), EvalSurface);
    this->AddConditionFunc(AeHash("BRICK"), EvalSurface);
    this->AddConditionFunc(AeHash("CARPET"), EvalSurface);
    this->AddConditionFunc(AeHash("CLOTH"), EvalSurface);
    this->AddConditionFunc(AeHash("CONCRETE"), EvalSurface);
    this->AddConditionFunc(AeHash("DIRT"), EvalSurface);
    this->AddConditionFunc(AeHash("FLESH"), EvalSurface);
    this->AddConditionFunc(AeHash("FOLIAGE"), EvalSurface);
    this->AddConditionFunc(AeHash("GLASS"), EvalSurface);
    this->AddConditionFunc(AeHash("GRASS"), EvalSurface);
    this->AddConditionFunc(AeHash("GRAVEL"), EvalSurface);
    this->AddConditionFunc(AeHash("ICE"), EvalSurface);
    this->AddConditionFunc(AeHash("METAL"), EvalSurface);
    this->AddConditionFunc(AeHash("MUD"), EvalSurface);
    this->AddConditionFunc(AeHash("PAPER"), EvalSurface);
    this->AddConditionFunc(AeHash("PLASTER"), EvalSurface);
    this->AddConditionFunc(AeHash("ROCK"), EvalSurface);
    this->AddConditionFunc(AeHash("SAND"), EvalSurface);
    this->AddConditionFunc(AeHash("SNOW"), EvalSurface);
    this->AddConditionFunc(AeHash("WATER"), EvalSurface);
    this->AddConditionFunc(AeHash("WOOD"), EvalSurface);
    this->AddConditionFunc(AeHash("SPRING_COMPRESSION"),
                           EvalSpringCompressionCond);
    this->AddConditionFunc(AeHash("VELOCITY"), EvalVelocity);
    this->AddConditionFunc(AeHash("THROTTLE"), EvalThrottle);
    this->AddConditionFunc(AeHash("THROTTLE_CHANGE"), EvalThrottleChange);
    this->AddConditionFunc(AeHash("IMPACT"), EvalImpact);
    this->AddConditionFunc(AeHash("BRAKE"), EvalBrake);
    this->AddConditionFunc(AeHash("DRIVER"), EvalDriver);
    this->AddConditionFunc(AeHash("PLAYER"), EvalPlayer);
    this->AddConditionFunc(AeHash("HEALTH"), EvalHealth);
    DebugRender_AddRenderer(&DebugRender_sInst, DebugCurveRender);
    RemainingTime* p = this->mRemainingTime;
    for (int i = 50; i != 0; --i)
    {
        p->mTrackId = 0;
        p->mEntityId = 0;
        p->mRemainingTime = 0.0f;
        ++p;
    }
}

// ea: 0x0061F780
CurveManager::~CurveManager()
{
}

// ea: 0x006385E0
float CurveManager::EvaluateKey(unsigned int frameId,
                                unsigned int entityHandleVal,
                                unsigned int type, unsigned int trackId,
                                float _default)
{
    CurveEvalFunc* m_head =
        (CurveEvalFunc*)this->mKeyEvaluators.m_head;
    CurveEvalFunc* m_next =
        m_head != nullptr ? (CurveEvalFunc*)m_head->m_next : nullptr;
    if (m_head
            == (CurveEvalFunc*)&this->mKeyEvaluators.m_end
        || m_next == nullptr)
        return _default;
    while (type != m_head->mType || m_head->mFunc == nullptr)
    {
        m_head = m_next;
        m_next = (CurveEvalFunc*)m_next->m_next;
        if (m_next == nullptr)
            return _default;
    }
    return m_head->mFunc(frameId, entityHandleVal, type, 0, 1.0f, trackId);
}

// ea: 0x00638650
float CurveManager::EvaluateCondition(unsigned int frameId,
                                      unsigned int entityHandleVal,
                                      unsigned int type, float min,
                                      float max, unsigned int trackId,
                                      float _default)
{
    CurveEvalFunc* m_head =
        (CurveEvalFunc*)this->mConditionEvaluators.m_head;
    CurveEvalFunc* m_next =
        m_head != nullptr ? (CurveEvalFunc*)m_head->m_next : nullptr;
    if (m_head
            == (CurveEvalFunc*)&this->mConditionEvaluators.m_end
        || m_next == nullptr)
        return _default;
    while (type != m_head->mType || m_head->mFunc == nullptr)
    {
        m_head = m_next;
        m_next = (CurveEvalFunc*)m_next->m_next;
        if (m_next == nullptr)
            return _default;
    }
    return m_head->mFunc(frameId, entityHandleVal, type, min, max, trackId);
}

// ea: 0x006386C0
void CurveManager::PostEvent(unsigned int entityHandle, unsigned int hash,
                             float value)
{
    CurveNode* m_head = (CurveNode*)this->mCurveList.m_head;
    CurveNode* m_next =
        m_head != nullptr ? (CurveNode*)m_head->m_next : nullptr;
    if (m_head != (CurveNode*)&this->mCurveList.m_end
        && m_next != nullptr)
    {
        unsigned int v6 = s_ImpactMessage_0;
        do
        {
            Curve* curve = (Curve*)m_head;
            if (m_head != nullptr
                && curve->mEntityHandle == entityHandle
                && hash == v6)
                curve->mCurveParams[17] = value;  // +0x4C
            m_head = m_next;
            m_next = m_next->m_next;
        } while (m_next != nullptr);
    }
}

// ea: 0x006466A0
void CurveManager::ClearEntities()
{
    CurveNode* node = (CurveNode*)this->mCurveList.m_head;
    CurveNode* m_end = (CurveNode*)&this->mCurveList.m_end;
    CurveNode* m_next =
        node != nullptr ? (CurveNode*)node->m_next : nullptr;
    if (node == m_end)
    {
        m_next = nullptr;
        node = nullptr;
    }
    while (m_next != nullptr)
    {
        Curve* curve = (Curve*)node;
        curve->~Curve();
        Curve::sAllocator->Release(curve);
        node = m_next;
        m_next = m_next->m_next;
    }
    this->mCurveList.m_head = m_end;
    this->mCurveList.m_tail = &this->mCurveList.m_head;
    this->mCurveList.m_size = 0;
}

// ea: 0x00662A00
Curve::~Curve()
{
    reserved_dlist_CurveEffectListElem_delete_all(&this->mEffectList);
    this->mEffectList.m_size = 0;
    this->mEffectList.m_head = &this->mEffectList.m_end;
    this->mEffectList.m_tail = &this->mEffectList.m_head;
}

// ea: 0x006298A0
Curve::Curve()
{
    this->m_dlist_node.m_next = nullptr;
    this->m_dlist_node.m_prev = nullptr;
    this->mEffectList.m_size = 0;
    this->mEffectList.m_end = nullptr;
    this->mEffectList.m_head = &this->mEffectList.m_end;
    this->mEffectList.m_tail = &this->mEffectList.m_head;
    for (int i = 0; i < 15; ++i)
        this->mCurveParams[i] = 0.0f;
}

// ============================================================================
// Curve attach/detach - ea: 0x642010..0x6465B0 (CurveManager.cpp)
// ============================================================================
extern unsigned char* BinFileManager_Find(void* self,
                                          const char* name);  // ?Find@BinFileManager@@QAEPAEPBD@Z
extern void* BinFileManager_sInst;  // ?sInst@BinFileManager@@2PAV1@A @ 0xF4EC18
extern void* EntityHandleDb_GetObject(unsigned int val);  // game.o

// ea: 0x00642010
bool AttachCurveVehicle(unsigned int entityHandleVal, char* filename,
                        float topSpeed, float topSpeedReverse)
{
    unsigned char* v2 =
        BinFileManager_Find(BinFileManager_sInst, filename);
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(entityHandleVal);
    if (v2 == nullptr)
        return false;
    if (mObject == nullptr)
        return false;
    if (mObject->curve != nullptr)
        return false;
    Curve* v5 =
        (Curve*)Curve::sAllocator->Allocate(0x6C, false);
    if (v5 == nullptr)
        return false;
    Curve* v6 = new (v5) Curve();
    Curve* v7 = v6;
    if (v6 == nullptr)
        return false;
    mObject->curve = v6;
    v6->mEntityHandle = entityHandleVal;
    v6->mCurveData = v2;
    reserved_dlist_CurveEffectListElem_delete_all(&v6->mEffectList);
    CurveManager::sInst->AttachCurve(v7);
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle != nullptr)
    {
        vehicle_info_t* v9 = s_vehicleInfos[scr_vehicle->infoIdx];
        if (v9 != nullptr)
        {
            if (*(float*)((char*)v9 + 0x1F4) <= 0.0f)
                *(float*)((char*)v9 + 0x1F4) = 400.0f;
            if (*(float*)((char*)v9 + 0x1F8) >= 0.0f)
                *(float*)((char*)v9 + 0x1F4) = -400.0f;
        }
    }
    return true;
}

// ea: 0x00642120
bool AttachCurveEntity(unsigned int entityHandleVal, char* filename)
{
    unsigned char* v2 =
        BinFileManager_Find(BinFileManager_sInst, filename);
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(entityHandleVal);
    if (v2 == nullptr)
        return false;
    if (mObject == nullptr)
        return false;
    if (mObject->curve != nullptr)
        return false;
    Curve* v5 =
        (Curve*)Curve::sAllocator->Allocate(0x6C, false);
    if (v5 == nullptr)
        return false;
    Curve* v6 = new (v5) Curve();
    Curve* v7 = v6;
    if (v6 == nullptr)
        return false;
    mObject->curve = v6;
    v6->mEntityHandle = entityHandleVal;
    v6->mCurveData = v2;
    reserved_dlist_CurveEffectListElem_delete_all(&v6->mEffectList);
    CurveManager::sInst->AttachCurve(v7);
    return true;
}

// ea: 0x006465B0
bool DetachCurveEntity(unsigned int entityHandleVal)
{
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(entityHandleVal);
    if (mObject == nullptr)
        return false;
    Curve* curve = mObject->curve;
    if (curve == nullptr)
        return false;
    curve->~Curve();
    Curve::sAllocator->Release(curve);
    return true;
}

// ea: 0x0060F200
unsigned int CurveManager::UInt32Lookup(unsigned char* data, unsigned int key,
                                        unsigned int _default)
{
    int v4 = 0;
    if (*data == 0)
        return _default;
    unsigned char* i = data + 4;
    unsigned int result;
    while (1)
    {
        result = *(unsigned int*)(i + 4);
        if (key == *(unsigned int*)i)
            break;
        if (++v4 >= *data)
            return _default;
        i += 8;
    }
    return result;
}
static BaseCmdFuncInfo* cmd_functions;      // ?cmd_functions (game.o)
static BaseCmdFuncInfo* sv_cmd_functions;   // ?sv_cmd_functions (game.o)

extern void _Z_FreeInternal(void* ptr);             // ?_Z_FreeInternal (hunk_mem)
extern int Com_Filter(char* filter, char* name, int casesensitive);  // core.o
extern void ButtonMgr_ClearBinding(const BaseCmdFuncInfo* boundCmd,
                                   int clnt);  // ?ClearBinding@ButtonMgr (game2.o)
extern void Com_Printf(const char* fmt, ...);
extern void Com_DefaultExtension(char* path, int maxSize,
                                 const char* extension);  // core.o
extern int  FS_ReadFile(const char* qpath, void** buffer);  // core.o
extern void FS_FreeFile(void* buffer);                      // core.o

// ============================================================================
// Cbuf_AddText - ea: 0x60E490
// ============================================================================
// ea: 0x0060E490
void Cbuf_AddText(const char* text)
{
    unsigned int v1 = (unsigned int)strlen(text);
    if ((cmd_text.cmdsize + (int)v1) < cmd_text.maxsize)
    {
        memcpy(&cmd_text.data[cmd_text.cmdsize], text, v1);
        cmd_text.cmdsize += v1;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 153;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Cbuf_AddText: overflow"))
            __debugbreak();
    }
}

// ============================================================================
// Cbuf_InsertText - ea: 0x60E530
// ============================================================================
// ea: 0x0060E530
void Cbuf_InsertText(const char* text)
{
    unsigned int v1 = (unsigned int)strlen(text) + 1;
    if ((cmd_text.cmdsize + (int)v1) <= cmd_text.maxsize)
    {
        for (int i = cmd_text.cmdsize - 1; i >= 0; --i)
            cmd_text.data[i + v1] = cmd_text.data[i];
        memcpy(cmd_text.data, text, v1 - 1);
        cmd_text.data[v1 - 1] = 10;
        cmd_text.cmdsize += v1;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 177;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Cbuf_Insert overflow"))
            __debugbreak();
    }
}

// ============================================================================
// Cbuf_Execute / Cmd_Exec_f / Cmd_Init - ea: 0x629560..0x629730
// ============================================================================

// ea: 0x00629560
void Cbuf_Execute()
{
    char quote = 0;
    if (cmd_text.cmdsize != 0)
    {
        while (cmd_wait == 0)
        {
            char* data = cmd_text.data;
            int v2 = 0;
            if (cmd_text.cmdsize > 0)
            {
                do
                {
                    char v3 = cmd_text.data[v2];
                    if (v3 == '"')
                        ++quote;
                    if ((quote & 1) == 0 && v3 == ';')
                        break;
                    if (v3 == 10)
                        break;
                    if (v3 == 13)
                        break;
                    ++v2;
                } while (v2 < cmd_text.cmdsize);
                if (v2 >= 4095)
                    v2 = 4095;
            }
            char line[4096];
            memcpy(line, cmd_text.data, v2);
            int cmdsize = cmd_text.cmdsize;
            quote = 0;
            bool wholeLine = v2 == cmd_text.cmdsize;
            line[v2] = 0;
            if (wholeLine)
            {
                cmd_text.cmdsize = 0;
            }
            else
            {
                int v6 = v2 + 1;
                cmd_text.cmdsize = cmdsize - v6;
                memmove(data, &data[v6], cmdsize - v6);
            }
            Cmd_ExecuteString(line);
            if (cmd_text.cmdsize == 0)
                goto LABEL_19;
        }
        --cmd_wait;
    }
LABEL_19:
    Cbuf_SV_Execute();
}

// ea: 0x00629660
void Cmd_Exec_f()
{
    if (cmd_argc == 2)
    {
        char filename[128];
        Q_strncpyz(filename, cmd_argv[1], 128);
        Com_DefaultExtension(filename, 128, ".cfg");
        char* f = nullptr;
        FS_ReadFile(filename, (void**)&f);
        if (f != nullptr)
        {
            const char* v1 = cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
            Com_Printf("execing %s\n", v1);
            Cbuf_InsertText(f);
            FS_FreeFile(f);
        }
        else
        {
            const char* v0 = cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
            Com_Printf("couldn't exec %s\n", v0);
        }
    }
    else
    {
        Com_Printf("exec <filename> : execute a script file\n");
    }
}

// ea: 0x00629730
void Cmd_Init()
{
    Cmd_AddCommand("cmdlist", Cmd_List_f);
    Cmd_AddCommand("exec", Cmd_Exec_f);
    Cmd_AddCommand("vstr", Cmd_Vstr_f);
    Cmd_AddCommand("echo", Cmd_Echo_f);
    Cmd_AddCommand("wait", Cmd_Wait_f);
}

// ============================================================================
// PadAliasMgr - vehicle/context button & stick alias tables (PadAliasMgr.cpp)
// ============================================================================
enum EPadAliasButton {
    kPadAliasButtonInvalid = -1,
    kPadAliasButtonGas = 0,
    kPadAliasButtonReverse = 1,
    kPadAliasButtonHandBrake = 2,
    kPadAliasButtonAlignTurret = 3,
    kPadAliasButtonFireCoax = 4,
    kPadAliasButtonSwitchSeats = 5,
};
enum EPadAliasStick {
    kPadAliasStickInvalid = -1,
    kPadAliasStickVehicleSteering = 0,
    kPadAliasStickTankSteering = 1,
};

// Minimal controller view (controller_xboxr). Values match the binary:
// kPadAliasButtonIndexDesc order == controller::ButtonIndex (LEFTBUTTON=0..),
// verified against the controller::button_value switch at 0x7E2050.
class controller {
public:
    enum ButtonIndex {
        LEFTBUTTON = 0,
        DOWNBUTTON = 1,
        RIGHTBUTTON = 2,
        UPBUTTON = 3,
        SQUARE = 4,
        X = 5,
        CIRCLE = 6,
        TRIANGLE = 7,
        R1 = 8,
        L1 = 9,
        R2 = 10,
        L2 = 11,
        R3 = 12,
        L3 = 13,
        START = 14,
        SELECT = 15,
    };
    enum StickIndex {
        LEFTSTICK = 0,
        RIGHTSTICK = 1,
    };
    static controller* inst();                              // controller_xbox.o 0x7E1D90
    int  button_value(int i_controller_num, ButtonIndex i_button);          // 0x7E2050
    bool button_released(int i_controller_num, ButtonIndex i_button);       // 0x7E2220
    bool button_released_clear(int i_controller_num, ButtonIndex i_button); // 0x7E23D0
    bool button_pressed(int i_controller_num, ButtonIndex i_button);        // 0x7E2670
    bool button_pressed_clear(int i_controller_num, ButtonIndex i_button);  // 0x7E2810
    void stick_value(int i_controller_num, StickIndex i_Stick,
                     int& o_x, int& o_y);                                   // 0x7E2AB0
};

static const char* kPadAliasCtxDesc[3] = {
    "Vehicle", "VehicleTank", "OnFoot",
};  // ?kPadAliasCtxDesc@@3PAPBDA (game.o @ 0xDF8230)
static const char* kPadAliasButtonIndexDesc[16] = {
    "LEFTBUTTON", "DOWNBUTTON", "RIGHTBUTTON", "UPBUTTON",
    "SQUARE", "X", "CIRCLE", "TRIANGLE",
    "R1", "L1", "R2", "L2",
    "R3", "L3", "START", "SELECT",
};  // game.o @ 0xDF81E8
static const char* kPadAliasStickIndexDesc[2] = {
    "LEFTSTICK", "RIGHTSTICK",
};  // game.o @ 0xDF8228
static const char* kPadAliasButtonAliasDesc[6] = {
    "Gas", "Reverse", "HandBrake", "AlignTurret", "FireCoax", "SwitchSeats",
};  // game.o @ 0xDF823C
static const char* kPadAliasStickAliasDesc[2] = {
    "VehicleSteering", "TankSteering",
};  // game.o @ 0xDF8254

static int GetButtonIndexFromDesc(const char* desc);    // game.o 0x6126A0
static int GetStickIndexFromDesc(const char* desc);     // game.o 0x6126D0
static EPadAliasButton GetButtonAliasFromDesc(const char* desc);  // game.o 0x612730
static EPadAliasStick GetStickAliasFromDesc(const char* desc);    // game.o 0x612760

struct PadAliasCtx {
    ae_sized_array<ae_sized_array<EPadAliasButton, 16>, 4> mButtonAlias;  // +0x00 (0x114)
    ae_sized_array<ae_sized_array<EPadAliasStick, 2>, 4> mStickAlias;     // +0x114 (0x34)
    void Clear();  // ?Clear@Context@PadAliasMgr@@QAEXXZ (game.o 0x620DD0)
    void Clear(int ctrlr);  // ?Clear@Context@PadAliasMgr@@QAEXH@Z (game.o 0x620EA0)
    void BindButton(int ctrlNum, int buttonIndex, EPadAliasButton buttonAlias);  // game.o 0x620F70
    void BindStick(int ctrlNum, int stickIndex, EPadAliasStick stickAlias);      // game.o 0x621050
    EPadAliasButton GetButtonAlias(int ctrlNum, controller::ButtonIndex buttonIndex);  // game.o 0x621160
    EPadAliasStick GetStickAlias(int ctrlNum, controller::StickIndex stickIndex);      // game.o 0x6211E0
    int GetButtonValue(int ctrlNum, EPadAliasButton buttonAlias);  // game.o 0x621260
    bool IsButtonReleased(int ctrlNum, EPadAliasButton buttonAlias);  // game.o 0x6212A0
    bool IsButtonReleasedClear(int ctrlNum, EPadAliasButton buttonAlias);  // game.o 0x6212F0
    bool IsButtonPressed(int ctrlNum, EPadAliasButton buttonAlias);  // game.o 0x621340
    bool IsButtonPressedClear(int ctrlNum, EPadAliasButton buttonAlias);  // game.o 0x621390
    void GetStickValue(int ctrlNum, EPadAliasStick stickAlias,
                       int& stickX, int& stickY);  // game.o 0x6213E0
};
static_assert(sizeof(PadAliasCtx) == 0x148, "PadAliasCtx size mismatch");
struct PadAliasMgr {
    PadAliasCtx mCtx[3];      // +0x00 (3 contexts, 0x148 stride; GetCtx returns this + idx*0x148)
    static PadAliasMgr* sInst;  // ?sInst@PadAliasMgr@@2PAV1@A @ 0xF4F458
    PadAliasMgr();            // ??0PadAliasMgr@@QAE@XZ (game.o 0x6431F0)
    void WriteBindings(int f);  // ?WriteBindings@PadAliasMgr@@QAEXH@Z (game.o 0x62B520)
};
static_assert(sizeof(PadAliasMgr) == 0x3D8, "PadAliasMgr size mismatch");
PadAliasMgr* PadAliasMgr::sInst = nullptr;

extern int LocalClient_ClientToPort(int client);  // cl.o
extern int cvar_modifiedFlags;  // ?cvar_modifiedFlags@@3HA (core.o @ 0xEF8194)
extern void FS_Printf(int h, const char* fmt, ...);  // filesystem

// ea: 0x006126A0
static int GetButtonIndexFromDesc(const char* desc)
{
    int v1 = 0;
    while (stricmp(desc, kPadAliasButtonIndexDesc[v1]) != 0)
    {
        if (++v1 >= 16)
            return -1;
    }
    return v1;
}

// ea: 0x006126D0
static int GetStickIndexFromDesc(const char* desc)
{
    int v1 = 0;
    while (stricmp(desc, kPadAliasStickIndexDesc[v1]) != 0)
    {
        if (++v1 >= 2)
            return -1;
    }
    return v1;
}

// ea: 0x00612730
static EPadAliasButton GetButtonAliasFromDesc(const char* desc)
{
    int v1 = 0;
    while (stricmp(desc, kPadAliasButtonAliasDesc[v1]) != 0)
    {
        if (++v1 >= 6)
            return kPadAliasButtonInvalid;
    }
    return (EPadAliasButton)v1;
}

// ea: 0x00612760
static EPadAliasStick GetStickAliasFromDesc(const char* desc)
{
    int v1 = 0;
    while (stricmp(desc, kPadAliasStickAliasDesc[v1]) != 0)
    {
        if (++v1 >= 2)
            return kPadAliasStickInvalid;
    }
    return (EPadAliasStick)v1;
}

// ea: 0x00620DD0
void PadAliasCtx::Clear()
{
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 16; ++j)
            mButtonAlias[i][j] = kPadAliasButtonInvalid;
}

// ea: 0x00620EA0
void PadAliasCtx::Clear(int ctrlr)
{
    for (int v4 = 0; v4 < 16; ++v4)
        mButtonAlias[ctrlr][v4] = kPadAliasButtonInvalid;
}

// ea: 0x00620F70
void PadAliasCtx::BindButton(int ctrlNum, int buttonIndex,
                             EPadAliasButton buttonAlias)
{
    if (ctrlNum < 0 || ctrlNum >= 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 344;
        AeAssert::gCurrentExpr = "ctrlNum >= 0 && ctrlNum < MAX_CONTROLLERS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad ctrlNum"))
            __debugbreak();
    }
    if (buttonIndex < 0 || buttonIndex >= 16)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 345;
        AeAssert::gCurrentExpr =
            "buttonIndex >= 0 && buttonIndex < kPadCtrlButtonIndexCount";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad buttonIndex"))
            __debugbreak();
    }
    if (buttonIndex >= 0 && buttonIndex < 16)
    {
        mButtonAlias[ctrlNum][buttonIndex] = buttonAlias;
        cvar_modifiedFlags |= 1;
    }
}

// ea: 0x00621050
void PadAliasCtx::BindStick(int ctrlNum, int stickIndex,
                            EPadAliasStick stickAlias)
{
    if (ctrlNum < 0 || ctrlNum >= 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 361;
        AeAssert::gCurrentExpr = "ctrlNum >= 0 && ctrlNum < MAX_CONTROLLERS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad ctrlNum"))
            __debugbreak();
    }
    if (stickIndex < 0 || stickIndex >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 362;
        AeAssert::gCurrentExpr =
            "stickIndex >= 0 && stickIndex < kPadCtrlStickIndexCount";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad stickIndex"))
            __debugbreak();
    }
    if (stickIndex >= 0 && stickIndex <= 1)
    {
        mStickAlias[ctrlNum][stickIndex] = stickAlias;
        int other = stickIndex != 1;
        if (mStickAlias[ctrlNum][other] == stickAlias)
            mStickAlias[ctrlNum][other] = kPadAliasStickInvalid;
        cvar_modifiedFlags |= 1;
    }
}

// ea: 0x00621160
EPadAliasButton PadAliasCtx::GetButtonAlias(
    int ctrlNum, controller::ButtonIndex buttonIndex)
{
    if (ctrlNum < 0 || ctrlNum >= 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 384;
        AeAssert::gCurrentExpr = "ctrlNum >= 0 && ctrlNum < MAX_CONTROLLERS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad ctrlNum"))
            __debugbreak();
    }
    return mButtonAlias[ctrlNum][buttonIndex];
}

// ea: 0x006211E0
EPadAliasStick PadAliasCtx::GetStickAlias(
    int ctrlNum, controller::StickIndex stickIndex)
{
    if (ctrlNum < 0 || ctrlNum >= 4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\PadAliasMgr.cpp";
        AeAssert::gCurrentLine = 394;
        AeAssert::gCurrentExpr = "ctrlNum >= 0 && ctrlNum < MAX_CONTROLLERS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad ctrlNum"))
            __debugbreak();
    }
    return mStickAlias[ctrlNum][stickIndex];
}

// ea: 0x00621260
int PadAliasCtx::GetButtonValue(int ctrlNum, EPadAliasButton buttonAlias)
{
    int v4 = controller::LEFTBUTTON;
    while (1)
    {
        if (GetButtonAlias(ctrlNum, (controller::ButtonIndex)v4)
            == buttonAlias)
        {
            int result = controller::inst()->button_value(
                ctrlNum, (controller::ButtonIndex)v4);
            if (result > 0)
                return result;
        }
        if (++v4 >= 16)
            return 0;
    }
}

// ea: 0x006212A0
bool PadAliasCtx::IsButtonReleased(int ctrlNum, EPadAliasButton buttonAlias)
{
    int v4 = controller::LEFTBUTTON;
    while (1)
    {
        if (GetButtonAlias(ctrlNum, (controller::ButtonIndex)v4)
            == buttonAlias)
        {
            if (controller::inst()->button_released(
                    ctrlNum, (controller::ButtonIndex)v4))
                return true;
        }
        if (++v4 >= 16)
            return false;
    }
}

// ea: 0x006212F0
bool PadAliasCtx::IsButtonReleasedClear(int ctrlNum,
                                        EPadAliasButton buttonAlias)
{
    int v4 = controller::LEFTBUTTON;
    while (1)
    {
        if (GetButtonAlias(ctrlNum, (controller::ButtonIndex)v4)
            == buttonAlias)
        {
            if (controller::inst()->button_released_clear(
                    ctrlNum, (controller::ButtonIndex)v4))
                return true;
        }
        if (++v4 >= 16)
            return false;
    }
}

// ea: 0x00621340
bool PadAliasCtx::IsButtonPressed(int ctrlNum, EPadAliasButton buttonAlias)
{
    int v4 = controller::LEFTBUTTON;
    while (1)
    {
        if (GetButtonAlias(ctrlNum, (controller::ButtonIndex)v4)
            == buttonAlias)
        {
            if (controller::inst()->button_pressed(
                    ctrlNum, (controller::ButtonIndex)v4))
                return true;
        }
        if (++v4 >= 16)
            return false;
    }
}

// ea: 0x00621390
bool PadAliasCtx::IsButtonPressedClear(int ctrlNum,
                                       EPadAliasButton buttonAlias)
{
    int v4 = controller::LEFTBUTTON;
    while (1)
    {
        if (GetButtonAlias(ctrlNum, (controller::ButtonIndex)v4)
            == buttonAlias)
        {
            if (controller::inst()->button_pressed_clear(
                    ctrlNum, (controller::ButtonIndex)v4))
                return true;
        }
        if (++v4 >= 16)
            return false;
    }
}

// ea: 0x006213E0
void PadAliasCtx::GetStickValue(int ctrlNum, EPadAliasStick stickAlias,
                                int& stickX, int& stickY)
{
    stickX = 0;
    stickY = 0;
    for (int i = controller::LEFTSTICK; i < 2; ++i)
    {
        if (GetStickAlias(ctrlNum, (controller::StickIndex)i) == stickAlias)
        {
            controller::inst()->stick_value(ctrlNum, (controller::StickIndex)i,
                                            stickX, stickY);
            if (stickX != 0 || stickY != 0)
                break;
        }
    }
}

// ea: 0x0062B1A0
void SetButtonAlias()
{
    int ctrlNum = LocalClient_ClientToPort(currCl);
    int count = cmd_argc;
    if (cmd_argc >= 3)
    {
        const char* v0 = cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
        int v1 = 0;
        while (stricmp(v0, kPadAliasCtxDesc[v1]) != 0)
        {
            if (++v1 >= 3)
            {
                const char* v2 =
                    cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
                Com_Printf("\"%s\" isn't a valid alias context\n", v2);
                return;
            }
        }
        const char* v3 = cmd_argc > 2 ? cmd_argv[2] : defaultFileName;
        int v5 = GetButtonIndexFromDesc(v3);
        if (v5 == -1)
        {
            const char* v6 =
                cmd_argc > 2 ? cmd_argv[2] : defaultFileName;
            Com_Printf("\"%s\" isn't a valid button name\n", v6);
        }
        else if (count == 3)
        {
            EPadAliasButton ButtonAlias = PadAliasMgr::sInst->mCtx[v1]
                                              .GetButtonAlias(
                                                  ctrlNum,
                                                  (controller::ButtonIndex)v5);
            if (ButtonAlias != kPadAliasButtonInvalid)
            {
                const char* v12 = kPadAliasButtonAliasDesc[ButtonAlias];
                const char* v8 = Cmd_Argv(2);
                Com_Printf("\"%s\" = \"%s\"\n", v8, v12);
            }
        }
        else
        {
            const char* v9 = Cmd_Argv(3);
            EPadAliasButton ButtonAliasFromDesc = GetButtonAliasFromDesc(v9);
            if (ButtonAliasFromDesc == kPadAliasButtonInvalid)
            {
                const char* v11 = Cmd_Argv(3);
                Com_Printf("\"%s\" isn't a valid button alias name\n", v11);
            }
            else
            {
                PadAliasMgr::sInst->mCtx[v1].BindButton(ctrlNum, v5,
                                                        ButtonAliasFromDesc);
            }
        }
    }
    else
    {
        Com_Printf("buttonalias <context> <button> [alias] : attach an alias "
                   "to a button\n");
    }
}

// ea: 0x0062B320
void SetStickAlias()
{
    int ctrlNum = LocalClient_ClientToPort(currCl);
    int count = cmd_argc;
    if (cmd_argc >= 3)
    {
        const char* v0 = cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
        int v1 = 0;
        while (stricmp(v0, kPadAliasCtxDesc[v1]) != 0)
        {
            if (++v1 >= 3)
            {
                const char* v2 =
                    cmd_argc > 1 ? cmd_argv[1] : defaultFileName;
                Com_Printf("\"%s\" isn't a valid alias context\n", v2);
                return;
            }
        }
        const char* v3 = cmd_argc > 2 ? cmd_argv[2] : defaultFileName;
        int v5 = GetStickIndexFromDesc(v3);
        if (v5 == -1)
        {
            const char* v6 =
                cmd_argc > 2 ? cmd_argv[2] : defaultFileName;
            Com_Printf("\"%s\" isn't a valid stick name\n", v6);
        }
        else if (count == 3)
        {
            EPadAliasStick StickAlias = PadAliasMgr::sInst->mCtx[v1]
                                            .GetStickAlias(
                                                ctrlNum,
                                                (controller::StickIndex)v5);
            if (StickAlias != kPadAliasStickInvalid)
            {
                const char* v12 = kPadAliasStickAliasDesc[StickAlias];
                const char* v8 = Cmd_Argv(2);
                Com_Printf("\"%s\" = \"%s\"\n", v8, v12);
            }
        }
        else
        {
            const char* v9 = Cmd_Argv(3);
            EPadAliasStick StickAliasFromDesc = GetStickAliasFromDesc(v9);
            if (StickAliasFromDesc == kPadAliasStickInvalid)
            {
                const char* v11 = Cmd_Argv(3);
                Com_Printf("\"%s\" isn't a valid stick alias name\n", v11);
            }
            else
            {
                PadAliasMgr::sInst->mCtx[v1].BindStick(ctrlNum, v5,
                                                       StickAliasFromDesc);
            }
        }
    }
    else
    {
        Com_Printf("stickalias <context> <stick> [alias] : attach an alias to "
                   "a stick\n");
    }
}

// ea: 0x0062B4A0
void ClearAllPadAliases()
{
    PadAliasMgr* v0 = PadAliasMgr::sInst;
    int v1 = LocalClient_ClientToPort(currCl);
    for (int i = 3; i != 0; --i)
    {
        v0->mCtx[0].Clear(v1);
        v0 = (PadAliasMgr*)((char*)v0 + 328);
    }
}

// ea: 0x0062B4E0
void InitPadAliasCommands()
{
    Cmd_AddCommand("buttonalias", SetButtonAlias);
    Cmd_AddCommand("stickalias", SetStickAlias);
    Cmd_AddCommand("clearallaliases", ClearAllPadAliases);
}

// ea: 0x0062B520
void PadAliasMgr::WriteBindings(int f)
{
    FS_Printf(f, "clearallaliases\n");
    const char** v3 = kPadAliasCtxDesc;
    PadAliasCtx* v4 = &this->mCtx[0];
    do
    {
        for (int i = 0; i < 16; ++i)
        {
            EPadAliasButton v6 = v4->mButtonAlias[0][i];
            if (v6 != kPadAliasButtonInvalid)
            {
                FS_Printf(f, "buttonalias %s %s \"%s\"\n", *v3,
                          kPadAliasButtonIndexDesc[i],
                          kPadAliasButtonAliasDesc[v6]);
            }
        }
        for (int j = 0; j < 2; ++j)
        {
            EPadAliasStick v8 = v4->mStickAlias[0][j];
            if (v8 != kPadAliasStickInvalid)
            {
                FS_Printf(f, "stickalias %s %s \"%s\"\n", *v3,
                          kPadAliasStickIndexDesc[j],
                          kPadAliasStickAliasDesc[v8]);
            }
        }
        ++v3;
        v4 = (PadAliasCtx*)((char*)v4 + 328);
    } while (v3 < kPadAliasCtxDesc + 3);
}

// C-style bridge used by core.o Com_WriteConfigToFile (common.cpp);
// the binary calls PadAliasMgr::WriteBindings(PadAliasMgr::sInst, f) there.
void PadAliasMgr_WriteBindings(int f)
{
    PadAliasMgr::sInst->WriteBindings(f);
}

// ea: 0x006431F0
PadAliasMgr::PadAliasMgr()
{
    for (int j = 0; j < 3; ++j)
        mCtx[j].Clear();
    for (int k = 0; k < 4; ++k)
    {
        mCtx[0].mButtonAlias[k][8] = kPadAliasButtonGas;
        cvar_modifiedFlags |= 1;
        mCtx[0].mButtonAlias[k][6] = kPadAliasButtonReverse;
        cvar_modifiedFlags |= 1;
        mCtx[0].mButtonAlias[k][4] = kPadAliasButtonHandBrake;
        cvar_modifiedFlags |= 1;
        mCtx[0].mButtonAlias[k][9] = kPadAliasButtonHandBrake;
        cvar_modifiedFlags |= 1;
        mCtx[0].mStickAlias[k][0] = kPadAliasStickVehicleSteering;
        if (mCtx[0].mStickAlias[k][1] == kPadAliasStickVehicleSteering)
            mCtx[0].mStickAlias[k][1] = kPadAliasStickInvalid;
        cvar_modifiedFlags |= 1;
        mCtx[1].mButtonAlias[k][9] = kPadAliasButtonAlignTurret;
        cvar_modifiedFlags |= 1;
        mCtx[1].mStickAlias[k][0] = kPadAliasStickVehicleSteering;
        if (mCtx[1].mStickAlias[k][1] == kPadAliasStickVehicleSteering)
            mCtx[1].mStickAlias[k][1] = kPadAliasStickInvalid;
        cvar_modifiedFlags |= 1;
    }
}

// ============================================================================
// Cmd_CallCmdFunction - ea: 0x60E6A0
// ============================================================================
// ea: 0x0060E6A0
void Cmd_CallCmdFunction(const BaseCmdFuncInfo* cmd, int key, int time)
{
    if (cmd == nullptr)
        return;
    int mFuncType = cmd->mFuncType;
    if (mFuncType == INPUT_CMD && cmd[1].mNext != nullptr)
    {
        (*(void (**)(int, int))&cmd[1].mNext)(key, time);
    }
    else if (mFuncType == CMD)
    {
        void (*mNext)() = (void (*)())cmd[1].mNext;
        if (mNext != nullptr)
            mNext();
    }
}

// ============================================================================
// Cmd_Argc / Cmd_Argv / Cmd_Args - ea: 0x60E6E0..0x60E7D0
// ============================================================================
// ea: 0x0060E6E0
int Cmd_Argc()
{
    return cmd_argc;
}

// ea: 0x0060E6F0
char* Cmd_Argv(int arg)
{
    if (arg < (unsigned int)cmd_argc)
        return cmd_argv[arg];
    return (char*)"";
}

// ea: 0x0061F5E0
void Cmd_ArgvBuffer(int arg, char* buffer, int bufferLength)
{
    if (arg < (unsigned int)cmd_argc)
        Q_strncpyz(buffer, cmd_argv[arg], bufferLength);
    else
        Q_strncpyz(buffer, "", bufferLength);
}

static char cmd_args1[1024];

// ea: 0x0060E710
char* Cmd_Args(int start)
{
    if (start < 1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 612;
        AeAssert::gCurrentExpr = "start >= 1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    cmd_args1[0] = 0;
    for (int i = start; i < cmd_argc; ++i)
    {
        strcat(cmd_args1, cmd_argv[i]);
        if (i != cmd_argc - 1)
            strcat(cmd_args1, " ");
    }
    return cmd_args1;
}

// ea: 0x0061F620
void Cmd_ArgsBuffer(char* buffer, int bufferLength)
{
    Q_strncpyz(buffer, Cmd_Args(1), bufferLength);
}

// ============================================================================
// Cmd_TokenizeString2 - ea: 0x60E7E0
// ============================================================================
// ea: 0x0060E7E0
void Cmd_TokenizeString2(const char* text_in, int max_tokens)
{
    cmd_argc = 0;
    if (text_in == nullptr)
        return;
    char* v4 = cmd_tokenized;
    const char* v2 = text_in;
    int v3 = 0;
    while (--max_tokens != 0)
    {
        // skip whitespace
        char v5 = *v2;
        if (v5 == 0)
            break;
        while (v5 <= 32)
        {
            v5 = *++v2;
            if (v5 == 0)
                return;
        }
        if (*v2 == 0)
            break;
        char v6 = *v2;
        if (v6 == 47)
        {
            char v7 = v2[1];
            if (v7 == 47)
                return;  // // comment to EOL
            if (v7 == 42)
            {
                // /* */ block comment
                char c = *v2;
                if (c != 0)
                {
                    while (c != 42 || v2[1] != 47)
                    {
                        c = *++v2;
                        if (c == 0)
                            return;
                    }
                    if (*v2 != 0)
                        v2 += 2;
                }
                continue;
            }
        }
        // token
        v8:
        cmd_argv[v3++] = v4;
        cmd_argc = v3;
        char v8 = *v2;
        if (v8 == 34)
        {
            // quoted string
            char v9 = v2[1];
            const char* i = v2 + 1;
            for (; v9 != 0; ++i)
            {
                if (v9 == 34)
                    break;
                *v4 = v9;
                v9 = i[1];
                ++v4;
            }
            *v4++ = 0;
            if (*i == 0)
                return;
            v2 = i + 1;
        }
        else
        {
            for (char j = *v2; j > 32; ++v2)
            {
                if (j == 34)
                    break;
                if (j == 47)
                {
                    char v12 = v2[1];
                    if (v12 == 47 || v12 == 42)
                        break;
                }
                *v4 = j;
                j = v2[1];
                ++v4;
            }
            *v4++ = 0;
        }
        if (*v2 != 0)
        {
            if (*v2 <= 32)
                ++v2;
            if (v3 != 512)
                continue;
        }
        return;
    }
    if (*v2 != 0)
    {
        cmd_argv[v3] = v4;
        char v13 = *v2;
        bool v14 = *v2 == 0;
        cmd_argc = v3 + 1;
        if (!v14)
        {
            do
            {
                *v4 = v13;
                ++v4;
                v13 = *++v2;
            } while (v13 != 0);
            *v4 = 0;
        }
    }
}

// ea: 0x0060E930
void Cmd_TokenizeString(const char* text_in)
{
    Cmd_TokenizeString2(text_in, 0);
}

extern char* CopyStringInternal(const char* in);  // ?CopyStringInternal (hunk_mem)

// ============================================================================
// Cmd_AddCommand - ea: 0x60E950
// ============================================================================
void Cmd_AddCommand(const char* cmd_name, void (*function)())
{
    if (cmd_name == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 793;
        AeAssert::gCurrentExpr = "cmd_name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BaseCmdFuncInfo* v2 = cmd_functions;
    if (cmd_functions != nullptr)
    {
        while (strcmp(cmd_name, v2->mName) != 0)
        {
            v2 = v2->mNext;
            if (v2 == nullptr)
                goto add_entry;
        }
        int mFuncType = v2->mFuncType;
        if (mFuncType == CMD && v2[1].mNext != nullptr
            || mFuncType == INPUT_CMD && v2[1].mNext != nullptr)
            Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
    }
    else
    {
    add_entry:
        BaseCmdFuncInfo* v3 = (BaseCmdFuncInfo*)_Z_MallocInternal(16);
        char* v4 = CopyStringInternal(cmd_name);
        BaseCmdFuncInfo* v5 = cmd_functions;
        v3->mName = v4;
        v3->mFuncType = CMD;
        v3->mNext = v5;
        v3[1].mNext = (BaseCmdFuncInfo*)function;
        cmd_functions = v3;
    }
}

// ============================================================================
// Cmd_AddInputCommand - ea: 0x60EA50
// ============================================================================
void Cmd_AddInputCommand(const char* cmd_name, void (*function)(int, int))
{
    if (cmd_name == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
        AeAssert::gCurrentLine = 824;
        AeAssert::gCurrentExpr = "cmd_name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BaseCmdFuncInfo* v2 = cmd_functions;
    if (cmd_functions != nullptr)
    {
        while (strcmp(cmd_name, v2->mName) != 0)
        {
            v2 = v2->mNext;
            if (v2 == nullptr)
                goto add_entry;
        }
        int mFuncType = v2->mFuncType;
        if (mFuncType == CMD && v2[1].mNext != nullptr
            || mFuncType == INPUT_CMD && v2[1].mNext != nullptr)
            Com_Printf("Cmd_AddCommand: %s already defined\n", cmd_name);
    }
    else
    {
    add_entry:
        BaseCmdFuncInfo* v3 = (BaseCmdFuncInfo*)_Z_MallocInternal(16);
        char* v4 = CopyStringInternal(cmd_name);
        BaseCmdFuncInfo* v5 = cmd_functions;
        v3->mName = v4;
        v3->mFuncType = INPUT_CMD;
        v3->mNext = v5;
        v3[1].mNext = (BaseCmdFuncInfo*)function;
        cmd_functions = v3;
    }
}

// ============================================================================
// GetCmd / Cmd_ExecuteServerString / Cmd_AddServerCommand
// ea: 0x61F730 / 0x61F320 / 0x61F4A0
// ============================================================================
extern char* g_text;  // ?g_text@@3PBDB (game.o @ 0xF3C458)
extern void Cmd_CallCmdFunctionWithInputArgs(BaseCmdFuncInfo* cmd);  // game.o 0x60E5B0
extern void Cbuf_AddServerText_f();  // game.o 0x60E5F0

// ea: 0x0061F730
const BaseCmdFuncInfo* GetCmd(const char* cmdName)
{
    BaseCmdFuncInfo** p_mNext = &cmd_functions;
    if (cmd_functions == nullptr)
        return nullptr;
    BaseCmdFuncInfo* v2;
    while (1)
    {
        v2 = *p_mNext;
        if (*p_mNext != nullptr)
        {
            const char* mName = v2->mName;
            if (cmdName != nullptr && mName != nullptr
                && ae_stricmpn(cmdName, mName, 0x7FFFFFFF) == 0)
                break;
        }
        p_mNext = &v2->mNext;
        if (v2->mNext == nullptr)
            return nullptr;
    }
    return v2;
}

// ea: 0x0061F320
void Cmd_ExecuteServerString(const char* text)
{
    g_text = (char*)text;
    Cmd_TokenizeString2(text, 0);
    if (cmd_argc != 0)
    {
        BaseCmdFuncInfo** p_mNext = &sv_cmd_functions;
        if (sv_cmd_functions != nullptr)
        {
            const char* v2 = cmd_argv[0];
            BaseCmdFuncInfo* v3;
            do
            {
                v3 = *p_mNext;
                const char* mName = (*p_mNext)->mName;
                if (v2 != nullptr && mName != nullptr)
                {
                    if (ae_stricmpn(v2, mName, 0x7FFFFFFF) == 0)
                    {
                        *p_mNext = v3->mNext;
                        v3->mNext = sv_cmd_functions;
                        sv_cmd_functions = v3;
                        Cmd_CallCmdFunctionWithInputArgs(v3);
                        return;
                    }
                    v2 = cmd_argv[0];
                }
                p_mNext = &v3->mNext;
            } while (v3->mNext != nullptr);
        }
    }
}

// ea: 0x0061F4A0
void Cmd_AddServerCommand(const char* cmd_name,
                          void (*function)())
{
    Cmd_AddCommand(cmd_name, Cbuf_AddServerText_f);
    BaseCmdFuncInfo* v2;
    BaseCmdFuncInfo* v3;
    if (sv_cmd_functions != nullptr)
    {
        v2 = sv_cmd_functions;
        while (strcmp(cmd_name, v2->mName) != 0)
        {
            v2 = v2->mNext;
            if (v2 == nullptr)
                goto LABEL_4;
        }
        if (function != nullptr)
            Com_Printf("Cmd_AddServerCommand: %s already defined\n",
                       cmd_name);
    }
    else
    {
    LABEL_4:
        v3 = (BaseCmdFuncInfo*)_Z_MallocInternal(16);
        char* v4 = CopyStringInternal(cmd_name);
        BaseCmdFuncInfo* v5 = sv_cmd_functions;
        v3->mName = v4;
        v3->mFuncType = CMD;
        v3->mNext = v5;
        v3->mFuncPtr = (void*)function;
        sv_cmd_functions = v3;
    }
}

// ============================================================================
// Cmd_ExecuteString / Cbuf_SV_Execute - ea: 0x61F640 / 0x61F3B0
// ============================================================================
extern int Cvar_Command();                // core.o
extern int CL_GameCommand();              // cl.o
extern int SV_GameCommand();              // sv.o
extern void CL_ForwardCommandToServer(const char* string);  // cl.o
extern cvar_t* com_cl_running;            // core.o
extern cvar_t* com_sv_running;            // core.o
extern int com_inServerFrame;             // core.o

// ea: 0x0061F640
void Cmd_ExecuteString(const char* text)
{
    g_text = (char*)text;
    Cmd_TokenizeString2(text, 0);
    if (cmd_argc == 0)
        return;
    BaseCmdFuncInfo** p_mNext = &cmd_functions;
    if (cmd_functions == nullptr)
        goto LABEL_9;
    const char* v2 = cmd_argv[0];
    while (1)
    {
        BaseCmdFuncInfo* v3 = *p_mNext;
        const char* mName = (*p_mNext)->mName;
        if (v2 != nullptr && mName != nullptr)
        {
            if (ae_stricmpn(v2, mName, 0x7FFFFFFF) == 0)
                break;
            v2 = cmd_argv[0];
        }
        p_mNext = &v3->mNext;
        if (v3->mNext == nullptr)
            goto LABEL_9;
    }
    BaseCmdFuncInfo* v3 = *p_mNext;
    *p_mNext = v3->mNext;
    v3->mNext = cmd_functions;
    cmd_functions = v3;
    int mFuncType = v3->mFuncType;
    if (mFuncType == CMD && v3[1].mFuncPtr != nullptr
        || mFuncType == INPUT_CMD && v3[1].mFuncPtr != nullptr)
    {
        Cmd_CallCmdFunctionWithInputArgs(v3);
        return;
    }
LABEL_9:
    if (Cvar_Command() == 0
        && (com_cl_running == nullptr || com_cl_running->integer == 0
            || CL_GameCommand() == 0)
        && (com_sv_running == nullptr || com_sv_running->integer == 0
            || SV_GameCommand() == 0))
    {
        CL_ForwardCommandToServer(text);
    }
}

// ea: 0x0061F3B0
void Cbuf_SV_Execute()
{
    int quoteCount = 0;
    if (com_sv_running == nullptr || com_sv_running->integer == 0
        || com_inServerFrame == 0)
    {
        while (sv_cmd_text.cmdsize != 0)
        {
            int v3 = 0;
            int i = sv_cmd_text.cmdsize;
            if (i > 0)
            {
                do
                {
                    unsigned char v4 = (unsigned char)sv_cmd_text.data[v3];
                    if (v4 == '"')
                        ++quoteCount;
                    if ((quoteCount & 1) == 0 && v4 == ';')
                        break;
                    if (v4 == '\n' || v4 == '\r')
                        break;
                    ++v3;
                } while (v3 < sv_cmd_text.cmdsize);
                if (v3 >= 4095)
                    v3 = 4095;
            }
            char line[4096];
            memcpy(line, sv_cmd_text.data, v3);
            int cmdsize = sv_cmd_text.cmdsize;
            quoteCount = 0;
            bool full = v3 == sv_cmd_text.cmdsize;
            line[v3] = 0;
            if (full)
            {
                sv_cmd_text.cmdsize = 0;
            }
            else
            {
                int v7 = v3 + 1;
                sv_cmd_text.cmdsize = cmdsize - v7;
                memmove(sv_cmd_text.data, &sv_cmd_text.data[v7],
                        cmdsize - v7);
            }
            Cmd_ExecuteServerString(line);
        }
    }
}

// ============================================================================
// Cmd_RemoveCommand - ea: 0x60EB50
// ============================================================================
// ea: 0x0060EB50
void Cmd_RemoveCommand(const char* cmd_name)
{
    BaseCmdFuncInfo* v1 = cmd_functions;
    BaseCmdFuncInfo** p_mNext = &cmd_functions;
    if (cmd_functions == nullptr)
        return;
    while (strcmp(cmd_name, v1->mName) != 0)
    {
        p_mNext = &v1->mNext;
        v1 = v1->mNext;
        if (v1 == nullptr)
            return;
    }
    *p_mNext = v1->mNext;
    if (v1->mName != nullptr)
        _Z_FreeInternal((void*)v1->mName);
    ButtonMgr_ClearBinding(v1, currCl);
    _Z_FreeInternal(v1);
}

// ============================================================================
// Cmd_Shutdown - ea: 0x60EBE0
// ============================================================================
// ea: 0x0060EBE0
void Cmd_Shutdown()
{
    for (BaseCmdFuncInfo* i = cmd_functions; cmd_functions != nullptr;
         i = cmd_functions)
    {
        cmd_functions = i->mNext;
        if (i->mName == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
            AeAssert::gCurrentLine = 884;
            AeAssert::gCurrentExpr = "cmd->mName";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        _Z_FreeInternal((void*)i->mName);
        ButtonMgr_ClearBinding(i, currCl);
        _Z_FreeInternal(i);
    }
    for (BaseCmdFuncInfo* j = sv_cmd_functions; sv_cmd_functions != nullptr;
         j = sv_cmd_functions)
    {
        sv_cmd_functions = j->mNext;
        if (j->mName == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cmd.cpp";
            AeAssert::gCurrentLine = 895;
            AeAssert::gCurrentExpr = "cmd->mName";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        _Z_FreeInternal((void*)j->mName);
        _Z_FreeInternal(j);
    }
}

// ============================================================================
// Cmd_CommandCompletion - ea: 0x60ECF0
// ============================================================================
// ea: 0x0060ECF0
void Cmd_CommandCompletion(void (*callback)(const char*))
{
    for (BaseCmdFuncInfo* i = cmd_functions; i != nullptr; i = i->mNext)
        callback(i->mName);
}

// ============================================================================
// Cmd_List_f - ea: 0x60ED20
// ============================================================================
// ea: 0x0060ED20
void Cmd_List_f()
{
    char* v0 = cmd_argc <= 1 ? nullptr : cmd_argv[1];
    BaseCmdFuncInfo* v1 = cmd_functions;
    int i = 0;
    for (; v1 != nullptr; v1 = v1->mNext)
    {
        if (v0 == nullptr
            || Com_Filter(v0, (char*)v1->mName, 0) != 0)
        {
            Com_Printf("%s\n", v1->mName);
            ++i;
        }
    }
    Com_Printf("%i commands\n", i);
}

// ============================================================================
// Sys_Time - ea: 0x60EE40
// ============================================================================
static bool first_time_0 = true;
static unsigned __int64 initial_time_0;

// ea: 0x0060EE40
double Sys_Time()
{
    unsigned int v0;
    unsigned int v1;
    if (first_time_0)
    {
        unsigned __int64 v3 = __rdtsc();
        v0 = (unsigned int)(v3 >> 32);
        v1 = (unsigned int)v3;
        initial_time_0 = v3;
        first_time_0 = false;
    }
    else
    {
        v0 = (unsigned int)(initial_time_0 >> 32);
        v1 = (unsigned int)initial_time_0;
    }
    unsigned __int64 now = __rdtsc();
    return (double)(now - (((unsigned __int64)v0 << 32) | v1))
        * 0.0000000013636364;
}

// ============================================================================
// DebugCurveRender / Sys_Print - empty stubs
// ============================================================================
// ea: 0x0060EEE0
void DebugCurveRender()
{
}

// ea: 0x0060F250
void Sys_Print()
{
}

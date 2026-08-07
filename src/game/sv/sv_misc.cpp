// ============================================================================
// sv_misc.cpp — remaining server functions (sv_game/sv_main/sv_world of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <string.h>
#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern void  Com_Error(int code, const char* fmt, ...);
extern int   Com_Milliseconds(void);
extern void  CM_AdjustAreaPortalState(int area1, int area2, int open);
extern void  CM_UnlinkEntity(EntityShared* ent);
extern DCGSet* TempBoxModel(const math::Position3* mins, const math::Position3* maxs, int contents, int capsule);
extern void  TraceXFormed(trace_t* results, const math::Position3* start, const math::Position3* end,
                          const math::Position3* mins, const math::Position3* maxs, DCGSet* model,
                          int brushmask, const math::Position3* origin, const math::Position3* angles, int capsule);
extern void  SCR_UpdateScreen(void);
extern void  j_nullsub_35(void);
static int   SV_InitGameVM(int restart, int savegame);   // ea: 0x520110
extern const math::Position3& Float4_Zero_2;
extern unsigned __int64 sLastTime_0;     // ?sLastTime_0  (sv_game.cpp static)
extern unsigned int _S8_40;              // ?$S8_40 (sv_game.cpp static)
extern int   SV_GameSystemCalls(int* args);
extern void  CL_ParseGamestate(Broc::string* configstrings);

// ============================================================================
// SV_SetCheckSum — ea: 0x51F300
// ============================================================================
void SV_SetCheckSum(int checksum) {
    sv.checksum = checksum;
}

// ============================================================================
// SV_RestartGameProgs — ea: 0x520170
// ============================================================================
int SV_RestartGameProgs(int savegame) {
    if (gvm == NULL) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1119;
        AeAssert::gCurrentExpr = "gvm";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    VM_Call(gvm, 1, savegame);
    return SV_InitGameVM(1, savegame);
}

// ============================================================================
// SV_InitGameProgs — ea: 0x5201E0
// ============================================================================
void SV_InitGameProgs(int savegame) {
    gvm = VM_Create("game", SV_GameSystemCalls);
    if (gvm == NULL)
        Com_Error(1, "\x15" "VM_Create on game failed");
    SV_InitGameVM(0, savegame);
}

// ============================================================================
// SV_InitGameVM — ea: 0x520110 (static)
// ============================================================================
static int SV_InitGameVM(int restart, int savegame) {
    j_nullsub_35();
    int checksum = sv.checksum;
    int v2 = Com_Milliseconds();
    int v3 = VM_Call(gvm, 0, v2, restart, savegame, checksum);
    j_nullsub_35();
    int v4 = 0;
    do {
        svs.clients[v4 / 0x1370].mEntityHandle.mHandle.mVal = 0;
        v4 += 4976;
    } while (v4 < 0x13700);
    return v3;
}

// ============================================================================
// SV_AdjustAreaPortalState — ea: 0x51EAF0
// ============================================================================
void SV_AdjustAreaPortalState(Entity* ent, int open) {
    int areanum2 = ent->r.areanum2;
    if (areanum2 != -1)
        CM_AdjustAreaPortalState(ent->r.areanum, areanum2, open);
}

// ============================================================================
// SV_inPVS — ea: 0x51EA50
// ============================================================================
int SV_inPVS(const math::Position3& p1, const math::Position3& p2) {
    (void)p1; (void)p2;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;   // JSV
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
    AeAssert::gCurrentLine = 130;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(defaultFileName))
        __debugbreak();
    return 1;
}

// ============================================================================
// SV_inPVSIgnorePortals — ea: 0x51EAA0
// ============================================================================
int SV_inPVSIgnorePortals(const float* const p1, const float* const p2) {
    (void)p1; (void)p2;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)3;   // JSV
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
    AeAssert::gCurrentLine = 250;
    AeAssert::gCurrentExpr = NULL;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(defaultFileName))
        __debugbreak();
    return 1;
}

// ============================================================================
// SV_CheckLoadLevel — ea: 0x51F200
// ============================================================================
void SV_CheckLoadLevel(int savegame) {
    if (++com_skelTimeStamp == 0)
        com_skelTimeStamp = 1;
    if (bSV_AllowedAllocSkel != 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1191;
        AeAssert::gCurrentExpr = "!bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 1;
    VM_Call(gvm, 9, savegame, sv.checksum);
    if (bSV_AllowedAllocSkel == 0) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
        AeAssert::gCurrentLine = 1197;
        AeAssert::gCurrentExpr = "bSV_AllowedAllocSkel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    bSV_AllowedAllocSkel = 0;
    com_time = VM_Call(gvm, 20);
}

// ============================================================================
// LoadingMenuCallback — ea: 0x51F640
// ============================================================================
void LoadingMenuCallback(float progress) {
    g_femanager.UpdateLoadingMenu(progress);
    unsigned __int64 v1;
    if ((_S8_40 & 1) != 0) {
        v1 = sLastTime_0;
    } else {
        _S8_40 |= 1u;
        v1 = __rdtsc();
        sLastTime_0 = v1;
    }
    unsigned __int64 v2 = __rdtsc();
    if ((v2 - v1) >= 24444444.0) {
        sLastTime_0 = v2;
        SCR_UpdateScreen();
    }
}

// ============================================================================
// GetShortName — ea: 0x51F710
// ============================================================================
const char* GetShortName(const char* long_name) {
    return long_name;
}

// ============================================================================
// SendClientThinkMsg — ea: 0x51F7D0
// ============================================================================
void SendClientThinkMsg() {
    if (svs.clients != NULL)
        VM_Call(gvm, 6, &svs.clients[currCl].mEntityHandle);
}

// ============================================================================
// MatrixTransposeTransformVector43 — ea: 0x51FB40
// ============================================================================
void MatrixTransposeTransformVector43(const math::Position3& in1, const float (*const in2)[3], math::Position3& out) {
    float v3 = in1.v.m128_f32[1] - (*in2)[10];
    float v4 = in1.v.m128_f32[2] - (*in2)[11];
    float v5 = in1.v.m128_f32[0] - (*in2)[9];
    out.v.m128_f32[0] = ((*in2)[2] * v4) + ((*in2)[1] * v3) + ((*in2)[0] * v5);
    out.v.m128_f32[1] = ((*in2)[5] * v4) + ((*in2)[4] * v3) + ((*in2)[3] * v5);
    out.v.m128_f32[2] = ((*in2)[8] * v4) + ((*in2)[7] * v3) + ((*in2)[6] * v5);
}

// ============================================================================
// SV_ClipHandleForEntity — ea: 0x51FC00
// ============================================================================
DCGSet* SV_ClipHandleForEntity(const Entity* ent) {
    DCGSet* result = ent->r.bmodel;
    if (result == NULL) {
        int contents = ent->r.contents;
        math::Position3* p_maxs = (math::Position3*)&ent->r.maxs;
        if ((ent->r.svFlags & 0x200) != 0)
            return TempBoxModel(&ent->r.mins, p_maxs, contents, 1);
        else
            return TempBoxModel(&ent->r.mins, p_maxs, contents, 0);
    }
    return result;
}

// ============================================================================
// SV_UnlinkEntity — ea: 0x51FC60
// ============================================================================
void SV_UnlinkEntity(Entity* gEnt) {
    gEnt->r.linked = 0;
    CM_UnlinkEntity(&gEnt->r);
}

// ============================================================================
// SV_EntityContact — ea: 0x520080
// ============================================================================
int SV_EntityContact(const math::Position3& mins, const math::Position3& maxs, const Entity* gEnt, int capsule) {
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    DCGSet* v5 = SV_ClipHandleForEntity(gEnt);
    tr.fraction = 1.0f;
    math::Position3 v9 = Float4_Zero_2;
    math::Position3 zero = Float4_Zero_2;
    TraceXFormed(&tr, &zero, &v9, &mins, &maxs, v5, -1, &gEnt->r.currentOrigin, &gEnt->r.currentAngles, capsule);
    return (int)(tr.fraction < 1.0f);
}

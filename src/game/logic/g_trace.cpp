// ============================================================================
// g_trace.cpp - game trace/sight wrappers (g.o: g_syscalls.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

// ea: 0x004508E0
void g_Trace(trace_t* results, const math::Position3* start, const math::Position3* mins,
             const math::Position3* maxs, const math::Position3* end,
             const collision_context_t* context)
{
    SV_Trace(results, start, mins, maxs, end, context, 0, 0, nullptr, 0, 0.0f);
}

// ea: 0x00450910
void g_TraceCapsule(trace_t* results, const math::Position3* start, const math::Position3* mins,
                    const math::Position3* maxs, const math::Position3* end,
                    const collision_context_t* context)
{
    SV_Trace(results, start, mins, maxs, end, context, 1, 0, nullptr, 0, 0.0f);
}

// ea: 0x00450940
void TraceDebugLine(const math::Position3* start, const math::Position3* end,
                    int hitNum, DbLinkedHandle<EntityHandleDb, Entity> entityHandle)
{
    if (g_drawDebugLos != 0 || (g_drawDebugEntityLos != 0 && g_debugThread.m_entityHandle.mHandle.mVal == entityHandle.mHandle.mVal))
    {
        if (hitNum != 0)
        {
            CL_AddDebugLine(start->v.m128_f32, end->v.m128_f32, colorRed, 1, 10, 1, 0);
            ++g_numLosHits;
        }
        else
        {
            CL_AddDebugLine(start->v.m128_f32, end->v.m128_f32, colorGreen, 1, 10, 1, 0);
            ++g_numLosMisses;
        }
    }
}

// ea: 0x004509C0
int g_SightTraceToEntity(const math::Position3* start, const math::Position3* mins,
                         const math::Position3* maxs, const math::Position3* end,
                         DbLinkedHandle<EntityHandleDb, Entity> entity,
                         const collision_context_t* context)
{
    return SV_SightTraceToEntity(start, mins, maxs, end, entity, context, 1);
}

// ea: 0x004509F0
void g_LocationalTrace(trace_t* results, const math::Position3* start,
                       const math::Position3* end, const collision_context_t* context,
                       unsigned char* priorityMap, float coneAngleTangent)
{
    math::Position3 zeroMaxs;
    math::Position3 zeroMins;
    zeroMaxs.v = _mm_setzero_ps();
    zeroMins.v = _mm_setzero_ps();
    SV_Trace(results, start, &zeroMins, &zeroMaxs, end, context, 0, 1, priorityMap, 1, coneAngleTangent);
}

// ea: 0x00450AC0
int g_EntityContact(const math::Position3* mins, const math::Position3* maxs, const Entity* ent)
{
    return SV_EntityContact(*mins, *maxs, ent, 0);
}

// ea: 0x00450AE0
int g_EntityContactCapsule(const math::Position3* mins, const math::Position3* maxs, const Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 237;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->r.maxs.v.m128_f32[0] < ent->r.mins.v.m128_f32[0])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 238;
        AeAssert::gCurrentExpr = "ent->r.maxs[0] >= ent->r.mins[0]";
        if (!AeAssert::IsIgnored())
        {
            const char* cls = ent->mClassName.mBlock != nullptr
                                  ? (const char*)(ent->mClassName.mBlock + 1)
                                  : &defaultFileName[0];
            char* msg = va("entnum: %d, origin: %g %g %g, classname: %s",
                           ent->mHandle.mHandle.mVal,
                           ent->r.currentOrigin.v.m128_f32[0],
                           ent->r.currentOrigin.v.m128_f32[1],
                           ent->r.currentOrigin.v.m128_f32[2],
                           cls);
            if (AeAssert::Assert(msg))
                __debugbreak();
        }
    }
    if (ent->r.maxs.v.m128_f32[1] < ent->r.mins.v.m128_f32[1])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 239;
        AeAssert::gCurrentExpr = "ent->r.maxs[1] >= ent->r.mins[1]";
        if (!AeAssert::IsIgnored())
        {
            const char* cls = ent->mClassName.mBlock != nullptr
                                  ? (const char*)(ent->mClassName.mBlock + 1)
                                  : &defaultFileName[0];
            char* msg = va("entnum: %d, origin: %g %g %g, classname: %s",
                           ent->mHandle.mHandle.mVal,
                           ent->r.currentOrigin.v.m128_f32[0],
                           ent->r.currentOrigin.v.m128_f32[1],
                           ent->r.currentOrigin.v.m128_f32[2],
                           cls);
            if (AeAssert::Assert(msg))
                __debugbreak();
        }
    }
    if (ent->r.maxs.v.m128_f32[2] < ent->r.mins.v.m128_f32[2])
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
        AeAssert::gCurrentLine = 240;
        AeAssert::gCurrentExpr = "ent->r.maxs[2] >= ent->r.mins[2]";
        if (!AeAssert::IsIgnored())
        {
            const char* cls = ent->mClassName.mBlock != nullptr
                                  ? (const char*)(ent->mClassName.mBlock + 1)
                                  : &defaultFileName[0];
            char* msg = va("entnum: %d, origin: %g %g %g, classname: %s",
                           ent->mHandle.mHandle.mVal,
                           ent->r.currentOrigin.v.m128_f32[0],
                           ent->r.currentOrigin.v.m128_f32[1],
                           ent->r.currentOrigin.v.m128_f32[2],
                           cls);
            if (AeAssert::Assert(msg))
                __debugbreak();
        }
    }
    return SV_EntityContact(*mins, *maxs, ent, 1);
}

// ea: 0x0045EBB0
void g_SightTrace(int* hitNum, const math::Position3* start, const math::Position3* mins,
                  const math::Position3* maxs, const math::Position3* end,
                  const collision_context_t* context)
{
    SV_SightTrace(hitNum, start, mins, maxs, end, context, 0);
    TraceDebugLine(start, end, *hitNum, context->pass_entity1);
}

// ea: 0x0045EBF0
void g_SightTraceCapsule(int* hitNum, const math::Position3* start, const math::Position3* mins,
                         const math::Position3* maxs, const math::Position3* end,
                         const collision_context_t* context)
{
    SV_SightTrace(hitNum, start, mins, maxs, end, context, 1);
    TraceDebugLine(start, end, *hitNum, context->pass_entity1);
}

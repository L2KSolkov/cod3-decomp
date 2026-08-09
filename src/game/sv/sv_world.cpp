// ============================================================================
// sv_world.cpp â€” server world collision (sv_world.cpp of sv.o)
// ============================================================================

#include "game/sv/sv_decl.h"
#include "game/sv/sv_stubs.h"

#include <string.h>
#include <math.h>
#include <stdlib.h>

extern "C" int __fpclass(float);

static bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// ============================================================================
// DObjTrace_s — DObj traceline results (fields used by SV_PointTraceToEntity)
// ============================================================================
struct DObjTrace_s {
    float    normal[3];      // +0x00
    float    fraction;       // +0x0C
    float    localHit[3];    // +0x10
    unsigned char allsolid;  // +0x1C
    unsigned char startsolid;// +0x1D
    // +0x1E pad
    unsigned char _pad[2];   // +0x1E
    HashString partName;     // +0x20
    uint32_t   partGroup;    // +0x24
    uint8_t    _pad2[0];     // +0x28 (end)
};

// ============================================================================
extern DCGSet*       SV_ClipHandleForEntity(const Entity* ent);
extern void          TraceXFormed(trace_t* results, const math::Position3* start, const math::Position3* end,
                                  const math::Position3* mins, const math::Position3* maxs, DCGSet* model,
                                  int brushmask, const math::Position3* origin, const math::Position3* angles, int capsule);
extern int           SightTraceXFormed(int hitNum, const math::Position3* start, const math::Position3* end,
                                       const math::Position3* mins, const math::Position3* maxs, DCGSet* model,
                                       int brushmask, const math::Position3* origin, const math::Position3* angles, int capsule);
extern const math::Position3& Float4_Zero_2;
extern const math::Position3& Float4_One_2;
extern float         threshold;
extern int           loc_800000;

// Collision-model helpers (unported game objects provide definitions later)
extern void          CM_LinkEntity(EntityShared* ent, const float* absmin, const float* absmax);
extern void          CM_UnlinkEntity(EntityShared* ent);
extern int           CM_BoxLeafnums(math::Vector4& cached_pos, int& cached_leaf,
                                    const math::Position3* pos, const math::Position3* mins,
                                    const math::Position3* maxs, int* list, int listsize, int* lastLeaf);
extern int           CM_LeafArea(int leafnum);
extern int           CM_LeafCluster(int leafnum);
extern int           R_CellForPoint(const math::Position3* pos);
extern float         RadiusFromBounds(const math::Position3* mins, const math::Position3* maxs);
extern void          DObjGetBounds(const DObj* obj, math::Position3& mins, math::Position3& maxs);
extern int           CM_TraceBox(const math::Position3* start, const math::Position3* end,
                                 const math::Position3* mins, const math::Position3* maxs, float fraction);
extern void          Trace(trace_t* results, const math::Position3* start, const math::Position3* end,
                           const math::Position3* mins, const math::Position3* maxs, DCGSet* model,
                           int brushmask, int capsule, const void* sphere);
extern void          TraceSphere(const proximity_data_t* data, trace_t* results,
                                 const math::Position3* start, const math::Position3* end,
                                 const math::Position3* mins, const math::Position3* maxs, int brushmask);
extern void          TracePoint(const proximity_data_t* data, trace_t* results,
                                const math::Position3* start, const math::Position3* end, int brushmask);
extern void          CM_PointTraceStaticModels(trace_t* results, const math::Position3* start,
                                               const math::Position3* end, const collision_context_t* context);
extern void          CM_PointTraceToEntities(pointtrace_t* clip, const collision_context_t* context);
extern void          CM_ClipMoveToEntities(moveclip_t* clip, const collision_context_t* context);
extern int           CM_PointSightTraceToEntities(sightpointtrace_t* clip, const collision_context_t* context);
extern int           CM_ClipSightTraceToEntities(sightclip_t* clip, const collision_context_t* context);
extern int           SightTrace(int oldHitNum, const math::Position3* start, const math::Position3* end,
                                const math::Position3* mins, const math::Position3* maxs, DCGSet* model,
                                const math::Position3* origin, int brushmask, int capsule, void* sphere);
extern int           CM_PointContents(const math::Position3* p, DCGSet* model);
extern int           CM_AreaEntities(const math::Position3* mins, const math::Position3* maxs,
                                     DbLinkedHandle<EntityHandleDb, Entity>* entityList, int maxcount, int contentmask);
extern DCGSet*       TempBoxModel(const math::Position3* mins, const math::Position3* maxs, int contents, int capsule);
extern int           CM_TransformedPointContents(const math::Position3* p, DCGSet* model,
                                                 const math::Position3* origin, const math::Position3* angles);
extern int           DObjHasContents(const DObj* obj, int contentmask);
extern void          DObjGeomTraceline(const DObj* obj, const math::Position3* localStart,
                                       const math::Position3* localEnd, int contentmask,
                                       struct DObjTrace_s* results, float extraDistanceCheck);
extern void          DObjTraceline(const DObj* obj, const math::Position3* start,
                                   const math::Position3* end, unsigned char* priorityMap,
                                   struct DObjTrace_s* trace, float extraDistanceCheck);
extern void          AnglesToAxis(const math::Position3* angles, float axis[3][3]);
extern void          MatrixTransformVector(const float* in1, const float (*in2)[3], float* out);
extern void          MatrixTransposeTransformVector43(const math::Position3* in1, const float (*in2)[3],
                                                      math::Position3* out);
extern int           VM_Call(vm_s* vm, int callnum, ...);
extern void          ValidatePakId(TPakId pakId);
extern unsigned int  HashString_CalcHash(const char* str);
extern TPakId        CurPakId(void);
extern DCGSet*       ClipHandleToDCGSet(TPakId pakId, int handle);
extern void          CM_ModelBounds(DCGSet* model, math::Position3& mins, math::Position3& maxs);
extern int           DCGSet_get_contents(const DCGSet* model);
extern void          SV_LinkEntity(Entity* gEnt);
extern void          Com_DPrintf(const char* fmt, ...);
extern EntityManager* EntityManager_sInst(void);

// Handle db dereference helper (reconstructed from IDA operator* / operator->)
static Entity* HandleDbDeref(const DbLinkedHandle<EntityHandleDb, Entity>& h) {
    unsigned int mVal = h.mHandle.mVal;
    unsigned int idx = mVal & 0xFFF;
    if (idx < 0x540 && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[idx].mKey)
        return EntityHandleDb::sInst.mElements[idx].mObject;
    return NULL;
}

// sv_world.cpp statics
extern math::Position3 actorProneLocationalMins;
extern math::Position3 actorProneLocationalMaxs;
extern math::Position3 actorCrouchLocationalMins;
extern math::Position3 actorCrouchLocationalMaxs;
extern math::Position3 actorLocationalMins;
extern math::Position3 actorLocationalMaxs;

// ============================================================================
// DObjTrace_s Ã¢â‚¬â€ DObj traceline results (fields used by SV_PointTraceToEntity)
// ============================================================================

// ============================================================================
// SV_LinkEntity Ã¢â‚¬â€ ea: 0x521550
// ============================================================================
void SV_LinkEntity(Entity* gEnt) {
    math::Position3* p_currentOrigin = &gEnt->r.currentOrigin;
    if (IS_NAN(gEnt->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(gEnt->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(gEnt->r.currentOrigin.v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 80;
        AeAssert::gCurrentExpr = "!IS_NAN((gEnt->r.currentOrigin)[0]) && !IS_NAN((gEnt->r.currentOrigin)[1]) && !IS_NAN((gEnt->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    unsigned int v3 = gEnt->mHandle.mHandle.mVal & 0xFFF;
    Entity* mObject = NULL;
    if (v3 < 0x540 && gEnt->mHandle.mHandle.mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v3].mKey)
        mObject = EntityHandleDb::sInst.mElements[v3].mObject;
    if (gEnt != mObject) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 82;
        AeAssert::gCurrentExpr = "gEnt == *( gEnt->GetHandle() )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bad/Deleted entity in world sector."))
            __debugbreak();
    }
    if (gEnt->mHandle.mHandle.mVal != 0) {
        if (gEnt->r.bmodel != NULL) {
            gEnt->s.solid = (uint16_t)0xFFFFFF;
        } else if ((gEnt->r.contents & 0x2000001) != 0) {
            int v5 = (int)gEnt->r.maxs.v.m128_f32[0];
            if (v5 >= 1) {
                if (v5 > 255)
                    v5 = 255;
            } else {
                v5 = 1;
            }
            int v6 = (int)(1.0f - gEnt->r.mins.v.m128_f32[2]);
            if (v6 >= 1) {
                if (v6 > 255)
                    v6 = 255;
            } else {
                v6 = 1;
            }
            int v7 = (int)(gEnt->r.maxs.v.m128_f32[2] + 32.0f);
            if (v7 >= 1) {
                if (v7 > 255)
                    v7 = 255;
            } else {
                v7 = 1;
            }
            gEnt->s.solid = (uint16_t)(v5 | ((v6 | (v7 << 8)) << 8));
        } else {
            gEnt->s.solid = 0;
        }
        math::Position3* v9;
        math::Position3* v10;
        if (gEnt->r.bmodel == NULL
            || (gEnt->r.currentAngles.v.m128_f32[0] == 0.0f
                && gEnt->r.currentAngles.v.m128_f32[1] == 0.0f
                && gEnt->r.currentAngles.v.m128_f32[2] == 0.0f)) {
            gEnt->r.absmin.v.m128_f32[0] = p_currentOrigin->v.m128_f32[0] + gEnt->r.mins.v.m128_f32[0];
            gEnt->r.absmin.v.m128_f32[1] = p_currentOrigin->v.m128_f32[1] + gEnt->r.mins.v.m128_f32[1];
            gEnt->r.absmin.v.m128_f32[2] = p_currentOrigin->v.m128_f32[2] + gEnt->r.mins.v.m128_f32[2];
            gEnt->r.absmax.v.m128_f32[0] = p_currentOrigin->v.m128_f32[0] + gEnt->r.maxs.v.m128_f32[0];
            gEnt->r.absmax.v.m128_f32[1] = p_currentOrigin->v.m128_f32[1] + gEnt->r.maxs.v.m128_f32[1];
            gEnt->r.absmax.v.m128_f32[2] = p_currentOrigin->v.m128_f32[2] + gEnt->r.maxs.v.m128_f32[2];
            v9 = &gEnt->r.absmin;
            v10 = &gEnt->r.absmax;
        } else {
            float v8 = RadiusFromBounds(&gEnt->r.mins, &gEnt->r.maxs);
            gEnt->r.absmin.v.m128_f32[0] = p_currentOrigin->v.m128_f32[0] - v8;
            gEnt->r.absmax.v.m128_f32[0] = v8 + p_currentOrigin->v.m128_f32[0];
            gEnt->r.absmin.v.m128_f32[1] = gEnt->r.currentOrigin.v.m128_f32[1] - v8;
            gEnt->r.absmax.v.m128_f32[1] = v8 + gEnt->r.currentOrigin.v.m128_f32[1];
            gEnt->r.absmin.v.m128_f32[2] = gEnt->r.currentOrigin.v.m128_f32[2] - v8;
            gEnt->r.absmax.v.m128_f32[2] = v8 + gEnt->r.currentOrigin.v.m128_f32[2];
            v9 = &gEnt->r.absmin;
            v10 = &gEnt->r.absmax;
        }
        v9->v.m128_f32[0] = v9->v.m128_f32[0] - 1.0f;
        gEnt->r.absmin.v.m128_f32[1] = gEnt->r.absmin.v.m128_f32[1] - 1.0f;
        gEnt->r.absmin.v.m128_f32[2] = gEnt->r.absmin.v.m128_f32[2] - 1.0f;
        v10->v.m128_f32[0] = v10->v.m128_f32[0] + 1.0f;
        gEnt->r.absmax.v.m128_f32[1] = gEnt->r.absmax.v.m128_f32[1] + 1.0f;
        gEnt->r.absmax.v.m128_f32[2] = gEnt->r.absmax.v.m128_f32[2] + 1.0f;
        gEnt->r.numClusters = 0;
        gEnt->r.lastCluster = 0;
        gEnt->r.areanum = -1;
        gEnt->r.areanum2 = -1;
        int leafs[128];
        int lastLeaf = 0;
        int v19 = CM_BoxLeafnums(gEnt->r.pos_cache, gEnt->r.lastLeaf, p_currentOrigin,
                                 &gEnt->r.absmin, &gEnt->r.absmax, leafs, 128, &lastLeaf);
        if (v19 != 0) {
            for (int i = 0; i < v19; ++i) {
                int v20 = CM_LeafArea(leafs[i]);
                if (v20 != -1) {
                    if (gEnt->r.areanum == -1 || gEnt->r.areanum == v20) {
                        gEnt->r.areanum = v20;
                    } else {
                        if (gEnt->r.areanum2 != -1 && gEnt->r.areanum2 != v20 && sv.state == SS_LOADING)
                            Com_DPrintf("Object %i touching 3 areas at %f %f %f\n",
                                        gEnt->mHandle.mHandle.mVal,
                                        gEnt->r.absmin.v.m128_f32[0],
                                        gEnt->r.absmin.v.m128_f32[1],
                                        gEnt->r.absmin.v.m128_f32[2]);
                        gEnt->r.areanum2 = v20;
                    }
                }
            }
            gEnt->r.numClusters = 0;
            int c;
            for (c = 0; c < v19; ++c) {
                int v23 = CM_LeafCluster(leafs[c]);
                if (v23 != -1) {
                    gEnt->r.clusternums[gEnt->r.numClusters] = v23;
                    int v24 = gEnt->r.numClusters + 1;
                    gEnt->r.numClusters = v24;
                    if (v24 == 16)
                        break;
                }
            }
            if (c != v19)
                gEnt->r.lastCluster = CM_LeafCluster(lastLeaf);
            gEnt->r.linked = 1;
            math::Position3 v50;
            v50.v = _mm_add_ps(p_currentOrigin->v,
                               _mm_mul_ps(_mm_set1_ps(20.0f), _mm_set1_ps(1.0f)));
            int16_t cell = R_CellForPoint(&v50);
            gEnt->cell_index = cell;
            if (cell < 0)
                gEnt->cell_index = R_CellForPoint(p_currentOrigin);
            if (gEnt->r.contents != 0) {
                DObj* mDObj = gEnt->mDObj;
                if (mDObj != NULL && ((gEnt->r.svFlags & 0x18) != 0)) {
                    if ((gEnt->r.svFlags & 8) != 0) {
                        math::Position3 mins;
                        math::Position3 maxs;
                        if (gEnt->r.maxs.v.m128_f32[2] == 30.0f) {
                            mins.v = _mm_add_ps(p_currentOrigin->v, actorProneLocationalMins.v);
                            maxs.v = _mm_add_ps(p_currentOrigin->v, actorProneLocationalMaxs.v);
                        } else if (gEnt->r.maxs.v.m128_f32[2] == 50.0f) {
                            mins.v = _mm_add_ps(p_currentOrigin->v, actorCrouchLocationalMins.v);
                            maxs.v = _mm_add_ps(p_currentOrigin->v, actorCrouchLocationalMaxs.v);
                        } else {
                            mins.v = _mm_add_ps(p_currentOrigin->v, actorLocationalMins.v);
                            maxs.v = _mm_add_ps(p_currentOrigin->v, actorLocationalMaxs.v);
                        }
                        gEnt->r.absmin.v = mins.v;
                        gEnt->r.absmax.v = maxs.v;
                    } else {
                        math::Position3 mins;
                        math::Position3 maxs;
                        DObjGetBounds(mDObj, mins, maxs);
                        gEnt->r.absmin.v = _mm_min_ps(_mm_add_ps(mins.v, p_currentOrigin->v), gEnt->r.absmin.v);
                        gEnt->r.absmax.v = _mm_max_ps(_mm_add_ps(maxs.v, p_currentOrigin->v), gEnt->r.absmax.v);
                    }
                    CM_LinkEntity(&gEnt->r, (const float*)&gEnt->r.absmin, (const float*)&gEnt->r.absmax);
                } else {
                    CM_LinkEntity(&gEnt->r, (const float*)&gEnt->r.absmin, (const float*)&gEnt->r.absmax);
                }
            } else {
                CM_UnlinkEntity(&gEnt->r);
            }
        } else {
            CM_UnlinkEntity(&gEnt->r);
        }
    }
}

// ============================================================================
// SV_PointTraceToEntity Ã¢â‚¬â€ ea: 0x521E10
// ============================================================================
void SV_PointTraceToEntity(pointtrace_t* clip, EntityShared* check) {
    const Entity* p_currentOrigin = (const Entity*)((char*)check - 0xE0);
    if ((clip->contentmask & check->contents) != 0) {
        unsigned int mVal = clip->mPassEntity.mHandle.mVal;
        unsigned int v5 = mVal & 0xFFF;
        if (v5 >= 0x540
            || mVal >> 12 != (unsigned int)EntityHandleDb::sInst.mElements[v5].mKey
            || EntityHandleDb::sInst.mElements[v5].mObject == NULL
            || (p_currentOrigin->mHandle.mHandle.mVal != mVal
                && (unsigned int)p_currentOrigin->r.mOwner.mHandle.mVal != mVal
                && ((unsigned int)p_currentOrigin->r.mOwner.mHandle.mVal != clip->mPassOwner.mHandle.mVal
                    || HandleDbDeref(clip->mPassOwner) == NULL))) {
            int bLocational = clip->bLocational;
            DObj* mDObj = p_currentOrigin->mDObj;
            if (bLocational != 0 && mDObj != NULL && ((p_currentOrigin->r.svFlags & 0x18) != 0)) {
                math::Position3 v27;
                math::Position3 v28;
                math::Position3 absmax;
                math::Position3 absmin;
                DObjTrace_s objTrace;
                memset(&objTrace, 0, sizeof(objTrace));
                float v29[3];
                float localEnd[3];
                if ((p_currentOrigin->r.svFlags & 0x10) != 0) {
                    if (DObjHasContents(mDObj, clip->contentmask) == 0)
                        return;
                    v27.v = p_currentOrigin->r.currentOrigin.v;
                    DObjGetBounds(mDObj, absmin, absmax);
                } else {
                    v27.v = p_currentOrigin->r.currentOrigin.v;
                    absmin.v.m128_f32[0] = actorLocationalMins.v.m128_f32[0] + v27.v.m128_f32[0];
                    absmin.v.m128_f32[1] = actorLocationalMins.v.m128_f32[1] + v27.v.m128_f32[1];
                    absmin.v.m128_f32[2] = actorLocationalMins.v.m128_f32[2] + v27.v.m128_f32[2];
                    absmax.v.m128_f32[0] = actorLocationalMaxs.v.m128_f32[0] + v27.v.m128_f32[0];
                    absmax.v.m128_f32[1] = actorLocationalMaxs.v.m128_f32[1] + v27.v.m128_f32[1];
                    absmax.v.m128_f32[2] = actorLocationalMaxs.v.m128_f32[2] + v27.v.m128_f32[2];
                    if (CM_TraceBox(&clip->start, &clip->end, &absmin, &absmax, clip->trace.fraction) != 0)
                        return;
                }
                int handle = p_currentOrigin->mHandle.mHandle.mVal;
                VM_Call(gvm, 19, &handle);
                AnglesToAxis(&p_currentOrigin->r.currentAngles, (float(*)[3])v29);
                MatrixTransposeTransformVector43(&clip->start, (const float(*)[3])v29, &absmin);
                MatrixTransposeTransformVector43(&clip->end, (const float(*)[3])v29, &absmax);
                float v35 = 0.0f;
                if (clip->mAngleTangent > 0.0f)
                    v35 = sqrtf(absmin.v.m128_f32[0] * absmin.v.m128_f32[0]
                                + absmin.v.m128_f32[1] * absmin.v.m128_f32[1]
                                + absmin.v.m128_f32[2] * absmin.v.m128_f32[2]) * clip->mAngleTangent;
                if ((p_currentOrigin->r.svFlags & 0x10) != 0) {
                    // locational: find first model with >1 bones, else box reject
                    int numModels = *(unsigned char*)((char*)mDObj + 206);
                    int found = 0;
                    if (numModels != 0) {
                        int idx = 0;
                        unsigned char* p = (unsigned char*)mDObj + 128;
                        for (; idx < numModels; ++idx, p += 8) {
                            IVPointer<XModel>* iv = (IVPointer<XModel>*)p;
                            if (IVPointer_IsValid(*iv)) {
                                XModel* xm = IVPointer_Deref(*iv);
                                if (XModel::GetNumBones(xm, xm->collLod) > 1) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                    }
                    if (!found) {
                        if (CM_TraceBox(&absmin, &absmax, &absmin, &absmax, clip->trace.fraction) != 0)
                            return;
                    }
                    DObjGeomTraceline(mDObj, &absmin, &absmax, clip->contentmask, &objTrace, v35);
                } else {
                    DObjTraceline(mDObj, &absmin, &absmax, clip->priorityMap, &objTrace, v35);
                }
                if (objTrace.fraction >= clip->trace.fraction) {
                    clip->trace.allsolid |= objTrace.allsolid;
                    clip->trace.startsolid = (unsigned char)(objTrace.startsolid | clip->trace.startsolid);
                    return;
                }
                if (objTrace.fraction >= 1.0f) {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
                    AeAssert::gCurrentLine = 469;
                    AeAssert::gCurrentExpr = "objTrace.fraction < 1.0f";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("%f", objTrace.fraction))
                        __debugbreak();
                }
                math::Position3 localHit;
                localHit.v.m128_f32[0] = objTrace.localHit[0];
                localHit.v.m128_f32[1] = objTrace.localHit[1];
                localHit.v.m128_f32[2] = objTrace.localHit[2];
                float normal[3];
                normal[0] = objTrace.normal[0];
                normal[1] = objTrace.normal[1];
                normal[2] = objTrace.normal[2];
                MatrixTransformVector(localHit.v.m128_f32, (const float(*)[3])v29, normal);
                math::Position3 endpos;
                endpos.v = _mm_add_ps(clip->start.v,
                                      _mm_mul_ps(_mm_sub_ps(clip->end.v, clip->start.v),
                                                 _mm_set1_ps(objTrace.fraction)));
                clip->trace.endpos = endpos;
                clip->trace.normal.v.m128_f32[0] = normal[0];
                clip->trace.normal.v.m128_f32[1] = normal[1];
                clip->trace.normal.v.m128_f32[2] = normal[2];
                clip->trace.fraction = objTrace.fraction;
                clip->trace.allsolid = objTrace.allsolid;
                clip->trace.startsolid = objTrace.startsolid;
                clip->trace.partName = objTrace.partName;
                clip->trace.partGroup = objTrace.partGroup;
                clip->trace.mEntity.mHandle.mVal = p_currentOrigin->mHandle.mHandle.mVal;
                clip->trace.contents = p_currentOrigin->r.contents;
            } else {
                DCGSet* v19 = SV_ClipHandleForEntity(p_currentOrigin);
                math::Position3 angles;
                const math::Position3* p_currentAngles;
                if (p_currentOrigin->r.bmodel != NULL) {
                    p_currentAngles = &p_currentOrigin->r.currentAngles;
                } else {
                    angles.v = Float4_Zero_2.v;
                    p_currentAngles = &angles;
                }
                trace_t tr;
                memset(&tr, 0, sizeof(tr));
                tr.fraction = clip->trace.fraction;
                TraceXFormed(&tr, &clip->start, &clip->end, &angles, &angles,
                             v19, clip->contentmask, &p_currentOrigin->r.currentOrigin,
                             p_currentAngles, 0);
                if (tr.fraction >= clip->trace.fraction) {
                    clip->trace.allsolid |= tr.allsolid;
                    clip->trace.startsolid = (unsigned char)(tr.startsolid | clip->trace.startsolid);
                    return;
                }
                clip->trace = tr;
                clip->trace.mEntity.mHandle.mVal = p_currentOrigin->mHandle.mHandle.mVal;
                clip->trace.contents = p_currentOrigin->r.contents;
            }
        }
    }
}

// ============================================================================
// SV_Trace Ã¢â‚¬â€ ea: 0x522720
// ============================================================================
void SV_Trace(trace_t* results, const math::Position3* start, const math::Position3* mins,
              const math::Position3* maxs, const math::Position3* end,
              const collision_context_t* context, int capsule, int bLocational,
              unsigned char* priorityMap, int staticmodels, float coneAngleTangent) {
    if (IS_NAN(start->v.m128_f32[0])
        || IS_NAN(start->v.m128_f32[1])
        || IS_NAN(start->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 618;
        AeAssert::gCurrentExpr = "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end->v.m128_f32[0])
        || IS_NAN(end->v.m128_f32[1])
        || IS_NAN(end->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 619;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(mins->v.m128_f32[0])
        || IS_NAN(mins->v.m128_f32[1])
        || IS_NAN(mins->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 620;
        AeAssert::gCurrentExpr = "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(maxs->v.m128_f32[0])
        || IS_NAN(maxs->v.m128_f32[1])
        || IS_NAN(maxs->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 621;
        AeAssert::gCurrentExpr = "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    tr.decal_radius = results->decal_radius;
    tr.check_decal = results->check_decal;
    Trace(&tr, start, end, mins, maxs, NULL, context->contentmask, capsule, NULL);
    if (tr.fraction == 1.0f) {
        tr.mEntity.mHandle.mVal = 0;
    } else {
        tr.mEntity.mHandle.mVal = EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
    }
    if (tr.fraction == 0.0f) {
        *results = tr;
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 634;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 635;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 636;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    } else {
        if (staticmodels != 0) {
            CM_PointTraceStaticModels(&tr, start, end, context);
            if (tr.fraction < 1.0f) {
                tr.mEntity.mHandle.mVal = EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
            }
        }
        float v9 = ((((maxs->v.m128_f32[0] - mins->v.m128_f32[0]) + maxs->v.m128_f32[1]) - mins->v.m128_f32[1])
                    + maxs->v.m128_f32[2]) - mins->v.m128_f32[2];
        if (v9 == 0.0f) {
                pointtrace_t clip;
                memset(&clip, 0, sizeof(clip));
                clip.start = *start;
                clip.end = *end;
                clip.trace = tr;
                clip.mPassEntity.mHandle.mVal = context->pass_entity1.mHandle.mVal;
                clip.contentmask = context->contentmask;
                clip.bLocational = bLocational;
                clip.priorityMap = priorityMap;
                clip.mAngleTangent = coneAngleTangent;
                Entity* e1 = HandleDbDeref(context->pass_entity1);
                if (e1 != NULL)
                    clip.mPassOwner.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
                CM_PointTraceToEntities(&clip, context);
                *results = clip.trace;
        } else {
            if (bLocational != 0) {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
                AeAssert::gCurrentLine = 673;
                AeAssert::gCurrentExpr = "!bLocational";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            moveclip_t clip;
            memset(&clip, 0, sizeof(clip));
            clip.trace = tr;
            clip.mPassEntity.mHandle.mVal = context->pass_entity1.mHandle.mVal;
            clip.contentmask = context->contentmask;
            clip.capsule = capsule;
            Entity* e1 = HandleDbDeref(context->pass_entity1);
            if (e1 != NULL)
                clip.mPassOwner.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
            clip.mins.v = _mm_mul_ps(_mm_sub_ps(maxs->v, mins->v), _mm_set1_ps(0.5f));
            clip.maxs.v = _mm_add_ps(maxs->v, mins->v);
            clip.start.v = _mm_add_ps(start->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
            clip.end.v = _mm_add_ps(end->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
            clip.outerSize.v = _mm_xor_ps(_mm_set1_ps(-0.0f), clip.mins.v);
            CM_ClipMoveToEntities(&clip, context);
            if ((unsigned int)clip.trace.partGroup > 0x12u) {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
                AeAssert::gCurrentLine = 694;
                AeAssert::gCurrentExpr = "( clip.trace.partGroup >= HITLOC_NONE ) && ( clip.trace.partGroup < HITLOC_NUM )";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            if (clip.trace.fraction > tr.fraction)
                clip.trace.endpos.v = _mm_add_ps(start->v,
                                                 _mm_mul_ps(_mm_sub_ps(end->v, start->v),
                                                            _mm_set1_ps(tr.fraction)));
            *results = clip.trace;
        }
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 704;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 705;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 706;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
}

// ============================================================================
// SV_SightTrace Ã¢â‚¬â€ ea: 0x523020
// ============================================================================
void SV_SightTrace(int* hit, const math::Position3* start, const math::Position3* mins,
                   const math::Position3* maxs, const math::Position3* end,
                   const collision_context_t* context, int capsule) {
    if (IS_NAN(start->v.m128_f32[0])
        || IS_NAN(start->v.m128_f32[1])
        || IS_NAN(start->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 720;
        AeAssert::gCurrentExpr = "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end->v.m128_f32[0])
        || IS_NAN(end->v.m128_f32[1])
        || IS_NAN(end->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 721;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(mins->v.m128_f32[0])
        || IS_NAN(mins->v.m128_f32[1])
        || IS_NAN(mins->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 722;
        AeAssert::gCurrentExpr = "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(maxs->v.m128_f32[0])
        || IS_NAN(maxs->v.m128_f32[1])
        || IS_NAN(maxs->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 723;
        AeAssert::gCurrentExpr = "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    math::Position3 zero;
    zero.v = Float4_Zero_2.v;
    *hit = SightTrace(*hit, start, end, mins, maxs, NULL, &zero, context->contentmask, capsule, NULL);
    if (*hit == 0) {
        float v9 = ((((maxs->v.m128_f32[0] - mins->v.m128_f32[0]) + maxs->v.m128_f32[1]) - mins->v.m128_f32[1])
                    + maxs->v.m128_f32[2]) - mins->v.m128_f32[2];
        if (v9 == 0.0f) {
            sightpointtrace_t clip;
            memset(&clip, 0, sizeof(clip));
            clip.start = *start;
            clip.end = *end;
            clip.mPassEntity1.mHandle.mVal = context->pass_entity1.mHandle.mVal;
            clip.mPassEntity2.mHandle.mVal = context->pass_entity2.mHandle.mVal;
            clip.contentmask = context->contentmask;
            Entity* e1 = HandleDbDeref(context->pass_entity1);
            if (e1 != NULL)
                clip.mPassOwner1.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
            Entity* e2 = HandleDbDeref(context->pass_entity2);
            if (e2 != NULL)
                clip.mPassOwner2.mHandle.mVal = e2->r.mOwner.mHandle.mVal;
            *hit = CM_PointSightTraceToEntities(&clip, context);
        } else {
            sightclip_t clip;
            memset(&clip, 0, sizeof(clip));
            clip.start = *start;
            clip.end = *end;
            clip.mPassEntity1.mHandle.mVal = context->pass_entity1.mHandle.mVal;
            clip.mPassEntity2.mHandle.mVal = context->pass_entity2.mHandle.mVal;
            clip.contentmask = context->contentmask;
            clip.capsule = capsule;
            Entity* e1 = HandleDbDeref(context->pass_entity1);
            if (e1 != NULL)
                clip.mPassOwner1.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
            Entity* e2 = HandleDbDeref(context->pass_entity2);
            if (e2 != NULL)
                clip.mPassOwner2.mHandle.mVal = e2->r.mOwner.mHandle.mVal;
            clip.mins.v = _mm_mul_ps(_mm_sub_ps(maxs->v, mins->v), _mm_set1_ps(0.5f));
            clip.maxs.v = _mm_add_ps(maxs->v, mins->v);
            clip.start.v = _mm_add_ps(start->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
            clip.end.v = _mm_add_ps(end->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
            clip.outerSize.v = _mm_xor_ps(_mm_set1_ps(-0.0f), clip.mins.v);
            *hit = CM_ClipSightTraceToEntities(&clip, context);
        }
    }
}

// ============================================================================
// SV_SightTraceToEntity Ã¢â‚¬â€ ea: 0x5234F0
// ============================================================================
int SV_SightTraceToEntity(const math::Position3* start, const math::Position3* mins,
                          const math::Position3* maxs, const math::Position3* end,
                          DbLinkedHandle<EntityHandleDb, Entity> entity,
                          const collision_context_t* context, int capsule) {
    unsigned int v8 = entity.mHandle.mVal & 0xFFF;
    Entity* mObject = NULL;
    if (v8 < 0x540 && entity.mHandle.mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v8].mKey)
        mObject = EntityHandleDb::sInst.mElements[v8].mObject;
    if ((mObject->r.contents & context->contentmask) == 0)
        return 0;
    if (IS_NAN(start->v.m128_f32[0])
        || IS_NAN(start->v.m128_f32[1])
        || IS_NAN(start->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 799;
        AeAssert::gCurrentExpr = "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end->v.m128_f32[0])
        || IS_NAN(end->v.m128_f32[1])
        || IS_NAN(end->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 800;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(mins->v.m128_f32[0])
        || IS_NAN(mins->v.m128_f32[1])
        || IS_NAN(mins->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 801;
        AeAssert::gCurrentExpr = "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(maxs->v.m128_f32[0])
        || IS_NAN(maxs->v.m128_f32[1])
        || IS_NAN(maxs->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 802;
        AeAssert::gCurrentExpr = "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    math::Position3 minv;
    minv.v = _mm_min_ps(start->v, end->v);
    math::Position3 maxv;
    maxv.v = _mm_max_ps(start->v, end->v);
    if ((_mm_movemask_ps(_mm_cmplt_ps(
             _mm_max_ps(
                 _mm_sub_ps(_mm_sub_ps(_mm_add_ps(minv.v, mins->v), Float4_One_2.v), mObject->r.absmax.v),
                 _mm_sub_ps(mObject->r.absmin.v, _mm_add_ps(_mm_add_ps(maxv.v, maxs->v), Float4_One_2.v))),
             Float4_Zero_2.v)) & 7) == 7) {
        DCGSet* v10 = SV_ClipHandleForEntity(mObject);
        const math::Position3* p_currentAngles;
        math::Position3 zero;
        if (mObject->r.bmodel == NULL) {
            zero.v = Float4_Zero_2.v;
            p_currentAngles = &zero;
        } else {
            p_currentAngles = &mObject->r.currentAngles;
        }
        if (SightTraceXFormed(0, start, end, mins, maxs, v10, context->contentmask,
                              &mObject->r.currentOrigin, p_currentAngles, capsule) != 0)
            return -1;
    }
    return 0;
}

// ============================================================================
// SV_PointContents Ã¢â‚¬â€ ea: 0x523850
// ============================================================================
int SV_PointContents(const math::Position3* p, const collision_context_t* context) {
    DbLinkedHandle<EntityHandleDb, Entity> touch[256];
    memset(touch, 0, sizeof(touch));
    int contents = CM_PointContents(p, NULL);
    int num = CM_AreaEntities(p, p, touch, 256, context->contentmask);
    for (int i = 0; i < num; ++i) {
        unsigned int mVal = touch[i].mHandle.mVal;
        if (mVal != context->pass_entity1.mHandle.mVal) {
            unsigned int v6 = mVal & 0xFFF;
            Entity* mObject = NULL;
            if (v6 < 0x540 && mVal >> 12 == (unsigned int)EntityHandleDb::sInst.mElements[v6].mKey)
                mObject = EntityHandleDb::sInst.mElements[v6].mObject;
            DCGSet* bmodel = mObject->r.bmodel;
            if (bmodel == NULL)
                bmodel = TempBoxModel(&mObject->r.mins, &mObject->r.maxs, mObject->r.contents,
                                      (mObject->r.svFlags & 0x200) != 0 ? 1 : 0);
            contents |= CM_TransformedPointContents(p, bmodel, &mObject->r.currentOrigin, &mObject->r.currentAngles);
        }
    }
    return contents & context->contentmask;
}

// ============================================================================
// TraceSphereFull Ã¢â‚¬â€ ea: 0x523960
// ============================================================================
void TraceSphereFull(const proximity_data_t* proximity_data, trace_t* results,
                     const math::Position3* start, const math::Position3* mins,
                     const math::Position3* maxs, const math::Position3* end,
                     const collision_context_t* context) {
    if (IS_NAN(start->v.m128_f32[0])
        || IS_NAN(start->v.m128_f32[1])
        || IS_NAN(start->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 863;
        AeAssert::gCurrentExpr = "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end->v.m128_f32[0])
        || IS_NAN(end->v.m128_f32[1])
        || IS_NAN(end->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 864;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(mins->v.m128_f32[0])
        || IS_NAN(mins->v.m128_f32[1])
        || IS_NAN(mins->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 865;
        AeAssert::gCurrentExpr = "!IS_NAN((mins)[0]) && !IS_NAN((mins)[1]) && !IS_NAN((mins)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(maxs->v.m128_f32[0])
        || IS_NAN(maxs->v.m128_f32[1])
        || IS_NAN(maxs->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 866;
        AeAssert::gCurrentExpr = "!IS_NAN((maxs)[0]) && !IS_NAN((maxs)[1]) && !IS_NAN((maxs)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    TraceSphere(proximity_data, &tr, start, end, mins, maxs, context->contentmask);
    if (tr.fraction == 1.0f) {
        tr.mEntity.mHandle.mVal = 0;
    } else {
        tr.mEntity.mHandle.mVal = EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
    }
    if (tr.fraction == 0.0f) {
        *results = tr;
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 878;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 879;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 880;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    } else {
        moveclip_t clip;
        memset(&clip, 0, sizeof(clip));
        clip.trace = tr;
            clip.mPassEntity.mHandle.mVal = context->pass_entity1.mHandle.mVal;
        clip.contentmask = context->contentmask;
        clip.capsule = 1;
        if (clip.mPassEntity.IsValid()) {
            Entity* e1 = HandleDbDeref(context->pass_entity1);
            if (e1 != NULL)
                clip.mPassOwner.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
        }
        clip.mins.v = _mm_mul_ps(_mm_sub_ps(maxs->v, mins->v), _mm_set1_ps(0.5f));
        clip.maxs.v = _mm_add_ps(maxs->v, mins->v);
        clip.start.v = _mm_add_ps(start->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
        clip.end.v = _mm_add_ps(end->v, _mm_mul_ps(clip.maxs.v, _mm_set1_ps(0.5f)));
        clip.outerSize.v = _mm_xor_ps(_mm_set1_ps(-0.0f), clip.mins.v);
        CM_ClipMoveToEntities(&clip, context);
        if ((unsigned int)clip.trace.partGroup > 0x12u) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 906;
            AeAssert::gCurrentExpr = "( clip.trace.partGroup >= HITLOC_NONE ) && ( clip.trace.partGroup < HITLOC_NUM )";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (clip.trace.fraction > tr.fraction)
                clip.trace.endpos.v = _mm_add_ps(start->v,
                                                 _mm_mul_ps(_mm_sub_ps(end->v, start->v),
                                                            _mm_set1_ps(tr.fraction)));
        *results = clip.trace;
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 916;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 917;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 918;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
}

// ============================================================================
// TracePointFull Ã¢â‚¬â€ ea: 0x5240E0
// ============================================================================
void TracePointFull(const proximity_data_t* proximity_data, trace_t* results,
                    const math::Position3* start, const math::Position3* end,
                    const collision_context_t* context) {
    if (IS_NAN(start->v.m128_f32[0])
        || IS_NAN(start->v.m128_f32[1])
        || IS_NAN(start->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 931;
        AeAssert::gCurrentExpr = "!IS_NAN((start)[0]) && !IS_NAN((start)[1]) && !IS_NAN((start)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    if (IS_NAN(end->v.m128_f32[0])
        || IS_NAN(end->v.m128_f32[1])
        || IS_NAN(end->v.m128_f32[2])) {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
        AeAssert::gCurrentLine = 932;
        AeAssert::gCurrentExpr = "!IS_NAN((end)[0]) && !IS_NAN((end)[1]) && !IS_NAN((end)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    trace_t tr;
    memset(&tr, 0, sizeof(tr));
    TracePoint(proximity_data, &tr, start, end, context->contentmask);
    if (tr.fraction == 1.0f) {
        tr.mEntity.mHandle.mVal = 0;
    } else {
        tr.mEntity.mHandle.mVal = EntityManager::sInst->mWorld->mHandle.mHandle.mVal;
    }
    if (tr.fraction == 0.0f) {
        *results = tr;
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 944;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 945;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 946;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    } else {
        pointtrace_t clip;
        memset(&clip, 0, sizeof(clip));
        clip.start = *start;
        clip.end = *end;
        clip.trace = tr;
            clip.mPassEntity.mHandle.mVal = context->pass_entity1.mHandle.mVal;
        clip.contentmask = context->contentmask;
        clip.bLocational = 1;
        if (clip.mPassEntity.IsValid()) {
            Entity* e1 = HandleDbDeref(context->pass_entity1);
            if (e1 != NULL)
                clip.mPassOwner.mHandle.mVal = e1->r.mOwner.mHandle.mVal;
        }
        CM_PointTraceToEntities(&clip, context);
        *results = clip.trace;
        if (IS_NAN(results->fraction)) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 967;
            AeAssert::gCurrentExpr = "!IS_NAN(results->fraction)";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid number!"))
                __debugbreak();
        }
        if (IS_NAN(results->normal.v.m128_f32[0])
            || IS_NAN(results->normal.v.m128_f32[1])
            || IS_NAN(results->normal.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 968;
            AeAssert::gCurrentExpr = "!IS_NAN((results->normal)[0]) && !IS_NAN((results->normal)[1]) && !IS_NAN((results->normal)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        if (IS_NAN(results->endpos.v.m128_f32[0])
            || IS_NAN(results->endpos.v.m128_f32[1])
            || IS_NAN(results->endpos.v.m128_f32[2])) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_world.cpp";
            AeAssert::gCurrentLine = 969;
            AeAssert::gCurrentExpr = "!IS_NAN((results->endpos)[0]) && !IS_NAN((results->endpos)[1]) && !IS_NAN((results->endpos)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
    }
}

// ============================================================================
// SV_SetBrushModel Ã¢â‚¬â€ ea: 0x524DA0
// ============================================================================
void SV_SetBrushModel(DCGSet* ent) {
    if (ent[5].brush_verts_m_count == 0) {
        unsigned short m_count_high = (unsigned short)(ent->objects_m_count >> 16);
        int h;
        if (m_count_high != 0) {
            h = m_count_high;
        } else {
            ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
            if ((int)ent[5].max.v.m128_f32[0] == 0)
                return;
            ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
            if (*(int*)((int)ent[5].max.v.m128_f32[0] + 72) == 0)
                return;
            ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
            h = (int)HashString_CalcHash(*(const char**)((int)ent[5].max.v.m128_f32[0] + 72));
        }
        if (h == 0) {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\sv_game.cpp";
            AeAssert::gCurrentLine = 84;
            AeAssert::gCurrentExpr = "h";
            const char* v6;
            Broc::string::Block* v4 = *(Broc::string::Block**)((int)ent[5].center.v.m128_f32[1]);
            if (v4 != NULL && v4->mLength != 0)
                v6 = (const char*)(v4 + 1);
            else
                v6 = "Unknown";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Found script brush model without a collidable brush targetname %s.", v6))
                __debugbreak();
        }
        TPakId v7 = (TPakId)(int)ent[5].nboxes;
        if (v7 == PAK_ID_INVALID)
            v7 = CurPakId();
        DCGSet* v8 = ClipHandleToDCGSet(v7, h);
        DCGSet* mod = v8;
        if (v8 == NULL) {
            ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
            if ((int)ent[5].max.v.m128_f32[0] != 0) {
                ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
                const char* name = *(const char**)((int)ent[5].max.v.m128_f32[0] + 72);
                if (strlen(name) > 7) {
                    ValidatePakId((TPakId)(int)ent[5].max.v.m128_f32[1]);
                    int v9 = atoi(name + 7);
                    TPakId v10 = (TPakId)(int)ent[5].nboxes;
                    if (v10 == PAK_ID_INVALID)
                        v10 = CurPakId();
                    mod = ClipHandleToDCGSet(v10, v9);
                    v8 = mod;
                }
            }
        }
        if (ent[5].brush_verts_m_count == 0) {
            math::Position3 mins;
            math::Position3 maxs;
            CM_ModelBounds(mod, mins, maxs);
            ent[2].brushes_m_elements = (int)mins.v.m128_f32[0];
            ent[2].gjk_brushes_m_count = (int)mins.v.m128_f32[0];
            ent[2].gjk_brushes_m_elements = (int)mins.v.m128_f32[0];
            ent[2].brush_sides_m_elements = (int)maxs.v.m128_f32[0];
            ent[2].brush_verts_m_count = (int)maxs.v.m128_f32[0];
            ent[2].brush_verts_m_elements = (int)maxs.v.m128_f32[0];
            if (h == 0) {
                ent[2].brushes_m_elements = -1063256064;
                ent[2].gjk_brushes_m_count = -1063256064;
                ent[2].gjk_brushes_m_elements = -1063256064;
                ent[2].brush_sides_m_elements = 1084227584;
                ent[2].brush_verts_m_count = 1084227584;
                ent[2].brush_verts_m_elements = 1084227584;
            }
            v8 = mod;
        }
        if (((int)ent[5].min.v.m128_f32[0] == 0 && (loc_800000 & ent[6].brush_verts_m_count) == 0)
            || ent[2].brushes_m_count == 0) {
            ent[2].brushes_m_count = (int)v8;
            int contents;
            if (v8 != NULL)
                contents = DCGSet_get_contents(v8);
            else
                contents = 0;
            ent[2].radius2 = (float)contents;
            SV_LinkEntity((Entity*)ent);
        }
    }
}

// ============================================================================
// HandleDb entity lookup â€” returns the Entity* for a handle (or NULL)
// ============================================================================
static Entity* SV_GetEntityFromHandle(unsigned int mVal) {
    unsigned int v5 = mVal & 0xFFF;
    if (v5 < 0x540 && (unsigned int)(mVal >> 12) == EntityHandleDb::sInst.mElements[v5].mKey)
        return EntityHandleDb::sInst.mElements[v5].mObject;
    return NULL;
}

// ============================================================================
// SV_ClipMoveToEntity â€” ea: 0x521C40
// ============================================================================
void SV_ClipMoveToEntity(moveclip_t* clip, EntityShared* check) {
    const Entity* p_currentOrigin = (const Entity*)((char*)check - 0xE0);  // Entity - 1 cast; see decompile
    trace_t localTrace;
    memset(&localTrace, 0, sizeof(localTrace));
    localTrace.fraction = clip->trace.fraction;

    if ((clip->contentmask & check->contents) != 0) {
        unsigned int mVal = clip->mPassEntity.mHandle.mVal;
        Entity* passEnt = SV_GetEntityFromHandle(mVal);
        if (passEnt == NULL
            || p_currentOrigin->mHandle.mHandle.mVal != mVal
                && (unsigned int)(p_currentOrigin->r.mOwner.mHandle.mVal) != mVal
                && (p_currentOrigin->r.mOwner.mHandle.mVal != clip->mPassOwner.mHandle.mVal
                    || SV_GetEntityFromHandle(clip->mPassOwner.mHandle.mVal) == NULL)) {
            DCGSet* v7 = SV_ClipHandleForEntity(p_currentOrigin);
            math::Position3 angles;
            angles.v = p_currentOrigin->r.currentAngles.v;
            if ((loc_800000 & p_currentOrigin->r.contents) != 0) {
                if (threshold > fabsf(angles.v.m128_f32[2]))
                    angles.v.m128_f32[2] = 0;
                if (threshold > fabsf(angles.v.m128_f32[0]))
                    angles.v.m128_f32[0] = 0;
            }
            if (p_currentOrigin->r.bmodel == NULL)
                angles.v = Float4_Zero_2.v;
            int capsule = clip->capsule;
            int contentmask = clip->contentmask;
            localTrace.fraction = clip->trace.fraction;
            TraceXFormed(&localTrace, &clip->start, &clip->end, &clip->mins, &clip->maxs,
                         v7, contentmask, &p_currentOrigin->r.currentOrigin, &angles, capsule);
            if (localTrace.fraction < clip->trace.fraction) {
                clip->trace.allsolid |= localTrace.allsolid;
                localTrace.mEntity.mHandle.mVal = p_currentOrigin->mHandle.mHandle.mVal;
                clip->trace = localTrace;
            } else {
                unsigned char startsolid = clip->trace.startsolid;
                clip->trace.allsolid |= localTrace.allsolid;
                clip->trace.startsolid = (unsigned char)(localTrace.startsolid | startsolid);
            }
        }
    }
}

// ============================================================================
// SV_ClipSightToEntity â€” ea: 0x522450
// ============================================================================
int SV_ClipSightToEntity(sightclip_t* clip, EntityShared* check) {
    const Entity* p_currentOrigin = (const Entity*)((char*)check - 0xE0);
    if ((check->contents & clip->contentmask) == 0)
        return 0;

    unsigned int mVal = clip->mPassEntity1.mHandle.mVal;
    Entity* e1 = SV_GetEntityFromHandle(mVal);
    if (e1 != NULL) {
        if (p_currentOrigin->mHandle.mHandle.mVal == mVal)
            return 0;
        unsigned int v7 = p_currentOrigin->r.mOwner.mHandle.mVal;
        if (v7 == mVal
            || (v7 == clip->mPassOwner1.mHandle.mVal
                && SV_GetEntityFromHandle(clip->mPassOwner1.mHandle.mVal) != NULL))
            return 0;
    }
    unsigned int v8 = clip->mPassEntity2.mHandle.mVal;
    Entity* e2 = SV_GetEntityFromHandle(v8);
    if (e2 != NULL) {
        if (p_currentOrigin->mHandle.mHandle.mVal == v8)
            return 0;
        unsigned int v10 = p_currentOrigin->r.mOwner.mHandle.mVal;
        if (v10 == v8
            || (v10 == clip->mPassOwner2.mHandle.mVal
                && SV_GetEntityFromHandle(clip->mPassOwner2.mHandle.mVal) != NULL))
            return 0;
    }

    DCGSet* v11 = SV_ClipHandleForEntity(p_currentOrigin);
    __m128 v15;
    const __m128* p_v;
    if (p_currentOrigin->r.bmodel != NULL)
        p_v = &p_currentOrigin->r.currentAngles.v;
    else {
        v15 = Float4_Zero_2.v;
        p_v = &v15;
    }
    math::Position3 v16;
    v16.v = *p_v;
    int capsule = clip->capsule;
    int contentmask = clip->contentmask;
    return -(SightTraceXFormed(0, &clip->start, &clip->end, &clip->mins, &clip->maxs,
                               v11, contentmask, &p_currentOrigin->r.currentOrigin, &v16, capsule) != 0);
}

// ============================================================================
// SV_PointSightTraceToEntity â€” ea: 0x5225B0
// ============================================================================
int SV_PointSightTraceToEntity(sightpointtrace_t* clip, EntityShared* check) {
    const Entity* p_currentOrigin = (const Entity*)((char*)check - 0xE0);
    if ((check->contents & clip->contentmask) == 0)
        return 0;

    unsigned int mVal = clip->mPassEntity1.mHandle.mVal;
    Entity* e1 = SV_GetEntityFromHandle(mVal);
    if (e1 != NULL) {
        if (p_currentOrigin->mHandle.mHandle.mVal == mVal)
            return 0;
        unsigned int v7 = p_currentOrigin->r.mOwner.mHandle.mVal;
        if (v7 == mVal
            || (v7 == clip->mPassOwner1.mHandle.mVal
                && SV_GetEntityFromHandle(clip->mPassOwner1.mHandle.mVal) != NULL))
            return 0;
    }
    unsigned int v8 = clip->mPassEntity2.mHandle.mVal;
    Entity* e2 = SV_GetEntityFromHandle(v8);
    if (e2 != NULL) {
        if (p_currentOrigin->mHandle.mHandle.mVal == v8)
            return 0;
        unsigned int v10 = p_currentOrigin->r.mOwner.mHandle.mVal;
        if (v10 == v8
            || (v10 == clip->mPassOwner2.mHandle.mVal
                && SV_GetEntityFromHandle(clip->mPassOwner2.mHandle.mVal) != NULL))
            return 0;
    }

    DCGSet* v11 = SV_ClipHandleForEntity(p_currentOrigin);
    math::Position3 v15;
    math::Position3* p_currentAngles;
    if (p_currentOrigin->r.bmodel != NULL)
        p_currentAngles = (math::Position3*)&p_currentOrigin->r.currentAngles;
    else {
        v15.v = Float4_Zero_2.v;
        p_currentAngles = &v15;
    }
    math::Position3 v16;
    v16.v = p_currentAngles->v;
    int contentmask = clip->contentmask;
    math::Position3 v14;
    v14.v = Float4_Zero_2.v;
    v15.v = Float4_Zero_2.v;
    return -(SightTraceXFormed(0, &clip->start, &clip->end, &v14, &v15,
                               v11, contentmask, &p_currentOrigin->r.currentOrigin, &v16, 0) != 0);
}

// ============================================================================
// cg_dobj.cpp - DObj pose/tag helpers (cg.o cg_ent.cpp / cg_main.cpp)
// ============================================================================

#include "game/cg/cg_local.h"
#include "game/game_types.h"
#include "game/core/core_types.h"

#include <string.h>

extern int currCl;
extern int dword_F63554[4 * 1580];
extern float dword_F64074[4 * 1580];
extern float dword_F64078[4 * 1580];
extern float dword_F6407C[4 * 1580];
extern float unk_F64080[4 * 6320];
extern void j_nullsub_89(DObj* obj, float dtime);
extern void j_nullsub_30(DObj* obj, int* partBits);
extern int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash);
extern void DObjGetHierarchyBits(DObj* obj, int boneIndex, int* partBits);
extern DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex);
extern int CL_DObjCreateSkelForBone(DObj* obj, int boneIndex);
extern int CL_DObjCreateSkelForBones(DObj* obj, int* partBits);
extern void CL_DObjCalcSkel(DObj* obj, int* partBits);
extern void AxisCopy(const float (*in)[3], float (*out)[3]);
extern void DObjSkel2MatrixMultiply43(const DObjSkelMat* in1,
                                      const float (*in2)[3], DObjSkelMat* out);
extern void AnglesToAxis(const math::Position3* angles, float (*axis)[3]);
extern void AnglesToAxis(const float* angles, float (*axis)[3]);
extern void* G_GetVehicleInfo(Entity* ent);
extern void G_CalcTagParentAxis(Entity* ent, float (*parentAxis)[3]);
extern bool IsPlayerFullySeatedInVehicle(Entity* player);

EntityHandleDb EntityHandleDb::sInst;

// ea: 0x0068A3A0
void CG_DObjUpdateInfo(DObj* obj)
{
    j_nullsub_89(obj, dword_F63554[1580 * currCl] * 0.001f);
}

// ea: 0x0068A3E0
int CG_DObjGetViewModelTagMatrix(DObj* obj, unsigned int tag_name_hash,
                                 DObjSkelMat* tagMat)
{
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 937);
    }
    int BoneIndex = DObjGetBoneIndex(obj, tag_name_hash);
    int v4 = BoneIndex;
    if (BoneIndex < 0)
        return 0;
    if (CL_DObjCreateSkelForBone(obj, BoneIndex) == 0)
    {
        int partBits[4];
        DObjGetHierarchyBits(obj, v4, partBits);
        j_nullsub_30(obj, partBits);
        CL_DObjCalcSkel(obj, partBits);
    }
    DObjSkelMat* MatrixArray = DObjGetMatrixArray(obj, 0);
    if (MatrixArray == nullptr)
    {
        CG_ASSERT("mat", "c:\\cod\\code\\game\\cg_ent.cpp", 953);
    }
    float ent_axis[12];
    AxisCopy((const float (*)[3])&unk_F64080[6320 * currCl],
             (float (*)[3])ent_axis);
    ent_axis[9] = dword_F64074[1580 * currCl];
    ent_axis[10] = dword_F64078[1580 * currCl];
    ent_axis[11] = dword_F6407C[1580 * currCl];
    DObjSkel2MatrixMultiply43(&MatrixArray[v4],
                              (const float (*)[3])ent_axis, tagMat);
    return 1;
}

// ea: 0x0068A640
void CG_DObjCalcPose(Entity* entity, DObj* obj, int* partBits)
{
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 1281);
    }
    if (obj != (DObj*)entity->mDObj)
    {
        CG_ASSERT("obj == entity->GetDObj()",
                  "c:\\cod\\code\\game\\cg_ent.cpp", 1282);
    }
    if (CL_DObjCreateSkelForBones(obj, partBits) == 0)
    {
        j_nullsub_30(obj, partBits);
        CL_DObjCalcSkel(obj, partBits);
    }
}

// ea: 0x0068A700
void CG_DObjCalcBone(Entity* entity, DObj* obj, int boneIndex)
{
    int partBits[4];
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 1300);
    }
    if (obj != (DObj*)entity->mDObj)
    {
        CG_ASSERT("obj == entity->GetDObj()",
                  "c:\\cod\\code\\game\\cg_ent.cpp", 1301);
    }
    if (CL_DObjCreateSkelForBone(obj, boneIndex) == 0)
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        j_nullsub_30(obj, partBits);
        CL_DObjCalcSkel(obj, partBits);
    }
}

// ea: 0x0068A7E0
void CG_DObjCalcBoneGeneric(DObj* obj, int boneIndex)
{
    int partBits[4];
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 1319);
    }
    if (CL_DObjCreateSkelForBone(obj, boneIndex) == 0)
    {
        DObjGetHierarchyBits(obj, boneIndex, partBits);
        j_nullsub_30(obj, partBits);
        CL_DObjCalcSkel(obj, partBits);
    }
}

// ea: 0x00696E10
const DObjSkelMat* CG_DObjGetLocalTagMatrix(Entity* entity, DObj* obj,
                                            unsigned int tag_name_hash)
{
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 899);
    }
    if (obj != (DObj*)entity->mDObj)
    {
        CG_ASSERT("obj ==entity->GetDObj()",
                  "c:\\cod\\code\\game\\cg_ent.cpp", 900);
    }
    int BoneIndex = DObjGetBoneIndex(obj, tag_name_hash);
    int v4 = BoneIndex;
    if (BoneIndex < 0)
        return nullptr;
    CG_DObjCalcBone(entity, obj, BoneIndex);
    DObjSkelMat* MatrixArray = DObjGetMatrixArray(obj, 0);
    if (MatrixArray == nullptr)
    {
        CG_ASSERT("mat", "c:\\cod\\code\\game\\cg_ent.cpp", 911);
    }
    return &MatrixArray[v4];
}

// ea: 0x00696F30
const DObjSkelMat* CG_DObjGetWorldTagMatrix(Entity* entity, DObj* obj,
                                            unsigned int tag_name_hash,
                                            DObjSkelMat* tagMat)
{
    if (obj == nullptr)
    {
        CG_ASSERT("obj", "c:\\cod\\code\\game\\cg_ent.cpp", 919);
    }
    if (obj != (DObj*)entity->mDObj)
    {
        CG_ASSERT("obj == entity->GetDObj()",
                  "c:\\cod\\code\\game\\cg_ent.cpp", 920);
    }
    const DObjSkelMat* result =
        CG_DObjGetLocalTagMatrix(entity, obj, tag_name_hash);
    const DObjSkelMat* v6 = result;
    if (result != nullptr)
    {
        float ent_axis[4][3];
        float v16[3][3];
        ent_axis[3][0] = entity->s.lerpAngles.v.m128_f32[0];
        ent_axis[3][1] = entity->s.lerpAngles.v.m128_f32[1];
        ent_axis[3][2] = entity->s.lerpAngles.v.m128_f32[2];
        AnglesToAxis(ent_axis[3], v16);
        ent_axis[3][0] = entity->s.lerpOrigin.v.m128_f32[0];
        ent_axis[3][1] = entity->s.lerpOrigin.v.m128_f32[1];
        ent_axis[3][2] = entity->s.lerpOrigin.v.m128_f32[2];
        DObjSkel2MatrixMultiply43(v6, v16, tagMat);
        return (const DObjSkelMat*)1;
    }
    return result;
}

// ea: 0x006A1D40
void CG_GetDObjOrientation(DObj* dobj, float* origin_out,
                           float (*axis_out)[3])
{
    if (origin_out == nullptr)
    {
        CG_ASSERT("origin_out", "c:\\cod\\code\\game\\cg_main.cpp", 641);
    }
    if (axis_out == nullptr)
    {
        CG_ASSERT("axis_out", "c:\\cod\\code\\game\\cg_main.cpp", 642);
    }
    if (dobj == nullptr)
    {
        CG_ASSERT("dobj", "c:\\cod\\code\\game\\cg_main.cpp", 643);
    }
    Entity* mEntity = dobj->mEntity;
    if (mEntity != nullptr)
    {
        if (mEntity->client != nullptr)
        {
            if ((mEntity->s.eFlags & 0x100000) == 0)
                goto LABEL_25;
            unsigned int own = mEntity->r.mOwner.mHandle.mVal;
            Entity* v5 = nullptr;
            if ((own & 0xFFF) < 0x540
                && own >> 12 == EntityHandleDb::sInst.mElements[own & 0xFFF].mKey)
                v5 = EntityHandleDb::sInst.mElements[own & 0xFFF].mObject;
            if (v5 == nullptr || v5->scr_vehicle == nullptr)
            {
                CG_ASSERT("vehicle && vehicle->scr_vehicle",
                          "c:\\cod\\code\\game\\cg_main.cpp", 655);
            }
            G_GetVehicleInfo(v5);
            if (mEntity->tagInfo != nullptr
                && IsPlayerFullySeatedInVehicle(mEntity))
            {
                origin_out[0] =
                    mEntity->r.currentOrigin.v.m128_f32[0];
                origin_out[1] =
                    mEntity->r.currentOrigin.v.m128_f32[1];
                origin_out[2] =
                    mEntity->r.currentOrigin.v.m128_f32[2];
                float parentAxis[4][3];
                G_CalcTagParentAxis(mEntity, parentAxis);
                (*axis_out)[0] = parentAxis[0][0];
                (*axis_out)[1] = parentAxis[0][1];
                (*axis_out)[2] = parentAxis[0][2];
                memcpy(&(*axis_out)[3], parentAxis, 24);
            }
            else
            {
            LABEL_25:
                origin_out[0] =
                    mEntity->r.currentOrigin.v.m128_f32[0];
                origin_out[1] =
                    mEntity->r.currentOrigin.v.m128_f32[1];
                origin_out[2] =
                    mEntity->r.currentOrigin.v.m128_f32[2];
                AnglesToAxis(&mEntity->r.currentAngles, axis_out);
            }
        }
        else
        {
            origin_out[0] = mEntity->s.lerpOrigin.v.m128_f32[0];
            origin_out[1] = mEntity->s.lerpOrigin.v.m128_f32[1];
            origin_out[2] = mEntity->s.lerpOrigin.v.m128_f32[2];
            AnglesToAxis(&mEntity->s.lerpAngles, axis_out);
        }
    }
    else
    {
        origin_out[0] = dword_F64074[1580 * currCl];
        origin_out[1] = dword_F64078[1580 * currCl];
        origin_out[2] = dword_F6407C[1580 * currCl];
        AxisCopy((const float (*)[3])&unk_F64080[6320 * currCl], axis_out);
    }
}

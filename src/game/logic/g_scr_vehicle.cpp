// ============================================================================
// g_scr_vehicle.cpp - scripted vehicle stubs (g.o: g_scr_vehicle.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

// ea: 0x00452BC0
void VehicleNodeAllocator::Initialize()
{
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x00452C10
vehicle_node_t* VehicleNodeAllocator::AllocNode()
{
    int16_t m_numBlocks = this->m_numBlocks;
    if (m_numBlocks == 0 || this->m_currentBlockIndex >= 128)
    {
        if (m_numBlocks >= 16)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_vehicle_path.cpp";
            AeAssert::gCurrentLine = 1605;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Out of vehicle Nodes - Tell MikeA"))
                __debugbreak();
        }
        this->m_pNodeBlocks[this->m_numBlocks] = mem_heap_malloc(16, 0x2000u);
        vehicle_node_t* v3 = (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks];
        memset(v3, 0, 0x2000);
        for (int i = 128; i != 0; --i)
        {
            if (v3 != nullptr)
            {
                v3->mName = Broc::string();
                v3->mTarget = Broc::string();
                v3->script_noteworthy = Broc::string();
            }
            ++v3;
        }
        ++this->m_numBlocks;
        this->m_currentBlockIndex = 0;
    }
    uint16_t m_currentBlockIndex = this->m_currentBlockIndex;
    vehicle_node_t* result =
        (vehicle_node_t*)this->m_pNodeBlocks[this->m_numBlocks] + m_currentBlockIndex;
    this->m_currentBlockIndex = m_currentBlockIndex + 1;
    s_nodes[this->m_numNodes++] = result;
    return result;
}

// ea: 0x0045F2B0
void VehicleNodeAllocator::FreeAll()
{
    if (this->m_numBlocks > 0)
    {
        for (int i = 0; i < this->m_numBlocks; ++i)
        {
            vehicle_node_t* v3 = (vehicle_node_t*)m_pNodeBlocks[i];
            for (int j = 128; j != 0; --j)
                v3++->~vehicle_node_t();
            mem_heap_free(m_pNodeBlocks[i]);
        }
    }
    m_numNodes = 0;
    m_numBlocks = 0;
    m_currentBlockIndex = 0;
    for (int i = 0; i < 16; ++i)
        m_pNodeBlocks[i] = nullptr;
}

// ea: 0x0044D8C0
void VEH_SetupCollmap(Entity* ent)
{
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
}

// ea: 0x0044F3A0
float VEH_GetMaxSpeed(scr_vehicle_t* veh)
{
    rb_vehicle* mRBVeh = (rb_vehicle*)veh->mRBVeh;
    if (mRBVeh != nullptr)
        return *(float*)(*(void**)((char*)mRBVeh + 0x250));  // m_parameter->m_speed_max
    return s_vehicleInfos[veh->infoIdx]->maxSpeed;
}

// ea: 0x0044F520
int scr_vehicle_t::GetStageAnim(Client* client)
{
    return animMap->stages[animMap->routes[client->mVehicleAnimRoute].stages[client->mVehicleAnimStage]].animRow;
}

// ea: 0x00452840
void G_VehFreePathPos(vehicle_pathpos_t* vpp)
{
    vpp->switchNode[0].mName.clear();
    vpp->switchNode[0].mTarget.clear();
    vpp->switchNode[1].mName.clear();
    vpp->switchNode[1].mTarget.clear();
}

// ea: 0x00451950
int16_t VP_GetNodeIndex(const Broc::string* name, float* origin)
{
    if (name->mBlock == nullptr)
        return -1;
    const char* v3 = (const char*)&name->mBlock[1];
    const char* v8 = v3;
    if (name->mBlock == (Broc::string::Block*)-12 || *v3 == 0)
        return -1;
    int16_t i = 0;
    if (s_numNodes <= 0)
        return -1;
    int v5 = 0;
    while (1)
    {
        vehicle_node_t* v6 = s_nodes[v5];
        const char* v7 = v6->mName.mBlock != nullptr
                             ? (const char*)&v6->mName.mBlock[1]
                             : defaultFileName;
        if (strcmp(v7, v3) == 0
            && (origin == nullptr
                || (v6->origin[0] == origin[0] && v6->origin[1] == origin[1]
                    && v6->origin[2] == origin[2])))
            break;
        v5 = ++i;
        if (i >= s_numNodes)
            return -1;
        v3 = v8;
    }
    return i;
}

// ea: 0x00451A50
float VP_CalcNodeSpeed(int16_t nodeIdx)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->speed >= 0.0f)
        return v2->speed;
    float v3 = -1.0f;
    int v4 = (16 * v2->nextIdx) >> 18;
    float v5 = 0.0f;
    float v6 = 0.0f;
    float speed = -1.0f;
    if (v4 >= 0)
    {
        vehicle_node_t* v8 = s_nodes[v4];
        int16_t v9 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                v6 = v8->length + v6;
                ++v9;
                if (v8->speed >= 0.0f)
                    break;
                int v10 = (16 * v8->nextIdx) >> 18;
                if (v10 >= 0 && v10 != nodeIdx)
                {
                    v8 = s_nodes[v10];
                    if (v9 < s_numNodes)
                        continue;
                }
                goto label_11;
            }
            speed = v8->speed;
        }
    }
label_11:
    int16_t v11 = 0;
    if (s_numNodes > 0)
    {
        while (1)
        {
            ++v11;
            if (v2->speed >= 0.0f)
                break;
            int v12 = (v2->nextIdx << 18) >> 18;
            if (v12 >= 0 && v12 != nodeIdx)
            {
                float length = v2->length;
                v2 = s_nodes[v12];
                v5 = length + v5;
                if (v11 < s_numNodes)
                    continue;
            }
            goto label_18;
        }
        v3 = v2->speed;
    }
label_18:
    if (speed >= 0.0f)
    {
        if (v3 >= 0.0f)
        {
            float v14 = v5 + v6;
            if (v14 > 0.0f)
                return ((v6 / v14) * (v3 - speed)) + speed;
        }
        else
        {
            return speed;
        }
    }
    else if (v3 >= 0.0f)
    {
        return v3;
    }
    return 0.0f;
}

// ea: 0x00451B60
float VP_CalcNodeLookAhead(int16_t nodeIdx)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->lookAhead >= 0.0f)
        return v2->lookAhead;
    float v3 = -1.0f;
    int v4 = (16 * v2->nextIdx) >> 18;
    float v5 = 0.0f;
    float v6 = 0.0f;
    float lookAhead = -1.0f;
    if (v4 >= 0)
    {
        vehicle_node_t* v8 = s_nodes[v4];
        int16_t v9 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                v6 = v8->length + v6;
                ++v9;
                if (v8->lookAhead > 0.0f)
                    break;
                int v10 = (16 * v8->nextIdx) >> 18;
                if (v10 >= 0 && v10 != nodeIdx)
                {
                    v8 = s_nodes[v10];
                    if (v9 < s_numNodes)
                        continue;
                }
                goto label_11;
            }
            lookAhead = v8->lookAhead;
        }
    }
label_11:
    int16_t v11 = 0;
    if (s_numNodes > 0)
    {
        while (1)
        {
            ++v11;
            if (v2->lookAhead > 0.0f)
                break;
            int v12 = (v2->nextIdx << 18) >> 18;
            if (v12 >= 0 && v12 != nodeIdx)
            {
                float length = v2->length;
                v2 = s_nodes[v12];
                v5 = length + v5;
                if (v11 < s_numNodes)
                    continue;
            }
            goto label_18;
        }
        v3 = v2->lookAhead;
    }
label_18:
    if (lookAhead >= 0.0f)
    {
        if (v3 >= 0.0f)
        {
            float v14 = v5 + v6;
            if (v14 > 0.0f)
                return ((v6 / v14) * (v3 - lookAhead)) + lookAhead;
        }
        else
        {
            return lookAhead;
        }
    }
    else if (v3 >= 0.0f)
    {
        return v3;
    }
    return 0.0f;
}

// ea: 0x00451C70
void VP_CalcNodeAngles(int16_t nodeIdx, float* angles)
{
    vehicle_node_t* v2 = s_nodes[nodeIdx];
    if (v2->angles[0] == s_invalidAngles[0]
        && v2->angles[1] == dword_DD7418 && v2->angles[2] == dword_DD741C)
    {
        int v3 = (16 * v2->nextIdx) >> 18;
        float v4 = dword_DD741C;
        float v5 = s_invalidAngles[0];
        float v6 = 0.0f;
        float prevDist = 0.0f;
        float prevAngles = s_invalidAngles[0];
        float v16 = dword_DD7418;
        float v17 = dword_DD741C;
        float nextAngles = s_invalidAngles[0];
        float v19 = dword_DD7418;
        float v20 = dword_DD741C;
        if (v3 >= 0)
        {
            vehicle_node_t* v7 = s_nodes[v3];
            int16_t v8 = 0;
            if (s_numNodes > 0)
            {
                float v9 = 0.0f;
                while (1)
                {
                    ++v8;
                    v9 = v7->length + v9;
                    if (v7->angles[0] != s_invalidAngles[0]
                        || v7->angles[1] != dword_DD7418
                        || v7->angles[2] != dword_DD741C)
                        break;
                    int v10 = (16 * v7->nextIdx) >> 18;
                    if (v10 >= 0 && v10 != nodeIdx)
                    {
                        v7 = s_nodes[v10];
                        if (v8 < s_numNodes)
                            continue;
                    }
                    prevDist = v9;
                    v4 = dword_DD741C;
                    goto label_14;
                }
                prevDist = v9;
                prevAngles = v7->angles[0];
                v16 = v7->angles[1];
                v4 = v7->angles[2];
                v17 = v4;
            }
        }
label_14:
        int16_t v11 = 0;
        if (s_numNodes > 0)
        {
            while (1)
            {
                ++v11;
                if (v2->angles[0] != s_invalidAngles[0]
                    || v2->angles[1] != dword_DD7418
                    || v2->angles[2] != dword_DD741C)
                    break;
                int v12 = (v2->nextIdx << 18) >> 18;
                if (v12 >= 0 && v12 != nodeIdx)
                {
                    float length = v2->length;
                    v2 = s_nodes[v12];
                    v6 = length + v6;
                    if (v11 < s_numNodes)
                        continue;
                }
                goto label_24;
            }
            v5 = v2->angles[0];
            v19 = v2->angles[1];
            nextAngles = v5;
            v20 = v2->angles[2];
        }
label_24:
        if (prevAngles == s_invalidAngles[0] && v16 == dword_DD7418)
        {
            if (v4 == dword_DD741C && v5 == s_invalidAngles[0]
                && v19 == dword_DD7418 && v20 == dword_DD741C)
            {
                angles[0] = 0.0f;
                angles[1] = 0.0f;
                angles[2] = 0.0f;
                return;
            }
            if (v4 == dword_DD741C)
            {
                angles[0] = v5;
                angles[1] = v19;
                angles[2] = v20;
                return;
            }
        }
        if (v5 == s_invalidAngles[0] && v19 == dword_DD7418 && v20 == dword_DD741C)
        {
            angles[0] = prevAngles;
            angles[1] = v16;
            angles[2] = v4;
            return;
        }
        float v14 = v6 + prevDist;
        if (v14 <= 0.0f)
        {
            angles[0] = 0.0f;
            angles[1] = 0.0f;
            angles[2] = 0.0f;
            return;
        }
        float totalDist = prevDist / v14;
        angles[0] = LerpAngle(prevAngles, nextAngles, totalDist);
        angles[1] = LerpAngle(v16, v19, totalDist);
        angles[2] = LerpAngle(v17, v20, totalDist);
    }
    else
    {
        angles[0] = v2->angles[0];
        angles[1] = v2->angles[1];
        angles[2] = v2->angles[2];
    }
}

// ea: 0x00452440
void G_SetupVehiclePaths(float v)
{
    for (int16_t i = 0; i < s_numNodes;)
    {
        int v2 = i;
        vehicle_node_t* node = s_nodes[v2];
        Broc::string* p_mName = &node->mName;
        Broc::string::Block* mBlock = p_mName[1].mBlock;
        if (mBlock != nullptr)
        {
            Broc::string::Block* v5 = mBlock + 1;
            if (v5 != nullptr && ((char*)&v5->mBuff)[0] != 0)
                node->nextIdx = (node->nextIdx ^ (node->nextIdx ^ VP_GetNodeIndex(p_mName + 1, nullptr)) & 0x3FFF);
        }
        int16_t v6 = 0;
        if (s_numNodes > 0)
        {
            while (i == v6 || !(*p_mName == s_nodes[v6]->mTarget))
            {
                if (++v6 >= s_numNodes)
                    goto label_12;
            }
            node->nextIdx = (node->nextIdx ^ (node->nextIdx ^ (v6 << 14)) & 0xFFFC000);
        }
label_12:
        if ((node->nextIdx << 18) >> 18 == v2)
            node->nextIdx |= 0x3FFF;
        if ((16 * node->nextIdx) >> 18 == v2)
            node->nextIdx |= 0xFFFC000;
        ++i;
    }
    for (int v7 = 0; v7 < s_numNodes; ++v7)
    {
        vehicle_node_t* v9 = s_nodes[v7];
        int v10 = (v9->nextIdx << 18) >> 18;
        if (v10 >= 0)
        {
            v9->dir[0] = s_nodes[v10]->origin[0] - v9->origin[0];
            v9->dir[1] = s_nodes[v10]->origin[1] - v9->origin[1];
            float a1 = s_nodes[v10]->origin[2] - v9->origin[2];
            v9->dir[2] = a1;
            v9->length = VectorNormalize(v9->dir);
            if ((v9->nextIdx & 0x30000000) == 0)
                vectoangles(v9->dir, v9->angles);
        }
    }
    for (int16_t v11 = 0; v11 < s_numNodes; ++v11)
    {
        vehicle_node_t* v13 = s_nodes[v11];
        float a1 = VP_CalcNodeSpeed(v11);
        v13->speed = a1;
        a1 = VP_CalcNodeLookAhead(v11);
        v13->lookAhead = a1;
        if (a1 < 0.0f)
            Com_Error(ERR_DROP, "%s", v13->origin);
        if ((v13->nextIdx & 0x30000000) != 0)
            VP_CalcNodeAngles(v11, v13->angles);
        v13->angles[0] = AngleNormalize180(v13->angles[0]);
        v13->angles[1] = AngleNormalize180(v13->angles[1]);
        v13->angles[2] = AngleNormalize180(v13->angles[2]);
        a1 = 0.0f;
        if (v13->speed <= 0.0f || v13->lookAhead <= 0.0f)
            v13->nextIdx |= 0x3FFF;
        if ((v13->nextIdx & 0x2000) != 0)
        {
            if (v13->speed <= 0.0f)
                v13->speed = 1.0f;
            if (v13->lookAhead <= 0.0f)
                v13->lookAhead = 1.0f;
        }
    }
}

// ea: 0x0044E120
int VEH_GetGenericDistancedFollowHistoryIndex(scr_vehicle_t* veh,
                                              const float* origin,
                                              float requiredDistance,
                                              int startingIndex)
{
    vehicle_follow* follow = veh->follow;
    int i = 0;
    float v6 = requiredDistance * requiredDistance;
    int j;
    for (j = startingIndex + 40;; --j)
    {
        int result = j % 40;
        float v9 = origin[1] - follow->positionHistory[j % 40][1];
        float v10 = origin[0] - follow->positionHistory[j % 40][0];
        float v11 = origin[2] - follow->positionHistory[j % 40][2];
        if ((v11 * v11 + v9 * v9 + v10 * v10) >= v6)
            return result;
        if (++i >= 40)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 3871;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            return veh->follow->historyBufferFront;
        }
    }
}

// ea: 0x0044DFE0
void VEH_GenerateRelativeFormationTable(scr_vehicle_t* veh)
{
    veh->follow->actualColumns = veh->follow->columns;
    vehicle_follow* follow = veh->follow;
    if (follow->numFollowingActors < follow->columns)
        follow->actualColumns = follow->numFollowingActors;
    veh->follow->actualRows =
        (int)ceil((float)veh->follow->numFollowingActors
                  / (float)veh->follow->actualColumns);
    vehicle_follow* v3 = veh->follow;
    int v4 = 0;
    float v5 = 0.0f;
    int r = 0;
    if (v3->actualRows > 0)
    {
        int actualColumns = v3->actualColumns;
        float startingX = (actualColumns - 1) * -0.5f * v3->columnSpacing;
        do
        {
            int v7 = 0;
            float v8 = startingX;
            if (actualColumns > 0)
            {
                vehicle_follow* v9 = veh->follow;
                int v10 = v4;
                do
                {
                    *(float*)((char*)v9->relativeFormation + v10) = v8;
                    *(float*)((char*)&v9->relativeFormation[0][0][1] + v10) = v5;
                    *(float*)((char*)&v9->relativeFormation[0][0][2] + v10) = 0.0f;
                    v9 = veh->follow;
                    actualColumns = v9->actualColumns;
                    ++v7;
                    v10 += 12;
                    v8 = v9->columnSpacing + v8;
                } while (v7 < actualColumns);
            }
            vehicle_follow* v11 = veh->follow;
            float rowSpacing = v11->rowSpacing;
            v5 = v5 - rowSpacing;
            v4 += 72;
            ++r;
        } while (r < veh->follow->actualRows);
    }
}

// ea: 0x0044E260
void VEH_UpdateFollowFormation(scr_vehicle_t* veh, float requiredDistance)
{
    float up[3] = {0.0f, 0.0f, 1.0f};
    vehicle_follow* follow = veh->follow;
    int v3 = (follow->historyBufferFront + 39) % 40;
    float requiredTotalRowDistance =
        (follow->actualRows - 1) * follow->rowSpacing;
    int GenericDistancedFollowHistoryIndex =
        VEH_GetGenericDistancedFollowHistoryIndex(
            veh, veh->phys.origin.v.m128_f32, requiredDistance, v3);
    float v5 = 0.0f;
    int v6 = VEH_GetGenericDistancedFollowHistoryIndex(
        veh,
        veh->follow->positionHistory[(GenericDistancedFollowHistoryIndex + 39) % 40],
        v5, (GenericDistancedFollowHistoryIndex + 39) % 40);
    int numFollowingActors = veh->follow->numFollowingActors;
    int v8 = 0;
    int slot = 0;
    if (numFollowingActors > 0)
    {
        int v28 = 4 * (3 * v6 + 27);
        int v9 = GenericDistancedFollowHistoryIndex;
        int v29 = 12 * v6;
        int v32 = 4 * (3 * GenericDistancedFollowHistoryIndex + 27);
        requiredTotalRowDistance = 0.0f;
        do
        {
            vehicle_follow* v10 = veh->follow;
            int actualColumns = v10->actualColumns;
            int v12 = v8 / actualColumns;
            int v13 = v8 % actualColumns;
            int v14 = v12;
            float minMaxDelta[3];
            minMaxDelta[0] = v10->positionHistory[v9][0]
                             - v10->positionHistory[v29 / 12][0];
            float v15 = v10->positionHistory[v9][1]
                        - v10->positionHistory[v29 / 12][1];
            minMaxDelta[1] = v15;
            minMaxDelta[2] = *(float*)((char*)&v10->numFollowingActors + v32)
                             - *(float*)((char*)&v10->numFollowingActors + v28);
            float angleDiff[3];
            vectoangles(minMaxDelta, angleDiff);
            float rotatedRelativePosition[3];
            RotatePointAroundVector(rotatedRelativePosition, up,
                                    veh->follow->relativeFormation[v14][v13],
                                    (angleDiff[1] - 90.0f));
            veh->follow->slotGoalPosition[slot][0] =
                veh->follow->positionHistory[v9][0] + rotatedRelativePosition[0];
            veh->follow->slotGoalPosition[slot][1] =
                veh->follow->positionHistory[v9][1] + rotatedRelativePosition[1];
            veh->follow->slotGoalPosition[slot][2] =
                *(float*)((char*)veh->follow + v32)
                + rotatedRelativePosition[2];
            v8 = slot + 1;
            slot = v8;
        } while (v8 < veh->follow->numFollowingActors);
    }
}

// ea: 0x0044E440
bool VEH_AcquirePlayerFollowSlot(Entity* vehicle, Entity* follower)
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3985;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3986;
        AeAssert::gCurrentExpr = "follower != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3987;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3988;
        AeAssert::gCurrentExpr = "follower->actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle->follow == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 3993;
        AeAssert::gCurrentExpr = "veh->follow";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor->iFollowSlot == -1)
    {
        if (scr_vehicle->follow->numFollowingActors + 1 > 6)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4004;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            return false;
        }
        scr_vehicle->follow->claimedSlotEntityHandleList[
            scr_vehicle->follow->numFollowingActors].mHandle.mVal =
            follower->mHandle.mHandle.mVal;
        follower->actor->iFollowSlot = scr_vehicle->follow->numFollowingActors++;
        VEH_GenerateRelativeFormationTable(scr_vehicle);
        VEH_UpdateFollowFormation(scr_vehicle, 0.0f);
        return true;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 3997;
    AeAssert::gCurrentExpr = "0";
    if (!AeAssert::IsIgnored()
        && AeAssert::Assert("This actor is already following a vehicle!!!"))
        __debugbreak();
    return false;
}

// ea: 0x0046CF00
void VEH_ReleasePlayerFollowSlot(Entity* vehicle, Entity* follower)
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4025;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4026;
        AeAssert::gCurrentExpr = "follower != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4027;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (follower->actor == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4028;
        AeAssert::gCurrentExpr = "follower->actor";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int iFollowSlot = follower->actor->iFollowSlot;
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (iFollowSlot == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4035;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("This actor was not following a vehicle!!!"))
            __debugbreak();
        return;
    }
    if (scr_vehicle->follow->numFollowingActors <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4039;
        AeAssert::gCurrentExpr = "veh->follow->numFollowingActors > 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(
        scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot]);
    if (mObject != follower)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4040;
        AeAssert::gCurrentExpr = "*veh->follow->claimedSlotEntityHandleList[slot] == follower";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot].mHandle.mVal = 0;
    --scr_vehicle->follow->numFollowingActors;
    follower->actor->iFollowSlot = -1;
    vehicle_follow* follow = scr_vehicle->follow;
    if (follow->numFollowingActors > 0 && iFollowSlot != follow->numFollowingActors)
    {
        Entity* last = HandleDbToEnt(
            follow->claimedSlotEntityHandleList[follow->numFollowingActors]);
        if (last == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4054;
            AeAssert::gCurrentExpr = "*veh->follow->claimedSlotEntityHandleList[veh->follow->numFollowingActors]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot].mHandle.mVal =
            scr_vehicle->follow->claimedSlotEntityHandleList[
                scr_vehicle->follow->numFollowingActors].mHandle.mVal;
        scr_vehicle->follow->claimedSlotEntityHandleList[
            scr_vehicle->follow->numFollowingActors].mHandle.mVal = 0;
        Entity* v7 = HandleDbToEnt(
            scr_vehicle->follow->claimedSlotEntityHandleList[iFollowSlot]);
        if (v7 == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4071;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("TELL STAVRO! Vehicle's entity follow handle got hosed!!! Was weird zone loading/unloading going on???"))
                __debugbreak();
            return;
        }
        actor_s* actor = v7->actor;
        if (actor == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 4064;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Vehicle's actor follow handle got hosed!!! Was weird zone loading/unloading going on???"))
                __debugbreak();
            return;
        }
        actor->iFollowSlot = iFollowSlot;
    }
    VEH_GenerateRelativeFormationTable(scr_vehicle);
    VEH_UpdateFollowFormation(scr_vehicle, 0.0f);
}

// ea: 0x0044E6A0
const float (*VEH_GetPlayerFollowGoalPosition(const Entity* vehicle,
                                              const Entity* follower))[3]
{
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4083;
        AeAssert::gCurrentExpr = "vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (vehicle->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4084;
        AeAssert::gCurrentExpr = "vehicle->scr_vehicle != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int iFollowSlot = follower->actor->iFollowSlot;
    if (iFollowSlot >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 4087;
        AeAssert::gCurrentExpr = "slot >= 0 && slot < 6";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return (const float(*)[3])vehicle->scr_vehicle->follow->slotGoalPosition[iFollowSlot];
}

// ea: 0x0046DC50
void G_FreeVehicle(Entity* ent)
{
    if (ent->scr_vehicle->mRBVeh != nullptr)
        rb_vehicle::remove_vehicle((rb_vehicle*)ent->scr_vehicle->mRBVeh);
    if (ent->scr_vehicle->follow != nullptr)
    {
        TPakId mPakId = (TPakId)ent->mPakId;
        if (mPakId == PAK_ID_INVALID)
            mPakId = CurPakId();
        vehicle_follow* follow = ent->scr_vehicle->follow;
        if (follow != nullptr)
            PakManager::sInst->MemFree(mPakId, follow, false);
        ent->scr_vehicle->follow = nullptr;
    }
    if (HandleDbToEnt(ent->scr_vehicle->mEntity) == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6850;
        AeAssert::gCurrentExpr = "*ent->scr_vehicle->mEntity != 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->health = 0;
    VEH_UpdateSounds(ent, 0);
    Entity* idle = HandleDbToEnt(ent->scr_vehicle->mIdleSndEnt);
    if (idle != nullptr)
        G_FreeEntity(idle, 0);
    Entity* engine = HandleDbToEnt(ent->scr_vehicle->mEngineSndEnt);
    if (engine != nullptr)
        G_FreeEntity(engine, 0);
    vehicle_path_node_t* switchNode = ent->scr_vehicle->pathPos.switchNode;
    ent->think = THINK__NULL;
    ent->pain = 0;
    ent->die = 0;
    ent->touch = 0;
    ent->use = 0;
    ent->controller = 0;
    ent->entinfo = 2;
    ent->nextthink = 0;
    ent->takedamage = 0;
    ent->speed = 0.0f;
    ent->active = 0;
    ent->s.eFlags = 0;
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.apos.trType = TR_STATIONARY;
    switchNode->mName.clear();
    ent->scr_vehicle->pathPos.switchNode[1].mName.clear();
    ent->scr_vehicle->pathPos.switchNode[0].mTarget.clear();
    ent->scr_vehicle->pathPos.switchNode[1].mTarget.clear();
    ent->scr_vehicle->mEntity.mHandle.mVal = 0;
    ent->scr_vehicle = nullptr;
}

// ea: 0x0046FF60
void vehicle_InitDynamicBuffers(unsigned short vehicles)
{
    scr_vehicle_t* v1 = s_vehicles;
    scr_vehicle_t* old_vehicles = nullptr;
    int old_vehicle_size = 0;
    int old_vehicles_used = 0;
    if (s_vehicles != nullptr)
    {
        if (level.MaxVehicles != 0)
        {
            DbLinkedHandle<EntityHandleDb, Entity>* p_mEntity =
                &s_vehicles->mEntity;
            int MaxVehicles = level.MaxVehicles;
            do
            {
                unsigned int v4 = p_mEntity->mHandle.mVal & 0xFFF;
                if (v4 < 0x540
                    && p_mEntity->mHandle.mVal >> 12
                           == EntityHandleDb::sInst.mElements[v4].mKey
                    && EntityHandleDb::sInst.mElements[v4].mObject != nullptr)
                    ++old_vehicles_used;
                p_mEntity += 468;
                --MaxVehicles;
            } while (MaxVehicles != 0);
            if (old_vehicles_used != 0)
            {
                old_vehicle_size = level.MaxVehicles;
                scr_vehicle_t* v5 = (scr_vehicle_t*)mem_heap_malloc(
                    16, sizeof(scr_vehicle_t) * level.MaxVehicles);
                old_vehicles = v5;
                if (v5 == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                    AeAssert::gCurrentLine = 10363;
                    AeAssert::gCurrentExpr = "old_vehicles";
                    if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of memory"))
                        __debugbreak();
                }
                memcpy(v5, s_vehicles, sizeof(scr_vehicle_t) * level.MaxVehicles);
                v1 = s_vehicles;
            }
        }
        if (v1 != nullptr)
        {
            mem_heap_free(v1);
            s_vehicles = nullptr;
        }
    }
    level.MaxVehicles = vehicles;
    if (vehicles != 0)
    {
        s_vehicles = (scr_vehicle_t*)mem_heap_malloc(
            16, sizeof(scr_vehicle_t) * vehicles);
        if (s_vehicles == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 10378;
            AeAssert::gCurrentExpr = "s_vehicles";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Out of memory"))
                __debugbreak();
        }
        memset(s_vehicles, 0, sizeof(scr_vehicle_t) * level.MaxVehicles);
    }
    G_InitScrVehicles();
    if (old_vehicles != nullptr)
    {
        if (old_vehicles_used > level.MaxVehicles)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 10389;
            AeAssert::gCurrentExpr = "old_vehicles_used<=level.MaxVehicles";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("Warning, you're scaling down the size of the vehicle array."))
                __debugbreak();
        }
        scr_vehicle_t* v6 = s_vehicles;
        if (old_vehicle_size > 0)
        {
            DbLinkedHandle<EntityHandleDb, Entity>* v7 =
                &old_vehicles->mEntity;
            for (int vehiclesa = old_vehicle_size; vehiclesa != 0; --vehiclesa)
            {
                unsigned int v8 = v7->mHandle.mVal & 0xFFF;
                if (v8 < 0x540
                    && v7->mHandle.mVal >> 12
                           == EntityHandleDb::sInst.mElements[v8].mKey
                    && EntityHandleDb::sInst.mElements[v8].mObject != nullptr)
                {
                    memcpy(v6, &v7[-92], sizeof(scr_vehicle_t));
                    unsigned int v9 = v7->mHandle.mVal & 0xFFF;
                    Entity* mObject = nullptr;
                    if (v9 < 0x540
                        && v7->mHandle.mVal >> 12
                               == EntityHandleDb::sInst.mElements[v9].mKey)
                        mObject = EntityHandleDb::sInst.mElements[v9].mObject;
                    mObject->scr_vehicle = v6++;
                }
                v7 += 468;
            }
        }
        mem_heap_free(old_vehicles);
    }
}

// ea: 0x0045E900
vehicle_info_t* G_GetVehicleInfo(Entity* ent)
{
    if (ent != nullptr && ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx];
    return nullptr;
}

// ea: 0x00457F90
void G_FreeAnimTreeInstances(void)
{
    for (int i = 0; i < 16; ++i)
        g_scr_data.actorCorpseInfo[i].mEntity.mHandle.mVal = 0;
}

// ea: 0x00490EA0
void VEH_PlayerInteractionEntry(Entity* vehicle)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_LinkPlayer(vehicle, Player, 0, 0, 0);
}

// ea: 0x0044C7E0
void UpdatePaths(Entity* ent)
{
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
            {
                PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
                return;
            }
        }
        else if (ent->moverState == 8)
        {
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            return;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
}

// ea: 0x0044D320
int IsVehicleTank(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    return scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2;
}

// ea: 0x0044EFD0
void G_FreeScrVehicleInfo(void)
{
    for (int i = 0; i < s_numVehicleInfos; mem_heap_free(s_vehicleInfos[i++]))
        ;
    s_numVehicleInfos = 0;
}

// ea: 0x0045E240
void G_FreeScrVehicles(void)
{
    if (level.MaxVehicles != 0)
    {
        for (int v0 = 0; v0 < level.MaxVehicles; ++v0)
        {
            Broc::string* v2 = (Broc::string*)&s_vehicles[v0];
            v2[14].clear();
            v2[15].clear();
            v2[30].clear();
            v2[31].clear();
        }
    }
}

// ea: 0x004523F0
int G_FreeVehiclePaths(void)
{
    int result = s_numNodes;
    int v1 = 0;
    if (s_numNodes > 0)
    {
        result = 0;
        do
        {
            vehicle_node_t* v2 = s_nodes[result];
            v2->mName.clear();
            v2->mTarget.clear();
            result = ++v1;
        } while (v1 < s_numNodes);
    }
    s_numNodes = 0;
    return result;
}

// ea: 0x0046F300
bool scr_vehicle_t::IsPhysicsPaused()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return (*(unsigned int*)((char*)mRBVeh + 0x280) & 1) != 0;  // m_flags.mMask
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x004890C0
void scr_vehicle_t::CollisionDamage(Entity* ent, const math::Position3* pos,
                                    const math::Position3* dir, float intensity)
{
    float damage = *(float*)((char*)s_vehicleInfos[infoIdx] + 0x68) * intensity;
    G_Damage(ent, nullptr, nullptr, dir->v.m128_f32, pos->v.m128_f32,
             (int)damage, 32, 27, HITLOC_NONE, -1);
}

// ea: 0x0044FB30
vehicle_info_t* VEH_GetVehicleInfo(unsigned int iIndex)
{
    if (iIndex > 0x40)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10667;
        AeAssert::gCurrentExpr = "(iIndex >= 0) && (iIndex <= 64)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return s_vehicleInfos[iIndex];
}

// ea: 0x0044F620
int scr_vehicle_t::GetSwitchPosRoute(int seatIdx, int fromPos, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes <= 0)
        return -1;
    for (vehicleAnimRoute_t* i = animMap->routes;
         i->vehPosSrc != fromPos || i->vehPosDest != seatIdx
             || ((i->flags & 4) != 0 && !hasFlag);
         ++i)
    {
        if (++result >= numRoutes)
            return -1;
    }
    return result;
}

// ea: 0x0044D480
int VEH_GetVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0044D4E0
int16_t VEH_GetPlayerVehicleInfo(const char* name)
{
    if (name == nullptr || *name == 0)
        return -1;
    int16_t v1 = 0;
    if (s_numVehicleInfos <= 0)
        return -1;
    while (_stricmp(name, s_vehicleInfos[v1]->name) != 0)
    {
        if (++v1 >= s_numVehicleInfos)
            return -1;
    }
    return v1;
}

// ea: 0x0046A370
void VEH_SetPosition(Entity* ent, const math::Position3* origin,
                     const math::Position3* angles, const float* vel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (ent->takedamage != 0)
    {
        ent->s.pos.trBase[0] = ent->r.currentOrigin.v.m128_f32[0];
        ent->s.pos.trBase[1] = ent->r.currentOrigin.v.m128_f32[1];
        ent->s.pos.trBase[2] = ent->r.currentOrigin.v.m128_f32[2];
        ent->s.pos.trDelta[0] = origin->v.m128_f32[0];
        ent->s.pos.trDelta[1] = origin->v.m128_f32[1];
        ent->s.pos.trDelta[2] = origin->v.m128_f32[2];
        if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
            || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 1292;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->r.currentOrigin.v.m128_f32[0] = origin->v.m128_f32[0];
        ent->r.currentOrigin.v.m128_f32[1] = origin->v.m128_f32[1];
        ent->r.currentOrigin.v.m128_f32[2] = origin->v.m128_f32[2];
        ent->s.apos.trBase[0] = ent->r.currentAngles.v.m128_f32[0];
        ent->s.apos.trBase[1] = ent->r.currentAngles.v.m128_f32[1];
        ent->s.apos.trBase[2] = ent->r.currentAngles.v.m128_f32[2];
        ent->s.apos.trDelta[0] = angles->v.m128_f32[0];
        ent->s.apos.trDelta[1] = angles->v.m128_f32[1];
        ent->s.apos.trDelta[2] = angles->v.m128_f32[2];
        ent->r.currentAngles.v.m128_f32[0] = angles->v.m128_f32[0];
        ent->r.currentAngles.v.m128_f32[1] = angles->v.m128_f32[1];
        ent->r.currentAngles.v.m128_f32[2] = angles->v.m128_f32[2];
        ent->s.pos.trType = TR_INTERPOLATE;
        ent->s.apos.trType = TR_INTERPOLATE;
        if (ent->takedamage != 0)
            g_LinkEntity(ent);
        Entity* v7 = HandleDbToEnt(scr_vehicle->mIdleSndEnt);
        if (v7 != nullptr)
        {
            G_SetOrigin(v7, origin);
            G_SetAngle(v7, angles);
            v7->s.pos.trType = TR_INTERPOLATE;
            v7->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v7);
        }
        Entity* v9 = HandleDbToEnt(scr_vehicle->mEngineSndEnt);
        if (v9 != nullptr)
        {
            G_SetOrigin(v9, origin);
            G_SetAngle(v9, angles);
            v9->s.pos.trType = TR_INTERPOLATE;
            v9->s.apos.trType = TR_INTERPOLATE;
            g_LinkEntity(v9);
        }
        if ((ent->flags & 0x20000000) != 0)
            G_SetEntityOceanHeight(ent);
    }
    else
    {
        SV_UnlinkEntity(ent);
    }
}

// ea: 0x0046DBF0
void G_SetupScrVehicles(void)
{
    int v0 = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            unsigned int mVal = s_vehicles[v1].mEntity.mHandle.mVal;
            if (mVal != 0)
            {
                Entity* Entity = VEH_GetEntity(mVal);
                Entity->s.brushmodel = 0;
                SV_SetBrushModel(Entity);
                Entity->r.contents = 0xA00000;
            }
            v1 = ++v0;
        } while (v0 < level.MaxVehicles);
    }
}

// ea: 0x0045F350
void shotgunrandom(float* x, float* y, float randomA, float randomB)
{
    float sinT;
    FastSinCos(((randomA * 360.0f) * 3.1415927f) * 0.0055555557f, &sinT, &randomA);
    *x = randomA * randomB;
    *y = sinT * randomB;
}

// ea: 0x0046F2A0
bool scr_vehicle_t::IsPhysicsStable()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
    {
        rb_extra_info* m_chassis_rbinf = *(rb_extra_info**)((char*)mRBVeh + 0x274);  // m_chassis_rbinf
        if (m_chassis_rbinf != nullptr)
            return (*(unsigned int*)((char*)m_chassis_rbinf->m_rb + 0x280) & 4) != 0;
    }
    Entity* mObject = HandleDbToEnt(
        *(DbLinkedHandle<EntityHandleDb, Entity>*)((char*)this + 0x1E0));  // seats[0].occupant
    return mObject == nullptr;
}

// ea: 0x0044F330
bool IsVehFlipped(Entity* ent)
{
    bool result = false;
    if (ent != nullptr)
    {
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        if (scr_vehicle != nullptr
            && scr_vehicle->mRBVeh != nullptr
            && ent->r.currentMat.z.v.m128_f32[2] < 0.2f)
        {
            return true;
        }
    }
    return result;
}

// ea: 0x0044F720
int scr_vehicle_t::GetMantleHintStringIndex()
{
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10113;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    return s_vehicleInfos[infoIdx]->mMantleHintStringIndex;
}

// ea: 0x0046F350
bool scr_vehicle_t::IsOppositeTeamInVehicle(int team)
{
    for (int v2 = 0; v2 < 11; ++v2)
    {
        Entity* mObject = HandleDbToEnt(seats[v2].occupant);
        if (mObject != nullptr)
        {
            sentient_s* sentient = mObject->sentient;
            if (sentient != nullptr && sentient->eTeam != team)
                return true;
        }
    }
    return false;
}

// ea: 0x0044F6A0
void scr_vehicle_t::Mantled(Entity* player)
{
    if (mMantleTime != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10101;
        AeAssert::gCurrentExpr = "mMantleTime == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Vehicle has already been mantled"))
            __debugbreak();
    }
    mMantleTime = level.time + 1800;
    mMantleEntity.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0045E1D0
int G_InitScrVehicles(void)
{
    int result = 0;
    if (level.MaxVehicles != 0)
    {
        int v0 = 0;
        int v1 = 0;
        do
        {
            int v2 = v1;
            G_VehInitPathPos(&s_vehicles[v2].pathPos);
            ++v0;
            s_vehicles[v2].mEntity.mHandle.mVal = 0;
            result = level.MaxVehicles;
            v1 = v0;
        } while (v0 < level.MaxVehicles);
    }
    level.vehicles = s_vehicles;
    return result;
}

// ea: 0x0044DBD0
void VEH_StopWheelEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (count != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[i].mVal);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle.mVal);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
}

// ea: 0x0046E040
Entity* G_IsVehicleUnusable(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return nullptr;
    if ((0x100000 & client->ps.eFlags) == 0)
        return nullptr;
    Entity* v4 = HandleDbToEnt(player->r.mOwner);
    if (v4 == nullptr)
        return nullptr;
    return (0x200000 & v4->r.contents) != 0 ? v4 : nullptr;
}

// ea: 0x0046E200
bool G_IsPlayerVehicleGunner(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* mOwner = HandleDbToEnt(player->r.mOwner);
    if (mOwner != nullptr && mOwner->scr_vehicle != nullptr)
        return client->ps.vehPos == 1;
    return false;
}

// ea: 0x0046F9F0
int G_EntryPointSeatAssociation(Entity* vehicle, int entryPosition)
{
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (s_vehicleInfos[scr_vehicle->infoIdx]->type != 2)
        return sEntryPointSeatAssociation[entryPosition];
    Entity* mObject = HandleDbToEnt(scr_vehicle->seats[0].occupant);
    return mObject != nullptr;
}

// ea: 0x00470490
vehicle_info_t* VEH_GetPlayerVehicleInfo(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player == nullptr)
        return nullptr;
    Entity* mObject = HandleDbToEnt(Player->r.mOwner);
    if (mObject == nullptr)
        return nullptr;
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle == nullptr)
        return nullptr;
    vehicle_info_t* result = s_vehicleInfos[scr_vehicle->infoIdx];
    if (result == nullptr)
        return nullptr;
    return result;
}

// ea: 0x0046F460
bool scr_vehicle_t::LetHatchClose()
{
    if (s_vehicleInfos[infoIdx]->type != 2)
        return true;
    int v2 = 0;
    for (int v4 = 0; v4 < 11; ++v4)
    {
        Entity* mObject = HandleDbToEnt(seats[v4].occupant);
        if (mObject == nullptr)
            continue;
        if (v4 != 0)
        {
            if (v4 == 7 && mObject->client->mVehicleAnimStage < 2)
                continue;
            return false;
        }
        if (!IsPlayerFullySeatedInVehicle(mObject))
            return false;
        v2 = 1;
    }
    return v2 != 0;
}

// ea: 0x0044F2A0
int G_GetVehicleOccupantCount(Entity* ent)
{
    if (ent->scr_vehicle != nullptr)
        return ent->scr_vehicle->playersAttached;
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7731;
    AeAssert::gCurrentExpr = "ent->scr_vehicle";
    if (AeAssert::IsIgnored() || AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
        __debugbreak();
    return ent->scr_vehicle->playersAttached;
}

// ea: 0x0045E930
int G_GetVehicleSeatCount(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7725;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_GetVehicleSeatCount: Entity not a vehicle"))
            __debugbreak();
    }
    if (ent->scr_vehicle != nullptr)
        return s_vehicleInfos[ent->scr_vehicle->infoIdx]->numSeats;
    return 0;
}

// ea: 0x004811F0
void Scr_Vehicle_GetOut(Entity* vehicle, Entity* occupant, int health)
{
    Entity* v3 = occupant;
    if (occupant != nullptr && occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController_Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController_Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController_Inst(occupant->GetPlayerIndex()), 1);
        }
    }
    VEH_UnlinkPlayer(v3, true);
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(v3))
    {
        unsigned int occupantHandle = v3->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.deactivate, &occupantHandle);
    }
}

// ea: 0x004807D0
void VEH_UnlinkPlayerDropped(Entity* ent)
{
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6449;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    ent->s.eFlags &= 0xFFEFFFFF;
    ent->active = 0;
    ent->r.mOwner.mHandle.mVal = 0;
    Scr_Notify(ent, hash_const.player_off_vehicle, 0);
}

// ea: 0x0046E150
bool G_IsPlayerDrivingVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7337;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    if (v3->scr_vehicle == nullptr)
        return false;
    return client->ps.vehPos == 0;
}

// ea: 0x004918E0
void Scr_Vehicle_GetIn(Entity* vehicle, Entity* occupant, int health,
                       unsigned int seatIdx, int entryIdx)
{
    unsigned int v5 = seatIdx;
    if (seatIdx == 0)
        vehicle->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(vehicle, occupant, (int)v5, entryIdx, 0);
    scr_vehicle_t* scr_vehicle = vehicle->scr_vehicle;
    if (scr_vehicle != nullptr && s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        scr_vehicle->noEntryTime = level.time + 4000;
    vehicle->health = health;
    if (EntityManager::sInst->IsLocalPlayer(occupant))
    {
        unsigned int h = occupant->mHandle.mHandle.mVal;
        vehicle->Notify(hash_const.activate, &h);
    }
}

// ea: 0x00480CF0
void VEH_RotateWheels(Entity* self, vehicle_info_t* info)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    float trans[3] = { 0.0f, 0.0f, 0.0f };
    float steerAngles[3] = { 0.0f, 0.0f, 0.0f };
    int numWheels = 2 * (info->type != 1) + 4;
    if (2 * (info->type != 1) != -4)
    {
        int v4 = 0;
        int* wheel = scr_vehicle->boneIndex.wheel;
        do
        {
            int v7 = *wheel;
            if (*wheel > 0)
            {
                steerAngles[1] = scr_vehicle->current.mSteeringAngle;
                steerAngles[0] = scr_vehicle->wheelPitch;
                if (v4 != TAG_WHEEL_FRONT_LEFT && v4 != TAG_WHEEL_FRONT_RIGHT)
                    steerAngles[1] = 0.0f;
                G_DObjSetLocalTagInternal_0(trans, steerAngles, v7, self, 0);
            }
            ++v4;
            ++wheel;
        } while (v4 < numWheels);
    }
}

// ea: 0x0044DC50
void VEH_StopAllEffects(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    int count = 2 * (v2->type != 1) + 4;
    if (2 * (v2->type != 1) != -4)
    {
        for (int i = 0; i < count; ++i)
        {
            if (scr_vehicle->mWheel_ParticleEffectHandle[i].mVal != 0)
            {
                EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[i].mVal);
                scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
            }
        }
    }
    if (scr_vehicle->mRumbleEffectHandle.mVal != 0)
    {
        EffectEventStopEmitting(scr_vehicle->mRumbleEffectHandle.mVal);
        scr_vehicle->mRumbleEffectHandle.mVal = 0;
    }
    for (int i = 6; i != 0; --i)
    {
        if (scr_vehicle->mSoundEffectHandle[6 - i].mVal != 0)
            EffectEventStopEmitting(scr_vehicle->mSoundEffectHandle[6 - i].mVal);
        scr_vehicle->mSoundEffectHandle[6 - i].mVal = 0;
    }
}

// ea: 0x0046DEA0
void G_FreeVehicleRefs(Entity* ent)
{
    int i = 0;
    if (level.MaxVehicles != 0)
    {
        int v1 = 0;
        do
        {
            scr_vehicle_t* v2 = &s_vehicles[v1];
            if (HandleDbToEnt(s_vehicles[v1].mEntity) != nullptr)
            {
                if (v2->mIdleSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mIdleSndEnt.mHandle.mVal = 0;
                if (v2->mEngineSndEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mEngineSndEnt.mHandle.mVal = 0;
                if (v2->mTargetEnt.mHandle.mVal == ent->mHandle.mHandle.mVal)
                    v2->mTargetEnt.mHandle.mVal = 0;
            }
            ++v1;
            ++i;
        } while (i < level.MaxVehicles);
    }
}

// ea: 0x0044F100
vehicle_info_t* G_GetVehicleInfoName(int16_t index)
{
    if (index < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7097;
        AeAssert::gCurrentExpr = "index >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < s_numVehicleInfos)
        return s_vehicleInfos[index];
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 7098;
    AeAssert::gCurrentExpr = "index < s_numVehicleInfos";
    if (AeAssert::IsIgnored() || AeAssert::Assert("old cod assert"))
        __debugbreak();
    return s_vehicleInfos[index];
}

// ea: 0x0045E9B0
void Scr_Vehicle_Pain(Entity* pSelf, Entity* pAttacker, int /*damage*/,
                      const float* /*point*/, int mod, const float* dir,
                      hitLocation_t /*hitLoc*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    if ((scr_vehicle == nullptr || scr_vehicle->mRBVeh == nullptr) && pAttacker != nullptr)
    {
        switch (mod)
        {
        case 3:
        case 4:
        case 5:
        case 6:
        case 9:
        case 10:
        case 17:
        case 18:
        case 27:
        case 28:
        case 32:
        {
            math::Position3 v8;
            v8.v.m128_f32[0] = dir[0];
            v8.v.m128_f32[1] = dir[1];
            v8.v.m128_f32[2] = dir[2];
            v8.v.m128_f32[3] = 0.0f;
            VEH_JoltBody(pSelf, &v8, 1.0f, 0.0f, 0.0f);
            break;
        }
        default:
            return;
        }
    }
}

// ea: 0x00480880
Client* G_IsVehicleUsable(Entity* ent, Entity* player, bool speedCheck)
{
    Client* result = player->client;
    if (result != nullptr)
    {
        if (ent->health < 0
            || (ent->s.eFlags & 0x80000) != 0
            || (result->ps.eFlags & 0x100000) != 0
            || HandleDbToEnt(player->r.mOwner) != nullptr
            || ent->scr_vehicle->noEntryTime + 200 > level.time)
        {
            return nullptr;
        }
        vehicle_info_t* VehicleInfo = G_GetVehicleInfo(ent);
        scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
        bool canUse = CanMantleVehicle(scr_vehicle, player)
                      || (scr_vehicle->playersAttached < VehicleInfo->numSeats
                          && (VehicleInfo->type != 2
                              || player->sentient == nullptr
                              || !scr_vehicle->IsOppositeTeamInVehicle(player->sentient->eTeam)));
        if (canUse
            && (ent->r.contents & 0x200000) != 0
            && (!speedCheck || (ent->speed <= 100.0f && ent->health > 0)))
        {
            return result;
        }
        return nullptr;
    }
    return result;
}

// ea: 0x0044F010
int16_t G_GetVehicleInfoIndex(const char* name)
{
    if (name == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7080;
        AeAssert::gCurrentExpr = "name";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (*name == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7081;
        AeAssert::gCurrentExpr = "name[0]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    int16_t VehicleInfo = (int16_t)VEH_GetVehicleInfo(name);
    if (VehicleInfo == -1)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7084;
        AeAssert::gCurrentExpr = "index != -1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    return VehicleInfo;
}

// ea: 0x0046E540
DbLinkedHandle<EntityHandleDb, Entity> G_GetTankEntNum(int index)
{
    DbLinkedHandle<EntityHandleDb, Entity> result;
    result.mHandle.mVal = 0;
    if (index >= level.MaxVehicles)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7601;
        AeAssert::gCurrentExpr = "index < level.MaxVehicles";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* v2 = &level.vehicles[index];
    Entity* v4 = HandleDbToEnt(v2->mEntity);
    if (v4 != nullptr
        && s_vehicleInfos[v2->infoIdx]->type == 2
        && v2->drawOnCompass != 0
        && v4->scr_vehicle != nullptr
        && v4->health > 0
        && HandleDbToEnt(v4->r.mOwner) == nullptr)
    {
        result.mHandle.mVal = v4->mHandle.mHandle.mVal;
    }
    return result;
}

// ea: 0x00488BE0
void Scr_Vehicle_Die(Entity* pSelf, Entity* pInflictor, Entity* pAttacker,
                     int /*damage*/, int mod, int weapon, const float* position,
                     const float* dir, hitLocation_t hitLoc)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    scr_vehicle->playEngineSound = 0;
    VEH_StopWheelEffects(pSelf);
    for (int seat = 0; seat < 11; ++seat)
    {
        Entity* mObject = HandleDbToEnt(scr_vehicle->seats[seat].occupant);
        if (mObject != nullptr)
        {
            Scr_Vehicle_GetOut(pSelf, mObject, pSelf->health);
            G_Damage(mObject, pInflictor, pAttacker, dir, position, 9999, 160,
                     mod, hitLoc, weapon);
        }
    }
    Entity* v16 = pAttacker;
    if (pAttacker == nullptr)
    {
        v16 = pInflictor;
        if (pInflictor == nullptr)
            v16 = pSelf;
    }
    Scr_NotifyFromEnt(pSelf, hash_const.death, v16);
}

// ea: 0x00480970
void Scr_Vehicle_Controller(Entity* pSelf)
{
    if (pSelf == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7746;
        AeAssert::gCurrentExpr = "pSelf";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (pSelf->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7747;
        AeAssert::gCurrentExpr = "pSelf->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    float bodyAngles[3];
    bodyAngles[0] = scr_vehicle->next.mBodyPosition.v.m128_f32[0];
    bodyAngles[1] = 0.0f;
    bodyAngles[2] = scr_vehicle->next.mBodyPosition.v.m128_f32[2];
    int body = scr_vehicle->boneIndex.body;
    if (body >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, bodyAngles, body, pSelf, 0);
    float turretAngles[3] = { 0.0f, scr_vehicle->next.mTurretAngles.v.m128_f32[1], 0.0f };
    float barrelAngles[3] = { scr_vehicle->next.mTurretAngles.v.m128_f32[0], 0.0f, 0.0f };
    int turret = scr_vehicle->boneIndex.turret;
    if (turret >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, turretAngles, turret, pSelf, 0);
    int barrel = scr_vehicle->boneIndex.barrel;
    if (barrel >= 0)
        G_DObjSetLocalTagInternal_0(vec3_origin, barrelAngles, barrel, pSelf, 0);
}

// ea: 0x0044FB90
bool VEH_VehicleTouchesMine(Entity* vehicle, EntityState* item)
{
    float v12[3];
    v12[0] = item->pos.trBase[0];
    v12[1] = item->pos.trBase[1];
    v12[2] = item->pos.trBase[2];
    weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(item->weapon);
    if (InfoForWeapon->iTriggerRadius == 0)
        return false;
    rb_vehicle* mRBVeh = (rb_vehicle*)vehicle->scr_vehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return false;
    int v4 = InfoForWeapon->iTriggerRadius * InfoForWeapon->iTriggerRadius;
    float dx = v12[0] - vehicle->r.currentOrigin.v.m128_f32[0];
    float dy = v12[1] - vehicle->r.currentOrigin.v.m128_f32[1];
    float dz = v12[2] - vehicle->r.currentOrigin.v.m128_f32[2];
    if (((dx * dx) + (dy * dy)) + (dz * dz)
        > (vehicle->scr_vehicle->mUseRadius * vehicle->scr_vehicle->mUseRadius) + v4)
    {
        return false;
    }
    for (int v7 = 0; v7 < 6; ++v7)
    {
        rigid_body_constraint_wheel* wheel = mRBVeh->m_wheels[v7];
        if (wheel != nullptr)
        {
            float wx = v12[0] - wheel->m_origin[0];
            float wy = v12[1] - wheel->m_origin[1];
            float wz = v12[2] - wheel->m_origin[2];
            if (v4 > ((wx * wx) + (wy * wy)) + (wz * wz))
                return true;
        }
    }
    return false;
}

// ea: 0x004526D0
void G_VehInitPathPos(vehicle_pathpos_t* vpp)
{
    vpp->nodeIdx = -1;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = 0.0f;
    vpp->lookAhead = 0.0f;
    vpp->slide = 0.0f;
    vpp->origin[0] = 0.0f;
    vpp->origin[1] = 0.0f;
    vpp->angles[0] = 0.0f;
    vpp->angles[1] = 0.0f;
    vpp->lookPos[0] = 0.0f;
    vpp->lookPos[1] = 0.0f;
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452870
void G_VehSetUpPathPos(vehicle_pathpos_t* vpp, int16_t nodeIdx)
{
    vehicle_node_t* v3 = s_nodes[nodeIdx];
    vpp->nodeIdx = nodeIdx;
    vpp->endOfPath = 0;
    vpp->frac = 0.0f;
    vpp->speed = v3->speed;
    vpp->lookAhead = v3->lookAhead;
    vpp->slide = (v3->nextIdx & 0x30000000) != 0 ? 1.0f : 0.0f;
    vpp->origin[0] = v3->origin[0];
    vpp->origin[1] = v3->origin[1];
    vpp->origin[2] = v3->origin[2];
    vpp->angles[0] = v3->angles[0];
    vpp->angles[1] = v3->angles[1];
    vpp->angles[2] = v3->angles[2];
    vpp->lookPos[0] = v3->origin[0];
    vpp->lookPos[1] = v3->origin[1];
    vpp->lookPos[2] = v3->origin[2];
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
}

// ea: 0x00452A10
void G_VehSetSwitchNode(vehicle_pathpos_t* vpp, int16_t srcNodeIdx, uint16_t dstNodeIdx)
{
    for (int i = 0; i < 2; ++i)
    {
        vehicle_path_node_t* node = &vpp->switchNode[i];
        node->mName.clear();
        node->mTarget.clear();
        node->speed = -1.0f;
        node->lookAhead = -1.0f;
        node->origin[0] = 0.0f;
        node->origin[1] = 0.0f;
        node->dir[0] = 0.0f;
        node->dir[1] = 0.0f;
        node->angles[0] = s_invalidAngles[0];
        node->angles[1] = dword_DD7418;
        node->angles[2] = dword_DD741C;
        node->length = 0.0f;
        node->nextIdx = 0xFFFFFFF;
    }
    if (srcNodeIdx >= 0 && (dstNodeIdx & 0x8000u) == 0)
    {
        vehicle_node_t* v6 = s_nodes[srcNodeIdx];
        vehicle_node_t* dstNode = s_nodes[dstNodeIdx];
        VP_CopyNode(v6, &vpp->switchNode[0]);
        VP_CopyNode(v6, &vpp->switchNode[1]);
        vpp->switchNode[0].nextIdx ^= (dstNodeIdx ^ vpp->switchNode[0].nextIdx) & 0x3FFF;
        vpp->switchNode[0].dir[0] = dstNode->origin[0] - v6->origin[0];
        vpp->switchNode[0].dir[1] = dstNode->origin[1] - v6->origin[1];
        vpp->switchNode[0].dir[2] = dstNode->origin[2] - v6->origin[2];
        vpp->switchNode[0].length = VectorNormalize(vpp->switchNode[0].dir);
    }
}

// ea: 0x0045F1A0 (VP_CopyNode)
void VP_CopyNode(vehicle_node_t* src, vehicle_path_node_t* dst)
{
    dst->mName = src->mName;
    dst->mTarget = src->mTarget;
    dst->speed = src->speed;
    dst->lookAhead = src->lookAhead;
    dst->script_noteworthy = src->script_noteworthy;
    memcpy(dst->origin, src->origin, sizeof(dst->origin));
    memcpy(dst->dir, src->dir, sizeof(dst->dir));
    memcpy(dst->angles, src->angles, sizeof(dst->angles));
    dst->length = src->length;
    dst->nextIdx = src->nextIdx;
}

// ea: 0x00488280
int G_SpawnVehicle(Entity* ent, const char* typeName)
{
    scr_vehicle_t* v3;
    if (ent->scr_vehicle != nullptr)
    {
        v3 = ent->scr_vehicle;
    }
    else
    {
        v3 = nullptr;
        int16_t v4 = 0;
        if (level.MaxVehicles != 0)
        {
            do
            {
                int v5 = v4;
                unsigned int mVal = s_vehicles[v5].mEntity.mHandle.mVal;
                v3 = &s_vehicles[v5];
                if (mVal & 0xFFF >= 0x540
                    || mVal >> 12 != EntityHandleDb::sInst.mElements[mVal & 0xFFF].mKey
                    || EntityHandleDb::sInst.mElements[mVal & 0xFFF].mObject == nullptr)
                {
                    break;
                }
                ++v4;
            } while (v4 < level.MaxVehicles);
        }
        if (v4 == level.MaxVehicles)
            Com_Error(ERR_DROP, "Too many vehicles");
    }
    int16_t v8 = v3->infoIdx;
    memset(v3, 0, sizeof(scr_vehicle_t));
    v3->mTargetEnt.mHandle.mVal = 0;
    v3->mIdleSndEnt.mHandle.mVal = 0;
    v3->mEngineSndEnt.mHandle.mVal = 0;
    int16_t infoIdxa = v8;
    v3->playEngineSound = 1;
    if (typeName != nullptr && (infoIdxa = (int16_t)VEH_GetVehicleInfo(typeName)) < 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6803;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't find info for script vehicle [%s]\n", typeName))
            __debugbreak();
        return 0;
    }
    VEH_InitEntity(ent, v3, infoIdxa);
    VEH_InitVehicle(v3);
    ent->s.brushmodel = 0;
    SV_SetBrushModel(ent);
    ent->r.contents = 0xA00000;
    if (ent->scr_vehicle != nullptr
        && s_vehicleInfos[ent->scr_vehicle->infoIdx]->type == 3)
    {
        ent->SetAlwaysRender(true);
    }
    return 1;
}

// ea: 0x0045F030
int G_VehUpdatePathPos(Entity* pEnt, vehicle_pathpos_t* vpp, bool overrideSpeed,
                       int msec, int waitNode)
{
    if (vpp->endOfPath != 0)
    {
        if (vpp->switchNode[0].mName.mBlock == nullptr
            || vpp->switchNode[0].mName.c_str()[0] == 0)
        {
            return 0;
        }
    }
    vehicle_path_node_t* switchNode = &vpp->switchNode[0];
    if (switchNode->mName.mBlock != nullptr && switchNode->mName.c_str()[0] != 0)
    {
        int NodeIndex = VP_GetNodeIndex(&switchNode->mName, nullptr);
        if (NodeIndex >= 0)
        {
            VP_CopyNode(s_nodes[NodeIndex], &vpp->switchNode[0]);
        }
    }
    VP_GetLookAheadXYZ(vpp, vpp->lookPos);
    float lookDir[3];
    lookDir[0] = vpp->lookPos[0] - vpp->origin[0];
    lookDir[1] = vpp->lookPos[1] - vpp->origin[1];
    lookDir[2] = vpp->lookPos[2] - vpp->origin[2];
    float dist = VectorNormalize(lookDir);
    if (dist <= 0.0f)
    {
        vpp->endOfPath = 1;
    }
    else
    {
        vectoangles(lookDir, vpp->angles);
        vpp->angles[0] = AngleNormalize180(vpp->angles[0]);
        vpp->angles[1] = AngleNormalize180(vpp->angles[1]);
        vpp->angles[2] = AngleNormalize180(vpp->angles[2]);
        float v14 = (msec * 0.001f) * vpp->speed;
        if (v14 > dist)
        {
            int next = s_nodes[vpp->nodeIdx]->nextIdx;
            if (next >= 0 && (s_nodes[next]->nextIdx & 0x2000) == 0)
                v14 = dist;
        }
        vpp->origin[0] += v14 * lookDir[0];
        vpp->origin[1] += v14 * lookDir[1];
        vpp->origin[2] += v14 * lookDir[2];
        VP_UpdatePathPos(pEnt, vpp, lookDir, overrideSpeed, waitNode);
        VP_GetAngles(vpp, vpp->angles);
    }
    if (switchNode->mName.mBlock != nullptr
        && switchNode->mName.mBlock != (Broc::string::Block*)-12
        && switchNode->mName.c_str()[0] != 0)
    {
        int v18 = VP_GetNodeIndex(&switchNode->mName, nullptr);
        if (v18 >= 0)
            VP_CopyNode(s_nodes[v18], &vpp->switchNode[1]);
    }
    return 0;
}

// ea: 0x00491980
unsigned int Scr_Vehicle_SeatChange(Entity* occupant, unsigned int newSeatIdx)
{
    if (newSeatIdx > 0xA)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9369;
        AeAssert::gCurrentExpr = "newSeatIdx > VEHPOS_UNKNOWN && newSeatIdx <= VEHPOS_LAST_USABLE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid seat index"))
            __debugbreak();
    }
    Entity* mObject = HandleDbToEnt(occupant->r.mOwner);
    if (mObject == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9371;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("G_VehicleOccupantChangeSeat:  specified occupant not attached to vehicle"))
            __debugbreak();
        return 0;
    }
    int vehPos = occupant->client->ps.vehPos;
    int fromPos = vehPos;
    if (occupant->IsLocalPlayer())
    {
        int PlayerIndex = occupant->GetPlayerIndex();
        if (InteractionController_Inst(PlayerIndex) != nullptr
            && *(void**)InteractionController_Inst(PlayerIndex) != nullptr)
        {
            InteractionController_EndInteraction(InteractionController_Inst(PlayerIndex), 1);
        }
    }
    scr_vehicle_t* scr_vehicle = mObject->scr_vehicle;
    if (scr_vehicle != nullptr
        && s_vehicleInfos[scr_vehicle->infoIdx]->type == 1
        && vehPos == 1
        && occupant == EntityManager::sInst->GetPlayer(currCl))
    {
        SetClientViewAngle(occupant, mObject->r.currentAngles.v.m128_f32);
        scr_vehicle->current.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->current.mGunnerAngles.v.m128_f32[1] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[0] = 0.0f;
        scr_vehicle->next.mGunnerAngles.v.m128_f32[1] = 0.0f;
    }
    VEH_UnlinkPlayer(occupant, false);
    if (newSeatIdx == 0)
        mObject->scr_vehicle->AssignPhysics(occupant);
    VEH_LinkPlayer(mObject, occupant, (int)newSeatIdx, 0, vehPos);
    return newSeatIdx;
}

// ea: 0x00488ED0
void VEH_RespawnVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    VEH_StopAllEffects(ent);
    void* mRBVeh = scr_vehicle->mRBVeh;
    float respawn_origin[3];
    respawn_origin[0] = scr_vehicle->respawn_origin.v.m128_f32[0];
    respawn_origin[1] = scr_vehicle->respawn_origin.v.m128_f32[1];
    respawn_origin[2] = scr_vehicle->respawn_origin.v.m128_f32[2];
    float respawn_angles[3];
    respawn_angles[0] = scr_vehicle->respawn_angles.v.m128_f32[0];
    respawn_angles[1] = scr_vehicle->respawn_angles.v.m128_f32[1];
    respawn_angles[2] = scr_vehicle->respawn_angles.v.m128_f32[2];
    if (mRBVeh != nullptr)
    {
        VEH_RemoveVehicle(mRBVeh);
        scr_vehicle->mRBVeh = nullptr;
    }
    ent->s.eFlags &= ~0x80u;
    G_SpawnVehicle(ent, nullptr);
    scr_vehicle->phys.origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.origin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = respawn_origin[2];
    ent->r.currentOrigin.v.m128_f32[0] = respawn_origin[0];
    ent->r.currentOrigin.v.m128_f32[1] = respawn_origin[1];
    ent->r.currentOrigin.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->phys.angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.angles.v.m128_f32[2] = respawn_angles[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = respawn_angles[2];
    ent->r.currentAngles.v.m128_f32[0] = respawn_angles[0];
    ent->r.currentAngles.v.m128_f32[1] = respawn_angles[1];
    ent->r.currentAngles.v.m128_f32[2] = respawn_angles[2];
    float vel[3] = { 0.0f, 0.0f, 0.0f };
    VEH_SetPosition(ent, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles, vel);
    scr_vehicle->respawn_origin.v.m128_f32[0] = respawn_origin[0];
    scr_vehicle->respawn_origin.v.m128_f32[1] = respawn_origin[1];
    scr_vehicle->respawn_origin.v.m128_f32[2] = respawn_origin[2];
    ent->rotate.v.m128_f32[0] = respawn_origin[0];
    ent->rotate.v.m128_f32[1] = respawn_origin[1];
    ent->rotate.v.m128_f32[2] = respawn_origin[2];
    scr_vehicle->respawn_angles.v.m128_f32[0] = respawn_angles[0];
    scr_vehicle->respawn_angles.v.m128_f32[1] = respawn_angles[1];
    scr_vehicle->respawn_angles.v.m128_f32[2] = respawn_angles[2];
}

// ea: 0x00488CE0
void SP_script_vehicle(Entity* pSelf)
{
    static unsigned char s_init = 0;
    static unsigned int vehicletype_hash = 0;
    if (!(s_init & 1))
    {
        s_init |= 1;
        vehicletype_hash = HashString::CalcHash("vehicletype");
    }
    const char* typeName = nullptr;
    G_SpawnString(vehicletype_hash, nullptr, &typeName);
    if (s_numVehicleInfos <= 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10231;
        AeAssert::gCurrentExpr = "s_numVehicleInfos > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("No vehicle type files are loaded.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (typeName == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10232;
        AeAssert::gCurrentExpr = "typeName";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid vehicletype found in level.\nDo all the vehicles in your level\nhave the \"vehicletype\" key?"))
            __debugbreak();
    }
    if (G_SpawnVehicle(pSelf, typeName) != 0)
    {
        scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
        if (s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
        {
            VEH_Backup(pSelf);
            if ((pSelf->flags & 0x400000) == 0)
                VEH_SetPosition(pSelf, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles,
                                scr_vehicle->phys.vel.v.m128_f32);
        }
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_origin.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->rotate.v.m128_f32[0] = pSelf->r.currentOrigin.v.m128_f32[0];
        pSelf->rotate.v.m128_f32[1] = pSelf->r.currentOrigin.v.m128_f32[1];
        pSelf->rotate.v.m128_f32[2] = pSelf->r.currentOrigin.v.m128_f32[2];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[0] = pSelf->r.currentAngles.v.m128_f32[0];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[1] = pSelf->r.currentAngles.v.m128_f32[1];
        pSelf->scr_vehicle->respawn_angles.v.m128_f32[2] = pSelf->r.currentAngles.v.m128_f32[2];
    }
}

// ea: 0x0046FD50
int scr_vehicle_t::GetEntryHintStringIndex(Entity* vehicle, unsigned int entryPosition)
{
    if (entryPosition >= 6)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10121;
        AeAssert::gCurrentExpr = "entryPosition >= 0 && entryPosition < NUM_ENTRY_POINTS";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid entry point index"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx] == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10122;
        AeAssert::gCurrentExpr = "s_vehicleInfos[ infoIdx ]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid info pointer in vehicle"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx]->type != 2)
    {
        if (sEntryPointHintIndicies[entryPosition] == -1)
        {
            G_GetHintStringIndex(&sEntryPointHintIndicies[entryPosition],
                                 sEntryPointHintText[entryPosition]);
            if (sEntryPointHintIndicies[entryPosition] < 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 10136;
                AeAssert::gCurrentExpr = "sEntryPointHintIndicies[entryPosition]>= 0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Invalid vehicle entry hint string."))
                    __debugbreak();
            }
        }
        if (sEntryPointSeatAssociation[entryPosition] != 1
            || *BG_GetInfoForWeapon((int)gunnerWeapon)->szUseHintString == 0)
        {
            return sEntryPointHintIndicies[entryPosition];
        }
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    }
    if (HandleDbToEnt(vehicle->scr_vehicle->seats[0].occupant) != nullptr)
        return BG_GetInfoForWeapon((int)gunnerWeapon)->iUseHintStringIndex;
    Entity* mObject = HandleDbToEnt(mEntity);
    return BG_GetInfoForWeapon(mObject->s.weapon)->iUseHintStringIndex;
}

// ea: 0x00480AC0
void Scr_Vehicle_Init(Entity* pSelf, int /*msec*/)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    bool v4 = pSelf->active == 2;
    if (!v4 || s_clientThink != 0)
    {
        float* wheelZPos = scr_vehicle->phys.wheelZPos;
        for (int i = 0; i < 6; ++i)
        {
            int BoneIndex = SV_DObjGetBoneIndex(pSelf, s_wheelTagHashes[i]);
            if (BoneIndex >= 0)
            {
                DObjSkelMat mtx;
                G_DObjGetWorldBoneIndexMatrix(pSelf, BoneIndex, &mtx);
                wheelZPos[i] = mtx.origin[2];
            }
            scr_vehicle->mWheel_ParticleEffectHandle[i].mVal = 0;
        }
        int turretPitch = scr_vehicle->boneIndex.turret;
        if (turretPitch >= 0)
        {
            DObjSkelMat mtx;
            G_DObjGetWorldBoneIndexMatrix(pSelf, turretPitch, &mtx);
            float turretPos[3] = { mtx.origin[0], mtx.origin[1], mtx.origin[2] };
            int turretSpan = scr_vehicle->boneIndex.barrel;
            if (turretSpan >= 0)
            {
                DObjSkelMat mtx2;
                G_DObjGetWorldBoneIndexMatrix(pSelf, turretSpan, &mtx2);
                float spanPos[3] = { mtx2.origin[0], mtx2.origin[1], mtx2.origin[2] };
                scr_vehicle->mUseRadius = VectorDistance(turretPos, spanPos);
            }
        }
        int type = info->type;
        if ((type == 1 || type == 2) && Entity_has_zone_collision(pSelf))
            VEH_GroundPlant(pSelf, 0, 10000);
        float vel[3] = { 0.0f, 0.0f, 0.0f };
        VEH_SetPosition(pSelf, &scr_vehicle->phys.origin, &scr_vehicle->phys.angles, vel);
        scr_vehicle->phys.prevOrigin.v.m128_f32[0] = scr_vehicle->phys.origin.v.m128_f32[0];
        scr_vehicle->phys.prevOrigin.v.m128_f32[1] = scr_vehicle->phys.origin.v.m128_f32[1];
        scr_vehicle->phys.prevOrigin.v.m128_f32[2] = scr_vehicle->phys.origin.v.m128_f32[2];
        scr_vehicle->phys.prevAngles.v.m128_f32[0] = scr_vehicle->phys.angles.v.m128_f32[0];
        scr_vehicle->phys.prevAngles.v.m128_f32[1] = scr_vehicle->phys.angles.v.m128_f32[1];
        scr_vehicle->phys.prevAngles.v.m128_f32[2] = scr_vehicle->phys.angles.v.m128_f32[2];
        math::Position3 angles = scr_vehicle->phys.angles;
        MultiplayerMgr::sInst->ApplyLocalPhysicsToVehicle(pSelf, &scr_vehicle->phys.origin,
                                                          &angles, vel);
        collision_context_t context;
        context.__vftable = nullptr;
        context.pass_entity1.mHandle.mVal = 0;
        context.pass_entity2.mHandle.mVal = 0;
        context.pass_owner1.mHandle.mVal = 0;
        context.pass_owner2.mHandle.mVal = 0;
        context.contentmask = -1;
        G_DoTouchTriggers(pSelf, &pSelf->r.currentOrigin, nullptr, &context);
        pSelf->think = THINK__Scr_Vehicle_Init;
        pSelf->nextthink = level.time + 1;
    }
    else
    {
        pSelf->nextthink = level.time;
    }
}

// ea: 0x0044D370
int VEH_ParseSpecificField(unsigned char* pStruct, const char* pValue, int fieldType)
{
    if (fieldType == 8)
    {
        int v6 = 0;
        while (_stricmp(pValue, s_vehicleTypeNames[v6]) != 0)
        {
            if (++v6 >= 6)
                break;
        }
        if (v6 == 6)
            Com_Error(ERR_DROP, "unknown vehicle type '%s'", pValue);
        *(pStruct + 16) = (unsigned char)v6;
        return 1;
    }
    if (fieldType == 9)
    {
        int v5 = 0;
        while (_stricmp(pValue, s_vehicleSubTypeNames[v5]) != 0)
        {
            if (++v5 >= 9)
                break;
        }
        if (v5 == 9)
        {
            Com_Error(ERR_DROP, "unknown vehicle subtype '%s'", pValue);
            return 1;
        }
        *(pStruct + 17) = (unsigned char)v5;
        return 1;
    }
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
    AeAssert::gCurrentLine = 1002;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored())
    {
        if (AeAssert::Warning(va("Bad vehicle field type %i\n", fieldType)))
            __debugbreak();
    }
    Com_Error(ERR_DROP, "Bad vehicle field type %i", fieldType);
    return 0;
}

// ea: 0x0044D350
void VEH_Strcpy(unsigned char* pMember, const char* pKeyValue, int)
{
    strcpy((char*)pMember, pKeyValue);
}

// ea: 0x0044EC90
void ParseVehicleConfigString(const char* name, const ConfigString* cfgstr)
{
    char buf[256];
    if (s_numVehicleInfos >= 64)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6552;
        AeAssert::gCurrentExpr = "s_numVehicleInfos < 64";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Too many vehicles"))
            __debugbreak();
    }
    if (strlen(name) > 0x20)
    {
        Com_sprintf(buf, 256, "Vehicle name too long(32max): %s", name);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6557;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(buf))
            __debugbreak();
    }
    vehicle_info_t* v3 = (vehicle_info_t*)mem_heap_malloc(16, 0x310);
    s_vehicleInfos[s_numVehicleInfos] = v3;
    memset(v3, 0, sizeof(vehicle_info_t));
    strcpy(v3->name, name);
    if (ParseConfigStringToStruct((unsigned char*)v3, s_vehicleFields, 73, cfgstr,
                                  10, VEH_ParseSpecificField,
                                  VEH_Strcpy) != 0)
    {
        int health = v3->health;
        v3->accel = v3->accel * 17.6f;
        v3->collisionSpeed = v3->collisionSpeed * 17.6f;
        v3->maxSpeed = v3->maxSpeed * 17.6f;
        v3->engineSndSpeed = v3->engineSndSpeed * 17.6f;
        if (health == 0)
            v3->health = 1500;
        if (v3->boundsRadius == 0.0f)
            v3->boundsRadius = 75.0f;
        if (v3->boundsLength == 0.0f)
            v3->boundsLength = 300.0f;
        if (v3->boundsRadius > 100.0f)
        {
            Com_sprintf(buf, 256, "Bounds radius too big for %s (MAX = %i)",
                        name, 100);
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 6594;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert(buf))
                __debugbreak();
        }
        if (v3->boundsHeight == 0.0f)
            v3->boundsHeight = 150.0f;
        v3->mins.v.m128_f32[0] = -v3->boundsRadius;
        v3->mins.v.m128_f32[1] = -v3->boundsRadius;
        v3->mins.v.m128_f32[2] = -v3->boundsHeight * 0.5f;
        v3->maxs.v.m128_f32[0] = v3->boundsRadius;
        v3->maxs.v.m128_f32[1] = v3->boundsRadius;
        v3->maxs.v.m128_f32[2] = v3->boundsHeight * 0.5f;
        if (v3->type == 1 && v3->subtype == 0)
            v3->subtype = 1;
        ++s_numVehicleInfos;
    }
    char v9 = v3->mMantleHintString[0];
    v3->mMantleHintStringIndex = -1;
    if (v9 != 0)
        G_GetHintStringIndex(&v3->mMantleHintStringIndex,
                             v3->mMantleHintString);
}

static const char* s_seatTags[11] = {
    "tag_driver", "tag_gunner", "tag_passenger1", "tag_passenger2",
    "tag_passenger3", "tag_passenger4", "tag_gunner", "tag_passenger1",
    "tag_driver", "tag_gunner", "tag_passenger1",
};

// ea: 0x00490A60
void VEH_LinkPlayer(Entity* ent, Entity* player, int seatIdx, int entryIdx,
                    int fromPos)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    Client* client = player->client;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    player->invulnerability_timeout = 0;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5664;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (HandleDbToEnt(player->r.mOwner) != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5668;
        AeAssert::gCurrentExpr = "*player->r.mOwner == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("VEH_LinkPlayer: Player already has an owner\n"))
            __debugbreak();
    }
    vehicleSeat_t& seat = scr_vehicle->seats[seatIdx];
    if (HandleDbToEnt(seat.occupant) != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 5671;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("VEH_LinkPlayer: Vehicle seat already occupied\n"))
            __debugbreak();
    }
    else
    {
        if (seat.boneIndex < 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 5682;
            AeAssert::gCurrentExpr = "veh->seats[seatIdx].boneIndex >= 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("VEH_LinkPlayer: Trying to use vehicle without a bone\n"))
                __debugbreak();
        }
        DObjSkelMat playerMtx;
        G_DObjGetWorldBoneIndexMatrix(ent, seat.boneIndex, &playerMtx);
        ent->active = 2;
        if (entryIdx != 0
            && scr_vehicle->animMap != nullptr
            && (scr_vehicle->GetEntryRoute(seatIdx, entryIdx - 1,
                                           player->client->ps.ctf_has_flag != 0))
                   >= 0
            && (player->flags |= 0x1000000,
                player->client->mVehicleAnimRoute =
                    scr_vehicle->GetEntryRoute(seatIdx, entryIdx - 1,
                                               player->client->ps.ctf_has_flag != 0),
                scr_vehicle->SetAnimRouteStage(player, ent,
                                               player->client->mVehicleAnimRoute,
                                               0)))
        {
            client->mVehicleAnimStageAnimPlayed = -1;
        }
        else
        {
            if (fromPos != -1
                && scr_vehicle->animMap != nullptr
                && (scr_vehicle->GetSwitchPosRoute(
                        seatIdx, fromPos,
                        player->client->ps.ctf_has_flag != 0))
                       >= 0
                && (player->flags |= 0x1000000,
                    player->client->mVehicleAnimRoute =
                        scr_vehicle->GetSwitchPosRoute(
                            seatIdx, fromPos,
                            player->client->ps.ctf_has_flag != 0),
                    scr_vehicle->SetAnimRouteStage(player, ent,
                                                   player->client->mVehicleAnimRoute,
                                                   0)))
            {
                client->mVehicleAnimStageAnimPlayed = -1;
            }
            else if (G_EntLinkToWithOffset(player, ent, s_seatTags[seatIdx],
                                          vec3_origin, vec3_origin, false) == 0)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                AeAssert::gCurrentLine = 5752;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("Missing  vehicle attach tag"))
                    __debugbreak();
            }
        }
        if (scr_vehicle->mTargetEnt.mHandle.mVal == player->mHandle.mHandle.mVal)
        {
            scr_vehicle->hasTarget = 0;
            scr_vehicle->mTargetEnt.mHandle.mVal = 0;
        }
        scr_vehicle->seats[seatIdx].occupant.mHandle.mVal =
            player->mHandle.mHandle.mVal;
        player->r.mOwner.mHandle.mVal = ent->mHandle.mHandle.mVal;
        if (seatIdx == 0 || HandleDbToEnt(ent->r.mOwner) == nullptr)
        {
            ent->r.mOwner.mHandle.mVal = player->mHandle.mHandle.mVal;
            ent->s.eFlags |= 0x100000;
        }
        client->ps.vehPos = seatIdx;
        client->ps.vehType = info->type;
        client->ps.vehSubType = info->subtype;
        client->ps.eFlags = (client->ps.eFlags & 0xFFBFFFFF) | 0x300000;
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            g_femanager.mDontDrawHud = false;
            cl_aADS[EntityManager::sInst->GetPlayerIndex(player)] = 1;
        }
        client->ps.mViewLockedEntity.mHandle.mVal = ent->mHandle.mHandle.mVal;
        G_DObjUpdate(player, false);
        Scr_Notify(ent, hash_const.player_on_vehicle, 0);
    }
}

// Local Camera view (core.o Camera; 496-byte instances)
struct LocalCamera {
    uint8_t _pad[0x40];
    math::Position3 mPrevAngles;   // +0x40
    uint8_t _pad50[0x194 - 0x50];
    int     mVehicleCamMode;       // +0x194
};
static LocalCamera* CameraAt(int idx)
{
    return (LocalCamera*)((char*)&gCamera + idx * 496);
}

// ea: 0x0048CE20
bool scr_vehicle_t::SetAnimRouteStage(Entity* player, Entity* ent,
                                      int routeIdx, int stageIdx)
{
    vehicle_info_t* info = s_vehicleInfos[this->infoIdx];
    Client* client = player->client;
    if (client == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9459;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    vehicleAnimMap_t* animMap = vehicleAnimMaps[info->type];
    if (animMap == nullptr)
        return false;
    vehicleAnimRoute_t* route = &animMap->routes[routeIdx];
    if (stageIdx >= route->numStages)
    {
        if (route->vehPosDest >= 8 || stageIdx <= 0)
            return false;
        vehicleAnimMap_t* v10 = this->animMap;
        int v11 = v10->routes[routeIdx].stages[stageIdx - 1];
        vehicleAnimStage_t* stages = v10->stages;
        v11 *= 16;
        int v14 = *(int*)((char*)&stages->endTag + v11);
        vehicleAnimStage_t* v15 = (vehicleAnimStage_t*)((char*)stages + v11);
        int BoneIndex = SV_DObjGetBoneIndex(ent, animMap->tags[v14].hash);
        DObjSkelMat tagMtx;
        G_DObjGetWorldBoneIndexMatrix(ent, BoneIndex, &tagMtx);
        SetClientOrigin(player, tagMtx.origin);
        float angles[3];
        AxisToAngles((const float(*)[3])tagMtx.axis, angles);
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            int vehPosDest = route->vehPosDest;
            angles[2] = 0.0f;
            if (vehPosDest == 0 && info->type != 2)
                angles[1] = 0.0f;
            else if (vehPosDest != 1)
                angles[0] = 0.0f;
            else
                angles[0] = 0.0f;
            angles[0] = AngleNormalize180(angles[0]);
            SetClientViewAngle(player, angles);
        }
        if (G_EntLinkToWithOffsetHash(
                player, ent, animMap->tags[*(int*)((char*)v15 + 4)].hash,
                vec3_origin, vec3_origin, true) == 0)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 9497;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        return false;
    }
    vehicleAnimMap_t* v19 = this->animMap;
    int v20 = v19->routes[routeIdx].stages[stageIdx];
    vehicleAnimStage_t* v21 = v19->stages;
    v20 *= 16;
    int v22 = *(int*)((char*)&v21->startTag + v20);
    vehicleAnimStage_t* v23 = (vehicleAnimStage_t*)((char*)v21 + v20);
    if (G_EntLinkToWithOffsetHash(
            player, ent, animMap->tags[v22].hash, vec3_origin, vec3_origin,
            true) == 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9506;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (player->client->ps.vehPos == 7)
    {
        sentient_s* sentient = player->sentient;
        if (sentient != nullptr)
        {
            char eventStr[64];
            if (sentient->eTeam == TEAM_ALLIES)
                sprintf(eventStr, "TANK_ALLIES_MANTLE_%i", stageIdx);
            else
                sprintf(eventStr, "TANK_AXIS_MANTLE_%i", stageIdx);
            PostEffectEventScriptCall(player, eventStr, false, PAK_ID_INVALID,
                                     false);
        }
    }
    client->mVehicleAnimRoute = routeIdx;
    client->mVehicleAnimMoving = true;
    client->mVehicleAnimStage = stageIdx;
    client->mVehicleAnimStageChangeTime = level.time;
    client->mFakerootOriginMatrixValid = false;
    if (stageIdx == 0)
    {
        client->mVehicleAnimStageAnim = stageIdx;
        client->mVehicleAnimStageAnimPlayed = -1;
        client->mVehicleAnimGetOut = stageIdx != 0;
        client->mVehicleAnimDisableCamera = stageIdx != 0;
        if (EntityManager::sInst->IsLocalPlayer(player))
        {
            int flags = route->flags;
            if ((flags & 1) != 0
                && ((flags & 2) == 0
                    || CameraAt(EntityManager::sInst->GetPlayerIndex(player))
                               ->mVehicleCamMode == 1 /* VEH_MODE_FIRSTPERSON */))
                client->mVehicleAnimDisableCamera = true;
        }
    }
    client->mVehicleAnimMoving = true;
    client->mVehicleAnimAngleOffset[1] = 0.0f;
    client->mVehicleAnimAngleOffset[2] = 0.0f;
    client->mVehicleAnimAngleOffset[0] = 0.0f;
    client->mVehicleAnimFirstPersonCam = v23->flags & 1;
    if ((v23->flags & 0x20) != 0)
        this->noEntryTime = level.time + 2000;
    if ((v23->flags & 0x40) != 0)
        this->forceGunnerCrouchTime = level.time + 2500;
    if (v23->flags < 0
        && HandleDbToEnt(this->mMantleEntity) != nullptr
        && HandleDbToEnt(this->mMantleEntity) != player)
        this->noExitTime = level.time + 2500;
    return true;
}

// ea: 0x0046C9E0
void VEH_InvalidateCaches(void)
{
    if (level.vehicles != nullptr)
    {
        for (int i = 0; i < level.MaxVehicles; ++i)
        {
            Entity* mObject = HandleDbToEnt(level.vehicles[i].mEntity);
            if (mObject != nullptr && mObject->proximity_data != nullptr)
            {
                mObject->proximity_data->lo.v.m128_f32[0] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[1] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[2] = 3.4028235e38f;
                mObject->proximity_data->lo.v.m128_f32[3] = 3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[0] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[1] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[2] = -3.4028235e38f;
                mObject->proximity_data->hi.v.m128_f32[3] = -3.4028235e38f;
            }
        }
    }
}

// ea: 0x0046E0B0
bool G_IsPlayerInVehicle(Entity* player)
{
    Client* client = player->client;
    if (client == nullptr)
        return false;
    int eFlags = client->ps.eFlags;
    if ((0x100000 & eFlags) == 0 || (0x400000 & eFlags) != 0)
        return false;
    Entity* v3 = HandleDbToEnt(player->r.mOwner);
    if (v3 == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 7302;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
        return false;
    }
    return v3->scr_vehicle != nullptr;
}

// ea: 0x0045F210
vehicle_node_t* SP_create_info_vehicle_node(void)
{
    vehicle_node_t* v0 = g_vehicleNodeManager.AllocNode();
    v0->mName.clear();
    v0->mTarget.clear();
    int v1 = v0->nextIdx;
    v0->speed = -1.0f;
    v0->lookAhead = -1.0f;
    v0->origin[0] = 0.0f;
    v0->origin[1] = 0.0f;
    v0->dir[0] = 0.0f;
    v0->dir[1] = 0.0f;
    v1 &= 0xCFFFFFFF;
    v0->nextIdx = v1;
    v0->angles[0] = s_invalidAngles[0];
    v0->angles[1] = dword_DD7418;
    float v2 = dword_DD741C;
    v0->nextIdx = v1 | 0xFFFFFFF;
    v0->angles[2] = v2;
    v0->length = 0.0f;
    ++s_numNodes;
    return v0;
}

// ea: 0x0044F3D0
float Scr_Vehicle_CalcSpeed(const scr_vehicle_t* pVehicle)
{
    if (pVehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 8963;
        AeAssert::gCurrentExpr = "pVehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vehicle"))
            __debugbreak();
        return 0.0f;
    }
    rb_vehicle* mRBVeh = (rb_vehicle*)pVehicle->mRBVeh;
    if (mRBVeh == nullptr)
        return 0.0f;
    float v4[3];
    rb_vehicle_get_velocity(mRBVeh, v4);
    return sqrtf(v4[0] * v4[0] + v4[1] * v4[1] + v4[2] * v4[2]);
}

// ea: 0x0046F3C0
void scr_vehicle_t::AssignPhysics(Entity* player)
{
    if (mRBVeh != nullptr)
    {
        if (EntityManager::sInst->IsLocalPlayer(player)
            && mPhysicsOwner.mHandle.mVal != player->mHandle.mHandle.mVal)
        {
            if (IsPhysicsPaused())
                rb_vehicle_unpause_physics((rb_vehicle*)mRBVeh);
            rb_vehicle_update_from_network((rb_vehicle*)mRBVeh,
                                           &phys.origin, &phys.angles, &phys.vel,
                                           &phys.rotVel);
        }
    }
    mPhysicsOwner.mHandle.mVal = player->mHandle.mHandle.mVal;
}

// ea: 0x0044F580
int scr_vehicle_t::GetEntryRoute(int seatIdx, int entryIdx, bool hasFlag)
{
    vehicleAnimMap_t* animMap = this->animMap;
    if (animMap == nullptr)
        return -1;
    int numRoutes = animMap->numRoutes;
    int result = 0;
    if (numRoutes > 0)
    {
        do
        {
            if (animMap->routes[result].vehPosSrc == -1
                && animMap->routes[result].vehPosDest == seatIdx)
            {
                if (this->animMap->stages[this->animMap->routes[result].stages[0]].startTag
                        == this->animMap->entryTags[entryIdx]
                    && ((animMap->routes[result].flags & 4) == 0 || hasFlag))
                {
                    return result;
                }
                numRoutes = animMap->numRoutes;
            }
            ++result;
        } while (result < numRoutes);
    }
    return -1;
}

// ea: 0x004639D0
void G_ParseScrVehicleInfo(void)
{
    ConfigStringManager* v0 = ConfigStringManager::sInst;
    s_numVehicleInfos = 0;
    TPakId v1 = CurPakId();
    v0->CallbackSearch(v1, "VEHICLEFILE", ParseVehicleConfigString);
    ConfigStringManager* v2 = ConfigStringManager::sInst;
    TPakId v3 = CurPakId();
    v2->CallbackSearch(v3, "VEHICLEPHYSICSFILE", ParseVehiclePhysicsConfigString);
}

// ea: 0x0044D2E0
vehicle_info_t* VEH_GetInfo(int idx)
{
    return s_vehicleInfos[idx];
}

// ea: 0x0044F290
int G_GetNonPVSTankInfo(float* /*origin*/, DbLinkedHandle<EntityHandleDb, Entity> /*ent*/)
{
    return 0;
}

// ea: 0x0044F470
void G_FreeVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F480
int G_RequestVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, HashString /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F490
int G_RequestVehicleBestSeat(Entity* /*ent*/, Entity* /*veh*/, bool /*bForce*/,
                             bool /*bPassenger*/, bool /*bCanDrive*/)
{
    return -1;
}

// ea: 0x0044F4A0
int G_StealVehicleSeat(Entity* /*ent*/, Entity* /*veh*/, int /*seat*/, bool /*bForce*/)
{
    return -1;
}

// ea: 0x0044F4B0
void Scr_Vehicle_OccupantStartEntering(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                       int /*seat*/)
{
    ;
}

// ea: 0x0044F4C0
void Scr_Vehicle_OccupantIsSeat(scr_vehicle_t* /*veh*/, const Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x0044F4D0
void Scr_Vehicle_OccupantStartExiting(scr_vehicle_t* /*veh*/, const Entity* /*ent*/,
                                      int /*seat*/)
{
    ;
}

// ea: 0x0044F4E0
void Scr_Vehicle_OccupantIsOut(scr_vehicle_t* /*veh*/, Entity* /*ent*/, int /*seat*/)
{
    ;
}

// ea: 0x004517F0
vehicle_node_t* GetVehicleNode(int idx)
{
    return s_nodes[idx];
}

// ea: 0x004523E0
void G_InitVehiclePaths(void)
{
    s_numNodes = 0;
}

// ea: 0x0044D2B0
void SP_script_prop_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 17;
}

// ea: 0x0044D2F0
int Is4WheeledVehicle(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle == nullptr)
        return 0;
    vehicle_info_t* v2 = s_vehicleInfos[scr_vehicle->infoIdx];
    if (v2->type != 1)
        return 0;
    return 1;
}

// ea: 0x0044F1D0
vehicle_info_t* G_GetVehicleInfo(scr_vehicle_t* veh)
{
    if (veh != nullptr)
        return s_vehicleInfos[veh->infoIdx];
    return nullptr;
}

// ea: 0x0044F4F0
vehicleAnimStage_t* scr_vehicle_t::GetRouteStage(int routeIdx, int stage)
{
    return &animMap->stages[animMap->routes[routeIdx].stages[stage]];
}

// ea: 0x0044F560
float scr_vehicle_t::GetAnimSpeedScale(Client* client)
{
    return animMap->routes[client->mVehicleAnimRoute].animSpeedScale;
}

// ea: 0x0044F680
float scr_vehicle_t::GetThrottle()
{
    rb_vehicle* mRBVeh = (rb_vehicle*)this->mRBVeh;
    if (mRBVeh != nullptr)
        return mRBVeh->m_throttle;
    return 0.0f;
}

// ea: 0x0044F790
void SP_script_vehicle_collmap(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.eType = 16;
}

// ea: 0x0044F7B0
void vehicle_FreeDynamicBuffers(void)
{
    if (s_vehicles != nullptr)
    {
        mem_heap_free(s_vehicles);
        s_vehicles = nullptr;
        level.MaxVehicles = 0;
    }
}

// ea: 0x0045EA80
void scr_vehicle_t::ReleasePhysics(Entity* player)
{
    if (mPhysicsOwner.mHandle.mVal == player->mHandle.mHandle.mVal)
        mPhysicsOwner.mHandle.mVal = 0;
}

// ea: 0x00480860
void VEH_PlayerInteractionExit(void)
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    VEH_UnlinkPlayer(Player, true);
}

// ea: 0x0045E290
void G_UpdateVehicleTags(Entity* ent)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6984;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->scr_vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 6985;
        AeAssert::gCurrentExpr = "ent->scr_vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    scr_vehicle->boneIndex.leader = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_leader"));
    scr_vehicle->boneIndex.player = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_driver"));
    scr_vehicle->boneIndex.detach = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_detach"));
    scr_vehicle->boneIndex.popout = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_popout"));
    scr_vehicle->boneIndex.body = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_body"));
    scr_vehicle->boneIndex.turret = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_turret"));
    scr_vehicle->boneIndex.barrel = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_barrel"));
    scr_vehicle->boneIndex.coax = SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_guncoax"));
    scr_vehicle->boneIndex.gunner_barrel =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_barrel"));
    scr_vehicle->boneIndex.gunner_player =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_player"));
    scr_vehicle->boneIndex.gunner_flash =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_gunner_flash"));
    scr_vehicle->boneIndex.steering_wheel =
        SV_DObjGetBoneIndex(ent, HashString::CalcHash("tag_steeringwheel"));
    for (int i = 0; i < 4; ++i)
        scr_vehicle->boneIndex.flash[i] = SV_DObjGetBoneIndex(ent, s_flashTagHashes[i]);
    for (int i = 0; i < 6; ++i)
        scr_vehicle->boneIndex.wheel[i] = SV_DObjGetBoneIndex(ent, s_wheelTagHashes[i]);
    float maxDist = 0.0f;
    if (scr_vehicle->animMap == nullptr)
    {
        for (int i = 0; i < 6; ++i)
        {
            int v24 = SV_DObjGetBoneIndex(ent, s_entryPointTagHashes[i]);
            scr_vehicle->boneIndex.entryPoint[i] = v24;
            if (v24 >= 0)
            {
                scr_vehicle->mHasEntryPoints = true;
                G_DObjCalcBone(ent, v24);
                const DObjSkelMat* MatrixArray = SV_DObjGetMatrixArray(ent);
                const float* origin = MatrixArray[v24].origin;
                float dist = sqrt(origin[0] * origin[0] + origin[1] * origin[1]
                                  + origin[2] * origin[2]);
                if (dist > maxDist)
                    maxDist = dist;
            }
        }
    }
    else
    {
        vehicleAnimMap_t* animMap = scr_vehicle->animMap;
        if (animMap->numEntryTags >= 6)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
            AeAssert::gCurrentLine = 7041;
            AeAssert::gCurrentExpr = "veh->animMap->numEntryTags < NUM_ENTRY_POINTS";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        for (int i = 0; i < animMap->numEntryTags; ++i)
        {
            int v30 = SV_DObjGetBoneIndex(ent,
                                          animMap->tags[animMap->entryTags[i]].hash);
            scr_vehicle->boneIndex.entryPoint[i] = v30;
            if (v30 >= 0)
            {
                scr_vehicle->mHasEntryPoints = true;
                G_DObjCalcBone(ent, v30);
                const DObjSkelMat* v32 = SV_DObjGetMatrixArray(ent);
                const float* origin = v32[v30].origin;
                float v36 = sqrt(origin[0] * origin[0] + origin[1] * origin[1]
                                 + origin[2] * origin[2]);
                if (v36 > maxDist)
                    maxDist = v36;
            }
        }
    }
    if (maxDist > 0.0f)
        scr_vehicle->mUseRadius = (maxDist + 50.0f);
}

int scr_vehicle_t::sDebugMantle;  // ?sDebugMantle@scr_vehicle_t@@2HA

// ea: 0x0046F680
bool scr_vehicle_t::CanUseVehicle(Entity* player, float* distToUsePoint,
                                  int* entryPoint)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9909;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid pointer"))
            __debugbreak();
    }
    Entity* vehicle = HandleDbToEnt(mEntity);
    if (vehicle == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 9914;
        AeAssert::gCurrentExpr = "vehicle";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid pointer."))
            __debugbreak();
    }
    float playerPos[4];
    playerPos[0] = player->r.currentOrigin.v.m128_f32[0];
    playerPos[1] = player->r.currentOrigin.v.m128_f32[1];
    playerPos[2] = player->r.currentOrigin.v.m128_f32[2];
    playerPos[3] = player->r.currentOrigin.v.m128_f32[3];
    float vehCenter[3];
    vehCenter[0] = (vehicle->r.absmax.v.m128_f32[0] + vehicle->r.absmin.v.m128_f32[0]) * 0.5f;
    vehCenter[1] = (vehicle->r.absmax.v.m128_f32[1] + vehicle->r.absmin.v.m128_f32[1]) * 0.5f;
    vehCenter[2] = (vehicle->r.absmax.v.m128_f32[2] + vehicle->r.absmin.v.m128_f32[2]) * 0.5f;
    float dx = playerPos[0] - vehCenter[0];
    float dy = playerPos[1] - vehCenter[1];
    float dz = playerPos[2] - vehCenter[2];
    float dist2 = dx * dx + dy * dy + dz * dz;
    if (dist2 > (mUseRadius * mUseRadius))
        return false;
    if (!mHasEntryPoints)
    {
        *distToUsePoint = sqrt(dist2);
        return true;
    }
    int numEntryTags = animMap != nullptr ? animMap->numEntryTags : 6;
    float bestDist = 2500.0f;
    int bestPoint = -1;
    for (int i = 0; i < numEntryTags; ++i)
    {
        int bone = boneIndex.entryPoint[i];
        if (bone < 0)
            continue;
        Client* client = player->client;
        if (client == nullptr
            || client->ps.ctf_has_flag == 0
            || s_vehicleInfos[infoIdx]->type != 2
                && (int)(sEntryPointSeatAssociation[i]) >= 2
                && (int)(sEntryPointSeatAssociation[i]) <= 5)
        {
            if (s_vehicleInfos[infoIdx]->type == 2
                || HandleDbToEnt(seats[sEntryPointSeatAssociation[i]].occupant) == nullptr)
            {
                DObjSkelMat tagMat;
                G_DObjGetWorldBoneIndexMatrix(vehicle, bone, &tagMat);
                float ex = playerPos[0] - tagMat.origin[0];
                float ey = playerPos[1] - tagMat.origin[1];
                float ez = playerPos[2] - tagMat.origin[2];
                float d = ex * ex + ey * ey + ez * ez;
                if (bestDist > d)
                {
                    bestDist = d;
                    bestPoint = i;
                }
            }
        }
    }
    if (bestPoint < 0)
        return false;
    *distToUsePoint = sqrt(bestDist);
    *entryPoint = bestPoint;
    return true;
}

// ea: 0x0046FA60
bool scr_vehicle_t::CanMantleVehicle(Entity* player)
{
    if (player == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10032;
        AeAssert::gCurrentExpr = "player";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid player"))
            __debugbreak();
    }
    if (player->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10033;
        AeAssert::gCurrentExpr = "player->sentient";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Player does not have a valid sentient"))
            __debugbreak();
    }
    if (s_vehicleInfos[infoIdx]->type != 2)
        return false;
    if (mMantleTime != 0)
        return false;
    if (HandleDbToEnt(seats[7].occupant) != nullptr)
        return false;
    Entity* vehEnt = HandleDbToEnt(mEntity);
    if (vehEnt == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10040;
        AeAssert::gCurrentExpr = "*mEntity";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Invalid entity handle in the vehicle"))
            __debugbreak();
    }
    if (HandleDbToEnt(seats[1].occupant) != nullptr
        || HandleDbToEnt(seats[6].occupant) != nullptr)
        return false;
    Entity* driver = HandleDbToEnt(vehEnt->r.mOwner);
    if (driver != nullptr && driver->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 10048;
        AeAssert::gCurrentExpr = "!driver || driver->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Driver is not a sentient"))
            __debugbreak();
    }
    Entity* v7 = player;
    if (IsLocalPlayer(player))
    {
        int v8 = player->client->ps.weaponslots[4];
        weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(v8);
        if (InfoForWeapon == nullptr
            || InfoForWeapon->bCanMantle == 0
            || player->client->ps.ammoclip[BG_ClipForWeapon(v8)] <= 0)
            return false;
        v7 = player;
    }
    Client* client = v7->client;
    if (client != nullptr && client->ps.ctf_has_flag != 0)
        return false;
    Entity* phyOwner = HandleDbToEnt(mPhysicsOwner);
    if (phyOwner != nullptr
        && IsLocalPlayer(phyOwner)
        && sqrt(phys.vel.v.m128_f32[0] * phys.vel.v.m128_f32[0]
                + phys.vel.v.m128_f32[1] * phys.vel.v.m128_f32[1]
                + phys.vel.v.m128_f32[2] * phys.vel.v.m128_f32[2])
               > ((rb_vehicle*)mRBVeh)->m_parameter->m_speed_max - 5.0f)
        return false;
    if (sDebugMantle != 0
        || driver != nullptr
            && (!cgGlobal.teamGame
                || driver->sentient->eTeam != player->sentient->eTeam))
        return true;
    return false;
}

// ea: 0x0044F7E0
float Scr_Vehicle_DamageScale(Entity* pSelf, Entity* pAttacker,
                              Entity* pInflictor, const float* point, int mod)
{
    scr_vehicle_t* scr_vehicle = pSelf->scr_vehicle;
    vehicle_info_t* v6 = s_vehicleInfos[scr_vehicle->infoIdx];
    auto* p_phys = &scr_vehicle->phys;
    float width = cos(0.3490658700466156f);
    float scalar_best_side;
    switch (mod)
    {
    case 4: case 6: case 10: case 18:
        scalar_best_side = 1.0f;
        break;
    default:
        scalar_best_side = 0.0f;
        break;
    }
    float bulletDamage;
    switch (mod)
    {
    case 1: case 2:
        bulletDamage = v6->bulletDamage;
        break;
    case 3: case 4:
        bulletDamage = v6->grenadeDamage;
        break;
    case 5: case 6:
        bulletDamage = v6->mineDamage;
        break;
    case 9: case 10: case 17: case 18:
        bulletDamage = v6->projectileDamage;
        break;
    default:
        bulletDamage = 1.0f;
        break;
    }
    float scale = bulletDamage;
    float axis[3][3];
    AnglesToAxis(&scr_vehicle->phys.angles, axis);
    float vdir[3];
    vdir[0] = point[0] - p_phys->origin.v.m128_f32[0];
    vdir[1] = point[1] - p_phys->origin.v.m128_f32[1];
    vdir[2] = 0.0f;
    VectorNormalize(vdir);
    float dotAxis = (axis[0][0] * vdir[0]) + (axis[0][1] * vdir[1]) + (axis[0][2] * vdir[2]);
    float dotOther = (axis[1][0] * vdir[0]) + (axis[1][1] * vdir[1]) + (axis[1][2] * vdir[2]);
    float bestDot = dotAxis;
    int v11 = 0;
    if (fabs(dotOther) > fabs(dotAxis))
    {
        bestDot = dotOther;
        v11 = 1;
    }
    if (scalar_best_side == 0.0f)
    {
        if (v11 != 0)
            return scale * 1.5f;
        if (bestDot >= 0.0f)
            return scale * 1.0f;
        if (width > -bestDot)
            return scale * 1.5f;
        return scale + scale;
    }
    scalar_best_side = 1.0f;
    float scalar_worst_side = 1.0f;
    width = pSelf->r.maxs.v.m128_f32[1] * 0.80000001f;
    float height = pSelf->r.maxs.v.m128_f32[2] * 0.5f;
    float dx = point[0] - p_phys->origin.v.m128_f32[0];
    float dy = point[1] - p_phys->origin.v.m128_f32[1];
    float dz = point[2] - p_phys->origin.v.m128_f32[2];
    float dist = dx * dx + dy * dy + dz * dz;
    if (mod == 4)
    {
        if ((p_phys->origin.v.m128_f32[2] + height) > (point[2] - 10.0f)
            && (width * width) > dist)
            return scale + scale;
        return scale;
    }
    float v14;
    if (v11 != 0)
    {
        scalar_best_side = 1.5f;
        float frontDot = (axis[0][0] * vdir[0]) + (axis[0][1] * vdir[1]) + (axis[0][2] * vdir[2]);
        v14 = frontDot >= 0.0f ? 1.0f : 2.0f;
    }
    else
    {
        v14 = 2.0f;
        if (bestDot >= 0.0f)
            v14 = 1.0f;
        scalar_best_side = v14;
        v14 = 1.5f;
    }
    float v16 = fabs(bestDot);
    return ((1.0f - v16) * v14 + scalar_best_side * v16) * scale * 0.69999999f;
}

static int last_use;  // @ 0xEF5934 (g_scr_vehicle.cpp local)

// ea: 0x00480DA0
void Scr_Vehicle_Use(Entity* pEnt, Entity* pOther)
{
    Client* v21 = pOther->client;
    if (v21 == nullptr)
        return;
    if (last_use != 0 && last_use > level.time - 1000 && last_use <= level.time)
        return;
    last_use = level.time;
    if ((0x100000 & v21->ps.eFlags) == 0)
    {
        if (IsVehFlipped(pEnt))
        {
            math::Position3 hitp;
            hitp.v = pOther->r.currentOrigin.v;
            math::Dir3 hitd;
            hitd.v = _mm_setzero_ps();
            hitd.v.m128_f32[2] = 1.0f;
            ApplyPhysics(pEnt, &hitp, &hitd, 70.0f, false, HITLOC_TORSO_UPR);
        }
        else if (v21->ps.ctf_has_flag == 0
                 || s_vehicleInfos[pEnt->scr_vehicle->infoIdx]->type != 2)
        {
            if (pEnt->scr_vehicle->CanMantleVehicle(pOther))
            {
                MultiplayerMgr::sInst->AttemptToGetInVehicle(pEnt, pOther, 7,
                                                             v21->mVehicleAnimRoute);
            }
            else
            {
                scr_vehicle_t* scr_vehicle = pEnt->scr_vehicle;
                if (scr_vehicle == nullptr)
                {
                    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                    AeAssert::gCurrentLine = 8906;
                    AeAssert::gCurrentExpr = "pEnt->scr_vehicle";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("Entity not a vehicle"))
                        __debugbreak();
                }
                if (s_vehicleInfos[scr_vehicle->infoIdx]->type == 2)
                {
                    int v17 = 0;
                    while (HandleDbToEnt(scr_vehicle->seats[v17].occupant) != nullptr)
                    {
                        ++v17;
                        if (v17 > 10)
                            return;
                    }
                    MultiplayerMgr::sInst->AttemptToGetInVehicle(pEnt, pOther, v17,
                                                                 v21->mVehicleAnimRoute);
                }
                else
                {
                    float distToUsePoint;
                    int entryPoint;
                    if (!scr_vehicle->CanUseVehicle(pOther, &distToUsePoint,
                                                    &entryPoint))
                    {
                        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
                        AeAssert::gCurrentLine = 8934;
                        AeAssert::gCurrentExpr = "canUseVehicle";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("Player not in an entry point"))
                            __debugbreak();
                    }
                    if (pOther->client->ps.ctf_has_flag == 0
                        || sEntryPointSeatAssociation[entryPoint] == 2)
                    {
                        int seatIdx = sEntryPointSeatAssociation[entryPoint];
                        if (HandleDbToEnt(pEnt->scr_vehicle->seats[seatIdx].occupant)
                            == nullptr)
                            MultiplayerMgr::sInst->AttemptToGetInVehicle(
                                pEnt, pOther, seatIdx, v21->mVehicleAnimRoute);
                    }
                }
            }
        }
    }
    else
    {
        scr_vehicle_t* v4 = pEnt->scr_vehicle;
        if (v4->noExitTime > level.time)
            return;
        if (v21->ps.vehType == 2 && v21->ps.vehPos == 0
            && HandleDbToEnt(v4->seats[7].occupant) != nullptr)
        {
            *(int*)((char*)v21 + 0xB78) = level.time;
        }
        else if (v21->ps.vehPos == 6)
        {
            MultiplayerMgr::sInst->AttemptVehicleSeatChange(pEnt, pOther, 1);
        }
        else if (IsPlayerFullySeatedInVehicle(pOther))
        {
            scr_vehicle_t* v6 = pEnt->scr_vehicle;
            vehicleAnimMap_t* animMap = v6->animMap;
            if (animMap != nullptr && animMap->exitMap != nullptr)
            {
                Client* client = pOther->client;
                int vehPos = client->ps.vehPos;
                int v11 = animMap->exitMap[vehPos];
                if (v11 == -1
                    || v6->GetSwitchPosRoute(v11, vehPos,
                                             client->ps.ctf_has_flag != 0) < 0)
                {
                    tlPrintf("=======================================GetOutOfVehicle due to a use during an anim\n");
                    MultiplayerMgr::sInst->GetOutOfVehicle(
                        pEnt, pOther->client->ps.vehPos);
                }
                else
                {
                    MultiplayerMgr::sInst->AttemptVehicleSeatChange(pEnt, pOther,
                                                                    v11);
                }
            }
            else
            {
                tlPrintf("=======================================GetOutOfVehicle due to a use\n");
                MultiplayerMgr::sInst->GetOutOfVehicle(pEnt,
                                                       pOther->client->ps.vehPos);
            }
        }
        else
        {
            Client* v5 = pOther->client;
            if (v5->ps.vehType == 2 && v5->mVehicleAnimStage <= 4)
                v5->mVehicleAnimGetOut = true;
        }
    }
}

static int s_newDebugLineLocal;  // @ 0xDD7410
static float s_start[3];   // @ 0xEF392C
static float s_end[3];     // @ 0xEF393C
static float s_dir[3];     // @ 0xEF394C

// ea: 0x0045EC30 (file-local)
static void VP_AddDebugLine(const float* start, const float* end, int forceDraw)
{
    float dir[3];
    dir[0] = end[0] - start[0];
    dir[1] = end[1] - start[1];
    dir[2] = end[2] - start[2];
    VectorNormalize(dir);
    if (s_newDebugLineLocal != 0)
    {
        s_newDebugLineLocal = 0;
    }
    else
    {
        if ((s_dir[0] * dir[0]) + (s_dir[1] * dir[1]) + (s_dir[2] * dir[2])
                >= 0.99989998f
            && forceDraw == 0)
        {
            s_end[0] = end[0];
            s_end[1] = end[1];
            s_end[2] = end[2];
            return;
        }
        float k_lineColor[4] = {1.0f, 0.0f, 0.0f, 1.0f};
        CL_AddDebugLine(s_start, s_end, k_lineColor, 1, 0, 1, 0);
    }
    s_start[0] = start[0];
    s_start[1] = start[1];
    s_start[2] = start[2];
    s_end[0] = end[0];
    s_end[1] = end[1];
    s_end[2] = end[2];
    s_dir[0] = dir[0];
    s_dir[1] = dir[1];
    s_dir[2] = dir[2];
}

// ea: 0x00464710
void VP_DrawPath(const vehicle_pathpos_t* vpp)
{
    vehicle_pathpos_t prevVPP = *vpp;
    vehicle_pathpos_t nextVPP = *vpp;
    s_newDebugLineLocal = 1;
    int v3 = 0;
    int loopNode = -1;
    int count = 0;
    while (1)
    {
        count = count + 1;
        if (count > 50000)
            break;
        if (prevVPP.nodeIdx != vpp->nodeIdx)
            loopNode = vpp->nodeIdx;
        prevVPP = nextVPP;
        int updated = G_VehUpdatePathPos(nullptr, &nextVPP, false,
                                         ServerTime::sInst.mTickMSec, loopNode);
        if (nextVPP.endOfPath != 0 || updated != 0)
            v3 = 1;
        VP_AddDebugLine(prevVPP.origin, nextVPP.origin, v3);
        if (v3 != 0)
            goto draw_boxes;
    }
    Com_Printf("WARNING: Invalid vehicle path.  Possible infinite loop\n");
draw_boxes:
    int nodeIdx = vpp->nodeIdx;
    vehicle_node_t* v7 = s_nodes[nodeIdx];
    float k_boxColor1[4] = {0.0f, 1.0f, 0.0f, 1.0f};
    float k_boxColor2[4] = {0.0f, 0.0f, 1.0f, 1.0f};
    int v11 = 0;
    for (int v8 = 0; v8 < s_numNodes; v7 = s_nodes[v11])
    {
        vehicle_node_t* v9 = s_nodes[nodeIdx];
        float mins[3] = {v7->origin[0] + 4.0f, v7->origin[1] + 4.0f,
                         v7->origin[2] + 4.0f};
        float maxs[3] = {v7->origin[0] - 4.0f, v7->origin[1] - 4.0f,
                         v7->origin[2] - 4.0f};
        ++v8;
        const float* v10 = v7 != v9 ? k_boxColor2 : k_boxColor1;
        G_DebugBox(mins, maxs, v10, 1, 0, 0);
        int v11 = (v7->nextIdx << 18) >> 18;
        if (v11 < 0)
            break;
        if (v11 == nodeIdx)
            break;
    }
}

// ea: 0x00464980
void G_DrawVehiclePaths()
{
    vehicle_pathpos_t vpp;
    memset(&vpp, 0, sizeof(vpp));
    if (g_vehicleDrawPath.string[0] == 0 || g_vehicleDrawPath.string[0] == '0')
        return;
    int16_t v0 = 0;
    if (s_numNodes > 0)
    {
        int v1 = 0;
        while (1)
        {
            Broc::string::Block* mBlock = s_nodes[v1]->mName.mBlock;
            const char* v3 = mBlock != nullptr ? (const char*)&mBlock[1]
                                               : defaultFileName;
            if (_stricmp(v3, g_vehicleDrawPath.string) == 0)
                break;
            v1 = ++v0;
            if (v0 >= s_numNodes)
                goto done;
        }
        vpp.switchNode[0].mName.clear();
        vpp.switchNode[0].mTarget.clear();
        vpp.switchNode[1].mName.clear();
        vpp.switchNode[1].mTarget.clear();
        G_VehSetUpPathPos(&vpp, v0);
        VP_DrawPath(&vpp);
    }
done:
    ;
}

static float rate = 1.0f;          // @ 0xDD7FDC (g_scr_vehicle.cpp local)
static float s_sndLerpMin = 0.1f;  // @ 0xDD7FE0 (g_scr_vehicle.cpp local)
static scr_vehicle_t s_backup;     // @ 0xEE60A0 (bss, g_scr_vehicle.cpp local)
static float intensity_scale = 500.0f;  // @ 0xDD7F48
static float hit_offset = 30.0f;        // @ 0xDD7F4C

// ea: 0x0044DD00
void VEH_Backup(Entity* ent)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    scr_vehicle->phys.prevOrigin.v.m128_f32[0] = ent->r.currentOrigin.v.m128_f32[0];
    scr_vehicle->phys.prevOrigin.v.m128_f32[1] = ent->r.currentOrigin.v.m128_f32[1];
    scr_vehicle->phys.prevOrigin.v.m128_f32[2] = ent->r.currentOrigin.v.m128_f32[2];
    scr_vehicle->phys.prevAngles.v.m128_f32[0] = ent->r.currentAngles.v.m128_f32[0];
    scr_vehicle->phys.prevAngles.v.m128_f32[1] = ent->r.currentAngles.v.m128_f32[1];
    scr_vehicle->phys.prevAngles.v.m128_f32[2] = ent->r.currentAngles.v.m128_f32[2];
    s_backup = *scr_vehicle;
}

// ea: 0x0044D8F0
void VEH_JoltBody(Entity* ent, const math::Position3* dir, float intensity,
                  float speedFrac, float decel)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    if (scr_vehicle != nullptr)
    {
        rb_vehicle* mRBVeh = (rb_vehicle*)scr_vehicle->mRBVeh;
        vehicle_info_t* v8 = s_vehicleInfos[scr_vehicle->infoIdx];
        if (mRBVeh != nullptr && v8->type == 2)
        {
            math::Position3 hitp;
            hitp.v = _mm_setzero_ps();
            hitp.v.m128_f32[2] = hit_offset;
            math::Position3 hitd;
            hitd.v = dir->v;
            ApplyPhysics(ent, &hitp, (const math::Dir3*)&hitd,
                         intensity_scale * intensity, true, HITLOC_TORSO_UPR);
        }
        if (intensity < 0.0f)
            intensity = 0.0f;
        else if (intensity > 1.0f)
            intensity = 1.0f;
        float axis[3][3];
        AnglesToAxis(&scr_vehicle->phys.angles, axis);
        scr_vehicle->joltDir[0] = (dir->v.m128_f32[0] * axis[0][0])
                                + (dir->v.m128_f32[1] * axis[0][1])
                                + (dir->v.m128_f32[2] * axis[0][2]);
        scr_vehicle->joltDir[1] = -((dir->v.m128_f32[0] * axis[1][0])
                                  + (dir->v.m128_f32[1] * axis[1][1])
                                  + (dir->v.m128_f32[2] * axis[1][2]));
        scr_vehicle->joltTime = 0.80000001f;
        scr_vehicle->joltWave = 0.0f;
        VectorNormalize2D(scr_vehicle->joltDir);
        scr_vehicle->joltDir[0] = (v8->maxBodyPitch * scr_vehicle->joltDir[0]) * intensity;
        scr_vehicle->joltDir[1] = (v8->maxBodyRoll * scr_vehicle->joltDir[1]) * intensity;
    }
    else
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_vehicle.cpp";
        AeAssert::gCurrentLine = 1699;
        AeAssert::gCurrentExpr = "veh";
        if (!AeAssert::IsIgnored())
        {
            const char* v6 = ent->mClassName.mBlock != nullptr
                                 ? (const char*)&ent->mClassName.mBlock[1]
                                 : defaultFileName;
            if (AeAssert::Assert("Non vehicle entity %s passed to VEH_JoltBody", v6))
                __debugbreak();
        }
    }
}

// ea: 0x0046CB00
void Svcmd_VehicleList_f()
{
    int v0 = 0;
    for (int i = 0; i < level.MaxVehicles; ++i)
    {
        unsigned int v3 = s_vehicles[i].mEntity.mHandle.mVal & 0xFFF;
        if (v3 < 0x540
            && s_vehicles[i].mEntity.mHandle.mVal >> 12
                   == EntityHandleDb::sInst.mElements[v3].mKey
            && EntityHandleDb::sInst.mElements[v3].mObject != nullptr)
            ++v0;
    }
    G_Printf("vehicles %d\n", v0);
}

// ea: 0x0046D2B0
void VEH_NetAltWeaponStatus(Entity* ent, int status)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    Entity* mObject = HandleDbToEnt(ent->r.mOwner);
    if (!EntityManager::sInst->IsLocalPlayer(mObject))
        scr_vehicle->seats[0].firing = (status != 0);
}

// ea: 0x0044DB90 (file-local)
static Handle VEH_StartWheelEffect(Entity* ent, unsigned int wheel_tag_hash,
                                   int mat)
{
    vehicle_info_t* info = s_vehicleInfos[ent->scr_vehicle->infoIdx];
    if (info->type == 2)
        wheel_tag_hash = 0;
    return PostEffectEventVehicleWheel(ent, (const char*)info,
                                       0x29 /* kActionVEHICLE_HORN */, mat,
                                       wheel_tag_hash);
}

// ea: 0x0045C7F0
void VEH_UpdateWheelParticleEffects(Entity* ent, int wheelIndex)
{
    scr_vehicle_t* scr_vehicle = ent->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    if (info->maxSpeed <= 88.0f)
        return;
    if (scr_vehicle->engineSndLerp > s_sndLerpMin
        && scr_vehicle->phys.wheelSurfType[wheelIndex] != 0)
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal == 0)
        {
            scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex] =
                VEH_StartWheelEffect(ent, s_wheelTagHashes[wheelIndex],
                                     scr_vehicle->phys.wheelSurfType[wheelIndex]);
        }
        float v8 = (ent->speed - 88.0f) / (info->maxSpeed - 88.0f);
        if (v8 < 0.0f)
            v8 = 0.0f;
        else if (v8 > 1.0f)
            v8 = 1.0f;
        float scaleValue2 = (float)(fabs(scr_vehicle->phys.rotVel.v.m128_f32[1])
                                    / info->rotRate);
        float v7 = (scr_vehicle->brakeSndLerp > 0.5f) ? 1.0f : 0.0f;
        float v9 = (v8 <= scaleValue2) ? scaleValue2 : v8;
        if (v9 <= v7)
            v8 = v7;
        else if (v8 <= scaleValue2)
            v8 = scaleValue2;
        EffectEventAdjustEffect_Scale(scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex],
                                      "EmissionRate", rate * v8);
    }
    else
    {
        if (scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal != 0)
            EffectEventStopEmitting(scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal);
        scr_vehicle->mWheel_ParticleEffectHandle[wheelIndex].mVal = 0;
    }
}

// ea: 0x0045C3A0
void VEH_DebugBox(const math::Position3* pos, float width, float r, float g, float b)
{
    float color[4] = {1.0f,
                      pos->v.m128_f32[0] + width * 0.5f,
                      pos->v.m128_f32[1] + width * 0.5f,
                      pos->v.m128_f32[2] + width * 0.5f};
    float mins[3] = {pos->v.m128_f32[0] - width * 0.5f,
                     pos->v.m128_f32[1] - width * 0.5f,
                     pos->v.m128_f32[2] - width * 0.5f};
    float boxColor[3] = {r, g, b};
    G_DebugBox(&color[1], mins, boxColor, 1, 0, 0);
}

// ea: 0x0045CA30
void VEH_FillFollowHistoryBuffer(scr_vehicle_t* veh)
{
    float facing[3];
    AnglesToForward(veh->phys.angles.v.m128_f32, facing);
    VectorNormalizeFast(facing);
    int v1 = 117;
    int v2 = -36;
    do
    {
        for (int b = 0; b < 10; ++b)
        {
            float dist = (float)(v2 + 18 - 18 * b);
            float* dst = veh->follow->positionHistory[v1 / 3 - b];
            dst[0] = veh->phys.origin.v.m128_f32[0] + dist * facing[0];
            dst[1] = veh->phys.origin.v.m128_f32[1] + dist * facing[1];
            dst[2] = veh->phys.origin.v.m128_f32[2] + dist * facing[2];
        }
        v2 -= 180;
        v1 -= 30;
    } while (v2 >= -576);
    veh->follow->numHistoryBufferEntries = 40;
    veh->follow->historyBufferFront = 0;
}

// ea: 0x0047F630
void VEH_UpdateOverHeat(Entity* self, int msec)
{
    scr_vehicle_t* scr_vehicle = self->scr_vehicle;
    vehicle_info_t* info = s_vehicleInfos[scr_vehicle->infoIdx];
    for (int i = 0; i < 11; ++i)
    {
        vehicleSeat_t& seat = scr_vehicle->seats[i];
        if (seat.gunMounted)
        {
            if (seat.weapon != 0)
            {
                weaponFileInfo_t* InfoForWeapon = BG_GetInfoForWeapon(seat.weapon);
                if (seat.heat < 1.0f)
                {
                    if (seat.overheating && seat.heat <= 0.5f)
                    {
                        Handle v9;
                        v9.mVal = seat.overheatEffect.mVal;
                        seat.overheating = false;
                        if (v9.mVal != 0)
                        {
                            EffectEventSys::sInst->StopEffect(v9.mVal, false);
                            seat.overheatEffect.mVal = 0;
                        }
                    }
                }
                else
                {
                    seat.overheating = true;
                    PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                          0x37 /* kActionMax|kActionWEAPON_PICKUP */);
                    Scr_Notify(self, hash_const.overheated, 0);
                    Handle v8;
                    v8.mVal = seat.overheatEffect.mVal;
                    if (v8.mVal != 0)
                        EffectEventSys::sInst->AdjustEffect_Scale(v8.mVal,
                                                                  "EmissionRate", 200.0f);
                }
                float heat = seat.heat;
                if (heat <= 0.0f)
                    seat.heat = 0.0f;
                else
                    seat.heat = heat - (msec * InfoForWeapon->fCooldownRate) * 0.001f;
                Handle v11;
                v11.mVal = seat.overheatEffect.mVal;
                if (seat.heat <= 0.25f)
                {
                    if (v11.mVal != 0)
                    {
                        EffectEventSys::sInst->StopEffect(v11.mVal, false);
                        seat.overheatEffect.mVal = 0;
                    }
                }
                else
                {
                    if (v11.mVal == 0)
                        seat.overheatEffect =
                            PostEffectEventWeapon(self, InfoForWeapon->szInternalName,
                                                  0x36 /* kActionMax|kActionWEAPON_LAST_SHOT_EJECT */);
                    float scale = (seat.heat - 0.25f) * 1.333333333333333f * emissionRate_0;
                    EffectEventSys::sInst->AdjustEffect_Scale(seat.overheatEffect.mVal,
                                                              "EmissionRate", scale);
                }
            }
        }
        else if (info->type == 1 && i == 0)
        {
            if (scr_vehicle->seats[0].heat < 1.0f)
            {
                if (scr_vehicle->seats[0].overheating
                    && scr_vehicle->seats[0].heat <= 0.5f)
                    scr_vehicle->seats[0].overheating = false;
            }
            else
            {
                scr_vehicle->seats[0].overheating = true;
            }
            float heat = scr_vehicle->seats[0].heat;
            if (heat <= 0.0f)
                scr_vehicle->seats[0].heat = 0.0f;
            else
                scr_vehicle->seats[0].heat = heat - msec * 0.00025000001f;
        }
    }
}

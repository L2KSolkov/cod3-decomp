// ============================================================================
// g_mover.cpp - movers/doors (g.o: g_mover.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

// ea: 0x0048CB90
void G_Activate(Entity* ent, Entity* activator)
{
    if (ent->s.apos.trType == TR_STATIONARY
        && ent->s.pos.trType == TR_STATIONARY
        && ent->active == 0
        && ent->key == 0)
    {
        Entity* teammaster = ent->teammaster;
        if (teammaster == nullptr || ent->team.is_empty() || ent == teammaster)
        {
            ent->active = 1;
            Use_BinaryMover(ent, activator, activator);
        }
        else
        {
            teammaster->active = 1;
            Use_BinaryMover(ent->teammaster, activator, activator);
        }
    }
}

#include <math.h>
#include <stdlib.h>
#include <string.h>

extern "C" int __fpclass(float);

static bool IS_NAN(float x) {
    return (__fpclass(x) & 0x297) != 0;
}

// g.o data (BSS, verified via refs)
pushed_t pushed[256];                            // 0xEAC948
pushed_t* pushed_p = pushed;                     // 0xEAE2E8
DbLinkedHandle<EntityHandleDb, Entity> entityList[256];  // 0xEF5E20
DbLinkedHandle<EntityHandleDb, Entity> moveList[256];    // 0xEF5950
unsigned int _S68_2;                             // 0xEF62EC

// dispatch tables (filled by g_main during init; extern declarations)
void (*reachedtable[3])(Entity* ent);
void (*blockedtable[3])(Entity* ent, Entity* other);

// ea: 0x0044C830
void finishSpawningKeyedMover(Entity* ent)
{
    ent->nextthink = level.time + 100;
    if ((ent->flags & 0x10) == 0)
    {
        if (ent->takedamage != 0)
        {
            ent->think = THINK__Scr_Vehicle_Think;
        }
        else if (ent->mClassNameHash.mHash == hash_const.func_door_rotating.mHash)
        {
            ent->think = THINK__Think_SpawnNewDoorTrigger;
        }
        else
        {
            ent->think = 25 + ((ent->spawnflags & 8) != 0);
        }
        Entity* v1 = ent;
        do
        {
            if (v1 != ent)
                v1->key = ent->key;
            v1 = v1->teamchain;
        } while (v1 != nullptr);
    }
}

// ea: 0x00459BF0
void SetMoverState(Entity* ent, moverState_t moverState, int time)
{
    int v4 = ent->flags & 0x100;
    if ((ent->flags & 0x800) != 0)
        v4 = 0;
    ent->moverState = (uint8_t)moverState;
    ent->s.pos.trTime = time;
    ent->s.apos.trTime = time;
    switch (moverState)
    {
    case MOVER_POS1:
        memcpy(ent->s.pos.trBase, &ent->pos1, sizeof(ent->s.pos.trBase));
        ent->s.pos.trType = TR_STATIONARY;
        ent->active = 0;
        break;
    case MOVER_POS2:
        memcpy(ent->s.pos.trBase, &ent->pos2, sizeof(ent->s.pos.trBase));
        ent->s.pos.trType = TR_STATIONARY;
        break;
    case MOVER_POS3:
        memcpy(ent->s.pos.trBase, &ent->pos3, sizeof(ent->s.pos.trBase));
        ent->s.pos.trType = TR_STATIONARY;
        break;
    case MOVER_1TO2:
        ent->s.pos.trBase[0] = ent->pos1.v.m128_f32[0];
        ent->s.pos.trBase[1] = ent->pos1.v.m128_f32[1];
        ent->s.pos.trBase[2] = ent->pos1.v.m128_f32[2];
        {
            int gDuration = ent->gDuration;
            float v12 = 1000.0f / gDuration;
            float v13 = v12 * (ent->pos2.v.m128_f32[0] - ent->pos1.v.m128_f32[0]);
            float v14 = v12 * (ent->pos2.v.m128_f32[1] - ent->pos1.v.m128_f32[1]);
            float v15 = v12 * (ent->pos2.v.m128_f32[2] - ent->pos1.v.m128_f32[2]);
            ent->s.pos.trDuration = gDuration;
            ent->s.pos.trDelta[0] = v13;
            ent->s.pos.trDelta[1] = v14;
            ent->s.pos.trDelta[2] = v15;
            if (IS_NAN(v13) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
                AeAssert::gCurrentLine = 725;
                AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            ent->s.pos.trType = TR_LINEAR_STOP;
        }
        break;
    case MOVER_2TO1:
        memcpy(ent->s.pos.trBase, &ent->pos2, sizeof(ent->s.pos.trBase));
        {
            float v17 = ent->pos1.v.m128_f32[0] - ent->pos2.v.m128_f32[0];
            float v18 = ent->pos1.v.m128_f32[1] - ent->pos2.v.m128_f32[1];
            float v19 = ent->pos1.v.m128_f32[2] - ent->pos2.v.m128_f32[2];
            int gDurationBack = ent->closespeed == 0.0f ? ent->gDuration : ent->gDurationBack;
            float v21 = 1000.0f / gDurationBack;
            ent->s.pos.trDelta[0] = v21 * v17;
            ent->s.pos.trDuration = gDurationBack;
            ent->s.pos.trDelta[1] = v21 * v18;
            ent->s.pos.trDelta[2] = v21 * v19;
            if (IS_NAN(ent->s.pos.trDelta[0]) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
                AeAssert::gCurrentLine = 739;
                AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                    __debugbreak();
            }
            ent->s.pos.trType = TR_LINEAR_STOP;
        }
        break;
    case MOVER_2TO3:
    {
        float trDuration = ent->s.pos.trDuration;
        memcpy(ent->s.pos.trBase, &ent->pos2, sizeof(ent->s.pos.trBase));
        float enta = (1000.0f / trDuration) * (ent->pos3.v.m128_f32[0] - ent->pos2.v.m128_f32[0]);
        float v6 = (1000.0f / trDuration) * (ent->pos3.v.m128_f32[1] - ent->pos2.v.m128_f32[1]);
        float v7 = (1000.0f / trDuration) * (ent->pos3.v.m128_f32[2] - ent->pos2.v.m128_f32[2]);
        ent->s.pos.trDelta[0] = enta;
        ent->s.pos.trDelta[1] = v6;
        ent->s.pos.trDelta[2] = v7;
        if (IS_NAN(enta) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 705;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->s.pos.trType = TR_LINEAR_STOP;
        break;
    }
    case MOVER_3TO2:
    {
        float v8 = ent->s.pos.trDuration;
        memcpy(ent->s.pos.trBase, &ent->pos3, sizeof(ent->s.pos.trBase));
        float entb = (1000.0f / v8) * (ent->pos2.v.m128_f32[0] - ent->pos3.v.m128_f32[0]);
        float v9 = (1000.0f / v8) * (ent->pos2.v.m128_f32[1] - ent->pos3.v.m128_f32[1]);
        float v10 = (1000.0f / v8) * (ent->pos2.v.m128_f32[2] - ent->pos3.v.m128_f32[2]);
        ent->s.pos.trDelta[0] = entb;
        ent->s.pos.trDelta[1] = v9;
        ent->s.pos.trDelta[2] = v10;
        if (IS_NAN(entb) || IS_NAN(ent->s.pos.trDelta[1]) || IS_NAN(ent->s.pos.trDelta[2]))
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 713;
            AeAssert::gCurrentExpr = "!IS_NAN((ent->s.pos.trDelta)[0]) && !IS_NAN((ent->s.pos.trDelta)[1]) && !IS_NAN((ent->s.pos.trDelta)[2])";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
                __debugbreak();
        }
        ent->s.pos.trType = TR_LINEAR_STOP;
        break;
    }
    case MOVER_POS1ROTATE:
    case MOVER_POS2ROTATE:
        memcpy(ent->s.apos.trBase, &ent->r.currentAngles, sizeof(ent->s.apos.trBase));
        ent->s.apos.trType = TR_STATIONARY;
        break;
    case MOVER_1TO2ROTATE:
    {
        int v22 = ent->gDuration;
        ent->s.apos.trBase[1] = 0.0f;
        ent->s.apos.trBase[0] = 0.0f;
        float v24;
        if (v4 == 0)
        {
            v24 = 1000.0f;
        }
        else
        {
            v24 = 500.0f;
            v22 *= 2;
        }
        float v26 = ent->angle * (v24 / v22);
        ent->s.apos.trDuration = v22;
        ent->s.apos.trDelta[0] = ent->rotate.v.m128_f32[0] * v26;
        ent->s.apos.trDelta[1] = ent->rotate.v.m128_f32[1] * v26;
        ent->s.apos.trDelta[2] = ent->rotate.v.m128_f32[2] * v26;
        ent->s.apos.trType = TR_LINEAR_STOP;
        break;
    }
    case MOVER_2TO1ROTATE:
    {
        int v27 = ent->gDuration;
        ent->s.apos.trBase[0] = ent->rotate.v.m128_f32[0] * ent->angle;
        ent->s.apos.trBase[1] = ent->rotate.v.m128_f32[1] * ent->angle;
        ent->s.apos.trBase[2] = ent->rotate.v.m128_f32[2] * ent->angle;
        float v28 = 1000.0f / v27;
        ent->s.apos.trDuration = v27;
        if (v4 != 0)
        {
            v28 = v28 * 0.5f;
            ent->s.apos.trDuration = 2 * v27;
        }
        ent->s.apos.trDelta[0] = ent->s.apos.trBase[0] * (0.0f - v28);
        ent->s.apos.trDelta[1] = ent->s.apos.trBase[1] * (0.0f - v28);
        ent->s.apos.trDelta[2] = ent->s.apos.trBase[2] * (0.0f - v28);
        ent->s.apos.trType = TR_LINEAR_STOP;
        ent->active = 0;
        break;
    }
    default:
        break;
    }
    BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
    if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
        g_LinkEntity(ent);
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto disconnect;
        }
        else if (ent->moverState == 8)
        {
disconnect:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            return;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
}

// ea: 0x0045A420
void MatchTeam(Entity* teamLeader, moverState_t moverState, int time)
{
    Entity* v3 = teamLeader;
    Entity* v4 = teamLeader;
    if (teamLeader != nullptr)
    {
        while (1)
        {
            if ((v3->flags & 0x100) != 0)
                v4->flags |= 0x100u;
            SetMoverState(v4, moverState, time);
            v4 = v4->teamchain;
            if (v4 == nullptr)
                break;
            v3 = teamLeader;
        }
    }
}

// ea: 0x0045A470
void MatchTeamReverseAngleOnSlaves(Entity* teamLeader, moverState_t moverState, int time)
{
    for (Entity* i = teamLeader; i != nullptr; i = i->teamchain)
    {
        i->angle = i->angle * -1.0f;
        if ((teamLeader->flags & 0x100) != 0)
            i->flags |= 0x100u;
        SetMoverState(i, moverState, time);
    }
}

// ea: 0x0045A4E0
void ReturnToPos1(Entity* ent)
{
    MatchTeam(ent, MOVER_2TO1, level.time);
}

// ea: 0x0045A500
void ReturnToPos2(Entity* ent)
{
    MatchTeam(ent, MOVER_3TO2, level.time);
}

// ea: 0x0045A520
void GotoPos3(Entity* ent)
{
    MatchTeam(ent, MOVER_2TO3, level.time);
}

// ea: 0x0045A540
int IsBinaryMoverBlocked(Entity* ent, Entity* other, Entity* activator)
{
    if (ent->mClassNameHash.mHash != hash_const.func_door_rotating.mHash || (ent->spawnflags & 0x20) != 0)
        return 0;
    float v3 = (ent->r.absmax.v.m128_f32[1] + ent->r.absmin.v.m128_f32[1]) * 0.5f;
    float v4 = (ent->r.absmax.v.m128_f32[2] + ent->r.absmin.v.m128_f32[2]) * 0.5f;
    float pos[3];
    pos[0] = (ent->r.absmax.v.m128_f32[0] + ent->r.absmin.v.m128_f32[0]) * 0.5f;
    pos[1] = v3;
    pos[2] = v4;
    float dir[3];
    dir[0] = pos[0] - ent->r.currentOrigin.v.m128_f32[0];
    dir[1] = v3 - ent->r.currentOrigin.v.m128_f32[1];
    dir[2] = v4 - ent->r.currentOrigin.v.m128_f32[2];
    float angles[3];
    vectoangles(dir, angles);
    if (ent->rotate.v.m128_f32[1] == 0.0f)
    {
        if (ent->rotate.v.m128_f32[0] == 0.0f)
        {
            if (ent->rotate.v.m128_f32[2] != 0.0f)
                angles[2] = ent->angle + angles[2];
        }
        else
        {
            angles[0] = ent->angle + angles[0];
        }
    }
    else
    {
        angles[1] = ent->angle + angles[1];
    }
    float forward[3];
    AnglesToForward(angles, forward);
    float vec[3];
    vec[0] = activator->r.currentOrigin.v.m128_f32[0] - pos[0];
    vec[1] = activator->r.currentOrigin.v.m128_f32[1] - pos[1];
    vec[2] = activator->r.currentOrigin.v.m128_f32[2] - pos[2];
    VectorNormalize(vec);
    return ((forward[2] * vec[2]) + (forward[1] * vec[1]) + (forward[0] * vec[0])) >= 0.0f;
}

// ea: 0x0045A700
void InitMover(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int noise_hash;
    static unsigned int light_hash;
    static unsigned int color_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        noise_hash = HashString::CalcHash("noise");
    }
    const char* sound = nullptr;
    if (G_SpawnString(noise_hash, "100", &sound) != 0)
        ent->s.loopSound = G_SoundAliasIndex(sound);
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        light_hash = HashString::CalcHash("light");
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        color_hash = HashString::CalcHash("color");
    }
    float light = 100.0f;
    int v3 = G_SpawnFloat(light_hash, 100.0, &light);
    float def_val[3] = { 1.0f, 1.0f, 1.0f };
    float color[3];
    int v4 = G_SpawnVector(color_hash, def_val, color);
    if (v3 != 0 || v4 != 0)
    {
        int v5 = (int)(color[0] * 255.0f);
        if (v5 > 255) v5 = 255;
        int v6 = (int)(color[1] * 255.0f);
        if (v6 > 255) v6 = 255;
        int v7 = (int)(color[2] * 255.0f);
        if (v7 > 255) v7 = 255;
        int v8 = (int)(light * 0.25f);
        if (v8 > 255) v8 = 255;
        ent->s.constantLight = v5 | ((v6 | ((v7 | (v8 << 8)) << 8)) << 8);
    }
    if (ent->mClassNameHash.mHash == hash_const.func_rotating.mHash)
    {
        ent->use = 7;
        ent->reached = 0;
    }
    else
    {
        ent->use = 5;
        ent->reached = 1;
    }
    ent->moverState = 0;
    ent->r.svFlags = 128;
    ent->s.eType = 4;
    if (IS_NAN(ent->r.currentOrigin.v.m128_f32[0])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[1])
        || IS_NAN(ent->r.currentOrigin.v.m128_f32[2]))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1405;
        AeAssert::gCurrentExpr = "!IS_NAN((ent->r.currentOrigin)[0]) && !IS_NAN((ent->r.currentOrigin)[1]) && !IS_NAN((ent->r.currentOrigin)[2])";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid vector"))
            __debugbreak();
    }
    ent->r.currentOrigin = ent->pos1;
    g_LinkEntity(ent);
    ent->s.pos.trType = TR_STATIONARY;
    ent->s.pos.trBase[0] = ent->pos1.v.m128_f32[0];
    ent->s.pos.trBase[1] = ent->pos1.v.m128_f32[1];
    ent->s.pos.trBase[2] = ent->pos1.v.m128_f32[2];
    float speed = ent->speed;
    double v10 = ent->pos2.v.m128_f32[1] - ent->pos1.v.m128_f32[1];
    double v11 = ent->pos2.v.m128_f32[2] - ent->pos1.v.m128_f32[2];
    double v12 = ent->pos2.v.m128_f32[0] - ent->pos1.v.m128_f32[0];
    float dist = (float)sqrt(v11 * v11 + v10 * v10 + v12 * v12);
    if (speed == 0.0f)
        ent->speed = 100.0f;
    float v13 = dist * 1000.0f;
    int v14 = (int)((dist * 1000.0f) / ent->speed);
    ent->s.pos.trDuration = v14;
    if (v14 <= 0)
        ent->s.pos.trDuration = 1;
    int trDuration = ent->s.pos.trDuration;
    float closespeed = ent->closespeed;
    ent->gDuration = trDuration;
    ent->gDurationBack = trDuration;
    if (closespeed != 0.0f)
    {
        int v17 = (int)(v13 / closespeed);
        ent->gDurationBack = v17;
        if (v17 <= 0)
            ent->gDurationBack = 1;
    }
}

// ea: 0x0045AAA0
void InitMoverRotate(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int light_hash;
    static unsigned int color_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        light_hash = HashString::CalcHash("light");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        color_hash = HashString::CalcHash("color");
    }
    float light = 100.0f;
    int v1 = G_SpawnFloat(light_hash, 100.0, &light);
    float def_val[3] = { 1.0f, 1.0f, 1.0f };
    float color[3];
    int v2 = G_SpawnVector(color_hash, def_val, color);
    if (v1 != 0 || v2 != 0)
    {
        int v3 = (int)(color[0] * 255.0f);
        if (v3 > 255) v3 = 255;
        int v4 = (int)(color[1] * 255.0f);
        if (v4 > 255) v4 = 255;
        int v5 = (int)(color[2] * 255.0f);
        if (v5 > 255) v5 = 255;
        int v6 = (int)(light * 0.25f);
        if (v6 > 255) v6 = 255;
        ent->s.constantLight = v3 | ((v4 | ((v5 | (v6 << 8)) << 8)) << 8);
    }
    ent->use = 5;
    if ((ent->spawnflags & 0x40) == 0)
        ent->reached = 1;
    ent->moverState = 7;
    ent->r.svFlags = 128;
    ent->s.eType = 4;
    g_LinkEntity(ent);
    float speed = ent->speed;
    ent->s.pos.trType = TR_STATIONARY;
    memcpy(ent->s.pos.trBase, &ent->r.currentOrigin, sizeof(ent->s.pos.trBase));
    if (speed == 0.0f)
        ent->speed = 100.0f;
    int v9 = (int)ent->speed;
    ent->s.apos.trDuration = v9;
    if (v9 <= 0)
        ent->s.apos.trDuration = 1;
    int trDuration = ent->s.apos.trDuration;
    ent->gDurationBack = trDuration;
    ent->gDuration = trDuration;
}

// ea: 0x0045ACB0 (static helper)
static Entity* Think_SpawnNewDoorTriggerInternal(Entity* ent)
{
    for (Entity* i = ent; i != nullptr; i = i->teamchain)
        i->takedamage = 1;
    math::Position3 mins = ent->r.absmin;
    math::Position3 maxs = ent->r.absmax;
    for (Entity* t = ent->teamchain; t != nullptr; t = t->teamchain)
    {
        mins.v = _mm_min_ps(mins.v, t->r.absmin.v);
        maxs.v = _mm_max_ps(maxs.v, t->r.absmax.v);
    }
    int v9 = (maxs.v.m128_f32[0] - mins.v.m128_f32[0]) > (maxs.v.m128_f32[1] - mins.v.m128_f32[1]);
    if ((maxs.v.m128_f32[v9] - mins.v.m128_f32[v9]) > (maxs.v.m128_f32[2] - mins.v.m128_f32[2]))
        v9 = 2;
    maxs.v.m128_f32[v9] += 120.0f;
    mins.v.m128_f32[v9] -= 120.0f;
    Entity* v10 = G_Spawn(PAK_ID_INVALID);
    Entity* v12 = v10;
    v10->s.eType = 0;
    v10->s.eventParm = (uint8_t)mins.v.m128_f32[1];
    v10->s.mOtherEntity.mHandle.mVal = (unsigned int)mins.v.m128_f32[2];
    v10->s.mGroundEntity.mHandle.mVal = (unsigned int)mins.v.m128_f32[3];
    v12->r.maxs.v.m128_f32[0] = maxs.v.m128_f32[0];
    v12->r.maxs.v.m128_f32[1] = maxs.v.m128_f32[1];
    v12->r.maxs.v.m128_f32[2] = maxs.v.m128_f32[2];
    v12->r.maxs.v.m128_f32[3] = maxs.v.m128_f32[3];
    v12->r.mins = mins;
    v12->parentHandle = ent->mHandle;
    v12->r.contents = 0x40000000;
    v12->touch = 8;
    SV_LinkEntity(v12);
    MatchTeam(ent, (moverState_t)ent->moverState, level.time);
    return v12;
}

// ea: 0x0045AE60
void Think_SpawnNewDoorTrigger(Entity* ent)
{
    Think_SpawnNewDoorTriggerInternal(ent);
}

// ea: 0x0045AE70 (static helper)
static void DoorRotateStartOpen(Entity* ent)
{
    if (ent->mClassNameHash.mHash != hash_const.func_door_rotating.mHash)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1693;
        AeAssert::gCurrentExpr = "ent->mClassNameHash == hash_const.func_door_rotating";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->s.apos.trType != TR_STATIONARY)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1694;
        AeAssert::gCurrentExpr = "ent->s.apos.trType == TR_STATIONARY";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->s.pos.trType != TR_STATIONARY)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1695;
        AeAssert::gCurrentExpr = "ent->s.pos.trType == TR_STATIONARY";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->moverState != 7)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1696;
        AeAssert::gCurrentExpr = "ent->moverState == MOVER_POS1ROTATE";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->active != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1697;
        AeAssert::gCurrentExpr = "!ent->active";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->nextthink != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1699;
        AeAssert::gCurrentExpr = "!ent->nextthink";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    float angle = ent->angle;
    ent->think = THINK__ReturnToPos1;
    ent->r.currentAngles.v.m128_f32[1] = angle + ent->r.currentAngles.v.m128_f32[1];
    ent->moverState = 8;
    ent->s.pos.trTime = level.time;
    ent->s.apos.trTime = level.time;
    memcpy(ent->s.apos.trBase, &ent->r.currentAngles, sizeof(ent->s.apos.trBase));
    ent->s.apos.trType = TR_STATIONARY;
    BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
    if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
        g_LinkEntity(ent);
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto disconnect;
        }
        else if (ent->moverState == 8)
        {
disconnect:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            goto done;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
done:
    if (ent->teammaster == ent || ent->teammaster == nullptr)
        SV_AdjustAreaPortalState(ent, 1);
}

// ea: 0x0045B110
void Think_SpawnNewAutoDoorTrigger(Entity* ent)
{
    Think_SpawnNewDoorTriggerInternal(ent)->r.contents = 0x1C0000;
    if ((ent->spawnflags & 1) != 0)
        DoorRotateStartOpen(ent);
}

// ea: 0x0045B140
void Think_MatchTeam(Entity* ent)
{
    MatchTeam(ent, (moverState_t)ent->moverState, level.time);
}

// ea: 0x0045B160
void Use_Static(Entity* ent)
{
    if (ent->r.linked != 0)
        SV_UnlinkEntity(ent);
    else
        g_LinkEntity(ent);
}

// ea: 0x0045B190
void SP_func_static(Entity* ent)
{
    Entity* v1 = ent;
    SV_SetBrushModel(ent);
    InitMover(v1);
    v1->s.pos.trBase[0] = v1->r.currentOrigin.v.m128_f32[0];
    unsigned char spawnflags = v1->spawnflags;
    v1->s.pos.trBase[1] = v1->r.currentOrigin.v.m128_f32[1];
    v1->s.pos.trBase[2] = v1->r.currentOrigin.v.m128_f32[2];
    v1->use = 11;
    if ((spawnflags & 1) != 0)
        SV_UnlinkEntity(v1);
    if ((v1->flags & 0x10) == 0)
    {
        static unsigned int sInit = 0;
        static unsigned int health_hash;
        if ((sInit & 1) == 0)
        {
            sInit |= 1u;
            health_hash = HashString::CalcHash("health");
        }
        int health = 0;
        G_SpawnInt(health_hash, 0, &health);
        if (health != 0)
            v1->takedamage = 1;
    }
    if ((v1->spawnflags & 6) != 0)
    {
        float v3;
        if (v1->delay == 0.0f)
            v3 = 1000.0f;
        else
            v3 = v1->delay * 1000.0f;
        int count = v1->count;
        v1->delay = v3;
        v1->takedamage = 1;
        v1->health = 9999;
        if (count == 0)
            v1->count = 4;
    }
}

// ea: 0x0045B2C0
void Use_Func_Rotate(Entity* ent)
{
    int spawnflags = ent->spawnflags;
    float speed = ent->speed;
    if ((spawnflags & 4) != 0)
        ent->s.apos.trDelta[2] = speed;
    else if ((spawnflags & 8) != 0)
        ent->s.apos.trDelta[0] = speed;
    else
        ent->s.apos.trDelta[1] = speed;
    if ((spawnflags & 2) != 0)
        ent->flags &= ~0x10u;
    g_LinkEntity(ent);
}

// ea: 0x0045B300
void SP_func_rotating(Entity* ent)
{
    if (ent->speed == 0.0f)
        ent->speed = 100.0f;
    int spawnflags = ent->spawnflags;
    ent->s.apos.trType = TR_LINEAR;
    if ((spawnflags & 1) != 0)
    {
        if ((spawnflags & 4) != 0)
            ent->s.apos.trDelta[2] = ent->speed;
        else if ((spawnflags & 8) != 0)
            ent->s.apos.trDelta[0] = ent->speed;
        else
            ent->s.apos.trDelta[1] = ent->speed;
    }
    if (ent->damage == 0)
        ent->damage = 2;
    SV_SetBrushModel(ent);
    InitMover(ent);
    ent->s.pos.trBase[0] = ent->r.currentOrigin.v.m128_f32[0];
    unsigned char v2 = ent->spawnflags;
    ent->s.pos.trBase[1] = ent->r.currentOrigin.v.m128_f32[1];
    ent->s.pos.trBase[2] = ent->r.currentOrigin.v.m128_f32[2];
    if ((v2 & 2) != 0)
    {
        ent->flags |= 0x10u;
        SV_UnlinkEntity(ent);
    }
    else
    {
        g_LinkEntity(ent);
    }
}

// ea: 0x0045B3D0
void SP_func_bobbing(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int speed_hash;
    static unsigned int height_hash;
    static unsigned int phase_hash;
    static unsigned int dmg_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        speed_hash = HashString::CalcHash("speed");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        height_hash = HashString::CalcHash("height");
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        phase_hash = HashString::CalcHash("phase");
    }
    Entity* v1 = ent;
    float* p_speed = &ent->speed;
    G_SpawnFloat(speed_hash, 4.0, &ent->speed);
    float height = 32.0f;
    G_SpawnFloat(height_hash, 32.0, &height);
    if ((sInit & 8) == 0)
    {
        sInit |= 8u;
        dmg_hash = HashString::CalcHash("dmg");
    }
    G_SpawnInt(dmg_hash, 2, &v1->damage);
    float phase = 0.0f;
    G_SpawnFloat(phase_hash, 0.0, &phase);
    SV_SetBrushModel(v1);
    InitMover(v1);
    float v3 = *p_speed * 1000.0f;
    v1->s.pos.trBase[0] = v1->r.currentOrigin.v.m128_f32[0];
    v1->s.pos.trBase[1] = v1->r.currentOrigin.v.m128_f32[1];
    v1->s.pos.trBase[2] = v1->r.currentOrigin.v.m128_f32[2];
    int v4 = (int)v3;
    float v5 = v3 * phase;
    v1->s.pos.trDuration = v4;
    int spawnflags = v1->spawnflags;
    v1->s.pos.trTime = (int)v5;
    v1->s.pos.trType = TR_SINE;
    if ((spawnflags & 1) != 0)
        v1->s.pos.trDelta[0] = height;
    else if ((spawnflags & 2) != 0)
        v1->s.pos.trDelta[1] = height;
    else
        v1->s.pos.trDelta[2] = height;
}

// ea: 0x0045B5B0
void SP_func_pendulum(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int speed_hash;
    static unsigned int dmg_hash;
    static unsigned int phase_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        speed_hash = HashString::CalcHash("speed");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        dmg_hash = HashString::CalcHash("dmg");
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        phase_hash = HashString::CalcHash("phase");
    }
    float speed = 30.0f;
    G_SpawnFloat(speed_hash, 30.0, &speed);
    G_SpawnInt(dmg_hash, 2, &ent->damage);
    float phase = 0.0f;
    G_SpawnFloat(phase_hash, 0.0, &phase);
    SV_SetBrushModel(ent);
    float length = (float)fabs(ent->r.mins.v.m128_f32[2]);
    if (length < 8.0f)
        length = 8.0f;
    int v3 = (int)(1000.0f / (sqrt(g_gravity.value / (length * 3.0f)) * 0.15915494f));
    ent->s.pos.trDuration = v3;
    InitMover(ent);
    ent->s.pos.trBase[0] = ent->r.currentOrigin.v.m128_f32[0];
    ent->s.pos.trBase[1] = ent->r.currentOrigin.v.m128_f32[1];
    ent->s.pos.trBase[2] = ent->r.currentOrigin.v.m128_f32[2];
    ent->s.apos.trBase[0] = ent->r.currentAngles.v.m128_f32[0];
    ent->s.apos.trBase[1] = ent->r.currentAngles.v.m128_f32[1];
    ent->s.apos.trDuration = v3;
    ent->s.apos.trBase[2] = ent->r.currentAngles.v.m128_f32[2];
    ent->s.apos.trTime = (int)(v3 * phase);
    ent->s.apos.trType = TR_SINE;
    ent->s.apos.trDelta[2] = speed;
}

// ea: 0x0045B760
void SP_func_door_rotating(Entity* ent)
{
    Entity* v1 = ent;
    int v2 = v1->spawnflags;
    if ((v2 & 1) != 0)
        v1->spawnflags = v2 | 2;
    float speed = v1->speed;
    unsigned int v4 = v1->spawnflags & 0xFFFFFFBF;
    v1->spawnflags = v4;
    if (speed == 0.0f)
        v1->speed = 1000.0f;
    if (v1->angle == 0.0f)
        v1->angle = 90.0f;
    if ((v4 & 0x10) != 0)
        v1->angle = v1->angle * -1.0f;
    if ((v4 & 2) != 0)
        v1->flags |= 0x80u;
    static unsigned int sInit = 0;
    static unsigned int key_hash;
    static unsigned int health_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        key_hash = HashString::CalcHash("key");
    }
    int key = 0;
    v1->key = G_SpawnInt(key_hash, 0, &key) != 0;
    v1->rotate.v.m128_f32[1] = 0.0f;
    v1->rotate.v.m128_f32[0] = 0.0f;
    int spawnflags = v1->spawnflags;
    if ((spawnflags & 0xC) == 0xC)
        goto axisY;
    if ((spawnflags & 4) != 0)
    {
        v1->rotate.v.m128_f32[2] = 1.0f;
        goto axisDone;
    }
    if ((spawnflags & 8) == 0)
axisY:
        v1->rotate.v.m128_f32[1] = 1.0f;
    else
        v1->rotate.v.m128_f32[0] = 1.0f;
axisDone:
    float len2 = (v1->rotate.v.m128_f32[0] * v1->rotate.v.m128_f32[0])
               + (v1->rotate.v.m128_f32[1] * v1->rotate.v.m128_f32[1])
               + (v1->rotate.v.m128_f32[2] * v1->rotate.v.m128_f32[2]);
    if (sqrt(len2) > 1.0f)
    {
        G_Error("Too many axis marked in func_door_rotating entity.  Only choose one axis of rotation. (defaulting to standard door rotation)");
        v1->rotate.v.m128_f32[2] = 0.0f;
        v1->rotate.v.m128_f32[0] = 0.0f;
        v1->rotate.v.m128_f32[1] = 1.0f;
    }
    if (v1->wait == 0.0f)
        v1->wait = 2.0f;
    v1->wait = v1->wait * 1000.0f;
    SV_SetBrushModel(v1);
    InitMoverRotate(v1);
    if ((v1->flags & 0x10) == 0)
    {
        if ((sInit & 2) == 0)
        {
            sInit |= 2u;
            health_hash = HashString::CalcHash("health");
        }
        int health = 0;
        G_SpawnInt(health_hash, 0, &health);
        if (health != 0)
            v1->takedamage = 1;
    }
    v1->flags |= 0x1000u;
    v1->nextthink = level.time + 100;
    v1->think = THINK__finishSpawningKeyedMover;
    v1->blocked = 2;
}

// ea: 0x0045C0C0
void InitScriptMover(Entity* pSelf)
{
    static unsigned int sInit = 0;
    static unsigned int light_hash;
    static unsigned int color_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        light_hash = HashString::CalcHash("light");
    }
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        color_hash = HashString::CalcHash("color");
    }
    float fLight = 100.0f;
    int v1 = G_SpawnFloat(light_hash, 100.0, &fLight);
    float val[3] = { 1.0f, 1.0f, 1.0f };
    float vColor[3];
    int v2 = G_SpawnVector(color_hash, val, vColor);
    if (v1 != 0 || v2 != 0)
    {
        int v3 = (int)(vColor[0] * 255.0f);
        if (v3 > 255) v3 = 255;
        int v4 = (int)(vColor[1] * 255.0f);
        if (v4 > 255) v4 = 255;
        int v5 = (int)(vColor[2] * 255.0f);
        if (v5 > 255) v5 = 255;
        int v6 = (int)(fLight * 0.25f);
        if (v6 > 255) v6 = 255;
        pSelf->s.constantLight = v3 | ((v4 | ((v5 | (v6 << 8)) << 8)) << 8);
    }
    pSelf->reached = 2;
    pSelf->r.svFlags = 128;
    pSelf->s.eType = 7;
    memcpy(pSelf->s.pos.trBase, &pSelf->r.currentOrigin, sizeof(pSelf->s.pos.trBase));
    pSelf->s.pos.trType = TR_STATIONARY;
    memcpy(pSelf->s.apos.trBase, &pSelf->r.currentAngles, sizeof(pSelf->s.apos.trBase));
    pSelf->flags |= 0x8000;
    pSelf->s.apos.trType = TR_STATIONARY;
}

// ea: 0x0045C290
void SP_script_brushmodel(Entity* pSelf)
{
    AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_scr_mover.cpp";
    AeAssert::gCurrentLine = 502;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning("SP_script_brushmodel - dead code"))
        __debugbreak();
    SV_SetBrushModel(pSelf);
    InitScriptMover(pSelf);
    pSelf->r.contents = 1;
    g_LinkEntity(pSelf);
    pSelf->flags |= 0x3000u;
}

// ea: 0x0045C310
void SP_script_origin(Entity* pSelf)
{
    InitScriptMover(pSelf);
    pSelf->r.contents = 0;
    g_LinkEntity(pSelf);
    if (pSelf->s.constantLight != 0)
        pSelf->s.eFlags |= 0x80u;
    else
        pSelf->r.svFlags |= 1u;
}

// ea: 0x00462B50
void ReturnToPos1Rotate(Entity* ent)
{
    MatchTeam(ent, MOVER_2TO1ROTATE, level.time);
    Entity* v1 = EntityHandleDb::sInst.Find(640, hash_const.player);
    if (v1 != nullptr)
        SV_inPVS(&v1->r.currentOrigin, &ent->r.currentOrigin);
}

// ea: 0x00462BA0
void Reached_BinaryMover(Entity* ent)
{
    unsigned char moverState = ent->moverState;
    ent->s.loopSound = 0;
    switch (moverState)
    {
    case 3u:  // MOVER_1TO2
        ent->s.pos.trTime = level.time;
        ent->s.apos.trTime = level.time;
        ent->moverState = 1;
        memcpy(ent->s.pos.trBase, &ent->pos2, sizeof(ent->s.pos.trBase));
        ent->s.pos.trType = TR_STATIONARY;
        BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
        if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
            g_LinkEntity(ent);
        if ((ent->flags & 0x1000) == 0)
            goto label_9;
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto label_8;
        }
        else if (ent->moverState == 8)
        {
label_8:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            goto label_9;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
label_9:
        if (ent->activator == nullptr)
            ent->activator = ent;
        if ((char)ent->flags >= 0)
        {
            float wait = ent->wait;
            if (wait != -1000.0f)
            {
                ent->think = THINK__RespawnItem;
                ent->nextthink = level.time + (int)wait;
            }
        }
        else
        {
            ent->active = 0;
            ent->think = THINK__RespawnItem;
            ent->nextthink = 0;
        }
        return;
    case 4u:  // MOVER_2TO1
        ent->s.pos.trTime = level.time;
        ent->s.apos.trTime = level.time;
        ent->moverState = 0;
        memcpy(ent->s.pos.trBase, &ent->pos1, sizeof(ent->s.pos.trBase));
        ent->s.pos.trType = TR_STATIONARY;
        ent->active = 0;
        BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
        if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
            g_LinkEntity(ent);
        if ((ent->flags & 0x1000) == 0)
            goto label_25;
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto label_24;
        }
        else if (ent->moverState == 8)
        {
label_24:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            goto label_25;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
label_25:
        if (ent->teammaster == ent || ent->teammaster == nullptr)
            SV_AdjustAreaPortalState(ent, 0);
        return;
    case 9u:  // MOVER_1TO2ROTATE
        ent->s.pos.trTime = level.time;
        ent->s.apos.trTime = level.time;
        ent->moverState = 8;
        memcpy(ent->s.apos.trBase, &ent->r.currentAngles, sizeof(ent->s.apos.trBase));
        ent->s.apos.trType = TR_STATIONARY;
        BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
        if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
            g_LinkEntity(ent);
        if ((ent->flags & 0x1000) == 0)
            goto label_38;
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto label_37;
        }
        else if (ent->moverState == 8)
        {
label_37:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            goto label_38;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
label_38:
        if (ent->activator == nullptr)
            ent->activator = ent;
        if ((char)ent->flags >= 0)
        {
            float v7 = ent->wait;
            if (v7 != -1000.0f)
            {
                ent->think = THINK__ReturnToPos1;
                ent->nextthink = level.time + (int)v7;
            }
        }
        else
        {
            ent->nextthink = 0;
            ent->active = 0;
            ent->think = THINK__ReturnToPos1;
        }
        return;
    default:
        break;
    }
    if (moverState != 10)
    {
        G_Error("Reached_BinaryMover: bad moverState");
        return;
    }
    // MOVER_2TO1ROTATE
    ent->moverState = 7;
    ent->s.pos.trTime = level.time;
    ent->s.apos.trTime = level.time;
    memcpy(ent->s.apos.trBase, &ent->r.currentAngles, sizeof(ent->s.apos.trBase));
    ent->s.apos.trType = TR_STATIONARY;
    BG_EvaluateTrajectory(&ent->s.pos, level.time, ent->r.currentOrigin);
    if ((ent->r.svFlags & 1) == 0 || ent->r.contents != 0)
        g_LinkEntity(ent);
    if ((ent->flags & 0x1000) != 0)
    {
        if (ent->moverState == 7)
        {
            if (ent->key != 0)
                goto disconnect2;
        }
        else if (ent->moverState == 8)
        {
disconnect2:
            PathNodeMgr::sInst->DisconnectPathsForEntity(ent);
            goto label_54;
        }
        PathNodeMgr::sInst->ConnectPathsForEntity(ent);
    }
label_54:
    Entity* v9 = EntityHandleDb::sInst.Find(640, hash_const.player);
    if (v9 != nullptr)
        SV_inPVS(&v9->r.currentOrigin, &ent->r.currentOrigin);
    ent->flags &= ~0x100u;
    if (ent->teammaster == ent || ent->teammaster == nullptr)
        SV_AdjustAreaPortalState(ent, 0);
}

// ea: 0x00462FE0
void SP_func_door(Entity* ent)
{
    Entity* v1 = ent;
    float speed = ent->speed;
    ent->blocked = 1;
    if (speed == 0.0f)
        v1->speed = 400.0f;
    if (v1->wait == 0.0f)
        v1->wait = 2.0f;
    static unsigned int sInit = 0;
    static unsigned int key_hash;
    static unsigned int lip_hash;
    static unsigned int dmg_hash;
    static unsigned int health_hash;
    v1->wait = v1->wait * 1000.0f;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        key_hash = HashString::CalcHash("key");
    }
    int key = 0;
    v1->key = G_SpawnInt(key_hash, 0, &key) != 0;
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        lip_hash = HashString::CalcHash("lip");
    }
    float lip = 8.0f;
    G_SpawnFloat(lip_hash, 8.0, &lip);
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        dmg_hash = HashString::CalcHash("dmg");
    }
    G_SpawnInt(dmg_hash, 2, &v1->damage);
    v1->pos1.v.m128_f32[0] = v1->r.currentOrigin.v.m128_f32[0];
    v1->pos1.v.m128_f32[1] = v1->r.currentOrigin.v.m128_f32[1];
    v1->pos1.v.m128_f32[2] = v1->r.currentOrigin.v.m128_f32[2];
    SV_SetBrushModel(v1);
    G_SetMovedir(&v1->r.currentAngles, &v1->movedir);
    float abs_movedir = (float)fabs(v1->movedir.v.m128_f32[0]);
    float v10 = (float)fabs(v1->movedir.v.m128_f32[1]);
    float v11 = (float)fabs(v1->movedir.v.m128_f32[2]);
    float v4 = ((((v1->r.maxs.v.m128_f32[2] - v1->r.mins.v.m128_f32[2]) * v11)
               + ((v1->r.maxs.v.m128_f32[1] - v1->r.mins.v.m128_f32[1]) * v10))
               + ((v1->r.maxs.v.m128_f32[0] - v1->r.mins.v.m128_f32[0]) * abs_movedir))
               - lip;
    v1->pos2.v.m128_f32[0] = (v1->movedir.v.m128_f32[0] * v4) + v1->pos1.v.m128_f32[0];
    v1->pos2.v.m128_f32[1] = (v1->movedir.v.m128_f32[1] * v4) + v1->pos1.v.m128_f32[1];
    v1->pos2.v.m128_f32[2] = (v1->movedir.v.m128_f32[2] * v4) + v1->pos1.v.m128_f32[2];
    if ((v1->spawnflags & 1) != 0)
    {
        float v5 = v1->pos2.v.m128_f32[0];
        float v6 = v1->pos2.v.m128_f32[1];
        float v7 = v1->pos2.v.m128_f32[2];
        v1->pos2.v.m128_f32[0] = v1->r.currentOrigin.v.m128_f32[0];
        v1->pos2.v.m128_f32[1] = v1->r.currentOrigin.v.m128_f32[1];
        v1->pos2.v.m128_f32[2] = v1->r.currentOrigin.v.m128_f32[2];
        v1->pos1.v.m128_f32[0] = v5;
        v1->pos1.v.m128_f32[1] = v6;
        v1->pos1.v.m128_f32[2] = v7;
        if (v1->closespeed != 0.0f)
        {
            float v8 = v1->speed;
            v1->speed = v1->closespeed;
            v1->closespeed = v8;
        }
    }
    if ((v1->spawnflags & 2) != 0)
        v1->flags |= 0x80u;
    InitMover(v1);
    if ((v1->flags & 0x10) == 0)
    {
        if ((sInit & 8) == 0)
        {
            sInit |= 8u;
            health_hash = HashString::CalcHash("health");
        }
        int health = 0;
        G_SpawnInt(health_hash, 0, &health);
        if (health != 0)
            v1->takedamage = 1;
    }
    v1->nextthink = level.time + 100;
    v1->think = THINK__finishSpawningKeyedMover;
}

// ea: 0x0047BC20
void Reached_ScriptMover(Entity* pEnt)
{
    trajectory_t* p_pos = &pEnt->s.pos;
    if (pEnt->s.pos.trType != TR_STATIONARY && pEnt->s.pos.trTime + pEnt->s.pos.trDuration <= level.time)
    {
        int bMoveFinished = ScriptMover_Updatemove(pEnt->speed, pEnt->delay, &pEnt->pos1);
        BG_EvaluateTrajectory(p_pos, level.time, pEnt->r.currentOrigin);
        g_LinkEntity(pEnt);
        if (bMoveFinished != 0)
            Scr_Notify(pEnt, hash_const.movedone, 0);
    }
    if (pEnt->s.apos.trType != TR_STATIONARY && pEnt->s.apos.trTime + pEnt->s.apos.trDuration <= level.time)
    {
        int bMoveFinisheda = ScriptMover_Updatemove(pEnt->closespeed, pEnt->random, &pEnt->movedir);
        BG_EvaluateTrajectory(&pEnt->s.apos, level.time, pEnt->r.currentAngles);
        g_LinkEntity(pEnt);
        if (bMoveFinisheda != 0)
        {
            pEnt->r.currentAngles.v.m128_f32[0] = AngleNormalize180(pEnt->r.currentAngles.v.m128_f32[0]);
            pEnt->r.currentAngles.v.m128_f32[1] = AngleNormalize360(pEnt->r.currentAngles.v.m128_f32[1]);
            pEnt->r.currentAngles.v.m128_f32[2] = AngleNormalize180(pEnt->r.currentAngles.v.m128_f32[2]);
            Scr_Notify(pEnt, hash_const.rotatedone, 0);
        }
    }
}

// ea: 0x004875F0
void G_RunMover(Entity* ent, int msec)
{
    if (ent->scripted != nullptr)
    {
        G_Animscripted_Think(ent);
        G_SetOrigin(ent, &ent->r.currentOrigin);
        G_SetAngle(ent, &ent->r.currentAngles);
        g_LinkEntity(ent);
        if (ent->scripted != nullptr)
        {
            ent->s.pos.trType = TR_INTERPOLATE;
            ent->s.apos.trType = TR_INTERPOLATE;
            memcpy(ent->s.pos.trDelta, &ent->r.currentOrigin, sizeof(ent->s.pos.trDelta));
            memcpy(ent->s.apos.trDelta, &ent->r.currentAngles, sizeof(ent->s.apos.trDelta));
            G_RunThink(ent, msec);
            return;
        }
        if (ent->s.pos.trType != TR_STATIONARY)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 614;
            AeAssert::gCurrentExpr = "ent->s.pos.trType == TR_STATIONARY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        if (ent->s.apos.trType != TR_STATIONARY)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 615;
            AeAssert::gCurrentExpr = "ent->s.apos.trType == TR_STATIONARY";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    if (ent->tagInfo == nullptr)
    {
        if ((ent->flags & 0x10) != 0)
        {
            if (ent->r.linked != 0)
            {
                unsigned int mHash = ent->mClassNameHash.mHash;
                if (mHash == hash_const.func_tramcar.mHash || mHash == hash_const.func_rotating.mHash)
                    SV_UnlinkEntity(ent);
            }
            return;
        }
        if (ent->s.pos.trType != TR_STATIONARY || ent->s.apos.trType != TR_STATIONARY)
            G_MoverTeam(ent);
    }
    if ((ent->flags & 0x20000000) != 0)
        G_SetEntityOceanHeight(ent);
    G_RunThink(ent, msec);
}

// ea: 0x004877A0
void Blocked_Door(Entity* ent, Entity* other)
{
    Entity* v2 = ent;
    if (other != nullptr)
    {
        if (other->sentient == nullptr)
        {
            if (other->s.eType == 4
                && (other->mClassName.mBlock == nullptr
                        ? strstr(&defaultFileName[0], "chair") != nullptr
                        : strstr((const char*)(other->mClassName.mBlock + 1), "chair") != nullptr))
            {
                G_Damage(other, ent, ent, nullptr, nullptr, 100000, 0, 20, HITLOC_NONE, -1);
            }
            else
            {
                G_FreeEntity(other, 0);
            }
            return;
        }
        int damage = ent->damage;
        if (damage != 0)
            G_Damage(other, ent, ent, nullptr, nullptr, damage, 0, 20, HITLOC_NONE, -1);
    }
    if ((ent->spawnflags & 4) == 0)
    {
        do
        {
            int v6 = 2 * level.time - v2->s.pos.trDuration - v2->s.pos.trTime;
            if (v2->moverState == 3)
                SetMoverState(v2, MOVER_2TO1, v6);
            else
                SetMoverState(v2, MOVER_1TO2, v6);
            SV_LinkEntity(v2);
            v2 = v2->teamchain;
        } while (v2 != nullptr);
    }
}

// ea: 0x00487890
void Blocked_DoorRotate(Entity* ent, Entity* other)
{
    Entity* v2 = ent;
    if (other != nullptr)
    {
        if (other->sentient == nullptr)
        {
            G_FreeEntity(other, 0);
            return;
        }
        if (other->health <= 0)
            G_Damage(other, ent, ent, nullptr, nullptr, 100000, 0, 20, HITLOC_NONE, -1);
        int damage = ent->damage;
        if (damage != 0)
            G_Damage(other, ent, ent, nullptr, nullptr, damage, 0, 20, HITLOC_NONE, -1);
    }
    if (ent != nullptr)
    {
        do
        {
            int v4 = 2 * level.time - v2->s.apos.trDuration - v2->s.apos.trTime;
            int flags = v2->flags;
            bool v6 = v2->moverState == 9;
            v2->s.apos.trTime = v4;
            v2->s.pos.trTime = v4;
            int gDuration = v2->gDuration;
            if (v6)
            {
                v2->moverState = 10;
                v2->s.apos.trBase[0] = v2->rotate.v.m128_f32[0] * v2->angle;
                int v8 = flags & 0x100;
                v2->s.apos.trBase[1] = v2->rotate.v.m128_f32[1] * v2->angle;
                if ((flags & 0x800) != 0)
                    v8 = 0;
                v2->s.apos.trBase[2] = v2->rotate.v.m128_f32[2] * v2->angle;
                float v9 = 1000.0f / gDuration;
                v2->s.apos.trDuration = gDuration;
                if (v8 != 0)
                {
                    v9 = v9 * 0.5f;
                    v2->s.apos.trDuration = 2 * gDuration;
                }
                v2->s.apos.trDelta[0] = (0.0f - v9) * v2->s.apos.trBase[0];
                v2->s.apos.trDelta[1] = v2->s.apos.trBase[1] * (0.0f - v9);
                v2->s.apos.trDelta[2] = v2->s.apos.trBase[2] * (0.0f - v9);
                v2->s.apos.trType = TR_LINEAR_STOP;
                v2->active = 0;
                BG_EvaluateTrajectory(&v2->s.pos, level.time, v2->r.currentOrigin);
            }
            else
            {
                int v11 = flags & 0x100;
                if ((flags & 0x800) != 0)
                    v11 = 0;
                v2->moverState = 9;
                v2->s.apos.trBase[2] = 0.0f;
                v2->s.apos.trBase[1] = 0.0f;
                v2->s.apos.trBase[0] = 0.0f;
                float v12;
                if (v11 != 0)
                {
                    v12 = 500.0f;
                    v2->s.apos.trDuration = 2 * gDuration;
                }
                else
                {
                    v12 = 1000.0f;
                    v2->s.apos.trDuration = gDuration;
                }
                float v14 = v2->angle * (v12 / gDuration);
                v2->s.apos.trDelta[0] = v2->rotate.v.m128_f32[0] * v14;
                v2->s.apos.trDelta[1] = v2->rotate.v.m128_f32[1] * v14;
                v2->s.apos.trDelta[2] = v2->rotate.v.m128_f32[2] * v14;
                v2->s.apos.trType = TR_LINEAR_STOP;
                BG_EvaluateTrajectory(&v2->s.pos, level.time, v2->r.currentOrigin);
            }
            if ((v2->r.svFlags & 1) == 0 || v2->r.contents != 0)
                g_LinkEntity(v2);
            if ((v2->flags & 0x1000) != 0)
            {
                if (v2->moverState == 7)
                {
                    if (v2->key == 0)
                    {
                        PathNodeMgr::sInst->ConnectPathsForEntity(v2);
                        goto label_40;
                    }
                }
                else if (v2->moverState != 8)
                {
                    PathNodeMgr::sInst->ConnectPathsForEntity(v2);
                    goto label_40;
                }
                PathNodeMgr::sInst->DisconnectPathsForEntity(v2);
            }
label_40:
            SV_LinkEntity(v2);
            v2 = v2->teamchain;
        } while (v2 != nullptr);
    }
}

// ea: 0x0048C770
void Use_BinaryMover(Entity* ent, Entity* other, Entity* activator)
{
    int nosound;
    bool v5;
    while (1)
    {
        int flags = ent->flags;
        nosound = 0;
        v5 = (flags & 0x100) != 0;
        if ((flags & 0x800) != 0)
            v5 = false;
        if (level.time <= 4000)
            nosound = 1;
        if ((flags & 0x10) == 0)
            break;
        if (v5)
            ent->teammaster->flags |= 0x100u;
        ent = ent->teammaster;
    }
    unsigned char moverState = ent->moverState;
    if ((moverState == 0 || moverState == 7) && IsBinaryMoverBlocked(ent, other, activator) != 0)
    {
        MatchTeamReverseAngleOnSlaves(ent, MOVER_1TO2ROTATE, level.time + 50);
        goto label_17;
    }
    unsigned char v7 = ent->moverState;
    ent->activator = activator;
    switch (v7)
    {
    case 0u:
        MatchTeam(ent, MOVER_1TO2, level.time + 50);
label_22:
        if (ent->teammaster == ent || ent->teammaster == nullptr)
            SV_AdjustAreaPortalState(ent, 1);
        return;
    case 7u:
        MatchTeam(ent, MOVER_1TO2ROTATE, level.time + 50);
        goto label_17;
    case 1u:
        if ((char)ent->flags >= 0)
        {
            float wait = ent->wait;
            if (wait != -1000.0f)
                ent->nextthink = level.time + (int)wait;
        }
        else
        {
            ent->nextthink = level.time + 50;
        }
        break;
    case 8u:
        if ((char)ent->flags >= 0)
            ent->nextthink = level.time + (int)ent->wait;
        else
            ent->nextthink = level.time + 50;
        break;
    case 4u:
    case 3u:
        Blocked_Door(ent, nullptr);
        break;
    case 0xAu:
    case 9u:
        Blocked_DoorRotate(ent, nullptr);
        break;
    default:
        break;
    }
    return;
label_17:
    if (nosound == 0 && !v5 && activator != nullptr && activator->sentient != nullptr)
        j_nullsub_17(activator, 0xC, 0, &ent->r.currentOrigin, 0.0f);
    goto label_22;
}

// ea: 0x0048C970
void Touch_DoorTrigger(Entity* ent, Entity* other)
{
    if (other->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
        AeAssert::gCurrentLine = 1617;
        AeAssert::gCurrentExpr = "other->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    Entity* mObject;
    unsigned int v2 = ent->parentHandle.mHandle.mVal & 0xFFF;
    if (v2 < 0x540
        && ent->parentHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey
        && (mObject = EntityHandleDb::sInst.mElements[v2].mObject) != nullptr)
    {
        if (mObject->key == 0)
        {
            unsigned char moverState = mObject->moverState;
            if (moverState == 0 || moverState == 7 || moverState == 10)
                Use_BinaryMover(mObject, ent, other);
        }
    }
    else
    {
        ent->touch = 0;
        ent->nextthink = level.time + 100;
        ent->think = THINK__G_FreeEntity;
    }
}

// ea: 0x0048CA50
void G_TryDoor(Entity* ent, Entity* other, Entity* activator)
{
    int v3 = ent->flags & 0x100;
    bool doorsound = false;
    if (ent->s.apos.trType == TR_STATIONARY && ent->s.pos.trType == TR_STATIONARY
        && ent->active == 0 && ent->key == 0)
    {
        Scr_NotifyFromEnt(ent, hash_const.trigger, activator);
        Entity* teammaster = ent->teammaster;
        Entity* v6;
        if (teammaster == nullptr || ent->team.is_empty() || ent == teammaster)
        {
            ent->active = 1;
            if (v3 != 0)
            {
                ent->flags |= 0x100u;
                v6 = activator;
            }
            else
            {
                v6 = activator;
                if (activator != nullptr)
                    doorsound = true;
            }
            Use_BinaryMover(ent, v6, v6);
        }
        else
        {
            bool v5 = v3 == 0;
            v6 = activator;
            teammaster->active = 1;
            if (v5)
                doorsound = activator != nullptr;
            else
                ent->teammaster->flags |= 0x100u;
            Use_BinaryMover(ent->teammaster, activator, activator);
        }
        if ((ent->flags & 0x800) != 0 || doorsound)
            j_nullsub_17(v6, 0xC, 0, &ent->r.currentOrigin, 0.0f);
    }
}

// ea: 0x00486590 (static helper)
int G_MoverPush(Entity* pusher, const float* move, const float* amove)
{
    // swept bounds
    math::Position3 mins;
    math::Position3 maxs;
    float v8, v9, v10, v11;
    if (pusher->r.currentAngles.v.m128_f32[0] == 0.0f
        && pusher->r.currentAngles.v.m128_f32[1] == 0.0f
        && pusher->r.currentAngles.v.m128_f32[2] == 0.0f
        && amove[0] == 0.0f && amove[1] == 0.0f && amove[2] == 0.0f)
    {
        v8 = pusher->r.absmin.v.m128_f32[0];
        v9 = pusher->r.absmin.v.m128_f32[1];
        v10 = pusher->r.absmin.v.m128_f32[2];
        v11 = pusher->r.absmax.v.m128_f32[2];
        mins.v.m128_f32[0] = v8 + move[0];
        mins.v.m128_f32[1] = pusher->r.absmin.v.m128_f32[1] + move[1];
        mins.v.m128_f32[2] = v10 + move[2];
        maxs.v.m128_f32[0] = pusher->r.absmax.v.m128_f32[0] + move[0];
        maxs.v.m128_f32[1] = pusher->r.absmax.v.m128_f32[1] + move[1];
        maxs.v.m128_f32[2] = v11 + move[2];
    }
    else
    {
        float radius = RadiusFromBounds(pusher->r.mins, pusher->r.maxs);
        v8 = pusher->r.currentOrigin.v.m128_f32[0] - radius;
        v9 = pusher->r.currentOrigin.v.m128_f32[1] - radius;
        v10 = pusher->r.currentOrigin.v.m128_f32[2] - radius;
        v11 = pusher->r.currentOrigin.v.m128_f32[2] + radius;
        mins.v.m128_f32[0] = v8 + move[0];
        mins.v.m128_f32[1] = v9 + move[1];
        mins.v.m128_f32[2] = v10 + move[2];
        maxs.v.m128_f32[0] = pusher->r.currentOrigin.v.m128_f32[0] + move[0] + radius;
        maxs.v.m128_f32[1] = pusher->r.currentOrigin.v.m128_f32[1] + move[1] + radius;
        maxs.v.m128_f32[2] = v11 + move[2];
    }
    float sweepMins[3] = { v8, v9, v10 };
    float sweepMaxs[3] = { maxs.v.m128_f32[0], maxs.v.m128_f32[1], maxs.v.m128_f32[2] };
    if (move[0] <= 0.0f)
        sweepMins[0] = move[0] + v8;
    else
        mins.v.m128_f32[0] = move[0] + mins.v.m128_f32[0];
    if (move[1] <= 0.0f)
        sweepMins[1] = v9 + move[1];
    else
        mins.v.m128_f32[1] = mins.v.m128_f32[1] + move[1];
    if (move[2] <= 0.0f)
        sweepMins[2] = move[2] + v10;
    else
        mins.v.m128_f32[2] = mins.v.m128_f32[2] + move[2];
    SV_UnlinkEntity(pusher);
    int num = CM_AreaEntities(*reinterpret_cast<math::Position3*>(sweepMins),
                              *reinterpret_cast<math::Position3*>(sweepMaxs),
                              entityList, 256, 100663680);
    pusher->r.currentOrigin.v.m128_f32[0] += move[0];
    pusher->r.currentOrigin.v.m128_f32[1] += move[1];
    pusher->r.currentOrigin.v.m128_f32[2] += move[2];
    pusher->r.currentAngles.v.m128_f32[0] += amove[0];
    pusher->r.currentAngles.v.m128_f32[1] += amove[1];
    pusher->r.currentAngles.v.m128_f32[2] += amove[2];
    SV_LinkEntity(pusher);
    int listed = 0;
    for (int i = 0; i < num; ++i)
    {
        unsigned int v23 = entityList[i].mHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v23 < 0x540 && entityList[i].mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v23].mKey)
            mObject = EntityHandleDb::sInst.mElements[v23].mObject;
        unsigned char eType = mObject->s.eType;
        if ((eType == 3 || eType == 2 || eType == 1 || eType == 11 || eType == 13 || mObject->physicsObject != 0)
            && (mObject->s.mGroundEntity.mHandle.mVal == pusher->mHandle.mHandle.mVal
                || (mObject->r.absmin.v.m128_f32[0] < mins.v.m128_f32[0]
                    && mObject->r.absmin.v.m128_f32[1] < mins.v.m128_f32[1]
                    && mObject->r.absmin.v.m128_f32[2] < mins.v.m128_f32[2]
                    && maxs.v.m128_f32[0] < mObject->r.absmax.v.m128_f32[0]
                    && maxs.v.m128_f32[1] < mObject->r.absmax.v.m128_f32[1]
                    && maxs.v.m128_f32[2] < mObject->r.absmax.v.m128_f32[2]
                    && G_TestEntityPosition(mObject, mObject->r.currentOrigin) == pusher)))
        {
            moveList[listed++] = entityList[i];
        }
    }
    for (int j = 0; j < listed; ++j)
    {
        unsigned int v28 = moveList[j].mHandle.mVal & 0xFFF;
        Entity* v29 = nullptr;
        if (v28 < 0x540 && moveList[j].mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v28].mKey)
            v29 = EntityHandleDb::sInst.mElements[v28].mObject;
        SV_UnlinkEntity(v29);
    }
    math::Position3 v40;
    math::Position3 v41;
    for (int i = 0; i < listed; ++i)
    {
        unsigned int v31 = moveList[i].mHandle.mVal;
        unsigned int v32 = v31 & 0xFFF;
        Entity* v33 = nullptr;
        if (v32 < 0x540 && v31 >> 12 == EntityHandleDb::sInst.mElements[v32].mKey)
            v33 = EntityHandleDb::sInst.mElements[v32].mObject;
        pushed_t* v34 = pushed_p;
        if (pushed_p >= &pushed[256])
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
            AeAssert::gCurrentLine = 413;
            AeAssert::gCurrentExpr = "pushed_p < &pushed[256]";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            v34 = pushed_p;
        }
        v34->ent = v33;
        v34->origin[0] = v33->r.currentOrigin.v.m128_f32[0];
        v34->origin[1] = v33->r.currentOrigin.v.m128_f32[1];
        v34->origin[2] = v33->r.currentOrigin.v.m128_f32[2];
        v34->deltayaw = amove[1];
        v41.v.m128_f32[0] = amove[0];
        v41.v.m128_f32[1] = amove[1];
        v41.v.m128_f32[2] = amove[2];
        v40.v.m128_f32[0] = move[0];
        v40.v.m128_f32[1] = move[1];
        v40.v.m128_f32[2] = move[2];
        ++pushed_p;
        if (G_TryPushingEntity(v33, pusher, v40, v41) != 0
            || v33->s.eType == 2
            || v33->s.eType == 13)
        {
            SV_LinkEntity(v33);
        }
        else
        {
            if (pusher->s.pos.trType != TR_SINE && pusher->s.apos.trType != TR_SINE)
                return 0;
            G_Damage(v33, pusher, pusher, nullptr, nullptr, 100000, 0, 20, HITLOC_NONE, -1);
        }
    }
    for (int k = 0; k < listed; ++k)
    {
        unsigned int v36 = moveList[k].mHandle.mVal & 0xFFF;
        Entity* v37 = nullptr;
        if (v36 >= 0x540
            || moveList[k].mHandle.mVal >> 12 != EntityHandleDb::sInst.mElements[v36].mKey
            || (v37 = EntityHandleDb::sInst.mElements[v36].mObject) == nullptr)
        {
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_syscalls.cpp";
            AeAssert::gCurrentLine = 220;
            AeAssert::gCurrentExpr = "ent";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
        SV_LinkEntity(v37);
    }
    return 1;
}

// ea: 0x00486FB0
void G_MoverTeam(Entity* ent)
{
    if ((_S68_2 & 1) == 0)
    {
        _S68_2 |= 1u;
        memset(entityList, 0, sizeof(entityList));
    }
    if ((_S68_2 & 2) == 0)
    {
        _S68_2 |= 2u;
        memset(moveList, 0, sizeof(moveList));
    }
    Entity* v3 = ent;
    Entity* v23 = nullptr;
    pushed_p = pushed;
    Entity* v4 = ent;
    if (ent != nullptr)
    {
        while (1)
        {
            float v18[3];
            math::Position3 angles;
            math::Position3 origin;
            float move[3];
            math::Position3 posEval;
            BG_EvaluateTrajectory(&v4->s.pos, level.time, posEval);
            v18[0] = posEval.v.m128_f32[0];
            v18[1] = posEval.v.m128_f32[1];
            v18[2] = posEval.v.m128_f32[2];
            BG_EvaluateTrajectory(&v4->s.apos, level.time, origin);
            float v5 = v18[0] - v4->r.currentOrigin.v.m128_f32[0];
            float delta[3] = { v5, v18[1] - v4->r.currentOrigin.v.m128_f32[1],
                               v18[2] - v4->r.currentOrigin.v.m128_f32[2] };
            float amove[3] = { origin.v.m128_f32[0] - v4->r.currentAngles.v.m128_f32[0],
                               origin.v.m128_f32[1] - v4->r.currentAngles.v.m128_f32[1],
                               origin.v.m128_f32[2] - v4->r.currentAngles.v.m128_f32[2] };
            if (G_MoverPush(v4, delta, amove) == 0)
                break;
            v4 = v4->teamchain;
            if (v4 == nullptr)
                goto label_20;
        }
        pushed_t* v11 = pushed_p - 1;
        if (pushed_p - 1 >= pushed)
        {
            do
            {
                Entity* v12 = v11->ent;
                v12->r.currentOrigin.v.m128_f32[0] = v11->origin[0];
                v12->r.currentOrigin.v.m128_f32[1] = v11->origin[1];
                v12->r.currentOrigin.v.m128_f32[2] = v11->origin[2];
                v12->s.pos.trBase[0] = v11->origin[0];
                v12->s.pos.trBase[1] = v11->origin[1];
                v12->s.pos.trBase[2] = v11->origin[2];
                if (v12->client != nullptr)
                {
                    v12->client->ps.delta_angles[1] -= (int)(v11->deltayaw * 182.04445f);
                    v12->client->ps.origin.v.m128_f32[0] = v11->origin[0];
                    v12->client->ps.origin.v.m128_f32[1] = v11->origin[1];
                    v12->client->ps.origin.v.m128_f32[2] = v11->origin[2];
                }
                else if (v12->actor != nullptr)
                {
                    j_nullsub_83(&v12->actor->CodeOrient, v12->actor->CodeOrient.fDesiredBodyYaw - v11->deltayaw);
                    j_nullsub_83(&v12->actor->ScriptOrient, v12->actor->ScriptOrient.fDesiredBodyYaw - v11->deltayaw);
                }
                SV_LinkEntity(v12);
                --v11;
            } while (v11 >= pushed);
            v3 = ent;
        }
        Entity* v15 = v3;
        do
        {
            int trTime = v15->s.apos.trTime;
            v15->s.pos.trTime += level.time - level.previousTime;
            v15->s.apos.trTime = level.time - level.previousTime + trTime;
            BG_EvaluateTrajectory(&v15->s.pos, level.time, v15->r.currentOrigin);
            BG_EvaluateTrajectory(&v15->s.apos, level.time, v15->r.currentAngles);
            SV_LinkEntity(v15);
            v15 = v15->teamchain;
        } while (v15 != nullptr);
        unsigned char blocked = v3->blocked;
        if (blocked != 0)
        {
            if (blocked >= 3u)
            {
                AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
                AeAssert::gCurrentLine = 534;
                AeAssert::gCurrentExpr = "ent->blocked > 0 && ent->blocked < BLOCKED_MAX";
                if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                    __debugbreak();
            }
            blockedtable[v3->blocked](v3, v23);
        }
    }
    else
    {
label_20:
        pushed_t* v6 = pushed_p - 1;
        if (pushed_p - 1 >= pushed)
        {
            do
            {
                Entity* v7 = v6->ent;
                if (v7->actor != nullptr)
                {
                    j_nullsub_60(v7->actor);
                    Sentient_InvalidateNearestNode(v7->actor->pSentient);
                }
                --v6;
            } while (v6 >= pushed);
            v3 = ent;
        }
        if (ent != nullptr)
        {
            do
            {
                if (v3->s.pos.trType != TR_STATIONARY && level.time >= v3->s.pos.trTime + v3->s.pos.trDuration)
                {
                    unsigned char reached = v3->reached;
                    if (reached != 0)
                    {
                        if (reached >= 3u)
                        {
                            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
                            AeAssert::gCurrentLine = 562;
                            AeAssert::gCurrentExpr = "part->reached > 0 && part->reached < REACHED_MAX";
                            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        reachedtable[v3->reached](v3);
                    }
                }
                if (v3->s.apos.trType != TR_STATIONARY && level.time >= v3->s.apos.trTime + v3->s.apos.trDuration)
                {
                    unsigned char v10 = v3->reached;
                    if (v10 != 0)
                    {
                        if (v10 >= 3u)
                        {
                            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
                            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_mover.cpp";
                            AeAssert::gCurrentLine = 575;
                            AeAssert::gCurrentExpr = "part->reached > 0 && part->reached < REACHED_MAX";
                            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                                __debugbreak();
                        }
                        reachedtable[v3->reached](v3);
                    }
                }
                v3 = v3->teamchain;
            } while (v3 != nullptr);
        }
    }
}

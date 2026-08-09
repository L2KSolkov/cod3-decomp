// ============================================================================
// g_spawn.cpp - entity spawning + spawn vars (g.o: g_spawn.cpp family)
// ============================================================================

#include "game/logic/g_local.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <new>

// .rdata @ 0xCD4298 (verified against XBE bytes; ent_field_t entries)
static ent_field_t gSpawnFields[] = {
    { "classname", 0x27C, F_BROCSTR, nullptr },
    { "origin", 0x150, F_VECTOR, nullptr },
    { "model", 0x270, F_MODEL, nullptr },
    { "spawnflags", 0x2C0, F_INT, nullptr },
    { "speed", 0x31C, F_FLOAT, nullptr },
    { "closespeed", 0x320, F_FLOAT, nullptr },
    { "target", 0x28C, F_BROCSTR, nullptr },
    { "targetname", 0x284, F_BROCSTR, nullptr },
    { "teamname", 0x318, F_STRING, nullptr },
    { "wait", 0x380, F_FLOAT, nullptr },
    { "random", 0x384, F_FLOAT, nullptr },
    { "count", 0x36C, F_INT, nullptr },
    { "health", 0x358, F_INT, nullptr },
    { "dmg", 0x360, F_INT, nullptr },
    { "angle", 0x160, F_VECTOR, nullptr },
    { "angles", 0x390, F_VECTOR, nullptr },
    { "degrees", 0x314, F_FLOAT, nullptr },
    { "time", 0x31C, F_FLOAT, nullptr },
    { "modelscale", 0x278, F_FLOAT, nullptr },
    { "key", 0x3B4, F_INT, nullptr },
    { "delay", 0x388, F_FLOAT, nullptr },
    { "shared", 0x36C, F_INT, nullptr },
    { "spawnitem", 0x3B8, F_BROCSTR, nullptr },
    { "groupname", 0x294, F_BROCSTR, nullptr },
    { "script_noteworthy", 0x29C, F_BROCSTR, nullptr },
    { "maxhealth", 0x35C, F_INT, nullptr },
    { "animname", 0x2A4, F_BROCSTR, nullptr },
    { "persistent_index", 0x3BE, F_SHORT, nullptr },
    { "takedamage", 0x2B8, F_INT, nullptr },
    { nullptr, 0, F_NONE, nullptr },
};

static char init;  // @ 0xEF357C (prepare_spawns one-time flag)

// ea: 0x0044BAD0
void SP_sound_blend(Entity* pSelf)
{
    pSelf->r.contents = 0;
    pSelf->s.pos.trType = TR_STATIONARY;
    pSelf->s.apos.trType = TR_STATIONARY;
    pSelf->s.eventParm = 0;
    pSelf->s.scale = 0;
    pSelf->r.svFlags |= 0x20;
    pSelf->s.eType = 8;
    pSelf->s.leanf = 0.0f;
}

// ea: 0x0044FD10
int G_SpawnString(unsigned int key, const char* defaultString, const char** out)
{
    int v3 = 0;
    if (level.numSpawnVars <= 0)
    {
        *out = defaultString;
        return 0;
    }
    while (key != level.spawnVars[v3].key)
    {
        if (++v3 >= level.numSpawnVars)
        {
            *out = defaultString;
            return 0;
        }
    }
    *out = level.spawnVars[v3].value;
    return 1;
}

// ea: 0x0044FD50
bool G_SpawnString(unsigned int key, const char** out)
{
    int v2 = 0;
    if (level.numSpawnVars <= 0)
        return 0;
    while (key != level.spawnVars[v2].key)
    {
        if (++v2 >= level.numSpawnVars)
            return 0;
    }
    *out = level.spawnVars[v2].value;
    return 1;
}

// ea: 0x0044FD90
int G_SpawnFloat(unsigned int key, float default_value, float* out)
{
    int v3 = 0;
    const char* second;
    int v6;
    if (level.numSpawnVars <= 0)
    {
        second = nullptr;
        v6 = 0;
    }
    else
    {
        while (key != level.spawnVars[v3].key)
        {
            if (++v3 >= level.numSpawnVars)
            {
                second = nullptr;
                v6 = 0;
                goto have_value;
            }
        }
        second = level.spawnVars[v3].value;
        v6 = 1;
    }
have_value:
    if (v6 != 0)
        *out = (float)atof(second);
    else
        *out = default_value;
    return v6;
}

// ea: 0x0044FE00
int G_SpawnInt(unsigned int key, int default_value, int* out)
{
    int v3 = 0;
    const char* second;
    int v6;
    if (level.numSpawnVars <= 0)
    {
        second = nullptr;
        v6 = 0;
    }
    else
    {
        while (key != level.spawnVars[v3].key)
        {
            if (++v3 >= level.numSpawnVars)
            {
                second = nullptr;
                v6 = 0;
                goto have_value;
            }
        }
        second = level.spawnVars[v3].value;
        v6 = 1;
    }
have_value:
    if (v6 != 0)
        *out = atoi(second);
    else
        *out = default_value;
    return v6;
}

// ea: 0x0044FE60
int G_SpawnVector(unsigned int key, const float* default_value, float* out)
{
    int v3 = 0;
    const char* second;
    int v6;
    if (level.numSpawnVars <= 0)
    {
        second = nullptr;
        v6 = 0;
    }
    else
    {
        while (key != level.spawnVars[v3].key)
        {
            if (++v3 >= level.numSpawnVars)
            {
                second = nullptr;
                v6 = 0;
                goto have_value;
            }
        }
        second = level.spawnVars[v3].value;
        v6 = 1;
    }
have_value:
    if (v6 != 0)
        sscanf(second, "%f %f %f", out, out + 1, out + 2);
    else
    {
        out[0] = default_value[0];
        out[1] = default_value[1];
        out[2] = default_value[2];
    }
    return v6;
}

// ea: 0x0044FEE0
void prepare_spawns()
{
    if (!init)
    {
        init = 1;
        for (unsigned int i = 0; i < 53; ++i)
        {
            gSpawnHashes[i].mHash = HashString::CalcHash(gSpawnStrings[i]);
        }
    }
}

// ea: 0x0044FF30
void G_ParseEntityField(const char* key, const char* value, Entity* ent)
{
    g_key = key;
    g_value = value;
    const ent_field_t* v3 = gSpawnFields;
    if (v3->name != nullptr)
    {
        while (Q_stricmp(v3->name, key) != 0)
        {
            ++v3;
            if (v3->name == nullptr)
                return;
        }
        char* base = (char*)ent;
        switch (v3->type)
        {
        case F_INT:
        case F_SHORT:
        case F_BYTE:
            *(int*)(base + v3->ofs) = atoi(value);
            break;
        case F_FLOAT:
            *(float*)(base + v3->ofs) = (float)atof(value);
            break;
        case F_STRING:
            AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
            AeAssert::gCurrentLine = 275;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("Is this still used? (CD)"))
                __debugbreak();
            break;
        case F_VECTOR:
        {
            float vec[3];
            sscanf(value, "%f %f %f", vec, &vec[1], &vec[2]);
            memcpy(base + v3->ofs, vec, 12);
            break;
        }
        case F_BROCSTR:
            ((Broc::string*)(base + v3->ofs))->clear();
            *((Broc::string*)(base + v3->ofs)) = value;
            break;
        default:
            return;
        }
    }
}

// ea: 0x004500F0
void G_DuplicateEntityFields(Entity* dest, const Entity* source)
{
    const ent_field_t* v2 = gSpawnFields;
    if (v2->name != nullptr)
    {
        do
        {
            char* dbase = (char*)dest;
            const char* sbase = (const char*)source;
            switch (v2->type)
            {
            case F_INT:
            case F_SHORT:
            case F_BYTE:
            case F_FLOAT:
                *(int*)(dbase + v2->ofs) = *(const int*)(sbase + v2->ofs);
                break;
            case F_VECTOR:
                memcpy(dbase + v2->ofs, sbase + v2->ofs, 12);
                break;
            case F_MODEL:
                *(void**)(dbase + v2->ofs) = *(void* const*)(sbase + v2->ofs);
                break;
            case F_BROCSTR:
                *((Broc::string*)(dbase + v2->ofs)) = *((const Broc::string*)(sbase + v2->ofs));
                break;
            default:
                break;
            }
            ++v2;
        } while (v2->name != nullptr);
    }
    dest->mClassNameHash.mHash = source->mClassNameHash.mHash;
    dest->mTargetHash = source->mTargetHash;
    dest->targetnameHash = source->targetnameHash;
    dest->mGroupNameHash = source->mGroupNameHash;
    dest->mScriptNoteworthyHash = source->mScriptNoteworthyHash;
}

// ea: 0x00450200
void G_CallSpawn()
{
    const char* classname = nullptr;
    G_SpawnString(classname_hash.mHash, &defaultFileName[0], &classname);
    if (classname == nullptr)
        G_Printf("G_CallSpawn: NULL classname\n");
}

// ea: 0x004504E0
void G_SpawnGEntityFromSpawnVars()
{
    bool result = ShouldConnectPaths();
    if (!result)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 782;
        AeAssert::gCurrentExpr = "0";
        result = AeAssert::IsIgnored();
        if (!result)
        {
            result = AeAssert::Assert("DEAD CODE");
            if (result)
                __debugbreak();
        }
    }
}

// ea: 0x00450530
char* G_AddSpawnVarToken(const char* string)
{
    unsigned int v1 = (unsigned int)strlen(string);
    if (level.numSpawnVarChars + v1 + 1 > 2048)
        G_Error("G_AddSpawnVarToken: MAX_SPAWN_VARS");
    char* result = &level.spawnVarChars[level.numSpawnVarChars];
    memcpy(&level.spawnVarChars[level.numSpawnVarChars], string, v1 + 1);
    level.numSpawnVarChars += v1 + 1;
    return result;
}

// ea: 0x00454120
Entity* G_Spawn(TPakId pakId)
{
    TPakId v1 = pakId;
    if (pakId == PAK_ID_INVALID || PakManager::sInst->IsUnloading(pakId))
        v1 = CurPakId();
    Entity* v2 = (Entity*)Entity::operator new(0x470u);
    Entity* result;
    if (v2 != nullptr)
        result = ::new (v2) Entity(v1);
    else
        result = nullptr;
    ++level.num_entities;
    return result;
}

// ea: 0x00458730
void SP_info_notnull(Entity* self)
{
    G_SetOrigin(self, &self->r.currentOrigin);
}

// ea: 0x00458750
void SP_info_notnull_big(Entity* self)
{
    G_SetOrigin(self, &self->r.currentOrigin);
}

// ea: 0x004587A0
Entity* G_SpawnSoundBlend()
{
    Entity* v0 = G_Spawn(PAK_ID_INVALID);
    v0->mClassName = str_const.sound_blend;
    v0->mClassNameHash.mHash = HashString(v0->mClassName).mHash;
    UpdateEntityHash(v0);
    v0->r.contents = 0;
    v0->s.pos.trType = TR_STATIONARY;
    v0->s.apos.trType = TR_STATIONARY;
    v0->s.eventParm = 0;
    v0->s.scale = 0;
    v0->r.svFlags |= 0x20;
    v0->s.eType = 8;
    v0->s.leanf = 0.0f;
    return v0;
}

// ea: 0x00458820
void G_SetSoundBlend(Entity* ent, int alias0, int alias1, float lerp)
{
    if (ent == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 189;
        AeAssert::gCurrentExpr = "ent";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (ent->s.eType != 8)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 190;
        AeAssert::gCurrentExpr = "ent->s.eType == ET_SOUND_BLEND";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (alias0 != (alias0 & 0xFF))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 191;
        AeAssert::gCurrentExpr = "alias0 == (byte)alias0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", alias0))
            __debugbreak();
    }
    if (alias1 != (alias1 & 0xFF))
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_misc.cpp";
        AeAssert::gCurrentLine = 192;
        AeAssert::gCurrentExpr = "alias1 == (byte)alias1";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("%i", alias1))
            __debugbreak();
    }
    ent->s.eventParm = (uint8_t)alias0;
    ent->s.scale = (uint8_t)alias1;
    ent->s.leanf = lerp;
    g_LinkEntity(ent);
}

// ea: 0x00458970
void use_corona(Entity* ent)
{
    if (ent->r.linked != 0)
    {
        SV_UnlinkEntity(ent);
    }
    else
    {
        ent->active = 0;
        g_LinkEntity(ent);
    }
}

// ea: 0x00459780
void misc_spawner_use(Entity* ent)
{
    ent->think = THINK__turret_think_init;
    ent->nextthink = level.time + 1;
    g_LinkEntity(ent);
}

// ea: 0x004597B0
void SP_misc_spawner(Entity* ent)
{
    if (ent->mSpawnItem.mBlock != nullptr)
    {
        ent->use = 2;
        g_LinkEntity(ent);
    }
    else
    {
        G_Printf("-----> WARNING <-------\n");
        G_Printf("misc_spawner at loc %s has no spawnitem!\n", vtos(&ent->r.currentOrigin));
    }
}

// ea: 0x0045BD90
void SP_skyportal(Entity* ent)
{
    static unsigned int sInit = 0;
    static unsigned int fov_hash;
    static unsigned int fogcolor_hash;
    static unsigned int fognear_hash;
    static unsigned int fogfar_hash;
    if ((sInit & 1) == 0)
    {
        sInit |= 1u;
        fov_hash = HashString::CalcHash("fov");
    }
    float fov_x;
    G_SpawnFloat(fov_hash, 90.0, &fov_x);
    float def_val[3] = { 0.0f, 0.0f, 0.0f };
    if ((sInit & 2) == 0)
    {
        sInit |= 2u;
        fogcolor_hash = HashString::CalcHash("fogcolor");
    }
    if ((sInit & 4) == 0)
    {
        sInit |= 4u;
        fognear_hash = HashString::CalcHash("fognear");
    }
    if ((sInit & 8) == 0)
    {
        sInit |= 8u;
        fogfar_hash = HashString::CalcHash("fogfar");
    }
    float fogv[3];
    int fogn;
    int fogf;
    int v1 = G_SpawnVector(fogcolor_hash, def_val, fogv);
    int v2 = G_SpawnInt(fognear_hash, 0, &fogn) + v1;
    int v3 = G_SpawnInt(fogfar_hash, 300, &fogf);
    const char* v4 = va("%.2f %.2f %.2f %.1f %i %.2f %.2f %.2f %i %i",
                        ent->r.currentOrigin.v.m128_f32[0],
                        ent->r.currentOrigin.v.m128_f32[1],
                        ent->r.currentOrigin.v.m128_f32[2],
                        fov_x,
                        v3 + v2,
                        fogv[0], fogv[1], fogv[2],
                        fogn,
                        fogf);
    SV_SetConfigstring(10, v4);
}

// ea: 0x0045EAB0
void G_ReplaceSpawnVars(const InplaceVector<InplaceTreeElement<unsigned int, InplaceString>>* keyValuePairs)
{
    level.numSpawnVars = 0;
    level.numSpawnVarChars = 0;
    for (unsigned int v1 = 0; v1 < keyValuePairs->mSize; ++v1)
    {
        unsigned int v3 = v1 < keyValuePairs->mSize ? v1 : 0;
        level.spawnVars[level.numSpawnVars].key = keyValuePairs->mList[v3].mKey;
        unsigned int v4 = v1 < keyValuePairs->mSize ? v1 : 0;
        level.spawnVars[level.numSpawnVars++].value = keyValuePairs->mList[v4].mVal.mStr;
    }
}

// ea: 0x00462350
void SP_info_null(Entity* self)
{
    G_FreeEntity(self, 0);
}

// ea: 0x00462370
void SP_light(Entity* self)
{
    G_FreeEntity(self, 0);
}

// ea: 0x00462390
void SP_misc_model(Entity* ent)
{
    G_FreeEntity(ent, 0);
}

// ea: 0x004623B0
void SP_corona(Entity* ent)
{
    G_FreeEntity(ent, 0);
}

// ea: 0x004655C0
Entity* G_PickTarget(unsigned short targetname)
{
    if (targetname != 0)
    {
        ae_sized_array<Entity*, 4096> results;
        results.m_size = 0;
        EntityHandleDb_Find<unsigned short>(644, targetname, results);
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_utils.cpp";
        AeAssert::gCurrentLine = 1739;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Is this still used? (CD)"))
            __debugbreak();
        G_Printf("G_PickTarget: target <not used> not found\n");
    }
    return nullptr;
}

// ea: 0x00470500
void G_DuplicateScriptFields(Entity* dest, const Entity* source)
{
    unsigned int v2 = dest->mHandle.mHandle.mVal & 0xFFF;
    Entity* mObject = nullptr;
    if (v2 < 0x540 && dest->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v2].mKey)
        mObject = EntityHandleDb::sInst.mElements[v2].mObject;
    if (mObject != dest)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 406;
        AeAssert::gCurrentExpr = "*dest->GetHandle() == dest";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    unsigned int v4 = source->mHandle.mHandle.mVal & 0xFFF;
    Entity* v5 = nullptr;
    if (v4 < 0x540 && source->mHandle.mHandle.mVal >> 12 == EntityHandleDb::sInst.mElements[v4].mKey)
        v5 = EntityHandleDb::sInst.mElements[v4].mObject;
    if (v5 != source)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_spawn.cpp";
        AeAssert::gCurrentLine = 407;
        AeAssert::gCurrentExpr = "*source->GetHandle() == source";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    BrocSys::CopyExtendedEntity(source, dest);
}

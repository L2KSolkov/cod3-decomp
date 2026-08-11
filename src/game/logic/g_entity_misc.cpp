// ============================================================================
// g_entity_misc.cpp - game.o stat monitor + Entity DObj/enemy helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"
#include "core/PoolAllocator.h"
#include "core/color.h"
#include "core/tlFixedString.h"

#include <new>
#include <stdio.h>
#include <string.h>

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);

extern int g_uniqueEntityIndex;  // ?g_uniqueEntityIndex@@3HA (game.o @ 0xF4F444)

// ============================================================================
// Stat monitor (StatMon_*.cpp)
// ============================================================================
struct statmonitor_s {
    int endtime;   // +0x00
    void* shader;  // +0x04 (nglTexture*)
};
static statmonitor_s stats[64];   // 0xF3C100 (game.o data)
static int statCount;             // 0xF3C300 (game.o data)
extern cvar_t* com_statmon;       // ?com_statmon@@3PAUcvar_t@@A (core.o)
extern int dword_F170E0;          // game.o data
extern void* GetTextureData(const char* name, int image_type,
                            const char* fromPak);  // ?GetTextureData (render.o)
extern int Sys_Milliseconds();    // ?Sys_Milliseconds@@YAHXZ

// ============================================================================
// StatMon_Warning - ea: 0x611C20
// ============================================================================
// ea: 0x00611C20
void StatMon_Warning(int type, int duration, const char* pszShaderName)
{
    if (com_statmon->integer != 0)
    {
        if (type >= 0x40)
            Com_Error(ERR_DROP, "StatMon_UpdateEntry: invalid entry '%i'",
                      type);
        stats[type].endtime = duration + Sys_Milliseconds();
        if (stats[type].shader == 0 && dword_F170E0 != 0)
            stats[type].shader =
                GetTextureData(pszShaderName, 0, "mp_frontEnd");
        if (type >= statCount)
            statCount = type + 1;
    }
}

// ============================================================================
// StatMon_GetStatsArray - ea: 0x611CA0
// ============================================================================
// ea: 0x00611CA0
void StatMon_GetStatsArray(const statmonitor_s** array, int* count)
{
    *array = stats;
    *count = statCount;
}

// ============================================================================
// StatMon_Reset - ea: 0x611CC0
// ============================================================================
// ea: 0x00611CC0
int StatMon_Reset()
{
    memset(stats, 0, sizeof(stats));
    statCount = 0;
    return 0;
}

// ============================================================================
// Entity::CreateDObj - ea: 0x611CE0
// ============================================================================
extern void register_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle);  // ?register_dobj (g.o)
extern void DObjCreate(DObjModel* models, unsigned short numModels,
                       XAnimTree* tree, DObj* dobj,
                       unsigned short gameId);  // ?DObjCreate (render.o)
extern biped_phys_info* create_biped_phys_info(Entity* owner);  // physics.o
extern void destroy_biped_phys_info(biped_phys_info* bp_info);  // physics.o
extern void Entity_set_bp_info(Entity* self, biped_phys_info* bpInfo);  // ?set_bp_info@Entity@@QAEXPAVbiped_phys_info@@@Z (game.o)

void Entity::CreateDObj(DObjModel* dobjModels, unsigned short numModels,
                        XAnimTree* tree, unsigned short gameId)
{
    if (this->mDObj == nullptr)
    {
        void* v6 = DObj::operator new(0xE8u);
        if (v6 != nullptr)
            this->mDObj = new (v6) DObj(this->mPakId);
        else
            this->mDObj = nullptr;
        register_dobj(this->mHandle);
    }
    this->mDObj->mEntity = this;
    DObjCreate(dobjModels, numModels, tree, this->mDObj, gameId);
    if (this->client != nullptr && this->mBPInfo == nullptr)
    {
        biped_phys_info* bp = create_biped_phys_info(this);
        Entity_set_bp_info(this, bp);
    }
}

// ============================================================================
// Entity::FreeDObj - ea: 0x611DB0
// ============================================================================
extern void DObjFree(void* obj, int bClearTree);  // ?DObjFree (render.o)
extern void unregister_dobj(DbLinkedHandle<EntityHandleDb, Entity> handle);  // ?unregister_dobj (g.o)

void Entity::FreeDObj(bool deleteDObjs)
{
    DObj* mDObj = this->mDObj;
    if (mDObj != nullptr)
    {
        DObjFree(mDObj, 0);
        if (deleteDObjs)
        {
            if (this->mBPInfo != nullptr)
            {
                destroy_biped_phys_info(this->mBPInfo);
                this->mBPInfo = nullptr;
            }
            DObj* v4 = this->mDObj;
            if (v4 != nullptr)
            {
                this->mDObj->~DObj();
                DObj::operator delete(v4);
            }
            this->mDObj = nullptr;
            unregister_dobj(this->mHandle);
        }
    }
}

// ============================================================================
// Entity::IsEnemy - ea: 0x611E30
// ============================================================================
bool Entity::IsEnemy(Entity* ent)
{
    if (ent == nullptr || ent->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 618;
        AeAssert::gCurrentExpr = "ent && ent->sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    if (this->sentient == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 619;
        AeAssert::gCurrentExpr = "sentient";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    team_t eTeam = (team_t)this->sentient->eTeam;
    return eTeam != TEAM_NEUTRAL
        && (team_t)ent->sentient->eTeam != TEAM_NEUTRAL
        && eTeam != (team_t)ent->sentient->eTeam;
}

// ============================================================================
// DecodeStub - ea: 0x611B60
// ============================================================================
// ea: 0x00611B60
void DecodeStub(const char* name, unsigned char* data, int size,
                TPakId pakId, PakFile* pakFile)
{
}

// ============================================================================
// CGBankManager::UnloadAll - ea: 0x611C00
// ============================================================================
void CGBankManager::UnloadAll()
{
    this->mCount = 0;
}

// ============================================================================
// render_brush - ea: 0x611C10
// ============================================================================
// ea: 0x00611C10
void render_brush()
{
}

// ============================================================================
// debug_brush + debug_brushes (game.o CollisionMgr debug rendering)
// ============================================================================
class cdlBrush {
public:
    uint8_t _pad[0x60];  // opaque (cdlBrush); debug_brush stores the pointer
};
struct debug_brush {
    const cdlBrush* brush;      // +0x00
    math::Mat43        mat;     // +0x10
    Color              color;   // +0x50
    debug_brush(const cdlBrush& _brush, const math::Mat43& _mat,
                const Color& _color)
        : brush(&_brush), mat(_mat), color(_color)
    {
    }
};
static_assert(sizeof(debug_brush) == 0x60, "debug_brush size mismatch");

ae_vector<debug_brush> debug_brushes;  // ?debug_brushes@@3V?$ae_vector@Udebug_brush@@@@A (game.o)

// ea: 0x00611C10 (empty stub)
void render_brush(const debug_brush& dbrush)
{
}

// ============================================================================
// render_brush (cdlBrush + Mat43 + Color) - ea: 0x6389D0
// ============================================================================
// ea: 0x006389D0
void render_brush(const cdlBrush& brush, const math::Mat43& mat,
                  const Color& color)
{
    debug_brush db(brush, mat, color);
    if (debug_brushes.mSize >= debug_brushes.mCapacity)
    {
        // grow (ae_vector growth: capacity * 2)
        int newCap = debug_brushes.mCapacity == 0
            ? 4
            : debug_brushes.mCapacity * 2;
        debug_brush* nb = (debug_brush*)tlMemAlloc(
            newCap * sizeof(debug_brush), 8, 0);
        for (int i = 0; i < debug_brushes.mSize; ++i)
            nb[i] = debug_brushes.mElements[i];
        if (debug_brushes.mElements != nullptr)
            tlMemFree(debug_brushes.mElements);
        debug_brushes.mElements = nb;
        debug_brushes.mCapacity = newCap;
    }
    debug_brushes.mElements[debug_brushes.mSize] = db;
    ++debug_brushes.mSize;
}

// ============================================================================
// render_brush (plane set) - ea: 0x638A10
// Render-layer externs used by the debug brush/box drawing.
// ============================================================================
struct nglShaderParamSet {
    unsigned char mData[4];  // minimal (4 bytes; full type in ngl_dx_gpu.h)
    static unsigned int NumParams;  // ?NumParams@nglShaderParamSet@@2IA (ngl_params.o)
};
struct gpuVertexFormat {
    int      VertexSize;         // +0x00
    const void* Elements;        // +0x04
    void*    VertexDeclaration;  // +0x08
};
struct nglMeshSection;
struct nglMesh;
struct nglMaterial;
template <typename T>
struct cdl_array {
    int m_count;     // +0x00
    T*   m_elements; // +0x04
    void resize(unsigned int n);  // ?resize@?$cdl_array@UcdlPlane@@@@QAEXI@Z
};

extern gpuVertexFormat cddebug_vertex_format;
    // ?cddebug_vertex_format@@3UgpuVertexFormat@@A (render_xboxr)
extern void setup_color(const Color& i_col, nglShaderParamSet& o_params);
    // ?setup_color@@YAXABVColor@@AAUnglShaderParamSet@@@Z (render.o)
extern nglMesh* auxCreateScratchMesh(int flags, int num);  // ?auxCreateScratchMesh (ngl_aux.o)
extern nglMeshSection* nglCreateScratchSection(
    int Prim, int NIndices, int NVertices, gpuVertexFormat* VertexFormat);
    // ?nglCreateScratchSection (ngl_dx_gpu.h)
extern void nglAddMeshSection(nglMesh* Mesh, nglMeshSection* Section,
                              nglMaterial* Material, int Flags);
    // ?nglAddMeshSection@@YAXPAUnglMesh@@PAUnglMeshSection@@PAUnglMaterial@@H@Z
extern void* nglLockSectionIndices(nglMeshSection* Section);
extern unsigned char* nglLockSectionVertices(nglMeshSection* Section);
extern nglMesh* auxCloseScratchMesh(nglMesh* m);  // ?auxCloseScratchMesh (ngl_aux.o)
extern void* nglListAddMesh(nglMesh* Mesh, const math::Mat43* LocalToWorld,
                            void* MeshParams, void* ShaderParams,
                            void (*fn)(void*));
extern void j_nullsub_67(nglMeshSection* Section);  // render_xboxr no-op
extern void j_nullsub_27(nglMeshSection* Section);  // render_xboxr no-op
extern void calc_winding(const cdl_array<cdlPlane>& planes, int plane_index,
                         ae_sized_array<math::Position3, 256>& winding);
    // ?calc_winding (game.o 0x62A6B0)
extern unsigned char* nglListWork;
extern unsigned char* nglListWorkPos;
extern int nglListWorkSize;
extern int nglLastListAllocWarnFrame;
extern int nglFrame;
extern void tlFatal(const char* fmt, ...);   // tl_system.o
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* desc);     // tl_system.o
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20

// ea: 0x00638A10
void render_brush(const math::Position3& bmin, const math::Position3& bmax,
                  const cdlPlane* sides, unsigned int nsides,
                  const Color& color)
{
    cdl_array<cdlPlane> planes;
    planes.m_count = 0;
    planes.m_elements = nullptr;
    planes.resize(nsides + 6);
    if (nsides != 0)
    {
        for (unsigned int v6 = 0; v6 < nsides; ++v6)
        {
            if (v6 >= planes.m_count
                && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                             "index >= 0 && index < size()", "invalid index"))
                __debugbreak();
            planes.m_elements[v6] = sides[v6];
        }
    }
    // Six box planes (bounding box) appended after the side planes.
    static const __m128 s_negX = _mm_setr_ps(-1.0f, 0.0f, 0.0f, 0.0f);
    static const __m128 s_negY = _mm_setr_ps(0.0f, -1.0f, 0.0f, 0.0f);
    static const __m128 s_negZ = _mm_setr_ps(0.0f, 0.0f, -1.0f, 0.0f);
    static const __m128 s_posX = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    static const __m128 s_posY = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    static const __m128 s_posZ = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    {
        if (nsides >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = 0.0f - bmin.v.m128_f32[0];
            planes.m_elements[nsides].packed[0] = s_negX.m128_i32[0];
            planes.m_elements[nsides].packed[1] = s_negX.m128_i32[1];
            planes.m_elements[nsides].packed[2] = s_negX.m128_i32[2];
            planes.m_elements[nsides].packed[3] = *(int*)&d;
        }
        if (nsides + 1 >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = 0.0f - bmin.v.m128_f32[1];
            planes.m_elements[nsides + 1].packed[0] = s_negY.m128_i32[0];
            planes.m_elements[nsides + 1].packed[1] = s_negY.m128_i32[1];
            planes.m_elements[nsides + 1].packed[2] = s_negY.m128_i32[2];
            planes.m_elements[nsides + 1].packed[3] = *(int*)&d;
        }
        if (nsides + 2 >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = 0.0f - bmin.v.m128_f32[2];
            planes.m_elements[nsides + 2].packed[0] = s_negZ.m128_i32[0];
            planes.m_elements[nsides + 2].packed[1] = s_negZ.m128_i32[1];
            planes.m_elements[nsides + 2].packed[2] = s_negZ.m128_i32[2];
            planes.m_elements[nsides + 2].packed[3] = *(int*)&d;
        }
        if (nsides + 3 >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = bmax.v.m128_f32[0];
            planes.m_elements[nsides + 3].packed[0] = s_posX.m128_i32[0];
            planes.m_elements[nsides + 3].packed[1] = s_posX.m128_i32[1];
            planes.m_elements[nsides + 3].packed[2] = s_posX.m128_i32[2];
            planes.m_elements[nsides + 3].packed[3] = *(int*)&d;
        }
        if (nsides + 4 >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = bmax.v.m128_f32[1];
            planes.m_elements[nsides + 4].packed[0] = s_posY.m128_i32[0];
            planes.m_elements[nsides + 4].packed[1] = s_posY.m128_i32[1];
            planes.m_elements[nsides + 4].packed[2] = s_posY.m128_i32[2];
            planes.m_elements[nsides + 4].packed[3] = *(int*)&d;
        }
        if (nsides + 5 >= planes.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        {
            float d = bmax.v.m128_f32[2];
            planes.m_elements[nsides + 5].packed[0] = s_posZ.m128_i32[0];
            planes.m_elements[nsides + 5].packed[1] = s_posZ.m128_i32[1];
            planes.m_elements[nsides + 5].packed[2] = s_posZ.m128_i32[2];
            planes.m_elements[nsides + 5].packed[3] = *(int*)&d;
        }
    }
    for (int plane_index = 0; plane_index < planes.m_count; ++plane_index)
    {
        ae_sized_array<math::Position3, 256> winding;
        winding.m_size = 0;
        calc_winding(planes, plane_index, winding);
        int v42 = winding.m_size - 2;
        if (v42 > 0)
        {
            int nVertices = 3 * v42;
            int nIndices = 5 * v42 - 2;
            nglMesh* mesh = auxCreateScratchMesh(0x40000, 1);
            nglMeshSection* section = nglCreateScratchSection(
                6, nIndices, nVertices, &cddebug_vertex_format);
            nglAddMeshSection(
                mesh, section,
                *(nglMaterial**)((char*)&DebugRender_sInst + 0xC), 1);
            unsigned short* indices =
                (unsigned short*)nglLockSectionIndices(section);
            float* vertices = (float*)nglLockSectionVertices(section);
            int v48 = 0;
            for (int i = 2; i < winding.m_size; ++i)
            {
                if (i - 1 >= 0x100)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 154;
                    AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                float prevX = winding[i - 1].v.m128_f32[0];
                float prevY = winding[i - 1].v.m128_f32[1];
                float prevZ = winding[i - 1].v.m128_f32[2];
                float curX = winding[i].v.m128_f32[0];
                float curY = winding[i].v.m128_f32[1];
                float curZ = winding[i].v.m128_f32[2];
                if (i >= 0x100)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                    AeAssert::gCurrentLine = 154;
                    AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                if (v48 > 0)
                {
                    indices[0] = (unsigned short)(v48 - 1);
                    indices[1] = (unsigned short)v48;
                    indices += 2;
                }
                vertices[0] = winding[0].v.m128_f32[0];
                vertices[1] = winding[0].v.m128_f32[1];
                vertices[2] = winding[0].v.m128_f32[2];
                indices[0] = (unsigned short)v48;
                vertices[3] = curX;
                vertices[4] = curY;
                vertices[5] = curZ;
                indices[1] = (unsigned short)(v48 + 1);
                vertices[6] = prevX;
                vertices[7] = prevY;
                vertices[8] = prevZ;
                indices[2] = (unsigned short)(v48 + 2);
                indices += 3;
                vertices += 9;
                v48 += 3;
                j_nullsub_67(section);
                j_nullsub_27(section);
            }
            unsigned char* v58 = (unsigned char*)(
                ~7 & ((uintptr_t)nglListWorkPos + 7));
            unsigned int v59 = 4 * nglShaderParamSet::NumParams + 8;
            if (v58 + v59 <= nglListWork + nglListWorkSize)
            {
                nglListWorkPos = v58 + v59;
            }
            else
            {
                if (nglLastListAllocWarnFrame != nglFrame)
                {
                    tlFatal(
                        "Render list allocation overflow. Reserved = %d "
                        "Requested = %d Free = %d.\n",
                        nglListWorkSize, 4 * nglShaderParamSet::NumParams + 8,
                        nglListWork + nglListWorkSize - v58);
                    nglLastListAllocWarnFrame = nglFrame;
                }
                v58 = nullptr;
            }
            nglShaderParamSet* npolies = (nglShaderParamSet*)v58;
            *(unsigned int*)v58 = 0;
            *(unsigned int*)(v58 + 4) = 0;
            setup_color(color, *npolies);
            math::Mat43 identity;
            identity.x.v = s_posX;
            identity.y.v = s_posY;
            identity.z.v = s_posZ;
            identity.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
            nglMesh* m = auxCloseScratchMesh(mesh);
            nglListAddMesh(m, &identity, nullptr, npolies, nullptr);
        }
    }
    if (planes.m_elements != nullptr)
        tlMemFree(planes.m_elements);
}

// ============================================================================
// Entity helpers - ea: 0x611F00..0x639170
// ============================================================================
extern void AnglesToAxis(const math::Position3* angles,
                         const math::Position3* origin,
                         math::Mat43* mat);  // core.o (3-arg variant)
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME (core.o)

extern void XAnimClearTree(void* tree);  // ?XAnimClearTree@@YAXPAVXAnimTree@@@Z
extern Entity* EntityHandleDb_GetObject(unsigned int val);  // game.o

// ea: 0x006389B0
void DecodeCGBank(const char* name, unsigned char* data, int size,
                  TPakId pakId, PakFile* pakFile)
{
    ((CGBankManager*)CGBankManager::sInst)
        ->DecodeCGBank(name, data, size, pakId);
}

// ea: 0x00639180
void DisableAI(unsigned int handle)
{
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(handle);
    if (mObject != nullptr)
    {
        DObj* mDObj = mObject->mDObj;
        mObject->flags |= 0x4000000u;
        if (mDObj != nullptr)
        {
            int v4 = 0;
            unsigned char numModels = *(unsigned char*)((char*)mDObj + 0xCE);
            if (numModels != 0)
            {
                do
                {
                    void* tree = *(void**)((char*)mDObj + 4 * v4);
                    if (tree != nullptr)
                        XAnimClearTree(tree);
                    ++v4;
                } while (v4 < numModels);
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 852;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("null entity passed to DisableAI?"))
            __debugbreak();
    }
}

// ============================================================================
// Entity::~Entity - ea: 0x642D50 (Entity.cpp)
// ============================================================================
extern int Path_IsDynamicBlockingEntity(Entity* ent);  // mp_actors.o
extern void PathNodeMgr_ConnectPathsForEntity(void* self, Entity* ent);  // mp_actors.o
extern void* PathNodeMgr_sInst;   // ?sInst@PathNodeMgr@@2PAV1@A @ 0xF9930C
extern void G_EntUnlinkFree(Entity* ent);           // g.o
extern void StopPhysics(Entity* e);                 // g.o
extern void g_UnlinkEntity(Entity* ent);            // g.o
extern void G_DelayFreeAnimTree(void* tree);        // g.o
extern void j_nullsub_57(actor_s* actor);           // g.o
extern void Sentient_Free(sentient_s* sentient);    // mp_actors.o
extern void G_FreeEntityRefs(Entity* ed);           // g.o
extern actor_s* Actor_FirstActor(int iTeamFlags);   // mp_actors.o
extern actor_s* Actor_NextActor(actor_s* prev, int iTeamFlags);  // mp_actors.o
extern void j_nullsub_15(actor_s* self, Entity* other);          // g.o
extern sentient_s* Sentient_FirstSentient(int iTeamFlags);       // mp_actors.o
extern sentient_s* Sentient_NextSentient(sentient_s* prev, int iTeamFlags);  // mp_actors.o
extern void Sentient_DissociateEntity(sentient_s* self, Entity* other);  // mp_actors.o
extern void j_nullsub_77(Entity* ent);              // g.o
extern void G_FreeTurret(Entity* self);             // g.o
extern void G_FreeVehicle(Entity* ent);             // g.o
extern void BrocDestroyEntity(Entity* ent);         // broc
extern void EntityHandleDb_Release(void* self, Entity* e);  // game.o
extern void* EntityHandleDb_sInst;  // ?sInst@EntityHandleDb@@0V1@A @ 0xECBFE8
extern void* ScriptEventHandler_sAllocator;  // ?sAllocator@ScriptEventHandler@@0PAVPoolAllocator@@A @ 0xF049A4
extern void ScriptEventHandler_dtor(void* self);  // game2.o
extern void mem_heap_free(void* ptr);           // mem_lib
extern void EntityNotifySet_dtor(void* self);   // ?~EntityNotifySet (core.o)
extern void PakManager_MemFree(TPakId id, void* ptr, bool bUseActorHeap);  // ?MemFree@PakManager@@QAEXW4TPakId@@PAX_N@Z
extern void* EntityNotifySet_sAllocator;  // ?sAllocator@EntityNotifySet@@0PAVPoolAllocator@@A @ 0xF00E2C

// ea: 0x00642D50
Entity::~Entity()
{
    if (Path_IsDynamicBlockingEntity(this) != 0)
        PathNodeMgr_ConnectPathsForEntity(PathNodeMgr_sInst, this);
    if (this->disconnectedLinks != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 368;
        AeAssert::gCurrentExpr = "!this->disconnectedLinks";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (this->scripted != nullptr)
    {
        mem_heap_free(this->scripted);
        this->scripted = nullptr;
    }
    G_EntUnlinkFree(this);
    while (this->tagChildren != nullptr)
        G_EntUnlinkFree(this->tagChildren);
    StopPhysics(this);
    this->FreeDObj(true);
    g_UnlinkEntity(this);
    if (this->pAnimTree != nullptr)
    {
        G_DelayFreeAnimTree(this->pAnimTree);
        this->pAnimTree = nullptr;
    }
    if (this->actor != nullptr)
    {
        j_nullsub_57(this->actor);
        if (this->actor != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
            AeAssert::gCurrentLine = 416;
            AeAssert::gCurrentExpr = "this->actor == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    if (this->sentient != nullptr)
    {
        Sentient_Free(this->sentient);
        if (this->sentient != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
            AeAssert::gCurrentLine = 422;
            AeAssert::gCurrentExpr = "this->sentient == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    else
    {
        G_FreeEntityRefs(this);
        for (actor_s* i = Actor_FirstActor(-1); i != nullptr;
             i = Actor_NextActor(i, -1))
            j_nullsub_15(i, this);
        for (sentient_s* j = Sentient_FirstSentient(-1); j != nullptr;
             j = Sentient_NextSentient(j, -1))
            Sentient_DissociateEntity(j, this);
    }
    if (this->s.eType == 13)
        j_nullsub_77(this);
    if (this->pTurretInfo != nullptr)
    {
        G_FreeTurret(this);
        if (this->pTurretInfo != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
            AeAssert::gCurrentLine = 444;
            AeAssert::gCurrentExpr = "this->pTurretInfo == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    if (this->scr_vehicle != nullptr)
    {
        G_FreeVehicle(this);
        if (this->scr_vehicle != nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
            AeAssert::gCurrentLine = 450;
            AeAssert::gCurrentExpr = "this->scr_vehicle == 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("old cod assert"))
                __debugbreak();
        }
    }
    BrocDestroyEntity(this);
    this->SetAlwaysRender(false);
    int useCount = this->s.useCount;
    EntityHandleDb_Release(EntityHandleDb_sInst, this);
    TPakId mPakId = (TPakId)this->mPakId;
    this->s.useCount = useCount + 1;
    if (mPakId == PAK_ID_INVALID)
        mPakId = CurPakId();
    DObj* mDObj = this->mDObj;
    TPakId pakId = mPakId;
    if (mDObj != nullptr)
    {
        mDObj->~DObj();
        DObj::operator delete(mDObj);
    }
    EntityNotifySet* mNotifySet = this->mNotifySet;
    if (mNotifySet != nullptr)
    {
        EntityNotifySet_dtor(mNotifySet);
        ((PoolAllocator*)EntityNotifySet_sAllocator)->Release(mNotifySet);
    }
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
    {
        ScriptEventHandler_dtor(mScriptEventHandler);
        ((PoolAllocator*)ScriptEventHandler_sAllocator)
            ->Release(mScriptEventHandler);
    }
    proximity_data_t* proximity_data = this->proximity_data;
    this->mDObj = nullptr;
    this->mNotifySet = nullptr;
    this->mScriptEventHandler = nullptr;
    if (proximity_data != nullptr)
        PakManager_MemFree(pakId, proximity_data, false);
    EntityAnimationDebug* mAnimDebug = this->mAnimDebug;
    this->proximity_data = nullptr;
    mem_heap_free(mAnimDebug);
    trRefEntity* mRenderEntity = this->mRenderEntity;
    if (mRenderEntity != nullptr)
        *(int*)((char*)mRenderEntity + 0x100) = -1347440721;
    trRefEntity* v11 = this->mRenderEntity;
    if (v11 != nullptr)
    {
        --gRefEntFreeList.mUsed;
        ++gRefEntFreeList.mFree;
        *(void**)v11 = gRefEntFreeList.mpFree;
        gRefEntFreeList.mpFree = v11;
    }
    this->mRenderEntity = nullptr;
    for (int i = 0; i < 7; ++i)
        this->mAttachModels[i].~AttachModelInfo();
    this->mSpawnItem.~string();
    this->team.~string();
    this->mAnimName.~string();
    this->mScriptNoteworthy.~string();
    this->mGroupName.~string();
    this->mTarget.~string();
    this->targetname.~string();
    this->mClassName.~string();
}

// ea: 0x00639250
void EnableAI(unsigned int handle)
{
    Entity* mObject = (Entity*)EntityHandleDb_GetObject(handle);
    if (mObject != nullptr)
    {
        mObject->flags &= ~0x4000000u;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 873;
        AeAssert::gCurrentExpr = "e";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("null entity passed to EnableAI?"))
            __debugbreak();
    }
}

// ============================================================================
// Entity::CalcRotTranMat43 - ea: 0x611F40
// ============================================================================
// ea: 0x00611F40
const math::Mat43 Entity::CalcRotTranMat43()
{
    math::Mat43 result;
    AnglesToAxis(&this->r.currentAngles, &this->r.currentOrigin,
                 &this->r.currentMat);
    result = this->r.currentMat;
    return result;
}

// ============================================================================
// Entity::IsVisible - ea: 0x6122A0
// ============================================================================
// ea: 0x006122A0
int Entity::IsVisible() const
{
    DObj* mDObj = this->mDObj;
    return mDObj != nullptr
        && (g_DOBJF_NOT_RENDERED_LAST_FRAME & mDObj->mFlags) == 0;
}

// ============================================================================
// Entity::IsDoingPhysics - ea: 0x6122D0
// ============================================================================
// ea: 0x006122D0
bool Entity::IsDoingPhysics()
{
    bool result = false;
    if (this->client != nullptr)
    {
        biped_phys_info* mBPInfo = this->mBPInfo;
        if (mBPInfo != nullptr && mBPInfo->m_bp_sys != nullptr)
            return true;
    }
    return result;
}

// ============================================================================
// Entity::IsInRagdoll - ea: 0x612300
// ============================================================================
// ea: 0x00612300
bool Entity::IsInRagdoll()
{
    return (this->flags & 0x400000) != 0;
}

// ============================================================================
// Entity::IsLocalPlayer - ea: 0x612310
// ============================================================================
// ea: 0x00612310
bool Entity::IsLocalPlayer() const
{
    Client* client = this->client;
    return client != nullptr
        && (int)client->mServerClientIndex >= 0
        && *(int*)((char*)&svs.clients[client->mServerClientIndex].netchan[8])
            == 2;
}

// ============================================================================
// Entity::GetPlayerIndex - ea: 0x612340
// ============================================================================
// ea: 0x00612340
int Entity::GetPlayerIndex() const
{
    if (this->client == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 1041;
        AeAssert::gCurrentExpr = "client";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "Calling GetPlayerIndex on a non-player entity"))
            __debugbreak();
    }
    Client* client = this->client;
    if (client != nullptr)
        return client->mServerClientIndex;
    return -1;
}

// ============================================================================
// Entity::FreeAllDObjs - ea: 0x62A8A0
// ============================================================================
// ea: 0x0062A8A0
void Entity::FreeAllDObjs(bool deleteDObjs)
{
    Entity** p_mActiveList = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity** v2 = &EntityHandleDb::sInst.mActiveList.m_elements[
        EntityHandleDb::sInst.mActiveList.m_size];
    if (v2 != p_mActiveList)
    {
        do
        {
            if (*p_mActiveList != nullptr)
                (*p_mActiveList)->FreeDObj(deleteDObjs);
            ++p_mActiveList;
        } while (p_mActiveList != v2);
    }
}

// ============================================================================
// Entity::GetRenderEntity - ea: 0x62AF30
// ============================================================================
// ea: 0x00620240
void* trRefEntity::operator new(size_t s)
{
    return gRefEntFreeList.Alloc();
}

// ea: 0x00620250
void trRefEntity::operator delete(void* ptr)
{
    if (ptr != nullptr)
    {
        --gRefEntFreeList.mUsed;
        ++gRefEntFreeList.mFree;
        *(void**)ptr = gRefEntFreeList.mpFree;
        gRefEntFreeList.mpFree = (trRefEntity*)ptr;
    }
}

// ea: 0x0062AF30
trRefEntity& Entity::GetRenderEntity()
{
    if (this->mRenderEntity == nullptr)
    {
        trRefEntity* v2 = gRefEntFreeList.Alloc();
        this->mRenderEntity = v2 != nullptr
            ? new (v2) trRefEntity(0)
            : nullptr;
    }
    return *this->mRenderEntity;
}

// ============================================================================
// Entity::SetAnimDebug - ea: 0x62AFB0
// ============================================================================
extern void* mem_heap_malloc(unsigned int size);  // mem_lib
extern void AeStringSupport_CStrToAeStr(char* oBuff, int* oLen,
                                        int capacity,
                                        const char* src);
    // ?CStrToAeStr@AeStringSupport@@YAXPADAAHHPBD@Z
// mAnimNameResolver lives in BrocAPI::mBrocExports at +0xC58.
struct BrocAPI_AnimView {
    uint8_t _pad[0xC58];
    const char* (*mAnimNameResolver)(unsigned int);  // +0xC58
};

// ea: 0x0062AFB0
void Entity::SetAnimDebug(int lastAnim)
{
    const char* v2 = (const char*)(uintptr_t)lastAnim;
    const char* v4 =
        ((BrocAPI_AnimView*)gpBrocAPI)->mAnimNameResolver(
            (unsigned int)lastAnim);
    EntityAnimationDebug* mAnimDebug = this->mAnimDebug;
    if (mAnimDebug != nullptr)
    {
        mAnimDebug->prev2lastAnimPlayed = mAnimDebug->lastAnimPlayed;
        this->mAnimDebug->lastAnimPlayed = v2;
        int oLen = 0;
        char oBuff[64];
        AeStringSupport_CStrToAeStr(oBuff, &oLen, 63, v4);
        oBuff[63] = (char)oLen;
        memcpy(&this->mAnimDebug->lastAnimNamed, oBuff,
               sizeof(ae_fixed_string<64, unsigned char>));
    }
    else
    {
        EntityAnimationDebug* v6 =
            (EntityAnimationDebug*)mem_heap_malloc(0x48);
        if (v6 != nullptr)
        {
            v6->lastAnimPlayed = v2;
            v6->prev2lastAnimPlayed = (const char*)-1;
            int oLen = 0;
            AeStringSupport_CStrToAeStr((char*)v6->lastAnimNamed.mBuff,
                                        &oLen, 63, v4);
            v6->lastAnimNamed.mLength = (unsigned char)oLen;
            this->mAnimDebug = v6;
        }
        else
        {
            this->mAnimDebug = nullptr;
        }
    }
}

// ============================================================================
// Entity::SetInSnapshot - ea: 0x639170
// ============================================================================
// ea: 0x00639170
void Entity::SetInSnapshot()
{
    trRefEntity& RenderEntity = GetRenderEntity();
    RenderEntity.SetInSnapshot();
}

// ============================================================================
// Entity::IsInSnapshot - ea: 0x612280
// ============================================================================
// ea: 0x00612280
bool Entity::IsInSnapshot() const
{
    trRefEntity* mRenderEntity = this->mRenderEntity;
    return mRenderEntity != nullptr && mRenderEntity->IsInSnapshot();
}

// ============================================================================
// Entity::IsCameraTweening - ea: 0x6123B0
// ============================================================================
// ea: 0x006123B0
bool Entity::IsCameraTweening() const
{
    unsigned int PlayerIndex = (unsigned int)GetPlayerIndex();
    if (PlayerIndex >= 2)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 1052;
        AeAssert::gCurrentExpr = "index >= 0 && index <= 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                   "IsCameraTweening has bad player index"))
            __debugbreak();
    }
    return gCamera[PlayerIndex].IsTweening();
}

// ============================================================================
// GamePause - ea: 0x612680..0x612690
// ============================================================================
GamePause::GamePauseData GamePause::mData;

// ea: 0x00612680
GamePause::GamePauseData::GamePauseData()
{
    mGamePaused[0] = false;
}

// ea: 0x00612690
void GamePause::SetAllPaused(bool paused)
{
    GamePause::mData.mGamePaused[0] = paused;
}

// ============================================================================
// Entity::ExecScriptHandler - ea: 0x611F10
// ============================================================================
// ScriptEventHandler lives in g_game2_misc.cpp (game2.o port); params are
// ScriptEventParams* in the binary, void* in the tree's game2.o port.
struct ScriptEventHandler {
    unsigned char m_dlist_node[8];      // +0x00
    unsigned char mEvents[0x38];        // +0x08 (ScriptEvent mEvents[7])
    ScriptEventHandler* mNext;          // +0x40
    bool ExecEvents(Entity* ent, HashString h, void* params);  // game2.o 0x4F5A50
};

// ea: 0x00611F10
void Entity::ExecScriptHandler(HashString h, void* params)
{
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
        mScriptEventHandler->ExecEvents(this, h, params);
}

// ============================================================================
// Entity::GetViewModelDObj / CalcAbsMat / GetRelMat
// ea: 0x6205F0 / 0x620670 / 0x6208E0 (Entity.cpp)
// ============================================================================
extern int dword_F6A2A0[4 * 802];  // view-model DObj handles (game.o @ 0xF6A2A0)
extern const math::Mat43* DObj_GetMat(void* obj, int boneIndex);  // anim
extern serverStatic_t svs;  // sv.o

// ea: 0x006205F0
const DObj* Entity::GetViewModelDObj() const
{
    Client* client = this->client;
    if (client != nullptr
        && client->mServerClientIndex >= 0
        && svs.clients[client->mServerClientIndex].netchan[8] == 2)
    {
        return (const DObj*)dword_F6A2A0[802 * client->mServerClientIndex];
    }
    return nullptr;
}

// ea: 0x00620630
DObj* Entity::GetViewModelDObj()
{
    Client* client = this->client;
    if (client != nullptr
        && client->mServerClientIndex >= 0
        && svs.clients[client->mServerClientIndex].netchan[8] == 2)
    {
        return (DObj*)dword_F6A2A0[802 * client->mServerClientIndex];
    }
    return nullptr;
}

// ea: 0x00620670
math::Mat43 Entity::CalcAbsMat(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 721;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    math::Mat43 v14;
    if (this->mDObj != nullptr)
    {
        const math::Mat43* Mat = DObj_GetMat(this->mDObj, boneIndex);
        math::Dir3 v7;
        math::Dir3 v8;
        math::Dir3 v9;
        v7.v = this->r.currentMat.z.v;
        v8.v = this->r.currentMat.y.v;
        v9.v = this->r.currentMat.x.v;
        v14.x.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Mat->x.v, Mat->x.v, 0), v9.v),
                _mm_mul_ps(_mm_shuffle_ps(Mat->x.v, Mat->x.v, 85), v8.v)),
            _mm_mul_ps(_mm_shuffle_ps(Mat->x.v, Mat->x.v, 170), v7.v));
        v14.y.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Mat->y.v, Mat->y.v, 0), v9.v),
                _mm_mul_ps(_mm_shuffle_ps(Mat->y.v, Mat->y.v, 85), v8.v)),
            _mm_mul_ps(_mm_shuffle_ps(Mat->y.v, Mat->y.v, 170), v7.v));
        v14.z.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Mat->z.v, Mat->z.v, 0), v9.v),
                _mm_mul_ps(_mm_shuffle_ps(Mat->z.v, Mat->z.v, 85), v8.v)),
            _mm_mul_ps(_mm_shuffle_ps(Mat->z.v, Mat->z.v, 170), v7.v));
        v14.w.v = _mm_add_ps(
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Mat->w.v, Mat->w.v, 0), v9.v),
                _mm_mul_ps(_mm_shuffle_ps(Mat->w.v, Mat->w.v, 85), v8.v)),
            _mm_add_ps(
                _mm_mul_ps(_mm_shuffle_ps(Mat->w.v, Mat->w.v, 170), v7.v),
                this->r.currentMat.w.v));
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 725;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "Calc bone matrix on entity with no DObj."))
            __debugbreak();
        v14.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
        v14.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
        v14.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
        v14.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    }
    return v14;
}

// ea: 0x006208E0
math::Mat43 Entity::GetRelMat(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 736;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    math::Mat43 result;
    if (this->mDObj != nullptr)
    {
        result = *DObj_GetMat(this->mDObj, boneIndex);
        return result;
    }
    AeAssert::gCurrentAuthor = AeAssert::JRS;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
    AeAssert::gCurrentLine = 740;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Calc bone matrix on entity with no DObj."))
        __debugbreak();
    result.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    result.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    result.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    result.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    return result;
}

// ============================================================================
// Entity::SetAlwaysRender - ea: 0x620AE0 (Entity.cpp)
// ============================================================================
// FLAG 0x200000 = always render (byte_200000)
static ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 32>
    g_AlwaysRenderEnts;  // ?g_AlwaysRenderEnts@@3V?$ae_sized_array@V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@$0CA@@@A @ 0xDF8260

// ea: 0x00620AE0
void Entity::SetAlwaysRender(bool r)
{
    if (r)
    {
        if (this->mDObj == nullptr)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
            AeAssert::gCurrentLine = 767;
            AeAssert::gCurrentExpr = nullptr;
            if (AeAssert::Assert("No DObj on always render entity"))
                __debugbreak();
            return;
        }
        int flags = this->flags;
        if ((0x200000 & flags) == 0)
        {
            this->flags = 0x200000 | flags;
            unsigned int mVal = this->mHandle.mHandle.mVal;
            DbLinkedHandle<EntityHandleDb, Entity> h;
            h.mHandle.mVal = mVal;
            g_AlwaysRenderEnts.push_back(h);
            return;
        }
    }
    if ((0x200000 & this->flags) != 0 && !r)
    {
        int v3 = 0;
        unsigned int mVal = this->mHandle.mHandle.mVal;
        if (g_AlwaysRenderEnts.m_size > 0)
        {
            int m_size;
            while (1)
            {
                bool found =
                    g_AlwaysRenderEnts.m_elements[v3].mHandle.mVal == mVal;
                m_size = g_AlwaysRenderEnts.m_size;
                if (found)
                {
                    if (g_AlwaysRenderEnts.m_size != 0)
                        m_size = --g_AlwaysRenderEnts.m_size;
                    if (v3 != m_size)
                        break;
                }
                if (++v3 >= m_size)
                    return;
            }
            unsigned int v7 =
                g_AlwaysRenderEnts.m_elements[m_size].mHandle.mVal;
            g_AlwaysRenderEnts.m_elements[v3].mHandle.mVal = v7;
        }
    }
}

// ============================================================================
// Entity::FootStep - ea: 0x620C20 (Entity.cpp)
// ============================================================================
// ea: 0x00620C20
void Entity::FootStep()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    Client* client = this->client;
    if (client != nullptr
        && this != Player
        && (client->ps.pm_flags & 3) == 0
        && VectorDistanceSquared2D(&Player->r.currentOrigin,
                                   &this->r.currentOrigin) <= 1000000.0f)
    {
        Client* v5 = this->client;
        trace_t trace;
        memset(&trace, 0, sizeof(trace));
        float v16[3];
        math::Position3 start;
        v16[0] = v5->ps.origin.v.m128_f32[0];
        v16[1] = v5->ps.origin.v.m128_f32[1];
        v16[2] = v5->ps.origin.v.m128_f32[2] + 25.0f;
        start.v.m128_f32[0] = v5->ps.origin.v.m128_f32[0];
        start.v.m128_f32[1] = v5->ps.origin.v.m128_f32[1];
        start.v.m128_f32[2] = v5->ps.origin.v.m128_f32[2] - 50.0f;
        start.v.m128_f32[3] = 0.0f;
        unsigned int mVal = this->mHandle.mHandle.mVal;
        collision_context_t context(
            *(DbLinkedHandle<EntityHandleDb, Entity>*)&mVal, 0x800011);
        math::Position3 zero;
        zero.v = _mm_setzero_ps();
        trace_t results;
        g_Trace(&results, *(const math::Position3*)v16, zero, zero, start,
                context);
        int v6 = (int)((unsigned int)((int)trace.normal.v.m128_f32[2]
                                      >> 20)
                       & 0x1F);
        if (trace.normal.v.m128_f32[1] == 1.0f || v6 == 0)
            v6 = 6;
        Client* v7 = this->client;
        int pm_flags = v7->ps.pm_flags;
        if ((pm_flags & 1) != 0)
        {
            BG_AddPredictableEventToPlayerstate(v6 + 47, 0, &v7->ps);
        }
        else if ((pm_flags & 0x10000) != 0)
        {
            BG_AddPredictableEventToPlayerstate(v6 + 70, 0, &v7->ps);
        }
        else
        {
            int v10 = (pm_flags & 0x80) != 0 ? v6 + 24 : v6 + 1;
            BG_AddPredictableEventToPlayerstate(v10, 0, &v7->ps);
        }
    }
}

// ============================================================================
// Entity::has_zone_collision - ea: 0x620BE0 (Entity.cpp)
// ============================================================================
extern bool ShouldConnectPaths();  // core.o
extern bool BspTree_CellHasMeshFile(int cell_index);  // g_cm_load.cpp
// ea: 0x00620BE0
bool Entity::has_zone_collision() const
{
    if (ShouldConnectPaths())
        return true;
    int16_t cell_index = this->cell_index;
    return cell_index >= 0 && BspTree_CellHasMeshFile(cell_index);
}

// C-style bridge for cross-TU callers (g_client / g_scr_vehicle / g_main)
bool Entity_has_zone_collision(const void* self)
{
    return ((const Entity*)self)->has_zone_collision();
}

// ============================================================================
// Entity notify plumbing - ea: 0x62AD70..0x62AFA0 (Entity.cpp)
// The reserved_dlist layout is verified here: m_size +0x00, m_head +0x04,
// m_end +0x08, m_tail +0x0C; node m_next +0x00, m_prev +0x04.
// ============================================================================
// Layout twins of core_systems.h (core_systems.h can't be included with
// g_local.h). ctors/allocators are provided by core.o (ctor_dtor.cpp).
struct WaitTilOutput;
class EntityNotify {
public:
    unsigned char m_dlist_node[8];   // +0x00
    unsigned int  mStr;              // +0x08
    DbLinkedHandle<EntityHandleDb, Entity> mOwner;  // +0x0C
    WaitTilOutput* mParam;           // +0x10

    EntityNotify(unsigned int hashStr,
                 DbLinkedHandle<EntityHandleDb, Entity> ent,
                 WaitTilOutput* param);  // core.o 0x4BDAA0
    static PoolAllocator* sAllocator;    // core.o @ 0xF00E28
};
struct EntityNotifySet {
    unsigned char m_dlist_node[8];   // +0x00
    DbLinkedHandle<void, void> mEnt; // +0x08
    unsigned char mStrings[0x10];    // +0x0C
    unsigned char mEndOnList[0x10];  // +0x1C

    EntityNotifySet(Entity* e);      // core.o 0x4C1D80
    static PoolAllocator* sAllocator;    // core.o @ 0xF00E2C
};

struct NotifyDList {
    int   m_size;  // +0x00
    void* m_head;  // +0x04
    void* m_end;   // +0x08
    void* m_tail;  // +0x0C
};
struct NotifyNode {
    NotifyNode* m_next;  // +0x00
    NotifyNode* m_prev;  // +0x04
};

// ea: 0x0062AD70
void Entity::AddNotify(EntityNotify* notify)
{
    if (this->mNotifySet == nullptr)
    {
        void* v3 = EntityNotifySet::sAllocator->Allocate(0x2C, false);
        EntityNotifySet* v4 =
            v3 != nullptr ? new (v3) EntityNotifySet(this) : nullptr;
        this->mNotifySet = v4;
    }
    NotifyDList* strings =
        (NotifyDList*)((char*)this->mNotifySet + 0x0C);
    NotifyNode* node = (NotifyNode*)&notify->m_dlist_node;
    node->m_next = (NotifyNode*)strings->m_end;
    node->m_prev = (NotifyNode*)strings->m_tail;
    ((NotifyNode*)strings->m_tail)->m_next = node;
    strings->m_tail = node;
    ++strings->m_size;
}

// game.o data (Entity.cpp)
static int          sNotifyInitFlags;  // $S69_1 @ 0xF58C38
static unsigned int footstep;          // ?footstep @ 0xF58C34
static unsigned int step;              // ?step @ 0xF58C30

// ea: 0x0062AE00
void Entity::Notify(HashString h)
{
    if (h.mHash == 0)
        return;
    if (this->client != nullptr)
    {
        if ((sNotifyInitFlags & 1) == 0)
        {
            sNotifyInitFlags |= 1;
            footstep = HashString::CalcHash("footstep");
        }
        if ((sNotifyInitFlags & 2) == 0)
        {
            sNotifyInitFlags |= 2;
            step = HashString::CalcHash("step");
        }
        if (h.mHash == step || h.mHash == footstep)
            this->FootStep();
    }
    void* v3 = EntityNotify::sAllocator->Allocate(0x14, false);
    EntityNotify* v4 =
        v3 != nullptr
            ? new (v3) EntityNotify(h.mHash, this->mHandle, nullptr)
            : nullptr;
    NotifyDList* pending =
        (NotifyDList*)((char*)&AeThreadManager::sInst + 0x24);
    NotifyNode* node = (NotifyNode*)&v4->m_dlist_node;
    node->m_next = (NotifyNode*)pending->m_end;
    node->m_prev = (NotifyNode*)pending->m_tail;
    ((NotifyNode*)pending->m_tail)->m_next = node;
    pending->m_tail = node;
    ++pending->m_size;
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
        mScriptEventHandler->ExecEvents(this, h, nullptr);
}

// refEntity_t - leading member of trRefEntity (+0x00) - matches cg_local.h
struct refEntity_t {
    int   reType;          // +0x00
    int   renderfx;        // +0x04
    float lightingOrigin[3]; // +0x08
    float axis[3][3];      // +0x14
    float scale;           // +0x38
    float origin[3];       // +0x3C
    float oldorigin[3];    // +0x48
    void* obj;             // +0x54
    Entity* entity;        // +0x58
    void* pStaticModel;    // +0x5C
};

// ea: 0x0062AFA0
refEntity_t& Entity::GetRefEntity()
{
    return (refEntity_t&)this->GetRenderEntity();
}

// ============================================================================
// EntityState / EntityShared / Entity ctors - ea: 0x620280..0x62AD65
// ============================================================================

// ea: 0x00620280
EntityShared::EntityShared()
{
    this->linked = 0;
    this->svFlags = 0;
    this->mSingleClient.mHandle.mVal = 0;
    this->bmodel = nullptr;
    this->mins.v = _mm_setzero_ps();
    this->maxs.v = _mm_setzero_ps();
    this->absmin.v = _mm_setzero_ps();
    this->absmax.v = _mm_setzero_ps();
    this->contents = 0;
    this->currentOrigin.v = _mm_setzero_ps();
    this->currentAngles.v = _mm_setzero_ps();
    this->currentMat.x.v = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
    this->currentMat.y.v = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
    this->currentMat.z.v = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);
    this->currentMat.w.v = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
    this->mOwner.mHandle.mVal = 0;
    this->eventType = 0;
    this->eventTime = 0;
    this->worldSector = nullptr;
    this->nextEntityInWorldSector = nullptr;
    this->numClusters = 0;
    this->lastCluster = 0;
    this->areanum = 0;
    this->areanum2 = 0;
    this->linkcontents = 0;
    for (int i = 0; i < 16; ++i)
        this->clusternums[i] = 0;
    this->linkmin[0] = 0.0f;
    this->linkmin[1] = 0.0f;
    this->linkmax[0] = 0.0f;
    this->linkmax[1] = 0.0f;
}

// ea: 0x00620480
EntityState::EntityState()
{
    this->eType = 0;
    this->loopSound = 0;
    this->surfType = 0;
    this->weapon = 0;
    this->eventParm = 0;
    this->scale = 0;
    this->mOtherEntity.mHandle.mVal = 0;
    this->mGroundEntity.mHandle.mVal = 0;
    this->eFlags = 0;
    this->pos.trType = TR_STATIONARY;
    this->pos.trTime = 0;
    this->pos.trDuration = 0;
    this->pos.trGravityOverride = 0;
    this->pos.trBase[0] = 0.0f;
    this->pos.trBase[1] = 0.0f;
    this->pos.trBase[2] = 0.0f;
    this->pos.trDelta[0] = 0.0f;
    this->pos.trDelta[1] = 0.0f;
    this->pos.trDelta[2] = 0.0f;
    this->apos.trType = TR_STATIONARY;
    this->apos.trTime = 0;
    this->apos.trDuration = 0;
    this->apos.trGravityOverride = 0;
    this->apos.trBase[0] = 0.0f;
    this->apos.trBase[1] = 0.0f;
    this->apos.trBase[2] = 0.0f;
    this->apos.trDelta[0] = 0.0f;
    this->apos.trDelta[1] = 0.0f;
    this->apos.trDelta[2] = 0.0f;
    this->lerpOrigin.v = _mm_setzero_ps();
    this->lerpAngles.v = _mm_setzero_ps();
    this->origin2.v = _mm_setzero_ps();
    this->angles2.v = _mm_setzero_ps();
    this->constantLight = 0;
    this->solid = 0;
    this->eventSequence = 0;
    this->leanf = 0.0f;
    this->dmgFlags = 0;
    this->useCount = 0;
    this->eTeam = 0;
    this->brushmodel = 0;
    unsigned char* eventParms = this->eventParms;
    for (int i = 4; i != 0; --i)
    {
        *(eventParms - 4) = 0;
        *eventParms++ = 0;
    }
}

// ea: 0x0062A8E0
Entity::Entity(TPakId pakId)
    : s(), r(),
      mClassName((Broc::string::Block*)nullptr),
      targetname((Broc::string::Block*)nullptr),
      mTarget((Broc::string::Block*)nullptr),
      mGroupName((Broc::string::Block*)nullptr),
      mScriptNoteworthy((Broc::string::Block*)nullptr),
      mAnimName((Broc::string::Block*)nullptr),
      team((Broc::string::Block*)nullptr),
      mSpawnItem((Broc::string::Block*)nullptr)
{
    this->mPakId = pakId;
    this->mHandle.mHandle.mVal = 0;
    this->mEntityArrayIndex = -1;
    this->mDObj = nullptr;
    this->mNotifySet = nullptr;
    this->mScriptEventHandler = nullptr;
    this->mBPInfo = nullptr;
    this->mDestructible.mValue = nullptr;
    this->mDestructible.mPakId = (unsigned int)PAK_ID_INVALID;
    this->client = nullptr;
    this->actor = nullptr;
    this->sentient = nullptr;
    this->scr_vehicle = nullptr;
    this->pTurretInfo = nullptr;
    this->mRenderEntity = nullptr;
    this->pAnimTree = nullptr;
    this->mModel.mValue = nullptr;
    this->mModel.mPakId = (unsigned int)PAK_ID_INVALID;
    this->modelscale = 1.0f;
    this->mClassNameHash.mHash = 0;
    this->targetnameHash = 0;
    this->mTargetHash = 0;
    this->mGroupNameHash = 0;
    this->mHintString = 0;
    this->physicsObject = 0;
    this->noise_index = 0;
    this->active = 0;
    this->moverState = 0;
    this->attachIgnoreCollision = 0;
    this->takedamage = 0;
    this->invulnerability_timeout = 0;
    this->spawnflags = 0;
    this->flags = 0;
    this->mFlags = 0;
    this->clipmask = 0;
    this->processedFrame = 0;
    this->parentHandle.mHandle.mVal = 0;
    this->timestamp = 0;
    this->angle = 0.0f;
    this->speed = 0.0f;
    this->closespeed = 0.0f;
    this->gDuration = 0;
    this->gDurationBack = 0;
    this->nextthink = 0;
    this->think = THINK__NULL;
    this->reached = 0;
    this->blocked = 0;
    this->touch = 0;
    this->use = 0;
    this->pain = 0;
    this->die = 0;
    this->entinfo = 0;
    this->controller = 0;
    this->health = 0;
    this->maxHealth = 0;
    this->damage = 0;
    this->methodOfDeath = 0;
    this->splashMethodOfDeath = 0;
    this->count = 0;
    this->enemy = nullptr;
    this->activator = nullptr;
    this->teamchain = nullptr;
    this->teammaster = nullptr;
    this->wait = 0.0f;
    this->random = 0.0f;
    this->delay = 0.0f;
    this->item = nullptr;
    this->key = 0;
    this->cell_index = -1;
    this->mPersistentIndex = -1;
    this->count2 = 0;
    this->grenadeExplodeTime = 0;
    this->snd_wait.notifyHash.mHash = 0;
    this->snd_wait.soundName.mHash = 0;
    this->curve = nullptr;
    this->tagInfo = nullptr;
    this->tagChildren = nullptr;
    this->scripted = nullptr;
    for (int i = 0; i < 7; ++i)
        new (&this->mAttachModels[i]) AttachModelInfo();
    this->disconnectedLinks = 0;
    this->iDisconnectTime = 0;
    this->currentValid = 0;
    this->fireSndDelay = 0;
    this->isFiring = 0;
    this->effectLoopingFire.mVal = 0;
    this->previousEventSequence = 0;
    this->previousPreEventSequence = 0;
    this->mAnimDebug = nullptr;
    this->mBrocExtendedEntity = nullptr;
    this->proximity_data = nullptr;
    this->mClassNameHash.mHash = 0;
    this->snd_wait.notifyHash.mHash = 0;
    this->snd_wait.soundName.mHash = 0;
    this->pos1.v = _mm_setzero_ps();
    this->pos2.v = _mm_setzero_ps();
    this->pos3.v = _mm_setzero_ps();
    this->movedir.v = _mm_setzero_ps();
    this->rotate.v = _mm_setzero_ps();
    this->TargetAngles.v = _mm_setzero_ps();
    this->uniqueIndex = g_uniqueEntityIndex++;
    UpdateEntityHash(this);
    this->mClassName = str_const.noclass;
    HashString hs(this->mClassName);
    this->mClassNameHash.mHash = hs.mHash;
    this->r.mOwner.mHandle.mVal = 0;
    this->parentHandle.mHandle.mVal = 0;
    this->r.eventType = 0;
    this->r.eventTime = 0;
    this->spawnflags = 0;
    this->r.pos_cache.v.m128_f32[3] = 0.0f;
    this->mScriptNoteworthy.clear();
    this->targetname.clear();
    this->previousPreEventSequence = 0;
    this->previousEventSequence = 0;
    this->speed = -1.0f;
    UpdateEntityHash(this);
    EntityHandleDb::sInst.AssignHandle(*this);
    this->mFlags |= 1u;
}

// ============================================================================
// Entity::CalcOriginAnglesFromMat - ea: 0x611FE0
// ============================================================================
extern void Axis4ToAngles(const float (*axis)[4], float* angles);  // core.o

// ea: 0x00611FE0
void Entity::CalcOriginAnglesFromMat()
{
    Client* client = this->client;
    this->s.pos.trDelta[0] = 0.0f;
    this->s.pos.trDelta[1] = 0.0f;
    this->s.pos.trDelta[2] = 0.0f;
    this->s.apos.trDelta[0] = 0.0f;
    this->s.apos.trDelta[1] = 0.0f;
    this->s.apos.trDelta[2] = 0.0f;
    this->s.pos.trTime = 0;
    this->s.pos.trDuration = 0;
    this->s.apos.trTime = 0;
    this->s.apos.trDuration = 0;
    if (client != nullptr)
    {
        this->s.apos.trBase[0] = this->r.currentAngles.v.m128_f32[0];
        this->s.apos.trBase[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trBase[2] = this->r.currentAngles.v.m128_f32[2];
    }
    float tmp = this->r.currentAngles.v.m128_f32[0];
    float v7 = this->r.currentAngles.v.m128_f32[1];
    float v8 = this->r.currentAngles.v.m128_f32[2];
    Axis4ToAngles((const float(*)[4])&this->r.currentMat, &tmp);
    this->r.currentAngles.v.m128_f32[0] = tmp;
    this->r.currentAngles.v.m128_f32[1] = v7;
    this->r.currentAngles.v.m128_f32[2] = v8;
    this->r.currentAngles.v.m128_f32[0] =
        AngleNormalize180(this->r.currentAngles.v.m128_f32[0]);
    this->r.currentOrigin.v.m128_f32[0] = this->r.currentMat.w.v.m128_f32[0];
    this->r.currentOrigin.v.m128_f32[1] = this->r.currentMat.w.v.m128_f32[1];
    float v3 = this->r.currentMat.w.v.m128_f32[3];
    this->r.currentOrigin.v.m128_f32[2] = this->r.currentMat.w.v.m128_f32[2];
    this->r.currentOrigin.v.m128_f32[3] = v3;
    this->s.pos.trBase[0] = this->r.currentOrigin.v.m128_f32[0];
    this->s.pos.trBase[1] = this->r.currentOrigin.v.m128_f32[1];
    Client* v4 = this->client;
    this->s.pos.trBase[2] = this->r.currentOrigin.v.m128_f32[2];
    float v5 = this->r.currentAngles.v.m128_f32[0];
    if (v4 != nullptr)
    {
        this->s.apos.trDelta[0] = v5;
        this->s.apos.trDelta[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trDelta[2] = this->r.currentAngles.v.m128_f32[2];
    }
    else
    {
        this->s.apos.trBase[0] = v5;
        this->s.apos.trBase[1] = this->r.currentAngles.v.m128_f32[1];
        this->s.apos.trBase[2] = this->r.currentAngles.v.m128_f32[2];
    }
    this->s.pos.trType = TR_STATIONARY;
    this->s.apos.trType = TR_STATIONARY;
    if (v4 != nullptr)
    {
        v4->oldOrigin.v.m128_f32[0] = v4->ps.origin.v.m128_f32[0];
        this->client->oldOrigin.v.m128_f32[1] =
            this->client->ps.origin.v.m128_f32[1];
        this->client->oldOrigin.v.m128_f32[2] =
            this->client->ps.origin.v.m128_f32[2];
        this->client->ps.origin.v.m128_f32[0] =
            this->r.currentOrigin.v.m128_f32[0];
        this->client->ps.origin.v.m128_f32[1] =
            this->r.currentOrigin.v.m128_f32[1];
        this->client->ps.origin.v.m128_f32[2] =
            this->r.currentOrigin.v.m128_f32[2];
    }
    g_LinkEntity(this);
}

// ============================================================================
// Entity::GetParentBoneIndex - ea: 0x6121A0
// ============================================================================
// ea: 0x006121A0
int Entity::GetParentBoneIndex(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 750;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
            __debugbreak();
    }
    return this->mDObj->GetBoneParent(boneIndex);
}

// ============================================================================
// Entity::GetBaseRelMat - ea: 0x612210
// ============================================================================
// ea: 0x00612210
const math::Mat43::Packed& Entity::GetBaseRelMat(int boneIndex)
{
    if (this->mDObj == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\Entity.cpp";
        AeAssert::gCurrentLine = 757;
        AeAssert::gCurrentExpr = "mDObj";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(""))
            __debugbreak();
    }
    return this->mDObj->GetBaseRelMat(boneIndex);
}

// ============================================================================
// MusicMgr - ea: 0x612E30
// ============================================================================
struct MusicMgr {
    Handle mMusic;           // +0x00
    Handle mMusicIndoor;     // +0x04
    float  mVolScale;        // +0x08
    float  mOutsideScale;    // +0x0C
    float  mIndoorScale;     // +0x10
    float  mIndoorFadeTime;  // +0x14
    float  mDelayCount;      // +0x18
    int    mCrossFadeType;   // +0x1C
    MusicMgr();              // ??0MusicMgr@@QAE@XZ
    ~MusicMgr();             // ??1MusicMgr@@QAE@XZ (game.o 0x63A7D0)
    void ScaleVolume(float scale);  // ?ScaleVolume@MusicMgr@@QAEXM@Z (game.o 0x62D6D0)
    void Stop(const float fadeOutTime);  // ?Stop@MusicMgr@@QAEXM@Z (game.o 0x62D830)
    void StopIndoor(float fadeOutTime);  // ?StopIndoor@MusicMgr@@QAEXM@Z (game.o 0x603FD0)
    void Update(float dt);       // ?Update@MusicMgr@@QAEXM@Z (game.o 0x62D8A0)
    bool IsMusicPlaying();       // ?IsMusicPlaying@MusicMgr@@QAE_NXZ (game.o 0x6217F0)
    void Play(const char* name); // ?Play@MusicMgr@@QAEXPBD@Z (game.o 0x63A890)
    void PlayIndoor(const char* name, float fadeInTime);  // ?PlayIndoor@MusicMgr@@QAEXPBDM@Z (game.o 0x63AA30)
    static MusicMgr* sInst;      // ?sInst@MusicMgr@@2PAV1@A @ 0xF4EBE4
};
MusicMgr* MusicMgr::sInst = nullptr;

// ea: 0x00612E30
MusicMgr::MusicMgr()
{
    this->mMusic.mVal = 0;
    this->mMusicIndoor.mVal = 0;
    this->mVolScale = 1.0f;
    this->mOutsideScale = 1.0f;
    this->mIndoorScale = 1.0f;
    this->mIndoorFadeTime = 1.0f;
    this->mDelayCount = 0.0f;
    this->mCrossFadeType = 0;
}

// ============================================================================
// SoundMediaMgr - ea: 0x603EF0..0x603F20 (SoundMediaMgr.cpp)
// ============================================================================
struct SoundMediaMgr {
    SoundMediaMgr();              // ??0SoundMediaMgr@@QAE@XZ (game.o 0x603EF0)
    ~SoundMediaMgr();             // ??1SoundMediaMgr@@QAE@XZ (game.o 0x603F00)
    void RegisterSounds();        // ?RegisterSounds@SoundMediaMgr@@QAEXXZ (game.o 0x603F10)
    void PlayLandingSound(Entity* entity, int surfaceType,
                          bool damage) const;  // game.o 0x603F20 (QBE)
};

extern struct CollisionDesc {
    math::Position3 coord;    // +0x00
    math::Position3 normal;   // +0x10
    int material;             // +0x20
};
extern Handle PostEffectEventLanding(const Entity* ent,
                                     const CollisionDesc* col_desc);

// ea: 0x00603EF0
SoundMediaMgr::SoundMediaMgr()
{
}

// ea: 0x00603F00
SoundMediaMgr::~SoundMediaMgr()
{
}

// ea: 0x00603F10
void SoundMediaMgr::RegisterSounds()
{
}

// ea: 0x00603F20
void SoundMediaMgr::PlayLandingSound(Entity* entity,
                                     int surfaceType,
                                     bool damage) const
{
    CollisionDesc v5;
    v5.coord.v.m128_f32[0] = entity->s.pos.trBase[0];
    v5.coord.v.m128_f32[1] = entity->s.pos.trBase[1];
    v5.coord.v.m128_f32[2] = entity->s.pos.trBase[2];
    memset(&v5.coord.v.m128_f32[3], 0, 20);
    v5.material = surfaceType;
    PostEffectEventLanding(entity, &v5);
}

extern float nslGetWaveParam(nslWaveID wave, int b, float c);  // nsl_xboxr
extern float nslGetSourceParam(nslSourceID sid, int index,
                               float defaultValue);  // nslSource.o
extern const char* nslGetSourceName(nslSourceID sid);  // nslSource.o
extern int nslIsWaveStreamed(nslWaveID a);             // nslCompat.o
extern int g_useOnScreenSoundDebugging;   // ?g_useOnScreenSoundDebugging@@3HA
extern void AeStrCopy(char* dst, int* dstLen, int dstCapacity,
                      const char* src, int srcLen);  // ae_string_support.cpp

static SoundDevice::Sound* SoundFromHandle(Handle h)
{
    unsigned int idx = h.mVal & 0xFFF;
    if (idx < 0x200
        && h.mVal >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[idx].mKey)
        return SoundDevice::SoundHandleDb::sInst.mElements[idx].mObject;
    return nullptr;
}

// ea: 0x0062D6D0
void MusicMgr::ScaleVolume(float scale)
{
    unsigned int mVal = this->mMusic.mVal;
    this->mVolScale = scale;
    SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
    if (mObject != nullptr)
    {
        float newVolume = nslGetWaveParam((nslWaveID)mObject->mWave, 0, 1.0f)
            * this->mOutsideScale * this->mVolScale;
        mObject->SetVolume(newVolume);
    }
    SoundDevice::Sound* v10 = SoundFromHandle(this->mMusicIndoor);
    if (v10 != nullptr)
    {
        float newVolumea = nslGetWaveParam((nslWaveID)v10->mWave, 0, 1.0f)
            * this->mIndoorScale * this->mVolScale;
        v10->SetVolume(newVolumea);
    }
}

// ea: 0x0062D830
void MusicMgr::Stop(const float fadeOutTime)
{
    SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
    if (mObject != nullptr)
    {
        mObject->Stop();
        this->mMusic.mVal = 0;
    }
}

// ea: 0x0062D8A0
void MusicMgr::Update(float dt)
{
    if (this->mCrossFadeType == 1)
    {
        float v4 = dt + this->mDelayCount;
        float v6 = v4 / this->mIndoorFadeTime;
        this->mDelayCount = v4;
        this->mIndoorScale = v6;
        this->mOutsideScale = 1.0f - v6;
        if (v4 >= this->mIndoorFadeTime)
        {
            this->mIndoorScale = 1.0f;
            this->mOutsideScale = 0.0f;
            this->mCrossFadeType = 0;
        }
        SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
        if (mObject != nullptr)
        {
            float newVolume = nslGetWaveParam((nslWaveID)mObject->mWave, 0,
                                              1.0f)
                * this->mVolScale * this->mOutsideScale;
            mObject->SetVolume(newVolume);
        }
        SoundDevice::Sound* v14 = SoundFromHandle(this->mMusicIndoor);
        if (v14 != nullptr)
        {
            float newVolumea = nslGetWaveParam((nslWaveID)v14->mWave, 0, 1.0f)
                * this->mVolScale * this->mIndoorScale;
            v14->SetVolume(newVolumea);
        }
    }
    else if (this->mCrossFadeType == 2)
    {
        float v15 = dt + this->mDelayCount;
        float v16 = v15 / this->mIndoorFadeTime;
        this->mDelayCount = v15;
        this->mOutsideScale = v16;
        this->mIndoorScale = 1.0f - v16;
        if (v15 >= this->mIndoorFadeTime)
        {
            this->mOutsideScale = 1.0f;
            this->mCrossFadeType = 0;
            SoundDevice::Sound* indoor = SoundFromHandle(this->mMusicIndoor);
            if (indoor != nullptr)
            {
                indoor->Stop();
                this->mMusicIndoor.mVal = 0;
            }
        }
        SoundDevice::Sound* v20 = SoundFromHandle(this->mMusic);
        if (v20 != nullptr)
        {
            float newVolumeb = nslGetWaveParam((nslWaveID)v20->mWave, 0, 1.0f)
                * this->mVolScale * this->mOutsideScale;
            v20->SetVolume(newVolumeb);
        }
        SoundDevice::Sound* v22 = SoundFromHandle(this->mMusicIndoor);
        if (v22 != nullptr)
        {
            float newVolumec = nslGetWaveParam((nslWaveID)v22->mWave, 0, 1.0f)
                * this->mVolScale * this->mIndoorScale;
            v22->SetVolume(newVolumec);
        }
    }
}

// ea: 0x006217F0
bool MusicMgr::IsMusicPlaying()
{
    unsigned int mVal = this->mMusic.mVal;
    unsigned int v2 = mVal & 0xFFF;
    return v2 < 0x200
        && mVal >> 12
            == (unsigned int)SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject != nullptr;
}

// ============================================================================
// MusicMgr play/stop - ea: 0x603FD0..0x63AA30 (MusicMgr.cpp)
// ============================================================================
struct SoundOptions {
    uint8_t _pad[0x34];
    int mFxDontPlayMusic;  // +0x34
};
extern SoundOptions gSoundOptions;  // ?gSoundOptions@@3VSoundOptions@@A @ 0xF00EF0
extern void* AudioBankMgr_sInst;  // ?sInst@AudioBankMgr@@2PAV1@A @ 0xF4EBD8 (cross-TU bridge)

// ea: 0x00603FD0
void MusicMgr::StopIndoor(float fadeOutTime)
{
    this->mIndoorFadeTime = fadeOutTime;
    this->mCrossFadeType = 2;
    this->mDelayCount = 0.0f;
}

// ea: 0x0063A7D0
MusicMgr::~MusicMgr()
{
    SoundDevice::Sound* mObject = SoundFromHandle(this->mMusic);
    if (mObject != nullptr)
        mObject->Stop();
    SoundDevice::Sound* v9 = SoundFromHandle(this->mMusicIndoor);
    if (v9 != nullptr)
        v9->Stop();
}

// ea: 0x0063A890
void MusicMgr::Play(const char* name)
{
    if (gSoundOptions.mFxDontPlayMusic == 0
        && *(int*)((char*)AudioBankMgr_sInst + 0x6C8) > 0)
    {
        if (name != nullptr)
        {
            if (SoundFromHandle(this->mMusic) != nullptr)
                this->Stop(0.0f);
            math::Position3 zeroPos;
            math::Dir3 zeroDir;
            memset(&zeroPos, 0, sizeof(zeroPos));
            memset(&zeroDir, 0, sizeof(zeroDir));
            DbLinkedHandle<EntityHandleDb, Entity> ent;
            ent.mHandle.mVal = 0;
            DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>
                result = SoundDevice::sInst->PlaySound(
                    name, ent, false, false, zeroPos, zeroDir,
                    -1.0f, -1.0f, -1.0f, -1.0f);
            this->mMusic.mVal = result.mHandle.mVal;
            SoundDevice::Sound* v4 = SoundFromHandle(this->mMusic);
            if (v4 != nullptr)
            {
                float v5 = nslGetWaveParam((nslWaveID)v4->mWave, 0, 1.0f)
                    * this->mOutsideScale * this->mVolScale;
                v4->SetVolume(v5);
                return;
            }
            if (cls.state != CA_LOADING
                && SoundFromHandle(this->mMusic) == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.cpp";
                AeAssert::gCurrentLine = 96;
                AeAssert::gCurrentExpr = "*mMusic";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "Music Error: Music playsound failed (%s)", name))
                    __debugbreak();
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.cpp";
            AeAssert::gCurrentLine = 75;
            AeAssert::gCurrentExpr = "name";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Music Error: NULL music name passed to MusicMgr::Play()"))
                __debugbreak();
        }
    }
}

// ea: 0x0063AA30
void MusicMgr::PlayIndoor(const char* name, float fadeInTime)
{
    if (gSoundOptions.mFxDontPlayMusic == 0)
    {
        if (name != nullptr)
        {
            if (SoundFromHandle(this->mMusicIndoor) != nullptr)
            {
                this->mCrossFadeType = 2;
                this->mIndoorFadeTime = 0.0f;
                this->mDelayCount = 0.0f;
            }
            math::Position3 zeroPos;
            math::Dir3 zeroDir;
            memset(&zeroPos, 0, sizeof(zeroPos));
            memset(&zeroDir, 0, sizeof(zeroDir));
            DbLinkedHandle<EntityHandleDb, Entity> ent;
            ent.mHandle.mVal = 0;
            DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>
                result = SoundDevice::sInst->PlaySound(
                    name, ent, false, false, zeroPos, zeroDir,
                    -1.0f, -1.0f, -1.0f, -1.0f);
            this->mMusicIndoor.mVal = result.mHandle.mVal;
            if (SoundFromHandle(this->mMusicIndoor) == nullptr)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.cpp";
                AeAssert::gCurrentLine = 126;
                AeAssert::gCurrentExpr = "*mMusicIndoor";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "Music Error: Music playsound failed (%s)", name))
                    __debugbreak();
            }
            this->mIndoorScale = 0.0f;
            this->mCrossFadeType = 1;
            this->mIndoorFadeTime = fadeInTime;
            SoundDevice::Sound* v11 = SoundFromHandle(this->mMusicIndoor);
            if (v11 != nullptr)
            {
                float v12 =
                    nslGetWaveParam((nslWaveID)v11->mWave, 0, 1.0f)
                    * this->mVolScale * this->mIndoorScale;
                v11->SetVolume(v12);
            }
        }
        else
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.cpp";
            AeAssert::gCurrentLine = 116;
            AeAssert::gCurrentExpr = "name";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Music Error: NULL music name passed to MusicMgr::Play()"))
                __debugbreak();
        }
    }
}

// ============================================================================
// SoundDevice::Sound::GetDebugString - ea: 0x6216F0
// ============================================================================
// nsl sound API declarations (shared by the SoundDevice helpers below)
enum nslSourceState {
    NSL_SOURCE_STATE_INVALID = 0,
    NSL_SOURCE_STATE_QUEUING = 2,
    NSL_SOURCE_STATE_QUEUED = 3,
    NSL_SOURCE_STATE_PLAYING = 4,
    NSL_SOURCE_STATE_PAUSED = 5,
};
extern nslSourceState nslGetSourceState(nslSourceID sid);   // nsl
extern unsigned int nslWaveGetHash(nslWaveID waveID);       // nsl
extern void nslStopSource(nslSourceID sid);                 // nsl
extern void nslFreeSource(nslSourceID sid);                 // nsl
extern void nslSetSourceParam(nslSourceID sid, int index,
                              float value);                 // nsl
extern void nslSetSourcePosition(nslSourceID sid,
                                 const float* position);    // nsl
extern void nslSetSourceVelocity(nslSourceID sid,
                                 const float* velocity);    // nsl
extern const char* nslWaveGetName(nslWaveID waveID);        // nsl
extern nslSourceID g_break_on_stop;  // ?g_break_on_stop@@3W4nslSourceID@@A (game.o)
extern void tlWarning(const char* fmt, ...);                // tl_xboxr
extern "C" int __fpclass(float);
extern const char* nslGetSourceName(nslSourceID sid);       // nsl
extern float nslGetSourceParam(nslSourceID sid, int index,
                               float defaultValue);         // nsl
extern unsigned int nslGetSourceLength(nslSourceID sid);    // nsl
extern int nslIsWaveLooped(nslWaveID a);                    // nsl
extern int nslGetWaveLength(nslWaveID waveID);              // nsl
extern void nslSetSourceEffectOn(nslSourceID sid);          // nsl
extern void nslSetSourceEffectOff(nslSourceID sid);         // nsl
extern void nslSetMasterVolume(float newVolume);            // nsl
extern void nslPauseSource(nslSourceID sid);                // nsl
extern void nslUnpauseSource(nslSourceID sid);              // nsl
extern void nslPlaySource(nslSourceID sid);                 // nsl
extern void nslDampenGuardSource(nslSourceID sid);          // nsl
extern void nslSetNumberOfListeners(int listeners);         // nsl
extern int nslAreAllBanksLoaded();                          // nsl
extern int nslNumBanksInUse();                              // nsl

// ea: 0x006216F0
ae_fixed_string<1024, unsigned short>
SoundDevice::Sound::GetDebugString() const
{
    const char* SourceName = nslGetSourceName((nslSourceID)this->mSource);
    const char* v11 = "loop";
    nslSourceState SourceState;
    if (((this->mSource == NSL_SOURCE_ID_INVALID
          || (SourceState = nslGetSourceState((nslSourceID)this->mSource))
                 != NSL_SOURCE_STATE_PLAYING
             && SourceState != NSL_SOURCE_STATE_QUEUING
             && SourceState != NSL_SOURCE_STATE_QUEUED
             && SourceState != NSL_SOURCE_STATE_PAUSED)
         && !this->mPaused)
        || nslIsWaveLooped((nslWaveID)this->mWave) == 0)
    {
        v11 = "one-shot";
    }
    if (SourceName == nullptr)
        SourceName = "(unknown)";
    unsigned int SourceLength =
        nslGetSourceLength((nslSourceID)this->mSource);
    float SourceParam =
        nslGetSourceParam((nslSourceID)this->mSource, 1, -1.0f);
    float v7 = nslGetSourceParam((nslSourceID)this->mSource, 0, -1.0f);
    char buf[1024];
    sprintf(buf, "%s V%.2f P%.2f L%.2f %s", SourceName, v7, SourceParam,
            SourceLength * 0.001f, v11);
    return ae_fixed_string<1024, unsigned short>(buf);
}

// ============================================================================
// SoundDevice::GetSoundForHandle - ea: 0x621670 / 0x6216B0
// ea: 0x0062CD90
void SoundDevice::DebugRender()
{
    static cvar_t* debug_bg = Cvar_Get("debug_bg", "0", 256);
    if (debug_bg->integer != 0)
    {
        MusicMgr* v3 = MusicMgr::sInst;
        unsigned int v4 = v3->mMusic.mVal & 0xFFF;
        if (v4 < 0x200
            && v3->mMusic.mVal >> 12
                == SoundDevice::SoundHandleDb::sInst.mElements[v4].mKey
            && SoundDevice::SoundHandleDb::sInst.mElements[v4].mObject
                != nullptr)
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            SoundDevice::Sound* mObject =
                SoundDevice::SoundHandleDb::sInst.mElements[v4].mObject;
            nslSourceID mSource = (nslSourceID)mObject->mSource;
            float param;
            if (mSource == NSL_SOURCE_ID_INVALID)
                param = -1073741824.0f;
            else
                param = nslGetSourceParam(mSource, 0, -1.0f);
            const char* SourceName = mSource == NSL_SOURCE_ID_INVALID
                                         ? nullptr
                                         : nslGetSourceName(mSource);
            DebugRender::RenderText(va("ext: %s %1.2f", SourceName, param),
                                    10, 70, col, 0.0f, 1.0f);
        }
        else
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            DebugRender::RenderText("ext: <no ext music>", 10, 70, col,
                                    0.0f, 1.0f);
        }
        MusicMgr* v13 = MusicMgr::sInst;
        unsigned int v14 = v13->mMusicIndoor.mVal & 0xFFF;
        if (v14 < 0x200
            && v13->mMusicIndoor.mVal >> 12
                == SoundDevice::SoundHandleDb::sInst.mElements[v14].mKey
            && SoundDevice::SoundHandleDb::sInst.mElements[v14].mObject
                != nullptr)
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            SoundDevice::Sound* v16 =
                SoundDevice::SoundHandleDb::sInst.mElements[v14].mObject;
            nslSourceID v17 = (nslSourceID)v16->mSource;
            float param;
            if (v17 == NSL_SOURCE_ID_INVALID)
                param = -1073741824.0f;
            else
                param = nslGetSourceParam(v17, 0, -1.0f);
            const char* v22 = v17 == NSL_SOURCE_ID_INVALID
                                  ? nullptr
                                  : nslGetSourceName(v17);
            DebugRender::RenderText(va("int: %s %1.2f", v22, param),
                                    10, 85, col, 0.0f, 1.0f);
        }
        else
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            DebugRender::RenderText("int: <no int music>", 10, 85, col,
                                    0.0f, 1.0f);
        }
    }
    if (g_useOnScreenSoundDebugging != 0)
    {
        ae_sized_array<ae_fixed_string<128, unsigned char>, 64> spu;
        ae_sized_array<ae_fixed_string<128, unsigned char>, 64> streams;
        for (int k = 0; k < 512; ++k)
        {
            if (this->mSounds[k].mSource != NSL_SOURCE_ID_INVALID)
            {
                if (nslIsWaveStreamed((nslWaveID)this->mSounds[k].mWave) != 0)
                {
                    ae_fixed_string<1024, unsigned short> DebugString =
                        this->mSounds[k].GetDebugString();
                    ae_fixed_string<128, unsigned char> tmp;
                    int m = 0;
                    AeStrCopy((char*)tmp.mBuff, &m, 127,
                              (const char*)DebugString.mBuff,
                              DebugString.mLength);
                    tmp.mLength = (unsigned char)m;
                    spu.push_back(tmp);
                }
                else
                {
                    ae_fixed_string<1024, unsigned short> DebugString =
                        this->mSounds[k].GetDebugString();
                    ae_fixed_string<128, unsigned char> tmp;
                    int m = 0;
                    AeStrCopy((char*)tmp.mBuff, &m, 127,
                              (const char*)DebugString.mBuff,
                              DebugString.mLength);
                    tmp.mLength = (unsigned char)m;
                    streams.push_back(tmp);
                }
            }
        }
        float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
        DebugRender::RenderText("Active sounds", 10, 20, col, 0.0f, 1.0f);
        int v33 = 32;
        for (unsigned int i = 0; i < (unsigned int)streams.size(); ++i)
        {
            float cola[4] = { 1.0f, 0.25f, 0.25f, 0.25f };
            DebugRender::RenderText((const char*)streams[i].mBuff, 10, v33,
                                    cola, 0.0f, 1.0f);
            v33 += 12;
        }
        for (unsigned int j = 0; j < (unsigned int)spu.size(); ++j)
        {
            float colb[4] = { 0.25f, 1.0f, 0.25f, 0.25f };
            DebugRender::RenderText((const char*)spu[j].mBuff, 10, v33, colb,
                                    0.0f, 1.0f);
            v33 += 12;
        }
        int streamed = 0;
        for (int m = 0; m < 512; ++m)
        {
            if (this->mSounds[m].mSource != NSL_SOURCE_ID_INVALID
                && nslIsWaveStreamed((nslWaveID)this->mSounds[m].mWave) != 0)
                ++streamed;
        }
        if (streamed > 3 && this->mShowStreams->integer == 0)
        {
            char buf[128];
            sprintf(buf, "Warning: %i streamed sounds", streamed);
            float colc[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
            DebugRender::RenderText(buf, 10, 20, colc, 0.0f, 1.0f);
        }
        for (int m = 0; m < 512; ++m)
        {
            if (this->mSounds[m].mSource != -1)
            {
                math::Position3 pos;
                pos.v.m128_f32[0] = this->mSounds[m].mDebugPos[0];
                pos.v.m128_f32[1] = this->mSounds[m].mDebugPos[1];
                pos.v.m128_f32[2] = this->mSounds[m].mDebugPos[2];
                pos.v.m128_f32[3] = 0.0f;
                float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                ae_fixed_string<1024, unsigned short> ds =
                    this->mSounds[m].GetDebugString();
                DebugRender::RenderText3D(&pos, white, 1.0f,
                                          (const char*)ds.mBuff);
            }
        }
        {
            math::Position3 pos;
            pos.v.m128_f32[0] = this->mDebugListenerPosition[0];
            pos.v.m128_f32[1] = this->mDebugListenerPosition[1];
            pos.v.m128_f32[2] = this->mDebugListenerPosition[2];
            pos.v.m128_f32[3] = 0.0f;
            math::Position3 fwdEnd = pos;
            fwdEnd.v.m128_f32[0] += this->mDebugListenerForward[0] * 10.0f;
            fwdEnd.v.m128_f32[1] += this->mDebugListenerForward[1] * 10.0f;
            fwdEnd.v.m128_f32[2] += this->mDebugListenerForward[2] * 10.0f;
            float colf[4] = { 1.0f, 1.0f, 0.0f, 0.5f };
            DebugRender::RenderLine(&pos, &fwdEnd, colf, 0.05f);
            math::Position3 upEnd = pos;
            upEnd.v.m128_f32[0] += this->mDebugListenerUp[0] * 10.0f;
            upEnd.v.m128_f32[1] += this->mDebugListenerUp[1] * 10.0f;
            upEnd.v.m128_f32[2] += this->mDebugListenerUp[2] * 10.0f;
            float colu[4] = { 0.0f, 1.0f, 1.0f, 0.5f };
            DebugRender::RenderLine(&pos, &upEnd, colu, 0.05f);
        }
    }
}

// ============================================================================
SoundDevice::SoundHandleDb SoundDevice::SoundHandleDb::sInst;  // @ 0xF50D10
SoundDevice* SoundDevice::sInst = nullptr;                     // @ 0xF4EBDC

// ea: 0x00621670
SoundDevice::Sound* SoundDevice::GetSoundForHandle(
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> handle)
{
    unsigned int v2 = handle.mHandle.mVal & 0xFFF;
    if (v2 < 0x200
        && handle.mHandle.mVal >> 12
            == (unsigned int)SoundHandleDb::sInst.mElements[v2].mKey)
        return SoundHandleDb::sInst.mElements[v2].mObject;
    return nullptr;
}

// ea: 0x006216B0
const SoundDevice::Sound* SoundDevice::GetSoundForHandle(
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> handle)
    const
{
    unsigned int v2 = handle.mHandle.mVal & 0xFFF;
    if (v2 < 0x200
        && handle.mHandle.mVal >> 12
            == (unsigned int)SoundHandleDb::sInst.mElements[v2].mKey)
        return SoundHandleDb::sInst.mElements[v2].mObject;
    return nullptr;
}

// ============================================================================
// CGBankManager::~CGBankManager - ea: 0x611B70
// ============================================================================
extern void Cmd_RemoveCommand(const char* cmd_name);  // game.o g_cmd.cpp
extern void Cmd_AddCommand(const char* cmd_name,
                           void (*function)());  // game.o g_cmd.cpp
extern void* CGBankManager_vftable;   // ??_7CGBankManager@@6B@ @ 0xD0543C
extern void ToggleRenderGeom();       // game.o 0x6119F0
extern void ToggleGraph();            // game.o 0x611A30
extern void ToggleRenderPerf();       // game.o 0x611A10
extern void ZoomIn();                 // game.o 0x611A50
extern void ZoomOut();                // game.o 0x611A70
extern void Teleport();               // game.o 0x611A90
extern void DebugRender_AddRenderer(void* self, void (*fp)());  // render.o
extern void* DebugRender_sInst;       // ?sInst@DebugRender@@2V1@A @ 0xF74D20
extern void* AssetBankSet_ctor(void* self);  // streamer.o
extern void CGBankManager_DebugRender_impl(void* self);  // 0x646700
void* CGBankManager::sInst = nullptr;         // ?sInst@CGBankManager@@2PAV1@A @ 0xF4F438

// Static bridge used by the DebugRender callback registration.
static void CGBankManager_DebugRender_bridge()
{
    CGBankManager_DebugRender_impl(CGBankManager::sInst);
}

// ea: 0x006492D0
CGBankManager::CGBankManager()
{
    AssetBankSet_ctor(this);
    *(void**)this = (void*)&CGBankManager_vftable;
    DebugRender_AddRenderer((void*)&DebugRender_sInst,
                            CGBankManager_DebugRender_bridge);
    this->mCount = 0;
    for (int i = 0; i < 99; ++i)
        this->mBankArray[i] = nullptr;
    *(unsigned int*)((char*)this + 0x04) = 0;
    *(float*)((char*)this + 0x08) = 0.0f;
    *(unsigned int*)((char*)this + 0x04) &= 0xFFFFFFF8;
    *(float*)((char*)this + 0x08) = 150.0f;
    Cmd_AddCommand("cg", ToggleRenderGeom);
    Cmd_AddCommand("cggraph", ToggleGraph);
    Cmd_AddCommand("cgperf", ToggleRenderPerf);
    Cmd_AddCommand("cgzoomin", ZoomIn);
    Cmd_AddCommand("cgzoomout", ZoomOut);
    Cmd_AddCommand("teleport", Teleport);
}

// ea: 0x00611B70
CGBankManager::~CGBankManager()
{
    Cmd_RemoveCommand("cg");
    Cmd_RemoveCommand("cggraph");
    Cmd_RemoveCommand("cgperf");
    Cmd_RemoveCommand("cgzoomin");
    Cmd_RemoveCommand("cgzoomout");
    Cmd_RemoveCommand("teleport");
}

// ============================================================================
// CGBankManager::AddBank / UnloadBank - ea: 0x61FE30 / 0x61FE60 (cgbank.cpp)
// ============================================================================
extern void j_nullsub_118(actor_s* actor);  // g.o
extern void InvalidateParticleCollisionCaches();  // aeps_xboxr
extern void VEH_InvalidateCaches();               // g.o
extern void InvalidateTurretCaches();             // g.o

// ea: 0x0061FE30
void CGBankManager::AddBank(TPakId pakId, CGBank* bank)
{
    this->mBankArray[this->mCount] = bank;
    this->mIds[this->mCount++] = (int)pakId;
}

// ea: 0x0061FE60
void CGBankManager::UnloadBank(TPakId pakId)
{
    bool removed = false;
    int v4 = 0;
    if (this->mCount > 0)
    {
        int* mIds = this->mIds;
        do
        {
            if (*mIds == (int)pakId)
            {
                if (v4 < this->mCount - 1)
                {
                    this->mBankArray[v4] =
                        this->mBankArray[this->mCount - 1];
                    *mIds = this->mIds[this->mCount - 1];
                    --v4;
                    --mIds;
                }
                --this->mCount;
                removed = true;
            }
            ++v4;
            ++mIds;
        } while (v4 < this->mCount);
        if (removed)
        {
            actor_s** actors = level.actors;
            do
            {
                if (*actors != nullptr && (*actors)->pEnt != nullptr)
                    j_nullsub_118(*actors);
                ++actors;
            } while (actors < &level.actors[32]);
            for (int i = 0; i < 16; ++i)
            {
                Entity* v7 = EntityManager::sInst->mPlayers[i];
                if (v7 != nullptr)
                {
                    float* lo = v7->proximity_data->lo.v.m128_f32;
                    if (lo != nullptr)
                    {
                        lo[0] = 3.402823466e38f;
                        lo[1] = 3.402823466e38f;
                        lo[2] = 3.402823466e38f;
                        lo[3] = 3.402823466e38f;
                        v7->proximity_data->hi.v =
                            _mm_xor_ps(_mm_set1_ps(-0.0f),
                                       v7->proximity_data->lo.v);
                    }
                }
            }
            VEH_InvalidateCaches();
            InvalidateTurretCaches();
            InvalidateParticleCollisionCaches();
        }
    }
}

// ============================================================================
// CGBankManager::DecodeCGBank - ea: 0x629F30 (cgbank.cpp)
// ============================================================================
extern void CGBank_load_inplace(CGBank* self, char* base, int* offs);
    // ?load_inplace@CGBank@@QAEXPADAAH@Z (inplace_xboxr)
extern bool _tlAssert(const char* file, int line, const char* expr,
                      const char* msg);  // core/tl_system.cpp

// ea: 0x00629F30
void CGBankManager::DecodeCGBank(const char* name, unsigned char* data,
                                 int size, TPakId pakId)
{
    CGBank* bank = (CGBank*)data;
    int offs = 0xD0;
    CGBank_load_inplace(bank, (char*)bank, &offs);
    if (offs != size)
    {
        AeAssert::gCurrentAuthor = AeAssert::JSV;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.cpp";
        AeAssert::gCurrentLine = 454;
        AeAssert::gCurrentExpr = "offs == size";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("oops"))
            __debugbreak();
    }
    this->mBankArray[this->mCount] = bank;
    this->mIds[this->mCount++] = (int)pakId;
    bank->rtree_root.simd_tree = (rtree_node_t*)bank->rtree_data;
    bank->rtree_root.simd_pointer_base = nullptr;
    unsigned int nobjects = bank->objects.m_count;
    int v8 = 0;
    unsigned int v9 = 0;
    for (; v9 < nobjects; ++v8)
    {
        if (v9 >= bank->objects.m_count
            && _tlAssert("c:\\cod\\code\\tl\\cdl\\source\\cdl_mem.h", 91,
                         "index >= 0 && index < size()", "invalid index"))
            __debugbreak();
        cdl_object_t* m_elements = (cdl_object_t*)bank->objects.m_elements;
        unsigned int* v12 = (unsigned int*)&m_elements[v8].cflags;
        int cflags = *v12;
        if ((0x1000000 & cflags) != 0)
            *v12 = cflags & 0xFEFFFFFF;
        if ((*v12 & 0x4000) != 0)
            *v12 = 0x1000000 | *v12 & 0xFFFFBFFF;
        ++v9;
    }
}

// ============================================================================
// AnimNotifyTask - ea: 0x602350..0x62B950 (AnimNotifyTask.cpp)
// ============================================================================
extern void* AnimNotifyTask_vftable;  // ??_7AnimNotifyTask@@6B@ @ 0xD03A60
typedef void (__cdecl* AnimNotifyCallback)(Broc::entity);
extern void ae_vector_push_back_uint(
    ae_vector<unsigned int>* self, const unsigned int* elem);  // ?push_back@?$ae_vector@I@@QAEXABI@Z (0x41AF42)
extern void ae_vector_push_back_funcptr(
    ae_vector<AnimNotifyCallback>* self,
    AnimNotifyCallback const* elem);  // ?push_back@?$ae_vector@P6AXVentity@Broc@@@Z@@QAEXABQ6AXVentity@Broc@@@Z@Z (0x42A9F1)

class AnimNotifyTask : public Task {
public:
    AnimNotifyTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                   const char* pAnimName, unsigned int killHash);
    AnimNotifyTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                   unsigned int animHash, unsigned int killHash);
    virtual void Update(Entity* pEnt, float deltaT);  // virtual in binary (UAEX)
    static void RegisterFunc(const char* pKey,
                             AnimNotifyCallback cbFunc);
private:
    static int Find(unsigned int key);  // ?Find@AnimNotifyTask@@CAHI@Z
public:

    unsigned int mAnimHash;        // +0x1C
    unsigned int mNotifyKillHash;  // +0x20
    static ae_vector<unsigned int> mKeys;   // ?mKeys@AnimNotifyTask@@0V?$ae_vector@I@@A @ 0xF4F460
    static ae_vector<AnimNotifyCallback> mPtrs;  // @ 0xF50CA0
};
static_assert(sizeof(AnimNotifyTask) == 0x24, "AnimNotifyTask size mismatch");
ae_vector<unsigned int> AnimNotifyTask::mKeys;
ae_vector<AnimNotifyCallback> AnimNotifyTask::mPtrs;

// file-scope hashes ($S12_2-guarded; .data @ 0xF58C3C..0xF58C50)
static unsigned int donotetracksdoneHash;  // @ 0xF58C4C
static unsigned int termHash;              // @ 0xF58C48
static unsigned int endHash;               // @ 0xF58C44
static unsigned int finishedHash;          // @ 0xF58C40
static unsigned int undefinedHash;         // @ 0xF58C3C
static int          sAnimNotifyInitFlags;  // $S12_2 @ 0xF58C50

// WaitTilOutputInst1<Broc::string> (Broc): WaitTilOutput base + value string.
// AssignData is called virtually at vtable slot 1 in the binary; bridged here
// through the core.o shim used by EntityNotifySet::AssignScriptVariable.
struct WaitTilOutputInst1 {
    void*        __vftable;   // +0x00
    void*        _dl[2];      // +0x04
    Broc::string data;        // +0x0C

    WaitTilOutputInst1(const Broc::string& d) : data(d)
    {
        __vftable = nullptr;
        _dl[0] = nullptr;
        _dl[1] = nullptr;
    }
    ~WaitTilOutputInst1() {}
};
extern void WaitTilOutput_AssignData(void* self, void* data);  // core.o

// ea: 0x00602350
AnimNotifyTask::AnimNotifyTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                               const char* pAnimName, unsigned int killHash)
    : Task(h, 0x414E4659)
{
    mNotifyKillHash = killHash;
    mAnimHash = HashString::CalcHash(pAnimName);
}

// ea: 0x006023C0
AnimNotifyTask::AnimNotifyTask(DbLinkedHandle<EntityHandleDb, Entity> h,
                               unsigned int animHash, unsigned int killHash)
    : Task(h, 0x414E4659)
{
    mNotifyKillHash = killHash;
    mAnimHash = animHash;
}

// ea: 0x0062B670
void AnimNotifyTask::Update(Entity* pEnt, float deltaT)
{
    if ((sAnimNotifyInitFlags & 1) == 0)
    {
        sAnimNotifyInitFlags |= 1;
        donotetracksdoneHash = HashString::CalcHash("donotetracksdone");
    }
    if ((this->mFlags & 4) == 0 && pEnt != nullptr)
    {
        EntityNotifySet* mNotifySet = pEnt->mNotifySet;
        if (mNotifySet != nullptr)
        {
            Broc::string outString((Broc::string::Block*)nullptr);
            WaitTilOutputInst1 outParams(outString);
            if ((sAnimNotifyInitFlags & 2) == 0)
            {
                sAnimNotifyInitFlags |= 2;
                termHash = HashString::CalcHash("killanimscript");
            }
            HashString chk;
            chk.mHash = termHash;
            if (EntityNotifySet_GetNotify(mNotifySet, chk.mHash) != nullptr
                || (chk.mHash = this->mNotifyKillHash,
                    EntityNotifySet_GetNotify(mNotifySet, chk.mHash)
                        != nullptr))
            {
                HashString h;
                h.mHash = donotetracksdoneHash;
                pEnt->Notify(h);
                this->mFlags |= 4;
                return;
            }
            chk.mHash = this->mAnimHash;
            EntityNotify* Notify =
                (EntityNotify*)EntityNotifySet_GetNotify(mNotifySet,
                                                         chk.mHash);
            if (Notify != nullptr && Notify->mParam != nullptr)
            {
                if ((sAnimNotifyInitFlags & 4) == 0)
                {
                    sAnimNotifyInitFlags |= 4;
                    endHash = HashString::CalcHash("end");
                }
                if ((sAnimNotifyInitFlags & 8) == 0)
                {
                    sAnimNotifyInitFlags |= 8;
                    finishedHash = HashString::CalcHash("finished");
                }
                if ((sAnimNotifyInitFlags & 0x10) == 0)
                {
                    sAnimNotifyInitFlags |= 0x10;
                    undefinedHash = HashString::CalcHash("undefined");
                }
                bool v9 = false;
                WaitTilOutput_AssignData(Notify->mParam, &outParams);
                const char* v10 =
                    outParams.data.mBlock != nullptr
                        ? (const char*)(outParams.data.mBlock + 1)
                        : defaultFileName;
                unsigned int v11 = HashString::CalcHash(v10);
                if (v11 == endHash || v11 == finishedHash
                    || v11 == undefinedHash)
                    v9 = true;
                int v12 = AnimNotifyTask::Find(v11);
                if (v12 != -1)
                {
                    Broc::entity ent;
                    ent.___u0 = pEnt->mHandle.mHandle.mVal;
                    AnimNotifyTask::mPtrs.mElements[v12](ent);
                    if (v9)
                    {
                        HashString h;
                        h.mHash = donotetracksdoneHash;
                        pEnt->Notify(h);
                        this->mFlags |= 4;
                        return;
                    }
                    return;
                }
                if (v9)
                {
                    HashString h;
                    h.mHash = donotetracksdoneHash;
                    pEnt->Notify(h);
                    this->mFlags |= 4;
                    return;
                }
            }
        }
    }
    if ((this->mFlags & 4) != 0 && pEnt != nullptr)
    {
        HashString h;
        h.mHash = donotetracksdoneHash;
        pEnt->Notify(h);
    }
}

// ea: 0x0062B950
void AnimNotifyTask::RegisterFunc(const char* pKey,
                                  AnimNotifyCallback cbFunc)
{
    unsigned int keyHash = HashString::CalcHash(pKey);
    int index = AnimNotifyTask::Find(keyHash);
    if (index == -1)
    {
        ae_vector_push_back_uint(&AnimNotifyTask::mKeys, &keyHash);
        ae_vector_push_back_funcptr(&AnimNotifyTask::mPtrs, &cbFunc);
    }
}

// ea: 0x00612790
int AnimNotifyTask::Find(unsigned int key)
{
    unsigned int* mElements = AnimNotifyTask::mKeys.mElements;
    unsigned int* v2 =
        &AnimNotifyTask::mKeys.mElements[AnimNotifyTask::mKeys.mSize];
    int result = 0;
    if (AnimNotifyTask::mKeys.mElements == v2)
        return -1;
    while (*mElements != key)
    {
        ++mElements;
        ++result;
        if (mElements == v2)
            return -1;
    }
    return result;
}

// ============================================================================
// AudioBankMgr - ea: 0x6127D0..0x612980
// ============================================================================
typedef int nflFileID;  // filesystem/nfl.cpp / core_systems.h use int
enum ELanguage : int {};  // core_globals.h ABI twin (mangles W4ELanguage)
extern nslWaveID nslGetWave(const char* name);   // ?nslGetWave (nsl)
extern void nslFreeBank(nslBankID bankID);       // ?nslFreeBank (nsl)
extern void nflCloseFile(nflFileID file);        // filesystem/nfl.cpp

class AudioBankMgr {
public:
    enum eState {
        kUnloaded = 0,
        kLoading = 1,     // verified vs disasm IsFinished
        kLoaded = 2,
        kUnloading = 3,   // verified vs disasm IsFinished
    };
    struct WbkEntry {
        tlFixedString name;          // +0x00 (32 bytes, verified 0x621470)
        int          pakFile;        // +0x20
        int          state[6];       // +0x24
        nflFileID    fileID[6];      // +0x3C
        nslBankID    bankId[6];      // +0x54
    };
    static_assert(sizeof(WbkEntry) == 0x6C, "WbkEntry view size mismatch");
    uint8_t  _pad0[4];                 // +0x00 (vftable)
    bool     mDoUnloadNotify;          // +0x04
    bool     mDoLoadNotify;            // +0x05
    uint8_t  _pad06[2];                // +0x06
    uint8_t  mAvailableWbks[0x6C0];    // +0x08 (16 * 0x6C stride)
    int      m_size;                   // +0x6C8
    static AudioBankMgr* sInst;        // ?sInst@AudioBankMgr@@2PAV1@A
    AudioBankMgr();                    // ??0AudioBankMgr@@QAE@XZ (game.o 0x621440)
    virtual ~AudioBankMgr();           // ??1AudioBankMgr@@UAE@XZ
    bool IsFinished() const;           // ?IsFinished@AudioBankMgr@@QBE_NXZ
private:
    const char* LanguageStr(ELanguage id) const;  // ?LanguageStr@AudioBankMgr@@ABEPBDW4ELanguage@@@Z
    void NotifyLoaded();               // ?NotifyLoaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9C0)
    void NotifyUnloaded();             // ?NotifyUnloaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9F0)
public:
    void Update();                     // ?Update@AudioBankMgr@@QAEXXZ (game.o 0x62BA20)
    void FinishLoading();              // ?FinishLoading@AudioBankMgr@@QAEXXZ (game.o 0x62BC40)
    void LoadWbkInternal(WbkEntry* wbk, const char* path, ELanguage lang,
                         bool async);  // ?LoadWbkInternal@AudioBankMgr@@AAEXAAUWbkEntry@1@PBDW4ELanguage@@_N@Z (game.o 0x62BD50)
    void FreeWbk(const tlFixedString& name, bool async);  // ?FreeWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z (game.o 0x62BE30)
    void LoadWbk(const tlFixedString& name, bool async);  // ?LoadWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z (game.o 0x639630)
protected:
    virtual void UnloadBank(TPakId pakId);  // ?UnloadBank@AudioBankMgr@@EAEXW4TPakId@@@Z (game.o 0x639550)
public:
    void RegisterWbk(const tlFixedString& name, const char* path,
                     ELanguage lang, TPakId pak);  // game.o 0x621470
};
AudioBankMgr* AudioBankMgr::sInst = nullptr;

// ============================================================================
// AudioBankMgr ctor / RegisterWbk - ea: 0x621440 / 0x621470
// ============================================================================
extern int nflOpenFile(int mediaID, const char* fileName);  // nfl_xboxr
extern int gNflMediaId;                                     // nfl_xboxr
extern void* AssetBankSet_ctor(void* self);                 // streamer.o
static tlFixedString dflt;          // ?dflt@@3VtlFixedString@@A @ 0xF58C04
static bool s_dflt_init;            // $S13_7

// ea: 0x00621440
AudioBankMgr::AudioBankMgr()
{
    AssetBankSet_ctor(this);
    this->mDoUnloadNotify = false;
    this->mDoLoadNotify = false;
    this->m_size = 0;
}

// ea: 0x00621470
void AudioBankMgr::RegisterWbk(const tlFixedString& name, const char* path,
                               ELanguage lang, TPakId pak)
{
    if (!s_dflt_init)
    {
        s_dflt_init = true;
        new (&dflt) tlFixedString("default");
    }
    if (name == dflt)
        path = "sp_test\\default.wbk";
    int fileId = nflOpenFile(gNflMediaId, path);
    if (fileId == -1)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\AudioBankManager.cpp";
        AeAssert::gCurrentLine = 169;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Couldn't open '%s'", path))
            __debugbreak();
    }
    for (int i = 0; i < this->m_size; ++i)
    {
        int off = 0x6C * i;
        if (off >= 0x6C0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
            AeAssert::gCurrentLine = 154;
            AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        WbkEntry* wbk = (WbkEntry*)((char*)this->mAvailableWbks + off);
        if (wbk->name == name)
        {
            wbk->fileID[lang] = fileId;
            return;
        }
    }
    WbkEntry wbk;
    wbk.name = name;
    wbk.pakFile = pak;
    memset(wbk.state, 0, sizeof(wbk.state));
    memset(&wbk.fileID[0], 0xFF,
           sizeof(wbk.fileID) + sizeof(wbk.bankId));
    wbk.fileID[lang] = fileId;
    ((WbkEntry*)this->mAvailableWbks)[this->m_size++] = wbk;
}

// ea: 0x006127D0
AudioBankMgr::~AudioBankMgr()
{
    int i = 0;
    if (this->m_size > 0)
    {
        unsigned int v2 = 0;
        for (unsigned int j = 0;; v2 = j)
        {
            if (v2 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* entry =
                (WbkEntry*)((char*)this->mAvailableWbks + v2);
            for (int k = 0; k < 6; ++k)
            {
                if (entry->bankId[k] != NSL_BANK_ID_INVALID)
                {
                    nslFreeBank(entry->bankId[k]);
                    nflCloseFile(entry->fileID[k]);
                    entry->bankId[k] = NSL_BANK_ID_INVALID;
                    entry->fileID[k] = (nflFileID)-1;
                }
            }
            bool v5 = ++i < this->m_size;
            j += 108;
            if (!v5)
                break;
        }
    }
}

// ea: 0x006128E0
bool AudioBankMgr::IsFinished() const
{
    int v2 = 0;
    if (this->m_size > 0)
    {
        unsigned int v3 = 0;
        while (1)
        {
            if (v3 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 148;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            const WbkEntry* entry =
                (const WbkEntry*)((char*)this->mAvailableWbks + v3);
            int v4 = 0;
            do
            {
                if (entry->state[v4] == kLoading
                    || entry->state[v4] == kUnloading)
                    return false;
                ++v4;
            } while (v4 < 6);
            ++v2;
            v3 += 108;
            if (v2 >= this->m_size)
                break;
        }
    }
    return true;
}

// ============================================================================
// AudioBankMgr load/notify/update - ea: 0x62B9C0..0x62C010
// ============================================================================
extern void codNflUpdate();                    // nfl_xboxr
extern void nslUpdateBanks();                  // nsl_xboxr
extern int  nslGetBankState(nslBankID bankID); // nsl_xboxr
extern nslBankID nslLoadBank(unsigned int flags, nflFileID file,
                             unsigned int fileOffset);  // nsl_xboxr
extern void tlPrintf(const char* fmt, ...);    // tl_xboxr

// ELanguage values (verified vs AudioBankMgr::LanguageStr disasm)
enum {
    kLanguageEnglish = 0,
    kLanguageGerman = 1,
    kLanguageFrench = 2,
    kLanguageSpanish = 3,
    kLanguageItalian = 4,
    kLanguageUnlocalized = 5,
};

// PakFile/PakManager minimal views (streamer.o; mPath string at +0x0C verified
// vs LoadWbk disasm, mSlots at +0x40 with 0x63 capacity).
struct PakFileView {
    uint8_t _pad[0x0C];
    char    mPath[0x100];  // +0x0C (ae_fixed_string; string bytes at +0x0C)
};
struct PakManagerView {
    uint8_t      _pad[0x40];
    PakFileView* mSlots[0x63];  // +0x40 (100 slots)
};
extern PakManagerView* PakManager_sInst;  // ?sInst@PakManager@@2PAV1@A @ 0xF592FC
extern ELanguage gLanguage;               // ?gLanguage@@3W4ELanguage@@A @ 0xF00EA4

// ea: 0x00639630
void AudioBankMgr::LoadWbk(const tlFixedString& name, bool async)
{
    int i = 0;
    if (this->m_size > 0)
    {
        unsigned int off = 0;
        while (1)
        {
            if (off >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* wbk = (WbkEntry*)((char*)this->mAvailableWbks + off);
            if (wbk->name == name)
            {
                PakFileView* pak = nullptr;
                if (wbk->pakFile >= 0 && wbk->pakFile < 0x63)
                    pak = PakManager_sInst->mSlots[wbk->pakFile];
                const char* path = (const char*)pak + 0x0C;
                ELanguage v11 = gLanguage;
                if (wbk->fileID[kLanguageUnlocalized] != (nflFileID)-1)
                    this->LoadWbkInternal(wbk, path,
                                          (ELanguage)kLanguageUnlocalized,
                                          async);
                if (wbk->fileID[v11] == (nflFileID)-1
                    || (this->LoadWbkInternal(wbk, path, v11, async),
                        wbk->fileID[v11] == (nflFileID)-1))
                {
                    if (wbk->fileID[kLanguageUnlocalized] == (nflFileID)-1)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AudioBankManager.cpp";
                        AeAssert::gCurrentLine = 286;
                        AeAssert::gCurrentExpr = nullptr;
                        if (AeAssert::Error(
                                "Didn't find audio data to load!"))
                            __debugbreak();
                    }
                }
                return;
            }
            off += 0x6C;
            if (++i >= this->m_size)
                break;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AudioBankManager.cpp";
    AeAssert::gCurrentLine = 292;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Unknown wbk: %s", name.str))
        __debugbreak();
    this->mDoLoadNotify = true;
}

// ea: 0x00639550
void AudioBankMgr::UnloadBank(TPakId pakId)
{
    int v2 = 0;
    if (this->m_size > 0)
    {
        int v4 = 0;
        do
        {
            if (v2 >= 0x10)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* entry =
                &((WbkEntry*)this->mAvailableWbks)[v4];
            if (entry->pakFile == (int)pakId)
            {
                this->FreeWbk(entry->name, false);
                int m_size = this->m_size;
                if (m_size > 1 && v2 < m_size)
                    *entry =
                        ((WbkEntry*)this->mAvailableWbks)[m_size - 1];
                int v6 = this->m_size;
                if (v6 != 0)
                    this->m_size = v6 - 1;
                --v2;
                --v4;
            }
            ++v2;
            ++v4;
        } while (v2 < this->m_size);
    }
}

// ea: 0x006023F0
const char* AudioBankMgr::LanguageStr(ELanguage id) const
{
    const char* result;
    switch (id)
    {
    case kLanguageEnglish:
        result = "English";
        break;
    case kLanguageGerman:
        result = "German";
        break;
    case kLanguageFrench:
        result = "French";
        break;
    case kLanguageSpanish:
        result = "Spanish";
        break;
    case kLanguageItalian:
        result = "Italian";
        break;
    case kLanguageUnlocalized:
        result = "<Unlocalized>";
        break;
    default:
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AudioBankManager.cpp";
        AeAssert::gCurrentLine = 63;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("Unknown language id"))
            __debugbreak();
        result = "<unknown language id>";
        break;
    }
    return result;
}

// ea: 0x0062B9C0
void AudioBankMgr::NotifyLoaded()
{
    Entity* mWorld = EntityManager::sInst->mWorld;
    if (mWorld != nullptr)
    {
        HashString v2;
        v2.mHash = HashString::CalcHash("wbk_loaded");
        mWorld->Notify(v2);
    }
}

// ea: 0x0062B9F0
void AudioBankMgr::NotifyUnloaded()
{
    Entity* mWorld = EntityManager::sInst->mWorld;
    if (mWorld != nullptr)
    {
        HashString v2;
        v2.mHash = HashString::CalcHash("wbk_unloaded");
        mWorld->Notify(v2);
    }
}

// ea: 0x0062BA20
void AudioBankMgr::Update()
{
    codNflUpdate();
    nslUpdateBanks();
    if (this->mDoUnloadNotify)
    {
        Entity* mWorld = EntityManager::sInst->mWorld;
        if (mWorld != nullptr)
        {
            HashString v3;
            v3.mHash = HashString::CalcHash("wbk_unloaded");
            mWorld->Notify(v3);
        }
    }
    if (this->mDoLoadNotify)
    {
        Entity* v4 = EntityManager::sInst->mWorld;
        if (v4 != nullptr)
        {
            HashString v5;
            v5.mHash = HashString::CalcHash("wbk_loaded");
            v4->Notify(v5);
        }
    }
    int m_size = this->m_size;
    this->mDoUnloadNotify = false;
    this->mDoLoadNotify = false;
    if (m_size > 0)
    {
        unsigned int v7 = 0;
        unsigned int v17 = 0;
        for (int i = 0;;)
        {
            if (v7 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* entry =
                (WbkEntry*)((char*)this->mAvailableWbks + v7);
            for (int j = 6; j != 0; --j)
            {
                if (entry->bankId[j - 1] == NSL_BANK_ID_INVALID)
                    continue;
                int BankState = nslGetBankState(entry->bankId[j - 1]);
                int state = entry->state[j - 1];
                if (state == kLoading)
                {
                    if (BankState != 0)
                    {
                        if (BankState != -1)
                            continue;
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AudioBankManager.cpp";
                        AeAssert::gCurrentLine = 131;
                        AeAssert::gCurrentExpr = nullptr;
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Warning(
                                "Problem loading wbk '%s'",
                                (const char*)entry + 4))
                            __debugbreak();
                        nflCloseFile(entry->fileID[j - 1]);
                        entry->state[j - 1] = kUnloaded;
                        entry->fileID[j - 1] = (nflFileID)-1;
                        entry->bankId[j - 1] = NSL_BANK_ID_INVALID;
                        Entity* v10 = EntityManager::sInst->mWorld;
                        if (v10 != nullptr)
                        {
                            HashString v11;
                            v11.mHash = HashString::CalcHash("wbk_loaded");
                            v10->Notify(v11);
                        }
                    }
                    else
                    {
                        entry->state[j - 1] = kLoaded;
                        Entity* v10 = EntityManager::sInst->mWorld;
                        if (v10 != nullptr)
                        {
                            HashString v11;
                            v11.mHash = HashString::CalcHash("wbk_loaded");
                            v10->Notify(v11);
                        }
                    }
                }
                else if (state == kUnloading && BankState == -1)
                {
                    entry->state[j - 1] = kUnloaded;
                    entry->bankId[j - 1] = NSL_BANK_ID_INVALID;
                    Entity* v10 = EntityManager::sInst->mWorld;
                    if (v10 != nullptr)
                    {
                        HashString v11;
                        v11.mHash = HashString::CalcHash("wbk_unloaded");
                        v10->Notify(v11);
                    }
                }
            }
            ++i;
            v7 = v17 + 108;
            v17 += 108;
            if (i >= this->m_size)
                break;
        }
    }
}

// ea: 0x0062BC40
void AudioBankMgr::FinishLoading()
{
    do
    {
        this->Update();
        bool v3 = true;
        for (int i = 0; i < this->m_size; ++i)
        {
            const WbkEntry* entry =
                (const WbkEntry*)((char*)this->mAvailableWbks + 108 * i);
            for (int k = 0; k < 6; ++k)
            {
                if (entry->state[k] == kLoading
                    || entry->state[k] == kUnloading)
                    v3 = false;
            }
        }
        if (v3)
            break;
    } while (1);
}

// ea: 0x0062BD50
void AudioBankMgr::LoadWbkInternal(WbkEntry* wbk, const char* path,
                                   ELanguage lang, bool async)
{
    if (wbk->fileID[lang] == (nflFileID)-1
        || wbk->bankId[lang] != NSL_BANK_ID_INVALID)
    {
        this->mDoLoadNotify = true;
    }
    else
    {
        const char* v6 = this->LanguageStr(lang);
        tlPrintf("[wbk] loading wbk [%s]: %s\n",
                 (const char*)wbk + 4, v6);
        nslBankID Bank = nslLoadBank(0, wbk->fileID[lang], 0);
        wbk->bankId[lang] = Bank;
        wbk->state[lang] = kLoading;
        if (Bank == NSL_BANK_ID_INVALID)
        {
            this->mDoLoadNotify = true;
            nflCloseFile(wbk->fileID[lang]);
            wbk->fileID[lang] = (nflFileID)-1;
            wbk->state[lang] = kUnloaded;
        }
        else if (!async)
        {
            if (nslGetBankState(Bank) == 1)
            {
                do
                    this->Update();
                while (nslGetBankState(wbk->bankId[lang]) == 1);
            }
            PakManager::sInst->SetSoundProgress(1.0f);
        }
    }
}

// ea: 0x0062BE30
void AudioBankMgr::FreeWbk(const tlFixedString& name, bool async)
{
    unsigned int v5 = 0;
    int i = 0;
    if (this->m_size > 0)
    {
        while (1)
        {
            if (v5 >= 0x6C0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_array.h";
                AeAssert::gCurrentLine = 154;
                AeAssert::gCurrentExpr = "idx >= 0 && idx < _CAPACITY";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            WbkEntry* entry =
                (WbkEntry*)((char*)this->mAvailableWbks + v5);
            // tlFixedString name compare (8 dwords = 32 bytes)
            if (memcmp(entry, &name, 32) == 0)
            {
                for (int lang = 6; lang != 0; --lang)
                {
                    int state = entry->state[lang - 1];
                    if (state == kLoaded)
                    {
                        tlPrintf("[wbk] freeing wbk: %s\n",
                                 (const char*)entry + 4);
                        nslFreeBank(entry->bankId[lang - 1]);
                        nflCloseFile(entry->fileID[lang - 1]);
                        entry->state[lang - 1] = kUnloading;
                        entry->fileID[lang - 1] = (nflFileID)-1;
                        if (!async
                            && nslGetBankState(entry->bankId[lang - 1]) >= 0)
                        {
                            do
                                this->Update();
                            while (nslGetBankState(
                                       entry->bankId[lang - 1])
                                   >= 0);
                        }
                    }
                    else if (state == kLoading)
                    {
                        AeAssert::gCurrentAuthor = AeAssert::ARO;
                        AeAssert::gCurrentFile =
                            "c:\\cod\\code\\game\\AudioBankManager.cpp";
                        AeAssert::gCurrentLine = 363;
                        AeAssert::gCurrentExpr =
                            "wbk.state[lang] != WbkEntry::kLoading";
                        if (!AeAssert::IsIgnored()
                            && AeAssert::Assert("not handling unloading case"))
                            __debugbreak();
                    }
                    this->mDoUnloadNotify = true;
                }
                return;
            }
            v5 += 108;
            if (++i >= this->m_size)
                break;
        }
    }
    AeAssert::gCurrentAuthor = AeAssert::ARO;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\AudioBankManager.cpp";
    AeAssert::gCurrentLine = 372;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("trying to free unknown wbk '%s'",
                             (const char*)&name + 4))
        __debugbreak();
    this->mDoUnloadNotify = true;
}

// ============================================================================
// SoundDevice::FindWave - ea: 0x612980
// ============================================================================
// ea: 0x00612980
nslWaveID SoundDevice::FindWave(const char* name)
{
    nslWaveID Wave = nslGetWave(name);
    if (AudioBankMgr::sInst->m_size > 0 && Wave == NSL_WAVE_ID_INVALID)
        strncmp(name, "loading_", 8u);
    return Wave;
}

// ============================================================================
// SoundDevice::SetListenerVectors - ea: 0x612A70
// ============================================================================
extern "C" int __fpclass(float);
extern void nslListenerSetPosition(unsigned int listenerIndex,
                                   const float* pos);  // nsl_xboxr
extern void nslListenerSetOrientation(unsigned int listenerIndex,
                                      const float* frt,
                                      const float* top);  // nsl_xboxr
extern void tlWarning(const char* Format, ...);  // tl_xboxr

// ea: 0x00612A70
void SoundDevice::SetListenerVectors(int listener,
                                     const math::Position3& position,
                                     const math::Dir3& front,
                                     const math::Dir3& up)
{
    if ((__fpclass(position.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(position.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(position.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(front.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(up.v.m128_f32[2]) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to adjust "
            "listener position\n");
        return;
    }
    float upv[3];
    upv[0] = position.v.m128_f32[0];
    upv[1] = position.v.m128_f32[1];
    upv[2] = position.v.m128_f32[2];
    float fwv[3];
    fwv[0] = front.v.m128_f32[0];
    fwv[1] = front.v.m128_f32[1];
    fwv[2] = front.v.m128_f32[2];
    float v12[3];
    v12[0] = up.v.m128_f32[0];
    v12[1] = up.v.m128_f32[1];
    v12[2] = up.v.m128_f32[2];
    nslListenerSetPosition((unsigned int)listener, upv);
    nslListenerSetOrientation((unsigned int)listener, fwv, v12);
    this->mDebugListenerPosition[0] = position.v.m128_f32[0];
    this->mDebugListenerPosition[1] = position.v.m128_f32[1];
    this->mDebugListenerPosition[2] = position.v.m128_f32[2];
    this->mDebugListenerForward[0] = front.v.m128_f32[0];
    this->mDebugListenerForward[1] = front.v.m128_f32[1];
    this->mDebugListenerForward[2] = front.v.m128_f32[2];
    this->mDebugListenerUp[0] = up.v.m128_f32[0];
    this->mDebugListenerUp[1] = up.v.m128_f32[1];
    this->mDebugListenerUp[2] = up.v.m128_f32[2];
}

// ============================================================================
// GetSurfaceTypeSounds - ea: 0x612DB0
// ============================================================================
extern const char* Com_SurfaceTypeToName(int iTypeIndex);  // core.o common.cpp

// ea: 0x00612DB0
void GetSurfaceTypeSounds(const char* pszType,
                          nslWaveID* const sounds)
{
    char szAliasName[256];
    for (int i = 0; i < 23; ++i)
    {
        const char* v3 = Com_SurfaceTypeToName(i);
        sprintf(szAliasName, "%s_%s", pszType, v3);
        nslWaveID Wave = nslGetWave(szAliasName);
        if (AudioBankMgr::sInst->m_size > 0 && Wave == NSL_WAVE_ID_INVALID)
            strncmp(szAliasName, "loading_", 8u);
        sounds[i] = Wave;
    }
}

// ============================================================================
// EntityManager - ea: 0x612420..0x612630 (inline COMDATs from g.o 0x4A6990)
// ============================================================================
extern int dword_F6A28C[];  // game.o data

// ea: 0x00612420
EntityManager::EntityManager()
{
    this->mWorld = nullptr;
    for (int i = 0; i < 16; ++i)
        this->mPlayers[i] = nullptr;
}

// ea: 0x00612470
EntityManager::~EntityManager()
{
}

// ea: 0x00612480
void EntityManager::CreatePlayers()
{
    Entity** mPlayers = this->mPlayers;
    if (this->mPlayers[0] == nullptr)
    {
        for (int i = 16; i != 0; --i)
        {
            TPakId PakId =
                PakManager::sInst->FindPakId(kPakTypeGlobal);
            *mPlayers++ = G_Spawn(PakId);
        }
    }
}

// ea: 0x006124C0
void EntityManager::CreateWorld()
{
    if (this->mWorld != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.cpp";
        AeAssert::gCurrentLine = 42;
        AeAssert::gCurrentExpr = "mWorld == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("World already created!"))
            __debugbreak();
    }
    TPakId PakId =
        PakManager::sInst->FindPakId(kPakTypeGlobal);
    this->mWorld = G_Spawn(PakId);
}

// ea: 0x00612530
int EntityManager::GetPlayerIndex(Entity* entity)
{
    int result = 0;
    Entity** i = this->mPlayers;
    while (entity != *i)
    {
        ++i;
        if (++result >= 16)
            return -1;
    }
    return result;
}

// ea: 0x00612560
int EntityManager::GetEntityController(Entity* entity)
{
    Client* client = entity->client;
    if (client == nullptr)
        return 0;
    int mServerClientIndex = client->mServerClientIndex;
    if (mServerClientIndex < 0
        || *(int*)((char*)&svs.clients[mServerClientIndex].netchan[8]) != 2)
        return 0;
    int v4 = 0;
    Entity** i = this->mPlayers;
    while (entity != *i)
    {
        ++i;
        if (++v4 >= 16)
        {
            v4 = -1;
            return dword_F6A28C[802 * v4];
        }
    }
    return dword_F6A28C[802 * v4];
}

// ea: 0x006125C0
bool EntityManager::IsLocalPlayer(Entity* entity)
{
    if (entity == nullptr)
        return false;
    Client* client = entity->client;
    return client != nullptr
        && (int)client->mServerClientIndex >= 0
        && *(int*)((char*)&svs.clients[client->mServerClientIndex].netchan[8])
            == 2;
}

// ea: 0x00612610
Entity* EntityManager::GetFirstLocalPlayer()
{
    int LocalClientIndex = LocalClient::FirstLocalClientIndex();
    return this->GetPlayer(LocalClientIndex);
}

// ea: 0x00612630
void EntityManager::SwapPlayers(int eA, int eB)
{
    Entity* v3 = this->mPlayers[eA];
    this->mPlayers[eA] = this->mPlayers[eB];
    this->mPlayers[eB] = v3;
    this->mPlayers[eA]->client->mServerClientIndex = eA;
    this->mPlayers[eB]->client->mServerClientIndex = eB;
}

extern bool gCareAboutCheckpoint;  // ?gCareAboutCheckpoint@@3_NA (game.o 0xDD74C8)
extern TPakId CurPakId(void);      // sv.o

// ea: 0x0062B0B0
void EntityManager::UnloadBank(TPakId pakId)
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    while (p != end)
    {
        Entity* v4 = *p;
        if (v4 != nullptr)
        {
            TPakId mPakId = (TPakId)v4->mPakId;
            if (mPakId == PAK_ID_INVALID)
                mPakId = CurPakId();
            if (mPakId == pakId)
            {
                gCareAboutCheckpoint = false;
                G_FreeEntity(v4, 0);
            }
        }
        ++p;
    }
}

// ea: 0x0062B110
void EntityManager::DeleteAllEntities()
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    while (p != end)
    {
        Entity* v4 = *p;
        if (v4 != nullptr)
        {
            gCareAboutCheckpoint = false;
            G_FreeEntity(v4, 0);
        }
        ++p;
    }
    EntityHandleDb::sInst.Compact();
    for (int i = 0; i < 16; ++i)
        this->mPlayers[i] = nullptr;
    this->mWorld = nullptr;
}

// g.o inline COMDATs (EntityManager.h)
// ea: 0x004A6990
EntityManager* EntityManager::Inst()
{
    return EntityManager::sInst;
}

// ea: 0x004A6A60
Entity* EntityManager::GetPlayer(int idx)
{
    if (idx >= 16)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 19;
        AeAssert::gCurrentExpr = "idx<16";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return this->mPlayers[idx];
}

// ea: 0x004A6AE0
Entity* EntityManager::GetWorld()
{
    return this->mWorld;
}

// ============================================================================
// SoundDevice::Sound - ea: 0x6129C0..0x612A10
// ============================================================================
// ea: 0x00612A10
SoundDevice::Sound::Sound()
{
    this->mEntHandle.mVal = 0;
    this->mHandle.mVal = 0;
    this->mDialogNotify.mHash = 0;
    this->mMinRange = 50.0f;
    this->mSource = -1;
    this->mWave = -1;
    this->mPaused = false;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
}

// ea: 0x006129C0
void SoundDevice::Sound::Reset()
{
    this->mMinRange = 50.0f;
    this->mSource = -1;
    this->mWave = -1;
    this->mPaused = false;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
}

// ============================================================================
// SoundDevice / Sound helpers - ea: 0x6025A0..0x6029D0 (SoundDevice.cpp)
// ============================================================================

// ea: 0x006025A0
float SoundDevice::GetWaveDuration(nslWaveID wave)
{
    if (wave == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 255;
        AeAssert::gCurrentExpr = "wave";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid wave ptr"))
            __debugbreak();
    }
    return (float)nslGetWaveLength(wave);
}

// ea: 0x00602600
float SoundDevice::Sound::GetPlaybackPosition() const
{
    return 0.0f;
}

// ea: 0x00602610
void SoundDevice::Sound::SetReverb(bool on)
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 452;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        if (on)
            nslSetSourceEffectOn(mSource);
        else
            nslSetSourceEffectOff(mSource);
    }
}

// ea: 0x00602690
float SoundDevice::Sound::GetVolume() const
{
    if (this->mSource == -1)
        return -2.0f;
    return nslGetSourceParam((nslSourceID)this->mSource, 0, -1.0f);
}

// ea: 0x006026B0
const char* SoundDevice::Sound::GetSourceName() const
{
    if (this->mSource == -1)
        return nullptr;
    return nslGetSourceName((nslSourceID)this->mSource);
}

// ea: 0x00602820
bool SoundDevice::Sound::IsQueuing() const
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        return nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUING;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 708;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    return this->mSource != -1
        && nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUING;
}

// ea: 0x00602890
bool SoundDevice::Sound::IsQueued() const
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        return nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUED;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 720;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    return this->mSource != -1
        && nslGetSourceState(mSource) == NSL_SOURCE_STATE_QUEUED;
}

// ea: 0x00602900
bool SoundDevice::Sound::IsPlaying() const
{
    if (this->mSource == -1)
        return false;
    nslSourceState SourceState =
        nslGetSourceState((nslSourceID)this->mSource);
    return SourceState == NSL_SOURCE_STATE_PLAYING
        || SourceState == NSL_SOURCE_STATE_QUEUING
        || SourceState == NSL_SOURCE_STATE_QUEUED
        || SourceState == NSL_SOURCE_STATE_PAUSED;
}

// ea: 0x00602930
bool SoundDevice::Sound::IsPaused() const
{
    return this->mPaused;
}

// ea: 0x00602940
bool SoundDevice::Sound::IsFinished() const
{
    bool result = false;
    if (!this->mPaused)
    {
        if (this->mSource == -1)
            return true;
        nslSourceState SourceState =
            nslGetSourceState((nslSourceID)this->mSource);
        if (SourceState != NSL_SOURCE_STATE_PLAYING
            && SourceState != NSL_SOURCE_STATE_QUEUING
            && SourceState != NSL_SOURCE_STATE_QUEUED
            && SourceState != NSL_SOURCE_STATE_PAUSED)
            return true;
    }
    return result;
}

// ea: 0x00602980
bool SoundDevice::Sound::IsLooped() const
{
    nslSourceState s = nslGetSourceState((nslSourceID)this->mSource);
    return (this->mSource != -1
            && (s == NSL_SOURCE_STATE_PLAYING
                || s == NSL_SOURCE_STATE_QUEUING
                || s == NSL_SOURCE_STATE_QUEUED
                || s == NSL_SOURCE_STATE_PAUSED)
            || this->mPaused)
        && nslIsWaveLooped((nslWaveID)this->mWave) != 0;
}

// ea: 0x006029D0
float SoundDevice::Sound::GetLength() const
{
    if (this->mSource == -1)
        return 0.0f;
    return (float)nslGetSourceLength((nslSourceID)this->mSource);
}

// ea: 0x006026D0
void SoundDevice::Sound::PlayQueued()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 667;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    LABEL_6:
        nslPlaySource(mSource);
}

// ea: 0x00602730
void SoundDevice::Sound::Pause()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 677;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        nslPauseSource(mSource);
        this->mPaused = true;
    }
}

// ea: 0x006027A0
void SoundDevice::Sound::Unpause()
{
    nslSourceID mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
        goto LABEL_6;
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
    AeAssert::gCurrentLine = 688;
    AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
    if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
        __debugbreak();
    mSource = (nslSourceID)this->mSource;
    if (this->mSource != -1)
    {
    LABEL_6:
        nslUnpauseSource(mSource);
        this->mPaused = false;
    }
}

// ea: 0x00602810
void SoundDevice::Sound::DampenGuard()
{
    if (this->mSource != -1)
        nslDampenGuardSource((nslSourceID)this->mSource);
}

// ============================================================================
// SoundDevice manager methods - ea: 0x602A10..0x602B40
// ============================================================================
// ea: 0x00602A10
void SoundDevice::ScaleVolume(float scale)
{
    this->mVolScale = scale;
    if (scale <= 0.0f)
        this->mVolScale = 0.0f;
    if (this->mVolScale >= 1.0f)
        this->mVolScale = 1.0f;
    nslSetMasterVolume(1.0f - ((1.0f - this->mVolScale)
                               * (1.0f - this->mVolScale)));
}

// ea: 0x00602A80
void SoundDevice::PauseAllSounds()
{
    Sound* p = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (p->mSource != -1)
            p->Pause();
        ++p;
    }
}

// ea: 0x00602AB0
SoundDevice::Sound* SoundDevice::GetSoundFromSourceId(nslSourceID id)
{
    int v2 = 0;
    for (Sound* i = this->mSounds; i->mSource != (int)id; ++i)
    {
        if (++v2 >= 0x200)
            return nullptr;
    }
    return &this->mSounds[v2];
}

// ea: 0x00602AE0
void SoundDevice::UnpauseAllSounds()
{
    Sound* p = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (p->mSource != -1)
            p->Unpause();
        ++p;
    }
}

// ea: 0x00602B10
int SoundDevice::GetNumberOfListeners()
{
    return this->mNumberOfListeners;
}

// ea: 0x00602B20
void SoundDevice::SetNumberOfListeners(int listeners)
{
    this->mNumberOfListeners = listeners;
    nslSetNumberOfListeners(listeners);
}

// ea: 0x00602B40
bool SoundDevice::IsSoundReady()
{
    return nslAreAllBanksLoaded() != 0 && nslNumBanksInUse() >= 2;
}

// ============================================================================
// SoundDevice init / bus / listener - ea: 0x6024A0..0x603EA0, 0x6397F0..
// ============================================================================
// nslSpeakerMode values verified vs GetOutputMode jump table + ctor (STEREO=2)
enum nslSpeakerMode {
    NSL_SPEAKER_MODE_MONO = 1,
    NSL_SPEAKER_MODE_STEREO = 2,
    NSL_SPEAKER_MODE_SURROUND = 3,
    NSL_SPEAKER_MODE_HEADPHONES = 4,
};
extern void nslUpdate();                                     // nsl
extern float nslGetBusPitch();                               // ?nslGetBusPitch@@YAMXZ
extern float nslGetBusVolume();                              // ?nslGetBusVolume@@YAMXZ
extern void nslSetBusPitch(unsigned int busId, float pitch); // nsl
extern void nslSetBusVolume(unsigned int busId, float volume);// nsl
extern void nslSetBusPitch(float pitch);                     // ?nslSetBusPitch@@YAXM@Z (master)
extern void nslSetBusVolume(float volume);                   // ?nslSetBusVolume@@YAXM@Z (master)
extern void nslSetBusPitchAddBus(unsigned int busId);        // nsl
extern void nslSetBusVolumeAddBus(unsigned int busId);       // nsl
extern void nslSetBusPitchRemoveBus(unsigned int busId);     // nsl
extern void nslSetBusVolumeRemoveBus(unsigned int busId);    // nsl
extern bool nslIsBusVolumeName(unsigned int busId);          // nsl
extern void nslSetEffect(const void* fx);                    // ?nslSetEffect@@YAXPBUnslEffect@@@Z
extern void nslDampen(float dampenLevel);                    // nsl
extern void nslUndampen();                                   // nsl
extern void nslSetSpeakerMode(nslSpeakerMode speakerMode);   // nsl
extern nslSpeakerMode nslGetSpeakerMode();                   // nsl
extern void nslSetListenerPosition(const float* pos);        // ?nslSetListenerPosition@@YAXQBM@Z
extern void nslSetListenerOrientation(const float* a,
                                      const float* b);       // ?nslSetListenerOrientation@@YAXQBM0@Z
extern void AnglesToAxis(const float* angles, float (*axis)[3]);  // core.o
extern int nslInit(const void* ip);                          // ?nslInit@@YAHPBUnslInitParams@@@Z (returns work size)
extern unsigned char nsl_initParams[0x44];                   // ?nsl_initParams@@3UnslInitParams@@A @ 0xE4B680
extern void nslStart(void* work);                            // ?nslStart@@YAXPAX@Z
extern void nslExit();                                       // ?nslExit@@YAXXZ
extern const char* nslGetWaveGroup(nslWaveID wave);          // ?nslGetWaveGroup@@YAPBDW4nslWaveID@@@Z
extern float nslGetWaveParam(nslWaveID wave, int b, float c);// nsl
extern nslSourceID nslNewSource(nslWaveID wave, int mImportance);  // ?nslNewSource@@YA?AW4nslSourceID@@W4nslWaveID@@H@Z
extern void nslQueueSource(nslSourceID sid);                 // ?nslQueueSource@@YAXW4nslSourceID@@@Z
extern unsigned int AeHash(const char* str);                 // ae_hash.cpp
extern int currCl;                                           // ?currCl@@3HA @ 0xF1579C
extern DbLinkedHandle<EntityHandleDb, Entity> g_SoundOnlyPlay;  // ?g_SoundOnlyPlay@@3V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@A @ 0xDEB5B4
extern void DebugRender_AddRenderer(void* self, void (*fp)());  // ?AddRenderer@DebugRender@@QAEXP6AXXZ@Z (render.o)
extern void* DebugRender_sInst;   // ?sInst@DebugRender@@2V1@A @ 0xF74D20

// ea: 0x006024A0
nslBankID SoundDevice::SyncLoadBank(const char* filename)
{
    nflFileID v2 = nflOpenFile(gNflMediaId, filename);
    if (v2 == (nflFileID)-1)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 173;
        AeAssert::gCurrentExpr = "bank_file != NFL_FILE_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("unable to open Sound bank"))
            __debugbreak();
        return (nslBankID)-1;
    }
    nslBankID Bank = nslLoadBank(0, v2, 0);
    nslBankID v5 = Bank;
    if (Bank == NSL_BANK_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 177;
        AeAssert::gCurrentExpr = "bank != NSL_BANK_ID_INVALID";
        if (AeAssert::IsIgnored()
            || !AeAssert::Assert("unable to load Sound bank"))
            return (nslBankID)-1;
        __debugbreak();
        return (nslBankID)-1;
    }
    int BankState = nslGetBankState(Bank);
    bool v7 = BankState < 0;
    if (BankState != 0)
    {
        while (!v7)
        {
            codNflUpdate();
            nslUpdate();
            int v8 = nslGetBankState(v5);
            v7 = v8 < 0;
            if (v8 == 0)
                return v5;
        }
        return (nslBankID)-1;
    }
    return v5;
}

// ea: 0x00602B60
void SoundDevice::DampenAllSounds(float level)
{
    nslDampen(level);
}

// ea: 0x00602B80
void SoundDevice::UndampenAllSounds()
{
    nslUndampen();
}

// ea: 0x00602B90
void SoundDevice::SetEnvironment(ESoundEnvironment env)
{
}

// ea: 0x00603640
void SoundDevice::UpdateReverb(float deltaTime)
{
    if (this->mUpdateReverb)
    {
        if (this->mRemainingReverbBlendTime <= 0.0f)
        {
            this->mUpdateReverb = false;
            memcpy(this->mCurrentReverb, this->mTargetReverb,
                   sizeof(this->mCurrentReverb));
            nslSetEffect(this->mCurrentReverb);
        }
        else
        {
            for (int i = 0; i < 14; ++i)
            {
                this->mCurrentReverb[i] =
                    (unsigned int)((float)this->mDeltaReverb[i] * deltaTime
                                   + (float)this->mCurrentReverb[i]);
            }
            this->mRemainingReverbBlendTime -= deltaTime;
            nslSetEffect(this->mCurrentReverb);
        }
    }
}

// ea: 0x00603820
bool SoundDevice::BusVolumeIsName(const char* name)
{
    return nslIsBusVolumeName(AeHash(name));
}

// ea: 0x00603840
void SoundDevice::BusPitchFade(const char* busName, float pitch, float time)
{
    unsigned int v5 = AeHash(busName);
    this->mBusPitchRemainingTime = time;
    float currentPitch = 1.0f / nslGetBusPitch();
    this->mBusPitchTargetPitch = 1.0f / pitch;
    this->mBusPitchCurrentPitch = currentPitch;
    this->mBusPitchDeltaPitch =
        (this->mBusPitchTargetPitch - currentPitch) / time;
    nslSetBusPitch(v5, 1.0f / currentPitch);
}

// ea: 0x006038C0
void SoundDevice::BusVolumeFade(const char* busName, float volume, float time)
{
    unsigned int v5 = AeHash(busName);
    this->mBusVolumeRemainingTime = time;
    float currentVolume = nslGetBusVolume();
    this->mBusVolumeCurrentVolume = currentVolume;
    this->mBusVolumeTargetVolume = volume;
    this->mBusVolumeDeltaVolume = (volume - currentVolume) / time;
    nslSetBusVolume(v5, currentVolume);
}

// ea: 0x00603920
void SoundDevice::BusPitchAddBus(const char* busName)
{
    nslSetBusPitchAddBus(AeHash(busName));
}

// ea: 0x00603940
void SoundDevice::BusVolumeAddBus(const char* busName)
{
    nslSetBusVolumeAddBus(AeHash(busName));
}

// ea: 0x00603960
void SoundDevice::BusPitchRemoveBus(const char* busName)
{
    nslSetBusPitchRemoveBus(AeHash(busName));
}

// ea: 0x00603980
void SoundDevice::BusVolumeRemoveBus(const char* busName)
{
    nslSetBusVolumeRemoveBus(AeHash(busName));
}

// ea: 0x006039A0
void SoundDevice::UpdateBusPitchFade(float deltaTime)
{
    float remaining = this->mBusPitchRemainingTime;
    if (remaining >= 0.0f)
    {
        float v3 = remaining - deltaTime;
        this->mBusPitchRemainingTime = v3;
        if (v3 >= 0.0f)
        {
            this->mBusPitchCurrentPitch =
                (this->mBusPitchDeltaPitch * deltaTime)
                + this->mBusPitchCurrentPitch;
        }
        else
        {
            this->mBusPitchRemainingTime = -1.0f;
            this->mBusPitchCurrentPitch = this->mBusPitchTargetPitch;
        }
        nslSetBusPitch(1.0f / this->mBusPitchCurrentPitch);
    }
}

// ea: 0x00603A30
void SoundDevice::UpdateBusVolumeFade(float deltaTime)
{
    float remaining = this->mBusVolumeRemainingTime;
    if (remaining >= 0.0f)
    {
        float v3 = remaining - deltaTime;
        this->mBusVolumeRemainingTime = v3;
        if (v3 >= 0.0f)
        {
            this->mBusVolumeCurrentVolume =
                (this->mBusVolumeDeltaVolume * deltaTime)
                + this->mBusVolumeCurrentVolume;
            nslSetBusVolume(this->mBusVolumeCurrentVolume);
        }
        else
        {
            this->mBusVolumeRemainingTime = -1.0f;
            this->mBusVolumeCurrentVolume = this->mBusVolumeTargetVolume;
            nslSetBusVolume(this->mBusVolumeTargetVolume);
        }
    }
}

// ea: 0x00603AC0
void SoundDevice::UpdateListener()
{
    Entity* player = EntityManager::sInst->GetPlayer(currCl);
    const math::Position3& origin = player->r.currentOrigin;
    float axis[3][3];
    AnglesToAxis(player->r.currentAngles.v.m128_f32, axis);
    const float* upv = axis[1];
    const float* pv = axis[2];
    nslSetListenerPosition(origin.v.m128_f32);
    nslSetListenerOrientation(upv, pv);
    this->mDebugListenerPosition[0] = origin.v.m128_f32[0];
    this->mDebugListenerPosition[1] = origin.v.m128_f32[1];
    this->mDebugListenerPosition[2] = origin.v.m128_f32[2];
    this->mDebugListenerForward[0] = upv[0];
    this->mDebugListenerForward[1] = upv[1];
    this->mDebugListenerForward[2] = upv[2];
    this->mDebugListenerUp[0] = pv[0];
    this->mDebugListenerUp[1] = pv[1];
    this->mDebugListenerUp[2] = pv[2];
}

// ea: 0x00603D90
float SoundDevice::GetGroupVolume(const char* group) const
{
    const char* v2 = group;
    if ((unsigned int)group < 0x10000)
        v2 = "SFX";
    if (_stricmp(v2, "SFX") != 0)
        _stricmp(v2, "MUSIC");
    return 1.0f;
}

// ea: 0x00603DD0
void SoundDevice::SetOutputMode(EOutputMode mode)
{
    switch (mode)
    {
    case kMono:
        nslSetSpeakerMode(NSL_SPEAKER_MODE_MONO);
        break;
    case kStereo:
        nslSetSpeakerMode(NSL_SPEAKER_MODE_STEREO);
        break;
    case kHeadPhones:
        nslSetSpeakerMode(NSL_SPEAKER_MODE_HEADPHONES);
        break;
    case kSurround:
        nslSetSpeakerMode(NSL_SPEAKER_MODE_SURROUND);
        break;
    default:
        return;
    }
}

// ea: 0x00603E30
SoundDevice::EOutputMode SoundDevice::GetOutputMode() const
{
    switch (nslGetSpeakerMode())
    {
    case NSL_SPEAKER_MODE_MONO:
        return kMono;
    case NSL_SPEAKER_MODE_STEREO:
        return kStereo;
    case NSL_SPEAKER_MODE_SURROUND:
        return kSurround;
    case NSL_SPEAKER_MODE_HEADPHONES:
        return kHeadPhones;
    default:
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 1680;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error("invalid speaker mode"))
            __debugbreak();
        return kMono;
    }
}

// ea: 0x00603EA0
void SetSurfaceTypeSounds(nslWaveID* const sounds, nslWaveID filler)
{
    for (int i = 0; i < 23; ++i)
        sounds[i] = filler;
}

// ============================================================================
// SoundDevice lifecycle / queue - ea: 0x6397F0..0x63A5A0
// ============================================================================
extern void* mem_heap_malloc(unsigned int size);  // mem_lib
extern void mem_heap_free(void* ptr);             // mem_lib
extern cvar_t* Cvar_Get(const char* var_name,
                        const char* var_value, int flags);  // core.o

// ea: 0x006397F0
SoundDevice::SoundDevice()
{
    for (int i = 0; i < 512; ++i)
        new (&this->mSounds[i]) Sound();
    for (int i = 0; i < 16; ++i)
    {
        this->mCrossFadeInfo[i].mSound1.mVal = 0;
        this->mCrossFadeInfo[i].mSound2.mVal = 0;
    }
    DebugRender_AddRenderer((void*)&DebugRender_sInst,
                            &SoundDevice::SingletonDebugRender);
    this->mNumberOfListeners = 1;
    nslSetNumberOfListeners(1);
    *(unsigned int*)&this->mNslParams[4] = 1;
    *(unsigned int*)this->mNslParams = 512;
    memcpy(this->mNslParams, nsl_initParams, sizeof(this->mNslParams));
    void* v5 = mem_heap_malloc(nslInit(this->mNslParams));
    this->mNslBuffer = v5;
    if (v5 == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 101;
        AeAssert::gCurrentExpr = "mNslBuffer";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("unable to alloc NSL work buffer"))
            __debugbreak();
    }
    nslStart(this->mNslBuffer);
    nslSetSpeakerMode(NSL_SPEAKER_MODE_STEREO);
    nslSetMasterVolume(1.0f);
    nslSetBusVolume(0xCDCDCDCD, 1.0f);
    nslSetBusPitch(0xCDCDCDCD, 1.0f);
    nslUndampen();
    this->mBusVolumeRemainingTime = -1.0f;
    this->mBusPitchRemainingTime = -1.0f;
    this->SetReverb("Preset_NoReverb", true);
    this->mMainBank = NSL_BANK_ID_INVALID;
    this->mVolScale = 1.0f;
    this->mShowStreams = Cvar_Get("snd_showstreams", "0", 256);
    this->mShowListenerPosition =
        Cvar_Get("snd_showlistenerposition", "0", 256);
    this->mShowEmitterPosition =
        Cvar_Get("snd_showemitterposition", "0", 256);
}

// ea: 0x00646610
SoundDevice::~SoundDevice()
{
    this->StopAllSounds();
    if (this->mMainBank != NSL_BANK_ID_INVALID)
        nslFreeBank(this->mMainBank);
    nslExit();
    mem_heap_free(this->mNslBuffer);
    this->mNslBuffer = nullptr;
    for (int i = 0; i < 512; ++i)
        this->mSounds[i].~Sound();
}

// ea: 0x006629D0 (static)
void SoundDevice::SingletonDebugRender()
{
    SoundDevice::sInst->DebugRender();
}

// ea: 0x0063A060
int SoundDevice::GetFreeSlot()
{
    int v1 = 0;
    while (this->mSounds[v1].mSource != NSL_SOURCE_ID_INVALID)
    {
        if (++v1 >= 0x200)
            return -1;
    }
    Sound* v4 = &this->mSounds[v1];
    if (v4->mHandle.mVal != 0)
        SoundDevice::SoundHandleDb::sInst.ReleaseHandle(v4->mHandle);
    Handle v6 = SoundDevice::SoundHandleDb::sInst.AllocateHandle();
    v4->mHandle.mVal = v6.mVal;
    SoundDevice::SoundHandleDb::sInst.BindObjectToHandle(v6, v4);
    return v1;
}

// ea: 0x006399D0
void SoundDevice::Sound::Queue(
    nslWaveID wave, float vol, float pitch, float minrange, float maxrange,
    const math::Position3* pos, const math::Dir3* vel, bool autoRelease,
    DbLinkedHandle<EntityHandleDb, Entity> entHandle, bool mImportant)
{
    if ((__fpclass(vel->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel->v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(vol) & 0x297) != 0
        || (__fpclass(pitch) & 0x297) != 0
        || (__fpclass(minrange) & 0x297) != 0
        || (__fpclass(maxrange) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to play %s\n",
            nslWaveGetName(wave));
        this->Stop();
    }
    else
    {
        bool v12 = this->mSource == NSL_SOURCE_ID_INVALID;
        this->mWave = wave;
        if (!v12)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 325;
            AeAssert::gCurrentExpr = "mSource == NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("source already in use"))
                __debugbreak();
        }
        this->mAutoRelease = autoRelease;
        nslSourceID v13 = nslNewSource(wave, mImportant);
        this->mSource = v13;
        if (v13 != NSL_SOURCE_ID_INVALID)
            goto LABEL_20;
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 329;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("failed to create source"))
            __debugbreak();
        if (this->mSource != NSL_SOURCE_ID_INVALID)
        {
        LABEL_20:
            nslQueueSource((nslSourceID)this->mSource);
            SoundDevice* v14 = SoundDevice::sInst;
            const char* WaveGroup = nslGetWaveGroup(wave);
            this->mGroupVolume = v14->GetGroupVolume(WaveGroup);
            float autoReleasea =
                vol <= 0.0f ? nslGetWaveParam(wave, 0, 1.0f) : vol;
            this->SetVolume(autoReleasea);
            float autoReleaseb =
                pitch <= 0.0f ? nslGetWaveParam(wave, 1, 1.0f) : pitch;
            this->SetPitch(autoReleaseb);
            float autoReleasec = maxrange <= 0.0f
                ? nslGetWaveParam(wave, 26, 1500.0f)
                : maxrange;
            float mImportanta = minrange <= 0.0f
                ? nslGetWaveParam(wave, 25, 50.0f)
                : minrange;
            this->SetRange(mImportanta, autoReleasec);
            this->SetPosition(*pos);
            this->SetVelocity(*vel);
            nslSetSourceEffectOn((nslSourceID)this->mSource);
            this->mEntHandle.mVal = entHandle.mHandle.mVal;
        }
    }
}

// ea: 0x00639CE0
void SoundDevice::Sound::Play(
    nslWaveID wave, float vol, float pitch, float minrange, float maxrange,
    const math::Position3* pos, const math::Dir3* vel, bool autoRelease,
    DbLinkedHandle<EntityHandleDb, Entity> entHandle, bool mImportant)
{
    if ((__fpclass(vel->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel->v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos->v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(vol) & 0x297) != 0
        || (__fpclass(pitch) & 0x297) != 0
        || (__fpclass(minrange) & 0x297) != 0
        || (__fpclass(maxrange) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to play %s\n",
            nslWaveGetName(wave));
        this->Stop();
        return;
    }
    bool v12 = this->mSource == NSL_SOURCE_ID_INVALID;
    this->mWave = wave;
    if (!v12)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 380;
        AeAssert::gCurrentExpr = "mSource == NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("source already in use"))
            __debugbreak();
    }
    this->mAutoRelease = autoRelease;
    nslSourceID v13 = nslNewSource(wave, mImportant);
    this->mSource = v13;
    if (v13 == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 384;
        AeAssert::gCurrentExpr = "mSource";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Failed to play sound"))
            __debugbreak();
    }
    if (!mImportant)
    {
        if (this->mSource == NSL_SOURCE_ID_INVALID)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 388;
            AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert(
                    "Failed to create source (non player weapon sound)"))
                __debugbreak();
        }
    }
    if (this->mSource == NSL_SOURCE_ID_INVALID)
        return;
    SoundDevice* v14 = SoundDevice::sInst;
    const char* WaveGroup = nslGetWaveGroup(wave);
    this->mGroupVolume = v14->GetGroupVolume(WaveGroup);
    float autoReleasea =
        vol <= 0.0f ? nslGetWaveParam(wave, 0, 1.0f) : vol;
    this->SetVolume(autoReleasea);
    float autoReleaseb =
        pitch <= 0.0f ? nslGetWaveParam(wave, 1, 1.0f) : pitch;
    this->SetPitch(autoReleaseb);
    float autoReleasec = maxrange <= 0.0f
        ? nslGetWaveParam(wave, 26, 1500.0f)
        : maxrange;
    float mImportanta = minrange <= 0.0f
        ? nslGetWaveParam(wave, 25, 50.0f)
        : minrange;
    this->SetRange(mImportanta, autoReleasec);
    this->SetPosition(*pos);
    this->SetVelocity(*vel);
    nslSetSourceEffectOn((nslSourceID)this->mSource);
    nslPlaySource((nslSourceID)this->mSource);
    this->mEntHandle.mVal = entHandle.mHandle.mVal;
}

// ea: 0x0063A040
void SoundDevice::Sound::SetPoPtr(const math::Mat43* poPtr)
{
    this->mPoPtr = (void*)poPtr;
    this->SetPosition(poPtr->w);
}

// ea: 0x0063A0D0
DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>
SoundDevice::QueueSound(nslWaveID id,
                        DbLinkedHandle<EntityHandleDb, Entity> entHandle,
                        bool mImportant, bool autoRelease,
                        const math::Position3& pos, const math::Dir3& vel,
                        float vol, float pitch, float min, float max)
{
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> result;
    if ((__fpclass(vel.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(vol) & 0x297) != 0
        || (__fpclass(pitch) & 0x297) != 0
        || (__fpclass(min) & 0x297) != 0
        || (__fpclass(max) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to play %s\n",
            nslWaveGetName(id));
    }
    else
    {
        int FreeSlot = this->GetFreeSlot();
        if (FreeSlot >= 0 && id != NSL_WAVE_ID_INVALID)
        {
            Sound* v14 = &this->mSounds[FreeSlot];
            v14->Queue(id, vol, pitch, min, max, &pos, &vel, autoRelease,
                       entHandle, mImportant);
            result.mHandle.mVal = v14->mHandle.mVal;
            return result;
        }
    }
    result.mHandle.mVal = 0;
    return result;
}

// ea: 0x0063A260
DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>
SoundDevice::PlaySound(nslWaveID id,
                       DbLinkedHandle<EntityHandleDb, Entity> entHandle,
                       bool mImportant, bool autoRelease,
                       const math::Position3& pos, const math::Dir3& vel,
                       float vol, float pitch, float min, float max)
{
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> result;
    if ((__fpclass(vel.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(vol) & 0x297) != 0
        || (__fpclass(pitch) & 0x297) != 0
        || (__fpclass(min) & 0x297) != 0
        || (__fpclass(max) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to play %s\n",
            nslWaveGetName(id));
    }
    else
    {
        int FreeSlot = this->GetFreeSlot();
        if (FreeSlot < 0)
        {
            tlWarning("increase NUM_AUDIO_SLOTS\n");
            result.mHandle.mVal = 0;
            return result;
        }
        if (id != NSL_WAVE_ID_INVALID && cls.state != CA_LOADING)
        {
            Sound* v15 = &this->mSounds[FreeSlot];
            v15->Play(id, vol, pitch, min, max, &pos, &vel, autoRelease,
                      entHandle, mImportant);
            result.mHandle.mVal = v15->mHandle.mVal;
            return result;
        }
    }
    result.mHandle.mVal = 0;
    return result;
}

// ea: 0x0063A420
DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>
SoundDevice::PlaySound(const char* name,
                       DbLinkedHandle<EntityHandleDb, Entity> entHandle,
                       bool mImportant, bool autoRelease,
                       const math::Position3& pos, const math::Dir3& vel,
                       float vol, float pitch, float min, float max)
{
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> result;
    if ((__fpclass(vel.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(vol) & 0x297) != 0
        || (__fpclass(pitch) & 0x297) != 0
        || (__fpclass(min) & 0x297) != 0
        || (__fpclass(max) & 0x297) != 0)
    {
        tlWarning(
            "A NAN was passed into the sound system while trying to play %s\n",
            name);
    }
    else if (name != nullptr)
    {
        nslWaveID Wave = this->FindWave(name);
        if (Wave != NSL_WAVE_ID_INVALID)
            return this->PlaySound(Wave, entHandle, mImportant, autoRelease,
                                   pos, vel, vol, pitch, min, max);
    }
    result.mHandle.mVal = 0;
    return result;
}

// ea: 0x0063A5A0
void SoundDevice::FrameAdvance(float delta)
{
    nslUpdate();
    SoundDevice* v3 = this;
    for (int i = 512; i != 0; --i)
    {
        bool v8 = false;
        if (g_SoundOnlyPlay.mHandle.mVal != (unsigned int)-1)
        {
            if (v3->mSounds[0].mEntHandle.mVal
                == g_SoundOnlyPlay.mHandle.mVal)
            {
                if (v3->mSounds[0].mSource == NSL_SOURCE_ID_INVALID)
                    goto next;
                if (v3->mSounds[0].mPaused
                    || (nslGetSourceState(
                            (nslSourceID)v3->mSounds[0].mSource)
                        == NSL_SOURCE_STATE_PLAYING
                        || nslGetSourceState(
                               (nslSourceID)v3->mSounds[0].mSource)
                            == NSL_SOURCE_STATE_QUEUING
                        || nslGetSourceState(
                               (nslSourceID)v3->mSounds[0].mSource)
                            == NSL_SOURCE_STATE_QUEUED
                        || nslGetSourceState(
                               (nslSourceID)v3->mSounds[0].mSource)
                            == NSL_SOURCE_STATE_PAUSED))
                {
                    v3->mSounds[0].Update();
                    goto next;
                }
            }
            v8 = v3->mSounds[0].mSource == NSL_SOURCE_ID_INVALID;
            goto LABEL_33;
        }
        if (v3->mSounds[0].mAutoRelease)
        {
            if (v3->mSounds[0].mSource == NSL_SOURCE_ID_INVALID)
                goto next;
            if (!v3->mSounds[0].mPaused)
            {
                nslSourceState v5 =
                    nslGetSourceState((nslSourceID)v3->mSounds[0].mSource);
                if (v5 != NSL_SOURCE_STATE_PLAYING
                    && v5 != NSL_SOURCE_STATE_QUEUING
                    && v5 != NSL_SOURCE_STATE_QUEUED
                    && v5 != NSL_SOURCE_STATE_PAUSED)
                    v3->mSounds[0].Stop();
            }
        }
        if (v3->mSounds[0].mSource != NSL_SOURCE_ID_INVALID)
        {
            if (v3->mSounds[0].mPaused
                || (nslGetSourceState(
                        (nslSourceID)v3->mSounds[0].mSource)
                    == NSL_SOURCE_STATE_PLAYING
                    || nslGetSourceState(
                           (nslSourceID)v3->mSounds[0].mSource)
                        == NSL_SOURCE_STATE_QUEUING
                    || nslGetSourceState(
                           (nslSourceID)v3->mSounds[0].mSource)
                        == NSL_SOURCE_STATE_QUEUED
                    || nslGetSourceState(
                           (nslSourceID)v3->mSounds[0].mSource)
                        == NSL_SOURCE_STATE_PAUSED))
                v3->mSounds[0].Update();
            if (v3->mSounds[0].mSource != NSL_SOURCE_ID_INVALID
                && !v3->mSounds[0].mPaused)
            {
                nslSourceState v7 =
                    nslGetSourceState((nslSourceID)v3->mSounds[0].mSource);
                if (v7 != NSL_SOURCE_STATE_PLAYING
                    && v7 != NSL_SOURCE_STATE_QUEUING
                    && v7 != NSL_SOURCE_STATE_QUEUED)
                {
                    v8 = v7 == NSL_SOURCE_STATE_PAUSED;
                LABEL_33:
                    if (!v8)
                        v3->mSounds[0].Stop();
                }
            }
        }
    next:
        v3 = (SoundDevice*)((char*)v3 + 0x3C);
    }
    this->UpdateReverb(delta);
    this->UpdateCrossFade(delta);
    this->UpdateBusPitchFade(delta);
    this->UpdateBusVolumeFade(delta);
}

// ============================================================================
// SoundDevice::SetReverb - ea: 0x602BA0 (reverb preset table, bits exact)
// ============================================================================
// ea: 0x00602BA0
void SoundDevice::SetReverb(const char* preset, bool immediate)
{
    if (preset != nullptr && *preset != 0)
    {
        int _newFx[14];
        int v3, v4, v5;
        if (_stricmp(preset, "Preset_Alley") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1063004406;
            _newFx[1] = -270;
            _newFx[5] = -1204;
            _newFx[7] = -4;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Arena") == 0)
        {
            _newFx[3] = 1088925204;
            _newFx[4] = 1051260355;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[1] = -698;
            _newFx[5] = -1166;
            _newFx[7] = 16;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Auditorium") == 0)
        {
            _newFx[3] = 1082801521;
            _newFx[4] = 1058474557;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[1] = -476;
            _newFx[5] = -789;
            _newFx[7] = -289;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Bathroom") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1057635697;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[9] = 1120403456;
            v5 = 1114636288;
            _newFx[0] = -1000;
            _newFx[1] = -1200;
            _newFx[5] = -370;
            _newFx[7] = 1030;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_CarpetedHallway") == 0)
        {
            _newFx[3] = 1050253722;
            _newFx[4] = 1036831949;
            _newFx[6] = 990057071;
            v4 = 1022739087;
            _newFx[1] = -4000;
            _newFx[5] = -1831;
            _newFx[7] = -1630;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Cave") == 0)
        {
            _newFx[3] = 1077558641;
            _newFx[4] = 1067869798;
            _newFx[6] = 1014350479;
            v4 = 1018444120;
            _newFx[1] = 0;
            _newFx[5] = -602;
            _newFx[7] = -302;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_City") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1059816735;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[9] = 1112014848;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -800;
            _newFx[5] = -2273;
            _newFx[7] = -2217;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_ConcertHall") == 0)
        {
            _newFx[3] = 1081794888;
            _newFx[4] = 1060320051;
            _newFx[6] = 1017370378;
            v4 = 1022202216;
            _newFx[1] = -500;
            _newFx[5] = -1230;
            _newFx[7] = -2;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Default2") == 0)
        {
            _newFx[3] = 1065353216;
            _newFx[4] = 1056964608;
            _newFx[6] = 1017370378;
            _newFx[8] = 1025758986;
            _newFx[1] = 0;
            _newFx[9] = 1120403456;
            _newFx[10] = 1120403456;
            goto LABEL_60;
        }
        if (_stricmp(preset, "Preset_Default") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1062501089;
            _newFx[6] = 1004888130;
            _newFx[8] = 1010055512;
            _newFx[0] = -1000;
            _newFx[1] = -100;
            _newFx[5] = -2602;
            _newFx[7] = 200;
            _newFx[9] = 1120403456;
            _newFx[10] = 1120403456;
            goto LABEL_61;
        }
        if (_stricmp(preset, "Preset_Forest") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1057635697;
            _newFx[6] = 1042670420;
            _newFx[8] = 1035221336;
            _newFx[9] = 1117650944;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -3300;
            _newFx[5] = -2560;
            _newFx[7] = -613;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_Generic") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1062501089;
            _newFx[1] = -100;
            _newFx[5] = -2602;
            _newFx[7] = 200;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Hallway") == 0)
        {
            _newFx[3] = 1069463634;
            v3 = 1058474557;
            _newFx[1] = -300;
            _newFx[5] = -1219;
            _newFx[7] = 441;
            goto LABEL_55;
        }
        if (_stricmp(preset, "Preset_Hangar") == 0)
        {
            _newFx[3] = 1092668621;
            _newFx[4] = 1047233823;
            _newFx[6] = 1017370378;
            v4 = 1022739087;
            _newFx[0] = -1000;
            _newFx[1] = -1000;
            _newFx[5] = -602;
            _newFx[7] = 198;
            goto LABEL_57;
        }
        if (_stricmp(preset, "Preset_LivingRoom") == 0)
        {
            _newFx[3] = 1056964608;
            _newFx[4] = 1036831949;
            _newFx[6] = 994352038;
            v4 = 998445679;
            _newFx[1] = -6000;
            _newFx[5] = -1376;
            _newFx[7] = -1104;
            goto LABEL_56;
        }
        if (_stricmp(preset, "Preset_Mountains") == 0)
        {
            _newFx[3] = 1069463634;
            _newFx[4] = 1045891645;
            _newFx[6] = 1050253722;
            _newFx[8] = 1036831949;
            _newFx[9] = 1104674816;
            v5 = 1120403456;
            _newFx[0] = -1000;
            _newFx[1] = -2500;
            _newFx[5] = -2780;
            _newFx[7] = -2014;
            goto LABEL_58;
        }
        if (_stricmp(preset, "Preset_NoReverb") != 0)
        {
            if (_stricmp(preset, "Preset_PaddedCell") == 0)
            {
                _newFx[3] = 1043207291;
                _newFx[4] = 1036831949;
                _newFx[6] = 981668463;
                v4 = 990057071;
                _newFx[1] = -6000;
                _newFx[5] = -1204;
                _newFx[7] = 207;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_ParkingLot") == 0)
            {
                _newFx[3] = 1070805811;
                _newFx[4] = 1069547520;
                _newFx[6] = 1006834287;
                v4 = 1011129254;
                _newFx[1] = 0;
                _newFx[5] = -1363;
                _newFx[7] = -1153;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_Plain") == 0)
            {
                _newFx[3] = 1069463634;
                _newFx[4] = 1056964608;
                _newFx[6] = 1043811271;
                _newFx[8] = 1036831949;
                _newFx[9] = 1101529088;
                v5 = 1120403456;
                _newFx[0] = -1000;
                _newFx[1] = -2000;
                _newFx[5] = -2466;
                _newFx[7] = -2514;
                goto LABEL_58;
            }
            if (_stricmp(preset, "Preset_Quarry") == 0)
                goto LABEL_43;
            if (_stricmp(preset, "Preset_Room") == 0)
            {
                _newFx[3] = 1053609165;
                _newFx[4] = 1062501089;
                _newFx[6] = 990057071;
                v4 = 994352038;
                _newFx[1] = -454;
                _newFx[5] = -1646;
                _newFx[7] = 53;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_SewerPipe") == 0)
            {
                _newFx[3] = 1077139210;
                _newFx[4] = 1041194025;
                _newFx[6] = 1013276738;
                _newFx[8] = 1017907249;
                _newFx[9] = 1117782016;
                v5 = 1114636288;
                _newFx[0] = -1000;
                _newFx[1] = -1000;
                _newFx[5] = 429;
                _newFx[7] = 648;
                goto LABEL_58;
            }
            if (_stricmp(preset, "Preset_Quarry") == 0)
            {
            LABEL_43:
                _newFx[3] = 1069463634;
                _newFx[4] = 1062501089;
                _newFx[6] = 1031396131;
                v4 = 1020054733;
                _newFx[0] = -1000;
                _newFx[1] = -1000;
                _newFx[5] = -10000;
                _newFx[7] = 500;
                goto LABEL_57;
            }
            if (_stricmp(preset, "Preset_StoneRoom") == 0)
            {
                _newFx[3] = 1075042058;
                _newFx[4] = 1059313418;
                _newFx[6] = 1011129254;
                v4 = 1015759766;
                _newFx[1] = -300;
                _newFx[5] = -711;
                _newFx[7] = 83;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_StoneCorridor") == 0)
            {
                _newFx[3] = 1076677837;
                _newFx[4] = 1061830001;
                _newFx[6] = 1012202996;
                v4 = 1017370378;
                _newFx[1] = -237;
                _newFx[5] = -1214;
                _newFx[7] = 395;
                goto LABEL_56;
            }
            if (_stricmp(preset, "Preset_Underwater") == 0)
            {
                _newFx[3] = 1069463634;
                v3 = 1036831949;
                _newFx[1] = -4000;
                _newFx[5] = -449;
                _newFx[7] = 1700;
                goto LABEL_55;
            }
        }
        _newFx[1] = -10000;
        _newFx[3] = 1065353216;
        _newFx[4] = 1065353216;
        _newFx[6] = 0;
        memset(&_newFx[8], 0, 12);
        goto LABEL_60;
    LABEL_55:
        _newFx[4] = v3;
        _newFx[6] = 1004888130;
        v4 = 1010055512;
        goto LABEL_56;
    LABEL_56:
        _newFx[0] = -1000;
        goto LABEL_57;
    LABEL_57:
        _newFx[8] = v4;
        v5 = 1120403456;
        _newFx[9] = 1120403456;
        goto LABEL_58;
    LABEL_58:
        _newFx[10] = v5;
        _newFx[12] = 25;
        goto LABEL_62;
    LABEL_60:
        _newFx[0] = -10000;
        _newFx[5] = -10000;
        _newFx[7] = -10000;
        goto LABEL_61;
    LABEL_61:
        _newFx[12] = -1;
        goto LABEL_62;
    LABEL_62:
        _newFx[13] = 0;
        _newFx[11] = 1167867904;
        _newFx[2] = 0;
        memcpy(this->mTargetReverb, _newFx, sizeof(this->mTargetReverb));
        memcpy(this->mCurrentReverb, _newFx, sizeof(this->mCurrentReverb));
        this->mUpdateReverb = true;
        this->mRemainingReverbBlendTime = 0.0f;
    }
}

// ea: 0x0062C020
SoundDevice::Sound::~Sound()
{
    nslSourceState SourceState =
        nslGetSourceState((nslSourceID)this->mSource);
    if (this->mSource != NSL_SOURCE_ID_INVALID
        && (this->mPaused || SourceState == NSL_SOURCE_STATE_PLAYING
            || SourceState == NSL_SOURCE_STATE_QUEUING
            || SourceState == NSL_SOURCE_STATE_QUEUED
            || SourceState == NSL_SOURCE_STATE_PAUSED))
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 288;
        AeAssert::gCurrentExpr =
            "mSource == NSL_SOURCE_ID_INVALID || IsFinished()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("source destructed while still playing"))
            __debugbreak();
    }
    unsigned int v3 = this->mEntHandle.mVal & 0xFFF;
    if (v3 < 0x540
        && this->mEntHandle.mVal >> 12
            == EntityHandleDb::sInst.mElements[v3].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        if (mObject != nullptr && this->mDialogNotify.mHash != 0)
            mObject->Notify(this->mDialogNotify);
    }
}

// ea: 0x0062C0D0
void SoundDevice::Sound::Stop()
{
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        nslWaveGetHash((nslWaveID)this->mWave);
        if (this->mSource == g_break_on_stop)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 421;
            AeAssert::gCurrentExpr = "mSource != g_break_on_stop";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("no!"))
                __debugbreak();
        }
        SoundDevice::SoundHandleDb::sInst.ReleaseHandle(this->mHandle);
        this->mHandle.mVal = 0;
        nslStopSource((nslSourceID)this->mSource);
        nslFreeSource((nslSourceID)this->mSource);
        this->mSource = NSL_SOURCE_ID_INVALID;
    }
    unsigned int v2 = this->mEntHandle.mVal & 0xFFF;
    if (v2 < 0x540
        && this->mEntHandle.mVal >> 12
            == EntityHandleDb::sInst.mElements[v2].mKey)
    {
        Entity* mObject = EntityHandleDb::sInst.mElements[v2].mObject;
        if (mObject != nullptr)
        {
            HashString v4;
            v4.mHash = nslWaveGetHash((nslWaveID)this->mWave);
            EffectEventSys::sInst->SendSpecificSoundNotify(mObject, v4);
            if (this->mDialogNotify.mHash != 0)
                mObject->Notify(this->mDialogNotify);
        }
    }
    this->mMinRange = 50.0f;
    this->mPaused = false;
    this->mDialogNotify.mHash = 0;
    this->mPoPtr = nullptr;
    this->mSource = NSL_SOURCE_ID_INVALID;
    this->mWave = NSL_WAVE_ID_INVALID;
    this->mAutoRelease = true;
    this->mPitch = 1.0f;
    this->mVolume = 1.0f;
    this->mMaxRange = 1500.0f;
    this->mGroupVolume = 1.0f;
}

// ea: 0x0062C210
void SoundDevice::Sound::SetVolume(float vol)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 474;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        if ((__fpclass(vol) & 0x297) != 0)
        {
            const char* Name = nslWaveGetName((nslWaveID)this->mWave);
            tlWarning("A NAN was passed into the sound system while trying to adjust the volume on %s\n", Name);
            this->Stop();
        }
        else
        {
            nslSetSourceParam((nslSourceID)this->mSource, 0,
                              this->mGroupVolume * vol);
            this->mVolume = vol;
        }
    }
}

// ea: 0x0062C2D0
void SoundDevice::Sound::SetPitch(float pitch)
{
    if ((__fpclass(pitch) & 0x297) != 0)
    {
        const char* Name = nslWaveGetName((nslWaveID)this->mWave);
        tlWarning("A NAN was passed into the sound system while trying to adjust the pitch on %s\n", Name);
        this->Stop();
    }
    else
    {
        nslSourceID mSource = (nslSourceID)this->mSource;
        if (this->mSource == NSL_SOURCE_ID_INVALID)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 528;
            AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
                __debugbreak();
            mSource = (nslSourceID)this->mSource;
        }
        if (this->mSource != NSL_SOURCE_ID_INVALID)
        {
            nslSetSourceParam(mSource, 1, pitch);
            this->mPitch = pitch;
        }
    }
}

// ea: 0x0062C380
void SoundDevice::Sound::SetRange(float min, float max)
{
    if ((__fpclass(min) & 0x297) != 0 || (__fpclass(max) & 0x297) != 0)
    {
        const char* Name = nslWaveGetName((nslWaveID)this->mWave);
        tlWarning("A NAN was passed into the sound system while trying to adjust the min or max on %s\n", Name);
        this->Stop();
    }
    else
    {
        if (this->mSource == NSL_SOURCE_ID_INVALID)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 552;
            AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
                __debugbreak();
        }
        if (this->mSource != NSL_SOURCE_ID_INVALID)
        {
            nslSetSourceParam((nslSourceID)this->mSource, 25, min);
            nslSetSourceParam((nslSourceID)this->mSource, 26, max);
            this->mMinRange = min;
            this->mMaxRange = max;
        }
    }
}

// ea: 0x0062C470
void SoundDevice::Sound::SetPosition(const math::Position3& pos)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 566;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        float v5[3];
        v5[0] = -pos.v.m128_f32[1];
        v5[1] = pos.v.m128_f32[2];
        v5[2] = pos.v.m128_f32[0];
        if ((__fpclass(v5[0]) & 0x297) != 0 || (__fpclass(v5[1]) & 0x297) != 0
            || (__fpclass(v5[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 574;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "Invalid position detected while setting sound position- this is fatal on xbox"))
                __debugbreak();
            this->Stop();
        }
        else
        {
            nslSetSourcePosition((nslSourceID)this->mSource, v5);
            this->mDebugPos[0] = pos.v.m128_f32[0];
            this->mDebugPos[1] = pos.v.m128_f32[1];
            this->mDebugPos[2] = pos.v.m128_f32[2];
        }
    }
}

// ea: 0x0062C630
void SoundDevice::Sound::SetVelocity(const math::Dir3& vel)
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 590;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        float v5[3];
        v5[0] = vel.v.m128_f32[0];
        v5[1] = vel.v.m128_f32[1];
        v5[2] = vel.v.m128_f32[2];
        if ((__fpclass(v5[0]) & 0x297) != 0 || (__fpclass(v5[1]) & 0x297) != 0
            || (__fpclass(v5[2]) & 0x297) != 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::ARO;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
            AeAssert::gCurrentLine = 596;
            AeAssert::gCurrentExpr = nullptr;
            if (!AeAssert::IsIgnored()
                && AeAssert::Warning(
                    "Invalid position detected while setting sound velocity- this is fatal on xbox"))
                __debugbreak();
            this->Stop();
        }
        else
        {
            nslSetSourceVelocity((nslSourceID)this->mSource, v5);
        }
    }
}

// ea: 0x0062C7A0
void SoundDevice::Sound::Update()
{
    if (this->mSource == NSL_SOURCE_ID_INVALID)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.cpp";
        AeAssert::gCurrentLine = 614;
        AeAssert::gCurrentExpr = "mSource != NSL_SOURCE_ID_INVALID";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("invalid source"))
            __debugbreak();
    }
    if (this->mSource != NSL_SOURCE_ID_INVALID)
    {
        this->SetVolume(this->mVolume);
        this->SetPitch(this->mPitch);
        this->SetRange(this->mMinRange, this->mMaxRange);
        unsigned int v3 = this->mEntHandle.mVal & 0xFFF;
        Entity* mObject = nullptr;
        if (v3 < 0x540
            && this->mEntHandle.mVal >> 12
                == EntityHandleDb::sInst.mElements[v3].mKey)
            mObject = EntityHandleDb::sInst.mElements[v3].mObject;
        const math::Mat43* mPoPtr = (const math::Mat43*)this->mPoPtr;
        if (mPoPtr != nullptr)
        {
            if ((__fpclass(mPoPtr->w.v.m128_f32[0]) & 0x297) == 0
                && (__fpclass(mPoPtr->w.v.m128_f32[1]) & 0x297) == 0
                && (__fpclass(mPoPtr->w.v.m128_f32[2]) & 0x297) == 0)
            {
                this->SetPosition(mPoPtr->w);
                math::Dir3 zeroVel;
                zeroVel.v = _mm_setzero_ps();
                this->SetVelocity(zeroVel);
                return;
            }
            tlWarning("A NAN was passed into the sound system update an sound position\n");
            goto LABEL_22;
        }
        if (mObject != nullptr)
        {
            math::Position3 v10;
            v10.v = mObject->r.currentOrigin.v;
            if ((__fpclass(v10.v.m128_f32[0]) & 0x297) == 0
                && (__fpclass(v10.v.m128_f32[1]) & 0x297) == 0
                && (__fpclass(v10.v.m128_f32[2]) & 0x297) == 0)
            {
                this->SetPosition(v10);
                math::Dir3 zeroVel;
                zeroVel.v = _mm_setzero_ps();
                this->SetVelocity(zeroVel);
                return;
            }
            tlWarning("A NAN was passed into the sound system update an entity position\n");
        LABEL_22:
            this->Stop();
        }
    }
}

// ea: 0x0062C9B0
void SoundDevice::ReleaseSound(Sound* s)
{
    s->Stop();
}

// ea: 0x0062C9C0
void SoundDevice::ReleaseSound(
    DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound> s)
{
    unsigned int v2 = s.mHandle.mVal & 0xFFF;
    if (v2 < 0x200
        && s.mHandle.mVal >> 12
            == SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey
        && SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject != nullptr)
    {
        Sound* mObject = nullptr;
        if ((s.mHandle.mVal & 0xFFF) < 0x200
            && s.mHandle.mVal >> 12
                == SoundDevice::SoundHandleDb::sInst.mElements[v2].mKey)
            mObject = SoundDevice::SoundHandleDb::sInst.mElements[v2].mObject;
        mObject->Stop();
    }
}

// ea: 0x0062CA10
void SoundDevice::StopAllSoundsNotPaused()
{
    Sound* s = this->mSounds;
    for (int i = 512; i != 0; --i)
    {
        if (!s->mPaused)
            s->Stop();
        ++s;
    }
}

// ea: 0x006435D0
void SoundDevice::StopAllSounds()
{
    for (int i = 0; i < 16; ++i)
        this->mCrossFadeInfo[i].mRemainingTime = 0.0f;
    SoundDevice* v4 = this;
    for (int j = 512; j != 0; --j)
    {
        v4->mSounds[0].Stop();
        v4 = (SoundDevice*)((char*)v4 + 0x3C);
    }
    MusicMgr* v6 = MusicMgr::sInst;
    SoundDevice::Sound* mObject = SoundFromHandle(v6->mMusic);
    if (mObject != nullptr)
    {
        mObject->Stop();
        v6->mMusic.mVal = 0;
    }
    v6->mCrossFadeType = 2;
    v6->mIndoorFadeTime = 0.0f;
    v6->mDelayCount = 0.0f;
    this->FrameAdvance(0.0f);
}

// ea: 0x0062CA40
void SoundDevice::UpdateCrossFade(float deltaTime)
{
    for (int i = 16; i != 0; --i)
    {
        CrossFadeInfo* info = &this->mCrossFadeInfo[16 - i];
        if (info->mRemainingTime > 0.0f)
        {
            info->mRemainingTime -= deltaTime;
            if (info->mRemainingTime < 0.0f)
                info->mRemainingTime = 0.0f;
            unsigned int v4 = info->mSound1.mVal & 0xFFF;
            Sound* mObject = nullptr;
            float adjustVolume1 = info->mAdjustVolume1 * deltaTime;
            float adjustVolume2 = info->mAdjustVolume2 * deltaTime;
            if (v4 < 0x200
                && info->mSound1.mVal >> 12
                    == SoundDevice::SoundHandleDb::sInst.mElements[v4].mKey)
                mObject = SoundDevice::SoundHandleDb::sInst.mElements[v4].mObject;
            unsigned int v8 = info->mSound2.mVal & 0xFFF;
            Sound* v9 = nullptr;
            if (v8 < 0x200
                && info->mSound2.mVal >> 12
                    == SoundDevice::SoundHandleDb::sInst.mElements[v8].mKey)
                v9 = SoundDevice::SoundHandleDb::sInst.mElements[v8].mObject;
            if (info->mRemainingTime <= 0.0f)
            {
                if (mObject != nullptr)
                {
                    mObject->SetVolume(0.0f);
                    mObject->Stop();
                }
                if (v9 != nullptr)
                    v9->SetVolume(1.0f);
            }
            else
            {
                if (mObject != nullptr)
                {
                    float v10;
                    if (mObject->mSource == NSL_SOURCE_ID_INVALID)
                        v10 = -2.0f;
                    else
                        v10 = nslGetSourceParam((nslSourceID)mObject->mSource,
                                                0, -1.0f);
                    float newVolume = v10 + adjustVolume1;
                    if (newVolume < 0.0f)
                        newVolume = 0.0f;
                    mObject->SetVolume(newVolume);
                }
                if (v9 != nullptr)
                {
                    float v11;
                    if (v9->mSource == NSL_SOURCE_ID_INVALID)
                        v11 = -2.0f;
                    else
                        v11 = nslGetSourceParam((nslSourceID)v9->mSource, 0,
                                                -1.0f);
                    float v15 = v11 + adjustVolume2;
                    if (v15 > 1.0f)
                        v15 = 1.0f;
                    v9->SetVolume(v15);
                }
            }
        }
    }
}

// ea: 0x0062CBE0
void SoundDevice::CrossFade(unsigned int sound1, unsigned int sound2,
                            float crossFadeTime)
{
    int v4 = -1;
    int v6 = 0;
    CrossFadeInfo* info = this->mCrossFadeInfo;
    while (v4 == -1)
    {
        if (info[0].mRemainingTime == 0.0f)
        {
            v4 = v6;
            break;
        }
        if (info[1].mRemainingTime == 0.0f)
        {
            v4 = v6 + 1;
            break;
        }
        if (info[2].mRemainingTime == 0.0f)
        {
            v4 = v6 + 2;
            break;
        }
        if (info[3].mRemainingTime == 0.0f)
        {
            v4 = v6 + 3;
            break;
        }
        if (info[4].mRemainingTime == 0.0f)
        {
            v4 = v6 + 4;
            break;
        }
        if (info[5].mRemainingTime == 0.0f)
        {
            v4 = v6 + 5;
            break;
        }
        if (info[6].mRemainingTime == 0.0f)
        {
            v4 = v6 + 6;
            break;
        }
        if (info[7].mRemainingTime == 0.0f)
            v4 = v6 + 7;
        v6 += 8;
        info += 8;
        if (v6 >= 16)
            break;
    }
    unsigned int v9 = sound1 & 0xFFF;
    Sound* mObject = nullptr;
    if (v9 < 0x200
        && sound1 >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[v9].mKey)
        mObject = SoundDevice::SoundHandleDb::sInst.mElements[v9].mObject;
    unsigned int v11 = sound2 & 0xFFF;
    Sound* sound1a = nullptr;
    if (v11 < 0x200
        && sound2 >> 12 == SoundDevice::SoundHandleDb::sInst.mElements[v11].mKey)
        sound1a = SoundDevice::SoundHandleDb::sInst.mElements[v11].mObject;
    if (v4 > -1)
    {
        CrossFadeInfo* v12 = &this->mCrossFadeInfo[v4];
        v12->mSound1.mVal = sound1;
        v12->mSound2.mVal = sound2;
        v12->mAdjustVolume1 = 0.0f;
        v12->mAdjustVolume2 = 0.0f;
        if (mObject != nullptr)
            v12->mAdjustVolume1 =
                (-1.0f / crossFadeTime) * mObject->mVolume;
        if (sound1a != nullptr)
        {
            v12->mAdjustVolume2 = sound1a->mVolume / crossFadeTime;
            sound1a->SetVolume(0.0f);
        }
        v12->mRemainingTime = crossFadeTime;
    }
}

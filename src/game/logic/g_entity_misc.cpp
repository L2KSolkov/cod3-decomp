// ============================================================================
// g_entity_misc.cpp - game.o stat monitor + Entity DObj/enemy helpers
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#ifdef PlaySound
#undef PlaySound
#endif

#include "game/logic/g_local.h"
#include "game/logic/g_camerashake.h"
#include "core/mem_heap.h"
#include "core/tlFixedString.h"
#include "input/controller.h"
#define D3DDevice_SetIndices cod3_d3d8_SetIndices_decl
#define D3DDevice_SetPixelShaderProgram cod3_d3d8_SetPixelShaderProgram_decl
#define D3DDevice_SetVertexShader cod3_d3d8_SetVertexShader_decl
#define D3DDevice_SetVertexShaderInputDirect cod3_d3d8_SetVertexShaderInputDirect_decl
#include "d3d8.h"
#undef D3DDevice_SetIndices
#undef D3DDevice_SetPixelShaderProgram
#undef D3DDevice_SetVertexShader
#undef D3DDevice_SetVertexShaderInputDirect

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

namespace AeAssert {
enum ECoderId : int;
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
bool Error(const char* fmt, ...);
}

extern void tlFinalPrint(const char* text);
extern void tlPrintf(const char* fmt, ...);
extern void nglDebugAddBox(const math::Mat43& mat,
                           const math::DiagMat33& size,
                           unsigned int color);

// IDA global: g_bAnimCheck (scr.o)
int g_bAnimCheck = 0;

// IDA global: off_CFBB58 (scr.o)
static const char off_CFBB58[] = {'\x15', '%', 's', '\0'};

// ??1DObj@@QAE@XZ (render.o)
DObj::~DObj()
{
}
#include "core/PoolAllocator.h"
#include "core/color.h"
#include "core/tlFixedString.h"

#include <new>
#include <stdio.h>
#include <string.h>

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file,
                                 int line);

int g_uniqueEntityIndex;  // ?g_uniqueEntityIndex@@3HA (game.o @ 0xF4F444)

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
struct nglTexture;
extern nglTexture* GetTextureData(const char* name, int image_type,
                                  const char* fromPak);  // render.o 0x6B2000
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
void StatMon_Reset()
{
    memset(stats, 0, sizeof(stats));
    statCount = 0;
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

void Entity::CreateDObj(DObjModel* dobjModels, unsigned short numModels,
                        XAnimTree* tree, unsigned short gameId)
{
    if (this->mDObj == nullptr)
    {
        void* v6 = DObj::operator new(0xE8u);
        if (v6 != nullptr)
            this->mDObj = new (v6) DObj((TPakId)this->mPakId);
        else
            this->mDObj = nullptr;
        register_dobj(this->mHandle);
    }
    this->mDObj->mEntity = this;
    DObjCreate(dobjModels, numModels, tree, this->mDObj, gameId);
    if (this->client != nullptr && this->mBPInfo == nullptr)
    {
        biped_phys_info* bp = create_biped_phys_info(this);
        this->set_bp_info(bp);
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
    debug_brush();
    debug_brush(const cdlBrush& _brush, const math::Mat43& _mat,
                const Color& _color)
        : brush(&_brush), mat(_mat), color(_color)
    {
    }
};
static_assert(sizeof(debug_brush) == 0x60, "debug_brush size mismatch");

// game.o 0x0065C4D0
debug_brush::debug_brush()
{
}

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
class cdl_array {
public:
    int m_count;     // +0x00
    T*   m_elements; // +0x04
    // ?resize@?$cdl_array@UcdlPlane@@@@QAEXI@Z (cdl_xboxr; stub)
    void resize(unsigned int n) { (void)n; }
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
extern void* nglLockSectionVertices(nglMeshSection* Section);
extern nglMesh* auxCloseScratchMesh(nglMesh* m);  // ?auxCloseScratchMesh (ngl_aux.o)
struct nglMeshParams;
struct nglShaderParamSet;
class nglMeshNode;
extern nglMeshNode* nglListAddMesh(nglMesh* Mesh,
                                   const math::Mat43& LocalToWorld,
                                   nglMeshParams* MeshParams,
                                   nglShaderParamSet* ShaderParams,
                                   void (*fn)(nglMeshNode*));
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
                *(nglMaterial**)((char*)DebugRender_sInst + 0xC), 1);
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
            nglListAddMesh(m, identity, nullptr, npolies, nullptr);
        }
    }
    if (planes.m_elements != nullptr)
        tlMemFree(planes.m_elements);
}

// ============================================================================
// Entity helpers - ea: 0x611F00..0x639170
// ============================================================================
extern void AnglesToAxis(const math::Position3& angles,
                         const math::Position3& origin,
                         math::Mat43& mat);  // core.o (3-arg variant)
extern int g_DOBJF_NOT_RENDERED_LAST_FRAME;  // ?g_DOBJF_NOT_RENDERED_LAST_FRAME (core.o)

extern void XAnimClearTree(XAnimTree* tree);  // ?XAnimClearTree@@YAXPAVXAnimTree@@@Z

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
    Entity* mObject = (Entity*)EntityHandleDb::sInst.GetObject(handle);
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
                        XAnimClearTree((XAnimTree*)tree);
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
extern void PathNodeMgr_ConnectPathsForEntity(void* self, Entity* ent);  // mp_actors.o
extern void G_EntUnlinkFree(Entity* ent);           // g.o
extern void StopPhysics(Entity* e);                 // g.o
extern void g_UnlinkEntity(Entity* ent);            // g.o
extern void G_DelayFreeAnimTree(void* tree);        // g.o
extern void j_nullsub_57(actor_s* actor);           // g.o
extern void Sentient_Free(sentient_s* sentient);    // mp_actors.o
extern void G_FreeEntityRefs(Entity* ed);           // g.o
extern void j_nullsub_15(actor_s* self, Entity* other);          // g.o
extern sentient_s* __fastcall Sentient_FirstSentient(int iTeamFlags);  // mp_actors.o
extern sentient_s* __fastcall Sentient_NextSentient(sentient_s* prev, int iTeamFlags);  // mp_actors.o
extern void Sentient_DissociateEntity(sentient_s* self, Entity* other);  // mp_actors.o
extern void j_nullsub_77(Entity* ent);              // g.o
extern void G_FreeTurret(Entity* self);             // g.o
extern void G_FreeVehicle(Entity* ent);             // g.o
extern void BrocDestroyEntity(Entity* ent);         // broc
extern void EntityHandleDb_Release(void* self, Entity* e);  // game.o
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
        PathNodeMgr_ConnectPathsForEntity(PathNodeMgr::sInst, this);
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
        // Scene entities can be filtered before the game VM has run G_InitGame.
        // The reference traversal is unchanged once level.sentients is live;
        // before that point there are no sentients to dissociate.
        if (level.sentients != nullptr)
        {
            for (sentient_s* j = Sentient_FirstSentient(-1); j != nullptr;
                 j = Sentient_NextSentient(j, -1))
                Sentient_DissociateEntity(j, this);
        }
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
    EntityHandleDb_Release(&EntityHandleDb::sInst, this);
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
}

// ea: 0x00639250
void EnableAI(unsigned int handle)
{
    Entity* mObject = (Entity*)EntityHandleDb::sInst.GetObject(handle);
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
    AnglesToAxis(this->r.currentAngles, this->r.currentOrigin,
                 this->r.currentMat);
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
// ea: 0x006618B0 (game.o)
trRefEntity::trRefEntity(int foo)
{
    *(void**)((char*)this + 0x00) = nullptr;  // m_dlist_node.m_next
    *(void**)((char*)this + 0x04) = nullptr;  // m_dlist_node.m_prev
    memset((char*)this + 0x08, 0, 0x60);      // refEntity_t (96 bytes)
    *(float*)((char*)this + 0x68) = 3.4028235e38f;  // mLightGrid.lastPos[0]
    *(float*)((char*)this + 0x6C) = 3.4028235e38f;  // mLightGrid.lastPos[1]
    *(float*)((char*)this + 0x70) = 3.4028235e38f;  // mLightGrid.lastPos[2]
    *(unsigned char*)((char*)this + 0x74) = 1;      // mLightGrid.moved
    *(float*)((char*)this + 0xEC) = 1.0f;           // mScale
    *(float*)((char*)this + 0xF0) = -1.0f;          // mAlpha
    *(uint8_t*)((char*)this + 0xFB) &= 0xFD;        // clear noShadow bit
    this->mWaterHeightOffset = 0;
    this->iflIndex = (uint8_t)-1;
    this->mSnapshotId = 0;
    *(float*)((char*)this + 0xDC) = 0.0f;  // lightingOrigin[0]
    *(float*)((char*)this + 0xE0) = 0.0f;  // lightingOrigin[1]
    *(float*)((char*)this + 0xE4) = 0.0f;  // lightingOrigin[2]
    (void)foo;
}

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
// ea: 0x006619C0
Entity::AnimationDebug::AnimationDebug(int lastAnim, int prev2last,
                                        const char* animName)
    : lastAnimPlayed(reinterpret_cast<const char*>(
          static_cast<uintptr_t>(static_cast<unsigned int>(lastAnim)))),
      prev2lastAnimPlayed(reinterpret_cast<const char*>(
          static_cast<uintptr_t>(static_cast<unsigned int>(prev2last))))
{
    AeStringSupport::CStrToAeStr((char*)lastAnimNamed.mBuff, &lastAnim, 63,
                                 animName);
    lastAnimNamed.mLength = (unsigned char)lastAnim;
}

extern void* mem_heap_malloc(unsigned int size);  // mem_lib
extern void AeStringSupport::CStrToAeStr(char* oBuff, int* const oLen,
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
        AeStringSupport::CStrToAeStr(oBuff, &oLen, 63, v4);
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
            AeStringSupport::CStrToAeStr((char*)v6->lastAnimNamed.mBuff,
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
// ea: 0x006C0810 (render.o)
void trRefEntity::SetInSnapshot()
{
    this->mSnapshotId = level.snapTime;
}

// ea: 0x006C0820 (render.o)
bool trRefEntity::IsInSnapshot() const
{
    return this->mSnapshotId == level.snapTime;
}

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

// ea: 0x004A9080 (g.o inline COMDAT)
bool GamePause::IsGamePaused(int client)
{
    if (client != 0)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\gamepause.h";
        AeAssert::gCurrentLine = 12;
        AeAssert::gCurrentExpr = "client >= 0 && client < 1";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("GamePause: Invalid client"))
            __debugbreak();
    }
    return GamePause::mData.mGamePaused[client];
}

// ea: 0x6126A0
void GamePause::SetGamePaused(int client, bool paused)
{
    (void)client;
    GamePause::mData.mGamePaused[0] = paused;
}

// ============================================================================
// Entity::ExecScriptHandler - ea: 0x611F10
// ============================================================================
// ScriptEventHandler lives in g_game2_misc.cpp (game2.o port); params are
// ScriptEventParams* in the binary, void* in the tree's game2.o port.
class ScriptEventParams;
struct ScriptEventHandler {
    unsigned char m_dlist_node[8];      // +0x00
    unsigned char mEvents[0x38];        // +0x08 (ScriptEvent mEvents[7])
    ScriptEventHandler* mNext;          // +0x40
    bool ExecEvents(Entity* ent, HashString h, ScriptEventParams* params);  // game2.o 0x4F5A50
};

// ea: 0x00611F10
void Entity::ExecScriptHandler(HashString h, ScriptEventParams* params)
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
// ea: 0x006BE320 (render.o) - DObj::GetMat inline; skel at +0x70, mat stride 0x40
const math::Mat43* DObj_GetMat(void* obj, int boneIndex)  // ?DObj_GetMat@@YAPBVMat43@math@@PAXH@Z
{
    if (obj == nullptr)
        return nullptr;
    void* skel = *(void**)((char*)obj + 0x70);
    return (const math::Mat43*)((char*)skel + 0x40 * boneIndex);
}
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
ae_sized_array<DbLinkedHandle<EntityHandleDb, Entity>, 32>
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
        && VectorDistanceSquared2D(Player->r.currentOrigin,
                                   this->r.currentOrigin) <= 1000000.0f)
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
class WaitTilOutput;
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
    void AddNotify(EntityNotify* notify);  // game.o 0x00661990
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

// ea: 0x00661990
void EntityNotifySet::AddNotify(EntityNotify* notify)
{
    NotifyDList* strings = (NotifyDList*)((char*)this + 0x0C);
    NotifyNode* node = (NotifyNode*)&notify->m_dlist_node;
    node->m_next = (NotifyNode*)strings->m_end;
    node->m_prev = (NotifyNode*)strings->m_tail;
    ((NotifyNode*)strings->m_tail)->m_next = node;
    strings->m_tail = node;
    ++strings->m_size;
}

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

// ea: 0x005EE310
void Entity::Notify(const char* n)
{
    HashString h;
    h.mHash = HashString::CalcHash(n);
    Notify(h);
}

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
    node->m_next = (NotifyNode*)&pending->m_end;
    node->m_prev = (NotifyNode*)pending->m_tail;
    ((NotifyNode*)pending->m_tail)->m_next = node;
    pending->m_tail = node;
    ++pending->m_size;
    ScriptEventHandler* mScriptEventHandler = this->mScriptEventHandler;
    if (mScriptEventHandler != nullptr)
        mScriptEventHandler->ExecEvents(this, h, nullptr);
}

// Entity::Notify overloads (game.o; stubs, port later)
void Entity::Notify(HashString h, unsigned int* e)
{
    (void)h; (void)e;
}
void Entity::SetLerpOrigin(EntityState* s, const math::Position3* origin)
{
    (void)s; (void)origin;
}
void cFreeList_Shutdown(void* freelist)
{
    (void)freelist;
}

void ae_vector_push_back_uint(ae_vector<unsigned int>* self,
                              const unsigned int* elem)
{
    (void)self; (void)elem;
}
void ae_vector_push_back_funcptr(
    ae_vector<void (__cdecl*)(Broc::entity)>* self,
    void (__cdecl* const* elem)(Broc::entity))
{
    (void)self; (void)elem;
}
// ea: 0x006CE610
const math::Mat43::Packed& DObj::GetBaseRelMat(int boneIndex)
{
    int baseBone = 0;
    int modelIndex = 0;
    if (this->numModels != 0)
    {
        IVPointer<XModel>* models = this->models;
        while (true)
        {
            ValidatePakId((TPakId)models->mPakId);
            int lodIndex = 0;
            XModelLod** lod = models->mValue->lod;
            while (*lod == nullptr)
            {
                ++lod;
                ++lodIndex;
            }
            XModelParts* parts = models->mValue->lod[lodIndex]->xmodelParts;
            unsigned int boneCount = parts->mHierarchy.mSize;
            if (boneIndex - baseBone < (int)boneCount)
                return parts->mTransforms.mList[boneIndex - baseBone];
            baseBone += (int)boneCount;
            ++models;
            if (++modelIndex >= this->numModels)
                break;
        }
    }

    AeAssert::gCurrentAuthor = AeAssert::JRS;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
    AeAssert::gCurrentLine = 2054;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning("Bad bone index in GetBaseRelMat"))
        __debugbreak();

    ValidatePakId((TPakId)this->models[0].mPakId);
    XModelParts* parts = this->models[0].mValue->parts;
    if (parts->mTransforms.mSize == 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceVector.h";
        AeAssert::gCurrentLine = 81;
        AeAssert::gCurrentExpr = "index < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Bounds check"))
            __debugbreak();
    }
    return parts->mTransforms.mList[0];
}
void CGBankManager_DebugRender_impl(void* self)
{
    (void)self;
}

// DebugRender helpers (render.o; stubs, port later)
void DebugRender::RenderBox(const math::Position3& mins,
                            const math::Position3& maxs, const Color& color)
{
    (void)mins; (void)maxs; (void)color;
}
void DebugRender::RenderLine(const math::Position3& pt1,
                             const math::Position3& pt2, const Color& color,
                             float thickness)
{
    (void)pt1; (void)pt2; (void)color; (void)thickness;
}
void DebugRender::RenderCone(const math::Position3& pos,
                             const math::Dir3& dir, float angle,
                             float length, const Color& color)
{
    (void)pos; (void)dir; (void)angle; (void)length; (void)color;
}
void DebugRender::RenderQuad2D(float x, float y, float w, float h, float z,
                               const Color& color)
{
    (void)x; (void)y; (void)w; (void)h; (void)z; (void)color;
}
void DebugRender::RenderLineBox(const math::Mat43& LToW,
                                const math::DiagMat33& size,
                                const Color& color)
{
    Color32 color32 = color.to_color32();
    nglDebugAddBox(LToW, size, color32.i);
}
void DebugRender::RenderText(const char* text, int x, int y,
                             const Color& color, float scaleX, float scaleY)
{
    (void)text; (void)x; (void)y; (void)color; (void)scaleX; (void)scaleY;
}
void DebugRender::RenderText3D(const math::Position3& pos,
                               const Color& color, float scale,
                               const char* text, ...)
{
    (void)pos; (void)color; (void)scale; (void)text;
}
void DebugRender::RenderAxis(const math::Mat43& mat, float length,
                             float width)
{
    (void)mat; (void)length; (void)width;
}
void DebugRender_AddRenderer(void* self, void (*fp)())
{
    (void)self; (void)fp;
}
void DebugRender_AddRenderer(void* self, void* fp)
{
    (void)self; (void)fp;
}
void DebugRender_Init(void* self)
{
    (void)self;
}

// PathNodes helpers (mp_actors.o; stubs, port later)
namespace PathNodes {
const PathNode* NodeHandle::operator*() const
{
    return PathNodeMgr::sInst->GetNode(*this);
}
}
PathNodes::PathNode* HandleDbToNode(PathNodes::NodeHandle h)
{
    (void)h.mValue;
    return nullptr;
}
const PathNodes::PathNode* PathNodes_NodeHandle_deref(
    const PathNodes::NodeHandle* h)
{
    (void)h;
    return nullptr;
}
void AnimationPlayer_DebugDump(Entity* ent)
{
    (void)ent;
}
int curFrame_0 = 0;  // ?curFrame_0@@3HA (game.o @ 0xDF8DE0)
unsigned int curFrame_1 = 0;  // ?curFrame_1@@3IA (game.o @ 0xDF8DE4)

// g.o / shell.o / render.o / scr.o stubs (port later)
PlayerState& GetPlayerState(int idx)
{
    (void)idx;
    static PlayerState dummy = {};
    return dummy;
}
bool Entity_IsInRagdoll(Entity* ent)
{
    (void)ent;
    return false;
}
bool Entity_IsLocalPlayer(const Entity* ent)
{
    (void)ent;
    return false;
}
AnimTree* Scr_GetAnims(int index)
{
    (void)index;
    return nullptr;
}
int Scr_IsSystemActive(unsigned char sys)
{
    (void)sys;
    // ea: 0x005C1AB0
    return 1;
}
int RE_Text_Width(const char* text, int font, float scaleX, float scaleY,
                  int style)
{
    (void)text; (void)font; (void)scaleX; (void)scaleY; (void)style;
    return 0;
}
int DObjGetBoneIndex(const DObj* obj, unsigned int boneNameHash)
{
    (void)obj; (void)boneNameHash;
    return -1;
}
team_t Sentient_EnemyTeam(team_t eTeam)
{
    (void)eTeam;
    return (team_t)0;
}
extern int R_CellForPoint(const math::Position3& pos);

int R_CellForPoint(const math::Position3* pos)
{
    return R_CellForPoint(*pos);
}
int R_CellForPoint(const float* pos)
{
    return R_CellForPoint(reinterpret_cast<const math::Position3*>(pos));
}
float random()
{
    return 0.0f;
}
int controller_button_pressed(void* self, int i_controller_num, int i_button)
{
    controller* pad = self != nullptr ? static_cast<controller*>(self)
                                       : controller::inst();
    return pad->button_pressed(i_controller_num,
                               static_cast<controller::ButtonIndex>(i_button));
}
int SmokeGrenadeMgr_EntityCanSeeEntity(void* self, Entity* ent,
                                       Entity* targEnt, float visThreshold)
{
    (void)self; (void)ent; (void)targEnt; (void)visThreshold;
    return 0;
}

// LocalClient namespace (canonical implementations live in cl_localclient.cpp).
extern int LocalClient_PortToValidClient(int port);
extern void LocalClient_SetFirstLocalClientIndex(int index);
extern void LocalClient_SetLastLocalClientIndex(int index);
extern void LocalClient_InitializeClientControllers();
namespace LocalClient {
int FirstLocalClientIndex();
int ClientToPort(int client);
int PortToValidClient(int port)
{
    return LocalClient_PortToValidClient(port);
}
void SetFirstLocalClientIndex(int index)
{
    LocalClient_SetFirstLocalClientIndex(index);
}
void SetLastLocalClientIndex(int index)
{
    LocalClient_SetLastLocalClientIndex(index);
}
void InitializeClientControllers()
{
    LocalClient_InitializeClientControllers();
}
}

// ea: 0x4A6B70 (g.o inline COMDAT)
bool IsLocalPlayer(Entity* entity)
{
    return EntityManager::sInst->IsLocalPlayer(entity);
}

// XModelManager accessors
// XModelManager::GetXModel defined in tr_aeps2.cpp (render.o canonical).
// Physics bank accessors are defined with their concrete bank layouts in
// g_physics.cpp so their template symbols retain the release class tags.
void Destructible::Initialize(Destructible* self, Entity* ent, bool reInit)
{
    (void)self; (void)ent; (void)reInit;
}

// PakFile statics (streamer.o; stubs, port later)
void PakFile::GetHeapUsage(PakFile* self, int* used, int* size)
{
    (void)self; (void)used; (void)size;
}
const PakInfoNode* PakFile::GetInfo(PakFile* self)
{
    (void)self;
    return nullptr;
}


// DObj operator new/delete (render.o; stubs)
void* DObj::operator new(size_t s)
{
    return mem_heap_malloc((unsigned int)s);
}
void DObj::operator delete(void* p)
{
    mem_heap_free(p);
}

// DObj::GetBoneParent - ea: 0x006CE560
int DObj::GetBoneParent(int boneIndex)
{
    int baseBoneIndex = 0;
    int modelIndex = 0;
    if (this->numModels == 0)
        return -1;

    IVPointer<XModel>* model = this->models;
    XModelParts* xmodelParts = nullptr;
    for (;; ++model)
    {
        ValidatePakId((TPakId)model->mPakId);
        XModelLod** lod = model->mValue->lod;
        int lodIndex = 0;
        while (*lod == nullptr)
        {
            ++lodIndex;
            ++lod;
        }

        xmodelParts = model->mValue->lod[lodIndex]->xmodelParts;
        int boneCount = xmodelParts->mHierarchy.mSize;
        if (boneIndex - baseBoneIndex < boneCount)
            break;

        baseBoneIndex += boneCount;
        ++modelIndex;
        if (modelIndex >= this->numModels)
            return -1;
    }

    if (modelIndex > 0 && boneIndex == baseBoneIndex)
        return this->modelParents[modelIndex];

    unsigned int localBoneIndex = (unsigned int)(boneIndex - baseBoneIndex);
    if (localBoneIndex >= xmodelParts->mHierarchy.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\XModelParts.h";
        AeAssert::gCurrentLine = 217;
        AeAssert::gCurrentExpr = "i >= 0 && i < mHierarchy.size()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Bad Bone Index"))
            __debugbreak();
    }
    return xmodelParts->mHierarchy.mList[localBoneIndex].mParentIndex;
}

// XModel::GetNumBones (render.o; stub)
int XModel::GetNumBones(XModel* model, int lodIndex)
{
    (void)model; (void)lodIndex;
    return 0;
}

// Task::~Task (game2.o; stub)
Task::~Task() {}

// RumbleManager free artifacts (core.o surface; forwarding stubs)
class RumbleEffectInstanceHandle {
public:
    int mVal;  // +0x00
};
void RumbleManager_StopMotors(void* self) { (void)self; }
void RumbleManager_Reset(void* self) { (void)self; }
void RumbleManager_Remove(void* self, RumbleEffectInstanceHandle handle)
{
    (void)self; (void)handle;
}
void RumbleManager_SetIntensity(void* self, int handle, float intensity)
{
    (void)self; (void)handle; (void)intensity;
}
void RumbleManager_FrameAdvance(void* self, float delta)
{
    (void)self; (void)delta;
}
void RumbleManager_Play(void* self, void* effect, float intensity)
{
    (void)self; (void)effect; (void)intensity;
}
void* RumbleManager_Inst(int instance)
{
    (void)instance;
    return nullptr;
}

// GamePause free artifacts (forward to the statics)
void GamePause_SetGamePaused(int client, bool paused)
{
    GamePause::SetGamePaused(client, paused);
}
void GamePause_SetAllPaused(bool paused)
{
    GamePause::SetAllPaused(paused);
}

const char* CG_SafeTranslateString_Internal(const char* string,
                                            const char* defaultString)
{
    (void)string; (void)defaultString;
    return "";
}
void* XModelParts_GetAnimDef(void* self)
{
    (void)self;
    return nullptr;
}

// DObj / anim free artifacts (render.o/anim.o surface; stubs, port later)
struct DObjSkelMat;
class nalMatrix4x4 {
public:
    float x[4];
    float y[4];
    float z[4];
    float w[4];
};
struct nalGenericBoneHandle {
    unsigned int index;   // +0x00
    void* skeleton;       // +0x04
};
struct nalPositionOrientation {
    float m_data[8];  // opaque
};
class AnimTree;
DObjSkelMat* DObjGetMatrixArray(const DObj* obj, int modelIndex)
{
    if (obj->skel == nullptr)
        return nullptr;

    if (obj->matOffset[0] != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 811;
        AeAssert::gCurrentExpr = "obj->matOffset[0] == 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    DSkel* skel = static_cast<DSkel*>(obj->skel);
    return &skel->mat[obj->matOffset[modelIndex]];
}
math::Quaternion nalQuaternionFromMatrix(const math::Mat44& m)
{
    (void)m;
    math::Quaternion q = {};
    return q;
}
nalPositionOrientation nalGenericPose_GetModelPositionOrientation(
    void* pose, const nalGenericBoneHandle* handle)
{
    (void)pose; (void)handle;
    nalPositionOrientation r = {};
    return r;
}
void nalGenericSkeleton_GetBoneHandle(void* skeleton,
                                      nalGenericBoneHandle* handle,
                                      const tlFixedString* boneName)
{
    (void)skeleton; (void)handle; (void)boneName;
}
void* nalGenericAnim_CreateInstance(void* anim, void* skeleton)
{
    (void)anim; (void)skeleton;
    return nullptr;
}
void* DObj_GetTree(void* obj)
{
    (void)obj;
    return nullptr;
}
void* DObj_New(unsigned int size)
{
    return mem_heap_malloc(size);
}
void DObj_Ctor(void* obj, int pakId)
{
    (void)obj; (void)pakId;
}
void DObj_Dtor(void* obj)
{
    (void)obj;
}
void DObj_OpDelete(void* obj)
{
    (void)obj;
}
void* Entity_GetViewModelDObj(Entity* ent)
{
    (void)ent;
    return nullptr;
}
void* g_femanager_IGMS_cur()
{
    return nullptr;
}
void* Hunk_AllocXAnimCreate(int size)
{
    (void)size;
    return nullptr;
}
void* Hunk_AllocXAnimCreate(void* a, unsigned int b)
{
    (void)a; (void)b;
    return nullptr;
}
void* MetaNalBaseAnim_Ctor(void* self)
{
    (void)self;
    return nullptr;
}
void MetaNalBaseAnim_Create(void* self, void* anim)
{
    (void)self; (void)anim;
}
void MetaNalBaseAnim_DelayCreate(void* self, void** anims, int count)
{
    (void)self; (void)anims; (void)count;
}
void* RE_RegisterModel(void* result, const char* name, int pakId, int imagetype)
{
    (void)result; (void)name; (void)pakId; (void)imagetype;
    return nullptr;
}
XAnimTree* XAnimCreateTree(Entity* ent, AnimTree* anims)
{
    (void)ent; (void)anims;
    return nullptr;
}
void* XAnimCreateTree(void* ent, void* anims)
{
    (void)ent; (void)anims;
    return nullptr;
}
// ea: 0x004F6220
void Axis4_to_nalMatrix4x4(const float (*axis)[3], nalMatrix4x4* mat)
{
    float* dstW = &mat->x[3];
    float* dstZ = &mat->x[2];
    const float* src = &axis[0][2];
    for (int i = 4; i != 0; --i)
    {
        dstZ[-2] = src[-2];
        dstZ[-1] = src[-1];
        *dstZ = *src;
        *dstW = 0.0f;
        src += 3;
        dstZ += 4;
        dstW += 4;
    }
    mat->w[3] = 1.0f;
}
void BrocString_ctor(void* self, const char* s)
{
    (void)self; (void)s;
}
void BrocString_dtor(void* self)
{
    (void)self;
}
void CL_CubemapShotUsage() {}
void* cdGetAnim(unsigned int a)
{
    (void)a;
    return nullptr;
}
void SmokeGrenadeMgr_AddSmokeGrenade(void* mgr, void* info)
{
    (void)mgr; (void)info;
}
void SmokeGrenadeMgr_ReInitialize() {}
void SmokeGrenadeMgr_Update(void* self, float deltaT)
{
    (void)self; (void)deltaT;
}
struct TaskHandler;

// ============================================================================
// Free-function stubs (various libs; port later)
// ============================================================================
struct KeyInfoEntry;
KeyInfoEntry (*gKeyInfoMKeys)[256] = nullptr;  // ?gKeyInfoMKeys@@3PAY0BAA@UKeyInfoEntry@@A
struct KeyInfoEntry3;
KeyInfoEntry3 (*KeyInfo_mKeys)[256] = nullptr;  // ?KeyInfo_mKeys@@3PAY0BAA@UKeyInfoEntry3@@A
struct PakInfoNode;
extern struct PakInfoNode const* sLoadingScreenInfo;
void (__cdecl* gpBrocAPI_mCallbackQuitGame)() = nullptr;
void (__cdecl* rd_flush)(char*) = nullptr;
// Entity function tables (g.o; declared extern in g_local.h).  The reference
// image provides the spawn dispatch table as initialized data at 0xDD7338;
// leaving this table zeroed makes G_CallEntitySpawnFunction call address 0.
extern void SP_sound_blend(Entity*);
extern void SP_script_brushmodel(Entity*);
extern void SP_script_model(Entity*);
extern void SP_script_origin(Entity*);
extern void SP_script_prop_collmap(Entity*);
extern void SP_script_vehicle(Entity*);
extern void SP_script_vehicle_collmap(Entity*);
extern void SP_misc_model(Entity*);
extern void SP_info_player_start(Entity*);
extern void SP_info_null(Entity*);
extern void SP_info_notnull(Entity*);
extern void SP_info_notnull_big(Entity*);
extern void SP_info_grenade_hint(Entity*);
extern void SP_func_door(Entity*);
extern void SP_func_static(Entity*);
extern void SP_func_rotating(Entity*);
extern void SP_func_bobbing(Entity*);
extern void SP_func_pendulum(Entity*);
extern void SP_func_door_rotating(Entity*);
extern void trigger_use(Entity*);
extern void SP_trigger_multiple(Entity*);
extern void SP_trigger_friendlychain(Entity*);
extern void SP_trigger_hurt(Entity*);
extern void SP_trigger_once(Entity*);
extern void SP_trigger_damage(Entity*);
extern void SP_trigger_lookat(Entity*);
extern void SP_trigger_mount(Entity*);
extern void SP_light(Entity*);
extern void SP_turret(Entity*);
extern void SP_skyportal(Entity*);
extern void SP_corona(Entity*);
extern void SP_intermission(Entity*);
extern void SP_deathmatch(Entity*);
extern void SP_teamdeathmatch(Entity*);
extern void SP_ctf_allies_primary(Entity*);
extern void SP_ctf_allies_secondary(Entity*);
extern void SP_ctf_axis_primary(Entity*);
extern void SP_ctf_axis_secondary(Entity*);
extern void SP_single_ctf_allies(Entity*);
extern void SP_single_ctf_axis(Entity*);
extern void SP_hq_allies_primary(Entity*);
extern void SP_hq_allies_secondary(Entity*);
extern void SP_hq_axis_primary(Entity*);
extern void SP_hq_axis_secondary(Entity*);
extern void HQ_Point(Entity*);
extern void SP_dom_allies(Entity*);
extern void SP_dom_axis(Entity*);
extern void SP_war_allies(Entity*);
extern void SP_war_axis(Entity*);
extern void SP_sd_allies(Entity*);
extern void SP_sd_axis(Entity*);
extern void Actor_CorpseThink(Entity*, int);
extern void Actor_Think(Entity*, int);
extern void BodySink(Entity*, int);
extern void G_FinishSetupSpawnPoint(Entity*, int);
extern void FinishSpawningItem(Entity*, int);
extern void finishSpawningKeyedMover(Entity*, int);
extern void G_DelayMissile(Entity*, int);
extern void G_LaunchMissile(Entity*, int);
extern void G_IncomingMissile(Entity*, int);
extern void GotoPos3(Entity*, int);
extern void hurt_think(Entity*, int);
extern void misc_spawner_think(Entity*, int);
extern void multi_wait(Entity*, int);
extern void ReturnToPos1(Entity*, int);
extern void ReturnToPos1Rotate(Entity*, int);
extern void ReturnToPos2(Entity*, int);
extern void Think_MatchTeam(Entity*, int);
extern void Think_SpawnNewDoorTrigger(Entity*, int);
extern void Think_SpawnNewAutoDoorTrigger(Entity*, int);
extern void Think_GeneralLink(Entity*, int);
extern void Think_EnableMine(Entity*, int);

void (*gSpawnFuncs[53])(Entity* ent) = {
    SP_sound_blend,
    SP_script_brushmodel,
    SP_script_model,
    SP_script_origin,
    SP_script_prop_collmap,
    SP_script_vehicle,
    SP_script_vehicle_collmap,
    SP_misc_model,
    SP_info_player_start,
    SP_info_null,
    SP_info_notnull,
    SP_info_notnull_big,
    SP_info_grenade_hint,
    SP_func_door,
    SP_func_static,
    SP_func_rotating,
    SP_func_bobbing,
    SP_func_pendulum,
    // The reference table uses the no-op info handler for func_group.
    SP_info_null,
    SP_func_door_rotating,
    trigger_use,
    SP_trigger_multiple,
    SP_trigger_friendlychain,
    SP_trigger_hurt,
    SP_trigger_once,
    SP_trigger_damage,
    SP_trigger_lookat,
    SP_trigger_mount,
    SP_light,
    SP_turret,
    SP_turret,
    SP_skyportal,
    SP_corona,
    SP_intermission,
    SP_deathmatch,
    SP_teamdeathmatch,
    SP_ctf_allies_primary,
    SP_ctf_allies_secondary,
    SP_ctf_axis_primary,
    SP_ctf_axis_secondary,
    SP_single_ctf_allies,
    SP_single_ctf_axis,
    SP_hq_allies_primary,
    SP_hq_allies_secondary,
    SP_hq_axis_primary,
    SP_hq_axis_secondary,
    HQ_Point,
    SP_dom_allies,
    SP_dom_axis,
    SP_war_allies,
    SP_war_axis,
    SP_sd_allies,
    SP_sd_axis,
};
void (*thinktable[64])(Entity* ent, int msec) = {
    nullptr,
    Actor_CorpseThink,
    Actor_Think,
    BodySink,
    Concussive_think,
    G_FinishSetupSpawnPoint,
    FinishSpawningItem,
    finishSpawningKeyedMover,
    G_ExplodeMissile,
    G_DelayMissile,
    G_LaunchMissile,
    G_IncomingMissile,
    G_FreeEntity,
    GotoPos3,
    hurt_think,
    turret_think,
    turret_think_init,
    misc_spawner_think,
    multi_wait,
    RespawnItem,
    ReturnToPos1,
    ReturnToPos1Rotate,
    ReturnToPos2,
    Scr_Vehicle_Init,
    Scr_Vehicle_Think,
    Think_MatchTeam,
    Think_SpawnNewDoorTrigger,
    Think_SpawnNewAutoDoorTrigger,
    Think_GeneralLink,
    Think_EnableMine,
};
void (*entinfotable[3])(Entity* ent);
void (*touchtable[0xD])(Entity* ent, Entity* other, int bTouched);
void (*usetable[0xE])(Entity* ent, Entity* other, Entity* activator);
void (*paintable[6])(Entity* ent, Entity* other, int damage,
                     const float* point, int mod, const float* dir,
                     hitLocation_t hitLoc);
void (*dietable[8])(Entity* self, Entity* inflictor, Entity* attacker,
                    int damage, int mod, int weapon, const float* point,
                    const float* dir, hitLocation_t hitLoc);
void (*controllertable[4])(Entity* ent, int* partBits);

void Client::Clear(bool clearPersistentAlso, bool clearWeapons)
{
    this->ps.Clear(clearWeapons);
    if (clearPersistentAlso)
        this->pers.Clear();

    this->oldOrigin = math::Position3_Zero();
    this->noclip = 0;
    this->ufo = 0;
    this->bFrozen = 0;
    this->lastCmdTime = 0;
    this->buttons = 0;
    this->oldbuttons = 0;
    this->latched_buttons = 0;
    this->fGunPitch = 0.0f;
    this->fGunYaw = 0.0f;
    this->fGunXOfs = 0.0f;
    this->fGunYOfs = 0.0f;
    this->fGunZOfs = 0.0f;
    this->damage_blood = 0;
    this->damage_from[0] = 0.0f;
    this->damage_from[1] = 0.0f;
    this->damage_from[2] = 0.0f;
    this->damage_fromWorld = 0;
    this->respawnTime = 0;
    this->currentAimSpreadScale = 0.0f;
    this->pHitHitEnt = nullptr;
    this->pLookatEnt = nullptr;
    this->iLastFriendlyUseTime = 0;
    this->fLastTraceDist = 0.0f;
    this->hLastCompassFriendlyInfoEnt.mHandle.mVal = 0;
    this->hLastCompassTankInfoEnt.mHandle.mVal = 0;
    this->prevLinkAngles[0] = 0.0f;
    this->prevLinkAngles[1] = 0.0f;
    this->prevLinkAngles[2] = 0.0f;
    this->linkAnglesFrac[0] = 0.0f;
    this->linkAnglesFrac[1] = 0.0f;
    this->linkAnglesFrac[2] = 0.0f;
    this->inControlTime = 0;
    this->lastTouchTime = 0;
    this->mUseHoldEntity.mHandle.mVal = 0;
    this->mProneBlockedTime = -1;
    this->mMedicNobodyToReviveTime = -1;
    this->mTankExitBlockedByMantleTime = -1;
    this->mVehicleAnimStageChangeTime = -1;
    this->mNoDrawTime = -1;
    this->mUseHoldTime = 0;
    this->bDisableAutoPickup = 0;
    this->pain_debounce_time = 0;
    this->mInvalidatedNodeNum = 0;
    this->iLookatEntLastTime = 0;
    this->mVehicleAnimRoute = 0;
    this->mVehicleAnimMoving = false;
    this->mVehicleAnimPauseRemoteAngles = false;
    this->mVehicleAnimAngleOffset[0] = 0.0f;
    this->mVehicleAnimAngleOffset[1] = 0.0f;
    this->mVehicleAnimAngleOffset[2] = 0.0f;
    this->mLeftFootLift = 0.0f;
    this->mVehicleNoWeaponTime = 0;
    this->mLadderData.lastLadderTime = 0;
    this->mVehicleAnimFirstPersonCam = false;

    for (int i = 0; i < 15; ++i)
    {
        this->AnimIKFireEvents[i].fireTime = 0;
        this->AnimIKFireEvents[i].fireWeapon = 0;
        this->AnimIKPainEvents[i].time = 0;
    }
}
void Client_Clear(Client* c, bool clearPersistentAlso, bool clearWeapons)
{
    if (c != nullptr)
        c->Clear(clearPersistentAlso, clearWeapons);
}
void Client_Clear(void* c, bool clearPersistentAlso, bool clearWeapons)
{
    Client_Clear(static_cast<Client*>(c), clearPersistentAlso, clearWeapons);
}

struct Task;
void TaskSys::PostTask(Task* t)
{
    if (t == nullptr)
        return;

    struct TaskHandlerPostView {
        unsigned char m_dlist_node[8];
        unsigned int mTaskId;
        unsigned int mFlags;
    };

    for (int i = 0; i < this->mTaskHandlersSize; ++i)
    {
        TaskHandlerPostView* handler =
            static_cast<TaskHandlerPostView*>(this->mTaskHandlers[i]);
        if (handler != nullptr && handler->mTaskId == t->mTaskId)
        {
            if ((handler->mFlags & 8u) != 0)
                t->mFlags |= 4u;
            break;
        }
    }

    struct TaskDListNode {
        TaskDListNode* m_next;
        TaskDListNode* m_prev;
    };
    TaskDListNode* node =
        reinterpret_cast<TaskDListNode*>(t->_dlist);
    TaskDListNode* tail =
        static_cast<TaskDListNode*>(this->mPostQueue.m_tail);
    node->m_next = static_cast<TaskDListNode*>(this->mPostQueue.m_end);
    node->m_prev = tail;
    tail->m_next = node;
    this->mPostQueue.m_tail = node;
    ++this->mPostQueue.m_size;
}
void TaskSys_PostTask_glue(Task* t) { (void)t; }
void TaskSys_DeliverTasks_glue() {}

void rigid_body::add_force(const math::Dir3& f) { (void)f; }
struct rigid_body_constraint_ragdoll {
    void set_joint_limit_active(unsigned int a, bool b);
};
void rigid_body_constraint_ragdoll::set_joint_limit_active(unsigned int a,
                                                           bool b)
{
    (void)a; (void)b;
}
struct rigid_body_constraint_contact;
struct rigid_body_constraint;
struct outer_time;
namespace rbcint {
const outer_time* get_time_scale(rigid_body_constraint* c);
}
const outer_time* rbcint::get_time_scale(rigid_body_constraint* c)
{
    (void)c;
    return nullptr;
}
void verify_is_in_physics_system(rigid_body_constraint_contact* a,
                                 class rigid_body* b, class rigid_body* c)
{
    (void)a; (void)b; (void)c;
}

CameraShakeInstance* CameraShake_StartCameraShake(CameraShake* self, int a,
                                                  math::Position3* b, float c,
                                                  float d, float e)
{
    (void)self; (void)a; (void)b; (void)c; (void)d; (void)e;
    return nullptr;
}
void CameraShake_StopCameraShake(CameraShake* self, CameraShakeInstance* inst)
{
    (void)self; (void)inst;
}
void CameraShake_StopCameraShake(void* self, void* inst)
{
    (void)self; (void)inst;
}
void CameraShakeInstance_OverrideSettings(CameraShakeInstance* self, float a,
                                          float b)
{
    (void)self; (void)a; (void)b;
}
void CameraShakeInstance_SetTime(CameraShakeInstance* self, float a)
{
    (void)self; (void)a;
}

class DbTable;
DbTable* DbTableSet_GetTable(void* self, const char* name)
{
    (void)self; (void)name;
    return nullptr;
}
struct InplaceString;
InplaceString* InplaceTree_FindStr(const void* tree, const char* const* key)
{
    if (tree == nullptr || key == nullptr || *key == nullptr)
        return nullptr;

    struct StringTreeElement {
        const char* mKey;
        const char* mValue;
    };
    struct StringTree {
        unsigned int mSize;
        StringTreeElement* mArray;
    };

    const StringTree* stringTree = reinterpret_cast<const StringTree*>(tree);
    if (stringTree->mSize == 0 || stringTree->mArray == nullptr)
        return nullptr;

    unsigned int index = 0;
    for (;;) {
        if (index >= stringTree->mSize)
            return nullptr;

        const StringTreeElement& element = stringTree->mArray[index];
        if (element.mKey == nullptr && element.mValue == nullptr)
            return nullptr;

        if (_stricmp(element.mKey, *key) == 0)
            return reinterpret_cast<InplaceString*>(
                &stringTree->mArray[index].mValue);

        const int comparison = _stricmp(element.mKey, *key);
        index = comparison >= 0 ? (2 * index + 1) : (2 * index + 2);
        if (index >= stringTree->mSize)
            return nullptr;
    }
}
class DbRow;
const InplaceString* DbRow_GetFieldValuePtrString(const DbRow* row, int col)
{
    (void)row; (void)col;
    return nullptr;
}

struct SplineGroup;
SplineGroup* SplineGroup_GetPath(void* self)
{
    (void)self;
    return nullptr;
}

struct TaskHandlerImpl;
Task* HandleDb_GetTask(void* self, Handle h)
{
    (void)self; (void)h;
    return nullptr;
}
Task* TaskHandler_GetTaskForEntity(TaskHandlerImpl* self,
                                   DbLinkedHandle<EntityHandleDb, Entity> h)
{
    (void)self; (void)h;
    return nullptr;
}
TaskHandlerImpl* TaskSys_LookupHandler(unsigned int id)
{
    for (int i = 0; i < TaskSys::sInst.mTaskHandlersSize; ++i)
    {
        TaskHandler* handler = reinterpret_cast<TaskHandler*>(
            TaskSys::sInst.mTaskHandlers[i]);
        if (handler != nullptr && handler->mTaskId.mVal == id)
            return reinterpret_cast<TaskHandlerImpl*>(handler);
    }
    return nullptr;
}
void HandleDb_AllocateTaskHandle(void* self, Task** t) { (void)self; (void)t; }
void HandleDb_BindTaskObject(void* self, Handle h, Task* t)
{
    (void)self; (void)h; (void)t;
}
void HandleDb_ReleaseTaskHandle(void* self, Handle h) { (void)self; (void)h; }

unsigned char* SceneBank_PersistentStorage(void* self, unsigned int a)
{
    (void)self; (void)a;
    return nullptr;
}

struct bdRandom;
unsigned int bdRandom_nextUInt(void* self)
{
    (void)self;
    return 0;
}
void bdRandom_setSeed(void* self, unsigned int seed) { (void)self; (void)seed; }

namespace BrocHelper {
unsigned int (*GetBroFuncByName(const char* name, bool a))(void*)
{
    (void)name; (void)a;
    return nullptr;
}
}
unsigned int BrocAPI_GetEnt(void* a, void* b, unsigned int c, void* d, int e,
                            int f)
{
    (void)a; (void)b; (void)c; (void)d; (void)e; (void)f;
    return 0;
}
unsigned int BrocSys_GetEnt(const Broc::string& a, int b, unsigned int* c,
                            int d, int e)
{
    (void)a; (void)b; (void)c; (void)d; (void)e;
    return 0;
}
unsigned int InplaceTree_Find(void* tree, const unsigned int* key)
{
    (void)tree; (void)key;
    return 0;
}

void* AddLight(TPakId pakId, int type, math::Position3* pos, int time)
{
    (void)pakId; (void)type; (void)pos; (void)time;
    return nullptr;
}
void RemoveLight(void* light) { (void)light; }
void* FX_PlayEffect(TPakId pakId, int id, math::Mat43* mat, void* boltObjHandle,
                    unsigned int boltEntHandle, int boltBoneIndex,
                    bool boltAttchedToEnt)
{
    (void)pakId; (void)id; (void)mat; (void)boltObjHandle;
    (void)boltEntHandle; (void)boltBoneIndex; (void)boltAttchedToEnt;
    return nullptr;
}
void* FX_PlayEffectID(TPakId pakId, int id, math::Position3* org,
                      const float* fwd)
{
    (void)pakId; (void)id; (void)org; (void)fwd;
    return nullptr;
}
void* FX_PlayEntityEffectID(TPakId pakId, int id, math::Position3* org,
                            void* axis, void* boltObjHandle,
                            unsigned int boltEntHandle, int boltBoneIndex,
                            bool boltAttchedToEnt)
{
    (void)pakId; (void)id; (void)org; (void)axis; (void)boltObjHandle;
    (void)boltEntHandle; (void)boltBoneIndex; (void)boltAttchedToEnt;
    return nullptr;
}
void* FX_PlaySimpleEffectID(TPakId pakId, int id, math::Position3* org)
{
    (void)pakId; (void)id; (void)org;
    return nullptr;
}
void AssetBankSet_Dtor(void* self) { (void)self; }
void* InplaceAssetBankSet_ConfigStringBank_ctor(void* self)
{
    // IDA 0x4E44C0: initialize the fixed 99-entry bank array.
    void** elements = reinterpret_cast<void**>(
        reinterpret_cast<unsigned char*>(self) + 4);
    for (unsigned int i = 0; i < 99; ++i)
        elements[i] = nullptr;
    return self;
}
void* InplaceAssetBankSet_GdbFileBank_ctor(void* self)
{
    // IDA 0x660870: construct the AssetBankSet base, then clear 99 slots.
    extern void AssetBankSet_ctor(void*);
    AssetBankSet_ctor(self);
    void** elements = reinterpret_cast<void**>(
        reinterpret_cast<unsigned char*>(self) + 4);
    for (unsigned int i = 0; i < 99; ++i)
        elements[i] = nullptr;
    return self;
}
void* InplaceAssetBankSet_StringTableBank_ctor(void* self)
{
    // IDA 0x4E40C0: initialize the fixed 99-entry bank array.
    void** elements = reinterpret_cast<void**>(
        reinterpret_cast<unsigned char*>(self) + 4);
    for (unsigned int i = 0; i < 99; ++i)
        elements[i] = nullptr;
    return self;
}
void* mem_heap_malloc_sz(unsigned int size)
{
    return mem_heap_malloc(size);
}
void* MPLiveEngine_GetHandle() { return nullptr; }
void* ShaderCommon_StartShotPerfTest() { return nullptr; }
void* COD3_mem_alloc(unsigned int a, unsigned int b)
{
    return mem_heap_malloc(static_cast<int>(b), a);
}
void COD3_mem_free(void* p) { mem_heap_free(p); }
void AdvanceSceneAnims(float a) { (void)a; }
void AnimNoteHandler_Advance(void* self, float a) { (void)self; (void)a; }
void AnimNoteHandler_ParseNoteTracks(void* self, void* a)
{
    (void)self; (void)a;
}
void AnimQueue_ClearMatrixQueue() {}
void AnimQueue_ExecuteMatrixQueue() {}
void ApplyPhysics(Entity* e, const math::Position3* a, const math::Dir3* b,
                  float c, bool d, EHitLocation e2)
{
    (void)e; (void)a; (void)b; (void)c; (void)d; (void)e2;
}
void Axis_Bind_f() {}
void Axis_Unbindall_f() {}
void BrocAddEntityThread(Entity* e, unsigned int a, void* b)
{
    (void)e; (void)a; (void)b;
}
void BrocDestroyEntity(Entity* e) { (void)e; }
void BrocSys_ShellShock(unsigned int a, const Broc::string& b, float c)
{
    (void)a; (void)b; (void)c;
}
void ButtonMgr_UpdateBinding(int a, int b) { (void)a; (void)b; }
void CalculatePhysData(Entity* ent, IVPointer<PhysData> physData)
{
    (void)ent; (void)physData;
}
void Camera_StartAnimating(void* self, float a) { (void)self; (void)a; }
void Camera_StopAnimating(void* self, float a) { (void)self; (void)a; }
struct CGBank;
void CGBank_load_inplace(CGBank* bank, char* data, int* size)
{
    (void)bank; (void)data; (void)size;
}
static void DCGAlignInplace(int* offs, int alignment)
{
    while ((*offs & (alignment - 1)) != 0)
        ++*offs;
}
static void DCGSet_load_inplace(unsigned char* set, char* base, int* offs)
{
    DCGAlignInplace(offs, 16);
    *reinterpret_cast<char**>(set + 8) = base + *offs;
    *offs += 36 * *reinterpret_cast<unsigned int*>(set + 4);

    DCGAlignInplace(offs, 16);
    *reinterpret_cast<char**>(set + 16) = base + *offs;
    *offs += 4 * *reinterpret_cast<unsigned int*>(set + 12);

    DCGAlignInplace(offs, 16);
    *reinterpret_cast<char**>(set + 24) = base + *offs;
    *offs += 10 * *reinterpret_cast<unsigned int*>(set + 20);

    DCGAlignInplace(offs, 16);
    *reinterpret_cast<char**>(set + 32) = base + *offs;
    *offs += 16 * *reinterpret_cast<unsigned int*>(set + 28);

    DCGAlignInplace(offs, 16);
    *reinterpret_cast<char**>(set + 40) = base + *offs;
    *offs += 4 * *reinterpret_cast<unsigned int*>(set + 36);
}
void DCGBank_load_inplace(void* bank, char* data, int* size)
{
    unsigned char* raw = static_cast<unsigned char*>(bank);
    DCGAlignInplace(size, 16);
    *reinterpret_cast<char**>(raw + 4) = data + *size;
    unsigned char* elements =
        *reinterpret_cast<unsigned char**>(raw + 4);
    unsigned int count = *reinterpret_cast<unsigned int*>(raw);
    *size += 112 * count;
    for (unsigned int i = 0; i < count; ++i)
        DCGSet_load_inplace(elements + i * 112, data, size);
    DCGAlignInplace(size, 4);
}
void CGBankManager_UnloadAll(void* self) { (void)self; }
void CGBankManager_UnloadAll() {}
void Client_ClaimNode(Entity* e) { (void)e; }
void ClientImpacts(Entity* e, struct pmove_t* pm) { (void)e; (void)pm; }
void Com_CleanupSkeletons() {}
void Com_Crash_f() {}
void Com_Error_f() {}
void Com_Freeze_f() {}
void CompleteCommand() {}
struct messagewindow_t;
enum msgwnd_mode_t;
void Con_DrawMessageWindow(messagewindow_t* w, int a, int b, float c,
                           msgwnd_mode_t m)
{
    (void)w; (void)a; (void)b; (void)c; (void)m;
}
void Con_UpdateMessageWindowLine(messagewindow_t* w, int a, int b, int c)
{
    (void)w; (void)a; (void)b; (void)c;
}
void controller_rumble(void* self, int a, int b, float c)
{
    controller* pad = self != nullptr ? static_cast<controller*>(self)
                                       : controller::inst();
    pad->rumble(a, static_cast<controller::RumbleIndex>(b), c);
}
void controller_stick_value(void* self, int a, int b, int* c, int* d)
{
    controller* pad = self != nullptr ? static_cast<controller*>(self)
                                       : controller::inst();
    pad->stick_value(a, static_cast<controller::StickIndex>(b), c, d);
}
void controller_stop_all_rumble(void* self)
{
    controller* pad = self != nullptr ? static_cast<controller*>(self)
                                       : controller::inst();
    pad->stop_all_rumble();
}
void CurveManager_PostEvent(void* self, unsigned int a, unsigned int b, float c)
{
    (void)self; (void)a; (void)b; (void)c;
}
void CurveManager_Update(void* self, float a) { (void)self; (void)a; }
void D3DDevice_SetIndices(void* a, int b) { (void)a; (void)b; }
void D3DDevice_SetPixelShaderProgram(void* a) { (void)a; }
void D3DDevice_SetVertexShader(unsigned int a) { (void)a; }
void D3DDevice_SetVertexShaderInputDirect(void* a, int b, void* c)
{
    (void)a; (void)b; (void)c;
}
void Destructible_CheckpointExplode(Destructible* d) { (void)d; }
void DialogMenuSystem_BringUp(void* self, const char* a, bool b, bool c,
                              const char* d, bool e)
{
    (void)self; (void)a; (void)b; (void)c; (void)d; (void)e;
}
void DialogMenuSystem_CloseDialog(void* self) { (void)self; }
void DObjCalcAnim(DObj* obj, int a) { (void)obj; (void)a; }
void DObjCalcAnim(void* obj, int a) { (void)obj; (void)a; }
void DObjCreate(DObjModel* models, int numModels, void* tree, void* out,
                int gameId)
{
    DObjCreate(models, (unsigned short)numModels, (XAnimTree*)tree,
               (DObj*)out, (unsigned short)gameId);
}
void DObjCreate(DObjModel* models, unsigned short numModels, XAnimTree* tree,
                DObj* out, unsigned short gameId)
{
    if (models == nullptr || out == nullptr || numModels == 0)
        return;

    out->gameId = gameId;
    out->tree[0] = tree;
    out->skel = nullptr;
    out->duplicateParts = 0;
    out->ignoreCollision = 0;

    unsigned int newNumModels = 0;
    unsigned int boneIndex = 0;
    for (unsigned int i = 0; i < numModels && i < 8; ++i)
    {
        DObjModel& src = models[i];
        XModel* model = (XModel*)src.model.mValue;
        if (model == nullptr)
            break;

        const TPakId pakId = (TPakId)src.model.mPakId;
        ValidatePakId(pakId);
        out->models[newNumModels].mValue = model;
        out->models[newNumModels].mPakId = pakId;
        out->modelParents[newNumModels] = 0xFF;
        out->matOffset[newNumModels] = (unsigned char)boneIndex;
        if (src.ignoreCollision != 0)
            out->ignoreCollision |= 1 << newNumModels;
        if (src.animTree != nullptr)
            out->tree[newNumModels] = src.animTree;

        XModelLod* lod = nullptr;
        for (int lodIndex = 0; lodIndex < 5; ++lodIndex)
        {
            if (model->lod[lodIndex] != nullptr)
            {
                lod = model->lod[lodIndex];
                break;
            }
        }
        const unsigned int modelBoneCount =
            lod != nullptr && lod->xmodelParts != nullptr
                ? (unsigned int)lod->xmodelParts->mHierarchy.mSize
                : 0;
        boneIndex += modelBoneCount;
        ++newNumModels;
    }

    out->numModels = (unsigned char)newNumModels;
    out->numBones = (unsigned char)boneIndex;
    if (newNumModels == 0)
        return;
    extern void DObjCreateSkel(DObj* obj, char* buf);
    DObjCreateSkel(out, nullptr);
    DObjCalcAnim(out, -1);
    out->SetLOD(0);
}
void DObjCreateSkel(DObj* obj, char* a)
{
    (void)a;

    DSkel* skel = nullptr;
    if (obj->numBones == 1)
        skel = gDSkelFreeList.Alloc();
    else if (obj->numBones > 4)
        skel = reinterpret_cast<DSkel*>(gDSkelMaxFreeList.Alloc());
    else
        skel = reinterpret_cast<DSkel*>(gDSkel4FreeList.Alloc());

    if (skel == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1130;
        AeAssert::gCurrentExpr = "skel";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    obj->skel = skel;
    if ((reinterpret_cast<uintptr_t>(skel) & 0xF) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1134;
        AeAssert::gCurrentExpr = "!(((int) obj->skel) & 15)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if ((reinterpret_cast<uintptr_t>(skel->mat) & 0xF) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DObj.cpp";
        AeAssert::gCurrentLine = 1135;
        AeAssert::gCurrentExpr = "!(((int) obj->skel->mat) & 15)";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }

    memset(skel->animPartBits, 0, sizeof(skel->animPartBits));
    memset(skel->controlPartBits, 0, sizeof(skel->controlPartBits));
    memset(skel->skelPartBits, 0, sizeof(skel->skelPartBits));

    for (unsigned int modelIndex = 0; modelIndex < obj->numModels;
         ++modelIndex)
    {
        DObjSkelMat* parentMat = nullptr;
        const unsigned char parentIndex = obj->modelParents[modelIndex];
        if (parentIndex != 0xFF)
            parentMat = &skel->mat[parentIndex];
        XModelGetBasePose(obj->models[modelIndex],
                          &skel->mat[obj->matOffset[modelIndex]], parentMat);
    }
}
void DObjDisplayAnim(DObj* obj) { (void)obj; }
void DObjDisplayAnim3D(int a, DObj* obj, float* const b, int c)
{
    (void)a; (void)obj; (void)b; (void)c;
}
void DObjDumpInfo(DObj* obj) { (void)obj; }
void DObjFree(void* obj, int a) { (void)obj; (void)a; }
void DObjGetBounds(const DObj* obj, math::Position3& a, math::Position3& b)
{
    (void)obj; (void)a; (void)b;
}
void DObjGetHierarchyBits(DObj* obj, int a, int* b)
{
    (void)obj; (void)a; (void)b;
}
void DObjUpdateChildren(DObj* obj, int a) { (void)obj; (void)a; }
void DObjUpdateLod(Entity* e) { (void)e; }
void DynamicDecalMgr_DestroyAllDecals() {}
void DynamicDecalMgr_Update(void* self, float a) { (void)self; (void)a; }
void Entity_Notify(Entity* e, unsigned int a) { (void)e; (void)a; }
void Entity_Notify(void* e, unsigned int a) { (void)e; (void)a; }
void EntityHandleDb_Compact(void* self) { (void)self; }
void EntityHandleDb_Init(void* self) { (void)self; }
void EntityHandleDb_Release(void* self, Entity* e)
{
    if (self != nullptr && e != nullptr)
        static_cast<EntityHandleDb*>(self)->Release(*e);
}
// EntityHandleDb_Find templates (g.o 0x4B1A40 / 0x4B1B00 / 0x4B1B60)
template <typename T>
void EntityHandleDb_Find(int fieldOfs, T match,
                         ae_sized_array<Entity*, 4096>& results);

template <>
void EntityHandleDb_Find<Broc::string>(int fieldOfs,
                                       Broc::string match,
                                       ae_sized_array<Entity*, 4096>& results)
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    for (; p != end; ++p)
    {
        Entity* e = *p;
        if (e != nullptr && Broc::operator==(
                *(Broc::string*)((char*)&e->s.eType + fieldOfs), match))
            results.push_back(e);
    }
}
template <>
void EntityHandleDb_Find<HashString>(int fieldOfs, HashString match,
                                     ae_sized_array<Entity*, 4096>& results)
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    for (; p != end; ++p)
    {
        Entity* e = *p;
        if (e != nullptr
            && *(unsigned int*)((char*)&e->s.eType + fieldOfs) == match.mHash)
            results.push_back(e);
    }
}
template <>
void EntityHandleDb_Find<unsigned short>(
    int fieldOfs, unsigned short match,
    ae_sized_array<Entity*, 4096>& results)
{
    Entity* const* p = EntityHandleDb::sInst.mActiveList.m_elements;
    Entity* const* end = p + EntityHandleDb::sInst.mActiveList.m_size;
    for (; p != end; ++p)
    {
        Entity* e = *p;
        if (e != nullptr
            && *(unsigned short*)((char*)&e->s.eType + fieldOfs) == match)
            results.push_back(e);
    }
}
void EntityManager_CreateWorld()
{
    EntityManager::sInst->CreateWorld();
}
void EntityNotifySet_dtor(void* self) { (void)self; }
void EntityNotifySet_UpdateList() {}
void* EntityNotifySet_GetNotify(void* self, unsigned int a)
{
    (void)self; (void)a;
    return nullptr;
}
// IDA types: GdbFileSet is {InplaceString, InplaceTree<uint,uint>,
// InplaceTree<InplaceString, InplaceVector<Value> const *>}; GdbFileBank's
// mTree is at +0x08 and its mPtrs vector is at +0x10.
struct GdbTreeElementLookup {
    const char* mKey;
    unsigned int mValue;
};
struct GdbTreeLookup {
    unsigned int mSize;
    GdbTreeElementLookup* mArray;
};
struct GdbFileSetLookup {
    const char* mName;
    unsigned char mLayout[8];
    GdbTreeLookup mRecords;
};
struct GdbFileBankLookup {
    unsigned int mFileId;
    float mVersion;
    GdbTreeLookup mTree;
    unsigned int mPtrsSize;
    GdbFileSetLookup** mPtrsList;
    void* mPtrFixupTable;
};
struct GdbFileLookupResult {
    GdbFileSetLookup* mValue;
    TPakId mPakId;
};
static_assert(sizeof(GdbTreeElementLookup) == 8,
              "GDB tree element layout mismatch");
static_assert(sizeof(GdbFileSetLookup) == 20,
              "GDB file set layout mismatch");
static_assert(sizeof(GdbFileBankLookup) == 28,
              "GDB bank layout mismatch");

extern void PtrFixupTable_Fixup(void* self, void* basePtr);
extern void GetPakPrerequisites(TPakId pakId,
                                ae_sized_array<TPakId, 32>* ret);

static unsigned int* GdbTreeFindIndex(GdbTreeLookup* tree,
                                      const char* key)
{
    if (tree == nullptr || tree->mSize == 0 || tree->mArray == nullptr)
        return nullptr;
    unsigned int index = 0;
    for (;;) {
        if (index >= tree->mSize)
            return nullptr;
        GdbTreeElementLookup& element = tree->mArray[index];
        if (element.mKey == nullptr && element.mValue == 0)
            return nullptr;
        int comparison = _stricmp(element.mKey, key);
        if (comparison == 0)
            return &element.mValue;
        index = comparison >= 0 ? (2 * index + 1) : (2 * index + 2);
    }
}

static void** GdbTreeFindRecord(GdbTreeLookup* tree, const char* key)
{
    unsigned int* value = GdbTreeFindIndex(tree, key);
    return reinterpret_cast<void**>(value);
}

void** InplaceTree_Find_GdbFileRecords(void* tree, const char* const* key)
{
    if (key == nullptr)
        return nullptr;
    return GdbTreeFindRecord(reinterpret_cast<GdbTreeLookup*>(tree), *key);
}

void* FEManager_GetDMS(void* self, int client)
{
    (void)self; (void)client;
    return nullptr;
}
void* FEManager_GetFont(void* self, int a)
{
    (void)self; (void)a;
    return nullptr;
}
void* FEManager_GetFont(void* self, int a, float b)
{
    (void)self; (void)a; (void)b;
    return nullptr;
}
void* FEManager_GetIGMS(void* self, int client)
{
    (void)self; (void)client;
    return nullptr;
}
void FEManager_DrawControllerError(void* self) { (void)self; }
void FEManager_DrawIGO(void* self, int client)
{
    static_cast<FEManager*>(self)->DrawIGO(client);
}
void FEManager_PlayFadeInOranScreen() {}
void FEManager_UpdateLoadingMenu(void* self, float a) { (void)self; (void)a; }
void FEManager_UpdateSplitScreen(void* self) { (void)self; }
void g_femanager_IGO_Update(int a) { (void)a; }
void G_FreeInteractionInfo() {}
void G_RunFrameForEntity(Entity* e, int a) { (void)e; (void)a; }
void G_TouchTriggersAndVehicles(Entity* e, const math::Position3* a,
                                const void* b)
{
    (void)e; (void)a; (void)b;
}
void gDObjFreeList_Init(int a) { gDObjFreeList.Init(a); }
void gDSkel4FreeList_Init(int a) { gDSkel4FreeList.Init(a); }
void gDSkelFreeList_Init(int a) { gDSkelFreeList.Init(a); }
void gDSkelMaxFreeList_Init(int a) { gDSkelMaxFreeList.Init(a); }
void gEntFreeList_Init(int a) { gEntFreeList.Init(a); }
void gRefEntFreeList_Init(int a) { gRefEntFreeList.Init(a); }
struct weaponFileInfo_t;
void GetADSLerpTimeRemaining(PlayerState* ps, weaponFileInfo_t* wi)
{
    (void)ps; (void)wi;
}
void GetPakPrerequisites(TPakId a, void* b) { (void)a; (void)b; }
void GlowCallback(void* a) { (void)a; }
extern unsigned int gpuHashVertexShader;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
void gpuSetVertexShader(const unsigned int* shader)
{
    if ((unsigned int)shader != gpuHashVertexShader)
    {
        gpuHashVertexShader = (unsigned int)shader;
        D3DDevice_LoadVertexShaderProgram(shader, 0);
        D3DDevice_SelectVertexShaderDirect(&gpuSetVertexShaderInputs, 0);
    }
}
void GScr_LoadScriptsAndAnimsForEntities() {}
struct game_hudelem_s;
void HudElem_SetDefaults(game_hudelem_s* h) { (void)h; }
void IGO_Update(void* self, float a) { (void)self; (void)a; }
void IGOCompassWidget_SetHideCompassStar(int a, int b, int c)
{
    (void)a; (void)b; (void)c;
}
void InGameMenuSystem_ActivateMenu(void* self, int a) { (void)self; (void)a; }
void InGameMenuSystem_ActivatePauseMenu(void* self) { (void)self; }
void InitCDAepsShader() {}
void InitLights() {}
void InplaceAssetBank_Fixup(void* self) { (void)self; }
void InplaceAssetBank_GdbFileSet_Fixup(void* self)
{
    unsigned char* bank = reinterpret_cast<unsigned char*>(self);
    unsigned int fixupOffset = *(unsigned int*)(bank + 0x18);
    if (fixupOffset >= 0x10000000u) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\inplace/InplaceAssetBank.h";
        AeAssert::gCurrentLine = 0x7A;
        AeAssert::gCurrentExpr =
            "((unsigned)mPtrFixupTable<0x10000000)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Fixup offset is unusually large"))
            __debugbreak();
    }
    void* fixup = bank + fixupOffset;
    *(void**)(bank + 0x18) = fixup;
    PtrFixupTable_Fixup(fixup, bank);
}
void InplaceAssetBankSet_Find_GdbFileBank(void* self, void* out,
                                          TPakId pakId, const char* key,
                                          void* formal, void* foundPakId)
{
    (void)formal;
    GdbFileLookupResult* result =
        reinterpret_cast<GdbFileLookupResult*>(out);
    result->mValue = nullptr;
    result->mPakId = PAK_ID_INVALID;
    if (pakId == PAK_ID_INVALID)
        return;

    ae_sized_array<TPakId, 32> prereqs;
    prereqs.m_size = 0;
    GetPakPrerequisites(pakId, &prereqs);
    void** bankArray = reinterpret_cast<void**>(
        reinterpret_cast<unsigned char*>(self) + 4);
    for (unsigned int i = 0; i < (unsigned int)prereqs.m_size; ++i) {
        TPakId candidate = prereqs.m_elements[i];
        if (candidate == PAK_ID_INVALID)
            continue;
        GdbFileBankLookup* bank =
            reinterpret_cast<GdbFileBankLookup*>(bankArray[(int)candidate]);
        if (bank == nullptr)
            continue;
        unsigned int* index = GdbTreeFindIndex(&bank->mTree, key);
        if (index == nullptr || *index >= bank->mPtrsSize)
            continue;
        if (foundPakId != nullptr)
            *reinterpret_cast<TPakId*>(foundPakId) = candidate;
        result->mPakId = candidate;
        result->mValue = bank->mPtrsList[*index];
        return;
    }
}
void InplaceAssetBankSet_GdbFileBank_AddBank(void* self, TPakId pak, void* b)
{
    if ((int)pak < 0 || pak > PAK_ID_MAX)
        return;
    void** slot = reinterpret_cast<void**>(
        reinterpret_cast<unsigned char*>(self) + 4) + (int)pak;
    if (*slot != nullptr) {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\InplaceAssetBankSet.h";
        AeAssert::gCurrentLine = 0x6D;
        AeAssert::gCurrentExpr = "mBankArray[(int)pakId] == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("We already have a bank for this pak id!"))
            __debugbreak();
    }
    *slot = b;
}
class InspectorManager;
void InspectorManager_Initialise(InspectorManager* self) { (void)self; }
void InspectorManager_Render(void* self) { (void)self; }
void InspectorManager_Update(InspectorManager* self) { (void)self; }
void InteractionController_ClearQueue(void* self) { (void)self; }
void InteractionController_Update(void* self, float a) { (void)self; (void)a; }
void InvalidateParticleCollisionCaches() {}
namespace phys_constraint_solver_multithreaded {
struct list_constraint_solver;
}
struct physics_system;
void list_constraint_solver_process(
    phys_constraint_solver_multithreaded::list_constraint_solver* self,
    physics_system* sys, int a)
{
    (void)self; (void)sys; (void)a;
}
void LiveWrapper_ClearRemotePlayers(void* self) { (void)self; }
void MemoryUnitManager_Service() {}
// ea: 0x005BBC20
void MemPrint(const char* Format, ...)
{
    char Work[512];
    va_list ap;
    va_start(ap, Format);
    vsprintf(Work, Format, ap);
    va_end(ap);
    tlFinalPrint(Work);
}
struct Mapinfo_t {
    char map_pack;
    char map_id_number;
    char map_name_string[32];
    char map_title_string[32];
    char short_name[16];
    char map_location_string[32];
};
extern Mapinfo_t g_TheMapInfo[64];
extern int g_NumBaseMaps;
extern int g_NumTotalMaps;

// Rebuild the map-index counts from the IDA map record table before menus
// consume the map conversion table.
void MI_ResetMapList()
{
    g_NumBaseMaps = 0;
    g_NumTotalMaps = 0;
    for (int i = 0; i < 64; ++i)
    {
        if ((unsigned char)g_TheMapInfo[i].map_id_number == 0xFF)
            break;
        ++g_NumTotalMaps;
    }
    g_NumBaseMaps = g_NumTotalMaps;
}
void MusicMgr_Update(void* self, float a) { (void)self; (void)a; }
class nglRenderNode;
void nglDxUnbindVertexBuffer() {}
void ngliExitList() {}
struct nglMeshSection;
void ngliUnloadSection(nglMeshSection* s) { (void)s; }
void nullsub_16(const char* a, const char* b) { (void)a; (void)b; }
void nullsub_34(const char* a, const char* b) { (void)a; (void)b; }
void nullsub_35() {}
void orthonormalize(math::Mat43* m) { (void)m; }
void PathNodeMgr_CleanUpManager(void* self) { (void)self; }
void PathNodeMgr_ConnectPathsForEntity(void* self, Entity* e)
{
    (void)self; (void)e;
}
void PathNodeMgr_DissociateSentient(void* self, sentient_s* s)
{
    (void)self; (void)s;
}
void PathNodeMgr_InitPaths(void* self) { (void)self; }
void PHYS_ASSERT_ORTHOGONAL(const math::Dir3& a, const math::Dir3& b)
{
    (void)a; (void)b;
}
void PHYS_ASSERT_ORTHONORMAL(const math::Mat43* m) { (void)m; }
void PHYS_ASSERT_UNIT(const math::Dir3* a) { (void)a; }
void physics_debug_render() {}
void Player_ActivateHoldCmd(Entity* e) { (void)e; }
void PlayerAnimMgr_Update(float a) { (void)a; }
void PrintPakNames() {}
void R_InitDebug() {}
void R_ShutdownDebug() {}
void R_ToggleSmpFrame() {}
void RE_AddRefEntityToScene(void* a, int b) { (void)a; (void)b; }
void RE_AddViewModelToScene(void* a) { (void)a; }
struct glconfig_t;
extern void RE_BeginRegistration(glconfig_t* glconfigOut);
void re_BeginRegistration(int* a)
{
    RE_BeginRegistration(reinterpret_cast<glconfig_t*>(a));
}
void re_DebugLines(int a) { (void)a; }
void re_DebugStrings(int a) { (void)a; }
void re_DrawQuadPic(const float* a, const float* b, void* c)
{
    (void)a; (void)b; (void)c;
}
void re_EndRegistration() {}
void re_LocateDebugLines(int a, int b) { (void)a; (void)b; }
void re_LocateDebugStrings(int a, int b) { (void)a; (void)b; }
void RE_SetViewModelInfoIndex(int a) { (void)a; }
void re_ShutdownFn(int a) { (void)a; }
void RenderCDHeatHazeShader() {}
void reserved_dlist_Curve_erase(void* a, void* b) { (void)a; (void)b; }
void reserved_dlist_CurveEffectListElem_erase(void* a, void* b)
{
    (void)a; (void)b;
}
void RumbleEffect_Ctor(void* self) { (void)self; }
void RumbleEffect_SetIntensity(void* self, int a, float b)
{
    (void)self; (void)a; (void)b;
}
void RumbleEffect_SetNotes(void* self, int a, void* b)
{
    (void)self; (void)a; (void)b;
}
void SceneManager_UpdateEffects(void* self, float a) { (void)self; (void)a; }
// ea: 0x005C1AC0
void Scr_Error(const char* error)
{
    AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
    AeAssert::gCurrentLine = 16;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(off_CFBB58, error))
        __debugbreak();
}
void Scr_FreePrecachedAnimTrees() {}
// ea: 0x005C1B10
void Scr_ParamError(unsigned int index, const char* error)
{
    (void)index;
    AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\scr_vm.cpp";
    AeAssert::gCurrentLine = 23;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning(off_CFBB58, error))
        __debugbreak();
}
void Scr_PrecacheAnimTrees(void* (*cb)(int), bool a) { (void)cb; (void)a; }
void Scr_PrecacheAnimTrees(void* (*cb)(void*, unsigned int), int a)
{
    (void)cb; (void)a;
}
void ScriptEventHandler_dtor(void* self) { (void)self; }
// ea: 0x005C1A50
void SetAnimCheck(int bAnimCheck)
{
    g_bAnimCheck = bAnimCheck;
}
struct nglShaderParamSet;
struct Color;
void setup_color(const Color& c, nglShaderParamSet& p) { (void)c; (void)p; }
void SetupCDHeatHazeShader() {}
void SoundDevice_DampenAllSounds(void* self, float a) { (void)self; (void)a; }
void SoundDevice_PauseAllSounds(void* self) { (void)self; }
void SoundDevice_ReleaseSound(void* self, void* s) { (void)self; (void)s; }
void SoundDevice_SetNumberOfListeners(void* self, int a) { (void)self; (void)a; }
void SoundDevice_StopAllSounds(void* self) { (void)self; }
void SoundDevice_UndampenAllSounds(void* self) { (void)self; }
void SoundDevice_UnpauseAllSounds(void* self) { (void)self; }
void SoundMediaMgr_PlayLandingSound(void* self, Entity* e, int a, bool b)
{
    (void)self; (void)e; (void)a; (void)b;
}
void StatusBar_Init(void* self) { (void)self; }
void StreamZoneManager_Update(void* self, int a, const float* b, bool c)
{
    (void)self; (void)a; (void)b; (void)c;
}
void StubData_ApplyStubOptions(void* a) { (void)a; }
void sWeaponAnimCallback() {}
extern void TaskHandlerImpl_Update(TaskHandlerImpl* h, float deltaT,
                                   void* ftor);
void TaskHandler_Update(TaskHandler* h, float a, TaskFunctor* f)
{
    TaskHandlerImpl_Update(reinterpret_cast<TaskHandlerImpl*>(h), a, f);
}
void TaskHandler_Update(TaskHandlerImpl* h, float a, void* f)
{
    TaskHandlerImpl_Update(h, a, f);
}
void TimerRenderBars_Init(void* self)
{
    (void)self;
    TimerRenderBars::sInst.Init();
}
void UpdateWheelMarks(Entity* e, int a, bool b, const math::Position3& c,
                      const math::Dir3& d)
{
    (void)e; (void)a; (void)b; (void)c; (void)d;
}
void j_nullsub_50(void* self) { (void)self; }
void ValidatePakId(int a) { (void)a; }
void View_SetViewportClipping(int a) { (void)a; }
void WaitTilOutput_AssignData(void* a, void* b) { (void)a; (void)b; }
void Weapon_MeleeHitShock(Entity* e) { (void)e; }
void WheelMarkMgr_Exit() {}
void WheelMarkMgr_Init() {}
void WheelMarkMgr_Reset() {}
struct XAnimTree;
struct XAnimEntry;
void XAnimCalcAbsDelta(XAnimTree* t, unsigned int a, float* const b,
                       float* const c)
{
    (void)t; (void)a; (void)b; (void)c;
}
void XAnimFreeTree(XAnimTree* t) { (void)t; }
void XAnimGetAbsDelta(AnimTree* t, unsigned int a, float* const b,
                      float* const c, float d)
{
    (void)t; (void)a; (void)b; (void)c; (void)d;
}
void XAnimGetRelDelta(AnimTree* t, unsigned int a, float* const b,
                      float* const c, float d, float e)
{
    (void)t; (void)a; (void)b; (void)c; (void)d; (void)e;
}
void XAnimSetCompleteGoalWeight(XAnimTree* t, unsigned int a, float b, float c,
                                float d, unsigned int e, unsigned short f,
                                int g)
{
    (void)t; (void)a; (void)b; (void)c; (void)d; (void)e; (void)f; (void)g;
}
void XFONT_OpenTrueTypeFont(const unsigned short* a, unsigned int b, void* c)
{
    (void)a; (void)b; (void)c;
}
void XModelEnforceExist(int a) { (void)a; }
void XModelGetBasePose(IVPointer<XModel> model, DObjSkelMat* mat,
                       DObjSkelMat* modelParentMat)
{
    ValidatePakId((TPakId)model.mPakId);

    int lodIndex = 0;
    while (model.mValue->lod[lodIndex] == nullptr)
        ++lodIndex;
    XModelParts* parts = model.mValue->lod[lodIndex]->xmodelParts;
    const unsigned int boneCount = parts->mHierarchy.mSize;

    for (unsigned int bone = 0; bone < boneCount; ++bone)
    {
        const int parentIndex = parts->mHierarchy.mList[bone].mParentIndex;
        const math::Mat43::Packed& local = parts->mTransforms.mList[bone];
        DObjSkelMat* out = &mat[bone];

        if (parentIndex < 0)
        {
            if (modelParentMat == nullptr)
            {
                out->axis[0][0] = local.x.x;
                out->axis[0][1] = local.x.y;
                out->axis[0][2] = local.x.z;
                out->axis[0][3] = 0.0f;
                out->axis[1][0] = local.y.x;
                out->axis[1][1] = local.y.y;
                out->axis[1][2] = local.y.z;
                out->axis[1][3] = 0.0f;
                out->axis[2][0] = local.z.x;
                out->axis[2][1] = local.z.y;
                out->axis[2][2] = local.z.z;
                out->axis[2][3] = 0.0f;
                out->origin[0] = local.w.x;
                out->origin[1] = local.w.y;
                out->origin[2] = local.w.z;
                out->origin[3] = 1.0f;
                continue;
            }
        }

        const DObjSkelMat* parent =
            parentIndex < 0 ? modelParentMat : &mat[parentIndex];

        out->axis[0][0] = local.x.x * parent->axis[0][0]
                         + local.x.y * parent->axis[1][0]
                         + local.x.z * parent->axis[2][0];
        out->axis[0][1] = local.x.x * parent->axis[0][1]
                         + local.x.y * parent->axis[1][1]
                         + local.x.z * parent->axis[2][1];
        out->axis[0][2] = local.x.x * parent->axis[0][2]
                         + local.x.y * parent->axis[1][2]
                         + local.x.z * parent->axis[2][2];
        out->axis[0][3] = 0.0f;
        out->axis[1][0] = local.y.x * parent->axis[0][0]
                         + local.y.y * parent->axis[1][0]
                         + local.y.z * parent->axis[2][0];
        out->axis[1][1] = local.y.x * parent->axis[0][1]
                         + local.y.y * parent->axis[1][1]
                         + local.y.z * parent->axis[2][1];
        out->axis[1][2] = local.y.x * parent->axis[0][2]
                         + local.y.y * parent->axis[1][2]
                         + local.y.z * parent->axis[2][2];
        out->axis[1][3] = 0.0f;
        out->axis[2][0] = local.z.x * parent->axis[0][0]
                         + local.z.y * parent->axis[1][0]
                         + local.z.z * parent->axis[2][0];
        out->axis[2][1] = local.z.x * parent->axis[0][1]
                         + local.z.y * parent->axis[1][1]
                         + local.z.z * parent->axis[2][1];
        out->axis[2][2] = local.z.x * parent->axis[0][2]
                         + local.z.y * parent->axis[1][2]
                         + local.z.z * parent->axis[2][2];
        out->axis[2][3] = 0.0f;
        out->origin[0] = local.w.x * parent->axis[0][0]
                       + local.w.y * parent->axis[1][0]
                       + local.w.z * parent->axis[2][0]
                       + parent->origin[0];
        out->origin[1] = local.w.x * parent->axis[0][1]
                       + local.w.y * parent->axis[1][1]
                       + local.w.z * parent->axis[2][1]
                       + parent->origin[1];
        out->origin[2] = local.w.x * parent->axis[0][2]
                       + local.w.y * parent->axis[1][2]
                       + local.w.z * parent->axis[2][2]
                       + parent->origin[2];
        out->origin[3] = 1.0f;
    }
}
void XModelTransform(IVPointer<XModel> model, DObjSkelMat* a,
                     DObjSkelMat* b)
{
    (void)model; (void)a; (void)b;
}
const void* DCGBank_get_set(void* self, int a)
{
    struct DCGBankView {
        unsigned int count;
        unsigned char* elements;
    };
    DCGBankView* bank = (DCGBankView*)self;
    for (unsigned int i = 0; i < bank->count; ++i)
    {
        unsigned char* set = bank->elements + i * 0x70;
        if (*(int*)(set + 0x68) == a)
            return set;
    }
    return nullptr;
}
const void* StreamZoneManager_GetCellZone(void* self, int a)
{
    (void)self; (void)a;
    return nullptr;
}

struct searchpath_s;
void FS_ShutdownSearchPaths(searchpath_s* sp) { (void)sp; }
struct weaponParms;
void G_BulletFireSpread(Entity* a, Entity* b, weaponParms* wp, int c, float d,
                        Entity* e, float f, int g)
{
    (void)a; (void)b; (void)wp; (void)c; (void)d; (void)e; (void)f; (void)g;
}
void AssetBankSet_dtor(void* self) { (void)self; }
void SceneManager_ResetAllStaticModels() {}
void ae_sized_array_push_back_handler(struct TaskSysImpl2* self,
                                      struct TaskHandlerImpl* const* elem)
{
    (void)self;
    if (elem == nullptr || *elem == nullptr
        || TaskSys::sInst.mTaskHandlersSize >= 32)
        return;
    TaskSys::sInst.mTaskHandlers[
        TaskSys::sInst.mTaskHandlersSize++] = *elem;
}
void ae_sized_array_push_back_pair(
    DroneAEMap* self, DroneHandlePair* const* elt)
{
    reinterpret_cast<DroneAEArray*>(self)->push_back(*elt);
}

void* cdScratchMaterial_Ctor(void* self, void* a, unsigned int b, int c,
                             bool d)
{
    (void)self; (void)a; (void)b; (void)c; (void)d;
    return nullptr;
}
void* DbTablesetMgr_Find(void* self, TPakId pak, const char* name,
                         TPakId* foundPak)
{
    (void)self; (void)pak; (void)name; (void)foundPak;
    return nullptr;
}
struct nglMesh;
struct nglMeshParams;
struct nglShaderParamSet;
class nglMeshNode;
nglMeshNode* nglListAddMesh(nglMesh* mesh, const math::Mat43& m,
                            nglMeshParams* mp, nglShaderParamSet* sp,
                            void (*fn)(nglMeshNode*))
{
    (void)mesh; (void)m; (void)mp; (void)sp; (void)fn;
    return nullptr;
}

// PakFile/PakManager minimal views (streamer.o; mPath at +0x0C and mSlots
// at +0x40 are verified by IDA's PakManager type and LoadScript disassembly).
struct PakFileView {
    uint8_t _pad[0x0C];
    char    mPath[0x100];  // +0x0C (ae_fixed_string bytes)
};
struct PakManagerView {
    uint8_t      _pad[0x40];
    PakFileView* mSlots[0x63];  // +0x40 (99 entries)
};
static_assert(offsetof(PakManagerView, mSlots) == 0x40,
              "PakManager::mSlots offset mismatch");

// IDA ae_heap layout: ae_heap_base at +0x00 and mem_heap storage at +0x04,
// total size 0x4A0.  The constructor itself is defined by the streamer/core
// object; this view supplies the verified storage size for placement new.
class ae_heap {
    void** mVtable;
    uint8_t mHeap[0x49C];
public:
    explicit ae_heap(unsigned int size);
};
static_assert(sizeof(ae_heap) == 0x4A0, "ae_heap size mismatch");

extern BrocExports gBrocExports;  // scr.o @ 0xF3A7B0

namespace BrocHelper {
void Init();
void RegisterBroFunc(char* name, unsigned int (__cdecl* func)(void*));
}

namespace mp_level {
typedef void (__cdecl* InitScriptFn)();
InitScriptFn InitScript(BrocAPI** gamesAPIptr,
                        BrocExports& exports);
}

namespace BrocSys {
void ValidateApiSize(int sizeofBrocAPI, int sizeofBrocExports);
void InitAPI();
void BrocDebugRender();
}

// IDA's global BrocExports is 456 bytes with mRegisterDebugStrings at +0x74,
// mInit at +0x1BC, and mRegisterFunction at +0x1C0.  The game header's
// reduced global view predates those fields, so LoadScript uses the complete
// IDA-derived engine view without changing the existing callers yet.
using BrocExportsLoadScriptView = Broc::BrocExports;
static_assert(sizeof(BrocExportsLoadScriptView) == 0x1C8,
              "BrocExports IDA layout mismatch");

// IDA global: gEntryFp (scr.o)
void (*gEntryFp)() = nullptr;

// BrocSys (scr.o; stubs, port later)
namespace BrocSys {
// ea: 0x005CA8B0
void Init()
{
    ae_sized_array<PoolAllocator::PoolConfig, 16> cfgList;
    memset(&cfgList, 0, sizeof(cfgList));
    cfgList.m_size = 0;

    PoolAllocator::PoolConfig elt;
    elt.blockSize = 0x20;
    elt.blockAlign = 4;
    elt.numBlocks = 0x2328;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 0x40;
    elt.blockAlign = 4;
    elt.numBlocks = 0x2EE;
    elt.block = nullptr;
    cfgList.push_back(elt);

    elt.blockSize = 0x80;
    elt.blockAlign = 4;
    elt.numBlocks = 0x1F4;
    elt.block = nullptr;
    cfgList.push_back(elt);

    void* block = mem_heap_malloc(0x3C);
    if (block != nullptr)
        gBrocPool = new (block) PoolAllocator(cfgList, 1u);
    else
        gBrocPool = nullptr;

    block = mem_heap_malloc(0x4A0);
    if (block != nullptr)
        gBrocHeap = new (block) ae_heap(0x10000u);
    else
        gBrocHeap = nullptr;

    DebugRender::sInst.AddRenderer(BrocSys::BrocDebugRender);
}
bool IsValidClientType(Entity* pEnt);
void TakeWeapon(Entity* pSelf, const char* pszWeaponName)
{
    if (IsValidClientType(pSelf))
    {
        int weaponIndex = BG_GetWeaponIndexForName(pszWeaponName);
        pSelf->client->ps.ammo[BG_AmmoForWeapon(weaponIndex)] = 0;
        pSelf->client->ps.ammoclip[BG_ClipForWeapon(weaponIndex)] = 0;
        BG_TakePlayerWeapon(&pSelf->client->ps, weaponIndex);
    }
}
// ea: 0x005BDF20
void CopyExtendedEntity(const Entity* source, Entity* dest)
{
    if (gpBrocAPI != nullptr && source->mBrocExtendedEntity != nullptr)
    {
        if (dest->mBrocExtendedEntity != nullptr)
        {
            AeAssert::gCurrentAuthor =
                static_cast<AeAssert::ECoderId>(1);  // ARO
            AeAssert::gCurrentFile =
                "c:\\cod\\code\\game\\BrocSys.cpp";
            AeAssert::gCurrentLine = 4662;
            AeAssert::gCurrentExpr = "!dest->mBrocExtendedEntity";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("This should be NULL"))
                __debugbreak();
        }
        dest->mBrocExtendedEntity =
            Broc::gBrocAPI.mBrocExports.mCopyExtendedEntity(
                source->mBrocExtendedEntity);
    }
}
// ea: 0x005BDFC0
void UnloadScript()
{
    if (::gEntryFp != nullptr)
        ::gEntryFp();
}

// ea: 0x005E0160
void LoadScript()
{
    struct AeThreadManagerLoadScriptView {
        unsigned int sNumThreads;  // IDA AeThreadManager +0x00
    };

    if (((AeThreadManagerLoadScriptView*)&AeThreadManager::sInst)
            ->sNumThreads != 0)
    {
        AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 4939;
        AeAssert::gCurrentExpr =
            "AeThreadManager::Inst()->sNumThreads == 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("need to clean up threads!"))
            __debugbreak();
    }

    TPakId pakId = CurPakId();
    if (pakId < 0 || pakId >= 0x63)
        return;
    if (((PakManagerView*)PakManager::sInst)->mSlots[pakId] == nullptr)
        return;

    if (_stricmp("mp_level", "mp_level") == 0
        && mp_level::InitScript != nullptr)
    {
        BrocExportsLoadScriptView* exports =
            reinterpret_cast<BrocExportsLoadScriptView*>(&gBrocExports);
        exports->mInit = BrocHelper::Init;
        exports->mRegisterFunction =
            reinterpret_cast<decltype(exports->mRegisterFunction)>(
                BrocHelper::RegisterBroFunc);
        exports->mValidateApiSize = BrocSys::ValidateApiSize;
        ::gEntryFp = mp_level::InitScript(&gpBrocAPI, gBrocExports);
        BrocSys::InitAPI();
        ::gEntryFp();
        exports->mRegisterDebugStrings();
    }
    else
    {
        AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(1);
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\BrocSys.cpp";
        AeAssert::gCurrentLine = 5234;
        AeAssert::gCurrentExpr = nullptr;
        if (AeAssert::Error(
                "The broc library for '%s' isn't linked in!", "mp_level"))
            __debugbreak();
        tlPrintf("The broc library for '%s' isn't linked in!", "mp_level");
    }
}
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
extern void Axis4ToAngles(const float (*const axis)[4],
                          float* const angles);  // core.o

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
    static void CreateInst(); // ?CreateInst@MusicMgr@@SAXXZ (core.o 0x4DCC00)
    static void DeleteInst(); // ?DeleteInst@MusicMgr@@SAXXZ (core.o 0x4E27E0)
    static void* operator new(size_t size, void* p);
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

// ea: 0x004DCBF0
void* MusicMgr::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

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

// ea: 0x004DCC00
void MusicMgr::CreateInst()
{
    if (MusicMgr::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.h";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x20u, 4, "fx", "c:\\cod\\code\\game\\MusicMgr.h", 21);
    if (memory != nullptr)
        MusicMgr::sInst = new (memory) MusicMgr();
    else
        MusicMgr::sInst = nullptr;
}

// ea: 0x004E27E0
void MusicMgr::DeleteInst()
{
    MusicMgr* instance = MusicMgr::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\MusicMgr.h";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
    {
        instance->~MusicMgr();
        mem_heap_free(instance);
    }
    MusicMgr::sInst = nullptr;
}

// ============================================================================
// SoundMediaMgr - ea: 0x603EF0..0x603F20 (SoundMediaMgr.cpp)
// ============================================================================
enum nslWaveID : int;
class SoundMediaMgr {
public:
    static SoundMediaMgr* sInst;        // ?sInst@SoundMediaMgr@@2PAV1@A
    static SoundMediaMgr* CreateInst();  // core.o 0x004DCAD0
    static void DeleteInst();            // core.o 0x004E2750
    static void* operator new(size_t size, void* p);
    nslWaveID mFoliageRustleSound;       // +0x00, IDA type size 0x04
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
                                     const CollisionDesc& col_desc);

SoundMediaMgr* SoundMediaMgr::sInst = nullptr;

// ea: 0x004DCAC0
void* SoundMediaMgr::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x004DCAD0
SoundMediaMgr* SoundMediaMgr::CreateInst()
{
    if (SoundMediaMgr::sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\SoundMediaMgr.h";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        4u, 4, "fx", "c:\\cod\\code\\game\\SoundMediaMgr.h", 21);
    if (memory != nullptr)
    {
        SoundMediaMgr* result = new (memory) SoundMediaMgr();
        SoundMediaMgr::sInst = result;
        return result;
    }
    SoundMediaMgr::sInst = nullptr;
    return nullptr;
}

// ea: 0x004E2750
void SoundMediaMgr::DeleteInst()
{
    SoundMediaMgr* instance = SoundMediaMgr::sInst;
    if (SoundMediaMgr::sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = (AeAssert::ECoderId)0;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\SoundMediaMgr.h";
        AeAssert::gCurrentLine = 21;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
    {
        instance->~SoundMediaMgr();
        mem_heap_free(instance);
    }
    SoundMediaMgr::sInst = nullptr;
}

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
    PostEffectEventLanding(entity, v5);
}

extern float nslGetWaveParam(nslWaveID wave, int b, float c);  // nsl_xboxr
extern float nslGetSourceParam(nslSourceID sid, int index,
                               float defaultValue);  // nslSource.o
extern const char* nslGetSourceName(nslSourceID sid);  // nslSource.o
extern int nslIsWaveStreamed(nslWaveID a);             // nslCompat.o
extern int g_useOnScreenSoundDebugging;   // ?g_useOnScreenSoundDebugging@@3HA
namespace AeStringSupport {
extern void AeStrCopy(char* dst, int* const dstLen, int dstCapacity,
                      const char* src, int srcLen);  // ae_string_support.cpp
}

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
class SoundOptions {
public:
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
                                 const float* const position); // nsl
extern void nslSetSourceVelocity(nslSourceID sid,
                                 const float* const velocity); // nsl
extern const char* nslWaveGetName(nslWaveID waveID);        // nsl
nslSourceID g_break_on_stop;         // ?g_break_on_stop@@3W4nslSourceID@@A (game.o)
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
                                    10, 70, Color(col[0], col[1], col[2], col[3]), 0.0f, 1.0f);
        }
        else
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            DebugRender::RenderText("ext: <no ext music>", 10, 70, Color(col[0], col[1], col[2], col[3]),
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
                                    10, 85, Color(col[0], col[1], col[2], col[3]), 0.0f, 1.0f);
        }
        else
        {
            float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
            DebugRender::RenderText("int: <no int music>", 10, 85, Color(col[0], col[1], col[2], col[3]),
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
                    AeStringSupport::AeStrCopy((char*)tmp.mBuff, &m, 127,
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
                    AeStringSupport::AeStrCopy((char*)tmp.mBuff, &m, 127,
                                               (const char*)DebugString.mBuff,
                                               DebugString.mLength);
                    tmp.mLength = (unsigned char)m;
                    streams.push_back(tmp);
                }
            }
        }
        float col[4] = { 0.25f, 0.25f, 1.0f, 1.0f };
        DebugRender::RenderText("Active sounds", 10, 20, Color(col[0], col[1], col[2], col[3]), 0.0f, 1.0f);
        int v33 = 32;
        for (unsigned int i = 0; i < (unsigned int)streams.size(); ++i)
        {
            float cola[4] = { 1.0f, 0.25f, 0.25f, 0.25f };
            DebugRender::RenderText((const char*)streams[i].mBuff, 10, v33,
                                    Color(cola[0], cola[1], cola[2], cola[3]), 0.0f, 1.0f);
            v33 += 12;
        }
        for (unsigned int j = 0; j < (unsigned int)spu.size(); ++j)
        {
            float colb[4] = { 0.25f, 1.0f, 0.25f, 0.25f };
            DebugRender::RenderText((const char*)spu[j].mBuff, 10, v33, Color(colb[0], colb[1], colb[2], colb[3]),
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
            DebugRender::RenderText(buf, 10, 20, Color(colc[0], colc[1], colc[2], colc[3]), 0.0f, 1.0f);
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
                DebugRender::RenderText3D(pos, Color(white[0], white[1], white[2], white[3]), 1.0f,
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
            DebugRender::RenderLine(pos, fwdEnd, Color(colf[0], colf[1], colf[2], colf[3]), 0.05f);
            math::Position3 upEnd = pos;
            upEnd.v.m128_f32[0] += this->mDebugListenerUp[0] * 10.0f;
            upEnd.v.m128_f32[1] += this->mDebugListenerUp[1] * 10.0f;
            upEnd.v.m128_f32[2] += this->mDebugListenerUp[2] * 10.0f;
            float colu[4] = { 0.0f, 1.0f, 1.0f, 0.5f };
            DebugRender::RenderLine(pos, upEnd, Color(colu[0], colu[1], colu[2], colu[3]), 0.05f);
        }
    }
}

// ============================================================================
// SoundHandleDb COMDATs (game.o)
// ============================================================================
// ea: 0x006629F0
SoundDevice::SoundHandleDb::SoundHandleDb()
{
    for (int i = 0; i < 16; ++i)
        _pad[i] = 0;
    for (int i = 0; i < 0x200; ++i)
    {
        mElements[i].mObject = nullptr;
        mElements[i].mKey = 1;
    }
    mDebugCallback = nullptr;
    for (int i = 0; i < 0x200; ++i)
        _pad[i >> 3] |= (uint8_t)(1u << (i & 7));
}

// ea: 0x00662280
void SoundDevice::SoundHandleDb::Dump()
{
    if (mDebugCallback != nullptr)
    {
        tlPrintf("handle db contents:\n");
        for (int i = 0; i < 0x200; ++i)
        {
            if ((_pad[i >> 3] & (1u << (i & 7))) == 0)
                mDebugCallback(i, mElements[i].mObject);
        }
    }
    AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
    AeAssert::gCurrentLine = 193;
    AeAssert::gCurrentExpr = nullptr;
    if (AeAssert::Error("out of handles! - Tell MikeA (MAX_GENTITIES)"))
        __debugbreak();
}

// ea: 0x006627B0
Handle SoundDevice::SoundHandleDb::AllocateHandle()
{
    int nextIndex = -1;
    for (int i = 0; i < 0x200; ++i)
    {
        if ((_pad[i >> 3] & (1u << (i & 7))) != 0)
        {
            nextIndex = i;
            _pad[i >> 3] &= (uint8_t)~(1u << (i & 7));
            break;
        }
    }
    if ((unsigned int)nextIndex >= 0x200u)
    {
        AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\HandleDb.h";
        AeAssert::gCurrentLine = 98;
        AeAssert::gCurrentExpr =
            "nextIndex >= 0 && nextIndex < _MaxEltements";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("index out of bounds!!! ILLEGAL array access!"))
            __debugbreak();
    }
    if (nextIndex < 0)
    {
        Dump();
        Handle result;
        result.mVal = 0xFFFFFFFFu;
        return result;
    }
    Handle result;
    result.mVal = (unsigned int)((mElements[nextIndex].mKey << 12)
                                 | nextIndex);
    return result;
}

SoundDevice::SoundHandleDb SoundDevice::SoundHandleDb::sInst;  // @ 0xF50D10
SoundDevice* SoundDevice::sInst = nullptr;                     // @ 0xF4EBDC

void SoundDevice::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\SoundDevice.h";
        AeAssert::gCurrentLine = 50;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x7AA0u, 4, "fx", "c:\\cod\\code\\game\\SoundDevice.h", 50);
    if (memory != nullptr)
        sInst = new (memory) SoundDevice();
    else
        sInst = nullptr;
}

void* EntityManager::operator new(size_t, void* p)
{
    return p;
}

EntityManager* EntityManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 9;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x48u, 4, "entity", "c:\\cod\\code\\game\\EntityManager.h", 9);
    if (memory != nullptr)
        sInst = new (memory) EntityManager();
    else
        sInst = nullptr;
    return sInst;
}

// ea: 0x004DB980
void EntityManager::DeleteInst()
{
    EntityManager* instance = EntityManager::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\EntityManager.h";
        AeAssert::gCurrentLine = 9;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
        delete instance;
    EntityManager::sInst = nullptr;
}

template <>
SoundDevice::Sound*
DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>::operator*() const
{
    unsigned int index = mHandle.mVal & 0xFFF;
    if (index < 0x200
        && mHandle.mVal >> 12
               == (unsigned int)SoundDevice::SoundHandleDb::sInst
                      .mElements[index].mKey)
        return SoundDevice::SoundHandleDb::sInst.mElements[index].mObject;
    return nullptr;
}

template <>
SoundDevice::Sound*
DbLinkedHandle<SoundDevice::SoundHandleDb, SoundDevice::Sound>::operator->() const
{
    unsigned int index = mHandle.mVal & 0xFFF;
    if (index < 0x200
        && mHandle.mVal >> 12
               == (unsigned int)SoundDevice::SoundHandleDb::sInst
                      .mElements[index].mKey)
        return SoundDevice::SoundHandleDb::sInst.mElements[index].mObject;
    return nullptr;
}

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
extern void* ToggleRenderGeom();      // game.o 0x6119F0
extern void* ToggleGraph();           // game.o 0x611A30
extern void* ToggleRenderPerf();      // game.o 0x611A10
extern void* ZoomIn();                // game.o 0x611A50
extern void* ZoomOut();               // game.o 0x611A70
extern void Teleport();               // game.o 0x611A90
extern void DebugRender_AddRenderer(void* self, void (*fp)());  // render.o
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20
extern void AssetBankSet_ctor(void* self);  // streamer.o
extern void CGBankManager_DebugRender_impl(void* self);  // 0x646700
void* CGBankManager::sInst = nullptr;         // ?sInst@CGBankManager@@2PAV1@A @ 0xF4F438

void* CGBankManager::operator new(size_t /*size*/, void* p) { return p; }

// Static bridge used by the DebugRender callback registration.
static void CGBankManager_DebugRender_bridge()
{
    CGBankManager_DebugRender_impl(CGBankManager::sInst);
}

// ea: 0x006631C0
void CGBankManager::SingletonDebugRender()
{
    static_cast<CGBankManager*>(sInst)->DebugRender();
}

// ea: 0x006492D0
CGBankManager::CGBankManager()
{
    AssetBankSet_ctor(this);
    DebugRender_AddRenderer(DebugRender_sInst,
                            CGBankManager_DebugRender_bridge);
    this->mCount = 0;
    for (int i = 0; i < 99; ++i)
        this->mBankArray[i] = nullptr;
    *(unsigned int*)((char*)this + 0x04) = 0;
    *(float*)((char*)this + 0x08) = 0.0f;
    *(unsigned int*)((char*)this + 0x04) &= 0xFFFFFFF8;
    *(float*)((char*)this + 0x08) = 150.0f;
    Cmd_AddCommand("cg", (void (*)())ToggleRenderGeom);
    Cmd_AddCommand("cggraph", (void (*)())ToggleGraph);
    Cmd_AddCommand("cgperf", (void (*)())ToggleRenderPerf);
    Cmd_AddCommand("cgzoomin", (void (*)())ZoomIn);
    Cmd_AddCommand("cgzoomout", (void (*)())ZoomOut);
    Cmd_AddCommand("teleport", Teleport);
}

// ea: 0x004B41D0
void CGBankManager::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 435;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }

    CGBankManager* result = static_cast<CGBankManager*>(
        mem_heap_malloc_ctx(0x328u, 4, "core",
                            "c:\\cod\\code\\game\\cgbank.h", 435));
    if (result != nullptr)
    {
        result = new (result) CGBankManager();
        sInst = result;
    }
    else
    {
        sInst = nullptr;
    }
}

// ea: 0x004B42D0
void CGBankManager::DeleteInst()
{
    if (sInst == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\cgbank.h";
        AeAssert::gCurrentLine = 435;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }

    if (sInst != nullptr)
        delete static_cast<CGBankManager*>(sInst);
    sInst = nullptr;
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
    static TaskHandler sHandler;          // ?sHandler@AnimNotifyTask@@0VTaskHandler@@A
    static TaskHandler* GetHandler();     // ea: 0x5187D0
    static FourCC GetTaskId();            // ea: 0x5187E0
private:
    static int Find(unsigned int key);  // ?Find@AnimNotifyTask@@CAHI@Z
public:

    unsigned int mAnimHash;        // +0x1C
    unsigned int mNotifyKillHash;  // +0x20
    static ae_vector<unsigned int> mKeys;   // ?mKeys@AnimNotifyTask@@0V?$ae_vector@I@@A @ 0xF4F460
    static ae_vector<AnimNotifyCallback> mPtrs;  // @ 0xF50CA0
};
static_assert(sizeof(AnimNotifyTask) == 0x24, "AnimNotifyTask size mismatch");
TaskHandler AnimNotifyTask::sHandler(FourCC(1095648857), 0u);
ae_vector<unsigned int> AnimNotifyTask::mKeys;
ae_vector<AnimNotifyCallback> AnimNotifyTask::mPtrs;

// ea: 0x005187D0
TaskHandler* AnimNotifyTask::GetHandler()
{
    return &AnimNotifyTask::sHandler;
}

TaskHandler* AnimNotifyTask_GetHandler()
{
    return AnimNotifyTask::GetHandler();
}

// ea: 0x005187E0
FourCC AnimNotifyTask::GetTaskId()
{
    FourCC result;
    result.mVal = 1095648857;
    return result;
}

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
enum nflFileID : unsigned { NFL_FILE_ID_INVALID = 0xFFFFFFFFu };
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

        WbkEntry& operator=(const WbkEntry& other)
        {
            unsigned int* dst = reinterpret_cast<unsigned int*>(this);
            const unsigned int* src = reinterpret_cast<const unsigned int*>(&other);
            for (int i = 0; i < 27; ++i)
                dst[i] = src[i];
            return *this;
        }
    };
    static_assert(sizeof(WbkEntry) == 0x6C, "WbkEntry view size mismatch");
    uint8_t  _pad0[4];                 // +0x00 (vftable)
    bool     mDoUnloadNotify;          // +0x04
    bool     mDoLoadNotify;            // +0x05
    uint8_t  _pad06[2];                // +0x06
    uint8_t  mAvailableWbks[0x6C0];    // +0x08 (16 * 0x6C stride)
    int      m_size;                   // +0x6C8
    static AudioBankMgr* sInst;        // ?sInst@AudioBankMgr@@2PAV1@A
    static void CreateInst();          // ?CreateInst@AudioBankMgr@@SAXXZ
    static void DeleteInst();          // ?DeleteInst@AudioBankMgr@@SAXXZ
    static AudioBankMgr* Inst();       // ?Inst@AudioBankMgr@@SAPAV1@XZ
    static void* operator new(size_t size, void* p);
    AudioBankMgr();                    // ??0AudioBankMgr@@QAE@XZ (game.o 0x621440)
    virtual ~AudioBankMgr();           // ??1AudioBankMgr@@UAE@XZ
    bool IsFinished() const;           // ?IsFinished@AudioBankMgr@@QBE_NXZ
    bool AnyBanksLoaded() const;       // ?AnyBanksLoaded@AudioBankMgr@@QBE_NXZ
private:
    const char* LanguageStr(ELanguage id) const;  // ?LanguageStr@AudioBankMgr@@ABEPBDW4ELanguage@@@Z
    void NotifyLoaded();               // ?NotifyLoaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9C0)
    void NotifyUnloaded();             // ?NotifyUnloaded@AudioBankMgr@@AAEXXZ (game.o 0x62B9F0)
    void LoadWbkInternal(WbkEntry& wbk, const char* path, ELanguage lang,
                         bool async);  // ?LoadWbkInternal@AudioBankMgr@@AAEXAAUWbkEntry@1@PBDW4ELanguage@@_N@Z (game.o 0x62BD50)
public:
    void Update();                     // ?Update@AudioBankMgr@@QAEXXZ (game.o 0x62BA20)
    void FinishLoading();              // ?FinishLoading@AudioBankMgr@@QAEXXZ (game.o 0x62BC40)
    void FreeWbk(const tlFixedString& name, bool async);  // ?FreeWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z (game.o 0x62BE30)
    void LoadWbk(const tlFixedString& name, bool async);  // ?LoadWbk@AudioBankMgr@@QAEXABVtlFixedString@@_N@Z (game.o 0x639630)
private:
    virtual void UnloadBank(TPakId pakId);  // ?UnloadBank@AudioBankMgr@@EAEXW4TPakId@@@Z (game.o 0x639550)
public:
    void RegisterWbk(const tlFixedString& name, const char* path,
                     ELanguage lang, TPakId pak);  // game.o 0x621470
};

template <>
ae_sized_array_base<AudioBankMgr::WbkEntry, 16>::ae_sized_array_base()
{
    for (int i = 0; i < 16; ++i)
    {
        unsigned int* words =
            reinterpret_cast<unsigned int*>(&m_elements[i]);
        for (int j = 0; j < 8; ++j)
            words[j] = 0;
        words[8] = NFL_FILE_ID_INVALID;
        for (int j = 9; j < 15; ++j)
            words[j] = 0;
        for (int j = 15; j < 21; ++j)
            words[j] = NFL_FILE_ID_INVALID;
        for (int j = 21; j < 27; ++j)
            words[j] = NFL_FILE_ID_INVALID;
    }
}
AudioBankMgr* AudioBankMgr::sInst = nullptr;
extern void* AudioBankMgr_sInst;

// game.o 0x006600F0
bool AudioBankMgr::AnyBanksLoaded() const
{
    return m_size > 0;
}

// ea: 0x004DC880
void* AudioBankMgr::operator new(size_t size, void* p)
{
    (void)size;
    return p;
}

// ea: 0x004DC890
AudioBankMgr* AudioBankMgr::Inst()
{
    return AudioBankMgr::sInst;
}

// ============================================================================
// AudioBankMgr ctor / RegisterWbk - ea: 0x621440 / 0x621470
// ============================================================================
enum nflMediaID : unsigned;
extern nflFileID nflOpenFile(nflMediaID mediaID, const char* fileName);  // ?nflOpenFile@@YA?AW4nflFileID@@W4nflMediaID@@PBD@Z
extern nflMediaID gNflMediaId;                              // nfl_xboxr
extern void AssetBankSet_ctor(void* self);                 // streamer.o
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                  const char* ctx, const char* file,
                                  int line);
static tlFixedString dflt;          // ?dflt@@3VtlFixedString@@A @ 0xF58C04
static bool s_dflt_init;            // $S13_7

void AudioBankMgr::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::ARO;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\AudioBankManager.h";
        AeAssert::gCurrentLine = 12;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x6CCu, 4, "fx", "c:\\cod\\code\\game\\AudioBankManager.h", 12);
    if (memory != nullptr)
        sInst = new (memory) AudioBankMgr();
    else
        sInst = nullptr;
    AudioBankMgr_sInst = sInst;
}

// ea: 0x004DC9A0
void AudioBankMgr::DeleteInst()
{
    AudioBankMgr* instance = AudioBankMgr::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\AudioBankManager.h";
        AeAssert::gCurrentLine = 12;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
        delete instance;
    AudioBankMgr::sInst = nullptr;
}

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
    nflFileID fileId = nflOpenFile((nflMediaID)gNflMediaId, path);
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
enum nflState : unsigned;
extern nflState codNflUpdate();                // nfl_xboxr
extern void nslUpdateBanks();                  // nsl_xboxr
extern int  nslGetBankState(nslBankID bankID); // nsl_xboxr
extern nslBankID nslLoadBank(unsigned int flags, unsigned int file,
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
    kLanguageCount = 6,
};

ELanguage gLanguage;                      // ?gLanguage@@3W4ELanguage@@A @ 0xF00EA4

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
                    pak = ((PakManagerView*)PakManager::sInst)->mSlots[wbk->pakFile];
                const char* path = (const char*)pak + 0x0C;
                ELanguage v11 = gLanguage;
                if (wbk->fileID[kLanguageUnlocalized] != (nflFileID)-1)
                    this->LoadWbkInternal(*wbk, path,
                                          (ELanguage)kLanguageUnlocalized,
                                          async);
                if (wbk->fileID[v11] == (nflFileID)-1
                    || (this->LoadWbkInternal(*wbk, path, v11, async),
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

// Cross-module bridges used by common.o/cl.o/sv.o.  The release call sites
// dispatch to AudioBankMgr::Update/FinishLoading on the singleton instance.
void AudioBankMgr_Update(void* self)
{
    static_cast<AudioBankMgr*>(self)->Update();
}

void AudioBankMgr_FinishLoading(void* self)
{
    static_cast<AudioBankMgr*>(self)->FinishLoading();
}

// ea: 0x0062BD50
void AudioBankMgr::LoadWbkInternal(WbkEntry& wbk, const char* path,
                                   ELanguage lang, bool async)
{
    if (wbk.fileID[lang] == (nflFileID)-1
        || wbk.bankId[lang] != NSL_BANK_ID_INVALID)
    {
        this->mDoLoadNotify = true;
    }
    else
    {
        const char* v6 = this->LanguageStr(lang);
        tlPrintf("[wbk] loading wbk [%s]: %s\n",
                 (const char*)&wbk + 4, v6);
        nslBankID Bank = nslLoadBank(0, (unsigned int)wbk.fileID[lang], 0);
        wbk.bankId[lang] = Bank;
        wbk.state[lang] = kLoading;
        if (Bank == NSL_BANK_ID_INVALID)
        {
            this->mDoLoadNotify = true;
            nflCloseFile(wbk.fileID[lang]);
            wbk.fileID[lang] = (nflFileID)-1;
            wbk.state[lang] = kUnloaded;
        }
        else if (!async)
        {
            if (nslGetBankState(Bank) == 1)
            {
                do
                    this->Update();
                while (nslGetBankState(wbk.bankId[lang]) == 1);
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
                                   const float* const pos);  // nsl_xboxr
extern void nslListenerSetOrientation(unsigned int listenerIndex,
                                      const float* const frt,
                                      const float* const top);  // nsl_xboxr
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
extern void nslSetListenerPosition(const float* const pos);        // ?nslSetListenerPosition@@YAXQBM@Z
extern void nslSetListenerOrientation(const float* const a,
                                      const float* const b);       // ?nslSetListenerOrientation@@YAXQBM0@Z
extern void AnglesToAxis(const float* const angles,
                         float (*const axis)[3]);  // core.o
struct nslInitParams { unsigned maxSources; unsigned maxEmitters; unsigned aramBase; unsigned aramSize; };
extern int nslInit(const nslInitParams* ip);                 // ?nslInit@@YAHPBUnslInitParams@@@Z
extern nslInitParams nsl_initParams;                         // ?nsl_initParams (nsl.o @ 0xE4B680)
extern void nslGetInitParams(nslInitParams* ip);              // ?nslGetInitParams@@YAXPAUnslInitParams@@@Z
extern void nslStart(void* work);                            // ?nslStart@@YAXPAX@Z
extern void nslExit();                                       // ?nslExit@@YAXXZ
extern const char* nslGetWaveGroup(nslWaveID wave);          // ?nslGetWaveGroup@@YAPBDW4nslWaveID@@@Z
extern float nslGetWaveParam(nslWaveID wave, int b, float c);// nsl
extern nslSourceID nslNewSource(nslWaveID wave, int mImportance);  // ?nslNewSource@@YA?AW4nslSourceID@@W4nslWaveID@@H@Z
extern void nslQueueSource(nslSourceID sid);                 // ?nslQueueSource@@YAXW4nslSourceID@@@Z
extern "C" unsigned int AeHash(const char* str);                 // ae_hash.cpp
extern int currCl;                                           // ?currCl@@3HA @ 0xF1579C
extern DbLinkedHandle<EntityHandleDb, Entity> g_SoundOnlyPlay;  // ?g_SoundOnlyPlay@@3V?$DbLinkedHandle@VEntityHandleDb@@VEntity@@@@A @ 0xDEB5B4
extern void DebugRender_AddRenderer(void* self, void (*fp)());  // ?AddRenderer@DebugRender@@QAEXP6AXXZ@Z (render.o)
extern void* DebugRender_sInst;  // ?sInst@DebugRender@@2V1@A @ 0xF74D20

// ea: 0x006024A0
nslBankID SoundDevice::SyncLoadBank(const char* filename)
{
    nflFileID v2 = (nflFileID)nflOpenFile((nflMediaID)gNflMediaId, filename);
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
    nslBankID Bank = nslLoadBank(0, (unsigned int)v2, 0);
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
    DebugRender_AddRenderer(DebugRender_sInst,
                            &SoundDevice::SingletonDebugRender);
    this->mNumberOfListeners = 1;
    nslSetNumberOfListeners(1);
    *(unsigned int*)&this->mNslParams[4] = 1;
    *(unsigned int*)this->mNslParams = 512;
    nslGetInitParams(reinterpret_cast<nslInitParams*>(this->mNslParams));
    void* v5 = mem_heap_malloc(nslInit((const nslInitParams*)this->mNslParams));
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

// ea: 0x004E26C0
void SoundDevice::DeleteInst()
{
    SoundDevice* instance = SoundDevice::sInst;
    if (instance == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\SoundDevice.h";
        AeAssert::gCurrentLine = 50;
        AeAssert::gCurrentExpr = "sInst!=0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton not created!"))
            __debugbreak();
    }
    if (instance != nullptr)
    {
        instance->~SoundDevice();
        mem_heap_free(instance);
    }
    SoundDevice::sInst = nullptr;
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
    const math::Position3& pos, const math::Dir3& vel, bool autoRelease,
    DbLinkedHandle<EntityHandleDb, Entity> entHandle, bool mImportant)
{
    if ((__fpclass(vel.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[2]) & 0x297) != 0
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
            this->SetPosition(pos);
            this->SetVelocity(vel);
            nslSetSourceEffectOn((nslSourceID)this->mSource);
            this->mEntHandle.mVal = entHandle.mHandle.mVal;
        }
    }
}

// ea: 0x00639CE0
void SoundDevice::Sound::Play(
    nslWaveID wave, float vol, float pitch, float minrange, float maxrange,
    const math::Position3& pos, const math::Dir3& vel, bool autoRelease,
    DbLinkedHandle<EntityHandleDb, Entity> entHandle, bool mImportant)
{
    if ((__fpclass(vel.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(vel.v.m128_f32[2]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[0]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[1]) & 0x297) != 0
        || (__fpclass(pos.v.m128_f32[2]) & 0x297) != 0
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
            this->SetPosition(pos);
            this->SetVelocity(vel);
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
            v14->Queue(id, vol, pitch, min, max, pos, vel, autoRelease,
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
            v15->Play(id, vol, pitch, min, max, pos, vel, autoRelease,
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

// ea: 0x006629E0
void SoundDevice::Sound::Release()
{
    Stop();
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

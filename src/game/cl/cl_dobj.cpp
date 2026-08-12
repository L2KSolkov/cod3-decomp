// ============================================================================
// cl_dobj.cpp - DObj/anim + cgame syscall bridge (cl.o cl_cgame.cpp)
// 12 functions, verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "cl_input.h"
#include "cl_console.h"

#include <string.h>

class DObj;

// ============================================================================
// Externs
// ============================================================================
extern void Com_Error(int code, const char* fmt, ...);
extern int com_skelTimeStamp;
extern int bCL_AllowedAllocSkel;
extern int animFrametime;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern char* va(const char* fmt, ...);
extern void SV_SetCheckSum(int checksum);
extern void CG_DrawActiveFrame(int serverTime, int demoPlayback,
                               int cubemapShot, int cubemapSize,
                               int animFrametime);

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// DObj externs (core engine)
// ============================================================================
extern int DObjSkelExists(DObj* obj, int timeStamp);
extern int DObjSkelExistsConst(DObj* obj, int timeStamp);
extern unsigned int DObjGetAllocSkelSize(DObj* obj);

extern void DObjCreateSkel(DObj* obj, char* buf);
extern void j_nullsub_82(DObj* obj, int* partBits);

// ============================================================================
// Renderer syscall externs (re.*)
// ============================================================================
struct refexport_t {
    int LoadWorld(const char* name, int* checksum);
    int RegisterShader(int a1, int a2);
    int RegisterShaderNoMip(int a1, int a2);
    int Text_Width(int a1, int a2, float a3, float a4, int a5);
    int Text_Height(int a1, float a2);
    void Text_Paint(float a1, float a2, int a3, float a4, int a5, int a6,
                    float a7, int a8, int a9);
    void Text_PaintWithCursor(float a1, float a2, int a3, float a4, int a5,
                              int a6, int a7, float a8, float a9, int a10,
                              int a11);
    void ClearScene();
    void AddPolyToScene(int a1, int a2, int a3);
    void AddLightToScene(int a1, float a2, float a3, float a4, float a5);
    void RenderScene(int a1);
    void SetColor(int a1);
    void DrawStretchPic(float a1, float a2, float a3, float a4, float a5,
                        float a6, float a7, float a8, int a9);
};
extern refexport_t re;

// ============================================================================
// DObj / anim wrappers
// ============================================================================

// ea: 0x528960
void CL_DObjCalcAnim()
{
}

// ea: 0x528970
int CL_DObjCreateSkelForBone(DObj* obj, int boneIndex)
{
    (void)boneIndex;
    if (obj == nullptr)
    {
        ASSERT("obj", "c:\\cod\\code\\game\\cl_cgame.cpp", 561);
    }
    if (DObjSkelExists(obj, com_skelTimeStamp) != 0)
        return 1;
    unsigned int AllocSkelSize = DObjGetAllocSkelSize(obj);
    char* v3 = (char*)mem_heap_malloc_ctx(AllocSkelSize, 16, "hunk",
                                          "c:\\cod\\code\\game\\cl_cgame.cpp",
                                          569);
    DObjCreateSkel(obj, v3);
    return 0;
}

// ea: 0x528A10
int CL_DObjCreateSkelForBones(DObj* obj, int* boneMask)
{
    (void)boneMask;
    if (bCL_AllowedAllocSkel == 0)
    {
        ASSERT("bCL_AllowedAllocSkel", "c:\\cod\\code\\game\\cl_cgame.cpp", 586);
    }
    if (obj == nullptr)
    {
        ASSERT("obj", "c:\\cod\\code\\game\\cl_cgame.cpp", 587);
    }
    if (DObjSkelExists(obj, com_skelTimeStamp) != 0)
        return 1;
    unsigned int AllocSkelSize = DObjGetAllocSkelSize(obj);
    char* v3 = (char*)mem_heap_malloc_ctx(AllocSkelSize, 16, "hunk",
                                          "c:\\cod\\code\\game\\cl_cgame.cpp",
                                          595);
    DObjCreateSkel(obj, v3);
    return 0;
}

// ea: 0x528AF0
void CL_DObjCalcSkel(DObj* obj, int* partBits)
{
    if (obj == nullptr)
    {
        ASSERT("obj", "c:\\cod\\code\\game\\cl_cgame.cpp", 608);
    }
    if (DObjSkelExistsConst(obj, com_skelTimeStamp) == 0)
    {
        ASSERT("DObjSkelExistsConst(obj, com_skelTimeStamp)",
               "c:\\cod\\code\\game\\cl_cgame.cpp", 609);
    }
    j_nullsub_82(obj, partBits);
}

// ea: 0x528F80
int CL_SaveViewModelAnimTrees()
{
    return VM_Call(cgvm, 13);
}

// ea: 0x528FA0
int CL_LoadViewModelAnimTrees()
{
    return VM_Call(cgvm, 14);
}

// ea: 0x528FC0
int CL_SaveWeaponInfo()
{
    return VM_Call(cgvm, 15);
}

// ea: 0x528FE0
int CL_LoadWeaponInfo()
{
    return VM_Call(cgvm, 16);
}

// ea: 0x528BA0
void LoadWorld(const char* name)
{
    int checksum;
    re.LoadWorld(name, &checksum);
    SV_SetCheckSum(checksum);
}

// ============================================================================
// Cgame syscall bridge
// ============================================================================

// ea: 0x528BD0
int CL_CgameSystemCalls(int* args)
{
    int result;
    switch (*args)
    {
    case '.':
        result = re.RegisterShader(args[1], args[2]);
        break;
    case '1':
        result = re.Text_Width(args[1], args[2], *(float*)(args + 3), 0.0f,
                               args[4]);
        break;
    case '2':
        result = re.Text_Height(args[1], *(float*)(args + 2));
        break;
    case '3':
        re.Text_Paint(*(float*)(args + 1), *(float*)(args + 2), args[3],
                      *(float*)(args + 4), args[5], args[6],
                      *(float*)(args + 7), args[8], args[9]);
        result = 0;
        break;
    case '4':
        re.Text_PaintWithCursor(*(float*)(args + 1), *(float*)(args + 2),
                                args[3], *(float*)(args + 4), args[5],
                                args[6], args[7], *(float*)(args + 8),
                                0.0f, args[9], args[10]);
        result = 0;
        break;
    case '9':
        re.ClearScene();
        result = 0;
        break;
    case '=':
        re.AddPolyToScene(args[1], args[2], args[3]);
        result = 0;
        break;
    case '>':
        re.AddLightToScene(args[1], *(float*)(args + 2), *(float*)(args + 3),
                           *(float*)(args + 4), *(float*)(args + 5));
        result = 0;
        break;
    case 'B':
        re.RenderScene(args[1]);
        result = 0;
        break;
    case 'E':
        re.SetColor(args[1]);
        result = 0;
        break;
    case 'F':
        re.DrawStretchPic(*(float*)(args + 1), *(float*)(args + 2),
                          *(float*)(args + 3), *(float*)(args + 4),
                          *(float*)(args + 5), *(float*)(args + 6),
                          *(float*)(args + 7), *(float*)(args + 8), args[9]);
        result = 0;
        break;
    case 'T':
        result = re.RegisterShaderNoMip(args[1], args[2]);
        break;
    default:
        if (va("Bad cgame system trap: %i", *args) == nullptr)
        {
            ASSERT("va(\"Bad cgame system trap: %i\", args[0])",
                   "c:\\cod\\code\\game\\cl_cgame.cpp", 1599);
        }
        Com_Error(1, "Bad cgame system trap: %i", *args);
        result = 0;
        break;
    }
    return result;
}

// ea: 0x528E20
struct vm_s* CL_GameCommand()
{
    if (cgvm != nullptr)
        return (struct vm_s*)VM_Call(cgvm, 2);
    return cgvm;
}

// ea: 0x528E40
void CL_CGameRendering()
{
    CG_DrawActiveFrame(cl[currCl].serverTime, 0, 0, 0, animFrametime);
}

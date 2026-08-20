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
enum errorParm_t;
extern void Com_Error(errorParm_t code, const char* fmt, ...);
extern int com_skelTimeStamp;
extern int bCL_AllowedAllocSkel;
extern int animFrametime;
extern struct vm_s { int (__cdecl* systemCall)(int*); }* cgvm;
extern int VM_Call(struct vm_s* vm, int callnum, ...);
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern char* va(const char* fmt, ...);
extern const char defaultFileName[];
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
// Pointer table matching core/common.cpp's `re` layout (cl.o re_export).
struct refexport_t {
    void (*Shutdown)(int);
    void (*BeginRegistration)(void*);
    void* (*RegisterModel)(void* result, const char*, int, int);
    int (*RegisterShader)(const char*, int);
    int (*RegisterShaderNoMip)(const char*, int);
    void (*LoadWorld)(const char*, int*);
    void (*SetFXImageMemory)(int);
    int (*GetFXImageMemory)();
    int (*GetImageMemory)();
    float (*GetFarPlaneDist)();
    void (*EndRegistration)();
    void (*ClearScene)();
    void (*AddPolyToScene)(void*, int, const void*);
    void (*AddLightToScene)(const float*, float, float, float, float);
    void (*SetCullDist)(float);
    void (*SetFog)(int, int, int, float, float, float, float);
    void (*RenderScene)(const void*);
    void (*ClearFlares)();
    void (*SetColor)(const float*);
    void (*DrawStretchPic)(float, float, float, float, float, float, float,
                           float, void*);
    void (*DrawStretchPicGradient)(float, float, float, float, float, float,
                                   float, float, void*, const float*, int);
    void (*DrawStretchPicRotate)(float, float, float, float, float, float,
                                 float, float, float, void*);
    void (*DrawQuadPic)(const float (*)[2], const float (*)[2], void*);
    void (*DrawStretchRaw)(int, int, int, int, int, int,
                           const unsigned char*, int, int);
    void (*UploadCinematic)(int, int, int, int, const unsigned char*, int, int);
    void (*BeginFrame)();
    void (*EndFrame)(int*, int*);
    void (*SaveScreen)();
    void (*TrackStatistics)(void*);
    int (*PickShader)(const float*, const float*, char*, char*, char*, int);
    void (*ResetImageAllocations)();
    void (*FreeImageAllocations)();
    void (*CubemapShot)(const char*, int, int, float, float);
    void (*CubemapWaterShot)(const char*, int, int, float*, float*);
    void (*LocateDebugStrings)(void*, int);
    void (*LocateDebugLines)(void*, int);
    int (*Text_Width)(const char*, int, float, float, int);
    int (*Text_Height)(int, float);
    void (*Text_Paint)(float, float, int, float, const float*, const char*,
                       float, int, int);
    int (*Text_ConsoleWidth)(const short*, int, float, float, int);
    void (*Text_ConsolePaint)(float, float, int, float, const float*,
                              const short*, float, int, int);
    void (*Text_PaintWithCursor)(float, float, int, float, const float*,
                                 const char*, int, char, float, int, int);
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
int CL_DObjCreateSkelForBone(DObj* obj)
{
    if (obj == nullptr)
    {
        ASSERT("obj", "c:\\cod\\code\\game\\cl_cgame.cpp", 561);
    }
    if (DObjSkelExists(obj, com_skelTimeStamp) != 0)
        return 1;
    unsigned int AllocSkelSize = DObjGetAllocSkelSize(obj);
    char* v3 = (char*)mem_heap_malloc_ctx(16, AllocSkelSize, "hunk",
                                          "c:\\cod\\code\\game\\cl_cgame.cpp",
                                          569);
    DObjCreateSkel(obj, v3);
    return 0;
}

// ea: 0x528A10
int CL_DObjCreateSkelForBones(DObj* obj)
{
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
    char* v3 = (char*)mem_heap_malloc_ctx(16, AllocSkelSize, "hunk",
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
        result = re.RegisterShader((const char*)args[1], args[2]);
        break;
    case '1':
        result = re.Text_Width((const char*)args[1], args[2],
                               *(float*)(args + 3), 0.0f, args[4]);
        break;
    case '2':
        result = re.Text_Height(args[1], *(float*)(args + 2));
        break;
    case '3':
        re.Text_Paint(*(float*)(args + 1), *(float*)(args + 2), args[3],
                      *(float*)(args + 4), (const float*)args[5],
                      (const char*)args[6],
                      *(float*)(args + 7), args[8], args[9]);
        result = 0;
        break;
    case '4':
        re.Text_PaintWithCursor(*(float*)(args + 1), *(float*)(args + 2),
                                args[3], *(float*)(args + 4),
                                (const float*)args[5], (const char*)args[6],
                                args[7], (char)*(float*)(args + 8),
                                0.0f, args[9], args[10]);
        result = 0;
        break;
    case '9':
        re.ClearScene();
        result = 0;
        break;
    case '=':
        re.AddPolyToScene((void*)args[1], args[2], (const void*)args[3]);
        result = 0;
        break;
    case '>':
        re.AddLightToScene((const float*)args[1], *(float*)(args + 2),
                           *(float*)(args + 3), *(float*)(args + 4),
                           *(float*)(args + 5));
        result = 0;
        break;
    case 'B':
        re.RenderScene((const void*)args[1]);
        result = 0;
        break;
    case 'E':
        re.SetColor((const float*)args[1]);
        result = 0;
        break;
    case 'F':
        re.DrawStretchPic(*(float*)(args + 1), *(float*)(args + 2),
                          *(float*)(args + 3), *(float*)(args + 4),
                          *(float*)(args + 5), *(float*)(args + 6),
                          *(float*)(args + 7), *(float*)(args + 8),
                          (void*)args[9]);
        result = 0;
        break;
    case 'T':
        result = re.RegisterShaderNoMip((const char*)args[1], args[2]);
        break;
    default:
        if (va("Bad cgame system trap: %i", *args) == nullptr)
        {
            ASSERT("va(\"Bad cgame system trap: %i\", args[0])",
                   "c:\\cod\\code\\game\\cl_cgame.cpp", 1599);
        }
        Com_Error((errorParm_t)1, "Bad cgame system trap: %i", *args);
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

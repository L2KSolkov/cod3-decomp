// ============================================================================
// tr_stats.cpp - render.o profile/debug-stat helpers (tr_main.cpp, xmodel.cpp)
// Types and bodies verified against IDA (codmp_xboxr.xbe).
// ============================================================================

#include "core/math_types.h"

#include <math.h>
#include <stdint.h>

// AeAssert (game.o defines the real symbols; local decls only)
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmtstring, ...);
}

// cdl_proftimer (cdl_base.h; value +0x00)
struct cdl_proftimer {
    uint64_t stamp;        // +0x00
    uint64_t value;        // +0x08
};
struct cdl_profcounter {
    uint64_t value;
};

void cdl_profile_reset();  // ?cdl_profile_reset@@YAXXZ (cdl_base.cpp)

#define CDL_PROFTIMER_EXTERN(name) extern cdl_proftimer cdl_proftimer_##name;
CDL_PROFTIMER_EXTERN(collide_segment)
CDL_PROFTIMER_EXTERN(collide_sphere)
CDL_PROFTIMER_EXTERN(segment_patch)
CDL_PROFTIMER_EXTERN(segment_brush)
CDL_PROFTIMER_EXTERN(traverse)
CDL_PROFTIMER_EXTERN(vsphere_poly)
CDL_PROFTIMER_EXTERN(collide_segment_list)
CDL_PROFTIMER_EXTERN(vsphere_traverse)
CDL_PROFTIMER_EXTERN(vsphere_brush)
CDL_PROFTIMER_EXTERN(vsphere_patch)
CDL_PROFTIMER_EXTERN(trace_point_list)
CDL_PROFTIMER_EXTERN(trace_sphere_list)
CDL_PROFTIMER_EXTERN(proxy_queries)
CDL_PROFTIMER_EXTERN(sight_trace_point)
CDL_PROFTIMER_EXTERN(sight_trace_sphere)
CDL_PROFTIMER_EXTERN(wheel_collision)
CDL_PROFTIMER_EXTERN(temp3)
CDL_PROFTIMER_EXTERN(temp2)
CDL_PROFTIMER_EXTERN(temp1)
CDL_PROFTIMER_EXTERN(temp0)
CDL_PROFTIMER_EXTERN(draw)
CDL_PROFTIMER_EXTERN(audio)
CDL_PROFTIMER_EXTERN(streaming)
CDL_PROFTIMER_EXTERN(pak_mgr)
CDL_PROFTIMER_EXTERN(music_mgr)
CDL_PROFTIMER_EXTERN(rumble_mgr)
CDL_PROFTIMER_EXTERN(scn_effect)
CDL_PROFTIMER_EXTERN(effect_sys)
CDL_PROFTIMER_EXTERN(entities)
CDL_PROFTIMER_EXTERN(drones)
CDL_PROFTIMER_EXTERN(task_sys)
CDL_PROFTIMER_EXTERN(smoke_mgr)
CDL_PROFTIMER_EXTERN(notifies)
CDL_PROFTIMER_EXTERN(subtitles)
CDL_PROFTIMER_EXTERN(aethread)
CDL_PROFTIMER_EXTERN(vmcalls)
CDL_PROFTIMER_EXTERN(scn_anim)
CDL_PROFTIMER_EXTERN(dobj_anim)
CDL_PROFTIMER_EXTERN(ent_actors)
CDL_PROFTIMER_EXTERN(update_rb)
CDL_PROFTIMER_EXTERN(interact)
CDL_PROFTIMER_EXTERN(cvar)
CDL_PROFTIMER_EXTERN(veh_ctrl)
CDL_PROFTIMER_EXTERN(cl_msgs)
CDL_PROFTIMER_EXTERN(ent_advance)
CDL_PROFTIMER_EXTERN(fx_all)
CDL_PROFTIMER_EXTERN(fx_update)
CDL_PROFTIMER_EXTERN(fx_render)
extern cdl_profcounter cdl_profcounter_temp0;  // ?cdl_profcounter_temp0@@3Ucdl_profcounter@@A
extern cdl_profcounter cdl_profcounter_temp1;  // ?cdl_profcounter_temp1@@3Ucdl_profcounter@@A

// ============================================================================
// profile_reset - ea: 0x006C4370
// ============================================================================
void profile_reset()
{
    cdl_profile_reset();
    cdl_proftimer_collide_segment.value = 0;
    cdl_proftimer_collide_sphere.value = 0;
    cdl_proftimer_segment_patch.value = 0;
    cdl_proftimer_segment_brush.value = 0;
    cdl_proftimer_traverse.value = 0;
    cdl_proftimer_vsphere_poly.value = 0;
    cdl_proftimer_collide_segment_list.value = 0;
    cdl_proftimer_vsphere_traverse.value = 0;
    cdl_proftimer_vsphere_brush.value = 0;
    cdl_proftimer_vsphere_patch.value = 0;
    cdl_proftimer_trace_point_list.value = 0;
    cdl_proftimer_trace_sphere_list.value = 0;
    cdl_proftimer_proxy_queries.value = 0;
    cdl_proftimer_sight_trace_point.value = 0;
    cdl_proftimer_sight_trace_sphere.value = 0;
    cdl_proftimer_wheel_collision.value = 0;
    cdl_proftimer_temp3.value = 0;
    cdl_proftimer_temp2.value = 0;
    cdl_proftimer_temp1.value = 0;
    cdl_proftimer_temp0.value = 0;
    cdl_proftimer_draw.value = 0;
    cdl_proftimer_audio.value = 0;
    cdl_proftimer_streaming.value = 0;
    cdl_proftimer_pak_mgr.value = 0;
    cdl_proftimer_music_mgr.value = 0;
    cdl_proftimer_rumble_mgr.value = 0;
    cdl_proftimer_scn_effect.value = 0;
    cdl_proftimer_effect_sys.value = 0;
    cdl_proftimer_entities.value = 0;
    cdl_proftimer_drones.value = 0;
    cdl_proftimer_task_sys.value = 0;
    cdl_proftimer_smoke_mgr.value = 0;
    cdl_proftimer_notifies.value = 0;
    cdl_proftimer_subtitles.value = 0;
    cdl_proftimer_aethread.value = 0;
    cdl_proftimer_vmcalls.value = 0;
    cdl_proftimer_scn_anim.value = 0;
    cdl_proftimer_dobj_anim.value = 0;
    cdl_proftimer_ent_actors.value = 0;
    cdl_proftimer_update_rb.value = 0;
    cdl_proftimer_interact.value = 0;
    cdl_proftimer_cvar.value = 0;
    cdl_proftimer_veh_ctrl.value = 0;
    cdl_proftimer_cl_msgs.value = 0;
    cdl_proftimer_ent_advance.value = 0;
    cdl_proftimer_fx_all.value = 0;
    cdl_proftimer_fx_update.value = 0;
    cdl_proftimer_fx_render.value = 0;
    cdl_profcounter_temp0.value = 0;
    cdl_profcounter_temp1.value = 0;
}

// ============================================================================
// TranslucentStat - ea: 0x006C3AD0..0x006C3B10
// ============================================================================
class TranslucentStat {
public:
    TranslucentStat();
    void StartQuery();
    void EndQuery();
    void RenderDebugStats();
    void Shutdown();
};

TranslucentStat::TranslucentStat()
{
}

void TranslucentStat::StartQuery()
{
}

void TranslucentStat::EndQuery()
{
}

void TranslucentStat::RenderDebugStats()
{
}

void TranslucentStat::Shutdown()
{
}

// ============================================================================
// WheelMarkMgr::Reset - ea: 0x006C38B0
// ============================================================================
class WheelMarkMgr {
protected:
    static unsigned int NMarks;
public:
    static void Reset();
};

unsigned int WheelMarkMgr::NMarks;

void WheelMarkMgr::Reset()
{
    WheelMarkMgr::NMarks = 0;
}

// ============================================================================
// ReadQuat - ea: 0x006C4570
// ============================================================================
void ReadQuat(short* in, short* out)
{
    short v2 = in[0];
    out[0] = in[0];
    short v3 = in[1];
    out[1] = v3;
    short v4 = in[2];
    out[2] = v4;
    int temp = 1073676289 - v4 * v4 - v3 * v3 - v2 * v2;
    short v6;
    if (temp <= 0)
    {
        v6 = 0;
    }
    else
    {
        double v5 = sqrt((double)temp) + 0.5;
        floor(v5);
        v6 = (short)v5;
    }
    if (v6 != v6)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 72;
        AeAssert::gCurrentExpr = "iQ[3] == (short) iQ[3]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    out[3] = v6;
}

// ============================================================================
// ReadQuat2 - ea: 0x006C4640
// ============================================================================
void ReadQuat2(short* in, short* out)
{
    short v2 = in[0];
    out[0] = in[0];
    int temp = 1073676289 - v2 * v2;
    short v4;
    if (temp <= 0)
    {
        v4 = 0;
    }
    else
    {
        double v3 = sqrt((double)temp) + 0.5;
        floor(v3);
        v4 = (short)v3;
    }
    if (v4 != v4)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\xmodel.cpp";
        AeAssert::gCurrentLine = 89;
        AeAssert::gCurrentExpr = "iQ[1] == (short) iQ[1]";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    out[1] = v4;
}

// ============================================================================
// shell_util.cpp - FloatingPQ, system_time, controller mapping, SE file list
// ============================================================================

#include "game/shell/shell_types.h"
#include "ngl/ngl_scene.h"

#include <string>

extern int giFilesFound;  // ?giFilesFound@@3HA @ 0xF30AF0
extern float sNaN;        // ?sNaN@@3MA @ 0x10F19D0
void SE_R_ListFiles(const char* psExtension, const char* psDir,
                    std::string* strResults);  // 0x0059BA20 (stub until ported)

// ============================================================================
// FloatingPQ - projected quad
// ============================================================================

// ea: 0x00591070
FloatingPQ::FloatingPQ(char* n) : PanelQuad(n)
{
    location_3d.x = sNaN;
    location_3d.y = sNaN;
    location_3d.z = sNaN;
    location_3d.x = 0.0f;
    location_3d.y = 0.0f;
    location_3d.z = 0.0f;
}

// ea: 0x005B76B0
void FloatingPQ::SetLocation3D(Broc::vector l)
{
    location_3d = l;
}

// ea: 0x005B76E0
Broc::vector FloatingPQ::GetLocation3D()
{
    return location_3d;
}

// ea: 0x005B7710
void FloatingPQ::CopyFrom(const FloatingPQ* pq)
{
    PanelQuad::CopyFrom(pq);
    location_3d = pq->location_3d;
}

// ea: 0x0056AD40
void FloatingPQ::UpdateInScene()
{
    math::Position3 local;
    local.v.m128_f32[0] = location_3d.x;
    local.v.m128_f32[1] = location_3d.y;
    local.v.m128_f32[2] = location_3d.z;
    local.v.m128_f32[3] = 0.0f;
    math::Position3 projected;
    nglProjectPoint(&projected, &local, nglBuildScene);
    SetCenterPos(projected.v.m128_f32[0], projected.v.m128_f32[1]);
}

// ============================================================================
// system_time
// ============================================================================

// ea: 0x005755C0
bool system_time::equals(system_time st)
{
    return year == st.year
        && month == st.month
        && day == st.day
        && hour == st.hour
        && minute == st.minute
        && second == st.second;
}

// ea: 0x00575620
bool system_time::newer_than(system_time st)
{
    if (year < st.year)
        return false;
    if (year > st.year)
        return true;
    if (month < st.month)
        return false;
    if (month > st.month)
        return true;
    if (day < st.day)
        return false;
    if (day > st.day)
        return true;
    if (hour < st.hour)
        return false;
    if (hour > st.hour)
        return true;
    if (minute < st.minute)
        return false;
    if (minute > st.minute)
        return true;
    if (second < st.second)
        return false;
    return second > st.second;
}

// ============================================================================
// controller button map
// ============================================================================

// ea: 0x005713D0
controller::ButtonIndex mapButton(int button)
{
    if (button > 128)
    {
        if (button > 2048)
        {
            if (button == 4096)
                return controller::L2;
            if (button == 0x2000)
                return controller::R2;
        }
        else
        {
            switch (button)
            {
            case 2048:
                return controller::R1;
            case 256:
                return controller::SQUARE;
            case 512:
                return controller::TRIANGLE;
            case 1024:
                return controller::L1;
            default:
                break;
            }
        }
        return (controller::ButtonIndex)(controller::R3
                                         | controller::RIGHTBUTTON);
    }
    if (button == 128)
        return controller::CIRCLE;
    switch (button)
    {
    case 1:
        return controller::SELECT;
    case 4:
        return controller::UPBUTTON;
    case 8:
        return controller::DOWNBUTTON;
    case 16:
        return controller::LEFTBUTTON;
    case 32:
        return controller::RIGHTBUTTON;
    case 64:
        return (controller::ButtonIndex)(controller::SQUARE
                                         | controller::DOWNBUTTON);
    default:
        return (controller::ButtonIndex)(controller::R3
                                         | controller::RIGHTBUTTON);
    }
}

// ============================================================================
// StringEd file listing
// ============================================================================

// ea: 0x0059BB30
int SE_BuildFileList(const char* psStartDir, std::string& strResults)
{
    giFilesFound = 0;
    strResults.assign("", 0);
    SE_R_ListFiles(".str", psStartDir, &strResults);
    return giFilesFound;
}

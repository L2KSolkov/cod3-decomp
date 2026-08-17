// ============================================================================
// g_vehicle_path.cpp - vehicle path helpers (g.o)
// ============================================================================

#include "game/logic/g_local.h"

// Signed extraction of vehicle_node_t::nextIdx, a 14-bit packed field from
// the IDA local type at +0x3c.
static int VehiclePathNextNode(const vehicle_node_t* node)
{
    return (node->nextIdx << 18) >> 18;
}

// ea: 0x004521F0
void VP_GetLookAheadXYZ(const vehicle_pathpos_t* vpp, float* lookXYZ)
{
    if (vpp->frac > 1.0f)
    {
        AeAssert::gCurrentAuthor = AeAssert::JRS;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_vehicle_path.cpp";
        AeAssert::gCurrentLine = 766;
        AeAssert::gCurrentExpr = "vpp->frac <= 1.0f";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("bad frac"))
            __debugbreak();
    }

    vehicle_node_t* node = s_nodes[vpp->nodeIdx];
    const int nextNode = VehiclePathNextNode(node);
    float distance;

    if (nextNode >= 0)
    {
        distance = ((s_nodes[nextNode]->speed - node->speed) * vpp->frac
                    + node->speed) * vpp->lookAhead
                 + node->length * vpp->frac;

        if (s_numNodes > 0)
        {
            int count = 0;
            while (true)
            {
                const int nodeNext = VehiclePathNextNode(node);
                ++count;
                if (nodeNext < 0)
                {
                    distance = 0.0f;
                    break;
                }

                const float length = node->length;
                if (length == 0.0f)
                {
                    distance = 0.0f;
                    break;
                }

                if (length <= distance)
                {
                    node = s_nodes[nodeNext];
                    distance -= length;
                    if (count < s_numNodes)
                        continue;
                }
                break;
            }
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = static_cast<AeAssert::ECoderId>(0);
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\g_vehicle_path.cpp";
        AeAssert::gCurrentLine = 774;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Vehicle node index -1 : thats bad"))
        {
            __debugbreak();
        }
        distance = 10.0f;
    }

    lookXYZ[0] = node->dir[0] * distance + node->origin[0];
    lookXYZ[1] = node->dir[1] * distance + node->origin[1];
    lookXYZ[2] = node->dir[2] * distance + node->origin[2];
}

// ============================================================================
// g_checkpoint.cpp - game.o CheckpointMgr helpers (checkpointmgr.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <string.h>

// ea: 0x00609240
void CheckpointMgr::RestorePlayerHealth()
{
    Entity* Player = EntityManager::sInst->GetPlayer(currCl);
    if (Player != nullptr && Player->client != nullptr)
    {
        if (this->mCheckpointSaveExists && this->mUsingCheckpoints)
        {
            int minHealth[3];
            minHealth[0] = (int)(0.75 * Player->client->ps.stats[2]);
            minHealth[1] = (int)(0.5 * Player->client->ps.stats[2]);
            minHealth[2] = (int)(0.25 * Player->client->ps.stats[2]);
            int v6 = minHealth[g_gameskill->integer];
            if (this->mPlayerHealth < v6)
            {
                Player->client->ps.stats[0] = v6;
                Player->health = v6;
                return;
            }
            Player->client->ps.stats[0] = this->mPlayerHealth;
            Player->health = this->mPlayerHealth;
        }
        else
        {
            Player->client->ps.stats[0] = Player->client->ps.stats[2];
            Player->health = Player->client->ps.stats[2];
        }
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
        AeAssert::gCurrentLine = 157;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning(
                "INVALID/NON_PLAYER ENTITY PASSED INTO RestorePlayerHealth"))
            __debugbreak();
    }
}

// ea: 0x00609330
bool CheckpointMgr::Restart()
{
    this->mTimeRemainingForHudText = 0.0f;
    return true;
}

// ea: 0x00609340
void CheckpointMgr::SetTosserValues()
{
}

// ea: 0x00609350
void CheckpointMgr::RestoreScriptExploders()
{
    if (this->mUsingCheckpoints && this->mCheckpointSaveExists)
        memcpy(this->mCurrentScriptExploded, this->mCheckpointScriptExploded,
               sizeof(this->mCurrentScriptExploded));
}

// ea: 0x00609380
void CheckpointMgr::SetCheckpointCvar()
{
    Broc::string::Block* mBlock = this->mEvent.mBlock;
    if (mBlock != nullptr)
        Cvar_Set("checkpoint", (const char*)&mBlock[1]);
    else
        Cvar_Set("checkpoint", defaultFileName);
}

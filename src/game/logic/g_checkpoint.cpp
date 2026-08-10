// ============================================================================
// g_checkpoint.cpp - game.o CheckpointMgr helpers (checkpointmgr.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdlib.h>
#include <string.h>

extern void* tlMemAlloc(unsigned size, unsigned align, unsigned flags);
extern void  tlMemFree(void* ptr);

// ============================================================================
// CheckpointVector<SCheckpointGameVar> - ae_vector COMDATs (game.o inlines)
// Verified against IDA (construct_array 0x65E970, push_back 0x660AA0,
// erase 0x65D6D0, resize 0x661190, clear 0x661EC0)
// ============================================================================
template <typename T>
static T* CheckpointVectorConstruct(int iCapacity, int iSize)
{
    T* p = (T*)tlMemAlloc(iCapacity * sizeof(T), 8, 0);
    for (T* q = p + iSize; q != p + iCapacity; ++q)
        memset(q, 0, sizeof(T));
    return p;
}

template <typename T>
static void CheckpointVectorResize(CheckpointVector<T>* v, int iNewSize)
{
    if (iNewSize > v->mCapacity)
    {
        T* newElements = CheckpointVectorConstruct<T>(iNewSize, 0);
        for (int i = 0; i < v->mSize; ++i)
            newElements[i] = v->mElements[i];
        if (v->mElements != nullptr)
        {
            tlMemFree(v->mElements);
            v->mElements = nullptr;
            v->mCapacity = 0;
        }
        v->mElements = newElements;
        v->mCapacity = iNewSize;
    }
    else if (iNewSize > v->mSize)
    {
        for (T* q = v->mElements + v->mSize; q != v->mElements + iNewSize;
             ++q)
            memset(q, 0, sizeof(T));
    }
    v->mSize = iNewSize;
}

template <typename T>
static void CheckpointVectorPushBack(CheckpointVector<T>* v, const T& iElement)
{
    if (v->mSize >= v->mCapacity)
    {
        int newCapacity = v->mSize + 4;
        if (v->mSize <= 3)
            newCapacity = v->mSize + 1;
        T* newElements = CheckpointVectorConstruct<T>(newCapacity, v->mSize + 1);
        for (int i = 0; i < v->mSize; ++i)
            newElements[i] = v->mElements[i];
        if (v->mElements != nullptr)
        {
            tlMemFree(v->mElements);
            v->mElements = nullptr;
            v->mCapacity = 0;
        }
        v->mElements = newElements;
        v->mCapacity = newCapacity;
    }
    v->mElements[v->mSize] = iElement;
    ++v->mSize;
}

template <typename T>
static void CheckpointVectorErase(CheckpointVector<T>* v, T* iBeginErase,
                                  T* iEndErase)
{
    if (v->mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 286;
        AeAssert::gCurrentExpr = "mSize > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Can't erase in empty vector"))
            __debugbreak();
    }
    T* end = v->mElements + v->mSize;
    T* dst = iBeginErase;
    if (iBeginErase != end)
    {
        T* src = iEndErase;
        while (src != end)
            *dst++ = *src++;
    }
    v->mSize -= (int)(iEndErase - iBeginErase);
}

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

// ea: 0x00618120
void CheckpointMgr::SetEvent(const char* checkpointName)
{
    this->mEvent = checkpointName;
}

// ea: 0x00618130
void CheckpointMgr::RestoreLastCheckpoint()
{
    if (this->mCheckpointSaveExists && this->mUsingCheckpoints)
    {
        Entity* Player = EntityManager::sInst->GetPlayer(currCl);
        memcpy(Player->client->ps.ammo, this->ammo, sizeof(Player->client->ps.ammo));
        memcpy(Player->client->ps.ammoclip, this->ammoclip,
               sizeof(Player->client->ps.ammoclip));
        Client* client = Player->client;
        client->ps.weapons[0] = this->weapons[0];
        client->ps.weapons[1] = this->weapons[1];
        memcpy(Player->client->ps.weaponslots, this->weaponslots,
               sizeof(this->weaponslots));
        Client* v5 = Player->client;
        v5->ps.weaponrechamber[0] = this->weaponrechamber[0];
        v5->ps.weaponrechamber[1] = this->weaponrechamber[1];
        int weapon = this->weapon;
        int v7 = 0;
        int v8 = 0;
        if (weapon == 0
            || ((1 << (this->weapon & 0x1F)) & this->weapons[weapon >> 5]) == 0)
        {
            if (this->weapons[0] != 0)
            {
                v7 = this->weapons[0];
            }
            else if (this->weapons[1] != 0)
            {
                v7 = this->weapons[1];
                v8 = 32;
            }
            else
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
                AeAssert::gCurrentLine = 793;
                AeAssert::gCurrentExpr = "0";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "Checkpoint couldn't restore weapon - Weapon index of 0...  Missing weapon about to occur"))
                    __debugbreak();
            }
            int v9 = 0;
            while (((1 << v9) & v7) == 0)
            {
                if (++v9 > 31)
                    goto LABEL_17;
            }
            this->weapon = v8 + v9;
        }
    LABEL_17:
        BG_SelectWeaponIndex(this->weapon, currCl);
        this->RestorePlayerHealth();
    }
}

// ea: 0x006182C0
bool CheckpointMgr::GetGameVar(unsigned int hashVarName, unsigned int* val,
                               unsigned int dataSize)
{
    if (dataSize == 0)
        return false;
    SCheckpointGameVar* mElements = this->mGameVars.mElements;
    SCheckpointGameVar* v6 = &mElements[this->mGameVars.mSize];
    if (mElements == v6)
        return false;
    while (mElements->mHashVarName == 0
           || mElements->mHashVarName != hashVarName)
    {
        if (++mElements == v6)
            return false;
    }
    *val = mElements->mVal;
    unsigned int v7 = 1;
    if (dataSize <= 1)
        return true;
    while (1)
    {
        unsigned int mHashVarName = mElements[1].mHashVarName;
        ++mElements;
        if (mHashVarName != 0)
            break;
        val[v7++] = mElements->mVal;
        if (v7 >= dataSize)
            return true;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\checkpointmgr.cpp";
    AeAssert::gCurrentLine = 891;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(
            "ATTEMPTED TO RETRIEVE GAME VARIABLE AS TYPE LARGER THAN IT WAS SAVED AS!!!"))
        __debugbreak();
    return false;
}

// ea: 0x00622290
void CheckpointMgr::SetGameVar(unsigned int hashVarName, unsigned int* val,
                               unsigned int dataSize)
{
    CheckpointVector<SCheckpointGameVar>* p_mGameVars = &this->mGameVars;
    int mSize = this->mGameVars.mSize;
    SCheckpointGameVar var;
    var.mVal = *val;
    var.mHashVarName = hashVarName;
    var.mDataSize = dataSize;
    SCheckpointGameVar* mElements = p_mGameVars->mElements;
    SCheckpointGameVar* v7 = &p_mGameVars->mElements[mSize];
    if (p_mGameVars->mElements != v7)
    {
        while (mElements->mHashVarName != hashVarName)
        {
            if (++mElements == v7)
                goto LABEL_6;
        }
        CheckpointVectorErase(p_mGameVars, mElements,
                              &mElements[mElements->mDataSize]);
    }
LABEL_6:
    CheckpointVectorPushBack(p_mGameVars, var);
    unsigned int v9 = 1;
    if (dataSize > 1)
    {
        var.mHashVarName = 0;
        var.mDataSize = 0;
        do
        {
            var.mVal = val[v9];
            CheckpointVectorPushBack(p_mGameVars, var);
            ++v9;
        } while (v9 < dataSize);
    }
}

// ea: 0x00622330
bool CheckpointMgr::ExploderCheckpointExploded(int exploderId)
{
    unsigned short* mElements =
        (unsigned short*)this->mCurrentScriptExploded;
    int mSize = *(int*)(this->mCurrentScriptExploded + 0x200);
    if (mSize == 0)
        return false;
    for (int i = 0; i < mSize; ++i)
    {
        if (mElements[i] == exploderId)
            return true;
    }
    return false;
}

// ea: 0x00622370
bool CheckpointMgr::PrecludeExploderPiece(const char* exploderType,
                                          int exploderId)
{
    if (exploderType == nullptr || *exploderType == 0)
        return false;
    return this->ExploderCheckpointExploded(exploderId)
        && (strcmp(exploderType, "exploder_swap_out") == 0
            || strcmp(exploderType, "exploder_piece") == 0
            || strcmp(exploderType, "exploder_piece_visible") == 0);
}

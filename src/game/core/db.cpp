// ============================================================================
// db.cpp - DbQuery / DbQueryResults (core.o DbQuery.cpp)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdlib.h>

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

// DbFieldSet::Clear
extern void DbFieldSet_Clear(DbFieldSet* self);

// ea: 0x004C0A70
DbRow* DbQueryResults::GetRandomResult()
{
    unsigned int m_size = mMatches.size();
    if (m_size == 0)
        return nullptr;
    unsigned int v4 = rand();
    return mMatches[v4 % m_size];
}

// ea: 0x004C0AA0
DbRow* DbQueryResults::GetRandomResultSpecific()
{
    unsigned int m_size = mMatchesSpec.size();
    if (m_size == 0)
        return nullptr;
    unsigned int v4 = rand();
    return mMatchesSpec[v4 % m_size];
}

// ea: 0x004C0AD0
DbRow* DbQueryResults::GetResult(unsigned int idx)
{
    int result = mMatches.size();
    if (result != 0)
    {
        if (idx >= (unsigned int)result)
        {
            ASSERT("idx < (unsigned)mMatches.size()",
                   "c:\\cod\\code\\game\\DbQuery.cpp", 400);
        }
        return mMatches[idx];
    }
    return nullptr;
}

// ea: 0x004C4550
void DbQuery::ResetConstraints()
{
    mConstraintPos = 0;
    DbFieldSet_Clear(&mConstraints);
    mAutomaticFail = false;
}

// ea: 0x004C4570
void DbQuery::AcceptMatchingLeaf(DbGraphNode* node, DbQueryResults* results)
{
    if ((node->mFieldId & 0x8000) == 0)
    {
        ASSERT("node->IsLeaf()", "c:\\cod\\code\\game\\DbQuery.cpp", 48);
    }
    // leaf hits live in the node's attachment array; walk them
    const unsigned short* hits = (const unsigned short*)&node->mAttachments[0];
    unsigned short NumHits = hits[1];
    if (NumHits != 0)
    {
        DbRow* const* hitRows = (DbRow* const*)hits;
        for (int i = NumHits; i != 0; --i)
        {
            int mColUsedNum = (*hitRows)->mColUsedNum;
            int mMaxNumFields = results->mMaxNumFields;
            if (mColUsedNum >= mMaxNumFields)
            {
                if (mColUsedNum > mMaxNumFields)
                {
                    results->mMaxNumFields = mColUsedNum;
                    results->mMatchesSpec.m_size = 0;
                }
                results->mMatchesSpec.push_back(*hitRows);
            }
            results->mMatches.push_back(*hitRows);
            ++hitRows;
        }
    }
}

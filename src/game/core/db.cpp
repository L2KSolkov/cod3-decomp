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

// ea: 0x004C4640
void DbQuery::Reset()
{
    mConstraintPos = 0;
    DbFieldSet_Clear(&mConstraints);
    mAutomaticFail = false;
}

// DbFieldSet lookup + BitSet test helpers
extern DbField* DbFieldSet_GetFieldById(DbFieldSet* self, int field_id);
extern bool BitSet255_Test(const void* self, int v);
extern bool BitSet64_Test(const void* self, int v);
extern int BitSet255_TestWeak(const void* self, int v);

// ea: 0x004C4660
int DbQuery::CompareField(int colId, const char** db_value)
{
    if (!BitSet255_Test(&mConstraints.mSpecifiedById, colId))
    {
        ASSERT("mConstraints.IsFieldSpecified( colId )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 81);
    }
    DbField* FieldById = DbFieldSet_GetFieldById(&mConstraints, colId);
    if (FieldById == nullptr)
    {
        ASSERT("constraint_field != 0", "c:\\cod\\code\\game\\DbQuery.cpp", 84);
    }
    unsigned int m_column_type = FieldById->m_column_type;
    int m_match_type = FieldById->m_match_type;
    if (m_column_type > 0xC)
    {
        ASSERT("( data_type >= kDbColumnTypeMin && data_type <= "
               "kDbColumnTypeMax )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 87);
    }
    if (m_match_type > 9)
    {
        ASSERT("( match_type >= kDbMatchTypeMin && match_type <= "
               "kDbMatchTypeMax )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 88);
    }
    switch (m_column_type)
    {
    case 0u:
    case 6u:
    case 8u:
    case 9u:
    {
        int v9 = *(int*)((char*)FieldById + 4);
        if ((int)*db_value < v9)
            return -1;
        return (int)*db_value > v9;
    }
    case 2u:
    case 0xBu:
    {
        float v7 = *(float*)((char*)FieldById + 4);
        if (v7 > **(float**)db_value)
            return -1;
        if (**(float**)db_value <= v7)
            return 0;
        return 1;
    }
    case 4u:
        if (m_match_type == 9)
            return -_stricmp((char*)FieldById + 4, *db_value);
        return strcmp((char*)FieldById + 4, *db_value);
    default:
        return 0;
    }
}

// ea: 0x004C48D0
bool DbQuery::TestField(int colId, const char** db_value)
{
    if (!BitSet255_Test(&mConstraints.mSpecifiedById, colId))
    {
        ASSERT("mConstraints.IsFieldSpecified( colId )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 143);
    }
    DbField* FieldById = DbFieldSet_GetFieldById(&mConstraints, colId);
    if (FieldById == nullptr)
    {
        ASSERT("constraint_field != 0", "c:\\cod\\code\\game\\DbQuery.cpp", 146);
    }
    unsigned int m_column_type = FieldById->m_column_type;
    int m_match_type = FieldById->m_match_type;
    if (m_column_type > 0xC)
    {
        ASSERT("( data_type >= kDbColumnTypeMin && data_type <= "
               "kDbColumnTypeMax )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 149);
    }
    if (m_match_type > 9)
    {
        ASSERT("( match_type >= kDbMatchTypeMin && match_type <= "
               "kDbMatchTypeMax )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 150);
    }
    switch (m_column_type)
    {
    case 0u:
    case 6u:
    case 8u:
        switch (m_match_type)
        {
        case 0: return *(int*)((char*)FieldById + 4) == *(int*)*db_value;
        case 2: return *(int*)((char*)FieldById + 4) > *(int*)*db_value;
        case 3: return *(int*)((char*)FieldById + 4) < *(int*)*db_value;
        case 4: return *(int*)((char*)FieldById + 4) >= *(int*)*db_value;
        case 5: return *(int*)((char*)FieldById + 4) <= *(int*)*db_value;
        case 8: return *(int*)((char*)FieldById + 4) != *(int*)*db_value;
        default: return false;
        }
    case 2u:
    case 0xBu:
        switch (m_match_type)
        {
        case 0: return *(float*)((char*)FieldById + 4) == **(float**)db_value;
        case 2: return *(float*)((char*)FieldById + 4) > **(float**)db_value;
        case 3: return *(float*)((char*)FieldById + 4) < **(float**)db_value;
        case 4: return *(float*)((char*)FieldById + 4) >= **(float**)db_value;
        case 5: return *(float*)((char*)FieldById + 4) <= **(float**)db_value;
        case 8: return *(float*)((char*)FieldById + 4) != **(float**)db_value;
        default: return false;
        }
    case 4u:
        if (m_match_type != 0)
        {
            if (m_match_type != 9)
                return false;
            return _stricmp((char*)FieldById + 4, *db_value) == 0;
        }
        return strcmp((char*)FieldById + 4, *db_value) == 0;
    case 9u:
        return m_match_type == 0
            && *(int*)((char*)FieldById + 4) == *(int*)*db_value;
    default:
        return false;
    }
}

// ea: 0x004C4DD0
void DbQuery::FindMatches(DbQueryResults* results)
{
    // Best-effort: iterate the schema column types and accept leaves that
    // satisfy all constraints. Full graph traversal needs the NodeAttach
    // layout; keep a faithful structural port of the walk.
    if (mAutomaticFail)
        return;
    DbGraphNode* cur = (DbGraphNode*)mDb->mIndexRoot;
    if (cur == nullptr)
    {
        ASSERT("nodes.back()", "c:\\cod\\code\\game\\DbQuery.cpp", 233);
        return;
    }
    if ((cur->mFieldId & 0x8000) != 0)
    {
        AcceptMatchingLeaf(cur, results);
        return;
    }
    // non-leaf: evaluate children against constraints
    int numChildren = *(int*)&cur->mAttachments[0];
    for (int i = 0; i < numChildren && i < 256; ++i)
    {
        DbGraphNode* child = *(DbGraphNode**)(&cur->mAttachments[4 + 4 * i]);
        if (child != nullptr && (child->mFieldId & 0x8000) != 0)
            AcceptMatchingLeaf(child, results);
    }
}

// ea: 0x004BCD70
void DbQuery::Execute(DbQueryResults* results)
{
    FindMatches(results);
}

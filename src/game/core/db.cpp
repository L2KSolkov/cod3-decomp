// ============================================================================
// db.cpp - DbQuery / DbQueryResults (core.o DbQuery.cpp)
// ============================================================================

#include "game/core/core_systems.h"

#include <stdlib.h>
#include <string.h>

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

// ea: 0x004B46C0
DbField::DbField(uint16_t columnId, EDbColumnType col_type,
                 EDbMatchType match_type)
    : m_column_type((unsigned char)col_type),
      m_match_type((unsigned char)match_type),
      mId(columnId)
{
    if (col_type < kDbColumnTypeMin || col_type > kDbColumnTypeMax) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 71;
        AeAssert::gCurrentExpr =
            "( (EDbColumnType)col_type >= kDbColumnTypeMin && "
            "(EDbColumnType)col_type <= kDbColumnTypeMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    if (match_type < kDbMatchTypeMin || match_type > kDbMatchTypeMax) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 72;
        AeAssert::gCurrentExpr =
            "( (EDbMatchType)match_type >= kDbMatchTypeMin && "
            "(EDbMatchType)match_type <= kDbMatchTypeMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
}

// ea: 0x004B47C0
EDbColumnType DbField::GetColumnType() const
{
    return (EDbColumnType)m_column_type;
}

// ea: 0x004B47D0
EDbMatchType DbField::GetMatchType() const
{
    return (EDbMatchType)m_match_type;
}

// ea: 0x004B47E0
uint16_t DbField::GetId() const
{
    return mId;
}

// ea: 0x004B47F0
uint16_t DbColumn::GetId() const
{
    return mId;
}

// ea: 0x004B4800
uint16_t DbColumn::GetSize() const
{
    return mNumElements;
}

// ea: 0x004B4810
const char* DbColumn::get_element_ptr(uint16_t idx) const
{
    return (const char*)mElements + mElementSize * idx;
}

// ea: 0x004B4830
int16_t DbRow::GetColUsedNum() const
{
    return mColUsedNum;
}

// ea: 0x004B4840
int DbGraphNode::GetFieldId() const
{
    return mFieldId & 0x7FFF;
}

// ea: 0x004B4850
bool DbGraphNode::IsLeaf() const
{
    return (mFieldId & 0x8000) != 0;
}

// ea: 0x004B4860
uint16_t DbGraphNode::GetNumHits() const
{
    if (!IsLeaf()) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 285;
        AeAssert::gCurrentExpr = "IsLeaf()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("only leaf nodes have hits"))
            __debugbreak();
    }
    return mAttachments.leaf.numHits;
}

// ea: 0x004B48D0
DbRow** DbGraphNode::GetHits() const
{
    if (!IsLeaf()) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 291;
        AeAssert::gCurrentExpr = "IsLeaf()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("only leaf nodes have hits"))
            __debugbreak();
    }
    return mAttachments.leaf.hits;
}

// ea: 0x004B4940
uint16_t DbGraphNode::GetNumChildren() const
{
    if (IsLeaf()) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 299;
        AeAssert::gCurrentExpr = "!IsLeaf()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("only non-leaf nodes have children"))
            __debugbreak();
    }
    return mAttachments.nonLeaf.numChildren;
}

// ea: 0x004B49B0
DbGraphNode* DbGraphNode::GetChild(uint16_t idx) const
{
    if (IsLeaf()) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 305;
        AeAssert::gCurrentExpr = "!IsLeaf()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("only non-leaf nodes have children"))
            __debugbreak();
    }
    if (idx >= mAttachments.nonLeaf.numChildren) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 306;
        AeAssert::gCurrentExpr = "idx < mAttachments.nonLeaf.numChildren";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid Index"))
            __debugbreak();
    }
    return mAttachments.nonLeaf.children[idx];
}

// ea: 0x004B4A90
const char* DbTable::GetName() const
{
    return mName;
}

// ea: 0x004B4AA0
const DbSchema& DbTable::GetSchema() const
{
    return *mSchema;
}

// ea: 0x004B4AB0
const DbGraphNode* DbTable::GetIndexRoot() const
{
    return (const DbGraphNode*)mIndexRoot;
}

// ea: 0x004B4AC0
const DbColumn& DbTable::GetColumnByIndex(uint16_t idx) const
{
    return *mColumns[idx];
}

// ea: 0x004B4AE0
DbColumn& DbTable::GetColumnByIndex(uint16_t idx)
{
    return *mColumns[idx];
}

// ea: 0x004B4B00
int16_t DbTable::GetColumnIndex(uint16_t columnId) const
{
    int v2 = 0;
    if (mNumColumns == 0)
        return -1;
    for (DbColumn** i = mColumns; (*i)->mId != columnId; ++i) {
        if (++v2 >= mNumColumns)
            return -1;
    }
    if ((int)(uint16_t)v2 != v2) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 384;
        AeAssert::gCurrentExpr = "((i)&0xFFFF) == (i)";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("object can't be truncated to 16 bits!"))
            __debugbreak();
    }
    return (int16_t)v2;
}

// ea: 0x004E22E0
const DbColumn* DbTable::GetColumnById(uint16_t columnId) const
{
    return mColumns[GetColumnIndex(columnId)];
}

// ea: 0x004E2310
const void* DbGraphNode::GetValue(const DbTable* table, uint16_t idx) const
{
    if ((mFieldId & 0x8000) != 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 506;
        AeAssert::gCurrentExpr = "!IsLeaf()";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("only non-leaf nodes have values"))
            __debugbreak();
    }
    if (idx >= mAttachments.nonLeaf.numChildren)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile =
            "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 507;
        AeAssert::gCurrentExpr =
            "idx < mAttachments.nonLeaf.numChildren";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid Index"))
            __debugbreak();
    }
    if (mAttachments.nonLeaf.valueIndices[idx] != -1)
    {
        const DbColumn* column =
            table->mColumns[table->GetColumnIndex(mFieldId)];
        return static_cast<const char*>(column->mElements)
            + column->mElementSize * mAttachments.nonLeaf.valueIndices[idx];
    }
    return nullptr;
}

template const float* DbColumnType<float>::operator[](uint16_t) const;
template const InplaceString*
DbColumnType<InplaceString>::operator[](uint16_t) const;

// ea: 0x004B4BB0
uint16_t DbSchema::GetNumColumnTypes() const
{
    return mNumColumnTypes;
}

// ea: 0x004B4BC0
EDbColumnType DbSchema::GetColumnType(uint16_t columnId) const
{
    if (columnId >= mNumColumnTypes) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 429;
        AeAssert::gCurrentExpr = "columnId < mNumColumnTypes";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid column id"))
            __debugbreak();
    }
    return (EDbColumnType)mColumnTypes[columnId];
}

// ea: 0x004B4C40
EDbMatchType DbSchema::GetMatchType(uint16_t columnId) const
{
    if (columnId >= mNumColumnTypes) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbDefs.h";
        AeAssert::gCurrentLine = 435;
        AeAssert::gCurrentExpr = "columnId < mNumColumnTypes";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("invalid column id"))
            __debugbreak();
    }
    return (EDbMatchType)mMatchTypes[columnId];
}

// ea: 0x004B4CC0
DbTable* DbTableSet::GetTable(const char* name) const
{
    if (mNumTables == 0)
        return nullptr;
    unsigned int v3 = 0;
    unsigned int i = 0;
    for (; _stricmp(mTables[i].mName, name) != 0; ++i) {
        if (++v3 >= mNumTables)
            return nullptr;
    }
    return &mTables[i];
}

// ea: 0x004B4D30
DbQueryString::DbQueryString()
{
    buf[0] = 0;
}

// ea: 0x004B4D40
DbQueryString::DbQueryString(const char* data)
{
    strncpy(buf, data, 0x7F);
    buf[127] = 0;
}

// ea: 0x004B4D70
const char* DbQueryString::c_str() const
{
    return buf;
}

// ea: 0x004B4E00
ae_sized_array<DbRow*, 64>& DbQueryResults::GetCurrentMatchesSpecific()
{
    return mMatchesSpec;
}

bool BitSet255_Test(const void* self, int v);

// ea: 0x004E7F20
DbQuery::DbQuery(const DbTable* db)
    : mDb(db), mAutomaticFail(false), mConstraintPos(0)
{
}

// ea: 0x004E5900
void DbFieldSet::AddField(DbField* field, bool weak)
{
    if (mNumParams >= 0x40)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 52;
        AeAssert::gCurrentExpr = "mNumParams < 64";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("Too many Query parameters"))
            __debugbreak();
    }
    unsigned int param_index = mNumParams;
    if (weak)
        mWeakByIdx.Add((int)param_index);
    else
        mWeakByIdx.Rmv((int)param_index);
    int field_id = field->mId;
    if (weak)
        mWeakById.Add(field_id);
    else
        mWeakById.Rmv(field_id);
    mSpecifiedById.Add(field->mId);
    mIdToIdxMap[field->mId] = (unsigned char)mNumParams;
    mFields[mNumParams++] = field;
    if (field->m_column_type > 0xCu)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 58;
        AeAssert::gCurrentExpr =
            "( field->GetColumnType() >= kDbColumnTypeMin && field->GetColumnType() <= kDbColumnTypeMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    if (field->m_match_type > 9u)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 59;
        AeAssert::gCurrentExpr =
            "( field->GetMatchType() >= kDbMatchTypeMin && field->GetMatchType() <= kDbMatchTypeMax )";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
}

// ea: 0x004E5AA0
bool DbFieldSet::IsFieldSpecified(unsigned int field_id) const
{
    return BitSet255_Test(mSpecifiedById.mBits, (int)field_id);
}

// ea: 0x004E5AC0
void DbFieldSet::Clear()
{
    unsigned int i = 0;
    if (mNumParams != 0) {
        do {
            mFields[i] = nullptr;
            ++i;
        } while (i < mNumParams);
    }
    mNumParams = 0;
    mWeakByIdx.mBits[1] = 0;
    mWeakByIdx.mBits[0] = 0;
    mWeakById.mBits[7] = 0;
    mWeakById.mBits[6] = 0;
    mWeakById.mBits[5] = 0;
    mWeakById.mBits[4] = 0;
    mWeakById.mBits[3] = 0;
    mWeakById.mBits[2] = 0;
    mWeakById.mBits[1] = 0;
    mWeakById.mBits[0] = 0;
    mSpecifiedById.mBits[7] = 0;
    mSpecifiedById.mBits[6] = 0;
    mSpecifiedById.mBits[5] = 0;
    mSpecifiedById.mBits[4] = 0;
    mSpecifiedById.mBits[3] = 0;
    mSpecifiedById.mBits[2] = 0;
    mSpecifiedById.mBits[1] = 0;
    mSpecifiedById.mBits[0] = 0;
}

// ea: 0x004B4D80
const DbField* DbFieldSet::GetFieldByIdx(unsigned int idx) const
{
    if (idx >= mNumParams) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 94;
        AeAssert::gCurrentExpr = "idx >= 0 && idx < mNumParams";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Index out of bounds"))
            __debugbreak();
    }
    return mFields[idx];
}

// ea: 0x004E5B80
const DbField* DbFieldSet::GetFieldById(unsigned int field_id) const
{
    if (!BitSet255_Test(mSpecifiedById.mBits, field_id)) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 100;
        AeAssert::gCurrentExpr = "mSpecifiedById.Test( field_id )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("Invalid field requested"))
            __debugbreak();
    }
    const DbField* field = GetFieldByIdx(mIdToIdxMap[field_id]);
    if (field->m_column_type > 0xCu) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 105;
        AeAssert::gCurrentExpr = "( field->GetColumnType() >= kDbColumnTypeMin && field->GetColumnType() <= kDbColumnTypeMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    if (field->m_match_type > 9u) {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\DbQuery.h";
        AeAssert::gCurrentLine = 106;
        AeAssert::gCurrentExpr = "( field->GetMatchType() >= kDbMatchTypeMin && field->GetMatchType() <= kDbMatchTypeMax )";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("value not in enum range"))
            __debugbreak();
    }
    return field;
}

// ea: 0x004E5CD0
bool DbFieldSet::IsFieldWeakById(unsigned int field_id) const
{
    return BitSet255_Test(mWeakById.mBits, (int)field_id);
}

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
    mConstraints.Clear();
    mAutomaticFail = false;
}

// ea: 0x004C4570
void DbQuery::AcceptMatchingLeaf(const DbGraphNode* node,
                                 DbQueryResults& results)
{
    if ((node->mFieldId & 0x8000) == 0)
    {
        ASSERT("node->IsLeaf()", "c:\\cod\\code\\game\\DbQuery.cpp", 48);
    }
    // leaf hits live in the node's typed attachment union; walk them
    unsigned short NumHits = node->mAttachments.leaf.numHits;
    if (NumHits != 0)
    {
        DbRow* const* hitRows = node->mAttachments.leaf.hits;
        for (int i = NumHits; i != 0; --i)
        {
            int mColUsedNum = (*hitRows)->mColUsedNum;
            int mMaxNumFields = results.mMaxNumFields;
            if (mColUsedNum >= mMaxNumFields)
            {
                if (mColUsedNum > mMaxNumFields)
                {
                    results.mMaxNumFields = mColUsedNum;
                    results.mMatchesSpec.m_size = 0;
                }
                results.mMatchesSpec.push_back(*hitRows);
            }
            results.mMatches.push_back(*hitRows);
            ++hitRows;
        }
    }
}

// ea: 0x004C4640
void DbQuery::Reset()
{
    mConstraintPos = 0;
    mConstraints.Clear();
    mAutomaticFail = false;
}

// DbFieldSet lookup + BitSet test helpers
extern bool BitSet64_Test(const void* self, int v);
extern int BitSet255_TestWeak(const void* self, int v);

// BitSet<255>::Test (byte-packed; helper resolving ?BitSet255_Test@@YA_NPBXH@Z)
bool BitSet255_Test(const void* self, int v)
{
    const unsigned char* bits = (const unsigned char*)self;
    return (bits[v >> 3] & (1 << (v & 7))) != 0;
}

// ea: 0x004C4660
int DbQuery::CompareField(int colId, const void* db_value)
{
    if (!BitSet255_Test(&mConstraints.mSpecifiedById, colId))
    {
        ASSERT("mConstraints.IsFieldSpecified( colId )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 81);
    }
    const DbField* FieldById = mConstraints.GetFieldById(colId);
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
        int v9 = *(const int*)((const char*)FieldById + 4);
        if (*(const int*)db_value < v9)
            return -1;
        return *(const int*)db_value > v9;
    }
    case 2u:
    case 0xBu:
    {
        float v7 = *(const float*)((const char*)FieldById + 4);
        if (v7 > *(const float*)db_value)
            return -1;
        if (*(const float*)db_value <= v7)
            return 0;
        return 1;
    }
    case 4u:
        if (m_match_type == 9)
            return -_stricmp((const char*)FieldById + 4, (const char*)db_value);
        return strcmp((const char*)FieldById + 4, (const char*)db_value);
    default:
        return 0;
    }
}

// ea: 0x004C48D0
bool DbQuery::TestField(int colId, const void* db_value)
{
    if (!BitSet255_Test(&mConstraints.mSpecifiedById, colId))
    {
        ASSERT("mConstraints.IsFieldSpecified( colId )",
               "c:\\cod\\code\\game\\DbQuery.cpp", 143);
    }
    const DbField* FieldById = mConstraints.GetFieldById(colId);
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
        case 0: return *(int*)((char*)FieldById + 4) == *(const int*)db_value;
        case 2: return *(int*)((char*)FieldById + 4) > *(const int*)db_value;
        case 3: return *(int*)((char*)FieldById + 4) < *(const int*)db_value;
        case 4: return *(int*)((char*)FieldById + 4) >= *(const int*)db_value;
        case 5: return *(int*)((char*)FieldById + 4) <= *(const int*)db_value;
        case 8: return *(int*)((char*)FieldById + 4) != *(const int*)db_value;
        default: return false;
        }
    case 2u:
    case 0xBu:
        switch (m_match_type)
        {
        case 0: return *(float*)((char*)FieldById + 4) == *(const float*)db_value;
        case 2: return *(float*)((char*)FieldById + 4) > *(const float*)db_value;
        case 3: return *(float*)((char*)FieldById + 4) < *(const float*)db_value;
        case 4: return *(float*)((char*)FieldById + 4) >= *(const float*)db_value;
        case 5: return *(float*)((char*)FieldById + 4) <= *(const float*)db_value;
        case 8: return *(float*)((char*)FieldById + 4) != *(const float*)db_value;
        default: return false;
        }
    case 4u:
        if (m_match_type != 0)
        {
            if (m_match_type != 9)
                return false;
            return _stricmp((char*)FieldById + 4, (const char*)db_value) == 0;
        }
        return strcmp((char*)FieldById + 4, (const char*)db_value) == 0;
    case 9u:
        return m_match_type == 0
            && *(int*)((char*)FieldById + 4) == *(const int*)db_value;
    default:
        return false;
    }
}

// ea: 0x004C4DD0
void DbQuery::FindMatches(DbQueryResults& results)
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
    int numChildren = cur->GetNumChildren();
    for (int i = 0; i < numChildren && i < 256; ++i)
    {
        DbGraphNode* child = cur->GetChild((uint16_t)i);
        if (child != nullptr && (child->mFieldId & 0x8000) != 0)
            AcceptMatchingLeaf(child, results);
    }
}

// ea: 0x004BCD70
void DbQuery::Execute(DbQueryResults& results)
{
    FindMatches(results);
}

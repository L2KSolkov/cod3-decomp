// ============================================================================
// ui_listbox.cpp - UIListBox + nested item/row/data-row classes (shell.o)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/sv/sv_stubs.h"
#include "game/platform_xbox/XboxLive.h"

#include <math.h>

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern const char defaultFileName[];  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o
extern void tlMemFree(void* ptr);                // core.o
extern void* tlMemAlloc(unsigned int size, unsigned int align,
                        unsigned int flags);     // core.o

extern FEManager g_femanager;

// default highlight colors (shell.o data @ 0xDF3BF4)
static color32 lUIHighlightListBoxDefaultSelectedColor(0xFFFFFFFF);
static color32 lUIHighlightListBoxDefaultUnselectedColor(0x80808080);

// ============================================================================
// UIListBoxData
// ============================================================================

// ea: 0x5AEBB0
UIListBox::UIListBoxData::UIListBoxData()
{
    mState = 0;
    mText = Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x5AEBE0
void UIListBox::UIListBoxData::SetState(int state)
{
    if (mState < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 220;
        AeAssert::gCurrentExpr = "mState >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxItem: State needs to be greater then zero"))
            __debugbreak();
    }
    mState = state;
}

// ea: 0x5AEC70
void UIListBox::UIListBoxData::SetText(const char* text)
{
    mText = text;
}

// ea: 0x5AEC80
void UIListBox::UIListBoxData::ClearText()
{
    mText = defaultFileName;
}

// ea: 0x5AECA0
void UIListBox::UIListBoxData::ClearItem()
{
    mText = defaultFileName;
    SetState(0);
}

// ea: 0x5AECD0
void UIListBox::UIListBoxData::operator=(const UIListBoxData& rhs)
{
    mState = rhs.mState;
    mText = rhs.mText;
}

// ea: 0x5B2580
const char* UIListBox::UIListBoxData::GetText()
{
    return mText.mBlock != nullptr ? (const char*)&mText.mBlock[1]
                                   : defaultFileName;
}

// ============================================================================
// UIListBoxDataRow
// ============================================================================

// ea: 0x5B6160
UIListBox::UIListBoxDataRow::UIListBoxDataRow()
{
    mColumns.mElements = nullptr;
    mColumns.mCapacity = 0;
    mColumns.mSize = 0;
    mColumnCount = 0;
    mEnabled = true;
}

// ea: 0x5AECF0
bool UIListBox::UIListBoxDataRow::IsEnabled()
{
    return mEnabled;
}

// ea: 0x5AED00
void UIListBox::UIListBoxDataRow::SetEnabled(bool enabled)
{
    mEnabled = enabled;
}

// ea: 0x5B2DB0
void UIListBox::UIListBoxDataRow::ClearItem()
{
    for (int i = 0; i < mColumnCount; ++i)
    {
        UIListBoxData* v3 = &mColumns.mElements[i];
        v3->mText = defaultFileName;
        if (v3->mState < 0)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
            AeAssert::gCurrentLine = 220;
            AeAssert::gCurrentExpr = "mState >= 0";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("UIListBoxItem: State needs to be greater then zero"))
                __debugbreak();
        }
        v3->mState = 0;
    }
    mEnabled = true;
}

// ea: 0x5B6180
void UIListBox::UIListBoxDataRow::SetColumnCount(int columns)
{
    if (columns <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 294;
        AeAssert::gCurrentExpr = "columns > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column count must be greater then zero"))
            __debugbreak();
    }
    mColumnCount = columns;
    mColumns.mCapacity = columns;
    mColumns.mSize = columns;
    mColumns.mElements =
        (UIListBoxData*)tlMemAlloc(8 * columns, 8u, 0);
    for (int i = 0; i < columns; ++i)
        new (&mColumns.mElements[i]) UIListBoxData();
}

// ea: 0x5B2BF0
void UIListBox::UIListBoxDataRow::SetItemState(int column, int state)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 300;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetState(state);
}

// ea: 0x5B2C80
void UIListBox::UIListBoxDataRow::SetText(int column, const char* text)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 301;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].mText = text;
}

// ea: 0x5B2D10
const char* UIListBox::UIListBoxDataRow::GetText(int column)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 302;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    return mColumns.mElements[column].GetText();
}

// ============================================================================
// UIListBoxItem
// ============================================================================

// ea: 0x5B4D50
UIListBox::UIListBoxItem::UIListBoxItem()
{
    mType = kTypeNone;
    mStateCount = 1;
    mState = 0;
    mObjects.mElements = nullptr;
    mObjects.mCapacity = 0;
    mObjects.mSize = 0;
    PanelAnimObject* iElement[1] = { nullptr };
    if (mObjects.mSize >= mObjects.mCapacity)
    {
        int v4 = mObjects.mSize + 4;
        if (mObjects.mSize <= 3)
            v4 = mObjects.mSize + 1;
        PanelAnimObject** v5 =
            (PanelAnimObject**)tlMemAlloc(4 * v4, 8u, 0);
        for (int i = 0; i < mObjects.mSize; ++i)
            v5[i] = mObjects.mElements[i];
        if (mObjects.mElements != nullptr)
        {
            tlMemFree(mObjects.mElements);
            mObjects.mElements = nullptr;
            mObjects.mCapacity = 0;
        }
        mObjects.mElements = v5;
        mObjects.mCapacity = v4;
    }
    mObjects.mElements[mObjects.mSize++] = iElement[0];
}

// ea: 0x5B1E30
void UIListBox::UIListBoxItem::SetState(int state)
{
    if (state < 0 || state > mStateCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 159;
        AeAssert::gCurrentExpr = "state >= 0 && state <= mStateCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxItem: State index invalid"))
            __debugbreak();
    }
    this->mState = state;
    for (int v4 = 0; v4 < this->mStateCount; ++v4)
    {
        if (mObjects.mElements[v4] != nullptr)
            mObjects.mElements[v4]->SetShown(false);
    }
    if (mObjects.mElements[state] != nullptr)
        mObjects.mElements[state]->SetShown(true);
}

// ea: 0x5B4DC0
void UIListBox::UIListBoxItem::SetStateCount(int count)
{
    if (count <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 158;
        AeAssert::gCurrentExpr = "count > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    mStateCount = count;
    PanelAnimObject* zero = nullptr;
    if (count > mObjects.mCapacity)
    {
        PanelAnimObject** v3 =
            (PanelAnimObject**)tlMemAlloc(4 * count, 8u, 0);
        for (int i = 0; i < mObjects.mSize; ++i)
            v3[i] = mObjects.mElements[i];
        if (mObjects.mElements != nullptr)
        {
            tlMemFree(mObjects.mElements);
            mObjects.mElements = nullptr;
            mObjects.mCapacity = 0;
        }
        mObjects.mElements = v3;
        mObjects.mCapacity = count;
    }
    for (int i = mObjects.mSize; i < count; ++i)
        mObjects.mElements[i] = zero;
    mObjects.mSize = count;
    mState = 0;
}

// ea: 0x5B4E60
void UIListBox::UIListBoxItem::SetItem(FEText* text, int state)
{
    SetObject(text, state);
    mType = kTypeText;
}

// ea: 0x5B4E90
void UIListBox::UIListBoxItem::SetItem(PanelQuad* quad, int state)
{
    SetObject(quad, state);
    mType = kTypeQuad;
}

// ea: 0x5B4FE0
void UIListBox::UIListBoxItem::operator=(const UIListBoxItem& rhs)
{
    mState = rhs.mState;
    mStateCount = rhs.mStateCount;
    if (mStateCount > mObjects.mCapacity)
    {
        PanelAnimObject** elements =
            (PanelAnimObject**)tlMemAlloc(4 * mStateCount, 8u, 0);
        for (int i = 0; i < mObjects.mSize; ++i)
            elements[i] = mObjects.mElements[i];
        if (mObjects.mElements != nullptr)
        {
            tlMemFree(mObjects.mElements);
            mObjects.mElements = nullptr;
            mObjects.mCapacity = 0;
        }
        mObjects.mElements = elements;
        mObjects.mCapacity = mStateCount;
        mObjects.mSize = mStateCount;
    }
    else
    {
        mObjects.mSize = mStateCount;
    }
    for (int i = 0; i < mStateCount; ++i)
    {
        if (i < 0 || i >= rhs.mObjects.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 161;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        if (i < 0 || i >= mObjects.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mObjects.mElements[i] = rhs.mObjects.mElements[i];
    }
}

// ea: 0x5813A0
void UIListBox::UIListBoxItem::SetText(const char* text)
{
    if (mType == kTypeText && mObjects.mSize > 0
        && mObjects.mElements[0] != nullptr)
    {
        ((FEText*)mObjects.mElements[0])->SetText(text);
    }
}

// ea: 0x581460
void UIListBox::UIListBoxItem::SetSelected(bool selected, bool flashing)
{
    if (mType == kTypeText && mObjects.mSize > 0
        && mObjects.mElements[0] != nullptr)
    {
        FEText* v6 = (FEText*)mObjects.mElements[0];
        v6->SetFlag(0x100, selected);
        if (flashing)
        {
            v6->SetFlash(v6->GetColor(), v6->GetUnselectedColor(), 1.0f);
        }
        else
        {
            v6->SetNoFlash(v6->GetUnselectedColor());
        }
    }
}

// ea: 0x581580
void UIListBox::UIListBoxItem::SetColor(color32 unselectedColor,
                                        color32 selectedcolor)
{
    if (mType == kTypeText && mObjects.mSize > 0
        && mObjects.mElements[0] != nullptr)
    {
        ((FEText*)mObjects.mElements[0])->SetColorMenuItem(selectedcolor,
                                                           unselectedColor);
    }
}

// ea: 0x581650
void UIListBox::UIListBoxItem::SetEnabled(bool enabled)
{
    if (mType == kTypeText && mObjects.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
}

// ea: 0x5B4EC0
void UIListBox::UIListBoxItem::ClearText()
{
    if (mObjects.mSize <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("out of bounds"))
            __debugbreak();
    }
    if (mObjects.mElements[0] != nullptr && mType == kTypeText)
    {
        ((FEText*)mObjects.mElements[0])->SetText(defaultFileName);
    }
    SetEnabled(true);
}

// ea: 0x5B4FC0
void UIListBox::UIListBoxItem::ClearItem()
{
    ClearText();
    SetState(0);
}

// ea: 0x5B2070
void UIListBox::UIListBoxItem::RemoveItems()
{
    for (int i = 0; i < mStateCount; ++i)
        mObjects.mElements[i] = nullptr;
    mType = kTypeNone;
}

// ea: 0x5B22B0
color32 UIListBox::UIListBoxItem::GetColor()
{
    color32 result;
    result.i = 0x80808080u;
    if (mStateCount > 0)
    {
        for (int v3 = 0; v3 < mStateCount; ++v3)
        {
            if (mObjects.mElements[v3] != nullptr)
                return mObjects.mElements[v3]->GetColor();
        }
    }
    return result;
}

// ea: 0x5B23D0
color32 UIListBox::UIListBoxItem::GetUnselectedColor()
{
    color32 result;
    result.i = 0x80808080u;
    if (mObjects.mSize > 0 && mObjects.mElements[0] != nullptr
        && mType == kTypeText)
        return ((FEText*)mObjects.mElements[0])->GetUnselectedColor();
    return result;
}

// ea: 0x5B21A0
float UIListBox::UIListBoxItem::GetY()
{
    for (int v2 = 0; v2 < mStateCount; ++v2)
    {
        if (mObjects.mElements[v2] != nullptr)
            return mObjects.mElements[v2]->GetY();
    }
    return 0.0f;
}

// ea: 0x5B24E0
void UIListBox::UIListBoxItem::SetObject(PanelAnimObject* object, int state)
{
    if (state < 0 || state >= mStateCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 203;
        AeAssert::gCurrentExpr = "state >= 0 && state < mStateCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxItem: State index invalid"))
            __debugbreak();
    }
    mObjects.mElements[state] = object;
    if (mState != state && object != nullptr)
        object->SetShown(false);
}

// ============================================================================
// UIListBoxRow
// ============================================================================

// ea: 0x5B6E30
UIListBox::UIListBoxRow::UIListBoxRow()
{
    mColumns.mElements = nullptr;
    mColumns.mCapacity = 0;
    mColumns.mSize = 0;
}

// ea: 0x5B5390
void UIListBox::UIListBoxRow::ClearItem()
{
    for (int v2 = 0; v2 < mColumnCount; ++v2)
    {
        UIListBoxItem* v3 = &mColumns.mElements[v2];
        v3->ClearText();
        v3->SetState(0);
    }
}

// ea: 0x5B2630
void UIListBox::UIListBoxRow::RemoveItems()
{
    for (int v2 = 0; v2 < mColumnCount; ++v2)
        mColumns.mElements[v2].RemoveItems();
}

// ea: 0x5B26D0
void UIListBox::UIListBoxRow::Draw()
{
    for (int i = 0; i < mColumnCount; ++i)
    {
        UIListBoxItem* v3 = &mColumns.mElements[i];
        int v4 = v3->mState;
        if (v3->mObjects.mElements[v4] != nullptr)
            v3->mObjects.mElements[v4]->Draw();
    }
}

// ea: 0x5B2870
void UIListBox::UIListBoxRow::Update(float time_delta)
{
    for (int i = 0; i < mColumnCount; ++i)
    {
        UIListBoxItem* v4 = &mColumns.mElements[i];
        int v5 = v4->mState;
        if (v4->mObjects.mElements[v5] != nullptr)
            v4->mObjects.mElements[v5]->Update(time_delta);
    }
}

// ea: 0x5B7580
void UIListBox::UIListBoxRow::SetColumnCount(int columns)
{
    if (columns <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 246;
        AeAssert::gCurrentExpr = "columns > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column count must be greater then zero"))
            __debugbreak();
    }
    mColumnCount = columns;
    mColumns.mCapacity = columns;
    mColumns.mSize = columns;
    mColumns.mElements =
        (UIListBoxItem*)tlMemAlloc(24 * columns, 8u, 0);
    for (int i = 0; i < columns; ++i)
        new (&mColumns.mElements[i]) UIListBoxItem();
}

// ea: 0x5B5130
void UIListBox::UIListBoxRow::SetColumnStateCount(int column, int count)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 247;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetStateCount(count);
}

// ea: 0x5B51C0
void UIListBox::UIListBoxRow::SetItem(int column, FEText* text, int state)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 250;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    UIListBoxItem* v5 = &mColumns.mElements[column];
    v5->SetObject(text, state);
    v5->mType = UIListBoxItem::kTypeText;
}

// ea: 0x5B5260
void UIListBox::UIListBoxRow::SetItem(int column, PanelQuad* quad, int state)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 251;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    UIListBoxItem* v5 = &mColumns.mElements[column];
    v5->SetObject(quad, state);
    v5->mType = UIListBoxItem::kTypeQuad;
}

// ea: 0x5B5300
void UIListBox::UIListBoxRow::SetText(int column, const char* text)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 252;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetText(text);
}

// ea: 0x5B25A0
void UIListBox::UIListBoxRow::SetItemState(int column, int state)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 249;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetState(state);
}

// ea: 0x5B5450
void UIListBox::UIListBoxRow::SetColumnColor(int column,
                                             color32 unselectedColor,
                                             color32 selectedcolor)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 262;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetColor(unselectedColor, selectedcolor);
}

// ea: 0x5B54E0
void UIListBox::UIListBoxRow::SetSelected(int column, bool selected,
                                          bool flashing)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 264;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    mColumns.mElements[column].SetSelected(selected, flashing);
}

// ea: 0x5B5570
void UIListBox::UIListBoxRow::SetEnabled(bool enabled)
{
    for (int v3 = 0; v3 < mColumnCount; ++v3)
        mColumns.mElements[v3].SetEnabled(enabled);
}

// ea: 0x5B2A20
float UIListBox::UIListBoxRow::GetY(int column)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 259;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    return mColumns.mElements[column].GetY();
}

// ea: 0x5B2AB0
color32 UIListBox::UIListBoxRow::GetColumnSelectedColor(int column)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 260;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    return mColumns.mElements[column].GetColor();
}

// ea: 0x5B2B40
color32 UIListBox::UIListBoxRow::GetColumnUnselectedColor(int column)
{
    if (column < 0 || column >= mColumnCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 261;
        AeAssert::gCurrentExpr = "column >= 0 && column < mColumnCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBoxRow: Column index invalid"))
            __debugbreak();
    }
    return mColumns.mElements[column].GetUnselectedColor();
}

// ============================================================================
// UIListBox
// ============================================================================

// ea: 0x59BB70
UIListBox::UIListBox(int visibleRows, int visibleColumns, int maxDataRows,
                     bool bIsWrapping)
{
    mItemRows.mElements =
        (UIListBoxRow*)tlMemAlloc(16 * visibleRows, 8u, 0);
    mItemRows.mCapacity = visibleRows;
    mItemRows.mSize = visibleRows;
    for (int i = 0; i < visibleRows; ++i)
        new (&mItemRows.mElements[i]) UIListBoxRow();

    mDataRows.mElements =
        (UIListBoxDataRow*)tlMemAlloc(20 * maxDataRows, 8u, 0);
    mDataRows.mCapacity = maxDataRows;
    mDataRows.mSize = maxDataRows;
    for (int i = 0; i < maxDataRows; ++i)
        new (&mDataRows.mElements[i]) UIListBoxDataRow();

    mSelectedFlashing = false;
    mSelectedRowOriginalColor.mElements =
        (color32*)tlMemAlloc(4 * visibleColumns, 8u, 0);
    mSelectedRowOriginalColor.mCapacity = visibleColumns;
    mSelectedRowOriginalColor.mSize = visibleColumns;
    for (int i = 0; i < visibleColumns; ++i)
        mSelectedRowOriginalColor.mElements[i].i = 0;

    mSelectedRowColorChangeColumns.mElements =
        (bool*)tlMemAlloc(1 * visibleRows, 8u, 0);
    mSelectedRowColorChangeColumns.mCapacity = visibleRows;
    mSelectedRowColorChangeColumns.mSize = visibleRows;
    for (int i = 0; i < visibleRows; ++i)
        mSelectedRowColorChangeColumns.mElements[i] = true;

    mLastRowContainingData = 0;
    mItemRowsCount = visibleRows;
    mItemColumnsCount = visibleColumns;
    mDataRowsCount = maxDataRows;
    mTopLine = 0;
    mSelectedLine = -1;
    mScrollBarTopY = 0;
    mScrollBarBottomY = 0;
    mScrollBarYInc = 0.0f;
    mIsWrapping = bIsWrapping;
    mBlockRefresh = false;
    mScrollBarUpFader.mQuad = nullptr;
    mScrollBarUpFader.mAlphaTo = 1.0f;
    mScrollBarUpFader.mTime = 0.0f;
    mScrollBarUpFader.mAlphaDelta = 0.0f;
    mScrollBarUpFader.mAlpha = 0.0f;
    mScrollBarUpFader.mFading = false;
    mScrollBarDownFader.mQuad = nullptr;
    mScrollBarDownFader.mAlphaTo = 1.0f;
    mScrollBarDownFader.mTime = 0.0f;
    mScrollBarDownFader.mAlphaDelta = 0.0f;
    mScrollBarDownFader.mAlpha = 0.0f;
    mScrollBarDownFader.mFading = false;

    if (visibleRows <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 94;
        AeAssert::gCurrentExpr = "visibleRows > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Visible row count must be greater then 0"))
            __debugbreak();
    }
    if (visibleColumns <= 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 95;
        AeAssert::gCurrentExpr = "visibleColumns > 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Visible column count must be greater then 0"))
            __debugbreak();
    }
    if (maxDataRows < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 96;
        AeAssert::gCurrentExpr = "maxDataRows >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Max data row count must be greater then 0"))
            __debugbreak();
    }
    for (int i = 0; i < visibleRows; ++i)
        mItemRows.mElements[i].SetColumnCount(visibleColumns);
    for (int i = 0; i < maxDataRows; ++i)
        mDataRows.mElements[i].SetColumnCount(visibleColumns);

    for (int i = 0; i < 6; ++i)
        mScrollBarQuads[i] = nullptr;
}

// ea: 0x5B9BB0
UIListBox::~UIListBox()
{
    if (mSelectedRowColorChangeColumns.mElements != nullptr)
    {
        tlMemFree(mSelectedRowColorChangeColumns.mElements);
        mSelectedRowColorChangeColumns.mElements = nullptr;
        mSelectedRowColorChangeColumns.mCapacity = 0;
    }
    if (mSelectedRowOriginalColor.mElements != nullptr)
    {
        tlMemFree(mSelectedRowOriginalColor.mElements);
        mSelectedRowOriginalColor.mElements = nullptr;
        mSelectedRowOriginalColor.mCapacity = 0;
    }
    for (int i = 0; i < mDataRows.mSize; ++i)
        mDataRows.mElements[i].~UIListBoxDataRow();
    if (mDataRows.mElements != nullptr)
    {
        tlMemFree(mDataRows.mElements);
        mDataRows.mElements = nullptr;
        mDataRows.mCapacity = 0;
    }
    for (int i = 0; i < mItemRows.mSize; ++i)
        mItemRows.mElements[i].~UIListBoxRow();
    if (mItemRows.mElements != nullptr)
    {
        tlMemFree(mItemRows.mElements);
        mItemRows.mElements = nullptr;
        mItemRows.mCapacity = 0;
    }
}

// ea: 0x58FFC0
void UIListBox::Clear()
{
    DeselectRow();
    for (int v2 = 0; v2 < mDataRowsCount; ++v2)
        mDataRows.mElements[v2].ClearItem();
    mLastRowContainingData = 0;
    mTopLine = 0;
    mSelectedLine = -1;
    Refresh();
}

// ea: 0x590050
void UIListBox::ClearRow(int row)
{
    if (row < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 136;
        AeAssert::gCurrentExpr = "row >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Invalid row to clear"))
            __debugbreak();
    }
    if (row <= mLastRowContainingData)
    {
        DeselectRow();
        mDataRows.mElements[row].mEnabled = true;
        if (mLastRowContainingData > row && row < mLastRowContainingData - 1)
        {
            for (int v2 = row; v2 < mLastRowContainingData - 1; ++v2)
            {
                mDataRows.mElements[v2] = mDataRows.mElements[v2 + 1];
            }
        }
        mDataRows.mElements[mLastRowContainingData].ClearItem();
        if (--mLastRowContainingData < 0)
            mLastRowContainingData = 0;
        if (mTopLine + mSelectedLine > mLastRowContainingData)
            MoveUpTo(mLastRowContainingData);
        SelectRow();
        Refresh();
    }
}

// ea: 0x5903F0
void UIListBox::SelectLine(int selection)
{
    if (selection < 0 || selection >= mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 572;
        AeAssert::gCurrentExpr =
            "selection >= 0 && selection < mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SelectLine: invalid selection index"))
            __debugbreak();
    }
    DeselectRow();
    if (mSelectedLine == -1)
        mSelectedLine = 0;
    if (selection >= mSelectedLine + mTopLine)
        MoveDownTo(selection);
    else
        MoveUpTo(selection);
    SelectRow();
    Refresh();
}

// ea: 0x5904A0
void UIListBox::SelectLine(int selection, int top_line)
{
    if (selection < 0 || selection >= mItemRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 602;
        AeAssert::gCurrentExpr =
            "selection >= 0 && selection < mItemRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SelectLine: invalid selection index"))
            __debugbreak();
    }
    DeselectRow();
    mTopLine = top_line;
    mSelectedLine = selection;
    SelectRow();
    UpdateScrollBar();
}

// ============================================================================
// UIHighlightListBox
// ============================================================================

// ea: 0x0059BF60
UIHighlightListBox::UIHighlightListBox(int visibleRows, int visibleColumns,
                                       int maxDataRows, bool bIsWrapping)
    : UIListBox(visibleRows, visibleColumns, maxDataRows, bIsWrapping)
{
    mHighlightQuad = nullptr;
    bool fillFalse = false;
    mHighlights.mElements = (bool*)tlMemAlloc(1 * maxDataRows, 8u, 0);
    mHighlights.mCapacity = maxDataRows;
    mHighlights.mSize = maxDataRows;
    for (int i = 0; i < maxDataRows; ++i)
        mHighlights.mElements[i] = fillFalse;

    mHighlightedRowOriginalSelectedColor.mElements =
        (color32*)tlMemAlloc(4 * visibleColumns, 8u, 0);
    mHighlightedRowOriginalSelectedColor.mCapacity = visibleColumns;
    mHighlightedRowOriginalSelectedColor.mSize = visibleColumns;
    for (int i = 0; i < visibleColumns; ++i)
        mHighlightedRowOriginalSelectedColor.mElements[i].i =
            lUIHighlightListBoxDefaultSelectedColor.i;

    mHighlightedRowOriginalUnselectedColor.mElements =
        (color32*)tlMemAlloc(4 * visibleColumns, 8u, 0);
    mHighlightedRowOriginalUnselectedColor.mCapacity = visibleColumns;
    mHighlightedRowOriginalUnselectedColor.mSize = visibleColumns;
    for (int i = 0; i < visibleColumns; ++i)
        mHighlightedRowOriginalUnselectedColor.mElements[i].i =
            lUIHighlightListBoxDefaultUnselectedColor.i;

    mHighlightedSelectedTextColor =
        lUIHighlightListBoxDefaultSelectedColor;
    mHighlightedUnselectedTextColor =
        lUIHighlightListBoxDefaultUnselectedColor;
    mHighlightedRow = -1;
}

// ea: 0x00581DC0
void UIHighlightListBox::ClearHighlights()
{
    for (int i = 0; i < mDataRowsCount; ++i)
    {
        if (i < 0 || i >= mHighlights.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mHighlights.mElements[i] = false;
    }
}

// ea: 0x00581E40
void UIHighlightListBox::UpdateHighlight()
{
    bool highlightOnScreen = false;
    if (mHighlightQuad != nullptr)
    {
        int mTopLine = this->mTopLine;
        int v3 = mTopLine + mItemRowsCount;
        mHighlightedRow = -1;
        if (mTopLine < v3)
        {
            while (1)
            {
                if (mTopLine < 0 || mTopLine >= mHighlights.mSize)
                {
                    AeAssert::gCurrentAuthor = AeAssert::COD3;
                    AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                    AeAssert::gCurrentLine = 167;
                    AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                    if (!AeAssert::IsIgnored()
                        && AeAssert::Assert("out of bounds"))
                        __debugbreak();
                }
                if (mHighlights.mElements[mTopLine])
                    break;
                if (++mTopLine >= mItemRowsCount + this->mTopLine)
                    goto LABEL_12;
            }
            mHighlightedRow = mTopLine - this->mTopLine;
            highlightOnScreen = true;
        }
    LABEL_12:
        mHighlightQuad->SetShown(highlightOnScreen);
        if (highlightOnScreen)
        {
            UIListBoxRow* v4 = &mItemRows.mElements[mHighlightedRow];
            if (v4->mColumnCount <= 0)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
                AeAssert::gCurrentLine = 259;
                AeAssert::gCurrentExpr =
                    "column >= 0 && column < mColumnCount";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert(
                        "UIListBoxRow: Column index invalid"))
                    __debugbreak();
            }
            UIListBoxItem* v5 = &v4->mColumns.mElements[0];
            float highlightOnScreena = v5->GetY();
            float v8 = mHighlightQuad->GetCenterX();
            mHighlightQuad->SetCenterPos(v8, highlightOnScreena);
        }
    }
}

// ea: 0x00581F90
void UIHighlightListBox::SaveHighlightRowColor()
{
    int mHighlightedRow = this->mHighlightedRow;
    if (mHighlightedRow != -1)
    {
        UIListBoxRow* row = &mItemRows.mElements[mHighlightedRow];
        for (int i = 0; i < mItemColumnsCount; ++i)
        {
            if (i < 0 || i >= mHighlightedRowOriginalUnselectedColor.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            color32* v4 = &mHighlightedRowOriginalUnselectedColor
                               .mElements[i];
            v4->i = row->GetColumnUnselectedColor(i).i;
            if (i < 0 || i >= mHighlightedRowOriginalSelectedColor.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            color32* v5 = &mHighlightedRowOriginalSelectedColor.mElements[i];
            v5->i = row->GetColumnSelectedColor(i).i;
        }
    }
}

// ea: 0x00588220
void UIHighlightListBox::RestoreHighlightRowColor()
{
    int mHighlightedRow = this->mHighlightedRow;
    if (mHighlightedRow != -1)
    {
        UIListBoxRow* row = &mItemRows.mElements[mHighlightedRow];
        for (int i = 0; i < mItemColumnsCount; ++i)
        {
            if (i < 0 || i >= mSelectedRowColorChangeColumns.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            if (mSelectedRowColorChangeColumns.mElements[i])
            {
                color32* v4 =
                    &mHighlightedRowOriginalSelectedColor.mElements[i];
                color32* v5 =
                    &mHighlightedRowOriginalUnselectedColor.mElements[i];
                row->SetColumnColor(i, v5->i, v4->i);
            }
        }
    }
}

// ea: 0x005882F0
void UIHighlightListBox::ColorHighlightRow()
{
    int mHighlightedRow = this->mHighlightedRow;
    if (mHighlightedRow != -1)
    {
        UIListBoxRow* v3 = &mItemRows.mElements[mHighlightedRow];
        for (int i = 0; i < mItemColumnsCount; ++i)
        {
            if (i < 0 || i >= mSelectedRowColorChangeColumns.mSize)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
                AeAssert::gCurrentLine = 167;
                AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
                if (!AeAssert::IsIgnored()
                    && AeAssert::Assert("out of bounds"))
                    __debugbreak();
            }
            if (mSelectedRowColorChangeColumns.mElements[i])
                v3->SetColumnColor(i, mHighlightedUnselectedTextColor,
                                   mHighlightedSelectedTextColor);
        }
    }
}

// ea: 0x00588390
void UIHighlightListBox::SetEntryColor(int y, int x, color32 colorUnLit,
                                       color32 colorLit)
{
    if (y < mItemRowsCount && x < mItemColumnsCount)
    {
        if (y == mHighlightedRow)
        {
            mHighlightedRowOriginalSelectedColor.mElements[x].i =
                colorLit.i;
            mHighlightedRowOriginalUnselectedColor.mElements[x].i =
                colorUnLit.i;
        }
        else
        {
            UIListBoxRow* v6 = &mItemRows.mElements[y];
            v6->SetColumnColor(x, colorUnLit, colorLit);
        }
    }
}

// ea: 0x005909B0
void UIHighlightListBox::Clear()
{
    UIListBox::Clear();
    DeselectRow();
    ClearHighlights();
}

// ea: 0x005909D0
void UIHighlightListBox::Refresh()
{
    RestoreHighlightRowColor();
    UIListBox::Refresh();
    UpdateHighlight();
    SaveHighlightRowColor();
    ColorHighlightRow();
}

// ============================================================================
// UIPlayerListBox
// ============================================================================

// ea: 0x0059C020
UIPlayerListBox::UIPlayerListBox(int visibleRows, int visibleColumns,
                                 int maxDataRows, bool bIsWrapping)
    : UIHighlightListBox(visibleRows, visibleColumns, maxDataRows,
                         bIsWrapping)
{
    mPlayerRow = -1;
    mPlayerIDs.mElements = nullptr;
    mPlayerIDs.mCapacity = 0;
    mPlayerIDs.mSize = 0;
    mPlayerXUIDs.mElements = nullptr;
    mPlayerXUIDs.mCapacity = 0;
    mPlayerXUIDs.mSize = 0;

    int fillId = (int)MPPlayer::GetNullId();
    mPlayerIDs.mElements = (int*)tlMemAlloc(4 * maxDataRows, 8u, 0);
    mPlayerIDs.mCapacity = maxDataRows;
    mPlayerIDs.mSize = maxDataRows;
    for (int i = 0; i < maxDataRows; ++i)
        mPlayerIDs.mElements[i] = fillId;

    myXUID.qwValue = 0;
    myXUID.dwUserFlags = 0;

    _XUID initializer;
    initializer.qwValue = 0;
    initializer.dwUserFlags = 0;
    mPlayerXUIDs.mElements = (_XUID*)tlMemAlloc(12 * maxDataRows, 8u, 0);
    mPlayerXUIDs.mCapacity = maxDataRows;
    mPlayerXUIDs.mSize = maxDataRows;
    for (int i = 0; i < maxDataRows; ++i)
        mPlayerXUIDs.mElements[i] = initializer;
}

// ea: 0x005820C0
void UIPlayerListBox::CheckIfLocalPlayer(int row)
{
    int* v4 = &mPlayerIDs.mElements[row];
    if (*v4 == MPPlayer::GetNullId()
        && mPlayerXUIDs.mElements[row].qwValue == 0)
    {
        mHighlights.mElements[row] = false;
        return;
    }
    if (g_femanager.inGame)
    {
        MPPeer* mPeer = MultiplayerMgr::sInst->mPeer;
        unsigned char v11 = (unsigned char)mPlayerIDs.mElements[row];
        MPPlayerManager* PlayerManager = mPeer->GetPlayerManager();
        MPPlayer* Player = PlayerManager->GetPlayer(v11);
        if (Player != nullptr && Player->IsLocalPlayer())
        {
            mHighlights.mElements[row] = true;
            Refresh();
            return;
        }
        mHighlights.mElements[row] = false;
        return;
    }
    unsigned int i = 0;
    while (1)
    {
        LivePlayer* LocalPlayer =
            LiveWrapper::theWrapper->GetLocalPlayer(i);
        _XUID* v9 = &mPlayerXUIDs.mElements[row];
        const XUID* v10 = &LocalPlayer->xuid;
        if (v10->qwValue == v9->qwValue)
            break;
        if (++i >= 4)
        {
            mHighlights.mElements[row] = false;
            return;
        }
    }
    mHighlights.mElements[row] = true;
    Refresh();
}

// ea: 0x00588400
void UIPlayerListBox::SetPlayerID(int row, int id)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIPlayerListBox.cpp";
        AeAssert::gCurrentLine = 56;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid row index"))
            __debugbreak();
    }
    mPlayerIDs.mElements[row] = id;
    CheckIfLocalPlayer(row);
}

// ea: 0x00588480
void UIPlayerListBox::SetPlayerXUID(int row, _XUID id)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIPlayerListBox.cpp";
        AeAssert::gCurrentLine = 66;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid row index"))
            __debugbreak();
    }
    mPlayerXUIDs.mElements[row] = id;
    CheckIfLocalPlayer(row);
}

// ea: 0x00590A00
void UIPlayerListBox::Clear()
{
    UIListBox::Clear();
    DeselectRow();
    ClearHighlights();
    for (int i = 0; i < mDataRowsCount; ++i)
    {
        if (i < 0 || i >= mPlayerIDs.mSize)
        {
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "../ae\\core/ae_vector.h";
            AeAssert::gCurrentLine = 167;
            AeAssert::gCurrentExpr = "iIndex >= 0 && iIndex < mSize";
            if (!AeAssert::IsIgnored()
                && AeAssert::Assert("out of bounds"))
                __debugbreak();
        }
        mPlayerIDs.mElements[i] = MPPlayer::GetNullId();
    }
    mPlayerRow = -1;
    myXUID.qwValue = 0;
    myXUID.dwUserFlags = 0;
}

// ea: 0x00590AB0
void UIPlayerListBox::ClearRow(int row)
{
    UIListBox::ClearRow(row);
    unsigned char NullId = MPPlayer::GetNullId();
    SetPlayerID(row, (int)NullId);
    if (row == mPlayerRow)
        mPlayerRow = -1;
}

// ea: 0x590290
short UIListBox::OnUp(int c)
{
    MoveUp(c);
    if (mLastRowContainingData != 0)
    {
        while (1)
        {
            int v3 = mTopLine + mSelectedLine;
            if (mDataRows.mElements[v3].mEnabled)
                break;
            MoveUp(c);
        }
    }
    Refresh();
    return (short)mSelectedLine;
}

// ea: 0x590340
short UIListBox::OnDown(int c)
{
    MoveDown(c);
    if (mLastRowContainingData != 0)
    {
        while (1)
        {
            int v3 = mTopLine + mSelectedLine;
            if (mDataRows.mElements[v3].mEnabled)
                break;
            MoveDown(c);
        }
    }
    Refresh();
    return (short)mSelectedLine;
}

// ea: 0x581750
void UIListBox::Update(float time_delta)
{
    if (mScrollBarUpFader.mQuad != nullptr && mScrollBarUpFader.mFading)
    {
        if (mScrollBarUpFader.mAlphaTo <= mScrollBarUpFader.mAlpha)
        {
            if (mScrollBarUpFader.mAlpha > mScrollBarUpFader.mAlphaTo)
            {
                mScrollBarUpFader.mAlpha -=
                    (time_delta / mScrollBarUpFader.mTime)
                    * mScrollBarUpFader.mAlphaDelta;
                if (mScrollBarUpFader.mAlphaTo < mScrollBarUpFader.mAlpha)
                    goto LABEL_10;
            }
            mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
            mScrollBarUpFader.mFading = false;
        }
        else
        {
            mScrollBarUpFader.mAlpha +=
                (time_delta / mScrollBarUpFader.mTime)
                * mScrollBarUpFader.mAlphaDelta;
            if (mScrollBarUpFader.mAlpha >= mScrollBarUpFader.mAlphaTo)
            {
                mScrollBarUpFader.mAlpha = mScrollBarUpFader.mAlphaTo;
                mScrollBarUpFader.mFading = false;
            }
        }
    LABEL_10:
        mScrollBarUpFader.mQuad->SetAlpha(mScrollBarUpFader.mAlpha);
    }
    if (mScrollBarDownFader.mQuad != nullptr && mScrollBarDownFader.mFading)
    {
        if (mScrollBarDownFader.mAlphaTo <= mScrollBarDownFader.mAlpha)
        {
            if (mScrollBarDownFader.mAlpha > mScrollBarDownFader.mAlphaTo)
            {
                mScrollBarDownFader.mAlpha -=
                    (time_delta / mScrollBarDownFader.mTime)
                    * mScrollBarDownFader.mAlphaDelta;
                if (mScrollBarDownFader.mAlphaTo
                    < mScrollBarDownFader.mAlpha)
                    goto LABEL_20;
            }
            mScrollBarDownFader.mAlpha = mScrollBarDownFader.mAlphaTo;
            mScrollBarDownFader.mFading = false;
        }
        else
        {
            mScrollBarDownFader.mAlpha +=
                (time_delta / mScrollBarDownFader.mTime)
                * mScrollBarDownFader.mAlphaDelta;
            if (mScrollBarDownFader.mAlpha
                >= mScrollBarDownFader.mAlphaTo)
            {
                mScrollBarDownFader.mAlpha =
                    mScrollBarDownFader.mAlphaTo;
                mScrollBarDownFader.mFading = false;
            }
        }
    LABEL_20:
        mScrollBarDownFader.mQuad->SetAlpha(
            mScrollBarDownFader.mAlpha);
    }
    if (mSelectedLine != -1)
        mItemRows.mElements[mSelectedLine].Update(time_delta);
}

// ea: 0x581910
void UIListBox::Draw()
{
    for (int v2 = 0; v2 < mItemRowsCount; ++v2)
        mItemRows.mElements[v2].Draw();
}

// ea: 0x5816B0
void UIListBox::RemoveAllItems()
{
    mScrollBarUpFader.mQuad = nullptr;
    mScrollBarDownFader.mQuad = nullptr;
    for (int i = 0; i < 6; ++i)
        mScrollBarQuads[i] = nullptr;
    for (int v2 = 0; v2 < mItemRowsCount; ++v2)
        mItemRows.mElements[v2].RemoveItems();
}

// ea: 0x581990
void UIListBox::SetItemState(int row, int column, int state)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 274;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItemState invalid row index"))
            __debugbreak();
    }
    if (column < 0 || column > mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 275;
        AeAssert::gCurrentExpr =
            "column >= 0 && column <= mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItemState invalid column index"))
            __debugbreak();
    }
    mDataRows.mElements[row].SetItemState(column, state);
    int v7 = row - mTopLine;
    if (v7 >= 0 && v7 < mItemRowsCount)
    {
        if (row == mSelectedLine + mTopLine)
            mItemRows.mElements[mSelectedLine].SetItemState(column, state);
        else
            mItemRows.mElements[v7].SetItemState(column, state);
    }
}

// ea: 0x581A90
void UIListBox::PlayNavigationSound()
{
    math::Position3 v2;
    math::Dir3 v3;
    memset(&v2, 0, sizeof(v2));
    memset(&v3, 0, sizeof(v3));
    SoundDevice::sInst->PlaySound(
        "UI_Select", DbLinkedHandle<EntityHandleDb, Entity>(), true, false,
        v2, v3, -1.0f, -1.0f, -1.0f, -1.0f);
}

// ea: 0x581B00
void UIListBox::PageDown(int numPageRows)
{
    PlayNavigationSound();
    if (mScrollBarDownFader.mQuad != nullptr)
    {
        mScrollBarDownFader.mAlpha = 1.0f;
        mScrollBarDownFader.mFading = true;
        mScrollBarDownFader.mAlphaTo = 0.5f;
        mScrollBarDownFader.mTime = 0.5f;
        mScrollBarDownFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarDownFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarDownFader.mFading = false;
    }
    int v5 = mLastRowContainingData - mItemRowsCount + 1;
    if (v5 < 0)
        v5 = 0;
    if (mTopLine + numPageRows <= v5)
        v5 = mTopLine + numPageRows;
    int v8 = mTopLine + mSelectedLine - v5 + numPageRows;
    if (v8 >= mItemRowsCount - 1)
        v8 = mItemRowsCount - 1;
    if (v8 > mLastRowContainingData)
        v8 = mLastRowContainingData;
    SelectLine(v8, v5);
    Refresh();
}

// ea: 0x581BC0
void UIListBox::PageUp(int numPageRows)
{
    PlayNavigationSound();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarUpFader.mFading = false;
    }
    int top = mTopLine - numPageRows;
    if (top < 0)
        top = 0;
    int v4 = mSelectedLine - (mTopLine - numPageRows < 0 ? 0
                                                         : mTopLine - numPageRows)
             - numPageRows + mTopLine;
    if (v4 <= 0)
        v4 = 0;
    SelectLine(v4, top);
    Refresh();
}

// ea: 0x581C70
void UIListBox::UpdateScrollBar()
{
    bool scroll_shown = mLastRowContainingData >= mItemRowsCount;
    for (int i = 0; i < 6; ++i)
    {
        if (mScrollBarQuads[i] != nullptr)
            mScrollBarQuads[i]->SetShown(scroll_shown);
    }
    if (scroll_shown)
    {
        if (mScrollBarQuads[3] != nullptr)
        {
            Broc::vector Min = mScrollBarQuads[3]->GetMin();
            Broc::vector Max = mScrollBarQuads[3]->GetMax();
            mScrollBarTopY = (int)Min.y;
            mScrollBarBottomY = (int)Max.y;
            mScrollBarYInc =
                (Max.y - Min.y) / mLastRowContainingData;
        }
        if (mScrollBarQuads[2] != nullptr)
        {
            mScrollBarQuads[2]->SetCenterPos(
                mScrollBarQuads[2]->GetCenterX(),
                (float)(mScrollBarTopY
                        + (mTopLine + mSelectedLine) * mScrollBarYInc));
        }
    }
}

// ea: 0x72ABD0
void UIListBox::SetColumnSelectable(int column, bool selectable)
{
    if (column < 0 || column >= mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.h";
        AeAssert::gCurrentLine = 125;
        AeAssert::gCurrentExpr =
            "column >= 0 && column < mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "UIListBoxItem: State count must be greater then zero"))
            __debugbreak();
    }
    mSelectedRowColorChangeColumns.mElements[column] = selectable;
}

// ea: 0x581D40
void UIListBox::SetAllColumnsSelectable(bool selectable)
{
    for (int i = 0; i < mItemColumnsCount; ++i)
        mSelectedRowColorChangeColumns.mElements[i] = selectable;
}

// ea: 0x5878D0
void UIListBox::SetRowEnabled(int row, bool enabled)
{
    if (row < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 180;
        AeAssert::gCurrentExpr = "row >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Invalid row to clear"))
            __debugbreak();
    }
    if (row <= mLastRowContainingData)
    {
        mDataRows.mElements[row].mEnabled = enabled;
        if (row >= mTopLine && row < mTopLine + mItemRowsCount)
            mItemRows.mElements[row - mTopLine].SetEnabled(enabled);
    }
}

// ea: 0x587960
void UIListBox::SetColumnStateCount(int column, int count)
{
    if (column < 0 || column > mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 257;
        AeAssert::gCurrentExpr =
            "column >= 0 && column <= mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetColumnStateCount invalid column index"))
            __debugbreak();
    }
    for (int j = 0; j < mItemRowsCount; ++j)
        mItemRows.mElements[j].SetColumnStateCount(column, count);
    for (int i = 0; i < mDataRowsCount; ++i)
        mDataRows.mElements[i].SetItemState(column, 0);
}

// ea: 0x587BA0
void UIListBox::SetItem(int row, int column, FEText* text, int state)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 298;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid row index"))
            __debugbreak();
    }
    if (column < 0 || column > mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 299;
        AeAssert::gCurrentExpr =
            "column >= 0 && column <= mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid column index"))
            __debugbreak();
    }
    if (text == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 300;
        AeAssert::gCurrentExpr = "text";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid FEText"))
            __debugbreak();
    }
    mItemRows.mElements[row].SetItem(column, text, state);
}

// ea: 0x587CB0
void UIListBox::SetItem(int row, int column, PanelQuad* quad, int state)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 309;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid row index"))
            __debugbreak();
    }
    if (column < 0 || column > mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 310;
        AeAssert::gCurrentExpr =
            "column >= 0 && column <= mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid column index"))
            __debugbreak();
    }
    if (quad == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 311;
        AeAssert::gCurrentExpr = "quad";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid PanelQuad"))
            __debugbreak();
    }
    mItemRows.mElements[row].SetItem(column, quad, state);
}

// ea: 0x587DC0
void UIListBox::SetText(int row, int column, const char* text)
{
    if (row < 0 || row > mDataRowsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 320;
        AeAssert::gCurrentExpr = "row >= 0 && row <= mDataRowsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid row index"))
            __debugbreak();
    }
    if (column < 0 || column > mItemColumnsCount)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 321;
        AeAssert::gCurrentExpr =
            "column >= 0 && column <= mItemColumnsCount";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetItem invalid column index"))
            __debugbreak();
    }
    if (text == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 322;
        AeAssert::gCurrentExpr = "text";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("SetText invalid char* passed in"))
            __debugbreak();
    }
    mDataRows.mElements[row].SetText(column, text);
    if (row > mLastRowContainingData)
        mLastRowContainingData = row;
    if (row >= mTopLine && row < mTopLine + mItemRowsCount)
        mItemRows.mElements[row - mTopLine].SetText(column, text);
}

// ea: 0x577050
void UIListBox::SetScrollBarQuad(EScrollQuads index, PanelQuad* quad)
{
    mScrollBarQuads[index] = quad;
    if (quad != nullptr)
    {
        quad->SetShown(false);
        if (index == kScrollBarArrowDown)
        {
            mScrollBarDownFader.mQuad = quad;
            quad->SetAlpha(0.5f);
        }
        else if (index == kScrollBarArrowUp)
        {
            mScrollBarUpFader.mQuad = quad;
            quad->SetAlpha(0.5f);
        }
    }
}

// ea: 0x598120
void UIListBox::SetScrollBarFromPanelFile(PanelFile* pf)
{
    if (pf == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 799;
        AeAssert::gCurrentExpr = "pf";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Invalid panel file"))
            __debugbreak();
    }
    PanelQuad* v4 = pf->GetPointer("scroll_arrow_down");
    mScrollBarQuads[1] = v4;
    if (v4 != nullptr)
    {
        v4->SetShown(false);
        mScrollBarDownFader.mQuad = v4;
        v4->SetAlpha(0.5f);
    }
    PanelQuad* v6 = pf->GetPointer("scroll_arrow_up");
    mScrollBarQuads[0] = v6;
    if (v6 != nullptr)
    {
        v6->SetShown(false);
        mScrollBarUpFader.mQuad = v6;
        v6->SetAlpha(0.5f);
    }
    PanelQuad* v7 = pf->GetPointer("scroll_detail_01");
    mScrollBarQuads[4] = v7;
    if (v7 != nullptr)
        v7->SetShown(false);
    PanelQuad* v8 = pf->GetPointer("scroll_detail_02");
    mScrollBarQuads[5] = v8;
    if (v8 != nullptr)
        v8->SetShown(false);
    PanelQuad* v9 = pf->GetPointer("scroll_indicator");
    mScrollBarQuads[2] = v9;
    if (v9 != nullptr)
        v9->SetShown(false);
    PanelQuad* v10 = pf->GetPointer("scroll_indicator_reference");
    mScrollBarQuads[3] = v10;
    if (v10 != nullptr)
        v10->SetShown(false);
}

// ea: 0x5770B0
void UIListBox::FormatForSplitScreen(int viewport, int old_viewport)
{
    (void)viewport;
    (void)old_viewport;
}

// ea: 0x5770C0
void UIListBox::FormatForWidescreen(bool widescreen, float about_x)
{
    (void)widescreen;
    (void)about_x;
}

// ea: 0x590150
void UIListBox::ResizeDataRows(int rowCount)
{
    if (rowCount < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\UIListBox.cpp";
        AeAssert::gCurrentLine = 216;
        AeAssert::gCurrentExpr = "rowCount >= 0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("UIListBox: Max data row count must be greater then 0"))
            __debugbreak();
    }
    mDataRowsCount = rowCount;
    if (rowCount > 0)
    {
        for (int i = 0; i < rowCount; ++i)
            mDataRows.mElements[i].SetColumnCount(mItemColumnsCount);
    }
    Refresh();
}

// ea: 0x576F70
void UIListBox::MoveUpTo(int newSelectedLine)
{
    if (newSelectedLine > mLastRowContainingData)
        newSelectedLine = mLastRowContainingData;
    if (newSelectedLine < 0)
        newSelectedLine = 0;
    int v4 = newSelectedLine - mItemRowsCount / 2 - 1;
    if (mItemRowsCount + v4 - 1 > mLastRowContainingData)
        v4 = mLastRowContainingData - mItemRowsCount + 1;
    if (v4 < 0)
        v4 = 0;
    int select_line = newSelectedLine - v4;
    if (select_line >= mItemRowsCount)
        select_line = mItemRowsCount - 1;
    SelectLine(select_line, v4);
}

// ea: 0x576FE0
void UIListBox::MoveDownTo(int newSelectedLine)
{
    if (newSelectedLine > mLastRowContainingData)
        newSelectedLine = mLastRowContainingData;
    if (newSelectedLine < 0)
        newSelectedLine = 0;
    int v4 = newSelectedLine - mItemRowsCount / 2;
    if (mItemRowsCount + v4 - 1 > mLastRowContainingData)
        v4 = mLastRowContainingData - mItemRowsCount + 1;
    if (v4 < 0)
        v4 = 0;
    int select_line = newSelectedLine - v4;
    if (select_line >= mItemRowsCount)
        select_line = mItemRowsCount - 1;
    SelectLine(select_line, v4);
}

// ea: 0x587F00
short UIListBox::MoveUp(int c)
{
    int v3 = mSelectedLine - 1;
    PlayNavigationSound();
    if (mScrollBarUpFader.mQuad != nullptr)
    {
        mScrollBarUpFader.mAlpha = 1.0f;
        mScrollBarUpFader.mFading = true;
        mScrollBarUpFader.mAlphaTo = 0.5f;
        mScrollBarUpFader.mTime = 0.5f;
        mScrollBarUpFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarUpFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarUpFader.mFading = false;
    }
    if (v3 >= mItemRowsCount / 2 - 1)
    {
        SelectLine(v3, mTopLine);
        return (short)mSelectedLine;
    }
    int v6 = mTopLine - 1;
    if (v6 >= 0)
    {
        mTopLine = v6;
        return (short)mSelectedLine;
    }
    if (v3 >= 0)
    {
        SelectLine(v3, 0);
        return (short)mSelectedLine;
    }
    if (!mIsWrapping)
        return (short)mSelectedLine;
    int v9 = mLastRowContainingData - mItemRowsCount + 1;
    if (v9 < 0)
        v9 = 0;
    int sel = mLastRowContainingData >= mItemRowsCount
                  ? mItemRowsCount - 1
                  : mLastRowContainingData;
    SelectLine(sel, v9);
    return (short)mSelectedLine;
}

// ea: 0x587FF0
short UIListBox::MoveDown(int c)
{
    int v3 = mSelectedLine + 1;
    PlayNavigationSound();
    if (mScrollBarDownFader.mQuad != nullptr)
    {
        mScrollBarDownFader.mAlpha = 1.0f;
        mScrollBarDownFader.mFading = true;
        mScrollBarDownFader.mAlphaTo = 0.5f;
        mScrollBarDownFader.mTime = 0.5f;
        mScrollBarDownFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarDownFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarDownFader.mFading = false;
    }
    if (v3 <= mItemRowsCount / 2)
    {
        if (v3 <= mLastRowContainingData)
        {
            SelectLine(v3, mTopLine);
            return (short)mSelectedLine;
        }
        if (mIsWrapping)
        {
            SelectLine(0, 0);
        }
    }
    else
    {
        int v6 = mTopLine + 1;
        if (v6 + mItemRowsCount <= mLastRowContainingData + 1)
        {
            mTopLine = v6;
            return (short)mSelectedLine;
        }
        if (v6 + mSelectedLine <= mLastRowContainingData)
        {
            SelectLine(v3, mTopLine);
            return (short)mSelectedLine;
        }
        if (mIsWrapping)
        {
            SelectLine(0, 0);
        }
    }
    return (short)mSelectedLine;
}

// ea: 0x5880E0
void UIListBox::DeselectRow()
{
    if (mSelectedLine != -1)
    {
        UIListBoxRow* row = &mItemRows.mElements[mSelectedLine];
        for (int i = 0; i < mItemColumnsCount; ++i)
        {
            if (mSelectedRowColorChangeColumns.mElements[i])
                row->SetSelected(i, false, false);
        }
    }
}

// ea: 0x588180
void UIListBox::SelectRow()
{
    if (mSelectedLine != -1)
    {
        UIListBoxRow* row = &mItemRows.mElements[mSelectedLine];
        for (int i = 0; i < mItemColumnsCount; ++i)
        {
            if (mSelectedRowColorChangeColumns.mElements[i])
                row->SetSelected(i, true, mSelectedFlashing);
        }
    }
}

// ea: 0x590520
void UIListBox::Refresh()
{
    if (mBlockRefresh)
        return;
    DeselectRow();
    for (int i = 0; i < mItemRowsCount; ++i)
    {
        UIListBoxRow* row = &mItemRows.mElements[i];
        row->ClearItem();
        for (int col = 0; col < mItemColumnsCount; ++col)
        {
            int dataRow = i + mTopLine;
            UIListBoxDataRow* v7 = &mDataRows.mElements[dataRow];
            const char* Text = v7->GetText(col);
            row->SetText(col, Text);
            int state = v7->mColumns.mElements[col].mState;
            row->SetItemState(col, state);
        }
        int v13 = i + mTopLine;
        bool enabled = mDataRows.mElements[v13].mEnabled;
        row->SetEnabled(enabled);
    }
    if (mSelectedLine == -1)
        mSelectedLine = 0;
    SelectRow();
    UpdateScrollBar();
}

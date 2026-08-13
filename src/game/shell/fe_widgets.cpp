// ============================================================================
// fe_widgets.cpp - FEComboBox, FESlider, FEDoubleEntry, FEMenuListBox
// (shell.o FEComboBox.cpp / FESlider.cpp / FEDoubleEntry.cpp /
// FEMenuListBox.cpp families)
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/sv/sv_stubs.h"

#include <math.h>
#include <stdio.h>

extern float sNaN;                       // ?sNaN@@3MA @ 0x10F19D0
extern const char* const defaultFileName;  // 0xCD67AE
extern void* mem_heap_malloc(unsigned int size);  // core.o
extern void mem_heap_free(void* ptr);            // core.o
extern void tlMemFree(void* ptr);                // core.o

extern FEManager g_femanager;
class STBManager {
public:
    static STBManager* sInst;
    const char* GetSTBString(const char* pszReference);
};

// ============================================================================
// FEComboBox
// ============================================================================

// ea: 0x57E0E0
FEComboBox::FEComboBox(FEMenu* parent, short maxOptions, FEText* text,
                       FEText* label, PanelQuad* leftArrow,
                       PanelQuad* rightArrow)
    : FEMenuEntry()
{
    CommonConstructor(text, parent);
    must_delete_text = false;
    mMaxOptions = maxOptions;
    mCurrOption = 0;
    mCachedOption = 0;
    mNumOptions = 0;
    mSound.mVal = 0;
    mEnableSound = true;
    mLabel = label;
    mScrollBarLeftFader.mQuad = nullptr;
    mScrollBarLeftFader.mAlphaTo = 1.0f;
    mScrollBarLeftFader.mTime = 0.0f;
    mScrollBarLeftFader.mAlphaDelta = 0.0f;
    mScrollBarLeftFader.mAlpha = 0.0f;
    mScrollBarLeftFader.mFading = false;
    mScrollBarRightFader.mQuad = nullptr;
    mScrollBarRightFader.mAlphaTo = 1.0f;
    mScrollBarRightFader.mTime = 0.0f;
    mScrollBarRightFader.mAlphaDelta = 0.0f;
    mScrollBarRightFader.mAlpha = 0.0f;
    mScrollBarRightFader.mFading = false;
    int* v9 = (int*)mem_heap_malloc(4 * mMaxOptions + 4);
    if (v9 != nullptr)
    {
        *v9 = mMaxOptions;
        mOptionStrings = (Broc::string*)(v9 + 1);
        for (int i = 0; i < mMaxOptions; ++i)
            new (&mOptionStrings[i]) Broc::string();
    }
    else
    {
        mOptionStrings = nullptr;
    }
    mScrollBarLeftFader.mQuad = leftArrow;
    mScrollBarRightFader.mQuad = rightArrow;
    if (leftArrow != nullptr)
        leftArrow->SetAlpha(0.5f);
    if (rightArrow != nullptr)
        rightArrow->SetAlpha(0.5f);
    if (label != nullptr)
        label->AddedToMenu(true);
}

// ea: 0x58E180
FEComboBox::~FEComboBox()
{
    if (mOptionStrings != nullptr)
    {
        int count = ((int*)mOptionStrings)[-1];
        for (int i = 0; i < count; ++i)
            (&mOptionStrings[i])->~string();
        mem_heap_free((int*)mOptionStrings - 1);
    }
    if (mSound.mVal != 0)
    {
        unsigned int mVal = mSound.mVal;
        unsigned int v5 = mVal & 0xFFF;
        if (v5 < 0x200
            && (mVal >> 12)
                   == SoundDevice::SoundHandleDb::sInst.mElements[v5].mKey
            && SoundDevice::SoundHandleDb::sInst.mElements[v5].mObject
                   != nullptr)
        {
            SoundDevice::sInst->ReleaseSound(
                SoundDevice::SoundHandleDb::sInst.mElements[v5].mObject);
        }
    }
    if (must_delete_text && text != nullptr)
        delete text;
}

// ea: 0x57E230
void FEComboBox::PlayNavigationSound()
{
    if (mEnableSound)
    {
        math::Position3 v2;
        math::Dir3 v3;
        memset(&v2, 0, sizeof(v2));
        memset(&v3, 0, sizeof(v3));
        mSound.mVal = SoundDevice::sInst
                          ->PlaySound("UI_Select",
                                      DbLinkedHandle<EntityHandleDb, Entity>(),
                                      true, false, v2, v3, -1.0f, -1.0f,
                                      -1.0f, -1.0f)
                          .mHandle.mVal;
    }
}

// ea: 0x586260
short FEComboBox::OnLeft()
{
    if (--mCurrOption < 0)
        mCurrOption = (short)(mNumOptions - 1);
    if (mScrollBarLeftFader.mQuad != nullptr)
    {
        mScrollBarLeftFader.mAlpha = 1.0f;
        mScrollBarLeftFader.mFading = true;
        mScrollBarLeftFader.mAlphaTo = 0.5f;
        mScrollBarLeftFader.mTime = 0.5f;
        mScrollBarLeftFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarLeftFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarLeftFader.mFading = false;
    }
    PlayNavigationSound();
    Broc::string::Block* mBlock = mOptionStrings[mCurrOption].mBlock;
    text->SetTextNoLocalize(mBlock != nullptr
                                ? (const char*)&mBlock[1]
                                : defaultFileName);
    return -1;
}

// ea: 0x586310
short FEComboBox::OnRight()
{
    if (++mCurrOption >= mNumOptions)
        mCurrOption = 0;
    if (mScrollBarRightFader.mQuad != nullptr)
    {
        mScrollBarRightFader.mAlpha = 1.0f;
        mScrollBarRightFader.mFading = true;
        mScrollBarRightFader.mAlphaTo = 0.5f;
        mScrollBarRightFader.mTime = 0.5f;
        mScrollBarRightFader.mAlphaDelta = fabsf(0.5f);
        mScrollBarRightFader.mQuad->SetAlpha(1.0f);
    }
    else
    {
        mScrollBarRightFader.mFading = false;
    }
    PlayNavigationSound();
    Broc::string::Block* mBlock = mOptionStrings[mCurrOption].mBlock;
    text->SetTextNoLocalize(mBlock != nullptr
                                ? (const char*)&mBlock[1]
                                : defaultFileName);
    return -1;
}

// ea: 0x5714F0
void FEComboBox::Draw()
{
    if (text != nullptr)
        text->Draw(highlight);
    if (mLabel != nullptr)
        mLabel->Draw(highlight);
}

// ea: 0x571520
void FEComboBox::SetWidgets(FEText* copy_this, FEText* label,
                            PanelQuad* leftArrow, PanelQuad* rightArrow)
{
    text = copy_this;
    mLabel = label;
    mScrollBarLeftFader.mQuad = leftArrow;
    mScrollBarRightFader.mQuad = rightArrow;
    if (leftArrow != nullptr)
        leftArrow->SetAlpha(0.5f);
    if (rightArrow != nullptr)
        rightArrow->SetAlpha(0.5f);
}

// ea: 0x571570
void FEComboBox::AddOption(Broc::string optionString)
{
    if (mNumOptions >= mMaxOptions)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEComboBox.cpp";
        AeAssert::gCurrentLine = 113;
        AeAssert::gCurrentExpr = "mNumOptions < mMaxOptions";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mNumOptions < mMaxOptions)
    {
        const char* v3 = optionString.mBlock != nullptr
                             ? (const char*)&optionString.mBlock[1]
                             : defaultFileName;
        const char* STBString = STBManager::sInst->GetSTBString(v3);
        mOptionStrings[mNumOptions] = STBString;
        if (mNumOptions == 0)
        {
            Broc::string::Block* mBlock = mOptionStrings->mBlock;
            text->SetTextNoLocalize(mBlock != nullptr
                                        ? (const char*)&mBlock[1]
                                        : defaultFileName);
        }
        ++mNumOptions;
    }
}

// ea: 0x571660
void FEComboBox::ClearOptions()
{
    Broc::string* v2 = nullptr;
    if (mNumOptions > 0)
    {
        Broc::string* mOptionStrings = this->mOptionStrings;
        mNumOptions = 0;
        if (mOptionStrings != nullptr)
        {
            int count = ((int*)mOptionStrings)[-1];
            for (int i = 0; i < count; ++i)
                (&mOptionStrings[i])->~string();
            mem_heap_free((int*)mOptionStrings - 1);
        }
        int* v6 = (int*)mem_heap_malloc(4 * mMaxOptions + 4);
        if (v6 != nullptr)
        {
            v2 = (Broc::string*)(v6 + 1);
            *v6 = mMaxOptions;
            for (int i = 0; i < mMaxOptions; ++i)
                new (&v2[i]) Broc::string();
        }
        mOptionStrings = v2;
    }
}

// ea: 0x571700
void FEComboBox::SetOption(int index, Broc::string optionString)
{
    if (index >= mMaxOptions)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEComboBox.cpp";
        AeAssert::gCurrentLine = 140;
        AeAssert::gCurrentExpr = "index < mMaxOptions";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < 0)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEComboBox.cpp";
        AeAssert::gCurrentLine = 141;
        AeAssert::gCurrentExpr = "index >= 0";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (index < mMaxOptions)
    {
        const char* v4 = optionString.mBlock != nullptr
                             ? (const char*)&optionString.mBlock[1]
                             : defaultFileName;
        const char* STBString = STBManager::sInst->GetSTBString(v4);
        mOptionStrings[index] = STBString;
    }
}

// ea: 0x571810
void FEComboBox::Update(float time_inc)
{
    if (text != nullptr)
        text->Update(time_inc);
    if (mLabel != nullptr)
        mLabel->Update(time_inc);
    if (mScrollBarLeftFader.mQuad != nullptr && mScrollBarLeftFader.mFading)
    {
        if (mScrollBarLeftFader.mAlphaTo <= mScrollBarLeftFader.mAlpha)
        {
            if (mScrollBarLeftFader.mAlpha > mScrollBarLeftFader.mAlphaTo)
            {
                mScrollBarLeftFader.mAlpha -=
                    (time_inc / mScrollBarLeftFader.mTime)
                    * mScrollBarLeftFader.mAlphaDelta;
                if (mScrollBarLeftFader.mAlphaTo
                    < mScrollBarLeftFader.mAlpha)
                    goto LABEL_14;
            }
            mScrollBarLeftFader.mAlpha = mScrollBarLeftFader.mAlphaTo;
            mScrollBarLeftFader.mFading = false;
        }
        else
        {
            mScrollBarLeftFader.mAlpha +=
                (time_inc / mScrollBarLeftFader.mTime)
                * mScrollBarLeftFader.mAlphaDelta;
            if (mScrollBarLeftFader.mAlpha
                >= mScrollBarLeftFader.mAlphaTo)
            {
                mScrollBarLeftFader.mAlpha =
                    mScrollBarLeftFader.mAlphaTo;
                mScrollBarLeftFader.mFading = false;
            }
        }
    LABEL_14:
        mScrollBarLeftFader.mQuad->SetAlpha(
            mScrollBarLeftFader.mAlpha);
    }
    if (mScrollBarRightFader.mQuad != nullptr
        && mScrollBarRightFader.mFading)
    {
        if (mScrollBarRightFader.mAlphaTo <= mScrollBarRightFader.mAlpha)
        {
            if (mScrollBarRightFader.mAlpha
                > mScrollBarRightFader.mAlphaTo)
            {
                mScrollBarRightFader.mAlpha -=
                    (time_inc / mScrollBarRightFader.mTime)
                    * mScrollBarRightFader.mAlphaDelta;
                if (mScrollBarRightFader.mAlphaTo
                    < mScrollBarRightFader.mAlpha)
                    goto LABEL_24;
            }
            mScrollBarRightFader.mAlpha =
                mScrollBarRightFader.mAlphaTo;
            mScrollBarRightFader.mFading = false;
        }
        else
        {
            mScrollBarRightFader.mAlpha +=
                (time_inc / mScrollBarRightFader.mTime)
                * mScrollBarRightFader.mAlphaDelta;
            if (mScrollBarRightFader.mAlpha
                >= mScrollBarRightFader.mAlphaTo)
            {
                mScrollBarRightFader.mAlpha =
                    mScrollBarRightFader.mAlphaTo;
                mScrollBarRightFader.mFading = false;
            }
        }
    LABEL_24:
        mScrollBarRightFader.mQuad->SetAlpha(
            mScrollBarRightFader.mAlpha);
    }
}

// ea: 0x571950
void FEComboBox::AddOptionNoLocalize(Broc::string optionString)
{
    if (mNumOptions >= mMaxOptions)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEComboBox.cpp";
        AeAssert::gCurrentLine = 167;
        AeAssert::gCurrentExpr = "mNumOptions < mMaxOptions";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mNumOptions < mMaxOptions)
    {
        mOptionStrings[mNumOptions] = optionString;
        if (mNumOptions == 0)
        {
            Broc::string::Block* mBlock = mOptionStrings->mBlock;
            text->SetTextNoLocalize(mBlock != nullptr
                                        ? (const char*)&mBlock[1]
                                        : defaultFileName);
        }
        ++mNumOptions;
    }
}

// ea: 0x571A30
void FEComboBox::SetCurrOption(short option)
{
    mCurrOption = option;
    Broc::string::Block* mBlock = mOptionStrings[option].mBlock;
    text->SetTextNoLocalize(mBlock != nullptr
                                ? (const char*)&mBlock[1]
                                : defaultFileName);
}

// ea: 0x571A80
void FEComboBox::Highlight(bool h, bool anim)
{
    highlight = h;
    AdjustColor();
    if (highlight)
        OnHighlight(anim);
    if (mLabel != nullptr)
        FEMenuEntry::AdjustColor(mLabel);
}

// ea: 0x571AC0
void FEComboBox::MoveForSplitScreen(int viewport, int old_viewport)
{
    text->MoveForSplitScreen(viewport, old_viewport);
    if (mLabel != nullptr)
        mLabel->MoveForSplitScreen(viewport, old_viewport);
}

// ea: 0x571B00
void FEComboBox::AdjustColor()
{
    FEMenuEntry::AdjustColor(text);
    if (mLabel != nullptr)
        FEMenuEntry::AdjustColor(mLabel);
}

// ea: 0x5B3590
void FEComboBox::SetShown(bool on)
{
    text->SetShown(on);
    if (mLabel != nullptr)
        mLabel->SetShown(on);
}

// ea: 0x5B3580
int FEComboBox::GetValue()
{
    return mCurrOption;
}

// ea: 0x5B35D0
void FEComboBox::SetValue(int value)
{
    mCurrOption = (short)value;
    Broc::string::Block* mBlock = mOptionStrings[value].mBlock;
    text->SetTextNoLocalize(mBlock != nullptr
                                ? (const char*)&mBlock[1]
                                : defaultFileName);
}

// ============================================================================
// FESlider
// ============================================================================

// ea: 0x57E2B0
FESlider::FESlider(FEMenu* parent, PanelQuad* bar, FEText* label,
                   FEText* barText)
    : FEMenuEntry()
{
    CommonConstructor(label, parent);
    must_delete_text = false;
    mValue = 50;
    mMin = 0;
    mMax = 100;
    mSound.mVal = 0;
    mEnableSound = true;
    mBarText = barText;
    mBar = bar;
    mNumeric = true;
    if (bar != nullptr)
        mNumeric = false;
    if (barText != nullptr)
        barText->AddedToMenu(true);
}

// ea: 0x58E270
FESlider::~FESlider()
{
    if (mSound.mVal != 0)
    {
        unsigned int mVal = mSound.mVal;
        unsigned int v3 = mVal & 0xFFF;
        if (v3 < 0x200
            && (mVal >> 12)
                   == SoundDevice::SoundHandleDb::sInst.mElements[v3].mKey
            && SoundDevice::SoundHandleDb::sInst.mElements[v3].mObject
                   != nullptr)
        {
            SoundDevice::sInst->ReleaseSound(
                SoundDevice::SoundHandleDb::sInst.mElements[v3].mObject);
        }
    }
    if (must_delete_text && text != nullptr)
        delete text;
}

// ea: 0x57E350
void FESlider::AdjustBar()
{
    if (mBar != nullptr)
        mBar->Mask((float)mValue / mMax, RIGHT_MASK, 1.0f);
    if (mBarText != nullptr)
    {
        char buffer[8];
        sprintf(buffer, "%i%c", mValue, 37);
        mBarText->SetTextNoLocalize(buffer);
    }
}

// ea: 0x57E3C0
void FESlider::PlayNavigationSound()
{
    if (mEnableSound)
    {
        math::Position3 v2;
        math::Dir3 v3;
        memset(&v2, 0, sizeof(v2));
        memset(&v3, 0, sizeof(v3));
        mSound.mVal = SoundDevice::sInst
                          ->PlaySound("UI_Select",
                                      DbLinkedHandle<EntityHandleDb, Entity>(),
                                      true, false, v2, v3, -1.0f, -1.0f,
                                      -1.0f, -1.0f)
                          .mHandle.mVal;
    }
}

// ea: 0x57E440
void FESlider::MoveForSplitScreen(int viewport, int old_viewport)
{
    text->MoveForSplitScreen(viewport, old_viewport);
    if (mBar != nullptr)
        mBar->MoveForSplitScreen(viewport, old_viewport);
}

// ea: 0x57E470
void FESlider::SetWidgets(PanelQuad* bar, FEText* label, FEText* barText)
{
    SetText(label);
    if (bar != nullptr)
        mBar = bar;
    if (barText != nullptr)
        mBarText = barText;
    AdjustBar();
}

// ea: 0x571B30
void FESlider::Draw()
{
    if (text != nullptr)
        text->Draw(highlight);
    if (mBar != nullptr)
        mBar->Draw();
    if (mBarText != nullptr)
        mBarText->Draw();
}

// ea: 0x571B70
void FESlider::Update(float time_inc)
{
    if (text != nullptr)
        text->Update(time_inc);
    if (mBar != nullptr)
        mBar->Update(time_inc);
    if (mBarText != nullptr)
        mBarText->Update(time_inc);
}

// ea: 0x571BB0
void FESlider::SetRange(int min, int max)
{
    mMin = min;
    mMax = max;
}

// ea: 0x571BD0
void FESlider::Highlight(bool h, bool anim)
{
    highlight = h;
    AdjustColor();
    if (highlight)
        OnHighlight(anim);
    if (mBarText != nullptr)
        FEMenuEntry::AdjustColor(mBarText);
}

// ea: 0x571C10
void FESlider::UpdateWidescreen(bool widescreen)
{
    if (mBar != nullptr)
        mBar->SetXYInitialToCurrentPos();
}

// ea: 0x5863C0
short FESlider::OnLeft()
{
    mValue = mValue - 1;
    if (mValue < mMin)
        mValue = mMin;
    AdjustBar();
    PlayNavigationSound();
    return -1;
}

// ea: 0x5863F0
short FESlider::OnRight()
{
    mValue = mValue + 1;
    if (mValue >= mMax)
        mValue = mMax;
    AdjustBar();
    PlayNavigationSound();
    return -1;
}

// ea: 0x5B3640
void FESlider::SetShown(bool on)
{
    text->SetShown(on);
    if (mBar != nullptr)
        mBar->SetShown(on);
    if (mBarText != nullptr)
        mBarText->SetShown(on);
}

// ea: 0x5B3630
int FESlider::GetValue()
{
    return mValue;
}

// ea: 0x5B5750
void FESlider::SetValue(int value)
{
    mValue = value;
    AdjustBar();
}

// ============================================================================
// FEDoubleEntry
// ============================================================================

// ea: 0x57E4B0
FEDoubleEntry::FEDoubleEntry(FEMenu* parent, FEText* label, FEText* text)
    : FEMenuEntry()
{
    CommonConstructor(text, parent);
    must_delete_text = false;
    mLabel = label;
    if (label != nullptr)
        label->AddedToMenu(true);
}

// ea: 0x571C30
FEDoubleEntry::~FEDoubleEntry()
{
    if (must_delete_text && text != nullptr)
        delete text;
}

// ea: 0x571C50
void FEDoubleEntry::Draw()
{
    if (text != nullptr)
        text->Draw(highlight);
    if (mLabel != nullptr)
        mLabel->Draw(highlight);
}

// ea: 0x571C80
void FEDoubleEntry::Update(float time_inc)
{
    if (text != nullptr)
        text->Update(time_inc);
    if (mLabel != nullptr)
        mLabel->Update(time_inc);
}

// ea: 0x571CB0
void FEDoubleEntry::Highlight(bool h, bool anim)
{
    highlight = h;
    AdjustColor();
    if (highlight)
        OnHighlight(anim);
    if (mLabel != nullptr)
        FEMenuEntry::AdjustColor(mLabel);
}

// ea: 0x571CF0
void FEDoubleEntry::MoveForSplitScreen(int viewport, int old_viewport)
{
    text->MoveForSplitScreen(viewport, old_viewport);
    if (mLabel != nullptr)
        mLabel->MoveForSplitScreen(viewport, old_viewport);
}

// ea: 0x571D30
void FEDoubleEntry::AdjustColor()
{
    FEMenuEntry::AdjustColor(text);
    if (mLabel != nullptr)
        FEMenuEntry::AdjustColor(mLabel);
}

// ea: 0x5B3690
void FEDoubleEntry::SetShown(bool on)
{
    text->SetShown(on);
    if (mLabel != nullptr)
        mLabel->SetShown(on);
}

// ============================================================================
// FEMenuListBoxItem
// ============================================================================

// ea: 0x5B33F0
FEMenuListBoxItem::FEMenuListBoxItem(unsigned int index,
                                     const Broc::string& text, int data)
{
    mIndex = index;
    mText = text;
    mSubItemCount = 0;
    mData = data;
    for (int i = 0; i < 3; ++i)
        mSubItems[i] = Broc::string((Broc::string::Block*)nullptr);
}

// ea: 0x5AFAB0
FEMenuListBoxItem::~FEMenuListBoxItem()
{
    for (int i = 0; i < 3; ++i)
        (&mSubItems[i])->~string();
    (&mText)->~string();
}

// ea: 0x571D60
unsigned int FEMenuListBoxItem::AddSubItem(const Broc::string& subItemText)
{
    unsigned int mSubItemCount = this->mSubItemCount;
    if (mSubItemCount >= 3)
        return -1;
    this->mSubItemCount = mSubItemCount + 1;
    mSubItems[mSubItemCount] = subItemText;
    return this->mSubItemCount - 1;
}

// ============================================================================
// FEMenuListBox
// ============================================================================

// FEMenuListBox(FEText* t, FEMenu* m, int numLines) - inline COMDAT
FEMenuListBox::FEMenuListBox(FEText* t, FEMenu* m, int numLines)
    : FEMenuEntry()
{
    CommonConstructor(t, m);
    must_delete_text = false;
    mItems.mElements = nullptr;
    mItems.mCapacity = 0;
    mItems.mSize = 0;
    for (int i = 0; i < 4; ++i)
        mColumnHeadings[i] = Broc::string((Broc::string::Block*)nullptr);
    mColumnWidths[0] = 100.0f;
    mColumnWidths[1] = 100.0f;
    mColumnWidths[2] = 100.0f;
    mColumnWidths[3] = 100.0f;
    mHeadingSpacing = 30.0f;
    mRowHeight = 35.0f;
    mTopLine = 0;
    mSelectedLine = 0;
    mNumLines = numLines;
    mHasHeadings = false;
}

// ea: 0x5B6950
FEMenuListBox::~FEMenuListBox()
{
    Clear();
    for (int i = 0; i < 4; ++i)
        (&mColumnHeadings[i])->~string();
    if (mItems.mElements != nullptr)
    {
        tlMemFree(mItems.mElements);
        mItems.mElements = nullptr;
        mItems.mCapacity = 0;
    }
    if (must_delete_text && text != nullptr)
        delete text;
}

// ea: 0x58E330
unsigned int FEMenuListBox::AddItem(const Broc::string& itemText,
                                    FEMenuListBoxItem* itemData)
{
    unsigned int mSize = mItems.mSize;
    FEMenuListBoxItem* v5 =
        (FEMenuListBoxItem*)mem_heap_malloc(0x1Cu);
    FEMenuListBoxItem* v6;
    if (v5 != nullptr)
        v6 = new (v5) FEMenuListBoxItem(mSize, itemText, (int)itemData);
    else
        v6 = nullptr;
    FEMenuListBoxItem* iElement = v6;
    if (mItems.mSize >= mItems.mCapacity)
    {
        int v4 = mItems.mSize + 4;
        if (mItems.mSize <= 3)
            v4 = mItems.mSize + 1;
        FEMenuListBoxItem** v7 =
            (FEMenuListBoxItem**)mem_heap_malloc(4 * v4);
        for (int i = 0; i < mItems.mSize; ++i)
            v7[i] = mItems.mElements[i];
        if (mItems.mElements != nullptr)
            mem_heap_free(mItems.mElements);
        mItems.mElements = v7;
        mItems.mCapacity = v4;
    }
    mItems.mElements[mItems.mSize++] = iElement;
    return mSize;
}

// ea: 0x57E520
unsigned int FEMenuListBox::AddSubItem(unsigned int itemIndex,
                                       const Broc::string& subItemText)
{
    if (itemIndex >= mItems.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenuListBox.cpp";
        AeAssert::gCurrentLine = 34;
        AeAssert::gCurrentExpr = "itemIndex < mItems.size()";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    FEMenuListBoxItem* v5 = mItems.mElements[itemIndex];
    unsigned int mSubItemCount = v5->mSubItemCount;
    if (mSubItemCount >= 3)
        return -1;
    v5->mSubItemCount = mSubItemCount + 1;
    v5->mSubItems[mSubItemCount] = subItemText;
    return v5->mSubItemCount - 1;
}

// ea: 0x57E5B0
void FEMenuListBox::Draw()
{
    float xStart = text->GetX();
    float yStart = text->GetY();
    if (mHasHeadings)
    {
        highlight = false;
        AdjustColor();
        for (int i = 0; i < 4; ++i)
        {
            Broc::string::Block* mBlock = mColumnHeadings[i].mBlock;
            text->SetTextNoLocalize(mBlock != nullptr
                                        ? (const char*)&mBlock[1]
                                        : defaultFileName);
            text->Draw(highlight);
            text->SetX(text->GetX() + mColumnWidths[i]);
        }
        text->SetY(text->GetY() + mHeadingSpacing);
    }
    int ia = 0;
    if (mTopLine < mItems.mSize)
    {
        for (int v9 = 0; v9 < mNumLines; ++v9)
        {
            int v10 = v9 + mTopLine;
            if (v10 == mSelectedLine)
            {
                highlight = true;
                AdjustColor();
            }
            unsigned int v12 = 0;
            Broc::string::Block* mBlock =
                mItems.mElements[v10]->mText.mBlock;
            text->SetTextNoLocalize(mBlock != nullptr
                                        ? (const char*)&mBlock[1]
                                        : defaultFileName);
            text->SetX(xStart);
            text->Draw(highlight);
            for (int j = 0; v12 < mItems.mElements[v10]->mSubItemCount;
                 ++j)
            {
                Broc::string::Block* subBlock =
                    mItems.mElements[v10]->mSubItems[v12].mBlock;
                text->SetTextNoLocalize(
                    subBlock != nullptr ? (const char*)&subBlock[1]
                                        : defaultFileName);
                text->SetX(text->GetX() + mColumnWidths[j]);
                text->Draw(highlight);
                ++v12;
            }
            text->SetY(text->GetY() + mRowHeight);
            if (v10 == mSelectedLine)
            {
                highlight = false;
                AdjustColor();
            }
            ia = v9 + 1;
            if (ia + mTopLine >= mItems.mSize)
                break;
        }
    }
    text->SetX(xStart);
    text->SetY(yStart);
}

// ea: 0x57E8F0
short FEMenuListBox::OnUp()
{
    if (mSelectedLine != 0)
    {
        unsigned int v4 = mSelectedLine - 1;
        mSelectedLine = v4;
        if (v4 < (unsigned int)mTopLine)
            mTopLine = v4;
    }
    math::Position3 v7;
    math::Dir3 v8;
    memset(&v7, 0, sizeof(v7));
    memset(&v8, 0, sizeof(v8));
    SoundDevice::sInst->PlaySound(
        "UI_Highlight", DbLinkedHandle<EntityHandleDb, Entity>(), true, false,
        v7, v8, -1.0f, -1.0f, -1.0f, -1.0f);
    return -1;
}

// ea: 0x57E970
short FEMenuListBox::OnDown()
{
    if (mSelectedLine < mItems.mSize - 1)
    {
        unsigned int v4 = mSelectedLine + 1;
        mSelectedLine = v4;
        if (v4 >= (unsigned int)(mNumLines + mTopLine))
            mTopLine = v4 - mNumLines + 1;
    }
    math::Position3 v7;
    math::Dir3 v8;
    memset(&v7, 0, sizeof(v7));
    memset(&v8, 0, sizeof(v8));
    SoundDevice::sInst->PlaySound(
        "UI_Highlight", DbLinkedHandle<EntityHandleDb, Entity>(), true, false,
        v7, v8, -1.0f, -1.0f, -1.0f, -1.0f);
    return -1;
}

// ea: 0x57EA00
void FEMenuListBox::SetCurrentSelection(int iCurrentSelection)
{
    if (iCurrentSelection < 0 || iCurrentSelection >= mItems.mSize)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenuListBox.cpp";
        AeAssert::gCurrentLine = 160;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert(
                "FEMenuListBox::SetCurrentSelection() - received invalid value\n"))
            __debugbreak();
    }
    else
    {
        mSelectedLine = iCurrentSelection;
    }
}

// ea: 0x57EA60
unsigned int FEMenuListBox::GetCurrentSelection()
{
    if (mSelectedLine >= mItems.mSize)
        return -1;
    return mItems.mElements[mSelectedLine]->mIndex;
}

// ea: 0x57EA80
int FEMenuListBox::GetCurrentSelectionData()
{
    if (mSelectedLine >= mItems.mSize)
        return 0;
    return mItems.mElements[mSelectedLine]->mData;
}

// ea: 0x571DF0
void FEMenuListBox::SetColumnWidth(unsigned int column, float width)
{
    if (column > 3)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenuListBox.cpp";
        AeAssert::gCurrentLine = 107;
        AeAssert::gCurrentExpr = "column <= 3";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    mColumnWidths[column] = width;
}

// ea: 0x571E60
void FEMenuListBox::SetColumnHeading(unsigned int column,
                                     const char* heading)
{
    if (column > 3)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\FEMenuListBox.cpp";
        AeAssert::gCurrentLine = 113;
        AeAssert::gCurrentExpr = "column <= 3";
        if (!AeAssert::IsIgnored() && AeAssert::Assert(defaultFileName))
            __debugbreak();
    }
    mColumnHeadings[column] = heading;
    mHasHeadings = true;
}

// ea: 0x571ED0
void FEMenuListBox::Sort(unsigned int column)
{
    (void)column;
}

// ea: 0x571EE0
void FEMenuListBox::FormatForSplitScreen(int viewport, int old_viewport)
{
    float x_trans = GetX();
    float y_trans = GetY();
    GetScaleX();
    GetScaleY();
    switch (old_viewport)
    {
    case 3:
        goto L151666;
    case 4:
        goto L151667;
    case 5:
        x_trans = (x_trans * 0.003125f) * 640.0f;
        goto L151666;
    case 6:
        x_trans = ((x_trans - 320.0f) * 0.003125f) * 640.0f;
        goto L151666;
    case 7:
        x_trans = (x_trans * 0.003125f) * 640.0f;
        goto L151667;
    case 8:
        x_trans = ((x_trans - 320.0f) * 0.003125f) * 640.0f;
    L151667:
        y_trans = ((y_trans - 240.0f) * 0.0041666669f) * 480.0f;
        break;
    L151666:
        y_trans = (y_trans * 0.0041666669f) * 480.0f;
        break;
    default:
        break;
    }
    SetPos(x_trans, y_trans);
    float x_transa = GetX();
    float y_transa = GetY();
    float x_scale = GetScaleX();
    GetScaleY();
    switch (viewport)
    {
    case 0:
        x_scale = 0.5f;
        break;
    case 3:
        x_scale = 0.40000001f;
        y_transa = (y_transa * 0.0020833334f) * 240.0f;
        break;
    case 4:
        x_scale = 0.40000001f;
        goto L151667_2;
    case 5:
        x_scale = 0.30000001f;
        x_transa = (x_transa * 0.0015625f) * 320.0f;
        y_transa = (y_transa * 0.0020833334f) * 240.0f;
        break;
    case 6:
        x_scale = 0.30000001f;
        x_transa = ((x_transa * 0.0015625f) + 1.0f) * 320.0f;
        y_transa = (y_transa * 0.0020833334f) * 240.0f;
        break;
    case 7:
        x_scale = 0.30000001f;
        x_transa = (x_transa * 0.0015625f) * 320.0f;
    L151667_2:
        y_transa = ((y_transa * 0.0020833334f) + 1.0f) * 240.0f;
        break;
    case 8:
        x_scale = 0.30000001f;
        x_transa = ((x_transa * 0.0015625f) + 1.0f) * 320.0f;
        y_transa = ((y_transa * 0.0020833334f) + 1.0f) * 240.0f;
        break;
    default:
        break;
    }
    SetScale(x_scale);
    SetPos(x_transa, y_transa);
}

// ea: 0x586420
void FEMenuListBox::Clear()
{
    while (mItems.mSize > 0)
    {
        FEMenuListBoxItem* v4 = mItems.mElements[mItems.mSize - 1];
        if (v4 != nullptr)
        {
            v4->~FEMenuListBoxItem();
            mem_heap_free(v4);
        }
        if (mItems.mSize != 0)
            mItems.mSize = mItems.mSize - 1;
    }
    mTopLine = 0;
    mSelectedLine = 0;
}

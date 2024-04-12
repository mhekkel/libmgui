/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2023 Maarten L. Hekkelman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "MControlsImpl.hpp"
#include "MGtkWidgetMixin.hpp"
#include "MGtkWindowImpl.hpp"

#include <vector>

template <class CONTROL>
class MGtkControlImpl : public CONTROL::MImpl, public MGtkWidgetMixin
{
  public:
	MGtkControlImpl(CONTROL *inControl, const std::string &inLabel);
	~MGtkControlImpl();

	void RequestSize(int32_t inWidth, int32_t inHeight) override
	{
		MGtkWidgetMixin::RequestSize(inWidth, inHeight);
	}

	bool IsFocus() const override;
	void SetFocus() override;

	void AddedToWindow() override;
	void EnableSelf() override;
	void DisableSelf() override;
	void ShowSelf() override;
	void HideSelf() override;

	std::string GetText() const override;
	void SetText(const std::string &inText) override;

	GObject *GetActionMapObject() override
	{
		if (auto w = this->mControl->GetWindow(); w != nullptr)
			return G_OBJECT(static_cast<MGtkWindowImpl *>(w->GetImpl())->GetWidget());
		return nullptr;
	}


  protected:
	void GetParentAndBounds(MGtkWidgetMixin *&outParent, MRect &outBounds);

	virtual void CreateWidget() = 0;

	void OnDestroy() override;

	virtual void OnChanged();
	MSlot<void()> mChanged;

	std::string mLabel;
};

// actual implementations

class MGtkSimpleControlImpl : public MGtkControlImpl<MSimpleControl>
{
  public:
	MGtkSimpleControlImpl(MSimpleControl *inControl);
	void CreateWidget() override;
	void Append(MGtkWidgetMixin *inChild) override;
};

class MGtkButtonImpl : public MGtkControlImpl<MButton>
{
  public:
	MGtkButtonImpl(MButton *inButton, const std::string &inLabel,
		MButtonFlags inFlags);

	void SimulateClick() override;
	void MakeDefault(bool inDefault) override;

	void SetText(const std::string &inText) override;

	void CreateWidget() override;
	void GetIdealSize(int32_t &outWidth, int32_t &outHeight) override;

	MSlot<void()> mClicked;
	void Clicked();

  private:
	// MButtonFlags	mFlags;
	bool mDefault;
};

// class MGtkImageButtonImpl : public MGtkControlImpl<MImageButton>
//{
//   public:
//					MGtkImageButtonImpl(MImageButton* inButton, const std::string& inImageResource);
//
//	void	CreateWidget() override;
//
//	MSlot<void()>	mClicked;
//	void			Clicked();
//
//	MBitmap			mBitmaps[3];
// };

class MGtkExpanderImpl : public MGtkControlImpl<MExpander>
{
  public:
	MGtkExpanderImpl(MExpander *inExpander, const std::string &inLabel);

	void SetOpen(bool inOpen) override;
	bool IsOpen() const override;

	void CreateWidget() override;
	void AddedToWindow() override;

	void Append(MGtkWidgetMixin *inChild) override;

  private:
	bool mIsOpen;
};

class MGtkScrollbarImpl : public MGtkControlImpl<MScrollbar>
{
  public:
	MGtkScrollbarImpl(MScrollbar *inScrollbar);

	void CreateWidget() override;

	int32_t GetValue() const override;
	void SetValue(int32_t inValue) override;

	int32_t GetTrackValue() const override;

	void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
		int32_t inScrollUnit, int32_t inPageSize, int32_t inValue) override;

	int32_t GetMinValue() const override;
	int32_t GetMaxValue() const override;

	MSlot<void()> eValueChanged;
	void ValueChanged();
};

class MGtkStatusbarImpl : public MGtkControlImpl<MStatusbar>
{
  public:
	MGtkStatusbarImpl(MStatusbar *inControl, uint32_t inPartCount, MStatusBarElement inParts[]);

	void CreateWidget() override;
	void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder) override;

  private:
	std::vector<MStatusBarElement> mParts;
	std::vector<GtkWidget *> mPanels;

	void OnGestureClickPressed(double inX, double inY, gint inClickCount) override;
};

class MGtkComboboxImpl : public MGtkControlImpl<MCombobox>
{
  public:
	MGtkComboboxImpl(MCombobox *inCombobox);

	void CreateWidget() override;
	void AddedToWindow() override;

	std::string GetText() const override;
	void SetText(const std::string &inText) override;

	void SetChoices(const std::vector<std::string> &inChoices) override;

	int GetActive() override;
	void SetActive(int inActive) override;

	void OnChanged() override;

  private:
	std::vector<std::string> mChoices;
};

class MGtkPopupImpl : public MGtkControlImpl<MPopup>
{
  public:
	MGtkPopupImpl(MPopup *inPopup);

	void SetChoices(const std::vector<std::string> &inChoices) override;

	int32_t GetValue() const override;
	void SetValue(int32_t inValue) override;

	void SetText(const std::string &inText) override;
	std::string GetText() const override;

	void CreateWidget() override;
	void AddedToWindow() override;

  private:
	std::vector<std::string>
		mChoices;
};

class MGtkEdittextImpl : public MGtkControlImpl<MEdittext>
{
  public:
	MGtkEdittextImpl(MEdittext *inEdittext, uint32_t inFlags);

	void CreateWidget() override;

	std::string GetText() const override;
	void SetText(const std::string &inText) override;

	uint32_t GetFlags() const override { return mFlags; }

	void SetPasswordChar(uint32_t inUnicode) override;

	bool OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers) override;

  protected:

	MSlot<void(guint, gchar*, guint)> mTextInserted;
	MSlot<void(guint, guint)> mTextDeleted;

	void TextInserted(guint, gchar*, guint);
	void TextDeleted(guint, guint);

	GtkEntryBuffer *mBuffer;
	uint32_t mFlags;
};

class MGtkCaptionImpl : public MGtkControlImpl<MCaption>
{
  public:
	MGtkCaptionImpl(MCaption *inControl, const std::string &inText);

	void CreateWidget() override;

	void SetText(const std::string &inText) override;
};

class MGtkSeparatorImpl : public MGtkControlImpl<MSeparator>
{
  public:
	MGtkSeparatorImpl(MSeparator *inControl);
	void CreateWidget() override;
};

class MGtkCheckboxImpl : public MGtkControlImpl<MCheckbox>
{
  public:
	MGtkCheckboxImpl(MCheckbox *inControl, const std::string &inText);

	void CreateWidget() override;

	bool IsChecked() const override;
	void SetChecked(bool inChecked) override;

  private:
	bool mChecked;

	MSlot<void()> mToggled;
	void Toggled();
};

class MGtkRadiobuttonImpl : public MGtkControlImpl<MRadiobutton>
{
  public:
	MGtkRadiobuttonImpl(MRadiobutton *inControl, const std::string &inText);

	void CreateWidget() override;

	void SetGroup(MRadiobuttonImpl *inGroup) override
	{
		mGroup = static_cast<MGtkRadiobuttonImpl *>(inGroup);
	}

	bool IsChecked() const override;
	void SetChecked(bool inChecked) override;

  private:
	bool mChecked;

	MSlot<void()> mToggled;
	void Toggled();

	MGtkRadiobuttonImpl *mGroup = nullptr;
};

// class MGtkColorSwatchImpl : public MGtkControlImpl<MColorSwatch>
// {
//   public:
// 	MGtkColorSwatchImpl(MColorSwatch *inColorSwatch, MColor inColor);

// 	void CreateWidget() override;

// 	MColor GetColor() const override;
// 	void SetColor(MColor inColor) override;

// 	void SetPalette(const std::vector<MColor> &colors) override;

//   private:
// 	MEventIn<void(MColor)> eSelectedColor;
// 	void SelectedColor(MColor inColor);

// 	MEventIn<void(MColor)> ePreviewColor;
// 	void PreviewColor(MColor inColor);

// 	MSlot<void()> mColorSet;
// 	void OnColorSet();

// 	MColor mColor;
// 	std::vector<MColor> mPalette;
// };

class MGtkListBoxImpl : public MGtkControlImpl<MListBox>
{
  public:
	MGtkListBoxImpl(MListBox *inListBox);

	void CreateWidget() override;
	void AddedToWindow() override;

	void AddItem(const std::string &inLabel) override;

	int32_t GetValue() const override;
	void SetValue(int32_t inValue) override;

  private:
	MSlot<void()> mSelectionChanged;

	virtual void OnSelectionChanged();

	std::vector<std::string> mItems;
	GtkListStore *mStore;
	int32_t mNr;
};

class MGtkBoxControlImpl : public MGtkControlImpl<MBoxControl>
{
  public:
	MGtkBoxControlImpl(MBoxControl *inControl, bool inHorizontal);

	void CreateWidget() override;

	void Append(MGtkWidgetMixin *inChild) override;
	void AddChild(MControlBase *inChild, MControlBase *inBefore) override;

	bool mHorizontal;
};

// --------------------------------------------------------------------

class MGtkStackControlImpl : public MGtkControlImpl<MStackControl>
{
  public:
	MGtkStackControlImpl(MStackControl *inControl);

	void CreateWidget() override;

	void AddChild(MView *inChild, const std::string &inName) override;
	void Select(const std::string &inName) override;

	void Append(MGtkWidgetMixin *inChild) override;
	
  private:
	std::map<MGtkWidgetMixin *, std::string> mNames;
};

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

#include <vector>

template <class CONTROL>
class MGtkControlImpl : public CONTROL::MImpl, public MGtkWidgetMixin
{
  public:
	MGtkControlImpl(CONTROL *inControl, const std::string &inLabel);
	virtual ~MGtkControlImpl();

	virtual bool IsFocus() const;
	virtual void SetFocus();

	virtual void AddedToWindow();
	virtual void FrameMoved();
	virtual void FrameResized();
	virtual void MarginsChanged();
	virtual void EnableSelf();
	virtual void DisableSelf();
	virtual void ShowSelf();
	virtual void HideSelf();
	virtual std::string
	GetText() const;
	virtual void SetText(const std::string &inText);

  protected:
	void GetParentAndBounds(MGtkWidgetMixin *&outParent, MRect &outBounds);

	virtual void CreateWidget() = 0;

	virtual bool OnDestroy();

	virtual bool OnKeyPressEvent(GdkEventKey *inEvent);
	virtual void OnPopupMenu();

	virtual void OnChanged();
	MSlot<void()> mChanged;

	std::string mLabel;
};

// actual implementations

class MGtkSimpleControlImpl : public MGtkControlImpl<MSimpleControl>
{
  public:
	MGtkSimpleControlImpl(MSimpleControl *inControl);
	virtual void CreateWidget();
};

class MGtkButtonImpl : public MGtkControlImpl<MButton>
{
  public:
	MGtkButtonImpl(MButton *inButton, const std::string &inLabel,
		MButtonFlags inFlags);

	virtual void SimulateClick();
	virtual void MakeDefault(bool inDefault);

	virtual void SetText(const std::string &inText);

	virtual void CreateWidget();
	virtual void GetIdealSize(int32_t &outWidth, int32_t &outHeight);

	virtual void AddedToWindow();

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
//	virtual void	CreateWidget();
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
	virtual ~MGtkExpanderImpl();

	virtual void SetOpen(bool inOpen);
	virtual bool IsOpen() const;

	virtual void CreateWidget();
	virtual void AddedToWindow();

	virtual void Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32_t inPadding);

  private:
	bool mIsOpen;
	// bool			mMouseInside;
	// bool			mMouseDown;
	// bool			mMouseTracking;
	// double			mLastExit;
};

class MGtkScrollbarImpl : public MGtkControlImpl<MScrollbar>
{
  public:
	MGtkScrollbarImpl(MScrollbar *inScrollbar);

	virtual void CreateWidget();

	//	virtual void	ShowSelf();
	//	virtual void	HideSelf();

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	virtual int32_t GetTrackValue() const;

	virtual void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
		int32_t inScrollUnit, int32_t inPageSize, int32_t inValue);

	virtual int32_t GetMinValue() const;
	//	virtual void	SetMinValue(int32_t inValue);
	virtual int32_t GetMaxValue() const;
	//	virtual void	SetMaxValue(int32_t inValue);
	//
	//	virtual void	SetViewSize(int32_t inValue);

	MSlot<void()> eValueChanged;
	void ValueChanged();
};

class MGtkStatusbarImpl : public MGtkControlImpl<MStatusbar>
{
  public:
	MGtkStatusbarImpl(MStatusbar *inControl, uint32_t inPartCount, MStatusBarElement inParts[]);

	virtual void CreateWidget();
	virtual void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder);
	virtual void AddedToWindow();

  private:
	std::vector<MStatusBarElement> mParts;
	std::vector<GtkWidget *> mPanels;

	bool Clicked(GdkEventButton *inEvent);
	MSlot<bool(GdkEventButton *)> mClicked;
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

	virtual void SetChoices(const std::vector<std::string> &inChoices);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	virtual void SetText(const std::string &inText);
	virtual std::string
	GetText() const;

	virtual void CreateWidget();
	virtual void AddedToWindow();

	virtual bool DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);

  private:
	std::vector<std::string>
		mChoices;
};

class MGtkEdittextImpl : public MGtkControlImpl<MEdittext>
{
  public:
	MGtkEdittextImpl(MEdittext *inEdittext, uint32_t inFlags);

	void CreateWidget() override;

	void SetFocus() override;

	std::string GetText() const override;
	void SetText(const std::string &inText) override;

	uint32_t GetFlags() const override { return mFlags; }

	void SetPasswordChar(uint32_t inUnicode) override;

	bool OnKeyPressEvent(GdkEventKey *inEvent) override;

  protected:
	uint32_t mFlags;
};

class MGtkCaptionImpl : public MGtkControlImpl<MCaption>
{
  public:
	MGtkCaptionImpl(MCaption *inControl, const std::string &inText);

	virtual void CreateWidget();

	virtual void SetText(const std::string &inText);
};

class MGtkSeparatorImpl : public MGtkControlImpl<MSeparator>
{
  public:
	MGtkSeparatorImpl(MSeparator *inControl);
	virtual void CreateWidget();
};

class MGtkCheckboxImpl : public MGtkControlImpl<MCheckbox>
{
  public:
	MGtkCheckboxImpl(MCheckbox *inControl, const std::string &inText);

	virtual void CreateWidget();

	//	virtual void	SubClass();
	virtual bool IsChecked() const;
	virtual void SetChecked(bool inChecked);

  private:
	bool mChecked;
};

class MGtkRadiobuttonImpl : public MGtkControlImpl<MRadiobutton>
{
  public:
	MGtkRadiobuttonImpl(MRadiobutton *inControl, const std::string &inText);

	virtual void CreateWidget();

	virtual bool IsChecked() const;
	virtual void SetChecked(bool inChecked);

	virtual void SetGroup(const std::list<MRadiobutton *> &inButtons);

  private:
	std::list<MRadiobutton *> mGroup;
};

class MGtkListHeaderImpl : public MGtkControlImpl<MListHeader>
{
  public:
	MGtkListHeaderImpl(MListHeader *inListHeader);

	virtual void CreateWidget();

	virtual void AppendColumn(const std::string &inLabel, int inWidth);
};

class MGtkNotebookImpl : public MGtkControlImpl<MNotebook>
{
  public:
	MGtkNotebookImpl(MNotebook *inControl);

	virtual void CreateWidget();
	virtual void AddedToWindow();
	virtual void FrameResized();

	virtual void AddPage(const std::string &inLabel, MView *inPage);

	virtual void SelectPage(uint32_t inPage);
	virtual uint32_t GetSelectedPage() const;

  private:
	struct MPage
	{
		std::string mTitle;
		MView *mPage;
	};

	std::vector<MPage> mPages;
};

class MGtkColorSwatchImpl : public MGtkControlImpl<MColorSwatch>
{
  public:
	MGtkColorSwatchImpl(MColorSwatch *inColorSwatch, MColor inColor);

	virtual void CreateWidget();

	virtual MColor GetColor() const;
	virtual void SetColor(MColor inColor);

	virtual void SetPalette(const std::vector<MColor> &colors);

  private:
	MEventIn<void(MColor)> eSelectedColor;
	void SelectedColor(MColor inColor);

	MEventIn<void(MColor)> ePreviewColor;
	void PreviewColor(MColor inColor);

	MSlot<void()> mColorSet;
	void OnColorSet();

	MColor mColor;
	std::vector<MColor> mPalette;
};

class MGtkListBoxImpl : public MGtkControlImpl<MListBox>
{
  public:
	MGtkListBoxImpl(MListBox *inListBox);

	virtual void CreateWidget();
	virtual void AddedToWindow();

	virtual void AddItem(const std::string &inLabel);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

  private:
	MSlot<void()> mSelectionChanged;

	virtual void OnSelectionChanged();

	std::vector<std::string> mItems;
	GtkListStore *mStore;
	int32_t mNr;
};

// class MGtkListViewImpl : public MGtkControlImpl<MListView>
// {
//   public:
// 	MGtkListViewImpl(MListView *inListView);

// 	virtual void CreateWidget();
// 	virtual void AddedToWindow();

// 	virtual void AddItem(const std::string &inLabel);

//   private:
// 	std::vector<std::string> mItems;
// 	GtkListStore *mStore;
// };

class MGtkBoxControlImpl : public MGtkControlImpl<MBoxControl>
{
  public:
	MGtkBoxControlImpl(MBoxControl *inControl,
		bool inHorizontal, bool inHomogeneous, bool inExpand, bool inFill,
		uint32_t inSpacing, uint32_t inPadding);

	virtual void CreateWidget();

	virtual void Append(MGtkWidgetMixin *inChild, MControlPacking inPacking,
		bool inExpand, bool inFill, uint32_t inPadding);

	bool mHorizontal, mHomogeneous, mExpand, mFill;
	uint32_t mSpacing, mPadding;
};

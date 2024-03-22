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

#include "MColor.hpp"
#include "MP2PEvents.hpp"
#include "MView.hpp"

struct MControlImplBase;

class MControlBase : public MView
{
  public:
	MControlBase(const std::string &inID, MRect inBounds)
		: MView(inID, inBounds)
		, mExpand(false)
	{
	}

	virtual MControlImplBase *GetControlImplBase() = 0;

	virtual bool IsFocus() const = 0;
	virtual void SetFocus() = 0;

	MRect GetMargins() const
	{
		return MRect(mLeftMargin, mTopMargin, mRightMargin, mBottomMargin);
	}

	bool GetExpand() const { return mExpand; }
	void SetExpand(bool inExpand) { mExpand = inExpand; }

	void SetLayout(bool inExpand, MRect inMargins)
	{
		mExpand = inExpand;
		SetMargins(inMargins.x, inMargins.y, inMargins.width, inMargins.height);
	}

  protected:
	bool mExpand;
};

template <class I>
class MControl : public MControlBase
{
  public:
	virtual ~MControl();

	virtual void MoveFrame(int32_t inXDelta, int32_t inYDelta);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
	virtual void SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin);

	virtual void Draw();

	virtual bool HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat)
	{
		return false;
	}

	virtual bool HandleCharacter(const std::string &inText, bool inRepeat)
	{
		return false;
	}

	virtual bool IsFocus() const;
	virtual void SetFocus();

	I *GetImpl() const { return mImpl; }
	void SetImpl(I *inImpl) { mImpl = inImpl; }

	virtual MControlImplBase *GetControlImplBase();

  protected:
	MControl(const std::string &inID, MRect inBounds, I *inImpl);

	virtual void EnableSelf();
	virtual void DisableSelf();

	virtual void ShowSelf();
	virtual void HideSelf();

	virtual void AddedToWindow();

  protected:
	MControl(const MControl &) = delete;
	MControl &operator=(const MControl &) = delete;

	I *mImpl;
};

// --------------------------------------------------------------------

class MSimpleControlImpl;

class MSimpleControl : public MControl<MSimpleControlImpl>
{
  public:
	typedef MSimpleControlImpl MImpl;

	MSimpleControl(const std::string &inID, MRect inBounds);
};

// --------------------------------------------------------------------

class MButtonImpl;

enum MButtonFlags
{
	eBF_None = 0,
	eBF_Split = (1 << 0),
};

class MButton : public MControl<MButtonImpl>
{
  public:
	typedef MButtonImpl MImpl;

	MButton(const std::string &inID, MRect inBounds, const std::string &inLabel,
		MButtonFlags = eBF_None);

	void SimulateClick();
	void MakeDefault(bool inDefault = true);

	virtual void SetText(const std::string &inText);

	MEventOut<void(const std::string &)>
		eClicked;
	MEventOut<void(const std::string &, int32_t, int32_t)>
		eDropDown;
};

//// --------------------------------------------------------------------
//
// class MImageButton : public MControl<MImageButtonImpl>
//{
//  public:
//	typedef MImageButtonImpl MImpl;
//
//					MImageButtonImpl(const std::string& inID, MRect inBounds,
//						const std::string& inImageResource);
//
//	MEventOut<void(const std::string&)> eClicked;
//};

// --------------------------------------------------------------------

class MExpanderImpl;

class MExpander : public MControl<MExpanderImpl>
{
  public:
	typedef MExpanderImpl MImpl;

	MExpander(const std::string &inID, MRect inBounds, const std::string &inLabel);

	void SetOpen(bool inOpen);
	bool IsOpen() const;

	MEventOut<void(const std::string &)>
		eClicked;
};

// --------------------------------------------------------------------

extern const int kScrollbarWidth;
class MScrollbarImpl;

class MScrollbar : public MControl<MScrollbarImpl>
{
  public:
	typedef MScrollbarImpl MImpl;

	MScrollbar(const std::string &inID, MRect inBounds);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	virtual int32_t GetTrackValue() const;
	virtual int32_t GetMinValue() const;
	virtual int32_t GetMaxValue() const;

	virtual void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
		int32_t inScrollUnit, int32_t inPageSize, int32_t inValue);

	MEventOut<void(MScrollMessage)> eScroll;
};

// --------------------------------------------------------------------

class MStatusbarImpl;

struct MStatusBarElement
{
	uint32_t width;
	MRect margins;
	bool expand;
};

class MStatusbar : public MControl<MStatusbarImpl>
{
  public:
	typedef MStatusbarImpl MImpl;

	MStatusbar(const std::string &inID, MRect inBounds,
		uint32_t inPartCount, MStatusBarElement inParts[]);

	virtual void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder);

	MEventOut<void(uint32_t, MRect)> ePartClicked;
};

// --------------------------------------------------------------------

class MComboboxImpl;

class MCombobox : public MControl<MComboboxImpl>
{
  public:
	typedef MComboboxImpl MImpl;

	MCombobox(const std::string &inID, MRect inBounds);

	MEventOut<void(const std::string &, int)>
		eValueChanged;

	virtual void SetText(const std::string &inText);
	virtual std::string GetText() const;

	virtual void SetChoices(const std::vector<std::string> &inChoices);

	int GetActive();
	void SetActive(int inActive);
};

// --------------------------------------------------------------------

class MPopupImpl;

class MPopup : public MControl<MPopupImpl>
{
  public:
	typedef MPopupImpl MImpl;

	MPopup(const std::string &inID, MRect inBounds);

	MEventOut<void(const std::string &, int32_t)>
		eValueChanged;

	virtual void SetValue(int32_t inValue);
	virtual int32_t GetValue() const;

	virtual void SetText(const std::string &inText);
	virtual std::string GetText() const;

	virtual void SetChoices(const std::vector<std::string> &inChoices);
};

// --------------------------------------------------------------------

class MCaptionImpl;

class MCaption : public MControl<MCaptionImpl>
{
  public:
	typedef MCaptionImpl MImpl;

	MCaption(const std::string &inID, MRect inBounds,
		const std::string &inText);

	virtual void SetText(const std::string &inText);
};

// --------------------------------------------------------------------

class MEdittextImpl;

enum
{
	eMEditTextNoFlags = 0,
	eMEditTextAlignRight = 1 << 0,
	eMEditTextNumbers = 1 << 1,
	eMEditTextMultiLine = 1 << 2,
	eMEditTextReadOnly = 1 << 3
};

class MEdittext : public MControl<MEdittextImpl>
{
  public:
	typedef MEdittextImpl MImpl;

	MEdittext(const std::string &inID, MRect inBounds,
		uint32_t inFlags = eMEditTextNoFlags);

	MEventOut<void(const std::string &, const std::string &)> eValueChanged;
	MEventOut<void(uint32_t inKeyCode, uint32_t inModifiers)> eKeyDown;

	virtual void SetText(const std::string &inText);
	virtual std::string GetText() const;

	uint32_t GetFlags() const;

	virtual void SetPasswordChar(uint32_t inUnicode = 0x2022);
};

// --------------------------------------------------------------------

class MSeparatorImpl;

class MSeparator : public MControl<MSeparatorImpl>
{
  public:
	typedef MSeparatorImpl MImpl;

	MSeparator(const std::string &inID, MRect inBounds);
};

// --------------------------------------------------------------------

class MCheckboxImpl;

class MCheckbox : public MControl<MCheckboxImpl>
{
  public:
	typedef MCheckboxImpl MImpl;

	MCheckbox(const std::string &inID, MRect inBounds,
		const std::string &inTitle);

	bool IsChecked() const;
	void SetChecked(bool inChecked);

	MEventOut<void(const std::string &, bool)>
		eValueChanged;
};

// --------------------------------------------------------------------

class MRadiobuttonImpl;

class MRadiobutton : public MControl<MRadiobuttonImpl>
{
  public:
	typedef MRadiobuttonImpl MImpl;

	MRadiobutton(const std::string &inID, MRect inBounds,
		const std::string &inTitle);

	bool IsChecked() const;
	void SetChecked(bool inChecked);

	void SetGroup(const std::list<MRadiobutton *> &inButtons);

	MEventOut<void(const std::string &, bool)>
		eValueChanged;
};

// --------------------------------------------------------------------

class MColorSwatchImpl;

class MColorSwatch : public MControl<MColorSwatchImpl>
{
  public:
	typedef MColorSwatchImpl MImpl;

	MColorSwatch(const std::string &inID, MRect inBounds,
		MColor inColor);

	virtual MColor GetColor() const;
	virtual void SetColor(MColor inColor);

	void SetPalette(const std::vector<MColor> &colors);

	MEventOut<void(const std::string &, MColor)> eColorChanged;
	MEventOut<void(const std::string &, MColor)> eColorPreview;
};

// --------------------------------------------------------------------

class MListBoxImpl;

class MListBox : public MControl<MListBoxImpl>
{
  public:
	typedef MListBoxImpl MImpl;

	MListBox(const std::string &inID, MRect inBounds);

	void AddItem(const std::string &inLabel);

	virtual int32_t GetValue() const;
	virtual void SetValue(int32_t inValue);

	MEventOut<void(const std::string &, int32_t)> eValueChanged;
};

// // --------------------------------------------------------------------

// class MListViewImpl;

// class MListView : public MControl<MListViewImpl>
// {
//   public:
// 	typedef MListViewImpl MImpl;

// 	MListView(const std::string &inID, MRect inBounds);

// 	void AddItem(const std::string &inLabel);

// 	MEventOut<void(const std::string &, int32_t)> eValueChanged;
// };

// --------------------------------------------------------------------
// Gtk specific controls

class MBoxControlImpl;

class MBoxControl : public MControl<MBoxControlImpl>
{
  public:
	typedef MBoxControlImpl MImpl;

	MBoxControl(const std::string &inID, MRect inBounds, bool inHorizontal,
		bool inHomogeneous = false, bool inExpand = false, bool inFill = false,
		uint32_t inSpacing = 0, uint32_t inPadding = 0);
};

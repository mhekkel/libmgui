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

struct MMargins
{
	uint32_t left, top, right, bottom;
};

struct MControlLayout
{
	bool mHExpand, mVExpand;
	bool mHFill, mVFill;
	MMargins mMargin;
};

class MControlBase : public MView
{
  public:
	MControlBase(const std::string &inID, MRect inBounds)
		: MView(inID, inBounds)
	{
	}

	virtual MControlImplBase *GetControlImplBase() = 0;

	virtual bool IsFocus() const = 0;
	virtual void SetFocus() = 0;

	MControlLayout GetLayout() const
	{
		return mLayout;
	}

	virtual void SetLayout(MControlLayout inLayout) = 0;
	void SetLayout(bool expand, bool fill, uint32_t margin)
	{
		SetLayout({ expand, expand, fill, fill, margin, margin, margin, margin });
	}

  protected:
	MControlLayout mLayout{};
};

template <class I>
class MControl : public MControlBase
{
  public:
	~MControl() override;

	void MoveFrame(int32_t inXDelta, int32_t inYDelta) override;

	void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta) override;

	using MControlBase::SetLayout;
	void SetLayout(MControlLayout inLayout) override;

	virtual void Draw();

	virtual void ClickPressed(int32_t inX, int32_t inY, int32_t inClickCount, uint32_t inModifiers) { }
	virtual void ClickReleased(int32_t inX, int32_t inY, uint32_t inModifiers) { }

	virtual void PointerEnter(int32_t inX, int32_t inY, uint32_t inModifiers) { }
	virtual void PointerMotion(int32_t inX, int32_t inY, uint32_t inModifiers) { }
	virtual void PointerLeave() { }

	virtual bool KeyPressed(uint32_t inKeyCode, char32_t inUnicode, uint32_t inModifiers, bool inAutoRepeat) { return false;}
	virtual void KeyReleased(uint32_t inKeyValue, uint32_t inModifiers) { }
	virtual void Modifiers(uint32_t inModifiers) { }
	virtual void EnterText(const std::string &inText) { }

	virtual void ScrollDecelerate(double inVelX, double inVelY) { }
	virtual bool Scroll(double inX, double inY) { return false;}
	virtual void ScrollBegin() { }
	virtual void ScrollEnd() { }

	bool IsFocus() const override;
	void SetFocus() override;

	I *GetImpl() const { return mImpl; }
	void SetImpl(I *inImpl) { mImpl = inImpl; }

	MControlImplBase *GetControlImplBase() override;

  protected:
	MControl(const std::string &inID, MRect inBounds, I *inImpl);

	void EnableSelf() override;
	void DisableSelf() override;

	void ShowSelf() override;
	void HideSelf() override;

	void AddedToWindow() override;

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

	void SetText(const std::string &inText);

	MEventOut<void(const std::string &)> eClicked;
	MEventOut<void(const std::string &, int32_t, int32_t)> eDropDown;
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

	MEventOut<void(const std::string &)> eClicked;
};

// --------------------------------------------------------------------

extern const int kScrollbarWidth;
class MScrollbarImpl;

class MScrollbar : public MControl<MScrollbarImpl>
{
  public:
	typedef MScrollbarImpl MImpl;

	MScrollbar(const std::string &inID, MRect inBounds);

	int32_t GetValue() const;
	void SetValue(int32_t inValue);

	int32_t GetTrackValue() const;
	int32_t GetMinValue() const;
	int32_t GetMaxValue() const;

	void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
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

	void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder);

	MEventOut<void(uint32_t, MRect)> ePartClicked;
};

// --------------------------------------------------------------------

class MComboboxImpl;

class MCombobox : public MControl<MComboboxImpl>
{
  public:
	typedef MComboboxImpl MImpl;

	MCombobox(const std::string &inID, MRect inBounds);

	MEventOut<void(const std::string &, int)> eValueChanged;

	void SetText(const std::string &inText);
	std::string GetText() const;

	void SetChoices(const std::vector<std::string> &inChoices);

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

	MEventOut<void(const std::string &, int32_t)> eValueChanged;

	void SetValue(int32_t inValue);
	int32_t GetValue() const;

	void SetText(const std::string &inText);
	std::string GetText() const;

	void SetChoices(const std::vector<std::string> &inChoices);
};

// --------------------------------------------------------------------

class MCaptionImpl;

class MCaption : public MControl<MCaptionImpl>
{
  public:
	typedef MCaptionImpl MImpl;

	MCaption(const std::string &inID, MRect inBounds,
		const std::string &inText);

	void SetText(const std::string &inText);
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

	void SetText(const std::string &inText);
	std::string GetText() const;

	uint32_t GetFlags() const;

	void SetPasswordChar(uint32_t inUnicode = 0x2022);
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

	MEventOut<void(const std::string &, bool)> eValueChanged;
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

	void SetGroup(MRadiobutton *inButton);

	MEventOut<void(const std::string &, bool)> eValueChanged;
};

// --------------------------------------------------------------------

class MColorSwatchImpl;

class MColorSwatch : public MControl<MColorSwatchImpl>
{
  public:
	typedef MColorSwatchImpl MImpl;

	MColorSwatch(const std::string &inID, MRect inBounds,
		MColor inColor);

	MColor GetColor() const;
	void SetColor(MColor inColor);

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

	int32_t GetValue() const;
	void SetValue(int32_t inValue);

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

	using MView::AddChild;
	void AddChild(MControlBase *inControl, MControlBase *inBefore);
};

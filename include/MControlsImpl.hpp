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

#include "MControls.hpp"
#include "MControls.inl"

class MWinProcMixin;

struct MControlImplBase
{
	virtual ~MControlImplBase() {}

#if defined(_MSC_VER)
	virtual MWinProcMixin *GetWinProcMixin() = 0;
#endif
};

template <class CONTROL>
class MControlImpl : public MControlImplBase
{
  public:
	MControlImpl(CONTROL *inControl)
		: mControl(inControl)
	{
	}
	virtual ~MControlImpl() {}

	virtual void RequestSize(int32_t inWidth, int32_t inHeight) = 0;

	virtual bool IsFocus() const { return false; }
	virtual void SetFocus() {}

	virtual void AddedToWindow() {}
	virtual void FrameMoved() {}
	virtual void FrameResized() {}
	virtual void LayoutChanged() {}
	virtual void Draw() {}
	virtual void Click(int32_t inX, int32_t inY) {}
	virtual void EnableSelf() {}
	virtual void DisableSelf() {}
	virtual void ShowSelf() {}
	virtual void HideSelf() {}

	virtual std::string GetText() const { return {}; }
	virtual void SetText(const std::string &) {}

  protected:
	CONTROL *mControl;
};

class MSimpleControlImpl : public MControlImpl<MSimpleControl>
{
  public:
	MSimpleControlImpl(MSimpleControl *inControl)
		: MControlImpl(inControl)
	{
	}

	static MSimpleControlImpl *
	Create(MSimpleControl *inControl);
};

class MScrollbarImpl : public MControlImpl<MScrollbar>
{
  public:
	MScrollbarImpl(MScrollbar *inControl)
		: MControlImpl<MScrollbar>(inControl)
	{
	}

	virtual int32_t GetValue() const = 0;
	virtual void SetValue(int32_t inValue) = 0;

	virtual int32_t GetTrackValue() const = 0;

	virtual void SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
		int32_t inScrollUnit, int32_t inPageSize, int32_t inValue) = 0;

	virtual int32_t GetMinValue() const = 0;
	//	virtual void	SetMinValue(int32_t inValue) = 0;
	virtual int32_t GetMaxValue() const = 0;
	//	virtual void	SetMaxValue(int32_t inValue) = 0;
	//
	//	virtual void	SetViewSize(int32_t inViewSize) = 0;

	static MScrollbarImpl *
	Create(MScrollbar *inControl);
};

class MButtonImpl : public MControlImpl<MButton>
{
  public:
	MButtonImpl(MButton *inButton)
		: MControlImpl<MButton>(inButton)
	{
	}

	virtual void SimulateClick() = 0;
	virtual void MakeDefault(bool inDefault) = 0;

	virtual void SetText(const std::string &inText) = 0;

	virtual void GetIdealSize(int32_t &outWidth, int32_t &outHeight) = 0;

	static MButtonImpl *
	Create(MButton *inButton, const std::string &inLabel,
		MButtonFlags inFlags);
};

// class MImageButtonImpl : public MControlImpl<MImageButton>
//{
// public:
//					MImageButtonImpl(MImageButton* inButton)
//						: MControlImpl<MImageButton>(inButton)				{}
//
//	static MImageButtonImpl*
//					Create(MImageButton* inButton, const std::string& inImageResource);
// };

class MExpanderImpl : public MControlImpl<MExpander>
{
  public:
	MExpanderImpl(MExpander *inExpander)
		: MControlImpl<MExpander>(inExpander)
	{
	}

	virtual void SetOpen(bool inOpen) = 0;
	virtual bool IsOpen() const = 0;

	static MExpanderImpl *
	Create(MExpander *inExpander, const std::string &inLabel);
};

class MStatusbarImpl : public MControlImpl<MStatusbar>
{
  public:
	MStatusbarImpl(MStatusbar *inStatusbar)
		: MControlImpl<MStatusbar>(inStatusbar)
	{
	}

	virtual void SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder) = 0;

	static MStatusbarImpl *
	Create(MStatusbar *inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[]);
};

class MComboboxImpl : public MControlImpl<MCombobox>
{
  public:
	MComboboxImpl(MCombobox *inCombobox)
		: MControlImpl<MCombobox>(inCombobox)
	{
	}

	virtual void SetText(const std::string &inText) = 0;
	virtual std::string GetText() const = 0;

	virtual void SetChoices(const std::vector<std::string> &inChoices) = 0;

	virtual int GetActive() = 0;
	virtual void SetActive(int inActive) = 0;

	static MComboboxImpl *
	Create(MCombobox *inCombobox);
};

class MPopupImpl : public MControlImpl<MPopup>
{
  public:
	MPopupImpl(MPopup *inPopup)
		: MControlImpl<MPopup>(inPopup)
	{
	}

	virtual void SetValue(int32_t inValue) = 0;
	virtual int32_t GetValue() const = 0;

	virtual void SetText(const std::string &inText) = 0;
	virtual std::string GetText() const = 0;

	virtual void SetChoices(const std::vector<std::string> &inChoices) = 0;

	static MPopupImpl *
	Create(MPopup *inPopup);
};

class MEdittextImpl : public MControlImpl<MEdittext>
{
  public:
	MEdittextImpl(MEdittext *inEdittext)
		: MControlImpl<MEdittext>(inEdittext)
	{
	}

	virtual void SetText(const std::string &inText) = 0;
	virtual std::string GetText() const = 0;

	virtual uint32_t GetFlags() const = 0;

	virtual void SetPasswordChar(uint32_t inUnicode) = 0;

	static MEdittextImpl *
	Create(MEdittext *inEdittext, uint32_t inFlags);
};

class MCaptionImpl : public MControlImpl<MCaption>
{
  public:
	MCaptionImpl(MCaption *inCaption)
		: MControlImpl<MCaption>(inCaption)
	{
	}

	virtual void SetText(const std::string &inText) = 0;

	static MCaptionImpl *
	Create(MCaption *inCaption, const std::string &inText);
};

class MSeparatorImpl : public MControlImpl<MSeparator>
{
  public:
	MSeparatorImpl(MSeparator *inSeparator)
		: MControlImpl<MSeparator>(inSeparator)
	{
	}

	static MSeparatorImpl *
	Create(MSeparator *inSeparator);
};

class MCheckboxImpl : public MControlImpl<MCheckbox>
{
  public:
	MCheckboxImpl(MCheckbox *inCheckbox)
		: MControlImpl<MCheckbox>(inCheckbox)
	{
	}

	virtual bool IsChecked() const = 0;
	virtual void SetChecked(bool inChecked) = 0;

	static MCheckboxImpl *
	Create(MCheckbox *inCheckbox, const std::string &inTitle);
};

class MRadiobuttonImpl : public MControlImpl<MRadiobutton>
{
  public:
	MRadiobuttonImpl(MRadiobutton *inRadiobutton)
		: MControlImpl<MRadiobutton>(inRadiobutton)
	{
	}

	virtual bool IsChecked() const = 0;
	virtual void SetChecked(bool inChecked) = 0;

	virtual void SetGroup(MRadiobuttonImpl *inButton) = 0;

	static MRadiobuttonImpl *
	Create(MRadiobutton *inRadiobutton, const std::string &inTitle);
};

class MColorSwatchImpl : public MControlImpl<MColorSwatch>
{
  public:
	MColorSwatchImpl(MColorSwatch *inColorSwatch)
		: MControlImpl<MColorSwatch>(inColorSwatch)
	{
	}

	virtual MColor GetColor() const = 0;
	virtual void SetColor(MColor inColor) = 0;

	virtual void SetPalette(const std::vector<MColor> &colors) = 0;

	static MColorSwatchImpl *
	Create(MColorSwatch *inColorSwatch, MColor inColor);
};

class MListBoxImpl : public MControlImpl<MListBox>
{
  public:
	MListBoxImpl(MListBox *inListBox)
		: MControlImpl<MListBox>(inListBox)
	{
	}

	virtual void AddItem(const std::string &inText) = 0;

	virtual int32_t GetValue() const = 0;
	virtual void SetValue(int32_t inValue) = 0;

	static MListBoxImpl *
	Create(MListBox *inListBox);
};

// class MListViewImpl : public MControlImpl<MListView>
// {
// public:
// 					MListViewImpl(MListView* inListView)
// 						: MControlImpl<MListView>(inListView)				{}

// 	virtual void	AddItem(const std::string& inText) = 0;

// 	static MListViewImpl*
// 					Create(MListView* inListView);
// };

// --------------------------------------------------------------------
// Some container controls

class MBoxControlImpl : public MControlImpl<MBoxControl>
{
  public:
	MBoxControlImpl(MBoxControl *inControl)
		: MControlImpl(inControl)
	{
	}

	virtual void AddChild(MControlBase *inChild, MControlBase *inBefore) = 0;

	static MBoxControlImpl *Create(MBoxControl *inControl, bool inHorizontal);
};

class MStackControlImpl : public MControlImpl<MStackControl>
{
  public:
	MStackControlImpl(MStackControl *inControl)
		: MControlImpl(inControl)
	{
	}

	virtual void AddChild(MView *inChild, const std::string &inName) = 0;
	virtual void Select(const std::string &inName) = 0;

	static MStackControlImpl *Create(MStackControl *inControl);
};

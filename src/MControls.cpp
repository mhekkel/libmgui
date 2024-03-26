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

#include "MControlsImpl.hpp"
#include "MWindow.hpp"

#include <cassert>

// --------------------------------------------------------------------

MSimpleControl::MSimpleControl(const std::string& inID, MRect inBounds)
	: MControl<MSimpleControlImpl>(inID, inBounds, MSimpleControlImpl::Create(this))
{
}

// --------------------------------------------------------------------

MButton::MButton(const std::string& inID, MRect inBounds, const std::string& inLabel,
	MButtonFlags inFlags)
	: MControl<MButtonImpl>(inID, inBounds, MButtonImpl::Create(this, inLabel, inFlags))
{
}

void MButton::SimulateClick()
{
	mImpl->SimulateClick();
}

void MButton::MakeDefault(bool inDefault)
{
	mImpl->MakeDefault(inDefault);
}

void MButton::SetText(const std::string& inText)
{
	mImpl->SetText(inText);
}

// --------------------------------------------------------------------

MExpander::MExpander(const std::string& inID, MRect inBounds, const std::string& inLabel)
	: MControl<MExpanderImpl>(inID, inBounds, MExpanderImpl::Create(this, inLabel))
{
}

void MExpander::SetOpen(bool inOpen)
{
	mImpl->SetOpen(inOpen);
}

bool MExpander::IsOpen() const
{
	return mImpl->IsOpen();
}

// --------------------------------------------------------------------

MScrollbar::MScrollbar(const std::string& inID, MRect inBounds)
	: MControl<MScrollbarImpl>(inID, inBounds, MScrollbarImpl::Create(this))
{
}

int32_t MScrollbar::GetValue() const
{
	return mImpl->GetValue();
}

void MScrollbar::SetValue(int32_t inValue)
{
	mImpl->SetValue(inValue);
}

int32_t MScrollbar::GetTrackValue() const
{
	return mImpl->GetTrackValue();
}

void MScrollbar::SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
	int32_t inScrollUnit, int32_t inPageSize, int32_t inValue)
{
	mImpl->SetAdjustmentValues(inMinValue, inMaxValue,
		inScrollUnit, inPageSize, inValue);
}

//void MScrollbar::SetMinValue(int32_t inMinValue)
//{
//	mImpl->SetMinValue(inMinValue);
//}

int32_t MScrollbar::GetMinValue() const
{
	return mImpl->GetMinValue();
}
	
//void MScrollbar::SetMaxValue(int32_t inMaxValue)
//{
//	mImpl->SetMaxValue(inMaxValue);
//}

int32_t MScrollbar::GetMaxValue() const
{
	return mImpl->GetMaxValue();
}

//void MScrollbar::SetViewSize(int32_t inViewSize)
//{
//	mImpl->SetViewSize(inViewSize);
//}

// --------------------------------------------------------------------

MStatusbar::MStatusbar(const std::string& inID, MRect inBounds, uint32_t inPartCount, MStatusBarElement inParts[])
	: MControl<MStatusbarImpl>(inID, inBounds, MStatusbarImpl::Create(this, inPartCount, inParts))
{
	SetBindings(true, false, true, true);
}

void MStatusbar::SetStatusText(uint32_t inPartNr, const std::string& inText, bool inBorder)
{
	mImpl->SetStatusText(inPartNr, inText, inBorder);
}

// --------------------------------------------------------------------

MCombobox::MCombobox(const std::string& inID, MRect inBounds)
	: MControl<MComboboxImpl>(inID, inBounds, MComboboxImpl::Create(this))
{
}

void MCombobox::SetText(const std::string& inText)
{
	mImpl->SetText(inText);
}

std::string MCombobox::GetText() const
{
	return mImpl->GetText();
}

void MCombobox::SetChoices(const std::vector<std::string>& inChoices)
{
	mImpl->SetChoices(inChoices);
}

int MCombobox::GetActive()
{
	return mImpl->GetActive();
}

void MCombobox::SetActive(int inActive)
{
	mImpl->SetActive(inActive);
}

// --------------------------------------------------------------------

MPopup::MPopup(const std::string& inID, MRect inBounds)
	: MControl<MPopupImpl>(inID, inBounds, MPopupImpl::Create(this))
{
}

void MPopup::SetValue(int32_t inValue)
{
	mImpl->SetValue(inValue);
}

int32_t MPopup::GetValue() const
{
	return mImpl->GetValue();
}

void MPopup::SetText(const std::string& inText)
{
	mImpl->SetText(inText);
}

std::string MPopup::GetText() const
{
	return mImpl->GetText();
}

void MPopup::SetChoices(const std::vector<std::string>& inChoices)
{
	mImpl->SetChoices(inChoices);
}

// --------------------------------------------------------------------

MEdittext::MEdittext(const std::string& inID, MRect inBounds, uint32_t inFlags)
	: MControl<MEdittextImpl>(inID, inBounds, MEdittextImpl::Create(this, inFlags))
{
}

void MEdittext::SetText(const std::string& inText)
{
	mImpl->SetText(inText);
}

std::string MEdittext::GetText() const
{
	return mImpl->GetText();
}

uint32_t MEdittext::GetFlags() const
{
	return mImpl->GetFlags();
}

void MEdittext::SetPasswordChar(uint32_t inUnicode)
{
	mImpl->SetPasswordChar(inUnicode);
}

// --------------------------------------------------------------------

MCaption::MCaption(const std::string& inID, MRect inBounds, const std::string& inText)
	: MControl<MCaptionImpl>(inID, inBounds, MCaptionImpl::Create(this, inText))
{
}

void MCaption::SetText(const std::string& inText)
{
	mImpl->SetText(inText);
}

// --------------------------------------------------------------------

MSeparator::MSeparator(const std::string& inID, MRect inBounds)
	: MControl<MSeparatorImpl>(inID, inBounds, MSeparatorImpl::Create(this))
{
}

// --------------------------------------------------------------------

MCheckbox::MCheckbox(const std::string& inID, MRect inBounds, const std::string& inTitle)
	: MControl<MCheckboxImpl>(inID, inBounds, MCheckboxImpl::Create(this, inTitle))
{
}

bool MCheckbox::IsChecked() const
{
	return mImpl->IsChecked();
}

void MCheckbox::SetChecked(bool inChecked)
{
	mImpl->SetChecked(inChecked);
}

// --------------------------------------------------------------------

MRadiobutton::MRadiobutton(const std::string& inID, MRect inBounds, const std::string& inTitle)
	: MControl<MRadiobuttonImpl>(inID, inBounds, MRadiobuttonImpl::Create(this, inTitle))
{
}

bool MRadiobutton::IsChecked() const
{
	return mImpl->IsChecked();
}

void MRadiobutton::SetChecked(bool inChecked)
{
	mImpl->SetChecked(inChecked);
}

void MRadiobutton::SetGroup(MRadiobutton *inButton)
{
	mImpl->SetGroup(inButton->GetImpl());
}

// --------------------------------------------------------------------

MColorSwatch::MColorSwatch(const std::string& inID, MRect inBounds, MColor inColor)
	: MControl<MColorSwatchImpl>(inID, inBounds, MColorSwatchImpl::Create(this, inColor))
{
}

MColor MColorSwatch::GetColor() const
{
	return mImpl->GetColor();
}

void MColorSwatch::SetColor(MColor inColor)
{
	mImpl->SetColor(inColor);
}

void MColorSwatch::SetPalette(const std::vector<MColor> &colors)
{
	mImpl->SetPalette(colors);
}

// --------------------------------------------------------------------

MListBox::MListBox(const std::string& inID, MRect inBounds)
	: MControl<MListBoxImpl>(inID, inBounds, MListBoxImpl::Create(this))
{
}

void MListBox::AddItem(const std::string& inLabel)
{
	mImpl->AddItem(inLabel);
}

int32_t MListBox::GetValue() const
{
	return mImpl->GetValue();
}

void MListBox::SetValue(int32_t inValue)
{
	mImpl->SetValue(inValue);
}

// // --------------------------------------------------------------------

// MListView::MListView(const string& inID, MRect inBounds)
// 	: MControl<MListViewImpl>(inID, inBounds, MListViewImpl::Create(this))
// {
// }

// void MListView::AddItem(const string& inLabel)
// {
// 	mImpl->AddItem(inLabel);
// }

// --------------------------------------------------------------------
// Gtk specific controls

MBoxControl::MBoxControl(const std::string& inID, MRect inBounds, bool inHorizontal,
		bool inHomogeneous, bool inExpand, bool inFill, uint32_t inSpacing, uint32_t inPadding)
	: MControl(inID, inBounds, MBoxControlImpl::Create(this,
		inHorizontal, inHomogeneous, inExpand, inFill, inSpacing, inPadding))
{
}

void MBoxControl::AddChild(MControlBase *inControl, MControlBase *inBefore)
{
	mImpl->AddChild(inControl, inBefore);
}
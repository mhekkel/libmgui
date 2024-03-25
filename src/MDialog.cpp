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

/*	$Id: MDialog.cpp 133 2007-05-01 08:34:48Z maarten $
    Copyright Maarten L. Hekkelman
    Created Sunday August 15 2004 13:51:21
*/

#include "MDialog.hpp"
#include "MControls.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"

#include <cassert>

#include <sstream>

MDialog *MDialog::sFirst;

MDialog::MDialog(const std::string &inDialogResource)
	: MWindow(MWindowImpl::CreateDialog(inDialogResource, this))
	, eButtonClicked(this, &MDialog::ButtonClicked)
	, eCheckboxClicked(this, &MDialog::CheckboxChanged)
	, eRadiobuttonClicked(this, &MDialog::RadiobuttonChanged)
	, eTextChanged(this, &MDialog::TextChanged)
	, eValueChanged(this, &MDialog::ValueChanged)
	, eColorChanged(this, &MDialog::ColorChanged)
	, mParentWindow(nullptr)
	, mNext(nullptr)
{
	GetImpl()->Finish();

	mNext = sFirst;
	sFirst = this;
}

MDialog::~MDialog()
{
	if (sFirst == this)
		sFirst = mNext;
	else
	{
		MDialog *dlog = sFirst;
		while (dlog->mNext != nullptr)
		{
			if (dlog->mNext == this)
			{
				dlog->mNext = mNext;
				break;
			}
			dlog = dlog->mNext;
		}
	}
}

void MDialog::Show(MWindow *inParent)
{
	MRect r, b;
	GetWindowPosition(r);

	// if parent exists, we position our dialog on top of it
	if (inParent != nullptr)
	{
		inParent->Select();
		inParent->GetWindowPosition(b);
	}
	else
		MWindow::GetMainScreenBounds(b);

	r.x = b.x + (b.width - r.width) / 2;
	r.y = b.y + (b.height - r.height) / 3;

	SetWindowPosition(r);

	if (inParent)
		GetImpl()->SetTransientFor(inParent);

	MWindow::Show();
	MWindow::Select();
}

bool MDialog::ShowModal(MWindow *inParent)
{
	Show(inParent);
	return GetImpl()->ShowModal();
}

void MDialog::RecalculateLayout()
{
	//	assert(mChildren.size() == 1);
	assert(mChildren.size() >= 1);

	for (auto child : mChildren)
		child->RecalculateLayout();

	mBounds = mChildren.front()->GetFrame();
	mFrame = mBounds;
	mBounds.x += mLeftMargin;
	mBounds.y += mTopMargin;
	mFrame.width += mLeftMargin + mRightMargin;
	mFrame.height += mTopMargin + mBottomMargin;
}

void MDialog::ChildResized()
{
	MRect frame = mFrame;

	assert(mChildren.size() == 1);
	mBounds = mChildren.front()->GetFrame();
	mFrame = mBounds;
	mBounds.x += mLeftMargin;
	mBounds.y += mTopMargin;
	mFrame.width += mLeftMargin + mRightMargin;
	mFrame.height += mTopMargin + mBottomMargin;

	if (frame != mFrame)
		ResizeWindow(mFrame.width - frame.width, mFrame.height - frame.height);
}

bool MDialog::OKClicked()
{
	return true;
}

bool MDialog::CancelClicked()
{
	return true;
}

void MDialog::ButtonClicked(const std::string &inID)
{
	if (inID == "ok")
	{
		if (OKClicked())
			Close();
	}
	else if (inID == "cancel")
	{
		if (CancelClicked())
			Close();
	}
}

void MDialog::CheckboxChanged(const std::string &inID, bool inChecked)
{
}

void MDialog::RadiobuttonChanged(const std::string &inID, bool inChecked)
{
}

void MDialog::TextChanged(const std::string &inID, const std::string &inText)
{
}

void MDialog::ValueChanged(const std::string &inID, int32_t inValue)
{
}

void MDialog::ColorChanged(const std::string &inID, MColor inColor)
{
}

void MDialog::SetFocus(const std::string &inID)
{
	// MControl* control = dynamic_cast<MControl*>(FindViewByID(inID));
	// if (control != nullptr)
	//	control->Focus();

	MView *view = FindSubViewByID(inID);
	if (view != nullptr)
	{
		if (dynamic_cast<MEdittext *>(view) != nullptr)
			static_cast<MEdittext *>(view)->SetFocus();
		else if (dynamic_cast<MCombobox *>(view) != nullptr)
			static_cast<MCombobox *>(view)->SetFocus();
	}
}

void MDialog::SavePosition(const char *inName)
{
	MRect r;
	GetWindowPosition(r);

	std::stringstream s;
	s << r.x << ' ' << r.y << ' ' << r.width << ' ' << r.height;

	Preferences::SetString(inName, s.str());
}

void MDialog::RestorePosition(const char *inName)
{
	std::string s = Preferences::GetString(inName, "");
	if (s.length() > 0)
	{
		MRect r;

		std::stringstream ss(s);
		ss >> r.x >> r.y >> r.width >> r.height;

		if (GetFlags() & kMFixedSize)
		{
			MRect bounds;
			GetWindowPosition(bounds);
			r.width = bounds.width;
			r.height = bounds.height;
		}

		//		SetWindowPosition(r, false);
	}
}

std::string MDialog::GetText(const std::string &inID) const
{
	std::string result;

	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (dynamic_cast<MCombobox *>(view) != nullptr)
		result = static_cast<MCombobox *>(view)->GetText();
	else if (dynamic_cast<MEdittext *>(view) != nullptr)
		result = static_cast<MEdittext *>(view)->GetText();
	else if (dynamic_cast<MPopup *>(view) != nullptr)
		result = static_cast<MPopup *>(view)->GetText();

	return result;
}

void MDialog::SetText(const std::string &inID, const std::string &inText)
{
	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (dynamic_cast<MCombobox *>(view) != nullptr)
		static_cast<MCombobox *>(view)->SetText(inText);
	else if (dynamic_cast<MPopup *>(view) != nullptr)
		static_cast<MPopup *>(view)->SetText(inText);
	else if (dynamic_cast<MEdittext *>(view) != nullptr)
		static_cast<MEdittext *>(view)->SetText(inText);
	else if (dynamic_cast<MCaption *>(view) != nullptr)
		static_cast<MCaption *>(view)->SetText(inText);
	else if (dynamic_cast<MButton *>(view) != nullptr)
		static_cast<MButton *>(view)->SetText(inText);
}

void MDialog::SetPasswordChar(const std::string &inID, const uint32_t inUnicode)
{
	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(dynamic_cast<MEdittext *>(view));
	static_cast<MEdittext *>(view)->SetPasswordChar(inUnicode);
}

int32_t MDialog::GetValue(const std::string &inID) const
{
	int32_t result = -1;

	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (dynamic_cast<MPopup *>(view) != nullptr)
		result = static_cast<MPopup *>(view)->GetValue();

	return result;
}

void MDialog::SetValue(const std::string &inID, int32_t inValue)
{
	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (dynamic_cast<MPopup *>(view) != nullptr)
		static_cast<MPopup *>(view)->SetValue(inValue);
}

bool MDialog::IsChecked(const std::string &inID) const
{
	MView *view = FindSubViewByID(inID);
	if (dynamic_cast<MCheckbox *>(view) != nullptr)
		return static_cast<MCheckbox *>(view)->IsChecked();
	else if (dynamic_cast<MRadiobutton *>(view) != nullptr)
		return static_cast<MRadiobutton *>(view)->IsChecked();
	else
		THROW_IF_NIL(nullptr);
}

void MDialog::SetChecked(const std::string &inID, bool inChecked)
{
	MView *view = FindSubViewByID(inID);
	if (dynamic_cast<MCheckbox *>(view) != nullptr)
		static_cast<MCheckbox *>(view)->SetChecked(inChecked);
	else if (dynamic_cast<MRadiobutton *>(view) != nullptr)
		static_cast<MRadiobutton *>(view)->SetChecked(inChecked);
}

void MDialog::SetChoices(const std::string &inID, std::vector<std::string> &inChoices)
{
	MView *view = FindSubViewByID(inID);
	if (dynamic_cast<MPopup *>(view) != nullptr)
		static_cast<MPopup *>(view)->SetChoices(inChoices);
	else if (dynamic_cast<MCombobox *>(view) != nullptr)
		static_cast<MCombobox *>(view)->SetChoices(inChoices);
}

bool MDialog::IsOpen(const std::string &inID) const
{
	MExpander *expander = dynamic_cast<MExpander *>(FindSubViewByID(inID));
	THROW_IF_NIL(expander);
	return expander->IsOpen();
}

void MDialog::SetOpen(const std::string &inID, bool inOpen)
{
	MExpander *expander = dynamic_cast<MExpander *>(FindSubViewByID(inID));
	THROW_IF_NIL(expander);
	expander->SetOpen(inOpen);
}

MColor MDialog::GetColor(const std::string &inID) const
{
	MColorSwatch *swatch = dynamic_cast<MColorSwatch *>(FindSubViewByID(inID));
	THROW_IF_NIL(swatch);
	return swatch->GetColor();
}

void MDialog::SetColor(const std::string &inID, MColor inColor)
{
	MColorSwatch *swatch = dynamic_cast<MColorSwatch *>(FindSubViewByID(inID));
	THROW_IF_NIL(swatch);
	swatch->SetColor(inColor);
}

void MDialog::SetEnabled(const std::string &inID, bool inEnabled)
{
	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (inEnabled)
		view->Enable();
	else
		view->Disable();
}

void MDialog::SetVisible(const std::string &inID, bool inVisible)
{
	MView *view = FindSubViewByID(inID);
	THROW_IF_NIL(view);
	if (inVisible)
		view->Show();
	else
		view->Hide();
}

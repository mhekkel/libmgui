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
#include "MCanvas.hpp"
#include "MColorPicker.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MError.hpp"
#include "MPreferences.hpp"
#include "MStrings.hpp"

#include "mrsrc.hpp"

#include <mxml/document.hpp>

#include <cassert>
#include <charconv>
#include <sstream>

// --------------------------------------------------------------------

MDialog::MDialog(const std::string &inDialogResource)
	: MWindow(MWindowImpl::CreateDialogImpl(this))
	, eButtonClicked(this, &MDialog::ButtonClicked)
	, eCheckboxClicked(this, &MDialog::CheckboxChanged)
	, eRadiobuttonClicked(this, &MDialog::RadiobuttonChanged)
	, eTextChanged(this, &MDialog::TextChanged)
	, eValueChanged(this, &MDialog::ValueChanged)
	, eColorChanged(this, &MDialog::ColorChanged)
	, mRsrc(inDialogResource)
	, mParentWindow(nullptr)
{
	Build();

	GetImpl()->Finish();
}

MDialog::~MDialog()
{
}

// --------------------------------------------------------------------

std::pair<int, bool> MDialog::GetAttributeSize(const mxml::element &e, const char *name, float inDLU)
{
	bool ok = false;
	int value = 0;

	if (auto attr = e.get_attribute(name); not attr.empty())
	{
		auto r = std::from_chars(attr.data(), attr.data() + attr.length(), value);
		auto suffix = std::string_view(r.ptr, attr.data() + attr.length());
		ok = r.ec == std::errc();
		if (suffix.empty())
			value *= inDLU;
		else if (suffix != "px")
			ok = false;
	}

	return { value, ok };
}

void MDialog::Build()
{
	std::string resource = std::string("Dialogs/") + mRsrc + ".xml";
	mrsrc::istream rsrc(resource);

	if (not rsrc)
		throw std::runtime_error("Dialog resource not found: " + resource);

	mxml::document doc(rsrc);

	auto dialog = doc.find_first("/dialog");
	if (dialog == doc.end())
		throw std::runtime_error("Invalid dialog resource");

	std::string title = l(dialog->get_attribute("title"));

	// setup the DLU values

	MDevice dev;
	dev.SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	mDLUX = dev.GetTextWidth() / (52 * 4.0f);
	mDLUY = dev.GetLineHeight() / 8.0f;

	// mFlags = kMFixedSize;
	// std::string flags = dialog->get_attribute("flags");
	// if (flags.find("flexible") != std::string::npos)
	// 	mFlags = MWindowFlags(mFlags & ~kMFixedSize);
	// if (flags.find("nosizebox") != std::string::npos)
	// 	mFlags = MWindowFlags(mFlags | kMNoSizeBox);

	uint32_t minWidth = 40;
	if (auto [w, ok] = GetAttributeSize(*dialog, "width", mDLUX); ok)
		minWidth = w;
	uint32_t minHeight = 40;
	if (auto [h, ok] = GetAttributeSize(*dialog, "height", mDLUY); ok)
		minHeight = h;

	MRect bounds(0, 0, minWidth, minHeight);

	// now create the dialog
	GetImpl()->CreateWindow(bounds, title);

	// create the dialog controls, all stacked on top of each other
	auto vbox = dialog->find_first("vbox");
	if (vbox == dialog->end())
		throw std::runtime_error("Invalid dialog resource");

	MView *content = CreateControls(*vbox, 0, 0);

	for (MRadiobutton *radiobutton : mRadioGroup)
		radiobutton->SetGroup(mRadioGroup.front());

	// Main vbox first
	MBoxControl *mainVBox = new MBoxControl("main-vbox", bounds, false);
	MViewLayout layout{ true, true, 0, 0, 0, 0 };
	layout.mMargin = GetMargins(*vbox);
	mainVBox->SetLayout(layout);

	AddChild(mainVBox);

	mainVBox->AddChild(content);

	// the buttons

	auto buttons = dialog->find_first("/dialog/hbox");
	if (buttons == dialog->end())
		throw std::runtime_error("Invalid dialog resource");

	MBoxControl *buttonBar = new MBoxControl("buttonbar", bounds, true);
	buttonBar->SetLayout({ true, false,
		0,
		static_cast<uint32_t>(2 * mDLUY),
		0,
		0 });
	mainVBox->AddChild(buttonBar);

	auto filler = new MSimpleControl("button-bar-filler", bounds);
	filler->SetLayout({ true, 0 });
	buttonBar->AddChild(filler);

	for (auto button : buttons->find("button"))
	{
		MButton *btn = static_cast<MButton *>(CreateButton(*button, 0, 0));

		auto layout = btn->GetLayout();
		layout.mMargin = GetMargins(*button);
		btn->SetLayout(layout);

		buttonBar->AddChild(btn);
	}
}

void MDialog::SetDefaultButton(MButton *inButton)
{
	GetImpl()->SetDefaultButton(inButton);
}

std::string MDialog::l(const std::string &s)
{
	return GetLocalisedStringForContext(mRsrc, s);
}

MMargins MDialog::GetMargins(const mxml::element &inTemplate)
{
	MMargins result = inTemplate.name() == "hbox" or inTemplate.name() == "vbox" ?
		MMargins{ 0, 0, 0, 0 } :
		MMargins{ static_cast<uint32_t>(2 * mDLUX), static_cast<uint32_t>(1 * mDLUY), static_cast<uint32_t>(2 * mDLUX), static_cast<uint32_t>(1 * mDLUY) };

	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin", 1); ok)
	{
		result.left = result.right = m * mDLUX;
		result.top = result.bottom = m * mDLUY;
	}
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-left-right", mDLUX); ok)
		result.left = result.right = m;
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-top-bottom", mDLUY); ok)
		result.top = result.bottom = m;
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-left", mDLUX); ok)
		result.left = m;
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-top", mDLUY); ok)
		result.top = m;
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-right", mDLUX); ok)
		result.right = m;
	if (auto [m, ok] = GetAttributeSize(inTemplate, "margin-bottom", mDLUY); ok)
		result.bottom = m;

	return result;
}

MView *MDialog::CreateButton(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	std::string title = l(inTemplate.get_attribute("title"));

	MRect bounds;

	MButtonFlags flags = eBF_None;

	if (inTemplate.get_attribute("split") == "true")
		flags = eBF_Split;

	MButton *button = new MButton(id, bounds, title, flags);

	if (inTemplate.get_attribute("default") == "true")
	{
		button->MakeDefault(true);
		mOKButton = button;
	}
	else if (id == "ok" and mOKButton == nullptr)
		mOKButton = button;

	if (id == "cancel")
		mCancelButton = button;

	AddRoute(button->eClicked, eButtonClicked);

	return button;
}

MView *MDialog::CreateColorSwatch(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	MRect bounds(inX, inY, static_cast<int32_t>(16 * mDLUX), static_cast<int32_t>(6 * mDLUY));

	MColor color(inTemplate.get_attribute("color").c_str());
	MColorSwatch *swatch = new MColorSwatch(id, bounds, color);

	AddRoute(swatch->eColorChanged, eColorChanged);

	return swatch;
}

MView *MDialog::CreateExpander(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	std::string title = l(inTemplate.get_attribute("title"));

	MRect bounds; //(inX, inY,
	              //		static_cast<int32_t>((13 + 3) * mDLUX) +
	              //			GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_LABEL, 0),
	              //		static_cast<int32_t>(12 * mDLUY));

	MExpander *expander = new MExpander(id, bounds, title);
	AddRoute(expander->eClicked, eButtonClicked);

	for (auto b : inTemplate)
	{
		if (b.get_attribute("if") == "WINDOWS")
			continue;

		expander->AddChild(CreateControls(b, 0, 0));
	}

	return expander;
}

MView *MDialog::CreateCanvas(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	return new MCanvas(id, { inX, inY, 2, 2 });
}

MView *MDialog::CreateCaption(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	if (id.empty())
		id = "caption";
	std::string text = l(inTemplate.get_attribute("text"));

	MRect bounds; //(inX, static_cast<int32_t>(inY), 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = GetTextWidth(text, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	return new MCaption(id, bounds, text);
}

MView *MDialog::CreateCheckbox(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	std::string title = l(inTemplate.get_attribute("title"));

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = static_cast<int32_t>(14 * mDLUX) +
	              //		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	              //		//GetTextWidth(title, VSCLASS_BUTTON, BP_CHECKBOX, PBS_NORMAL);

	MCheckbox *checkbox = new MCheckbox(id, bounds, title);
	AddRoute(checkbox->eValueChanged, eCheckboxClicked);
	return checkbox;
}

MView *MDialog::CreateRadiobutton(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	std::string title = l(inTemplate.get_attribute("title"));

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = static_cast<int32_t>(14 * mDLUX) +
	              //		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	              //		GetTextWidth(title, VSCLASS_BUTTON, BP_RADIOBUTTON, PBS_NORMAL);
	MRadiobutton *radiobutton = new MRadiobutton(id, bounds, title);
	AddRoute(radiobutton->eValueChanged, eRadiobuttonClicked);

	mRadioGroup.push_back(radiobutton);

	return radiobutton;
}

MView *MDialog::CreateCombobox(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	MRect bounds; //(inX, inY, static_cast<int32_t>(50 * mDLUX), static_cast<int32_t>(14 * mDLUY));
	MCombobox *combobox = new MCombobox(id, bounds);
	AddRoute(combobox->eValueChanged, eValueChanged);
	return combobox;
}

MView *MDialog::CreateEdittext(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	uint32_t flags = eMEditTextNoFlags;
	if (inTemplate.get_attribute("style").find("right") != std::string::npos)
		flags |= eMEditTextAlignRight;
	if (inTemplate.get_attribute("style").find("number") != std::string::npos)
		flags |= eMEditTextNumbers;
	if (inTemplate.get_attribute("style").find("multiline") != std::string::npos)
		flags |= eMEditTextMultiLine;
	if (inTemplate.get_attribute("style").find("readonly") != std::string::npos)
		flags |= eMEditTextReadOnly;

	MRect bounds; //(inX, inY, static_cast<int32_t>(5 * mDLUX), static_cast<int32_t>(14 * mDLUY));
	MEdittext *edittext = new MEdittext(id, bounds, flags);

	if (inTemplate.get_attribute("password") == "true")
		edittext->SetPasswordChar();

	AddRoute(edittext->eValueChanged, eTextChanged);
	return edittext;
}

MView *MDialog::CreateFiller(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	return new MSimpleControl(id, { inX, inY, 0, 0 });
}

MView *MDialog::CreatePopup(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(14 * mDLUY));

	std::vector<std::string> choices;
	for (auto option : inTemplate.find("./option"))
	{
		std::string label = option->get_content();
		//		int32_t width = GetTextWidth(label, VSCLASS_COMBOBOX, CP_DROPDOWNBUTTON, CBXSL_NORMAL);
		//		if (bounds.width < width)
		//			bounds.width = width;
		choices.push_back(label);
	}

	//	bounds.width += static_cast<int32_t>(14 * mDLUX);

	MPopup *popup = new MPopup(id, bounds);

	popup->SetChoices(choices);
	AddRoute(popup->eValueChanged, eValueChanged);

	return popup;
}

MView *MDialog::CreatePager(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MStackControl *result = new MStackControl(id, r);

	MRect b;
	std::string firstName;

	for (bool first = true; mxml::element * page : inTemplate.find("./page"))
	{
		if (page->get_attribute("if") == "WINDOWS")
			continue;

		MView *control = CreateControls(*page, 0, 0);
		control->SetLayout({ true, 0 });
		result->AddChild(control, control->GetID());

		if (std::exchange(first, false))
			firstName = control->GetID();

		control->RecalculateLayout();

		b |= control->GetFrame();
	}

	r.width = b.width;
	r.height = b.height;

	result->SetFrame(r);
	result->Select(firstName);

	return result;
}

MView *MDialog::CreateListBox(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MListBox *result = new MListBox(id, r);

	for (auto listitem : inTemplate.find("./listitem"))
	{
		std::string text = l(listitem->get_content());
		//		int32_t textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
		//		if (r.width < textWidth)
		//			r.width = textWidth;
		result->AddItem(text);
	}

	//	r.width += static_cast<int32_t>(mDLUX * 6);
	//	result->SetFrame(r);

	AddRoute(result->eValueChanged, eValueChanged);

	return result;
}

// MView *MDialog::CreateListView(const mxml::element &inTemplate, int32_t inX, int32_t inY)
// {
// 	std::string id = inTemplate.get_attribute("id");

// 	MRect r(inX, inY, 0, 0);
// 	MListView *result = new MListView(id, r);

// 	for (const mxml::element &listitem : inTemplate.find("./listitem"))
// 	{
// 		std::string text = l(listitem->get_content());
// 		//		int32_t textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
// 		//		if (r.width < textWidth)
// 		//			r.width = textWidth;
// 		result->AddItem(text);
// 	}

// 	//	r.width += static_cast<int32_t>(mDLUX * 6);
// 	//	result->SetFrame(r);

// 	AddRoute(result->eValueChanged, eValueChanged);

// 	return result;
// }

MView *MDialog::CreateSeparator(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	MRect bounds(inX, inY, 2, 2);
	return new MSeparator("separator", bounds);
}

MView *MDialog::CreateScrollbar(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	std::string id = inTemplate.get_attribute("id");
	std::string orientation = inTemplate.get_attribute("orientation");

	MRect bounds(inX, inY, kScrollbarWidth, kScrollbarWidth);

	if (orientation == "horizontal")
		bounds.width *= 2;
	else
		bounds.height *= 2;

	return new MScrollbar(id, bounds);
}

MView *MDialog::CreateBox(const mxml::element &inTemplate, int32_t inX, int32_t inY, bool inHorizontal)
{
	std::string id = inTemplate.get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MView *result = new MBoxControl(id, r, inHorizontal);

	for (auto &b : inTemplate)
	{
		if (b.get_attribute("if") == "WINDOWS")
			continue;

		auto view = CreateControls(b, 0, 0);
		if (view != nullptr)
			result->AddChild(view);
	}

	return result;
}

// MView *MDialog::CreateTable(const mxml::element &inTemplate, int32_t inX, int32_t inY)
// {
// 	std::string id = inTemplate.get_attribute("id");

// 	std::vector<MView *> views;
// 	uint32_t colCount = 0, rowCount = 0;

// 	for (const mxml::element &row : inTemplate.find("./row"))
// 	{
// 		uint32_t cn = 0;

// 		for (auto &col : *row)
// 		{
// 			if (col.get_attribute("if") == "WINDOWS")
// 				continue;

// 			++cn;
// 			if (colCount < cn)
// 				colCount = cn;
// 			views.push_back(CreateControls(&col, 0, 0));
// 		}

// 		++rowCount;
// 	}

// 	// fix me!
// 	while (views.size() < (rowCount * colCount))
// 		views.push_back(nullptr);

// 	MRect r(inX, inY, 0, 0);
// 	MTable *result = new MTable(id, r,
// 		//		&views[0], colCount, rowCount, static_cast<int32_t>(4 * mDLUX), static_cast<int32_t>(4 * mDLUY));
// 		&views[0], colCount, rowCount, 4, 4);

// 	return result;
// }

MView *MDialog::CreateControls(const mxml::element &inTemplate, int32_t inX, int32_t inY)
{
	MView *result = nullptr;

	std::string name = inTemplate.name();

	if (name == "button")
		result = CreateButton(inTemplate, inX, inY);
	else if (name == "canvas")
		result = CreateCanvas(inTemplate, inX, inY);
	else if (name == "caption")
		result = CreateCaption(inTemplate, inX, inY);
	else if (name == "checkbox")
		result = CreateCheckbox(inTemplate, inX, inY);
	else if (name == "swatch")
		result = CreateColorSwatch(inTemplate, inX, inY);
	else if (name == "radiobutton")
		result = CreateRadiobutton(inTemplate, inX, inY);
	else if (name == "expander")
		result = CreateExpander(inTemplate, inX, inY);
	else if (name == "combobox")
		result = CreateCombobox(inTemplate, inX, inY);
	else if (name == "edittext")
		result = CreateEdittext(inTemplate, inX, inY);
	else if (name == "popup")
		result = CreatePopup(inTemplate, inX, inY);
	else if (name == "scrollbar")
		result = CreateScrollbar(inTemplate, inX, inY);
	else if (name == "separator")
		result = CreateSeparator(inTemplate, inX, inY);
	// else if (name == "table")
	// 	result = CreateTable(inTemplate, inX, inY);
	else if (name == "vbox" or name == "dialog" or name == "page")
		result = CreateBox(inTemplate, inX, inY, false);
	else if (name == "hbox")
		result = CreateBox(inTemplate, inX, inY, true);
	else if (name == "pager")
		result = CreatePager(inTemplate, inX, inY);
	else if (name == "listbox")
		result = CreateListBox(inTemplate, inX, inY);
	// else if (name == "listview")
	// 	result = CreateListView(inTemplate, inX, inY);
	else if (name == "filler")
		result = CreateFiller(inTemplate, inX, inY);
	else
		throw std::logic_error("This element is not defined: " + name);

	// MControlBase *control = dynamic_cast<MControlBase *>(result);

	MViewLayout layout = result->GetLayout();
	layout.mMargin = GetMargins(inTemplate);
	result->SetLayout(layout);

	if (not inTemplate.get_attribute("width").empty())
	{
		int32_t width = layout.mMargin.left + layout.mMargin.right;

		if (inTemplate.get_attribute("width") == "scrollbarwidth")
			width += kScrollbarWidth;
		else if (auto [m, ok] = GetAttributeSize(inTemplate, "width", mDLUX); ok)
			width += m;

		MRect frame = result->GetFrame();
		if (frame.width < width)
			result->ResizeFrame(width - frame.width, 0);

		result->RequestSize(width, -1);
	}

	if (not inTemplate.get_attribute("height").empty())
	{
		int32_t height = layout.mMargin.top + layout.mMargin.bottom;

		if (inTemplate.get_attribute("height") == "scrollbarheight")
			height += kScrollbarWidth;
		else if (auto [m, ok] = GetAttributeSize(inTemplate, "height", mDLUY); ok)
			height += m;

		MRect frame = result->GetFrame();
		if (frame.height < height)
			result->ResizeFrame(0, height - frame.height);
	}

	return result;
}

// --------------------------------------------------------------------

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
		GetImpl()->SetParentWindow(inParent);

	MWindow::Show();
	MWindow::Select();
}

void MDialog::RecalculateLayout()
{
	//	assert(mChildren.size() == 1);
	assert(mChildren.size() >= 1);

	for (auto child : mChildren)
		child->RecalculateLayout();

	mBounds = mChildren.front()->GetFrame();
	mFrame = mBounds;
	mBounds.x += mLayout.mMargin.left;
	mBounds.y += mLayout.mMargin.top;
	mFrame.width += mLayout.mMargin.left + mLayout.mMargin.right;
	mFrame.height += mLayout.mMargin.top + mLayout.mMargin.bottom;
}

void MDialog::ChildResized()
{
	MRect frame = mFrame;

	assert(mChildren.size() == 1);
	mBounds = mChildren.front()->GetFrame();
	mFrame = mBounds;
	mBounds.x += mLayout.mMargin.left;
	mBounds.y += mLayout.mMargin.top;
	mFrame.width += mLayout.mMargin.left + mLayout.mMargin.right;
	mFrame.height += mLayout.mMargin.top + mLayout.mMargin.bottom;

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

bool MDialog::AllowClose(bool inQuitting)
{
	return CancelClicked();
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

bool MDialog::KeyPressed(uint32_t inKeyCode, char32_t inUnicode, uint32_t inModifiers, bool inAutoRepeat)
{
	bool result = true;
	if (mOKButton and (inKeyCode == kEnterKeyCode or inKeyCode == kReturnKeyCode))
		mOKButton->SimulateClick();
	else if (mCancelButton and inKeyCode == kEscapeKeyCode)
		mCancelButton->SimulateClick();
	else
		result = false;
	return result;
}

void MDialog::KeyReleased(uint32_t inKeyValue, uint32_t inModifiers)
{
}

void MDialog::Modifiers(uint32_t inModifiers)
{
}

void MDialog::EnterText(const std::string &inText)
{
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

	MPrefs::SetString(inName, s.str());
}

void MDialog::RestorePosition(const char *inName)
{
	std::string s = MPrefs::GetString(inName, "");
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
	}
}

std::string MDialog::GetText(const std::string &inID) const
{
	std::string result;

	MView *view = FindSubViewByID(inID);
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
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
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
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
	if (dynamic_cast<MEdittext *>(view) == nullptr)
		throw std::runtime_error("unexpected nullptr");
	static_cast<MEdittext *>(view)->SetPasswordChar(inUnicode);
}

int32_t MDialog::GetValue(const std::string &inID) const
{
	int32_t result = -1;

	MView *view = FindSubViewByID(inID);
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
	if (dynamic_cast<MPopup *>(view) != nullptr)
		result = static_cast<MPopup *>(view)->GetValue();

	return result;
}

void MDialog::SetValue(const std::string &inID, int32_t inValue)
{
	MView *view = FindSubViewByID(inID);
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
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
	else if (nullptr == nullptr)
		throw std::runtime_error("unexpected nullptr");
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
	if (expander == nullptr)
		throw std::runtime_error("unexpected nullptr");
	return expander->IsOpen();
}

void MDialog::SetOpen(const std::string &inID, bool inOpen)
{
	MExpander *expander = dynamic_cast<MExpander *>(FindSubViewByID(inID));
	if (expander == nullptr)
		throw std::runtime_error("unexpected nullptr");
	expander->SetOpen(inOpen);
}

MColor MDialog::GetColor(const std::string &inID) const
{
	MColorSwatch *swatch = dynamic_cast<MColorSwatch *>(FindSubViewByID(inID));
	if (swatch == nullptr)
		throw std::runtime_error("unexpected nullptr");
	return swatch->GetColor();
}

void MDialog::SetColor(const std::string &inID, MColor inColor)
{
	MColorSwatch *swatch = dynamic_cast<MColorSwatch *>(FindSubViewByID(inID));
	if (swatch == nullptr)
		throw std::runtime_error("unexpected nullptr");
	swatch->SetColor(inColor);
}

void MDialog::SetEnabled(const std::string &inID, bool inEnabled)
{
	MView *view = FindSubViewByID(inID);
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
	if (inEnabled)
		view->Enable();
	else
		view->Disable();
}

void MDialog::SetVisible(const std::string &inID, bool inVisible)
{
	MView *view = FindSubViewByID(inID);
	if (view == nullptr)
		throw std::runtime_error("unexpected nullptr");
	if (inVisible)
		view->Show();
	else
		view->Hide();
}

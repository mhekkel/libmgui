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

#include "Gtk/MGtkApplicationImpl.hpp"
#include "Gtk/MGtkControlsImpl.hpp"
#include "Gtk/MGtkWindowImpl.hpp"

#include "MAcceleratorTable.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"

#include "mrsrc.hpp"

#include <charconv>

#include <zeep/xml/document.hpp>

using namespace std;
using namespace zeep;

namespace
{

int get_attribute_int(const zeep::xml::element *e, const char *name)
{
	std::string attr = e->get_attribute(name);

	int result = 0;
	auto r = std::from_chars(attr.data(), attr.data() + attr.length(), result);
	return r.ec != std::errc() ? 0 : result;
}

} // namespace

class MGtkDialogImpl : public MGtkWindowImpl
{
  public:
	MGtkDialogImpl(const string &inResource, MWindow *inParent)
		: MGtkWindowImpl(MWindowFlags(0), inParent)
		, mResponse(this, &MGtkDialogImpl::OnResponse)
		, mRsrc(inResource)
		, mResultIsOK(false)
	{
	}

	bool ShowModal() override;

	void Create(MRect inBounds, const std::string &inTitle) override;
	void Finish() override;

	bool OnKeyPressEvent(GdkEvent *inEvent) override;

	void Append(MGtkWidgetMixin *inChild, bool inExpand, MRect inMargins) override;

	void GetMargins(xml::element *inTemplate,
		int32_t &outLeftMargin, int32_t &outTopMargin, int32_t &outRightMargin, int32_t &outBottomMargin);

	MView *CreateControls(xml::element *inTemplate, int32_t inX, int32_t inY);

	MView *CreateButton(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateColorSwatch(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateCaption(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateCheckbox(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateRadiobutton(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateExpander(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateCombobox(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateEdittext(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreatePopup(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateScrollbar(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateSeparator(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateBox(xml::element *inTemplate, int32_t inX, int32_t inY, bool inHorizontal);
	MView *CreateTable(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreatePager(xml::element *inTemplate, int32_t inX, int32_t inY);
	MView *CreateListBox(xml::element *inTemplate, int32_t inX, int32_t inY);
	// MView *CreateListView(xml::element *inTemplate, int32_t inX, int32_t inY);

	uint32_t GetTextWidth(const string &inText, const wchar_t *inClass, int inPartID, int inStateID);

	string l(const string &s) { return GetLocalisedStringForContext(mRsrc, s); }

	void OnResponse(int32_t inResponseID)
	{
		// PRINT(("Response: %d", inResponseID));

		MDialog *dlog = static_cast<MDialog *>(mWindow);

		switch (inResponseID)
		{
			case GTK_RESPONSE_OK:
				mResultIsOK = true;
				if (dlog->OKClicked())
					dlog->Close();
				break;

			case GTK_RESPONSE_CANCEL:
			case GTK_RESPONSE_DELETE_EVENT:
				if (dlog->CancelClicked() and inResponseID != GTK_RESPONSE_DELETE_EVENT)
					dlog->Close();
				break;

			default:
				if (inResponseID > 0 and static_cast<uint32_t>(inResponseID) <= mResponseIDs.size())
					dlog->ButtonClicked(mResponseIDs[inResponseID - 1]);
				break;
		}
	}

	MSlot<void(int32_t)> mResponse;

	float mDLUX, mDLUY;

	string mRsrc;
	list<MRadiobutton *> mRadioGroup;
	vector<string> mResponseIDs;
	int32_t mDefaultResponse;
	bool mResultIsOK;
};

bool MGtkDialogImpl::OnKeyPressEvent(GdkEvent *inEvent)
{
	// PRINT(("MGtkDialogImpl::OnKeyPressEvent"));

	bool result = MGtkWidgetMixin::OnKeyPressEvent(inEvent);

	if (not result)
	{
		uint32_t keyCode = MapKeyCode(gdk_key_event_get_keyval(inEvent));
		uint32_t modifiers = MapModifier(gdk_event_get_modifier_state(inEvent));

		if ((keyCode == kEnterKeyCode or keyCode == kReturnKeyCode) and modifiers == 0)
		{
			OnResponse(mDefaultResponse);
			result = true;
		}
	}

	// PRINT(("MGtkDialogImpl::OnKeyPressEvent => %d", result));

	return result;
}

bool MGtkDialogImpl::ShowModal()
{
	MGtkWindowImpl::Select();

	// (void)gtk_dialog_run(GTK_DIALOG(GetWidget()));
	return mResultIsOK;
}

void MGtkDialogImpl::Create(MRect inBounds, const std::string &inTitle)
{
	GtkWidget *widget = gtk_dialog_new();
	THROW_IF_NIL(widget);

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());

	SetWidget(widget);

	mMapEvent.Connect(widget, "map-event");
	mResponse.Connect(widget, "response");
}

void MGtkDialogImpl::Finish()
{
	string resource = string("Dialogs/") + mRsrc + ".xml";
	mrsrc::istream rsrc(resource);

	if (not rsrc)
		THROW(("Dialog resource not found: %s", resource.c_str()));

	xml::document doc(rsrc);

	xml::element *dialog = doc.find_first("/dialog");
	if (dialog == nullptr)
		THROW(("Invalid dialog resource"));

	string title = l(dialog->get_attribute("title"));

	mFlags = kMFixedSize;
	string flags = dialog->get_attribute("flags");
	if (flags.find("flexible") != std::string::npos)
		mFlags = MWindowFlags(mFlags & ~kMFixedSize);
	if (flags.find("nosizebox") != std::string::npos)
		mFlags = MWindowFlags(mFlags | kMNoSizeBox);

	uint32_t minWidth = 40;
	if (not dialog->get_attribute("width").empty())
		minWidth = get_attribute_int(dialog, "width");
	uint32_t minHeight = 40;
	if (not dialog->get_attribute("height").empty())
		minHeight = get_attribute_int(dialog, "height");

	MRect bounds(0, 0, minWidth, minHeight);

	// now create the dialog
	Create(bounds, title);

	// setup the DLU values

	MDevice dev;
	dev.SetText("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	mDLUX = dev.GetTextWidth() / (52 * 4.0f);
	mDLUY = dev.GetLineHeight() / 8.0f;

	// create the dialog controls, all stacked on top of each other
	xml::element *vbox = dialog->find_first("vbox");
	if (vbox == nullptr)
		THROW(("Invalid dialog resource"));

	MView *content = CreateControls(vbox, 0, 0);
	//	content->SetBindings(true, true, true, true);

	for (MRadiobutton *radiobutton : mRadioGroup)
		radiobutton->SetGroup(mRadioGroup);

	//	MControlBase* control = dynamic_cast<MControlBase*>(content);
	//	if (control != nullptr)
	//		control->SetPadding(4);

	GtkWidget *dlogBox = gtk_dialog_get_content_area(GTK_DIALOG(GetWidget()));

	int padding = get_attribute_int(dialog, "padding");
	if (padding == 0)
		padding = 4;

	gtk_widget_set_margin_top(dlogBox, padding * mDLUY);
	gtk_widget_set_margin_bottom(dlogBox, padding * mDLUY);
	gtk_widget_set_margin_start(dlogBox, padding * mDLUX);
	gtk_widget_set_margin_end(dlogBox, padding * mDLUX);

	mWindow->AddChild(content);

	// the buttons

	xml::element *buttons = dialog->find_first("hbox");
	if (buttons == nullptr)
		THROW(("Invalid dialog resource"));

	mDefaultResponse = 0;

	for (auto button : *buttons)
	{
		if (button.name() == "button")
		{
			mResponseIDs.push_back(button.get_attribute("id"));

			int32_t response = mResponseIDs.size();
			if (button.get_attribute("id") == "ok")
				response = GTK_RESPONSE_OK;
			else if (button.get_attribute("id") == "cancel")
				response = GTK_RESPONSE_CANCEL;

			GtkWidget *wdgt = gtk_dialog_add_button(GTK_DIALOG(GetWidget()),
				l(button.get_attribute("title")).c_str(), response);

			if (button.get_attribute("default") == "true")
			{
#warning FIXME
				// gtk_widget_grab_default(wdgt);
				gtk_dialog_set_default_response(GTK_DIALOG(GetWidget()), response);
				mDefaultResponse = response;
			}
		}
	}
}

void MGtkDialogImpl::Append(MGtkWidgetMixin *inChild, bool inExpand, MRect inMargins)
{
	GtkWidget *box =
		gtk_dialog_get_content_area(GTK_DIALOG(GetWidget()));

	auto childWidget = inChild->GetWidget();
	gtk_widget_set_margin_top(childWidget, inMargins.y);
	gtk_widget_set_margin_bottom(childWidget, inMargins.height);
	gtk_widget_set_margin_start(childWidget, inMargins.x);
	gtk_widget_set_margin_end(childWidget, inMargins.width);

#warning FIXME
	// if (inPacking == ePackStart)
	// 	gtk_box_pack_start(GTK_BOX(box), childWidget, inExpand, inFill, 0);
	// else
	// 	gtk_box_pack_end(GTK_BOX(box), childWidget, inExpand, inFill, 0);

	gtk_box_append(GTK_BOX(box), childWidget);
}

void MGtkDialogImpl::GetMargins(xml::element *inTemplate,
	int32_t &outLeftMargin, int32_t &outTopMargin, int32_t &outRightMargin, int32_t &outBottomMargin)
{
	outLeftMargin = outTopMargin = outRightMargin = outBottomMargin = 0;

	outLeftMargin = outRightMargin =
		outTopMargin = outBottomMargin = get_attribute_int(inTemplate, "margin");

	outLeftMargin = outRightMargin = get_attribute_int(inTemplate, "margin-left-right");
	outTopMargin = outBottomMargin = get_attribute_int(inTemplate, "margin-top-bottom");

	outLeftMargin = get_attribute_int(inTemplate, "margin-left");
	outTopMargin = get_attribute_int(inTemplate, "margin-top");
	outRightMargin = get_attribute_int(inTemplate, "margin-right");
	outBottomMargin = get_attribute_int(inTemplate, "margin-bottom");
}

MView *MGtkDialogImpl::CreateButton(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	//	float idealWidth = GetTextWidth(title, VSCLASS_BUTTON, BP_PUSHBUTTON, PBS_NORMAL) + 10 * mDLUX;
	//	if (idealWidth < 50 * mDLUX)
	//		idealWidth = 50 * mDLUX;
	MRect bounds; //(inX, inY, static_cast<int32_t>(idealWidth), static_cast<int32_t>(14 * mDLUY));

	MButtonFlags flags = eBF_None;

	if (inTemplate->get_attribute("split") == "true")
		flags = eBF_Split;

	MButton *button = new MButton(id, bounds, title, flags);

	//	if (inTemplate->get_attribute("default") == "true")
	//		button->MakeDefault(true);
	//
	//	if (id == "ok" and mOKButton == nullptr)
	//		mOKButton = button;
	//
	//	if (id == "cancel")
	//		mCancelButton = button;

	AddRoute(button->eClicked, static_cast<MDialog *>(mWindow)->eButtonClicked);

	return button;
}

MView *MGtkDialogImpl::CreateColorSwatch(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	MRect bounds; //(inX, inY, static_cast<int32_t>(25 * mDLUX), static_cast<int32_t>(14 * mDLUY));

	MColor color(inTemplate->get_attribute("color").c_str());
	MColorSwatch *swatch = new MColorSwatch(id, bounds, color);

	AddRoute(swatch->eColorChanged, static_cast<MDialog *>(mWindow)->eColorChanged);

	return swatch;
}

MView *MGtkDialogImpl::CreateExpander(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	MRect bounds; //(inX, inY,
	              //		static_cast<int32_t>((13 + 3) * mDLUX) +
	              //			GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_LABEL, 0),
	              //		static_cast<int32_t>(12 * mDLUY));

	MExpander *expander = new MExpander(id, bounds, title);
	AddRoute(expander->eClicked, static_cast<MDialog *>(mWindow)->eButtonClicked);

	for (auto &b : *inTemplate)
	{
		if (b.get_attribute("if") == "WINDOWS")
			continue;

		expander->AddChild(CreateControls(&b, 0, 0));
	}

	return expander;
}

MView *MGtkDialogImpl::CreateCaption(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	if (id.empty())
		id = "caption";
	string text = l(inTemplate->get_attribute("text"));

	MRect bounds; //(inX, static_cast<int32_t>(inY), 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = GetTextWidth(text, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	return new MCaption(id, bounds, text);
}

MView *MGtkDialogImpl::CreateCheckbox(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = static_cast<int32_t>(14 * mDLUX) +
	              //		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	              //		//GetTextWidth(title, VSCLASS_BUTTON, BP_CHECKBOX, PBS_NORMAL);

	MCheckbox *checkbox = new MCheckbox(id, bounds, title);
	AddRoute(checkbox->eValueChanged,
		static_cast<MDialog *>(mWindow)->eCheckboxClicked);
	return checkbox;
}

MView *MGtkDialogImpl::CreateRadiobutton(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	string title = l(inTemplate->get_attribute("title"));

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(10 * mDLUY));
	              //	bounds.width = static_cast<int32_t>(14 * mDLUX) +
	              //		GetTextWidth(title, VSCLASS_TEXTSTYLE, TEXT_BODYTEXT, 0);
	              //		GetTextWidth(title, VSCLASS_BUTTON, BP_RADIOBUTTON, PBS_NORMAL);
	MRadiobutton *radiobutton = new MRadiobutton(id, bounds, title);
	AddRoute(radiobutton->eValueChanged,
		static_cast<MDialog *>(mWindow)->eRadiobuttonClicked);

	mRadioGroup.push_back(radiobutton);

	return radiobutton;
}

MView *MGtkDialogImpl::CreateCombobox(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	MRect bounds; //(inX, inY, static_cast<int32_t>(50 * mDLUX), static_cast<int32_t>(14 * mDLUY));
	MCombobox *combobox = new MCombobox(id, bounds);
	AddRoute(combobox->eValueChanged,
		static_cast<MDialog *>(mWindow)->eValueChanged);
	return combobox;
}

MView *MGtkDialogImpl::CreateEdittext(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	uint32_t flags = eMEditTextNoFlags;
	if (inTemplate->get_attribute("style").find("right") != std::string::npos)
		flags |= eMEditTextAlignRight;
	if (inTemplate->get_attribute("style").find("number") != std::string::npos)
		flags |= eMEditTextNumbers;
	if (inTemplate->get_attribute("style").find("multiline") != std::string::npos)
		flags |= eMEditTextMultiLine;
	if (inTemplate->get_attribute("style").find("readonly") != std::string::npos)
		flags |= eMEditTextReadOnly;

	MRect bounds; //(inX, inY, static_cast<int32_t>(5 * mDLUX), static_cast<int32_t>(14 * mDLUY));
	MEdittext *edittext = new MEdittext(id, bounds, flags);

	if (inTemplate->get_attribute("password") == "true")
		edittext->SetPasswordChar();

	AddRoute(edittext->eValueChanged,
		static_cast<MDialog *>(mWindow)->eTextChanged);
	return edittext;
}

MView *MGtkDialogImpl::CreatePopup(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	MRect bounds; //(inX, inY, 0, static_cast<int32_t>(14 * mDLUY));

	vector<string> choices;
	for (xml::element *option : inTemplate->find("./option"))
	{
		string label = option->get_content();
		//		int32_t width = GetTextWidth(label, VSCLASS_COMBOBOX, CP_DROPDOWNBUTTON, CBXSL_NORMAL);
		//		if (bounds.width < width)
		//			bounds.width = width;
		choices.push_back(label);
	}

	//	bounds.width += static_cast<int32_t>(14 * mDLUX);

	MPopup *popup = new MPopup(id, bounds);

	popup->SetChoices(choices);
	AddRoute(popup->eValueChanged,
		static_cast<MDialog *>(mWindow)->eValueChanged);

	return popup;
}

MView *MGtkDialogImpl::CreatePager(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MPager *result = new MPager(id, r);

	MRect b;

	for (xml::element *page : inTemplate->find("./page"))
	{
		if (page->get_attribute("if") == "WINDOWS")
			continue;

		MView *control = CreateControls(page, 0, 0);
		control->SetBindings(true, true, true, true);
		result->AddPage(control);

		control->RecalculateLayout();

		b |= control->GetFrame();;
	}

	r.width = b.width;
	r.height = b.height;

	result->SetFrame(r);
	result->SelectPage(0);

	return result;
}

MView *MGtkDialogImpl::CreateListBox(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");

	MRect r(inX, inY, 0, 0);
	MListBox *result = new MListBox(id, r);

	for (auto listitem : inTemplate->find("./listitem"))
	{
		string text = l(listitem->get_content());
		//		int32_t textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
		//		if (r.width < textWidth)
		//			r.width = textWidth;
		result->AddItem(text);
	}

	//	r.width += static_cast<int32_t>(mDLUX * 6);
	//	result->SetFrame(r);

	AddRoute(result->eValueChanged, static_cast<MDialog *>(mWindow)->eValueChanged);

	return result;
}

// MView *MGtkDialogImpl::CreateListView(xml::element *inTemplate, int32_t inX, int32_t inY)
// {
// 	string id = inTemplate->get_attribute("id");

// 	MRect r(inX, inY, 0, 0);
// 	MListView *result = new MListView(id, r);

// 	for (xml::element *listitem : inTemplate->find("./listitem"))
// 	{
// 		string text = l(listitem->get_content());
// 		//		int32_t textWidth = GetTextWidth(text, VSCLASS_LISTBOX, LBCP_ITEM, 0);
// 		//		if (r.width < textWidth)
// 		//			r.width = textWidth;
// 		result->AddItem(text);
// 	}

// 	//	r.width += static_cast<int32_t>(mDLUX * 6);
// 	//	result->SetFrame(r);

// 	AddRoute(result->eValueChanged, static_cast<MDialog *>(mWindow)->eValueChanged);

// 	return result;
// }

MView *MGtkDialogImpl::CreateSeparator(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	MRect bounds(inX, inY, 2, 2);
	return new MSeparator("separator", bounds);
}

MView *MGtkDialogImpl::CreateScrollbar(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	string id = inTemplate->get_attribute("id");
	string orientation = inTemplate->get_attribute("orientation");

	MRect bounds(inX, inY, kScrollbarWidth, kScrollbarWidth);

	if (orientation == "horizontal")
		bounds.width *= 2;
	else
		bounds.height *= 2;

	return new MScrollbar(id, bounds);
}

MView *MGtkDialogImpl::CreateBox(xml::element *inTemplate, int32_t inX, int32_t inY, bool inHorizontal)
{
	string id = inTemplate->get_attribute("id");

	uint32_t spacing = get_attribute_int(inTemplate, "spacing");
	uint32_t padding = get_attribute_int(inTemplate, "padding");

	bool expand = inTemplate->get_attribute("expand") == "true";
	bool fill = inTemplate->get_attribute("fill") == "true";
	bool homogeneous = inTemplate->get_attribute("homogeneous") == "true";

	MRect r(inX, inY, 0, 0);
	MView *result = new MBoxControl(id, r, inHorizontal, homogeneous, expand, fill, spacing, padding);

	for (auto &b : *inTemplate)
	{
		if (b.get_attribute("if") == "WINDOWS")
			continue;

		auto view = CreateControls(&b, 0, 0);
		if (view != nullptr)
			result->AddChild(view);
	}

	return result;
}

// MView *MGtkDialogImpl::CreateTable(xml::element *inTemplate, int32_t inX, int32_t inY)
// {
// 	string id = inTemplate->get_attribute("id");

// 	vector<MView *> views;
// 	uint32_t colCount = 0, rowCount = 0;

// 	for (xml::element *row : inTemplate->find("./row"))
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

MView *MGtkDialogImpl::CreateControls(xml::element *inTemplate, int32_t inX, int32_t inY)
{
	MView *result = nullptr;

	string name = inTemplate->name();

	if (name == "button")
		result = CreateButton(inTemplate, inX, inY);
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
		result = new MView(inTemplate->get_attribute("id"), MRect(inX, inY, 0, 0));
	else
		throw std::logic_error("This element is not defined: " + name);

	MControlBase *control = dynamic_cast<MControlBase *>(result);

	int32_t marginLeft, marginTop, marginRight, marginBottom;
	GetMargins(inTemplate, marginLeft, marginTop, marginRight, marginBottom);
	//	result->SetMargins(marginLeft, marginTop, marginRight, marginBottom);

	if (not inTemplate->get_attribute("width").empty())
	{
		int32_t width = marginLeft + marginRight;

		if (inTemplate->get_attribute("width") == "scrollbarwidth")
			width += kScrollbarWidth;
		else
			//			width += static_cast<int32_t>(std::stoi(inTemplate->get_attribute("width")) * mDLUX);
			width += get_attribute_int(inTemplate, "width");

		MGtkWidgetMixin *impl = dynamic_cast<MGtkWidgetMixin *>(control->GetControlImplBase());
		if (impl != nullptr)
			impl->RequestSize(width * mDLUX, -1);
	}

	//	if (not inTemplate->get_attribute("height").empty())
	//	{
	//		int32_t height = marginTop + marginBottom;
	//
	//		if (inTemplate->get_attribute("height") == "scrollbarheight")
	//			height += kScrollbarWidth;
	//		else
	////			height += static_cast<int32_t>(std::stoi(inTemplate->get_attribute("height")) * mDLUY);
	//			height += static_cast<int32_t>(std::stoi(inTemplate->get_attribute("height")));
	//
	//		MRect frame;
	//		result->GetFrame(frame);
	//		if (frame.height < height)
	//			result->ResizeFrame(0, height - frame.height);
	//	}

	if (control != nullptr)
	{
		auto x = get_attribute_int(inTemplate, "padding");
		MRect m{ x, x, x, x };
		control->SetLayout(inTemplate->get_attribute("expand") == "true", m);
	}

	return result;
}

uint32_t MGtkDialogImpl::GetTextWidth(const string &inText,
	const wchar_t *inClass, int inPartID, int inStateID)
{
	uint32_t result = 0;
	//	wstring text(c2w(inText));
	//
	//	HTHEME hTheme = ::OpenThemeData(GetHandle(), inClass);
	//
	//	if (hTheme != nullptr)
	//	{
	//		RECT r;
	//		THROW_IF_HRESULT_ERROR(::GetThemeTextExtent(hTheme, mDC,
	//			inPartID, inStateID, text.c_str(), text.length(), 0, nullptr, &r));
	//		result = r.right - r.left;
	//		::CloseThemeData(hTheme);
	//	}
	//	else
	//	{
	//		SIZE size;
	//		::GetTextExtentPoint32_t(mDC, text.c_str(), text.length(), &size);
	//		result = size.cx;
	//	}

	return result;
}

// --------------------------------------------------------------------

MWindowImpl *MWindowImpl::CreateDialog(const string &inResource, MWindow *inWindow)
{
	return new MGtkDialogImpl(inResource, inWindow);
}

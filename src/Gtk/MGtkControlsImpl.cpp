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

#include "MGtkControlsImpl.hpp"
#include "MGtkCanvasImpl.hpp"
#include "MGtkControlsImpl.inl"
#include "MGtkWindowImpl.hpp"

#include "MColorPicker.hpp"
#include "MUtils.hpp"

const int kScrollbarWidth = 16; //::GetThemeSysSize(nullptr, SM_CXVSCROLL);

// --------------------------------------------------------------------

MGtkSimpleControlImpl::MGtkSimpleControlImpl(MSimpleControl *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkSimpleControlImpl::CreateWidget()
{
	SetWidget(gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0));
}

void MGtkSimpleControlImpl::Append(MGtkWidgetMixin *inChild)
{
	assert(GTK_IS_BOX(GetWidget()));
	gtk_box_append(GTK_BOX(GetWidget()), inChild->GetWidget());
}

MSimpleControlImpl *MSimpleControlImpl::Create(MSimpleControl *inControl)
{
	return new MGtkSimpleControlImpl(inControl);
}

// --------------------------------------------------------------------

MGtkButtonImpl::MGtkButtonImpl(MButton *inButton, const std::string &inLabel,
	MButtonFlags inFlags)
	: MGtkControlImpl(inButton, inLabel)
	, mClicked(this, &MGtkButtonImpl::Clicked)
	, mDefault(false)
{
}

void MGtkButtonImpl::CreateWidget()
{
	SetWidget(gtk_button_new_with_label(mLabel.c_str()));
	mClicked.Connect(GetWidget(), "clicked");
}

void MGtkButtonImpl::Clicked()
{
	mControl->eClicked(mControl->GetID());
}

void MGtkButtonImpl::SimulateClick()
{
	g_signal_emit_by_name(G_OBJECT(GetWidget()), "activate");
}

void MGtkButtonImpl::MakeDefault(bool inDefault)
{
	mDefault = inDefault;
}

void MGtkButtonImpl::SetText(const std::string &inText)
{
	mLabel = inText;
	gtk_button_set_label(GTK_BUTTON(GetWidget()), mLabel.c_str());
}

void MGtkButtonImpl::GetIdealSize(int32_t &outWidth, int32_t &outHeight)
{
}

MButtonImpl *MButtonImpl::Create(MButton *inButton, const std::string &inLabel,
	MButtonFlags inFlags)
{
	return new MGtkButtonImpl(inButton, inLabel, inFlags);
}

// --------------------------------------------------------------------

MGtkExpanderImpl::MGtkExpanderImpl(MExpander *inExpander, const std::string &inLabel)
	: MGtkControlImpl(inExpander, inLabel)
	, mIsOpen(false)
{
}

void MGtkExpanderImpl::CreateWidget()
{
	SetWidget(gtk_expander_new(mLabel.c_str()));
}

void MGtkExpanderImpl::Append(MGtkWidgetMixin *inChild)
{
	gtk_expander_set_child(GTK_EXPANDER(GetWidget()), inChild->GetWidget());
}

void MGtkExpanderImpl::SetOpen(bool inOpen)
{
	if (inOpen != mIsOpen)
	{
		mIsOpen = inOpen;

		gtk_expander_set_expanded(GTK_EXPANDER(GetWidget()), mIsOpen);

		mControl->Invalidate();
		mControl->eClicked(mControl->GetID());
	}
}

bool MGtkExpanderImpl::IsOpen() const
{
	return mIsOpen;
}

void MGtkExpanderImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();
}

MExpanderImpl *MExpanderImpl::Create(MExpander *inExpander, const std::string &inLabel)
{
	return new MGtkExpanderImpl(inExpander, inLabel);
}

// --------------------------------------------------------------------

MGtkScrollbarImpl::MGtkScrollbarImpl(MScrollbar *inScrollbar)
	: MGtkControlImpl(inScrollbar, "")
	, eValueChanged(this, &MGtkScrollbarImpl::ValueChanged)
{
}

void MGtkScrollbarImpl::CreateWidget()
{
	MRect bounds = mControl->GetBounds();

	GtkAdjustment *adjustment = GTK_ADJUSTMENT(gtk_adjustment_new(0, 0, 1, 1, 1, 1));

	if (bounds.width > bounds.height)
		SetWidget(gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, adjustment));
	else
		SetWidget(gtk_scrollbar_new(GTK_ORIENTATION_VERTICAL, adjustment));

	eValueChanged.Connect(G_OBJECT(adjustment), "value-changed");
}

int32_t MGtkScrollbarImpl::GetValue() const
{
	int32_t result = 0;

	if (auto adj = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(GetWidget())); adj != nullptr)
		result = gtk_adjustment_get_value(adj);

	return result;
}

void MGtkScrollbarImpl::SetValue(int32_t inValue)
{
	GtkAdjustment *adj = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(GetWidget()));

	if (adj != nullptr)
	{
		int32_t minValue = GetMinValue();

		if (inValue < minValue)
			inValue = minValue;

		int32_t maxValue = GetMaxValue();
		if (inValue > maxValue)
			inValue = maxValue;
	}

	gtk_adjustment_set_value(adj, inValue);
}

int32_t MGtkScrollbarImpl::GetTrackValue() const
{
	return GetValue();
}

void MGtkScrollbarImpl::SetAdjustmentValues(int32_t inMinValue, int32_t inMaxValue,
	int32_t inScrollUnit, int32_t inPageSize, int32_t inValue)
{
	GtkAdjustment *adj = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(GetWidget()));

	if (adj != nullptr)
	{
		if (inValue < inMinValue)
			inValue = inMinValue;

		int32_t maxValue = inMaxValue + 1 - inPageSize;
		if (inValue > maxValue)
			inValue = maxValue;

		// PRINT(("min: %d, max: %d, scrollunit: %d, page: %d, value: %d\n", inMinValue, inMaxValue, inScrollUnit, inPageSize, inValue));

		gtk_adjustment_set_lower(adj, inMinValue);
		gtk_adjustment_set_upper(adj, inMaxValue + 1);
		gtk_adjustment_set_step_increment(adj, inScrollUnit);
		gtk_adjustment_set_page_increment(adj, inPageSize);
		gtk_adjustment_set_page_size(adj, inPageSize);
		gtk_adjustment_set_value(adj, inValue);
	}
}

int32_t MGtkScrollbarImpl::GetMinValue() const
{
	GtkAdjustment *adj = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(GetWidget()));
	return adj == nullptr ? 0 : gtk_adjustment_get_lower(adj);
}

int32_t MGtkScrollbarImpl::GetMaxValue() const
{
	GtkAdjustment *adj = gtk_scrollbar_get_adjustment(GTK_SCROLLBAR(GetWidget()));

	int32_t result = 0;
	if (adj != nullptr)
	{
		result = gtk_adjustment_get_upper(adj);
		if (gtk_adjustment_get_page_size(adj) > 1)
			result -= gtk_adjustment_get_page_size(adj);
	}

	return result;
}

void MGtkScrollbarImpl::ValueChanged()
{
	mControl->eScroll(kScrollToThumb);
}

MScrollbarImpl *MScrollbarImpl::Create(MScrollbar *inScrollbar)
{
	return new MGtkScrollbarImpl(inScrollbar);
}

// --------------------------------------------------------------------

MGtkStatusbarImpl::MGtkStatusbarImpl(MStatusbar *inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
	: MGtkControlImpl(inStatusbar, "")
	, mParts(inParts, inParts + inPartCount)
{
}

void MGtkStatusbarImpl::CreateWidget()
{
	SetEventMask(MEventMask::GestureClick);

	// GtkWidget *statusBar = gtk_statusbar_new();

	// GtkRequisition minimum, natural;
	// gtk_widget_get_preferred_size(statusBar, &minimum, &natural);

	// MRect bounds(0, 0, natural.width, natural.height);
	// g_object_ref_sink(statusBar);

	GtkWidget *dummy = gtk_label_new("test");

	GtkRequisition minimum, natural;
	gtk_widget_get_preferred_size(dummy, &minimum, &natural);

	MRect bounds(0, 0, natural.width, natural.height + 2);
	g_object_ref_sink(dummy);

	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);

	gtk_widget_set_size_request(box, natural.width, natural.height);

	for (auto part : mParts)
	{
		GtkWidget *label = gtk_label_new("");
		gtk_label_set_single_line_mode(GTK_LABEL(label), true);
		gtk_label_set_selectable(GTK_LABEL(label), false);
		gtk_label_set_xalign(GTK_LABEL(label), 0);
		gtk_label_set_yalign(GTK_LABEL(label), 0.5);
		gtk_label_set_ellipsize(GTK_LABEL(label), PANGO_ELLIPSIZE_END);

		gtk_box_append(GTK_BOX(box), label);

		if (part.width > 0)
			gtk_widget_set_size_request(label, part.width, -1);
		
		gtk_widget_set_hexpand(label, part.expand);
		
		gtk_widget_set_margin_start(label, part.margins.left);
		gtk_widget_set_margin_top(label, part.margins.top);
		gtk_widget_set_margin_end(label, part.margins.right);
		gtk_widget_set_margin_bottom(label, part.margins.bottom);

		mPanels.push_back(label);
	}

	SetWidget(box);
}

void MGtkStatusbarImpl::SetStatusText(uint32_t inPartNr, const std::string &inText, bool inBorder)
{
	if (inPartNr < mPanels.size())
		gtk_label_set_text(GTK_LABEL(mPanels[inPartNr]), inText.c_str());
}

void MGtkStatusbarImpl::OnGestureClickPressed(double inX, double inY, gint inClickCount)
{
	for (std::size_t ix = 0; auto panel : mPanels)
	{
		graphene_rect_t r;
		if (not gtk_widget_compute_bounds(panel, GetWidget(), &r))
			continue;

		if (inX >= r.origin.x and inX <= r.origin.x + r.size.width and
			inY >= r.origin.y and inY <= r.origin.y + r.size.height)
		{
			mControl->ePartClicked(ix, MRect(r.origin.x, r.origin.y, r.size.width, r.size.height));
			break;
		}

		++ix;
	}
}

MStatusbarImpl *MStatusbarImpl::Create(MStatusbar *inStatusbar, uint32_t inPartCount, MStatusBarElement inParts[])
{
	return new MGtkStatusbarImpl(inStatusbar, inPartCount, inParts);
}

// --------------------------------------------------------------------

MGtkComboboxImpl::MGtkComboboxImpl(MCombobox *inCombobox)
	: MGtkControlImpl(inCombobox, "")
{
}

void MGtkComboboxImpl::CreateWidget()
{
	GtkTreeModel *list_store = GTK_TREE_MODEL(gtk_list_store_new(1, G_TYPE_STRING));
	GtkWidget *wdgt = gtk_combo_box_new_with_model_and_entry(list_store);
	gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(wdgt), 0);

	SetWidget(wdgt);
}

std::string MGtkComboboxImpl::GetText() const
{
	std::string result;

	auto entry = gtk_combo_box_get_child(GTK_COMBO_BOX(GetWidget()));
	auto buffer = gtk_entry_get_buffer(GTK_ENTRY(entry));
	auto text = gtk_entry_buffer_get_text(GTK_ENTRY_BUFFER(buffer));
	if (text != nullptr)
		result = text;
	return result;
}

void MGtkComboboxImpl::SetText(const std::string &inText)
{
	auto i = find(mChoices.begin(), mChoices.end(), inText);
	if (i == mChoices.end())
	{
		mChoices.insert(mChoices.begin(), inText);
		i = mChoices.begin();

		SetChoices(mChoices);
	}

	GtkWidget *wdgt = GetWidget();

	if (not GTK_IS_COMBO_BOX(wdgt))
		throw std::runtime_error("Item is not a combo box");

	auto ix = i - mChoices.begin();
	if (ix != gtk_combo_box_get_active(GTK_COMBO_BOX(wdgt)))
		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), ix);
}

void MGtkComboboxImpl::SetChoices(const std::vector<std::string> &inChoices)
{
	mChoices = inChoices;

	GtkWidget *wdgt = GetWidget();

	if (wdgt != nullptr)
	{
		mChanged.Disconnect();

		if (not GTK_IS_COMBO_BOX(wdgt))
			throw std::runtime_error("Item is not a combo box");

		GtkListStore *model = GTK_LIST_STORE(gtk_combo_box_get_model(GTK_COMBO_BOX(wdgt)));
		gtk_list_store_clear(model);

		for (auto s : inChoices)
		{
			GtkTreeIter iter;

			gtk_list_store_append(model, &iter);
			gtk_list_store_set(model, &iter,
				0, s.c_str(),
				-1);
		}

		gtk_combo_box_set_active(GTK_COMBO_BOX(wdgt), 0);

		// connect signal
		mChanged.Connect(wdgt, "changed");
	}
}

void MGtkComboboxImpl::SetActive(int inActive)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), inActive);
}

int MGtkComboboxImpl::GetActive()
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(GetWidget()));
}

void MGtkComboboxImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	if (not mChoices.empty())
		SetChoices(mChoices);
}

void MGtkComboboxImpl::OnChanged()
{
	mControl->eValueChanged(mControl->GetID(), GetActive());
}

MComboboxImpl *MComboboxImpl::Create(MCombobox *inCombobox)
{
	return new MGtkComboboxImpl(inCombobox);
}

// --------------------------------------------------------------------

MGtkPopupImpl::MGtkPopupImpl(MPopup *inPopup)
	: MGtkControlImpl(inPopup, "")
{
}

void MGtkPopupImpl::CreateWidget()
{
	SetWidget(gtk_combo_box_text_new());
}

void MGtkPopupImpl::SetChoices(const std::vector<std::string> &inChoices)
{
	mChoices = inChoices;

	if (GetWidget() != nullptr)
	{
		for (auto s : inChoices)
			gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(GetWidget()), s.c_str());

		gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), 0);

		// connect signal
		mChanged.Connect(GetWidget(), "changed");
	}
}

void MGtkPopupImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	if (not mChoices.empty())
		SetChoices(mChoices);
}

int32_t MGtkPopupImpl::GetValue() const
{
	return gtk_combo_box_get_active(GTK_COMBO_BOX(GetWidget()));
}

void MGtkPopupImpl::SetValue(int32_t inValue)
{
	gtk_combo_box_set_active(GTK_COMBO_BOX(GetWidget()), inValue);
}

void MGtkPopupImpl::SetText(const std::string &inText)
{
	auto i = find(mChoices.begin(), mChoices.end(), inText);
	if (i != mChoices.end())
		SetValue(i - mChoices.begin());
}

std::string MGtkPopupImpl::GetText() const
{
	const char *s = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(GetWidget()));
	return s ? s : "";
}

MPopupImpl *MPopupImpl::Create(MPopup *inPopup)
{
	return new MGtkPopupImpl(inPopup);
}

// --------------------------------------------------------------------

MGtkEdittextImpl::MGtkEdittextImpl(MEdittext *inEdittext, uint32_t inFlags)
	: MGtkControlImpl(inEdittext, "")
	, mTextInserted(this, &MGtkEdittextImpl::TextInserted)
	, mTextDeleted(this, &MGtkEdittextImpl::TextDeleted)
	, mBuffer(nullptr)
	, mFlags(inFlags)
{
}

void MGtkEdittextImpl::CreateWidget()
{
	mBuffer = gtk_entry_buffer_new(nullptr, 0);

	mTextInserted.Connect(G_OBJECT(mBuffer), "inserted-text");
	mTextDeleted.Connect(G_OBJECT(mBuffer), "deleted-text");

	auto entry = gtk_entry_new();
	gtk_entry_set_buffer(GTK_ENTRY(entry), mBuffer);

	SetEventMask(MEventMask::KeyCapture);
	SetWidget(entry);

	gtk_widget_set_focus_on_click(entry, true);
}

std::string MGtkEdittextImpl::GetText() const
{
	const char *result = nullptr;
	if (GTK_IS_ENTRY(GetWidget()))
		result = gtk_entry_buffer_get_text(mBuffer);
	return result ? result : "";
}

void MGtkEdittextImpl::SetText(const std::string &inText)
{
	if (GTK_IS_ENTRY(GetWidget()))
		gtk_entry_buffer_set_text(mBuffer, inText.data(), inText.size());
}

void MGtkEdittextImpl::SetPasswordChar(uint32_t inUnicode)
{
	GtkWidget *wdgt = GetWidget();
	if (GTK_IS_ENTRY(wdgt))
	{
		gtk_entry_set_visibility(GTK_ENTRY(wdgt), false);
		gtk_entry_set_invisible_char(GTK_ENTRY(wdgt), inUnicode);
	}
	else
		throw std::runtime_error("item is not an entry");
}

bool MGtkEdittextImpl::OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
	auto [keycode, modifiers] = MapFromGdkKey(inKeyValue, inModifiers);
	mControl->eKeyDown(keycode, modifiers);
	return false;
}

void MGtkEdittextImpl::TextInserted(guint, gchar*, guint)
{
	mControl->eValueChanged(mControl->GetID(), GetText());
}

void MGtkEdittextImpl::TextDeleted(guint, guint)
{
	mControl->eValueChanged(mControl->GetID(), GetText());
}

MEdittextImpl *MEdittextImpl::Create(MEdittext *inEdittext, uint32_t inFlags)
{
	return new MGtkEdittextImpl(inEdittext, inFlags);
}

// --------------------------------------------------------------------

MGtkCaptionImpl::MGtkCaptionImpl(MCaption *inControl, const std::string &inText)
	: MGtkControlImpl(inControl, inText)
{
}

void MGtkCaptionImpl::CreateWidget()
{
	GtkWidget *widget = gtk_label_new(mLabel.c_str());
	//	gtk_label_set_justify(GTK_LABEL(widget), GTK_JUSTIFY_LEFT);
	gtk_label_set_xalign(GTK_LABEL(widget), 0);
	SetWidget(widget);
}

void MGtkCaptionImpl::SetText(const std::string &inText)
{
	mLabel = inText;
	if (GTK_IS_LABEL(GetWidget()))
		gtk_label_set_text(GTK_LABEL(GetWidget()), inText.c_str());
}

MCaptionImpl *MCaptionImpl::Create(MCaption *inCaption, const std::string &inText)
{
	return new MGtkCaptionImpl(inCaption, inText);
}

// --------------------------------------------------------------------

MGtkSeparatorImpl::MGtkSeparatorImpl(MSeparator *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkSeparatorImpl::CreateWidget()
{
	SetWidget(gtk_separator_new(GTK_ORIENTATION_HORIZONTAL));
}

MSeparatorImpl *MSeparatorImpl::Create(MSeparator *inSeparator)
{
	return new MGtkSeparatorImpl(inSeparator);
}

// --------------------------------------------------------------------

MGtkCheckboxImpl::MGtkCheckboxImpl(MCheckbox *inControl, const std::string &inText)
	: MGtkControlImpl(inControl, inText)
	, mChecked(false)
	, mToggled(this, &MGtkCheckboxImpl::Toggled)
{
}

void MGtkCheckboxImpl::CreateWidget()
{
	SetWidget(gtk_check_button_new_with_label(mLabel.c_str()));
	gtk_check_button_set_active(GTK_CHECK_BUTTON(GetWidget()), mChecked);
	mToggled.Connect(GetWidget(), "toggled");
}

void MGtkCheckboxImpl::Toggled()
{
	mControl->eValueChanged(mControl->GetID(), IsChecked());
}

bool MGtkCheckboxImpl::IsChecked() const
{
	return gtk_check_button_get_active(GTK_CHECK_BUTTON(GetWidget()));
}

void MGtkCheckboxImpl::SetChecked(bool inChecked)
{
	mChecked = inChecked;
	if (GetWidget())
		gtk_check_button_set_active(GTK_CHECK_BUTTON(GetWidget()), mChecked);
}

MCheckboxImpl *MCheckboxImpl::Create(MCheckbox *inCheckbox, const std::string &inText)
{
	return new MGtkCheckboxImpl(inCheckbox, inText);
}

// --------------------------------------------------------------------

MGtkRadiobuttonImpl::MGtkRadiobuttonImpl(MRadiobutton *inControl, const std::string &inText)
	: MGtkControlImpl(inControl, inText)
	, mChecked(false)
	, mToggled(this, &MGtkRadiobuttonImpl::Toggled)
{
}

void MGtkRadiobuttonImpl::CreateWidget()
{
	SetWidget(gtk_check_button_new_with_label(mLabel.c_str()));
	gtk_check_button_set_active(GTK_CHECK_BUTTON(GetWidget()), mChecked);

	if (mGroup != nullptr and mGroup != this)
	{
		gtk_check_button_set_group(GTK_CHECK_BUTTON(GetWidget()),
			GTK_CHECK_BUTTON(mGroup->GetWidget()));
	}

	mToggled.Connect(GetWidget(), "toggled");
}

void MGtkRadiobuttonImpl::Toggled()
{
	mControl->eValueChanged(mControl->GetID(), IsChecked());
}

bool MGtkRadiobuttonImpl::IsChecked() const
{
	return gtk_check_button_get_active(GTK_CHECK_BUTTON(GetWidget()));
}

void MGtkRadiobuttonImpl::SetChecked(bool inChecked)
{
	mChecked = inChecked;
	if (GetWidget())
		gtk_check_button_set_active(GTK_CHECK_BUTTON(GetWidget()), mChecked);
}


MRadiobuttonImpl *MRadiobuttonImpl::Create(MRadiobutton *inRadiobutton, const std::string &inText)
{
	return new MGtkRadiobuttonImpl(inRadiobutton, inText);
}

// // --------------------------------------------------------------------

// MGtkColorSwatchImpl::MGtkColorSwatchImpl(MColorSwatch *inColorSwatch, MColor inColor)
// 	: MGtkControlImpl(inColorSwatch, "")
// 	, eSelectedColor(this, &MGtkColorSwatchImpl::SelectedColor)
// 	, ePreviewColor(this, &MGtkColorSwatchImpl::PreviewColor)
// 	, mColorSet(this, &MGtkColorSwatchImpl::OnColorSet)
// 	, mColor(inColor)
// {
// }

// void MGtkColorSwatchImpl::CreateWidget()
// {
// 	GdkRGBA color = {};
// 	color.red = mColor.red / 255.0;
// 	color.green = mColor.green / 255.0;
// 	color.blue = mColor.blue / 255.0;
// 	color.alpha = 1.0;

// 	SetWidget(gtk_color_button_new_with_rgba(&color));

// 	mColorSet.Connect(GetWidget(), "color-set");
// }

// void MGtkColorSwatchImpl::OnColorSet()
// {
// 	GdkRGBA color = {};
// 	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(GetWidget()), &color);

// 	mColor.red = static_cast<uint8_t>(255 * color.red);
// 	mColor.green = static_cast<uint8_t>(255 * color.green);
// 	mColor.blue = static_cast<uint8_t>(255 * color.blue);

// 	mControl->eColorChanged(mControl->GetID(), mColor);
// }

// void MGtkColorSwatchImpl::SelectedColor(MColor inColor)
// {
// 	SetColor(inColor);
// 	mControl->eColorChanged(mControl->GetID(), mColor);
// }

// void MGtkColorSwatchImpl::PreviewColor(MColor inColor)
// {
// 	mControl->eColorPreview(mControl->GetID(), inColor);
// }

// MColor MGtkColorSwatchImpl::GetColor() const
// {
// 	return mColor;
// }

// void MGtkColorSwatchImpl::SetColor(MColor inColor)
// {
// 	mColor = inColor;

// 	GdkRGBA color = {};
// 	color.red = mColor.red / 255.0;
// 	color.green = mColor.green / 255.0;
// 	color.blue = mColor.blue / 255.0;
// 	color.alpha = 1.0;
// 	if (GTK_IS_COLOR_BUTTON(GetWidget()))
// 		gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(GetWidget()), &color);
// }

// void MGtkColorSwatchImpl::SetPalette(const std::vector<MColor> &colors)
// {
// 	mPalette = colors;

// 	std::vector<GdkRGBA> gtkColors(colors.size());
// 	for (std::size_t i = 0; auto c : colors)
// 	{
// 		GdkRGBA &color = gtkColors[i];
// 		color.red = c.red / 255.0;
// 		color.green = c.green / 255.0;
// 		color.blue = c.blue / 255.0;
// 		color.alpha = 1.0;
// 		++i;
// 	}

// 	if (GTK_IS_COLOR_BUTTON(GetWidget()))
// 		gtk_color_chooser_add_palette(GTK_COLOR_CHOOSER(GetWidget()), GTK_ORIENTATION_HORIZONTAL,
// 			9, gtkColors.size(), gtkColors.data());
// }

// MColorSwatchImpl *MColorSwatchImpl::Create(MColorSwatch *inColorSwatch, MColor inColor)
// {
// 	return new MGtkColorSwatchImpl(inColorSwatch, inColor);
// }

// --------------------------------------------------------------------

MGtkListBoxImpl::MGtkListBoxImpl(MListBox *inListBox)
	: MGtkControlImpl(inListBox, "")
	, mSelectionChanged(this, &MGtkListBoxImpl::OnSelectionChanged)
	, mStore(nullptr)
	, mNr(0)
{
}

void MGtkListBoxImpl::CreateWidget()
{
	mStore = gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);

	SetWidget(gtk_tree_view_new_with_model(GTK_TREE_MODEL(mStore)));

	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(GetWidget()), false);

	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("Author",
		renderer,
		"text", 0,
		NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(GetWidget()), column);

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));
	gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
	mSelectionChanged.Connect(G_OBJECT(selection), "changed");
}

void MGtkListBoxImpl::AddedToWindow()
{
	MGtkControlImpl::AddedToWindow();

	for (std::string &item : mItems)
		AddItem(item);

	mItems.clear();

	SetValue(0);
}

void MGtkListBoxImpl::AddItem(const std::string &inText)
{
	if (mStore == nullptr)
		mItems.push_back(inText);
	else
	{
		GtkTreeIter iter;

		gtk_list_store_append(mStore, &iter);
		gtk_list_store_set(mStore, &iter,
			0, inText.c_str(), 1, mNr++, -1);
	}
}

int32_t MGtkListBoxImpl::GetValue() const
{
	int32_t result = -1;

	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &result, -1);

	return result;
}

void MGtkListBoxImpl::SetValue(int32_t inValue)
{
	//	::SendMessage(GetWidget(), LB_SETCURSEL, inValue, 0);
}

void MGtkListBoxImpl::OnSelectionChanged()
{
	GtkTreeIter iter;
	GtkTreeModel *model;

	GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(GetWidget()));

	if (gtk_tree_selection_get_selected(selection, &model, &iter))
	{
		int32_t selected;
		gtk_tree_model_get(GTK_TREE_MODEL(mStore), &iter, 1, &selected, -1);
		mControl->eValueChanged(mControl->GetID(), selected);
	}
}

MListBoxImpl *MListBoxImpl::Create(MListBox *inListBox)
{
	return new MGtkListBoxImpl(inListBox);
}

// --------------------------------------------------------------------

MGtkBoxControlImpl::MGtkBoxControlImpl(MBoxControl *inControl, bool inHorizontal)
	: MGtkControlImpl(inControl, "")
	, mHorizontal(inHorizontal)
{
}

void MGtkBoxControlImpl::CreateWidget()
{
	SetWidget(gtk_box_new(mHorizontal ? GTK_ORIENTATION_HORIZONTAL : GTK_ORIENTATION_VERTICAL, 0));
}

MBoxControlImpl *MBoxControlImpl::Create(MBoxControl *inControl, bool inHorizontal)
{
	return new MGtkBoxControlImpl(inControl, inHorizontal);
}

void MGtkBoxControlImpl::Append(MGtkWidgetMixin *inChild)
{
	assert(GTK_IS_BOX(GetWidget()));
	gtk_box_append(GTK_BOX(GetWidget()), inChild->GetWidget());
}

void MGtkBoxControlImpl::AddChild(MControlBase *inControl, MControlBase *inBefore)
{
	if (auto ci = dynamic_cast<MGtkWidgetMixin *>(inControl->GetControlImplBase()); ci != nullptr)
	{
		mControl->MView::AddChild(inControl);

		if (inBefore)
		{
			auto bi = inBefore ? dynamic_cast<MGtkWidgetMixin *>(inBefore->GetControlImplBase()) : nullptr;

			if (bi == nullptr)
				gtk_box_reorder_child_after(GTK_BOX(GetWidget()), bi->GetWidget(), ci->GetWidget());
			else
				gtk_box_reorder_child_after(GTK_BOX(GetWidget()), ci->GetWidget(), nullptr);
		}
	}
}

// --------------------------------------------------------------------

MGtkStackControlImpl::MGtkStackControlImpl(MStackControl *inControl)
	: MGtkControlImpl(inControl, "")
{
}

void MGtkStackControlImpl::CreateWidget()
{
	SetWidget(gtk_stack_new());
}

void MGtkStackControlImpl::AddChild(MView *inChild, const std::string &inName)
{
	if (auto cntrl = dynamic_cast<MControlBase *>(inChild); cntrl != nullptr)
	{
		auto ci = cntrl->GetControlImplBase();
		auto wm = dynamic_cast<MGtkWidgetMixin *>(ci);
		mNames[wm] = inName;
	}
	mControl->MView::AddChild(inChild);
}

void MGtkStackControlImpl::Append(MGtkWidgetMixin *inChild)
{
	if (auto cntrl = dynamic_cast<MControlImplBase *>(inChild); cntrl != nullptr)
	{
		if (auto ci = dynamic_cast<MGtkWidgetMixin *>(cntrl); ci != nullptr)
		{
			auto name = mNames.at(ci);
			gtk_stack_add_named(GTK_STACK(GetWidget()), ci->GetWidget(), name.c_str());
		}
	}
}

void MGtkStackControlImpl::Select(const std::string &inName)
{
	if (GTK_IS_STACK(GetWidget()))
		gtk_stack_set_visible_child_name(GTK_STACK(GetWidget()), inName.c_str());
}

MStackControlImpl *MStackControlImpl::Create(MStackControl *inControl)
{
	return new MGtkStackControlImpl(inControl);
}
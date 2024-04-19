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

#include "MGtkWindowImpl.hpp"

template <class CONTROL>
MGtkControlImpl<CONTROL>::MGtkControlImpl(CONTROL *inControl, const std::string &inLabel)
	: CONTROL::MImpl(inControl)
	, mChanged(this, &MGtkControlImpl<CONTROL>::OnChanged)
	, mLabel(inLabel)
{
}

template <class CONTROL>
MGtkControlImpl<CONTROL>::~MGtkControlImpl()
{
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::OnDestroy()
{
	SetWidget(nullptr);

	if (this->mControl != nullptr)
	{
		this->mControl->SetImpl(nullptr);
		delete this;
	}
}

template <class CONTROL>
bool MGtkControlImpl<CONTROL>::IsFocus() const
{
	return GetWidget() != nullptr and gtk_widget_has_focus(GetWidget());
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::SetFocus()
{
	if (GetWidget() != nullptr)
	{
		bool b = gtk_widget_grab_focus(GetWidget());

		if (not b)
		{
			// try harder
			MWindow *window = this->mControl->GetWindow();
			if (window != nullptr)
			{
				auto w = static_cast<MGtkWindowImpl *>(window->GetImpl());
				gtk_window_set_focus(GTK_WINDOW(w->GetWidget()), GetWidget());
			}
		}
	}
}

template <class CONTROL>
std::string MGtkControlImpl<CONTROL>::GetText() const
{
	return mLabel;
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::SetText(const std::string &inText)
{
	mLabel = inText;

	GtkWidget *wdgt = GetWidget();
	if (GTK_IS_LABEL(wdgt))
		gtk_label_set_text(GTK_LABEL(wdgt), inText.c_str());
	else if (GTK_IS_BUTTON(wdgt))
		gtk_button_set_label(GTK_BUTTON(wdgt), inText.c_str());
	else if (GTK_IS_TEXT_VIEW(wdgt))
	{
		GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(wdgt));
		if (buffer == nullptr)
			throw std::runtime_error("Invalid text buffer");
		gtk_text_buffer_set_text(buffer, inText.c_str(), inText.length());
	}
	else if (GTK_IS_PROGRESS_BAR(wdgt))
	{
		gtk_progress_bar_set_text(GTK_PROGRESS_BAR(wdgt), inText.c_str());
		gtk_progress_bar_set_ellipsize(GTK_PROGRESS_BAR(wdgt),
			PANGO_ELLIPSIZE_MIDDLE);
	}
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::EnableSelf()
{
	if (auto w = GetWidget(); w != nullptr)
		gtk_widget_set_sensitive(w, true);
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::DisableSelf()
{
	if (auto w = GetWidget(); w != nullptr)
		gtk_widget_set_sensitive(w, false);
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::ShowSelf()
{
	if (GetWidget() != nullptr)
		gtk_widget_set_visible(GetWidget(), true);
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::HideSelf()
{
	if (GetWidget() != nullptr)
		gtk_widget_set_visible(GetWidget(), false);
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::GetParentAndBounds(MGtkWidgetMixin *&outParent, MRect &outBounds)
{
	MView *view = this->mControl;
	MView *parent = view->GetParent();

	outBounds = view->GetBounds();

	while (parent != nullptr)
	{
		view->ConvertToParent(outBounds.x, outBounds.y);

		MControlBase *control = dynamic_cast<MControlBase *>(parent);
		if (control != nullptr)
		{
			MGtkWidgetMixin *impl = dynamic_cast<MGtkWidgetMixin *>(control->GetControlImplBase());

			if (impl != nullptr and impl->GetWidget() != nullptr)
			{
				outParent = impl;
				break;
			}
		}

		MCanvas *canvas = dynamic_cast<MCanvas *>(parent);
		if (canvas != nullptr)
		{
			outParent = static_cast<MGtkCanvasImpl *>(canvas->GetImpl());
			break;
		}

		MWindow *window = dynamic_cast<MWindow *>(parent);
		if (window != nullptr)
		{
			outParent = static_cast<MGtkWindowImpl *>(window->GetImpl());
			break;
		}

		view = parent;
		parent = parent->GetParent();
	}
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::AddedToWindow()
{
	CreateWidget();

	MGtkWidgetMixin *parent = nullptr;
	MRect bounds;

	GetParentAndBounds(parent, bounds);
	assert(parent);

	MControlBase *control = this->mControl;

	auto layout = control->GetLayout();
	
	assert(GTK_IS_WIDGET(mWidget));

	gtk_widget_set_margin_top(mWidget, layout.mMargin.top);
	gtk_widget_set_margin_bottom(mWidget, layout.mMargin.bottom);
	gtk_widget_set_margin_start(mWidget, layout.mMargin.left);
	gtk_widget_set_margin_end(mWidget, layout.mMargin.right);

	gtk_widget_set_hexpand(mWidget, layout.mHExpand);
	gtk_widget_set_vexpand(mWidget, layout.mVExpand);

	parent->Append(this);
}

template <class CONTROL>
void MGtkControlImpl<CONTROL>::OnChanged()
{
}

// template <class CONTROL>
// void MGtkControlImpl<CONTROL>::OnPopupMenu()
// {
// 	// PRINT(("OnPopupMenu for %s", this->mControl->GetID().c_str()));

// 	int32_t x = 0, y = 0;

// #if GTK_CHECK_VERSION(3, 20, 0)
// 	auto seat = gdk_display_get_default_seat(gdk_display_get_default());
// 	auto mouse_device = gdk_seat_get_pointer(seat);
// #else
// 	auto devman = gdk_display_get_device_manager(gdk_display_get_default());
// 	auto mouse_device = gdk_device_manager_get_client_pointer(devman);
// #endif

// #warning "FIXME"
// 	// auto window = gdk_display_get_default_group(gdk_display_get_default());

// 	// if (window == nullptr)
// 	// 	window = gtk_widget_get_window(GetWidget());

// 	// gdk_window_get_device_position(window, mouse_device, &x, &y, NULL);
// 	// g_message ("pointer: %i %i", x, y);

// 	this->mControl->SecondaryMouseButtonClick(x, y);
// }

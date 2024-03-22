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

/*	$Id$
    Copyright Drs M.L. Hekkelman
    Created 28-09-07 11:18:30
*/

#include "MControls.hpp"
#include "MControls.inl"
#include "MUnicode.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include "Gtk/MGtkCanvasImpl.hpp"
#include "Gtk/MGtkControlsImpl.inl"
#include "Gtk/MGtkDeviceImpl.hpp"
#include "Gtk/MGtkWindowImpl.hpp"
#include "Gtk/MPrimary.hpp"

#include <cassert>
#include <iostream>

MGtkCanvasImpl::MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight)
	: MGtkControlImpl(inCanvas, "canvas")
{
	RequestSize(inWidth, inHeight);

	CreateIMContext();
}

MGtkCanvasImpl::~MGtkCanvasImpl()
{
}

void MGtkCanvasImpl::CreateWidget()
{
	SetWidget(gtk_drawing_area_new());

	gtk_widget_set_can_focus(GetWidget(), true);

	g_object_set_data(G_OBJECT(GetWidget()), "m-canvas", this);
}

bool MGtkCanvasImpl::OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers)
{
	bool result = false;

	// PRIMARY paste?
	switch (inButtonNr)
	{
		case 1:
			mControl->MouseDown(inX, inY, inClickCount, inModifiers);
			result = true;
			break;

		case 2:
			if (MPrimary::Instance().HasText())
			{
				std::string text;
				MPrimary::Instance().GetText(text);
				result = mControl->PastePrimaryBuffer(text);
			}
			break;

		case 3:
			PRINT(("Show Contextmenu!"));
			mControl->ShowContextMenu(inX, inY);
			break;
	}

	return result;
}

bool MGtkCanvasImpl::OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	mControl->MouseMove(inX, inY, inModifiers);
	return true;
}

bool MGtkCanvasImpl::OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	mControl->MouseUp(inX, inY, inModifiers);
	return true;
}

bool MGtkCanvasImpl::OnMouseExit()
{
	mControl->MouseExit();
	return true;
}

void MGtkCanvasImpl::Invalidate()
{
	if (GTK_IS_WIDGET(GetWidget()))
		gtk_widget_queue_draw(GetWidget());
}

bool MGtkCanvasImpl::OnConfigureEvent(GdkEvent *inEvent)
{
	// PRINT(("MGtkCanvasImpl::OnConfigureEvent"));

	MRect frame = mControl->GetFrame();
	MRect bounds;

	GtkWidget *parent = gtk_widget_get_parent(GetWidget());

	if (GTK_IS_VIEWPORT(parent))
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(parent, &allocation);

		bounds.width = allocation.width;
		bounds.height = allocation.height;

		double x, y;

		gtk_widget_translate_coordinates(parent, GetWidget(),
			bounds.x, bounds.y,
			&x, &y);

		bounds.x = x;
		bounds.y = y;
	}
	else
	{
		GtkAllocation allocation;
		gtk_widget_get_allocation(GetWidget(), &allocation);

		bounds.width = allocation.width;
		bounds.height = allocation.height;
	}

	// PRINT(("bounds(%d,%d,%d,%d)", bounds.x, bounds.y, bounds.width, bounds.height));

	mControl->ResizeFrame(bounds.width - frame.width, bounds.height - frame.height);

	return false;
}

bool MGtkCanvasImpl::OnKeyPressEvent(GdkEvent *inEvent)
{
	bool result = MGtkControlImpl<MCanvas>::OnKeyPressEvent(inEvent);

	if (not result)
	{
		const uint32_t kValidModifiersMask = gtk_accelerator_get_default_mod_mask();

		// PRINT(("OnKeyPressEvent(keyval=0x%x)", inEvent->keyval));

		uint32_t modifiers = MapModifier(gdk_event_get_modifier_state(inEvent) & kValidModifiersMask);
		uint32_t keyValue = MapKeyCode(gdk_key_event_get_keyval(inEvent));

		if (keyValue >= 0x60 and keyValue <= 0x7f and modifiers == kControlKey)
		{
			char ch = static_cast<char>(keyValue) - 0x60;
			std::string text(&ch, 1);
			result = mControl->HandleCharacter(text, mAutoRepeat);
		}
		else
			result = mControl->HandleKeyDown(keyValue, modifiers, mAutoRepeat);

		if (not result and modifiers == 0)
		{
			unicode ch = gdk_keyval_to_unicode(keyValue);

			if (ch != 0)
			{
				char s[8] = {};
				char *sp = s;
				uint32_t length = MEncodingTraits<kEncodingUTF8>::WriteUnicode(sp, ch);

				std::string text(s, length);
				result = mControl->HandleCharacter(text, mAutoRepeat);
			}
		}
	}

	return result;
}

// bool MGtkCanvasImpl::OnExposeEvent(GdkEventExpose* inEvent)
// {
// 	MRect update(inEvent->area.x, inEvent->area.y, inEvent->area.width, inEvent->area.height);

// 	mControl->Draw(update);

// 	return true;
// }

bool MGtkCanvasImpl::OnDrawEvent(cairo_t *inCairo)
{
	mCurrentCairo = inCairo;

	try
	{
		mControl->Draw();
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what() << '\n';
	}

	mCurrentCairo = nullptr;

	return true;
}

bool MGtkCanvasImpl::OnCommit(gchar *inText)
{
	std::string text(inText);
	return mControl->HandleCharacter(text, mAutoRepeat);
}

bool MGtkCanvasImpl::OnScrollEvent(GdkEvent *inEvent)
{
	uint32_t modifiers = MapModifier(gdk_event_get_modifier_state(inEvent));
	double x, y;
	gdk_event_get_position(inEvent, &x, &y);

	switch (gdk_scroll_event_get_direction(inEvent))
	{
		case GDK_SCROLL_UP:
			mControl->MouseWheel(x, y, 0, 1, modifiers);
			break;

		case GDK_SCROLL_DOWN:
			mControl->MouseWheel(x, y, 0, -1, modifiers);
			break;

		case GDK_SCROLL_LEFT:
			mControl->MouseWheel(x, y, 1, 0, modifiers);
			break;

		case GDK_SCROLL_RIGHT:
			mControl->MouseWheel(x, y, -1, 0, modifiers);
			break;

		case GDK_SCROLL_SMOOTH:
		{
			double delta_x, delta_y;
			gdk_scroll_event_get_deltas(inEvent, &delta_x, &delta_y);
			mControl->MouseWheel(x, y, -delta_x, -delta_y, modifiers);
			break;
		}
	}

	return true;
}

// void MGtkCanvasImpl::AcceptDragAndDrop(bool inFiles, bool inText)
// {
// }

// void MGtkCanvasImpl::StartDrag()
// {
// }

MCanvasImpl *MCanvasImpl::Create(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight)
{
	return new MGtkCanvasImpl(inCanvas, inWidth, inHeight);
}

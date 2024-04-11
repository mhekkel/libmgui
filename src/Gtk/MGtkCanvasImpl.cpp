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

#include "MGtkCanvasImpl.hpp"
#include "MGtkControlsImpl.inl"
#include "MGtkDeviceImpl.hpp"
#include "MGtkWindowImpl.hpp"

#include <cassert>
#include <fstream>
#include <iostream>

MGtkCanvasImpl::MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight,
	MCanvasDropTypes inDropTypes)
	: MGtkControlImpl(inCanvas, "canvas")
	, mResize(this, &MGtkCanvasImpl::Resize)
	, mDropTypes(inDropTypes)
{
	RequestSize(inWidth, inHeight);

	CreateIMContext();
}

MGtkCanvasImpl::~MGtkCanvasImpl()
{
}

void MGtkCanvasImpl::CreateWidget()
{
	MEventMask eventMask = MEventMask::All;

	if ((mDropTypes & MCanvasDropTypes::Text) == MCanvasDropTypes::Text)
		eventMask = eventMask | MEventMask::AcceptDropText;

	if ((mDropTypes & MCanvasDropTypes::File) == MCanvasDropTypes::File)
		eventMask = eventMask | MEventMask::AcceptDropFile;

	SetEventMask(eventMask);
	SetWidget(gtk_drawing_area_new());

	gtk_widget_set_focusable(GetWidget(), true);
	gtk_widget_set_can_focus(GetWidget(), true);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(GetWidget()), &MGtkCanvasImpl::DrawCB, this, nullptr);

	mResize.Connect(GetWidget(), "resize");
}

void MGtkCanvasImpl::OnGestureClickPressed(double inX, double inY, gint inClickCount)
{
	if (not mControl->GetWindow()->IgnoreSelectClick())
	{
		auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
			GTK_EVENT_CONTROLLER(mGestureClickPressed.GetSourceGObject())));

		mControl->ClickPressed(inX, inY, inClickCount, modifiers);
	}
}

void MGtkCanvasImpl::OnGestureClickReleased(double inX, double inY, gint inClickCount)
{
	auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mGestureClickReleased.GetSourceGObject())));

	mControl->ClickReleased(inX, inY, modifiers);
}

void MGtkCanvasImpl::OnGestureClickStopped()
{
}

void MGtkCanvasImpl::OnMiddleButtonClick(double inX, double inY, gint inClickCount)
{
	if (not mControl->GetWindow()->IgnoreSelectClick())
		mControl->MiddleMouseButtonClick(inX, inY);
}

void MGtkCanvasImpl::OnSecondaryButtonClick(double inX, double inY, gint inClickCount)
{
	if (not mControl->GetWindow()->IgnoreSelectClick())
		mControl->SecondaryMouseButtonClick(inX, inY);
}

void MGtkCanvasImpl::OnPointerEnter(double inX, double inY)
{
	auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mPointerEnter.GetSourceGObject())));

	mControl->PointerEnter(inX, inY, modifiers);
}

void MGtkCanvasImpl::OnPointerMotion(double inX, double inY)
{
	auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mPointerMotion.GetSourceGObject())));

	mControl->PointerMotion(inX, inY, modifiers);
}

void MGtkCanvasImpl::OnPointerLeave()
{
	mControl->PointerLeave();
}

bool MGtkCanvasImpl::OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
	auto [keycode, modifiers] = MapFromGdkKey(inKeyValue, inModifiers);

	return mControl->KeyPressed(keycode, gdk_keyval_to_unicode(inKeyValue),
		modifiers, mAutoRepeat);
}

void MGtkCanvasImpl::OnKeyReleased(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
	auto [keycode, modifiers] = MapFromGdkKey(inKeyValue, inModifiers);

	mControl->KeyReleased(keycode, modifiers);
}

void MGtkCanvasImpl::OnKeyModifiers(GdkModifierType inModifiers)
{
}

void MGtkCanvasImpl::Invalidate()
{
	if (GTK_IS_WIDGET(GetWidget()))
		gtk_widget_queue_draw(GetWidget());
}

void MGtkCanvasImpl::Resize(int width, int height)
{
	MRect frame = mControl->GetFrame();
	MRect bounds;
	bounds.width = width;
	bounds.height = height;

	GtkWidget *parent = gtk_widget_get_parent(GetWidget());

	if (GTK_IS_VIEWPORT(parent))
	{
		graphene_point_t pt{};

		if (gtk_widget_compute_point(parent, GetWidget(), &pt, &pt))
		{
			bounds.x = pt.x;
			bounds.y = pt.y;
		}
	}

	mControl->ResizeFrame(bounds.width - frame.width, bounds.height - frame.height);
}

void MGtkCanvasImpl::DrawCB(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
{
	MGtkCanvasImpl *self = reinterpret_cast<MGtkCanvasImpl *>(data);
	self->mCurrentCairo = cr;

	try
	{
		self->mControl->Draw();
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what() << '\n';
	}

	self->mCurrentCairo = nullptr;
}

void MGtkCanvasImpl::OnCommit(char *inText)
{
	mControl->EnterText({ inText } /* , mAutoRepeat */);
}

void MGtkCanvasImpl::OnDecelerate(double inVelX, double inVelY)
{
	mControl->ScrollDecelerate(inVelX, inVelY);
}

bool MGtkCanvasImpl::OnScroll(double inX, double inY)
{
	auto modifiers = gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mScroll.GetSourceGObject()));

	int32_t x, y;

	auto w = static_cast<MGtkWindowImpl *>(mControl->GetWindow()->GetImpl())->GetWidget();

	auto n = gtk_widget_get_native(w);
	auto surface = n ? gtk_native_get_surface(n) : nullptr;
	if (GDK_IS_SURFACE(surface))
	{
		GdkDisplay *display = gdk_display_get_default();
		GdkSeat *seat = gdk_display_get_default_seat(display);
		GdkDevice *device = gdk_seat_get_pointer(seat);

		double dx = 0, dy = 0;
		gdk_surface_get_device_position(surface, device, &dx, &dy, nullptr);

		gtk_widget_translate_coordinates(w, GetWidget(), dx, dy, &dx, &dy);

		x = dx;
		y = dy;
	}

	return mControl->Scroll(x, y, inX, inY, modifiers);
}

void MGtkCanvasImpl::OnScrollBegin()
{
	mControl->ScrollBegin();
}

void MGtkCanvasImpl::OnScrollEnd()
{
	mControl->ScrollEnd();
}

// --------------------------------------------------------------------

bool MGtkCanvasImpl::OnDrop(const GValue *inValue, double x, double y)
{
	bool result = false;

	if (G_VALUE_HOLDS(inValue, G_TYPE_FILE))
	{
		GFile *file = static_cast<GFile *>(g_value_get_object(inValue));
		std::filesystem::path p;

		if (G_IS_FILE(file))
		{
			auto path = g_file_get_path(file);
			if (path)
			{
				p = path;
				g_free(path);
			}
		}

		if (std::filesystem::exists(p))
			result = mControl->DragAcceptFile(x, y, p);
	}
	else if (G_VALUE_HOLDS(inValue, G_TYPE_STRING))
	{
		auto text = g_value_get_string(inValue);
		result = mControl->DragAcceptData(x, y, { text });
	}

	return result;
}

bool MGtkCanvasImpl::OnDropAccept(GdkDrop *inDrop)
{
	bool result = true;

	auto types = gdk_drop_get_formats(inDrop);

	if (gdk_content_formats_contain_gtype(types, G_TYPE_FILE))
		result = mControl->DragAcceptsFile();

	if (not result and gdk_content_formats_contain_mime_type(types, "text/plain"))
		result = mControl->DragAcceptsMimeType("text/plain");

	return result;
}

GdkDragAction MGtkCanvasImpl::OnDropEnter(double x, double y)
{
	mControl->DragEnter(x, y);
	return GDK_ACTION_COPY;
}

void MGtkCanvasImpl::OnDropLeave()
{
	mControl->DragLeave();
}

GdkDragAction MGtkCanvasImpl::OnDropMotion(double x, double y)
{
	mControl->DragMotion(x, y);
	return GDK_ACTION_COPY;
}

// --------------------------------------------------------------------

MCanvasImpl *MCanvasImpl::Create(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight,
	MCanvasDropTypes inDropTypes)
{
	return new MGtkCanvasImpl(inCanvas, inWidth, inHeight, inDropTypes);
}

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
#include "MPrimary.hpp"

#include <cassert>
#include <iostream>

MGtkCanvasImpl::MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight)
	: MGtkControlImpl(inCanvas, "canvas")
	, mResize(this, &MGtkCanvasImpl::Resize)
{
	RequestSize(inWidth, inHeight);

	CreateIMContext();
}

MGtkCanvasImpl::~MGtkCanvasImpl()
{
}

void MGtkCanvasImpl::CreateWidget()
{
	SetEventMask(MEventMask::All);
	SetWidget(gtk_drawing_area_new());

	gtk_widget_set_can_focus(GetWidget(), true);
	gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(GetWidget()), &MGtkCanvasImpl::Draw, this, nullptr);

	mResize.Connect(GetWidget(), "resize");
}

void MGtkCanvasImpl::OnGestureClickPressed(double inX, double inY, gint inClickCount)
{
	auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mGestureClickPressed.GetSourceGObject())));

	mControl->ClickPressed(inX, inY, inClickCount, modifiers);
}

void MGtkCanvasImpl::OnGestureClickReleased(double inX, double inY, gint inClickCount)
{
	auto modifiers = MapModifier(gtk_event_controller_get_current_event_state(
		GTK_EVENT_CONTROLLER(mGestureClickReleased.GetSourceGObject())));

	mControl->ClickReleased(inX, inY, inClickCount, modifiers);
}

void MGtkCanvasImpl::OnGestureClickStopped()
{
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
	return false;
}

void MGtkCanvasImpl::OnKeyReleased(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
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
		double x, y;
		gtk_widget_translate_coordinates(parent, GetWidget(), 0, 0, &x, &y);

		bounds.x = x;
		bounds.y = y;
	}

	mControl->ResizeFrame(bounds.width - frame.width, bounds.height - frame.height);
}

void MGtkCanvasImpl::Draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data)
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
	mControl->HandleCharacter({ inText }, mAutoRepeat);
}

void MGtkCanvasImpl::OnDecelerate(double inVelX, double inVelY)
{
	mControl->ScrollDecelerate(inVelX, inVelY);
}

bool MGtkCanvasImpl::OnScroll(double inX, double inY)
{
	return mControl->Scroll(inX, inY);
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

MCanvasImpl *MCanvasImpl::Create(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight)
{
	return new MGtkCanvasImpl(inCanvas, inWidth, inHeight);
}

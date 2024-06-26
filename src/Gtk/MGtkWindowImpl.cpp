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

#include "MGtkWindowImpl.hpp"
#include "MGtkApplicationImpl.hpp"
#include "MGtkControlsImpl.hpp"

#include "MApplication.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include "mrsrc.hpp"

#include <iostream>

// --------------------------------------------------------------------

G_BEGIN_DECLS

#define MGTK_TYPE_WINDOW (mgtk_window_get_type())

G_DECLARE_FINAL_TYPE(MGtkWindow, mgtk_window, MGTK, WINDOW, GtkApplicationWindow)

struct _MGtkWindow
{
	GtkApplicationWindow parent_instance;

	MGtkWindowImpl *m_impl;
};

typedef struct _MGtkWindow MGtkWindow;

G_DEFINE_FINAL_TYPE(MGtkWindow, mgtk_window, GTK_TYPE_APPLICATION_WINDOW)

G_END_DECLS

// --------------------------------------------------------------------

static void mgtk_window_finalize(GObject *object)
{
	MGtkWindow *self = MGTK_WINDOW(object);
	delete self->m_impl->GetWindow();

	G_OBJECT_CLASS(mgtk_window_parent_class)->finalize(object);
}

static void mgtk_window_class_init(MGtkWindowClass *klass)
{
	GObjectClass *object_class = G_OBJECT_CLASS(klass);

	object_class->finalize = mgtk_window_finalize;
}

static void mgtk_window_init(MGtkWindow *self)
{
}

MGtkWindow *mgtk_window_new(GtkApplication *app, MGtkWindowImpl *impl)
{
	GtkWindowGroup *group = gtk_window_group_new();

	MGtkWindow *result = static_cast<MGtkWindow *>(g_object_new(MGTK_TYPE_WINDOW, "application", app, nullptr));
	gtk_window_group_add_window(group, GTK_WINDOW(result));
	g_object_unref(group);

	result->m_impl = impl;

	return result;
}

// --------------------------------------------------------------------
//
//	MGtkWindowImpl
//

MGtkWindowImpl::MGtkWindowImpl(MWindowFlags inFlags, MWindow *inWindow)
	: MWindowImpl(inFlags, inWindow)
	, mCloseRequest(this, &MGtkWindowImpl::OnCloseRequest)
	, mIsActiveChanged(this, &MGtkWindowImpl::OnIsActiveChanged)
	, mFocus(this)
	, mConfigured(false)
{
}

MGtkWindowImpl::~MGtkWindowImpl()
{
}

void MGtkWindowImpl::CreateWindow(MRect inBounds, const std::string &inTitle)
{
	auto w = mgtk_window_new(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp(), this);

	GtkWidget *widget = GTK_WIDGET(w);
	if (widget == nullptr)
		throw std::runtime_error("unexpected nullptr");

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());

	SetWidget(widget);

	if (mFlags & kMDoNotHandleF10)
		gtk_window_set_handle_menubar_accel(GTK_WINDOW(widget), false);

	mCloseRequest.Connect(GetWidget(), "close-request");
	mIsActiveChanged.Connect(GetWidget(), "notify::is-active");

	if (mFlags & MWindowFlags::kMShowMenubar)
		MMenuBar::Instance().AddToWindow(this);
}

MGtkWindowImpl *MGtkWindowImpl::GetWindowImpl(GtkWindow *inW)
{
	return MGTK_IS_WINDOW(inW) ? MGTK_WINDOW(inW)->m_impl : nullptr;
}

void MGtkWindowImpl::Append(MGtkWidgetMixin *inChild)
{
	assert(gtk_window_get_child(GTK_WINDOW(GetWidget())) == nullptr);
	gtk_window_set_child(GTK_WINDOW(GetWidget()), inChild->GetWidget());
}

void MGtkWindowImpl::SetDefaultButton(MButton *inButton)
{
	auto ci = inButton->GetControlImplBase();
	auto wm = dynamic_cast<MGtkWidgetMixin *>(ci);
	gtk_window_set_default_widget(GTK_WINDOW(GetWidget()), wm->GetWidget());
}

void MGtkWindowImpl::SetParentWindow(MWindow *inWindow)
{
	gtk_window_set_transient_for(
		GTK_WINDOW(GetWidget()),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inWindow->GetImpl())->GetWidget()));
}

bool MGtkWindowImpl::OnCloseRequest()
{
	return mWindow->AllowClose(false) ? false : true;
}

void MGtkWindowImpl::OnIsActiveChanged(GParamSpec *inProperty)
{
	if (gtk_window_is_active(GTK_WINDOW(GetWidget())))
		mWindow->Activate();
	else
		mWindow->Deactivate();
}

void MGtkWindowImpl::Show()
{
	gtk_widget_set_visible(GetWidget(), true);
}

void MGtkWindowImpl::Hide()
{
	gtk_widget_set_visible(GetWidget(), false);
}

void MGtkWindowImpl::Select()
{
	gtk_window_present(GTK_WINDOW(GetWidget()));
}

void MGtkWindowImpl::Close()
{
	if (mWindow->AllowClose(false))
		gtk_window_close(GTK_WINDOW(GetWidget()));
}

void MGtkWindowImpl::SetTitle(std::string inTitle)
{
	gtk_window_set_title(GTK_WINDOW(GetWidget()), inTitle.c_str());
}

void MGtkWindowImpl::SetIconName(const std::string &inIconName)
{
	gtk_window_set_icon_name(GTK_WINDOW(GetWidget()), inIconName.c_str());
}

void MGtkWindowImpl::OnDestroy()
{
	SetWidget(nullptr);
}

void MGtkWindowImpl::OnMap()
{
	mWindow->Show();
}

void MGtkWindowImpl::OnUnmap()
{
	mWindow->Hide();
}

void MGtkWindowImpl::ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta)
{
	int w, h;
	gtk_window_get_default_size(GTK_WINDOW(GetWidget()), &w, &h);
	gtk_window_set_default_size(GTK_WINDOW(GetWidget()), w + inWidthDelta, h + inHeightDelta);
}

void MGtkWindowImpl::GetWindowPosition(MRect &outPosition) const
{
}

void MGtkWindowImpl::SetWindowPosition(MRect inPosition, bool inTransition)
{
}

void MGtkWindowImpl::UpdateNow()
{
}

void MGtkWindowImpl::SetCursor(MCursor inCursor)
{
	switch (inCursor)
	{
		case eNormalCursor:
			gtk_widget_set_cursor_from_name(GetWidget(), "default");
			break;

		case eIBeamCursor:
			gtk_widget_set_cursor_from_name(GetWidget(), "text");
			break;

		case eRightCursor:
			gtk_widget_set_cursor_from_name(GetWidget(), "pointer");
			break;
	
		case eBlankCursor:
			gtk_widget_set_cursor_from_name(GetWidget(), "none");
			break;
	}
}

void MGtkWindowImpl::ObscureCursor()
{
	gtk_widget_set_cursor_from_name(GetWidget(), "none");
}

void MGtkWindowImpl::ConvertToScreen(int32_t &ioX, int32_t &ioY) const
{
}

void MGtkWindowImpl::ConvertFromScreen(int32_t &ioX, int32_t &ioY) const
{
}

bool MGtkWindowImpl::OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
	auto [keycode, modifiers] = MapFromGdkKey(inKeyValue, inModifiers);

	return mWindow->KeyPressed(keycode, gdk_keyval_to_unicode(inKeyValue),
		modifiers, mAutoRepeat);
}

void MGtkWindowImpl::OnKeyReleased(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers)
{
	auto [keycode, modifiers] = MapFromGdkKey(inKeyValue, inModifiers);

	mWindow->KeyReleased(keycode, modifiers);
}

void MGtkWindowImpl::OnKeyModifiers(GdkModifierType inModifiers)
{
}

// --------------------------------------------------------------------

MWindowImpl *MWindowImpl::Create(const std::string &inTitle, MRect inBounds,
	MWindowFlags inFlags, MWindow *inWindow)
{
	MGtkWindowImpl *result = new MGtkWindowImpl(inFlags, inWindow);
	result->CreateWindow(inBounds, inTitle);
	return result;
}

void MWindow::GetMainScreenBounds(MRect &outRect)
{
	GdkDisplay *display = gdk_display_get_default();

	GdkSeat *seat = gdk_display_get_default_seat(display);
	GdkDevice *device = gdk_seat_get_pointer(seat);
	auto surface = gdk_device_get_surface_at_position(device, nullptr, nullptr);

	GdkRectangle r{ 0, 0, 1024, 768 };

	if (surface != nullptr)
	{
		double x, y;
		gdk_surface_get_device_position(surface, device, &x, &y, nullptr);

		auto monitor = gdk_display_get_monitor_at_surface(display, surface);

		if (GDK_IS_MONITOR(monitor))
			gdk_monitor_get_geometry(monitor, &r);
	}

	outRect = MRect(r.x, r.y, r.width, r.height);
}

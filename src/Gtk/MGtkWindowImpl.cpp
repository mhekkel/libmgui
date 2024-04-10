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

#include "xdg-activation-v1-protocol.h"

#include <gdk/wayland/gdkwayland.h>

#include <cstring>
#include <iostream>

// --------------------------------------------------------------------

G_BEGIN_DECLS

#define MGTK_TYPE_WINDOW (mgtk_window_get_type())

G_DECLARE_FINAL_TYPE(MGtkWindow, mgtk_window, MGTK, WINDOW, GtkApplicationWindow)

struct _MGtkWindow
{
	GtkApplicationWindow parent_instance;

	MGtkWindowImpl *m_impl;

	struct xdg_activation_token_v1 *xdg_activation_token = nullptr;
	wl_surface *surface = nullptr;
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
	{
		auto win = MGTK_WINDOW(GetWidget());

		if (win->surface == nullptr)
		{
			auto native = GTK_NATIVE(win);
			auto surface = gtk_native_get_surface(native);
			win->surface = gdk_wayland_surface_get_wl_surface(surface);
		}

		mWindow->Activate();
	}
	else
		mWindow->Deactivate();
}

void MGtkWindowImpl::Show()
{
	// gtk_window_present(GTK_WINDOW(GetWidget()));
	// gtk_window_present_with_time(GTK_WINDOW(GetWidget()), GDK_CURRENT_TIME);
	gtk_widget_show(GetWidget());
}

void MGtkWindowImpl::Hide()
{
	gtk_widget_hide(GetWidget());
}

// This is probably a no-no but I don't care, for now
static xdg_activation_v1 *xdg_activation_manager = nullptr;

static void display_handle_global(void *data, struct wl_registry *registry, uint32_t id,
	const char *interface, uint32_t version)
{
	if (strcmp(interface, "xdg_activation_v1") == 0)
		xdg_activation_manager = static_cast<xdg_activation_v1 *>(wl_registry_bind(registry, id, &xdg_activation_v1_interface, 1));
}

static void display_remove_global(void *data, struct wl_registry *registry, uint32_t id)
{
	if (xdg_activation_manager != nullptr)
	{
		xdg_activation_v1_destroy(xdg_activation_manager);
		xdg_activation_manager = nullptr;
	}
}

static const struct wl_registry_listener registry_listener = {
	display_handle_global,
	display_remove_global
};

static void handle_xdg_activation_done(void *data,
	struct xdg_activation_token_v1 *xdg_activation_token_v1,
	const char *token)
{
	auto *window = MGTK_WINDOW(data);
	if (xdg_activation_token_v1 == window->xdg_activation_token)
	{
		xdg_activation_v1_activate(xdg_activation_manager, token, window->surface);
		xdg_activation_token_v1_destroy(window->xdg_activation_token);
		window->xdg_activation_token = nullptr;
	}
}

static const struct xdg_activation_token_v1_listener activation_listener_xdg = {
	handle_xdg_activation_done
};

void MGtkWindowImpl::Select()
{
	auto win = MGTK_WINDOW(GetWidget());

	GdkDisplay *display = gdk_display_get_default();

	if (GDK_IS_WAYLAND_DISPLAY(display) and gdk_wayland_display_query_registry(display, "xdg_activation_v1") and win->surface != nullptr)
	{
		auto wl_display = gdk_wayland_display_get_wl_display(display);
		auto wl_registry = wl_display_get_registry(wl_display);

		GdkWaylandSeat *seat =
			GDK_WAYLAND_SEAT(gdk_display_get_default_seat(display));

		if (xdg_activation_manager == nullptr)
		{
			wl_registry_add_listener(wl_registry, &registry_listener, nullptr);
			wl_display_roundtrip(wl_display);
		}

		if (win->xdg_activation_token)
		{
			/* We're about to overwrite this with a new request. */
			xdg_activation_token_v1_destroy(win->xdg_activation_token);
		}

		win->xdg_activation_token = xdg_activation_v1_get_activation_token(xdg_activation_manager);

		xdg_activation_token_v1_add_listener(win->xdg_activation_token, &activation_listener_xdg, win);

		xdg_activation_token_v1_set_app_id(win->xdg_activation_token, gApp->GetApplicationID().c_str());

		xdg_activation_token_v1_set_serial(win->xdg_activation_token,
			0/* _gdk_wayland_seat_get_last_implicit_grab_serial(seat, NULL) */,
			gdk_wayland_seat_get_wl_seat(GDK_SEAT(seat)));

		xdg_activation_token_v1_set_surface(win->xdg_activation_token, win->surface);
		xdg_activation_token_v1_commit(win->xdg_activation_token);
	}

	gtk_window_present_with_time(GTK_WINDOW(GetWidget()), GDK_CURRENT_TIME);
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
}

void MGtkWindowImpl::ObscureCursor()
{
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

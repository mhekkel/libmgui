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

#include "MApplication.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include "mrsrc.hpp"

// #include <gdk/gdkx.h>

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
	// GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);
	// GtkWindowClass *window_class = GTK_WINDOW_CLASS(klass);

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
	// , mIsSuspendedChanged(this, &MGtkWindowImpl::OnIsSuspendedChanged)
	, mIsActiveChanged(this, &MGtkWindowImpl::OnIsActiveChanged)
	, mMainVBox(nullptr)
	, mFocus(this)
	, mConfigured(false)
{
}

MGtkWindowImpl::~MGtkWindowImpl()
{
}

void MGtkWindowImpl::Create(MRect inBounds, const std::string &inTitle)
{
	auto w = mgtk_window_new(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp(), this);
	// auto w = gtk_application_window_new(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());

	GtkWidget *widget = GTK_WIDGET(w);
	THROW_IF_NIL(widget);

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());

	SetWidget(widget);

	if (mFlags & kMDoNotHandleF10)
		gtk_window_set_handle_menubar_accel(GTK_WINDOW(widget), false);

	mCloseRequest.Connect(GetWidget(), "close-request");
	// mIsSuspendedChanged.Connect(GetWidget(), "notify::is-suspended");
	mIsActiveChanged.Connect(GetWidget(), "notify::is-active");

	// GList *iconList = nullptr;

	// mrsrc::rsrc appIconResource("Icons/appicon.png");
	// GInputStream *s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
	// THROW_IF_NIL(s);

	// GError *error = nullptr;
	// GdkPixbuf *icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	// if (icon)
	// 	iconList = g_list_append(iconList, icon);

	// if (error)
	// 	g_free(error);

	// mrsrc::rsrc smallAppIconResource("Icons/appicon.png");
	// s = g_memory_input_stream_new_from_data(smallAppIconResource.data(), smallAppIconResource.size(), nullptr);
	// THROW_IF_NIL(s);

	// icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
	// if (icon)
	// 	iconList = g_list_append(iconList, icon);

	// if (error)
	// 	g_free(error);

#warning FIXME
	// if (iconList)
	// 	gtk_window_set_icon_list(GTK_WINDOW(widget), iconList);

	//	GList* defaulIconList = gtk_window_get_default_icon_list();
	//	if (defaulIconList != nullptr)
	//	{
	//		gtk_window_set_icon_list(GTK_WINDOW(widget), defaulIconList);
	//		g_list_free(defaulIconList);
	//	}

	// mMapEvent.Connect(widget, "map-event");

	// if (mMenubar != nullptr)
	// {
	// 	mMenubar->AddToWindow(this);
	// 	mMenubar->SetTarget(mWindow);
	// }

	if (mFlags & MWindowFlags::kMShowMenubar)
		MMenuBar::Instance().AddToWindow(this);
}

MGtkWindowImpl *MGtkWindowImpl::GetWindowImpl(GtkWindow *inW)
{
	return MGTK_IS_WINDOW(inW) ? MGTK_WINDOW(inW)->m_impl : nullptr;
}

void MGtkWindowImpl::CreateMainVBox()
{
	mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_window_set_child(GTK_WINDOW(GetWidget()), mMainVBox);

	gtk_widget_set_hexpand(mMainVBox, true);
	gtk_widget_set_vexpand(mMainVBox, true);

	gtk_widget_show(mMainVBox);
}

void MGtkWindowImpl::AddStatusbarWidget(MGtkWidgetMixin *inChild)
{
	if (mMainVBox == nullptr)
		CreateMainVBox();

	gtk_box_append(GTK_BOX(mMainVBox), inChild->GetWidget());
	// gtk_widget_show_all(inChild->GetWidget());
}

void MGtkWindowImpl::Append(MGtkWidgetMixin *inChild)
{
	if (mMainVBox == nullptr)
		CreateMainVBox();

	gtk_box_append(GTK_BOX(mMainVBox), inChild->GetWidget());
}

void MGtkWindowImpl::SetTransientFor(MWindow *inWindow)
{
	gtk_window_set_transient_for(
		GTK_WINDOW(GetWidget()),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inWindow->GetImpl())->GetWidget()));
}

bool MGtkWindowImpl::OnCloseRequest()
{
	return mWindow->AllowClose(false) ? false : true;
}

// void MGtkWindowImpl::OnIsSuspendedChanged(GParamSpec *inProperty)
// {
// 	if (gtk_window_is_suspended(GTK_WINDOW(GetWidget())))
// 		mWindow->Hide();
// 	else
// 		mWindow->Show();
// }

void MGtkWindowImpl::OnIsActiveChanged(GParamSpec *inProperty)
{
	if (gtk_window_is_active(GTK_WINDOW(GetWidget())))
		mWindow->Activate();
	else
		mWindow->Deactivate();
}

void MGtkWindowImpl::Show()
{
	gtk_window_present_with_time(GTK_WINDOW(GetWidget()), GDK_CURRENT_TIME);
	gtk_widget_show(GetWidget());
}

void MGtkWindowImpl::Hide()
{
	gtk_widget_hide(GetWidget());
}

bool MGtkWindowImpl::Visible() const
{
	return gtk_window_get_focus_visible(GTK_WINDOW(GetWidget()));
#warning FIXME
	// return gtk_widget_get_window(GetWidget()) != nullptr and gdk_window_is_visible(gtk_widget_get_window(GetWidget()));
}

// // Mijn eigen xdo implementatie... zucht

// bool GetProperty(Display *display, Window window, const string &name,
// 	long maxLength, Atom &type, int &format, unsigned long &numItems, unsigned char *&prop)
// {
// 	Atom propertyAtom = XInternAtom(display, name.c_str(), false);
// 	unsigned long remainingBytes;
// 	return XGetWindowProperty(display, window, propertyAtom, 0, maxLength, False,
// 			   AnyPropertyType, &type, &format, &numItems, &remainingBytes, &prop) == Success;
// }

// bool PropertyExists(Display *display, Window window, const string &name)
// {
// 	Atom type = None;
// 	int format = 0;
// 	unsigned long numItems = 0;
// 	unsigned char *property = nullptr;

// 	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);

// 	if (property != nullptr)
// 		XFree(property);

// 	return result and numItems > 0;
// }

// bool GetXIDProperty(Display *display, Window window, const string &name, XID &xid)
// {
// 	Atom type = None;
// 	int format = 0;
// 	unsigned long numItems = 0;
// 	unsigned char *property = nullptr;

// 	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);

// 	if (result and numItems > 0 and format == 32 and property != nullptr)
// 		xid = *reinterpret_cast<XID *>(property);
// 	else
// 		result = false;

// 	if (property != nullptr)
// 		XFree(property);

// 	return result;
// }

// bool GetLongPropery(Display *display, Window window, const string &name, long &v)
// {
// 	Atom type = None;
// 	int format = 0;
// 	unsigned long numItems = 0;
// 	unsigned char *property = nullptr;

// 	bool result = GetProperty(display, window, name, 1024, type, format, numItems, property);

// 	if (result and numItems > 0 and format == 32 and property != nullptr)
// 		v = *reinterpret_cast<long *>(property);
// 	else
// 		result = false;

// 	if (property != nullptr)
// 		XFree(property);

// 	return result;
// }

// long GetDesktopForWindow(Display *display, Window window)
// {
// 	long desktop = -1;

// 	if (not GetLongPropery(display, window, "_NET_WM_DESKTOP", desktop) and
// 		not GetLongPropery(display, window, "_WIN_WORKSPACE", desktop))
// 	{
// 		//	PRINT(("Error getting desktop for window"));
// 	}

// 	return desktop;
// }

// long GetCurrentDesktop(Display *display)
// {
// 	long desktop = -1;

// 	Window root = DefaultRootWindow(display);

// 	if (not GetLongPropery(display, root, "_NET_CURRENT_DESKTOP", desktop) and
// 		not GetLongPropery(display, root, "_WIN_WORKSPACE", desktop))
// 	{
// 		//	PRINT(("Failed to get current desktop"));
// 	}

// 	return desktop;
// }

// void SetCurrentDesktop(Display *display, long desktop)
// {
// 	Window root = DefaultRootWindow(display);

// 	XEvent xev = {};
// 	xev.type = ClientMessage;
// 	xev.xclient.display = display;
// 	xev.xclient.window = root;
// 	xev.xclient.message_type = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);
// 	xev.xclient.format = 32;
// 	xev.xclient.data.l[0] = desktop;
// 	xev.xclient.data.l[1] = CurrentTime;

// 	int ret = XSendEvent(display, root, False, SubstructureNotifyMask | SubstructureRedirectMask, &xev);

// 	if (ret == 0)
// 		PRINT(("_NET_CURRENT_DESKTOP failed"));
// }

// bool ActivateWindow(Display *display, Window window)
// {
// 	// See: https://specifications.freedesktop.org/wm-spec/wm-spec-1.3.html#idm46463187634240

// 	long desktop = GetDesktopForWindow(display, window);
// 	long current = GetCurrentDesktop(display);

// 	if (desktop != current and desktop != -1)
// 		SetCurrentDesktop(display, desktop);

// 	XEvent xev = {};
// 	xev.type = ClientMessage;
// 	xev.xclient.display = display;
// 	xev.xclient.window = window;
// 	xev.xclient.message_type = XInternAtom(display, "_NET_ACTIVE_WINDOW", False);
// 	xev.xclient.format = 32;
// 	xev.xclient.data.l[0] = 1; // Comes from an application
// 	xev.xclient.data.l[1] = CurrentTime;

// 	XWindowAttributes attr;
// 	XGetWindowAttributes(display, window, &attr);
// 	int ret = XSendEvent(display, attr.screen->root, False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);

// 	if (ret == 0)
// 		PRINT(("_NET_ACTIVE_WINDOW failed"));

// 	return ret != 0;
// }

void MGtkWindowImpl::Select()
{
	//	PRINT(("Select Window (%p)", std::this_thread::get_id()));

	if (Visible())
		gtk_window_present_with_time(GTK_WINDOW(GetWidget()), GDK_CURRENT_TIME);
	else
		Show();

	// mWindow->BeFocus();
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

// string MGtkWindowImpl::GetTitle() const
//{
//	const char* title = gtk_window_get_title(GTK_WINDOW(GetWidget()));
//	return title ? title : "";
// }

// void MGtkWindowImpl::SetModifiedMarkInTitle(
//	bool		inModified)
//{
//	if (mModified != inModified)
//	{
//		mModified = inModified;
//		SetTitle(mTitle);
//	}
// }

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
#warning FIXME
	// //	PRINT(("MGtkWindowImpl::ResizeWindow(%d,%d)", inWidthDelta, inHeightDelta));
	// int w, h;
	// gtk_window_get_size(GTK_WINDOW(GetWidget()), &w, &h);
	// gtk_window_resize(GTK_WINDOW(GetWidget()), w + inWidthDelta, h + inHeightDelta);
}

void MGtkWindowImpl::GetWindowPosition(MRect &outPosition) const
{
#warning FIXME
	// int x, y;
	// gtk_window_get_position(GTK_WINDOW(GetWidget()), &x, &y);

	// int w, h;
	// gtk_window_get_size(GTK_WINDOW(GetWidget()), &w, &h);

	// outPosition = MRect(x, y, w, h);
}

void MGtkWindowImpl::SetWindowPosition(MRect inPosition, bool inTransition)
{
#warning FIXME
	// //	PRINT(("MGtkWindowImpl::SetWindowPosition"));
	// if (inTransition)
	// {
	// 	//		if (mTransitionThread != nullptr)
	// 	//			THROW(("SetWindowPosition called to fast"));
	// 	//
	// 	//		mTransitionThread =
	// 	//			new boost::thread(std::bind(&MGtkWindowImpl::TransitionTo, this, inPosition));
	// }
	// else
	// {
	// 	gtk_window_move(GTK_WINDOW(GetWidget()),
	// 		inPosition.x, inPosition.y);

	// 	gtk_window_resize(GTK_WINDOW(GetWidget()),
	// 		inPosition.width, inPosition.height);
	// }
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

// --------------------------------------------------------------------

MWindowImpl *MWindowImpl::Create(const std::string &inTitle, MRect inBounds,
	MWindowFlags inFlags, MWindow *inWindow)
{
	MGtkWindowImpl *result = new MGtkWindowImpl(inFlags, inWindow);
	result->Create(inBounds, inTitle);
	return result;
}

void MWindow::GetMainScreenBounds(MRect &outRect)
{
	GdkDisplay *display = gdk_display_get_default();

	GdkSeat *seat = gdk_display_get_default_seat(display);
	GdkDevice *device = gdk_seat_get_pointer(seat);
	auto surface = gdk_device_get_surface_at_position(device, nullptr, nullptr);

	double x, y;
	gdk_surface_get_device_position(surface, device, &x, &y, nullptr);

	auto monitor = gdk_display_get_monitor_at_surface(display, surface);
	GdkRectangle r{ 0, 0, 1024, 768 };

	if (GDK_IS_MONITOR(monitor))
		gdk_monitor_get_geometry(monitor, &r);

	outRect = MRect(r.x, r.y, r.width, r.height);
}

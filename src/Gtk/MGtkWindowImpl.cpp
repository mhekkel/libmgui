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

#include "Gtk/MGtkWindowImpl.hpp"
#include "Gtk/MGtkApplicationImpl.hpp"

#include "MApplication.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include "mrsrc.hpp"

// #include <gdk/gdkx.h>

#include <iostream>

using namespace std;

list<MWindow *> MGtkWindowImpl::sRecycle;

// --------------------------------------------------------------------
//
//	MGtkWindowImpl
//

MGtkWindowImpl::MGtkWindowImpl(MWindowFlags inFlags, MWindow *inWindow)
	: MWindowImpl(inFlags, inWindow)
	//	, mModified(false)
    //	, mTransitionThread(nullptr)
	// , mChildFocus(this, &MGtkWindowImpl::ChildFocus)
	// , mMapEvent(this, &MGtkWindowImpl::OnMapEvent)
	//	, mChanged(this, &MGtkWindowImpl::Changed)
	, mCloseRequest(this, &MGtkWindowImpl::OnCloseRequest)
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
	GtkWidget *widget = gtk_application_window_new(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	THROW_IF_NIL(widget);

	gtk_window_set_default_size(GTK_WINDOW(widget), inBounds.width, inBounds.height);
	gtk_window_set_title(GTK_WINDOW(widget), inTitle.c_str());

	SetWidget(widget);

	mCloseRequest.Connect(GetWidget(), "close-request");

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
		MMenuBar::instance().AddToWindow(this);

	//	mChanged.Connect(this, "on_changed");
}

void MGtkWindowImpl::AddMenubarWidget(GtkWidget *inWidget)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		// gtk_box_append(GTK_BOX(GetWidget()), mMainVBox);
		gtk_window_set_child(GTK_WINDOW(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}

	// gtk_box_pack_start(GTK_BOX(mMainVBox), inWidget, FALSE, FALSE, 0);
	gtk_box_append(GTK_BOX(mMainVBox), inWidget);
	// gtk_widget_show_all(inWidget);
}

void MGtkWindowImpl::AddStatusbarWidget(MGtkWidgetMixin *inChild)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_window_set_child(GTK_WINDOW(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}

	gtk_box_append(GTK_BOX(mMainVBox), inChild->GetWidget());
	// gtk_widget_show_all(inChild->GetWidget());
}

void MGtkWindowImpl::Append(MGtkWidgetMixin *inChild, bool inExpand, MRect inMargins)
{
	if (mMainVBox == nullptr)
	{
		mMainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_window_set_child(GTK_WINDOW(GetWidget()), mMainVBox);
		gtk_widget_show(mMainVBox);
	}

	auto childWidget = inChild->GetWidget();

	assert(GTK_IS_WIDGET(childWidget));

	gtk_widget_set_margin_top(childWidget, inMargins.y);
	gtk_widget_set_margin_bottom(childWidget, inMargins.height);
	gtk_widget_set_margin_start(childWidget, inMargins.x);
	gtk_widget_set_margin_end(childWidget, inMargins.width);

	gtk_widget_set_hexpand(childWidget, inExpand);

	gtk_box_append(GTK_BOX(mMainVBox), childWidget);
}

void MGtkWindowImpl::SetTransientFor(MWindow *inWindow)
{
	gtk_window_set_transient_for(
		GTK_WINDOW(GetWidget()),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inWindow->GetImpl())->GetWidget()));
}

bool MGtkWindowImpl::OnCloseRequest()
{
	return mWindow->AllowClose(false);
}

void MGtkWindowImpl::Show()
{
	gtk_window_present(GTK_WINDOW(GetWidget()));
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

	// auto gdkWindow = gtk_widget_get_window(GetWidget());

	// if (getenv("WAYLAND_DISPLAY") == nullptr)
	// {
	// 	auto d = gdk_x11_display_get_xdisplay(gdk_display_get_default());
	// 	auto w = gdk_x11_window_get_xid(gdkWindow);

	// 	if (d and w)
	// 		ActivateWindow(d, w);
	// 	else
	// 		gdk_window_focus(gdkWindow, GDK_CURRENT_TIME);
	// }
	// else
	// 	gdk_window_focus(gdkWindow, GDK_CURRENT_TIME);

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

void MGtkWindowImpl::SetTitle(string inTitle)
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

void MGtkWindowImpl::RecycleWindows()
{
	for (MWindow *w : sRecycle)
		delete w;
	sRecycle.clear();
}

void MGtkWindowImpl::OnDestroy()
{
	SetWidget(nullptr);

	sRecycle.push_back(mWindow);
}

void MGtkWindowImpl::OnMap()
{
	mWindow->Mapped();
}

void MGtkWindowImpl::OnUnmap()
{
	mWindow->Unmapped();
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

MWindowImpl *MWindowImpl::Create(const string &inTitle, MRect inBounds,
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

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

#include "Gtk/MGtkLib.hpp"

#include "MApplication.hpp"
#include "MError.hpp"
#include "MUtils.hpp"

#include <gdk/gdkkeysyms.h>

#include <pwd.h>
#include <sys/time.h>
#include <sys/types.h>

#include <cmath>
#include <sstream>
#include <stack>
#include <string>

using namespace std;

double GetLocalTime()
{
	struct timeval tv;

	gettimeofday(&tv, nullptr);

	return tv.tv_sec + tv.tv_usec / 1e6;
}

double GetDoubleClickTime()
{
	//	return ::GetDblTime() / 60.0;
	return 0.2;
}

string GetUserName(bool inShortName)
{
	string result;

	int uid = getuid();
	struct passwd *pw = getpwuid(uid);

	if (pw != nullptr)
	{
		if (inShortName or *pw->pw_gecos == 0)
			result = pw->pw_name;
		else
		{
			result = pw->pw_gecos;

			if (result.length() > 0)
			{
				string::size_type p = result.find(',');

				if (p != string::npos)
					result.erase(p, result.length() - p);

				p = result.find('&');

				if (p != string::npos)
					result.replace(p, 1, pw->pw_name);
			}
		}
	}

	return result;
}

bool IsModifierDown(int inModifierMask)
{
	bool result = false;

	GdkModifierType state;

	if (gtk_get_current_event_state(&state))
		result = (state & inModifierMask) != 0;

	return result;
}

// --------------------------------------------------------------------
// code to create a GdkPixbuf containing a single dot.
//
// use cairo to create an alpha mask, then set the colour
// into the pixbuf.

GdkPixbuf *CreateDot(MColor inColor, uint32_t inSize)
{
	// first draw in a buffer with cairo
	cairo_surface_t *cs = cairo_image_surface_create(
		CAIRO_FORMAT_ARGB32, inSize, inSize);

	cairo_t *c = cairo_create(cs);

	cairo_translate(c, inSize / 2., inSize / 2.);
	cairo_scale(c, inSize / 2., inSize / 2.);
	cairo_arc(c, 0., 0., 1., 0., 2 * M_PI);
	cairo_fill(c);

	cairo_destroy(c);

	// then copy the data over to a pixbuf;

	GdkPixbuf *result = gdk_pixbuf_new(
		GDK_COLORSPACE_RGB, true, 8, inSize, inSize);
	THROW_IF_NIL(result);

	unsigned char *dst = gdk_pixbuf_get_pixels(result);
	unsigned char *src = cairo_image_surface_get_data(cs);

	uint32_t dst_rowstride = gdk_pixbuf_get_rowstride(result);
	uint32_t src_rowstride = cairo_image_surface_get_stride(cs);
	uint32_t n_channels = gdk_pixbuf_get_n_channels(result);

	for (uint32_t x = 0; x < inSize; ++x)
	{
		for (uint32_t y = 0; y < inSize; ++y)
		{
			unsigned char *p = dst + y * dst_rowstride + x * n_channels;
			uint32_t cp = *reinterpret_cast<uint32_t *>(src + y * src_rowstride + x * 4);

			p[0] = inColor.red;
			p[1] = inColor.green;
			p[2] = inColor.blue;

			p[3] = (cp >> 24) & 0xFF;
		}
	}

	cairo_surface_destroy(cs);

	return result;
}

#include <dlfcn.h>

void OpenURI(const string &inURI)
{
	bool opened = false;

	void *libgnome = dlopen("libgnomevfs-2.so.0", RTLD_LAZY);
	if (libgnome != nullptr)
	{
		typedef gboolean (*gnome_vfs_url_show_func)(const char *);

		gnome_vfs_url_show_func gnome_url_show =
			(gnome_vfs_url_show_func)dlsym(libgnome, "gnome_vfs_url_show");

		if (gnome_url_show != nullptr)
		{
			int r = (*gnome_url_show)(inURI.c_str());
			opened = r == 0;
		}
	}

	if (not opened)
	{
		int err = system((string("gnome-open ") + inURI).c_str());
		if (err < 0)
			std::cerr << "Failed to open " << inURI << '\n';
	}
}

string GetHomeDirectory()
{
	const char *home = getenv("HOME");
	return home ? string(home) : "~";
}

string GetPrefsDirectory()
{
	const char *user_config_dir = g_get_user_config_dir();
	return user_config_dir ? (fs::path(user_config_dir) / kAppName).string() : (GetHomeDirectory() + '/' + kAppName);
}

string GetApplicationVersion()
{
	// return PACKAGE_VERSION;
	return "?.?";
}

uint32_t MapModifier(uint32_t inModifier)
{
	uint32_t result = 0;

	if (inModifier & GDK_SHIFT_MASK)
		result |= kShiftKey;
	if (inModifier & GDK_CONTROL_MASK)
		result |= kControlKey;

	return result;
}

uint32_t MapKeyCode(uint32_t inKeyValue)
{
	uint32_t result;

	switch (inKeyValue)
	{
		case GDK_KEY_Home: result = kHomeKeyCode; break;
		case GDK_KEY_Cancel: result = kCancelKeyCode; break;
		case GDK_KEY_End: result = kEndKeyCode; break;
		case GDK_KEY_Insert:
			result = kInsertKeyCode;
			break;
			//		case GDK_KEY_Be:		result = kBellKeyCode; break;
		case GDK_KEY_BackSpace: result = kBackspaceKeyCode; break;
		case GDK_KEY_ISO_Left_Tab:
			result = kTabKeyCode;
			break;
		case GDK_KEY_Tab: result = kTabKeyCode; break;
		case GDK_KEY_Linefeed:
			result = kLineFeedKeyCode;
			break;
			//		case GDK_KEY_:		result = kVerticalTabKeyCode; break;
		case GDK_KEY_Page_Up:
			result = kPageUpKeyCode;
			break;
			//		case GDK_KEY_:		result = kFormFeedKeyCode; break;
		case GDK_KEY_Page_Down: result = kPageDownKeyCode; break;
		case GDK_KEY_Return: result = kReturnKeyCode; break;
		case GDK_KEY_KP_Enter:
			result = kEnterKeyCode;
			break;
			//		case GDK_KEY_:		result = kFunctionKeyKeyCode; break;
		case GDK_KEY_Pause: result = kPauseKeyCode; break;
		case GDK_KEY_Escape: result = kEscapeKeyCode; break;
		case GDK_KEY_Clear: result = kClearKeyCode; break;
		case GDK_KEY_Left: result = kLeftArrowKeyCode; break;
		case GDK_KEY_Right: result = kRightArrowKeyCode; break;
		case GDK_KEY_Up: result = kUpArrowKeyCode; break;
		case GDK_KEY_Down:
			result = kDownArrowKeyCode;
			break;
			//		case GDK_KEY_:		result = kSpaceKeyCode; break;
		case GDK_KEY_Delete:
			result = kDeleteKeyCode;
			break;
			//		case GDK_KEY_:		result = kDivideKeyCode; break;
			//		case GDK_KEY_:		result = kMultiplyKeyCode; break;
			//		case GDK_KEY_:		result = kSubtractKeyCode; break;
		case GDK_KEY_Num_Lock: result = kNumlockKeyCode; break;
		case GDK_KEY_F1: result = kF1KeyCode; break;
		case GDK_KEY_F2: result = kF2KeyCode; break;
		case GDK_KEY_F3: result = kF3KeyCode; break;
		case GDK_KEY_F4: result = kF4KeyCode; break;
		case GDK_KEY_F5: result = kF5KeyCode; break;
		case GDK_KEY_F6: result = kF6KeyCode; break;
		case GDK_KEY_F7: result = kF7KeyCode; break;
		case GDK_KEY_F8: result = kF8KeyCode; break;
		case GDK_KEY_F9: result = kF9KeyCode; break;
		case GDK_KEY_F10: result = kF10KeyCode; break;
		case GDK_KEY_F11: result = kF11KeyCode; break;
		case GDK_KEY_F12: result = kF12KeyCode; break;
		case GDK_KEY_F13: result = kF13KeyCode; break;
		case GDK_KEY_F14: result = kF14KeyCode; break;
		case GDK_KEY_F15: result = kF15KeyCode; break;
		case GDK_KEY_F16: result = kF16KeyCode; break;
		case GDK_KEY_F17: result = kF17KeyCode; break;
		case GDK_KEY_F18: result = kF18KeyCode; break;
		case GDK_KEY_F19: result = kF19KeyCode; break;
		case GDK_KEY_F20: result = kF20KeyCode; break;

		default: result = inKeyValue; break;
	}

	return result;
}

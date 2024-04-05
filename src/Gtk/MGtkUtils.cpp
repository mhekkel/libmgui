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

#include "MGtkLib.hpp"

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

std::string GetUserName(bool inShortName)
{
	std::string result;

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
				std::string::size_type p = result.find(',');

				if (p != std::string::npos)
					result.erase(p, result.length() - p);

				p = result.find('&');

				if (p != std::string::npos)
					result.replace(p, 1, pw->pw_name);
			}
		}
	}

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

	GdkPixbuf *result = gdk_pixbuf_new(GDK_COLORSPACE_RGB, true, 8, inSize, inSize);
	if (result == nullptr)
		throw std::runtime_error("unexpected nullptr");

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

void OpenURI(const std::string &inURI)
{
	bool opened = false;

	GError *error = nullptr;
	opened = g_app_info_launch_default_for_uri(inURI.c_str(), nullptr, &error);

	if (error)
	{
		std::cerr << error->message << '\n';
		g_error_free(error);
	}

	if (not opened)
	{
		int err = system((std::string("gnome-open ") + inURI).c_str());
		if (err < 0)
			std::cerr << "Failed to open " << inURI << '\n';
	}
}

std::string GetHomeDirectory()
{
	const char *home = getenv("HOME");
	return home ? std::string(home) : "~";
}

std::string GetPrefsDirectory()
{
	const char *user_config_dir = g_get_user_config_dir();
	return user_config_dir ? (std::filesystem::path(user_config_dir) / kAppName).string() : (GetHomeDirectory() + '/' + kAppName);
}

std::string GetApplicationVersion()
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
	if (inModifier & GDK_ALT_MASK)
		result |= kOptionKey;

	return result;
}

uint32_t MapToKeyCode(uint32_t inKeyValue)
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

std::pair<uint32_t, uint32_t> MapFromGdkKey(uint32_t inKeyValue, uint32_t inModifier)
{
	uint32_t modifier, keycode;

	modifier = MapModifier(inModifier);

	switch (inKeyValue)
	{
		case GDK_KEY_KP_Space:
			modifier |= kNumPad;
			keycode = kSpaceKeyCode;
			break;
		case GDK_KEY_KP_Tab:
			modifier |= kNumPad;
			keycode = kTabKeyCode;
			break;
		case GDK_KEY_KP_Enter:
			modifier |= kNumPad;
			keycode = kEnterKeyCode;
			break;
		case GDK_KEY_KP_F1:
			modifier |= kNumPad;
			keycode = kF1KeyCode;
			break;
		case GDK_KEY_KP_F2:
			modifier |= kNumPad;
			keycode = kF2KeyCode;
			break;
		case GDK_KEY_KP_F3:
			modifier |= kNumPad;
			keycode = kF3KeyCode;
			break;
		case GDK_KEY_KP_F4:
			modifier |= kNumPad;
			keycode = kF4KeyCode;
			break;
		case GDK_KEY_KP_Home:
			modifier |= kNumPad;
			keycode = kHomeKeyCode;
			break;
		case GDK_KEY_KP_Left:
			modifier |= kNumPad;
			keycode = kLeftArrowKeyCode;
			break;
		case GDK_KEY_KP_Up:
			modifier |= kNumPad;
			keycode = kUpArrowKeyCode;
			break;
		case GDK_KEY_KP_Right:
			modifier |= kNumPad;
			keycode = kRightArrowKeyCode;
			break;
		case GDK_KEY_KP_Down:
			modifier |= kNumPad;
			keycode = kDownArrowKeyCode;
			break;
		// case GDK_KEY_KP_Prior:
		// 	modifier = kNumPad;
		// 	keycode = kPriorKeyCode;
		// 	break;
		case GDK_KEY_KP_Page_Up:
			modifier |= kNumPad;
			keycode = kPageUpKeyCode;
			break;
		// case GDK_KEY_KP_Next:
		// 	modifier = kNumPad;
		// 	keycode =
		// 	break;
		case GDK_KEY_KP_Page_Down:
			modifier |= kNumPad;
			keycode = kPageDownKeyCode;
			break;
		case GDK_KEY_KP_End:
			modifier |= kNumPad;
			keycode = kEndKeyCode;
			break;
		case GDK_KEY_KP_Begin:
			modifier |= kNumPad;
			keycode = kHomeKeyCode;
			break;
		case GDK_KEY_KP_Insert:
			modifier |= kNumPad;
			keycode = kInsertKeyCode;
			break;
		case GDK_KEY_KP_Delete:
			modifier |= kNumPad;
			keycode = kDeleteKeyCode;
			break;
		case GDK_KEY_KP_Equal:
			modifier |= kNumPad;
			keycode = '=';
			break;
		case GDK_KEY_KP_Multiply:
			modifier |= kNumPad;
			keycode = kMultiplyKeyCode;
			break;
		case GDK_KEY_KP_Add:
			modifier |= kNumPad;
			keycode = '+';
			break;
		case GDK_KEY_KP_Separator:
			modifier |= kNumPad;
			keycode = ',';
			break;
		case GDK_KEY_KP_Subtract:
			modifier |= kNumPad;
			keycode = '-';
			break;
		case GDK_KEY_KP_Decimal:
			modifier |= kNumPad;
			keycode = '.';
			break;
		case GDK_KEY_KP_Divide:
			modifier |= kNumPad;
			keycode = '/';
			break;
		case GDK_KEY_KP_0:
			modifier |= kNumPad;
			keycode = '0';
			break;
		case GDK_KEY_KP_1:
			modifier |= kNumPad;
			keycode = '1';
			break;
		case GDK_KEY_KP_2:
			modifier |= kNumPad;
			keycode = '2';
			break;
		case GDK_KEY_KP_3:
			modifier |= kNumPad;
			keycode = '3';
			break;
		case GDK_KEY_KP_4:
			modifier |= kNumPad;
			keycode = '4';
			break;
		case GDK_KEY_KP_5:
			modifier |= kNumPad;
			keycode = '5';
			break;
		case GDK_KEY_KP_6:
			modifier |= kNumPad;
			keycode = '6';
			break;
		case GDK_KEY_KP_7:
			modifier |= kNumPad;
			keycode = '7';
			break;
		case GDK_KEY_KP_8:
			modifier |= kNumPad;
			keycode = '8';
			break;
		case GDK_KEY_KP_9:
			modifier |= kNumPad;
			keycode = '9';
			break;
		default:
			keycode = MapToKeyCode(inKeyValue);
			break;
	}

	return { keycode, modifier };
}

std::pair<uint32_t, GdkModifierType> MapToGdkKey(uint32_t inKeyValue, uint32_t inModifier)
{
	uint32_t modifier = 0;
	if (inModifier & kShiftKey)
		modifier |= GDK_SHIFT_MASK;
	if (inModifier & kOptionKey)
		modifier |= GDK_ALT_MASK;
	if (inModifier & kControlKey)
		modifier |= GDK_CONTROL_MASK;

	uint32_t keycode;

	// TODO: numpad keys

	switch (inKeyValue)
	{
		case kHomeKeyCode: keycode = GDK_KEY_Home; break;
		case kCancelKeyCode: keycode = GDK_KEY_Cancel; break;
		case kEndKeyCode: keycode = GDK_KEY_End; break;
		case kInsertKeyCode: keycode = GDK_KEY_Insert; break;
		case kBackspaceKeyCode: keycode = GDK_KEY_BackSpace; break;
		case kTabKeyCode: keycode = GDK_KEY_Tab; break;
		case kLineFeedKeyCode: keycode = GDK_KEY_Linefeed; break;
		case kPageUpKeyCode: keycode = GDK_KEY_Page_Up; break;
		case kPageDownKeyCode: keycode = GDK_KEY_Page_Down; break;
		case kReturnKeyCode: keycode = GDK_KEY_Return; break;
		case kEnterKeyCode: keycode = GDK_KEY_KP_Enter; break;
		case kPauseKeyCode: keycode = GDK_KEY_Pause; break;
		case kEscapeKeyCode: keycode = GDK_KEY_Escape; break;
		// case kClearKeyCode: keycode = GDK_KEY_Clear; break;
		case kLeftArrowKeyCode: keycode = GDK_KEY_Left; break;
		case kRightArrowKeyCode: keycode = GDK_KEY_Right; break;
		case kUpArrowKeyCode: keycode = GDK_KEY_Up; break;
		case kDownArrowKeyCode: keycode = GDK_KEY_Down; break;
		case kDeleteKeyCode: keycode = GDK_KEY_Delete; break;
		case kDivideKeyCode: keycode = GDK_KEY_KP_Divide; break;
		case kMultiplyKeyCode: keycode = GDK_KEY_KP_Multiply; break;
		case kSubtractKeyCode: keycode = GDK_KEY_KP_Subtract; break;
		case kNumlockKeyCode: keycode = GDK_KEY_Num_Lock; break;
		case kF1KeyCode: keycode = GDK_KEY_F1; break;
		case kF2KeyCode: keycode = GDK_KEY_F2; break;
		case kF3KeyCode: keycode = GDK_KEY_F3; break;
		case kF4KeyCode: keycode = GDK_KEY_F4; break;
		case kF5KeyCode: keycode = GDK_KEY_F5; break;
		case kF6KeyCode: keycode = GDK_KEY_F6; break;
		case kF7KeyCode: keycode = GDK_KEY_F7; break;
		case kF8KeyCode: keycode = GDK_KEY_F8; break;
		case kF9KeyCode: keycode = GDK_KEY_F9; break;
		case kF10KeyCode: keycode = GDK_KEY_F10; break;
		case kF11KeyCode: keycode = GDK_KEY_F11; break;
		case kF12KeyCode: keycode = GDK_KEY_F12; break;
		case kF13KeyCode: keycode = GDK_KEY_F13; break;
		case kF14KeyCode: keycode = GDK_KEY_F14; break;
		case kF15KeyCode: keycode = GDK_KEY_F15; break;
		case kF16KeyCode: keycode = GDK_KEY_F16; break;
		case kF17KeyCode: keycode = GDK_KEY_F17; break;
		case kF18KeyCode: keycode = GDK_KEY_F18; break;
		case kF19KeyCode: keycode = GDK_KEY_F19; break;
		case kF20KeyCode: keycode = GDK_KEY_F20; break;

		default:
			keycode = inKeyValue;
			break;
	}

	return { keycode, GdkModifierType(modifier) };
}

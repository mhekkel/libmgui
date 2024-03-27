//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MGtkApplicationImpl.hpp"
#include "MGtkControlsImpl.hpp"
#include "MGtkWidgetMixin.hpp"
#include "MGtkWindowImpl.hpp"

#include "MApplication.hpp"
#include "MCommand.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MMenu.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include <zeep/xml/document.hpp>

#include <gdk/gdkkeysyms.h>

#include <algorithm>
#include <cstring>
#include <iostream>

namespace xml = zeep::xml;

// --------------------------------------------------------------------

struct MAccel
{
	MAccel(char32_t inAcceleratorKeyCode, uint32_t inAcceleratorModifiers)
	{
		std::ostringstream os;

		if (inAcceleratorKeyCode != 0)
		{
			if (inAcceleratorModifiers & kControlKey)
				os << "<Control>";
			if (inAcceleratorModifiers & kShiftKey)
				os << "<Shift>";
			if (inAcceleratorModifiers & kOptionKey)
				os << "<Alt>";

			switch (inAcceleratorKeyCode)
			{
				case kHomeKeyCode: os << "Home"; break;
				case kCancelKeyCode: os << "Cancel"; break;
				case kEndKeyCode: os << "End"; break;
				case kInsertKeyCode: os << "Insert"; break;
				case kBellKeyCode: os << "Bell"; break;
				case kBackspaceKeyCode: os << "Backspace"; break;
				case kTabKeyCode: os << "Tab"; break;
				case kLineFeedKeyCode: os << "LineFeed"; break;
				case kPageUpKeyCode: os << "PageUp"; break;
				case kPageDownKeyCode: os << "PageDown"; break;
				case kReturnKeyCode: os << "Return"; break;
				case kPauseKeyCode: os << "Pause"; break;
				case kEscapeKeyCode: os << "Escape"; break;
				case kLeftArrowKeyCode: os << "LeftArrow"; break;
				case kRightArrowKeyCode: os << "RightArrow"; break;
				case kUpArrowKeyCode: os << "UpArrow"; break;
				case kDownArrowKeyCode: os << "DownArrow"; break;
				case kSpaceKeyCode: os << "Space"; break;
				case kDeleteKeyCode: os << "Delete"; break;
				case kDivideKeyCode: os << "Divide"; break;
				case kMultiplyKeyCode: os << "Multiply"; break;
				case kSubtractKeyCode: os << "Subtract"; break;
				case kNumlockKeyCode: os << "Numlock"; break;
				case kF1KeyCode: os << "F1"; break;
				case kF2KeyCode: os << "F2"; break;
				case kF3KeyCode: os << "F3"; break;
				case kF4KeyCode: os << "F4"; break;
				case kF5KeyCode: os << "F5"; break;
				case kF6KeyCode: os << "F6"; break;
				case kF7KeyCode: os << "F7"; break;
				case kF8KeyCode: os << "F8"; break;
				case kF9KeyCode: os << "F9"; break;
				case kF10KeyCode: os << "F10"; break;
				case kF11KeyCode: os << "F11"; break;
				case kF12KeyCode: os << "F12"; break;
				case kF13KeyCode: os << "F13"; break;
				case kF14KeyCode: os << "F14"; break;
				case kF15KeyCode: os << "F15"; break;
				case kF16KeyCode: os << "F16"; break;
				case kF17KeyCode: os << "F17"; break;
				case kF18KeyCode: os << "F18"; break;
				case kF19KeyCode: os << "F19"; break;
				case kF20KeyCode: os << "F20"; break;
				case kEnterKeyCode: os << "Enter"; break;
				default: os << char(inAcceleratorKeyCode); break;
			}
		}

		mStr = os.str();
		mAccel[0] = mStr.c_str();
	}

	std::string mStr;
	const char *mAccel[2]{};
};

template <>
MCommandImpl *MCommand<void()>::RegisterCommand(MWindow *win, const std::string &inAction,
	char32_t inAcceleratorKeyCode, uint32_t inAcceleratorModifiers)
{
	auto impl = static_cast<MGtkWindowImpl *>(win->GetImpl());
	auto result = impl->RegisterAction(inAction, *this);

	if (inAcceleratorKeyCode)
	{
		std::tie(inAcceleratorKeyCode, inAcceleratorModifiers) = MapToGdkKey(inAcceleratorKeyCode, inAcceleratorModifiers);

		auto action = gtk_named_action_new(("win." + inAction).c_str());
		auto trigger = gtk_keyval_trigger_new(inAcceleratorKeyCode,
			GdkModifierType(inAcceleratorModifiers));
		auto shortcut = gtk_shortcut_new(trigger, action);
		impl->AddShortcut(shortcut);
	}

	return result;
}

template <>
MCommandImpl *MCommand<void()>::RegisterCommand(MControlBase *cntrl, const std::string &inAction,
	char32_t inAcceleratorKeyCode, uint32_t inAcceleratorModifiers)
{
	auto impl = dynamic_cast<MGtkWidgetMixin *>(cntrl->GetControlImplBase());
	auto result = impl->RegisterAction(inAction, *this);

	if (inAcceleratorKeyCode)
	{
		std::tie(inAcceleratorKeyCode, inAcceleratorModifiers) = MapToGdkKey(inAcceleratorKeyCode, inAcceleratorModifiers);

		auto action = gtk_named_action_new(("win." + inAction).c_str());
		auto trigger = gtk_keyval_trigger_new(inAcceleratorKeyCode, GdkModifierType(inAcceleratorModifiers));
		auto shortcut = gtk_shortcut_new(trigger, action);
		impl->AddShortcut(shortcut);
	}

	return result;
}

template <>
MCommandImpl *MCommand<void()>::RegisterCommand(MApplication *app, const std::string &action,
	char32_t inAcceleratorKeyCode, uint32_t inAcceleratorModifiers)
{
	auto impl = static_cast<MGtkApplicationImpl *>(app->GetImpl());

	auto result = impl->RegisterAction(action, *this);

	if (inAcceleratorKeyCode != 0)
	{
		MAccel accel(inAcceleratorKeyCode, inAcceleratorModifiers);
		gtk_application_set_accels_for_action(impl->GetGtkApp(), ("app." + action).c_str(), accel.mAccel);
	}

	return result;
}

// --------------------------------------------------------------------
// MGtkMenuImpl

class MGtkMenuImpl : public MMenu::MMenuImpl
{
  public:
	MGtkMenuImpl(MMenu *inMenu)
		: MMenuImpl(inMenu)
		, mGMenu(g_menu_new())
	{
	}

	void AppendItem(const std::string &inLabel, const std::string &inSection, const std::string &inAction, bool inStateful) override;
	void AppendRadioItems(const std::vector<std::string> &inLabels, const std::string &inSection, const std::string &inAction) override;
	void AppendSubmenu(MMenu *inMenu, const std::string &inSection) override;

	void RemoveItemsFromSection(const std::string &inSection) override;

	MMenu *GetSubmenu(uint32_t inIndex) const override;

	void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu) override;
	void AddToWindow(MWindowImpl *inWindow) override;

  protected:
	// for the menubar
	MGtkMenuImpl(MMenu *inMenu, GMenu *inGMenu)
		: MGtkMenuImpl(inMenu)
	{
		mGMenu = inGMenu;
	}

	void Append(const std::string &inSection, GMenuItem *item)
	{
		auto s = mSections.find(inSection);

		if (s == mSections.end())
		{
			auto menu = g_menu_new();
			std::tie(s, std::ignore) = mSections.emplace(inSection, menu);
			g_menu_append_section(mGMenu, nullptr, G_MENU_MODEL(menu));
		}

		g_menu_append_item(s->second, item);
		g_object_unref(item);
	}

	GMenu *mGMenu = nullptr;
	std::map<std::string, GMenu *> mSections;
	GApplication *mApp;
};

void MGtkMenuImpl::AppendItem(const std::string &inLabel, const std::string &inSection, const std::string &inAction, bool inStateful)
{
	Append(inSection, g_menu_item_new(inLabel.c_str(), inAction.empty() ? nullptr : inAction.c_str()));
}

void MGtkMenuImpl::AppendRadioItems(const std::vector<std::string> &inLabels, const std::string &inSection, const std::string &inAction)
{
	for (int i = 0; auto &label : inLabels)
	{
		std::string action = inAction + '(' + std::to_string(i++) + ')';
		AppendItem(inSection, label, action, false);
	}
}

void MGtkMenuImpl::AppendSubmenu(MMenu *inMenu, const std::string &inSection)
{
	Append(inSection, g_menu_item_new_submenu(inMenu->GetLabel().c_str(), G_MENU_MODEL(static_cast<MGtkMenuImpl *>(inMenu->impl())->mGMenu)));
	// g_menu_append_submenu(mGMenu, inMenu->GetLabel().c_str(),
	// 	G_MENU_MODEL(static_cast<MGtkMenuImpl *>(inMenu->impl())->mGMenu));
}

void MGtkMenuImpl::RemoveItemsFromSection(const std::string &inSection)
{
}

MMenu *MGtkMenuImpl::GetSubmenu(uint32_t inIndex) const
{
	return nullptr;
}

void MGtkMenuImpl::Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu)
{
}

void MGtkMenuImpl::AddToWindow(MWindowImpl *inWindow)
{
}

// --------------------------------------------------------------------

MMenu::MMenuImpl *MMenu::MMenuImpl::Create(MMenu *inMenu, bool inPopup)
{
	return new MGtkMenuImpl(inMenu);
}

// --------------------------------------------------------------------

class MGtkMenuBarImpl : public MGtkMenuImpl
{
  public:
	MGtkMenuBarImpl(MMenu *inMenu, GApplication *inApp)
		: MGtkMenuImpl(inMenu, g_menu_new())
	{
		gtk_application_set_menubar(GTK_APPLICATION(inApp), G_MENU_MODEL(mGMenu));
	}

	void AddToWindow(MWindowImpl *inWindow) override
	{
		auto impl = static_cast<MGtkWindowImpl *>(inWindow);
		gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(impl->GetWidget()), true);
	}
};

MMenu::MMenuImpl *MMenu::MMenuImpl::CreateBar(MMenu *inMenu)
{
	GApplication *app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	return new MGtkMenuBarImpl(inMenu, app);
}

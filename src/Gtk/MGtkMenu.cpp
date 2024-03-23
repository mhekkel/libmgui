//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Gtk/MGtkApplicationImpl.hpp"
#include "Gtk/MGtkWidgetMixin.hpp"
#include "Gtk/MGtkWindowImpl.hpp"

#include "MAcceleratorTable.hpp"
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

using namespace std;
namespace xml = zeep::xml;

// --------------------------------------------------------------------

template <>
MCommandImpl *MCommand<void()>::RegisterCommand(MWindow *win, const std::string &action)
{
	auto impl = static_cast<MGtkWindowImpl *>(win->GetImpl());
	return impl->RegisterAction(action, *this);
}

template <>
MCommandImpl *MCommand<void()>::RegisterCommand(MApplication *app, const std::string &action)
{
	auto impl = static_cast<MGtkApplicationImpl *>(app->GetImpl());
	return impl->RegisterAction(action, *this);
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
	Append(inSection, g_menu_item_new(inLabel.c_str(), inAction.c_str()));
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
	g_menu_append_submenu(mGMenu, inMenu->GetLabel().c_str(),
		G_MENU_MODEL(static_cast<MGtkMenuImpl* >(inMenu->impl())->mGMenu));
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

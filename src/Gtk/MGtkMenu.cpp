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
#include "MMenuImpl.hpp"
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
void MCommand<void()>::RegisterCommand(MWindow *win, const std::string &action)
{
	auto impl = static_cast<MGtkWindowImpl *>(win->GetImpl());
	impl->RegisterAction(action, *this);
}

template <>
void MCommand<void()>::RegisterCommand(MApplication *app, const std::string &action)
{
	auto impl = static_cast<MGtkApplicationImpl *>(app->GetImpl());
	impl->RegisterAction(action, *this);
}

// --------------------------------------------------------------------
// MMenuItem

struct MMenuItem;
typedef list<MMenuItem *> MMenuItemList;

struct MMenuItem
{
  public:
	MMenuItem(MMenu *inMenu, const string &inLabel, const std::string &inAction, GApplication *app)
		: mLabel(inLabel)
		, mMenu(inMenu)
	{
		if (not inAction.empty())
		{
			// auto name = inAction;
			// if (auto p = name.find('.'); p != std::string::npos)
			// 	name.erase(0, p + 1);

			// GSimpleAction *action = g_simple_action_new(name.c_str(), NULL);

			// g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(action));
			// g_signal_connect(action, "activate", G_CALLBACK(MGtkApplicationImpl::ActionActivated), app);

			// std::cout << "action name: " << g_action_get_name(G_ACTION(action)) << "\n";
			// std::cout << "enabled: " << std::boolalpha << g_action_get_enabled(G_ACTION(action)) << "\n";

			mGMenuItem = g_menu_item_new(_(mLabel.c_str()), inAction.c_str());
		}
		else
			mGMenuItem = g_menu_item_new(_(mLabel.c_str()), nullptr);
	}

	~MMenuItem()
	{
		g_object_unref(mGMenuItem);
	}

	void SetChecked(bool inChecked);
	void ItemToggled();

	GMenuItem *mGMenuItem;
	string mLabel;
	std::string mCommand;
	uint32_t mIndex = 0;
	MMenu *mMenu;
	MMenu *mSubMenu = nullptr;
	bool mEnabled = true;
	bool mCanCheck = false;
	bool mChecked = false;
	bool mInhibitCallBack = false;
};

// void MMenuItem::ItemCallback()
// {
// #warning FIXME
// 	// try
// 	// {
// 	// 	if (mMenu != nullptr and
// 	// 		mMenu->GetTarget() != nullptr and
// 	// 		not mInhibitCallBack)
// 	// 	{
// 	// 		bool process = true;

// 	// 		if (mCanCheck)
// 	// 		{
// 	// 			mChecked = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mGMenuItem));
// 	// 			process = mChecked;
// 	// 		}

// 	// 		uint32_t modifiers = 0;
// 	// 		uint32_t gdkModifiers = 0;

// 	// 		auto keyMap = gdk_keymap_get_for_display(gdk_display_get_default());
// 	// 		if (GDK_IS_KEYMAP(keyMap))
// 	// 			gdkModifiers = gdk_keymap_get_modifier_state(keyMap);

// 	// 		if (gdkModifiers & GDK_SHIFT_MASK)
// 	// 			modifiers |= kShiftKey;
// 	// 		if (gdkModifiers & GDK_CONTROL_MASK)
// 	// 			modifiers |= kControlKey;
// 	// 		if (gdkModifiers & GDK_MOD1_MASK)
// 	// 			modifiers |= kOptionKey;

// 	// 		MHandler *target = mMenu->GetTarget();
// 	// 		MWindow *window = dynamic_cast<MWindow *>(target);
// 	// 		if (window != nullptr)
// 	// 		{
// 	// 			auto focus = window->FindFocus();
// 	// 			if (focus != nullptr)
// 	// 				target = focus;
// 	// 		}

// 	// 		if (process and target != nullptr and not target->ProcessCommand(mCommand, mMenu, mIndex, modifiers))
// 	// 			PRINT(("Unhandled command: %s", (const char *)MCommandToString(mCommand)));
// 	// 	}
// 	// }
// 	// catch (const exception &e)
// 	// {
// 	// 	DisplayError(e);
// 	// }
// 	// catch (...)
// 	// {
// 	// }
// }

void MMenuItem::SetChecked(bool inChecked)
{
#warning FIXME
	// if (inChecked != mChecked and GTK_IS_CHECK_MENU_ITEM(mGMenuItem))
	// {
	// 	mInhibitCallBack = true;
	// 	mChecked = inChecked;
	// 	gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(mGMenuItem), mChecked);
	// 	mInhibitCallBack = false;
	// }
}

// --------------------------------------------------------------------
// MGtkMenuImpl

class MGtkMenuImpl : public MMenuImpl
{
  public:
	MGtkMenuImpl(MMenu *inMenu, GApplication *inApp);

	void SetItemState(uint32_t inItem, bool inEnabled, bool inChecked) override;
	void AppendItem(const string &inLabel, const std::string &inAction) override;
	void AppendSubmenu(MMenu *inSubmenu) override;
	void AppendSeparator() override;
	void AppendCheckbox(const string &inLabel, const std::string &inAction) override;
	void AppendRadiobutton(const string &inLabel, const std::string &inAction) override;
	uint32_t CountItems() const override;
	void RemoveItems(uint32_t inFirstIndex, uint32_t inCount) override;
	string GetItemLabel(uint32_t inIndex) const override;
	void SetItemCommand(uint32_t inIndex, const std::string &inAction) override;
	uint32_t GetItemCommand(uint32_t inIndex) const override;
	MMenu *GetSubmenu(uint32_t inIndex) const override;
	void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu) override;
	void AddToWindow(MWindowImpl *inWindow) override;
	void MenuUpdated() override;

  protected:
	MMenuItem *CreateNewItem(const std::string &inLabel, const std::string &inAction, GSList **ioRadioGroup);

	// for the menubar
	MGtkMenuImpl(MMenu *inMenu, GMenu *inGMenu, GApplication *inApp)
		: MGtkMenuImpl(inMenu, inApp)
	{
		mGMenu = inGMenu;
	}

	GMenu *mGMenu = nullptr;
	// GMenu *mSection = nullptr;
	GApplication *mApp;
	std::string mLabel;
	MMenuItemList mItems;
	GSList *mRadioGroup;
	int32_t mPopupX, mPopupY;
};

MGtkMenuImpl::MGtkMenuImpl(MMenu *inMenu, GApplication *inApp)
	: MMenuImpl(inMenu)
	, mGMenu(g_menu_new())
	, mApp(inApp)
	, mRadioGroup(nullptr)
{
}

// void MGtkMenuImpl::SetTarget(MHandler *inHandler)
// {
// 	mTarget = inHandler;

// 	for (auto mi : mItems)
// 	{
// 		if (mi->mSubMenu != nullptr)
// 			mi->mSubMenu->SetTarget(inHandler);
// 	}
// }

void MGtkMenuImpl::SetItemState(uint32_t inIndex, bool inEnabled, bool inChecked)
{
	// if (inIndex >= mItems.size())
	// 	THROW(("Item index out of range"));

	// MMenuItemList::iterator item = mItems.begin();
	// advance(item, inIndex);

	// if (inEnabled != (*item)->mEnabled)
	// {
	// 	gtk_widget_set_sensitive((*item)->mGMenuItem, inEnabled);
	// 	(*item)->mEnabled = inEnabled;
	// }

	// if ((*item)->mCanCheck)
	// 	(*item)->SetChecked(inChecked);
}

MMenuItem *MGtkMenuImpl::CreateNewItem(const string &inLabel, const std::string &inAction, GSList **ioRadioGroup)
{
	MMenuItem *item = new MMenuItem(mMenu, inLabel, inAction, mApp);

	// if (ioRadioGroup != nullptr)
	// 	item->CreateWidget(*ioRadioGroup);
	// else
	// {
	// 	item->CreateWidget();

	// 	if (inLabel != "-")
	// 		mRadioGroup = nullptr;
	// }

	item->mIndex = mItems.size();
	mItems.push_back(item);

	// if (mSection == nullptr)
	// {
	// 	mSection = g_menu_new();
	// 	g_menu_append_section(mGMenu, nullptr, G_MENU_MODEL(mSection));
	// }

	// g_menu_append_item(mSection, item->mGMenuItem);

	g_menu_append_item(mGMenu, item->mGMenuItem);

	return item;
}

void MGtkMenuImpl::AppendItem(const string &inLabel, const std::string &inAction)
{
	CreateNewItem(inLabel, inAction, nullptr);
}

void MGtkMenuImpl::AppendSubmenu(MMenu *inSubmenu)
{

	//	if (inSubmenu->IsRecentMenu())
	//		item->mRecentItemActivated.Connect(inSubmenu->GetGtkMenu(), "item-activated");

	MGtkMenuImpl *subImpl = dynamic_cast<MGtkMenuImpl *>(inSubmenu->impl());

	MMenuItem *item = new MMenuItem(mMenu, inSubmenu->GetLabel().c_str(), {}, mApp);
	item->mSubMenu = inSubmenu;
	g_menu_item_set_submenu(item->mGMenuItem, G_MENU_MODEL(subImpl->mGMenu));
	g_menu_append_item(mGMenu, item->mGMenuItem);

	std::cerr << "Aantal items in submenu: " << g_menu_model_get_n_items(G_MENU_MODEL(subImpl->mGMenu)) << "\n";

	// g_menu_append_submenu(subImpl->mGMenu, inSubmenu->GetLabel().c_str(), G_MENU_MODEL(subImpl->mGMenu));

	std::cerr << "Aantal items in dit menu: " << g_menu_model_get_n_items(G_MENU_MODEL(mGMenu)) << "\n";

	// gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->mGMenuItem), subImpl->mGtkMenu);

	// if (mGtkAccel)
	// 	subImpl->SetAcceleratorGroup(mGtkAccel);
}

void MGtkMenuImpl::AppendSeparator()
{
	CreateNewItem("-", 0, nullptr);
}

void MGtkMenuImpl::AppendCheckbox(const string &inLabel, const std::string &inAction)
{
}

void MGtkMenuImpl::AppendRadiobutton(const string &inLabel, const std::string &inAction)
{
}

uint32_t MGtkMenuImpl::CountItems() const
{
	return mItems.size();
}

void MGtkMenuImpl::RemoveItems(uint32_t inFirstIndex, uint32_t inCount)
{
	if (inFirstIndex < mItems.size())
	{
		MMenuItemList::iterator b = mItems.begin();
		advance(b, inFirstIndex);

		if (inFirstIndex + inCount > mItems.size())
			inCount = mItems.size() - inFirstIndex;

		MMenuItemList::iterator e = b;
		advance(e, inCount);

#warning FIXME
		// for (MMenuItemList::iterator mi = b; mi != e; ++mi)
		// 	gtk_container_remove(GTK_CONTAINER(mGtkMenu), (*mi)->mGMenuItem);
		// gtk_widget_destroy((*mi)->mGMenuItem);

		mItems.erase(b, e);
	}
}

string MGtkMenuImpl::GetItemLabel(uint32_t inIndex) const
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::const_iterator i = mItems.begin();
	advance(i, inIndex);

	return (*i)->mLabel;
}

void MGtkMenuImpl::SetItemCommand(uint32_t inIndex, const std::string &inAction)
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::iterator i = mItems.begin();
	advance(i, inIndex);

	(*i)->mCommand = inAction;
}

uint32_t MGtkMenuImpl::GetItemCommand(uint32_t inIndex) const
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::const_iterator i = mItems.begin();
	advance(i, inIndex);

	return 0; //(*i)->mCommand;
}

MMenu *MGtkMenuImpl::GetSubmenu(uint32_t inIndex) const
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::const_iterator i = mItems.begin();
	advance(i, inIndex);

	return (*i)->mSubMenu;
}

void MGtkMenuImpl::Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu)
{
}

void MGtkMenuImpl::AddToWindow(MWindowImpl *inWindowImpl)
{
	// MGtkWindowImpl *impl = dynamic_cast<MGtkWindowImpl *>(inWindowImpl);
	// impl->AddMenubarWidget(mGMenu);
}

void MGtkMenuImpl::MenuUpdated()
{
#warning FIXME
	// gtk_widget_show_all(mGtkMenu);
}

MMenuImpl *MMenuImpl::Create(MMenu *inMenu, bool inPopup)
{
	GApplication *app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	return new MGtkMenuImpl(inMenu, app);
}

// --------------------------------------------------------------------

class MGtkMenuBarImpl : public MGtkMenuImpl
{
  public:
	MGtkMenuBarImpl(MMenu *inMenu, GApplication *inApp)
		: MGtkMenuImpl(inMenu, g_menu_new(), inApp)
	{
		gtk_application_set_menubar(GTK_APPLICATION(inApp), G_MENU_MODEL(mGMenu));
	}

	void AddToWindow(MWindowImpl *inWindow) override
	{
		auto impl = static_cast<MGtkWindowImpl *>(inWindow);
		gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(impl->GetWidget()), true);
	}
};

MMenuImpl *MMenuImpl::CreateBar(MMenu *inMenu)
{
	GApplication *app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	return new MGtkMenuBarImpl(inMenu, app);
}


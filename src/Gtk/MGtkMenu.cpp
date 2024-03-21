//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "Gtk/MGtkApplicationImpl.hpp"
#include "Gtk/MGtkWidgetMixin.hpp"
#include "Gtk/MGtkWindowImpl.hpp"

#include "MAcceleratorTable.hpp"
#include "MApplication.hpp"
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

namespace
{

struct MCommandToString
{
	char mCommandString[10];

	MCommandToString(
		uint32_t inCommand)
	{
		strcpy(mCommandString, "MCmd_xxxx");

		mCommandString[5] = ((inCommand & 0xff000000) >> 24) & 0x000000ff;
		mCommandString[6] = ((inCommand & 0x00ff0000) >> 16) & 0x000000ff;
		mCommandString[7] = ((inCommand & 0x0000ff00) >> 8) & 0x000000ff;
		mCommandString[8] = ((inCommand & 0x000000ff) >> 0) & 0x000000ff;
	}

	operator const char *() const { return mCommandString; }
};

} // namespace

// --------------------------------------------------------------------
// MMenuItem

struct MMenuItem;
typedef list<MMenuItem *> MMenuItemList;

struct MMenuItem
{
  public:
	MMenuItem(MMenu *inMenu, const string &inLabel, uint32_t inCommand);

	void CreateWidget();                      // plain, simple item
	void CreateWidget(GSList *&ioRadioGroup); // radio menu item
	void CreateCheckWidget();
	void SetChecked(bool inChecked);
	void ItemCallback();
	void ItemToggled();

	MSlot<void()> mCallback;

	GMenuItem *mGMenuItem;
	string mLabel;
	uint32_t mCommand;
	uint32_t mIndex;
	MMenu *mMenu;
	MMenu *mSubMenu;
	bool mEnabled;
	bool mCanCheck;
	bool mChecked;
	bool mInhibitCallBack;
};

MMenuItem::MMenuItem(MMenu *inMenu, const string &inLabel, uint32_t inCommand)
	: mCallback(this, &MMenuItem::ItemCallback)
	, mGMenuItem(nullptr)
	, mLabel(inLabel)
	, mCommand(inCommand)
	, mIndex(0)
	, mMenu(inMenu)
	, mSubMenu(nullptr)
	, mEnabled(true)
	, mCanCheck(false)
	, mChecked(false)
	, mInhibitCallBack(false)
{
}

void MMenuItem::CreateWidget()
{
	mGMenuItem = g_menu_item_new(_(mLabel.c_str()), (std::string{ "app." } + (const char *)MCommandToString(mCommand)).c_str());
	// if (mCommand != 0)
	// 	mCallback.Connect(mGMenuItem, "activate");
}

void MMenuItem::CreateWidget(GSList *&ioRadioGroup)
{
	// #warning FIXME
	// 	// mGMenuItem = gtk_radio_menu_item_new_with_label(ioRadioGroup, _(mLabel.c_str()));

	// 	// ioRadioGroup = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(mGMenuItem));

	// 	// if (mCommand != 0)
	// 	// 	mCallback.Connect(mGMenuItem, "toggled");

	// 	// mCanCheck = true;
}

void MMenuItem::CreateCheckWidget()
{
#warning FIXME
	// mGMenuItem = gtk_check_menu_item_new_with_label(_(mLabel.c_str()));

	// if (mCommand != 0)
	// 	mCallback.Connect(mGMenuItem, "toggled");

	// //	mCanCheck = true;
}

void MMenuItem::ItemCallback()
{
#warning FIXME
	// try
	// {
	// 	if (mMenu != nullptr and
	// 		mMenu->GetTarget() != nullptr and
	// 		not mInhibitCallBack)
	// 	{
	// 		bool process = true;

	// 		if (mCanCheck)
	// 		{
	// 			mChecked = gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(mGMenuItem));
	// 			process = mChecked;
	// 		}

	// 		uint32_t modifiers = 0;
	// 		uint32_t gdkModifiers = 0;

	// 		auto keyMap = gdk_keymap_get_for_display(gdk_display_get_default());
	// 		if (GDK_IS_KEYMAP(keyMap))
	// 			gdkModifiers = gdk_keymap_get_modifier_state(keyMap);

	// 		if (gdkModifiers & GDK_SHIFT_MASK)
	// 			modifiers |= kShiftKey;
	// 		if (gdkModifiers & GDK_CONTROL_MASK)
	// 			modifiers |= kControlKey;
	// 		if (gdkModifiers & GDK_MOD1_MASK)
	// 			modifiers |= kOptionKey;

	// 		MHandler *target = mMenu->GetTarget();
	// 		MWindow *window = dynamic_cast<MWindow *>(target);
	// 		if (window != nullptr)
	// 		{
	// 			auto focus = window->FindFocus();
	// 			if (focus != nullptr)
	// 				target = focus;
	// 		}

	// 		if (process and target != nullptr and not target->ProcessCommand(mCommand, mMenu, mIndex, modifiers))
	// 			PRINT(("Unhandled command: %s", (const char *)MCommandToString(mCommand)));
	// 	}
	// }
	// catch (const exception &e)
	// {
	// 	DisplayError(e);
	// }
	// catch (...)
	// {
	// }
}

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

	virtual void SetTarget(MHandler *inHandler);
	virtual void SetItemState(uint32_t inItem, bool inEnabled, bool inChecked);
	virtual void AppendItem(const string &inLabel, uint32_t inCommand);
	virtual void AppendSubmenu(MMenu *inSubmenu);
	virtual void AppendSeparator();
	virtual void AppendCheckbox(const string &inLabel, uint32_t inCommand);
	virtual void AppendRadiobutton(const string &inLabel, uint32_t inCommand);
	virtual uint32_t CountItems() const;
	virtual void RemoveItems(uint32_t inFirstIndex, uint32_t inCount);
	virtual string GetItemLabel(uint32_t inIndex) const;
	virtual void SetItemCommand(uint32_t inIndex, uint32_t inCommand);
	virtual uint32_t GetItemCommand(uint32_t inIndex) const;
	virtual MMenu *GetSubmenu(uint32_t inIndex) const;
	virtual void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu);
	virtual void AddToWindow(MWindowImpl *inWindow);
	virtual void MenuUpdated();

	// void SetAcceleratorGroup(GtkAccelGroup *inAcceleratorGroup);

	virtual bool OnDestroy();
	virtual void OnSelectionDone();

	MSlot<bool()> mOnDestroy;
	MSlot<void()> mOnSelectionDone;

  protected:
	MMenuItem *CreateNewItem(const std::string &inLabel, uint32_t inCommand, GSList **ioRadioGroup);

	// for the menubar
	MGtkMenuImpl(MMenu *inMenu, GMenu *inGMenu, GApplication *inApp)
		: MGtkMenuImpl(inMenu, inApp)
	{
		mGMenu = inGMenu;
	}

	GMenu *mGMenu = nullptr;
	GApplication *mApp;
	std::string mLabel;
	MMenuItemList mItems;
	MHandler *mTarget;
	GSList *mRadioGroup;
	int32_t mPopupX, mPopupY;
};

MGtkMenuImpl::MGtkMenuImpl(MMenu *inMenu, GApplication *inApp)
	: MMenuImpl(inMenu)
	, mOnDestroy(this, &MGtkMenuImpl::OnDestroy)
	, mOnSelectionDone(this, &MGtkMenuImpl::OnSelectionDone)
	, mGMenu(g_menu_new())
	, mApp(inApp)
	, mTarget(nullptr)
	, mRadioGroup(nullptr)
{
}

void MGtkMenuImpl::SetTarget(MHandler *inHandler)
{
	mTarget = inHandler;

	for (auto mi : mItems)
	{
		if (mi->mSubMenu != nullptr)
			mi->mSubMenu->SetTarget(inHandler);
	}
}

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

MMenuItem *MGtkMenuImpl::CreateNewItem(const string &inLabel, uint32_t inCommand, GSList **ioRadioGroup)
{
	MMenuItem *item = new MMenuItem(mMenu, inLabel, inCommand);

	if (ioRadioGroup != nullptr)
		item->CreateWidget(*ioRadioGroup);
	else
	{
		item->CreateWidget();

		if (inLabel != "-")
			mRadioGroup = nullptr;
	}

	item->mIndex = mItems.size();
	mItems.push_back(item);
#warning FIXME
	// gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenu), item->mGMenuItem);

	return item;
}

void MGtkMenuImpl::AppendItem(const string &inLabel, uint32_t inCommand)
{
	CreateNewItem(inLabel, inCommand, nullptr);
}

void MGtkMenuImpl::AppendSubmenu(MMenu *inSubmenu)
{
	MMenuItem *item = CreateNewItem(inSubmenu->GetLabel(), 0, nullptr);
	item->mSubMenu = inSubmenu;

	//	if (inSubmenu->IsRecentMenu())
	//		item->mRecentItemActivated.Connect(inSubmenu->GetGtkMenu(), "item-activated");

	MGtkMenuImpl *subImpl = dynamic_cast<MGtkMenuImpl *>(inSubmenu->impl());

#warning FIXME
	g_menu_append_item(subImpl->mGMenu, item->mGMenuItem);

	// gtk_menu_item_set_submenu(GTK_MENU_ITEM(item->mGMenuItem), subImpl->mGtkMenu);

	// if (mGtkAccel)
	// 	subImpl->SetAcceleratorGroup(mGtkAccel);
}

void MGtkMenuImpl::AppendSeparator()
{
	// CreateNewItem("-", 0, nullptr);
}

void MGtkMenuImpl::AppendCheckbox(const string &inLabel, uint32_t inCommand)
{
}

void MGtkMenuImpl::AppendRadiobutton(const string &inLabel, uint32_t inCommand)
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

void MGtkMenuImpl::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::iterator i = mItems.begin();
	advance(i, inIndex);

	(*i)->mCommand = inCommand;
}

uint32_t MGtkMenuImpl::GetItemCommand(uint32_t inIndex) const
{
	if (inIndex >= mItems.size())
		THROW(("Item index out of range"));

	MMenuItemList::const_iterator i = mItems.begin();
	advance(i, inIndex);

	return (*i)->mCommand;
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

bool MGtkMenuImpl::OnDestroy()
{
	return false;
}

void MGtkMenuImpl::OnSelectionDone()
{
}

// void MGtkMenuImpl::SetAcceleratorGroup(GtkAccelGroup *inAcceleratorGroup)
// {
// 	MAcceleratorTable &at = MAcceleratorTable::Instance();

// 	gtk_menu_set_accel_group(GTK_MENU(mGtkMenu), inAcceleratorGroup);

// 	for (auto &item : mItems)
// 	{
// 		uint32_t key, mod;

// 		if (at.GetAcceleratorKeyForCommand(item->mCommand, key, mod))
// 		{
// 			int m = 0;

// 			if (mod & kShiftKey)
// 				m |= GDK_SHIFT_MASK;
// 			if (mod & kControlKey)
// 				m |= GDK_CONTROL_MASK;
// 			if (mod & kOptionKey)
// 				m |= GDK_MOD1_MASK;

// 			switch (key)
// 			{
// 				case kTabKeyCode: key = GDK_KEY_Tab; break;
// 				case kF3KeyCode: key = GDK_KEY_F3; break;
// 				default: break;
// 			}

// 			gtk_widget_add_accelerator(item->mGMenuItem, "activate", inAcceleratorGroup,
// 				key, GdkModifierType(m), GTK_ACCEL_VISIBLE);
// 		}

// 		if (item->mSubMenu != nullptr)
// 		{
// 			MGtkMenuImpl *impl = dynamic_cast<MGtkMenuImpl *>(item->mSubMenu->impl());
// 			if (impl != nullptr)
// 				impl->SetAcceleratorGroup(inAcceleratorGroup);
// 		}
// 	}
// }

MMenuImpl *MMenuImpl::Create(MMenu *inMenu, bool inPopup)
{
	GApplication *app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	return new MGtkMenuImpl(inMenu, app);
}

// --------------------------------------------------------------------

static void
quit_activated(GSimpleAction *action, GVariant *parameter, GApplication *application)
{
	g_application_quit(application);
}

class MGtkMenuBarImpl : public MGtkMenuImpl
{
  public:
	MGtkMenuBarImpl(MMenu *inMenu, GApplication *inApp)
		: MGtkMenuImpl(inMenu, g_menu_new(), inApp)
	{
		// GSimpleAction *act_quit = g_simple_action_new("quit", NULL);
		// g_action_map_add_action(G_ACTION_MAP(inApp), G_ACTION(act_quit));
		// g_signal_connect(act_quit, "activate", G_CALLBACK(quit_activated), inApp);

		// GMenu *menubar = g_menu_new();
		// GMenuItem *menu_item_menu = g_menu_item_new("Menu", NULL);
		// GMenu *menu = g_menu_new();
		// GMenuItem *menu_item_quit = g_menu_item_new("Quit", "app.quit");
		// g_menu_append_item(menu, menu_item_quit);
		// g_object_unref(menu_item_quit);
		// g_menu_item_set_submenu(menu_item_menu, G_MENU_MODEL(menu));
		// g_object_unref(menu);
		// g_menu_append_item(menubar, menu_item_menu);
		// g_object_unref(menu_item_menu);

		gtk_application_set_menubar(GTK_APPLICATION(inApp), G_MENU_MODEL(mGMenu));

		mGMenuBar = gtk_popover_menu_bar_new_from_model(G_MENU_MODEL(mGMenu));
		// mGtkAccel = gtk_accel_group_new();
		// mOnButtonPressEvent.Connect(mGMenu, "button-press-event");
	}

	virtual void AddToWindow(MWindowImpl *inWindow)
	{
		gtk_application_window_set_show_menubar(GTK_APPLICATION_WINDOW(static_cast<MGtkWindowImpl *>(inWindow)->operator GtkWidget *()), true);
	}

	// bool OnButtonPress(GdkEventButton *inEvent);
	// MSlot<bool(GdkEventButton *)> mOnButtonPressEvent;

	GtkWidget *mGMenuBar;
};

MMenuImpl *MMenuImpl::CreateBar(MMenu *inMenu)
{
	GApplication *app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(gApp->GetImpl())->GetGtkApp());
	return new MGtkMenuBarImpl(inMenu, app);
}

// bool MGtkMenuBarImpl::OnButtonPress(GdkEvent *inEvent)
// {
// 	mMenu->UpdateCommandStatus();

// 	//	gtk_widget_show_all(mGtkMenu);

// 	return false;
// }

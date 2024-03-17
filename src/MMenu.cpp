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

#include "MMenu.hpp"
#include "MAcceleratorTable.hpp"
#include "MApplication.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MMenuImpl.hpp"
#include "MPreferences.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include "mrsrc.hpp"

#include <zeep/xml/document.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>

#undef AppendMenu

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

MMenu::MMenu(
	const string &inLabel,
	bool inPopup)
	: mImpl(MMenuImpl::Create(this, inPopup))
	, mLabel(inLabel)
	, mTarget(nullptr)
{
}

MMenu::MMenu(MMenuImpl *inImpl)
	: mImpl(inImpl)
	, mTarget(nullptr)
{
}

MMenu::~MMenu()
{
	delete mImpl;
}

MMenu *MMenu::CreateFromResource(const char *inResourceName, bool inPopup)
{
	using namespace std::literals;

	MMenu *result = nullptr;

	mrsrc::istream rsrc("Menus/"s + inResourceName + ".xml");
	xml::document doc(rsrc);

	// build a menu from the resource XML
	xml::element *root = doc.find_first("/menu");

	if (root != nullptr)
		result = Create(root, inPopup);

	return result;
}

MMenu *MMenu::Create(xml::element *inXMLNode, bool inPopup)
{
	string label;

	if (inXMLNode->name() == "menu")
	{
		label = GetLocalisedStringForContext("menu", inXMLNode->get_attribute("label"));
		if (label.length() == 0)
			THROW(("Invalid menu specification, label is missing"));
	}

	string special = inXMLNode->get_attribute("special");

	MMenu *menu = new MMenu(label, inPopup);
	menu->mSpecial = special;

	for (auto item : *inXMLNode)
	{
		if (item.name() == "item")
		{
			label = GetLocalisedStringForContext("menu", item.get_attribute("label"));

			if (label == "-")
				menu->AppendSeparator();
			else
			{
				string cs = item.get_attribute("cmd").c_str();

				if (cs.length() != 4)
					THROW(("Invalid menu item specification, cmd is not correct"));

				uint32_t cmd = 0;
				for (int i = 0; i < 4; ++i)
					cmd |= cs[i] << ((3 - i) * 8);

				if (item.get_attribute("check") == "radio")
					menu->AppendRadioItem(label, cmd);
				else if (item.get_attribute("check") == "checkbox")
					menu->AppendCheckItem(label, cmd);
				else
					menu->AppendItem(label, cmd);
			}
		}
		else if (item.name() == "menu")
			menu->AppendMenu(Create(&item, false));
	}

	return menu;
}

void MMenu::AppendItem(
	const string &inLabel,
	uint32_t inCommand)
{
	mImpl->AppendItem(inLabel, inCommand);
}

void MMenu::AppendRadioItem(
	const string &inLabel,
	uint32_t inCommand)
{
	mImpl->AppendRadiobutton(inLabel, inCommand);
}

void MMenu::AppendCheckItem(
	const string &inLabel,
	uint32_t inCommand)
{
	mImpl->AppendCheckbox(inLabel, inCommand);
}

void MMenu::AppendSeparator()
{
	mImpl->AppendSeparator();
}

void MMenu::AppendMenu(
	MMenu *inMenu)
{
	mImpl->AppendSubmenu(inMenu);
}

uint32_t MMenu::CountItems()
{
	return mImpl->CountItems();
}

void MMenu::RemoveItems(
	uint32_t inFromIndex,
	uint32_t inCount)
{
	mImpl->RemoveItems(inFromIndex, inCount);
}

string MMenu::GetItemLabel(
	uint32_t inIndex) const
{
	return mImpl->GetItemLabel(inIndex);
}

void MMenu::SetItemCommand(uint32_t inIndex, uint32_t inCommand)
{
	mImpl->SetItemCommand(inIndex, inCommand);
}

uint32_t MMenu::GetItemCommand(
	uint32_t inIndex) const
{
	return mImpl->GetItemCommand(inIndex);
}

void MMenu::SetTarget(MHandler *inTarget)
{
	mTarget = inTarget;
	mImpl->SetTarget(inTarget);

	for (uint32_t i = 0; i < CountItems(); ++i)
	{
		MMenu *subMenu = mImpl->GetSubmenu(i);
		if (subMenu != nullptr)
			subMenu->SetTarget(inTarget);
	}
}

void MMenu::UpdateCommandStatus()
{
	if (not mSpecial.empty())
		gApp->UpdateSpecialMenu(mSpecial, this);

	for (uint32_t i = 0; i < CountItems(); ++i)
	{
		MMenu *subMenu = mImpl->GetSubmenu(i);
		if (subMenu != nullptr)
			subMenu->UpdateCommandStatus();
		else
		{
			bool enabled = false, checked = false;
			if (mTarget != nullptr)
			{
				MWindow *window = dynamic_cast<MWindow *>(mTarget);
				if (window != nullptr and window->FindFocus() != nullptr)
					window->FindFocus()->UpdateCommandStatus(GetItemCommand(i), this, i, enabled, checked);
				else
					mTarget->UpdateCommandStatus(GetItemCommand(i), this, i, enabled, checked);
			}
			mImpl->SetItemState(i, enabled, checked);
		}
	}

	mImpl->MenuUpdated();
}

// void MMenu::SetAcceleratorGroup(
//	GtkAccelGroup*		inAcceleratorGroup)
//{
//	MAcceleratorTable& at = MAcceleratorTable::Instance();
//
//	gtk_menu_set_accel_group(GTK_MENU(mGtkMenu), inAcceleratorGroup);
//
//	for (MMenuItemList::iterator mi = mItems.begin(); mi != mItems.end(); ++mi)
//	{
//		MMenuItem* item = *mi;
//
//		uint32_t key, mod;
//
//		if (at.GetAcceleratorKeyForCommand(item.mCommand, key, mod))
//		{
//			gtk_widget_add_accelerator(item.mGtkMenuItem, "activate", inAcceleratorGroup,
//				key, GdkModifierType(mod), GTK_ACCEL_VISIBLE);
//		}
//
//		if (item.mSubMenu != nullptr)
//			item.mSubMenu->SetAcceleratorGroup(inAcceleratorGroup);
//	}
// }

// void MMenu::MenuPosition(
//	GtkMenu*			inMenu,
//	gint*				inX,
//	gint*				inY,
//	gboolean*			inPushIn,
//	gpointer			inUserData)
//{
//	MMenu* self = reinterpret_cast<MMenu*>(g_object_get_data(G_OBJECT(inMenu), "MMenu"));
//
//	*inX = self->mPopupX;
//	*inY = self->mPopupY;
//	*inPushIn = true;
// }

void MMenu::Popup(
	MWindow *inHandler,
	int32_t inX,
	int32_t inY,
	bool inBottomMenu)
{
	mImpl->Popup(inHandler, inX, inY, inBottomMenu);
	// SetTarget(inHandler);
	//
	// mOnSelectionDone.Connect(mGtkMenu, "selection-done");
	//
	// mPopupX = inX;
	// mPopupY = inY;
	//
	// g_object_set_data(G_OBJECT(mGtkMenu), "MMenu", this);

	// gtk_widget_show_all(mGtkMenu);

	// int32_t button = 0;
	// uint32_t time = 0;
	// if (inEvent != nullptr)
	//{
	//	button = inEvent->button;
	//	time = inEvent->time;
	// }
	//
	// UpdateCommandStatus();

	// gtk_menu_popup(GTK_MENU(mGtkMenu), nullptr, nullptr,
	//	&MMenu::MenuPosition, nullptr, button, time);
}

// bool MMenu::OnDestroy()
//{
//	mGtkMenu = nullptr;
//
//	delete this;
//
//	return false;
// }
//
// void MMenu::OnSelectionDone()
//{
//	gtk_widget_destroy(mGtkMenu);
// }

// --------------------------------------------------------------------

MMenuBar::MMenuBar()
	: MMenu(MMenuImpl::CreateBar(this))
{
}

MMenuBar *MMenuBar::Create(zeep::xml::element *inXMLNode)
{
	if (inXMLNode->name() != "menubar")
		THROW(("Invalid menubar specification"));

	MMenuBar *result = new MMenuBar();

	for (auto item : *inXMLNode)
	{
		if (item.name() == "menu")
			result->AppendMenu(MMenu::Create(&item, false));
	}

	return result;
}

void MMenuBar::AddToWindow(MWindowImpl *inWindowImpl)
{
	mImpl->AddToWindow(inWindowImpl);
}

// void MMenubar::Initialize(
//	GtkWidget*		inMBarWidget,
//	const char*		inResourceName)
//{
//	mGtkMenubar = inMBarWidget;
//	mOnButtonPressEvent.Connect(mGtkMenubar, "button-press-event");
//
//	mrsrc::rsrc rsrc(string("Menus/") + inResourceName + ".xml");
//
//	if (not rsrc)
//		THROW(("Menu resource not found: %s", inResourceName));
//
//	io::stream<io::array_source> data(rsrc.data(), rsrc.size());
//	xml::document doc(data);
//
//	// build a menubar from the resource XML
//	for (xml::element* menu : doc.find("/menubar/menu"))
//	{
//		MMenu* obj = CreateMenu(menu);
//		AddMenu(obj);
//	}
//
//	gtk_widget_show_all(mGtkMenubar);
// }
//
// MMenu* MMenubar::CreateMenu(
//	xml::element*	inXMLNode)
//{
//	string label = inXMLNode->get_attribute("label");
//	if (label.length() == 0)
//		THROW(("Invalid menu specification, label is missing"));
//
//	string special = inXMLNode->get_attribute("special");
//
//	MMenu* menu;
//
//	if (special == "recent")
//		menu = new MMenu(label, gtk_recent_chooser_menu_new_for_manager(MRecentItems::Instance()));
//	else
//	{
//		menu = new MMenu(label);
//
//		for (xml::element* item : inXMLNode->children<xml::element>())
//		{
//			if (item.qname() == "item")
//			{
//				label = item.get_attribute("label");
//
//				if (label == "-")
//					menu->AppendSeparator();
//				else
//				{
//					string cs = item.get_attribute("cmd").c_str();
//
//					if (cs.length() != 4)
//						THROW(("Invalid menu item specification, cmd is not correct"));
//
//					uint32_t cmd = 0;
//					for (int i = 0; i < 4; ++i)
//						cmd |= cs[i] << ((3 - i) * 8);
//
//					if (item.get_attribute("check") == "radio")
//						menu->AppendRadioItem(label, cmd);
//					else if (item.get_attribute("check") == "checkbox")
//						menu->AppendCheckItem(label, cmd);
//					else
//						menu->AppendItem(label, cmd);
//				}
//			}
//			else if (item.qname() == "menu")
//				menu->AppendMenu(CreateMenu(item));
//		}
//	}
//
//	if (not special.empty() and special != "recent")
//		mSpecialMenus.push_back(make_pair(special, menu));
//
//	return menu;
// }
//
// void MMenubar::AddMenu(
//	MMenu*				inMenu)
//{
//	GtkWidget* menuItem = gtk_menu_item_new_with_label(_(inMenu->GetLabel().c_str()));
//	gtk_menu_item_set_submenu(GTK_MENU_ITEM(menuItem), inMenu->GetGtkMenu());
//	gtk_menu_shell_append(GTK_MENU_SHELL(mGtkMenubar), menuItem);
//
//	if (mTarget != nullptr)
//		inMenu->SetTarget(mTarget);
//
//	inMenu->SetAcceleratorGroup(mGtkAccel);
//	mMenus.push_back(inMenu);
// }
//
// bool MMenubar::OnButtonPress(
//	GdkEventButton*		inEvent)
//{
//	for (list<MMenu*>::iterator m = mMenus.begin(); m != mMenus.end(); ++m)
//		(*m)->UpdateCommandStatus();
//
//	for (MSpecialMenus::iterator m = mSpecialMenus.begin(); m != mSpecialMenus.end(); ++m)
//		eUpdateSpecialMenu(m->first, m->second);
//
//	gtk_widget_show_all(mGtkMenubar);
//
//	return false;
// }
//
// void MMenubar::SetTarget(
//	MHandler*			inTarget)
//{
//	mTarget = inTarget;
//
//	for (list<MMenu*>::iterator m = mMenus.begin(); m != mMenus.end(); ++m)
//		(*m)->SetTarget(inTarget);
// }
//
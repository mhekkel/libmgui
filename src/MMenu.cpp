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

// --------------------------------------------------------------------

MMenu::MMenu(const string &inLabel, bool inPopup)
	: mImpl(MMenuImpl::Create(this, inPopup))
	, mLabel(inLabel)
{
}

MMenu::MMenu(MMenuImpl *inImpl)
	: mImpl(inImpl)
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
				string action = item.get_attribute("cmd").c_str();

				if (item.get_attribute("check") == "radio")
					menu->AppendRadioItem(label, action);
				else if (item.get_attribute("check") == "checkbox")
					menu->AppendCheckItem(label, action);
				else
					menu->AppendItem(label, action);
			}
		}
		else if (item.name() == "menu")
			menu->AppendMenu(Create(&item, false));
	}

	return menu;
}

void MMenu::AppendItem(const string &inLabel, const std::string &inAction)
{
	mImpl->AppendItem(inLabel, inAction);
}

void MMenu::AppendRadioItem(const string &inLabel, const std::string &inAction)
{
	mImpl->AppendRadiobutton(inLabel, inAction);
}

void MMenu::AppendCheckItem(const string &inLabel, const std::string &inAction)
{
	mImpl->AppendCheckbox(inLabel, inAction);
}

void MMenu::AppendSeparator()
{
	mImpl->AppendSeparator();
}

void MMenu::AppendMenu(MMenu *inMenu)
{
	mImpl->AppendSubmenu(inMenu);
}

uint32_t MMenu::CountItems()
{
	return mImpl->CountItems();
}

void MMenu::RemoveItems(uint32_t inFromIndex, uint32_t inCount)
{
	mImpl->RemoveItems(inFromIndex, inCount);
}

string MMenu::GetItemLabel(uint32_t inIndex) const
{
	return mImpl->GetItemLabel(inIndex);
}

void MMenu::SetItemCommand(uint32_t inIndex, const std::string &inAction)
{
	mImpl->SetItemCommand(inIndex, inAction);
}

uint32_t MMenu::GetItemCommand(uint32_t inIndex) const
{
	return mImpl->GetItemCommand(inIndex);
}

void MMenu::Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu)
{
	mImpl->Popup(inHandler, inX, inY, inBottomMenu);
}

// --------------------------------------------------------------------

std::unique_ptr<MMenuBar> MMenuBar::sInstance;

MMenuBar::MMenuBar()
	: MMenu(MMenuImpl::CreateBar(this))
{
}

void MMenuBar::Init(const std::string &inMenuResourceName)
{
	mrsrc::istream rsrc("Menus/" + inMenuResourceName + ".xml");
	xml::document doc(rsrc);

	zeep::xml::element *node = &doc.front();

	if (node->name() != "menubar")
		THROW(("Invalid menubar specification"));

	sInstance.reset(new MMenuBar());

	for (auto item : *node)
	{
		if (item.name() == "menu")
			sInstance->AppendMenu(MMenu::Create(&item, false));
	}
}

void MMenuBar::AddToWindow(MWindowImpl *inWindowImpl)
{
	mImpl->AddToWindow(inWindowImpl);
}

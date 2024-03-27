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
#include "MApplication.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MPreferences.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include "mrsrc.hpp"

#include <zeep/xml/document.hpp>

#include <algorithm>
#include <cstring>
#include <iostream>

#undef AppendSubmenu

namespace xml = zeep::xml;

// --------------------------------------------------------------------

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
	std::string label;

	if (inXMLNode->name() == "menu")
	{
		label = GetLocalisedStringForContext("menu", inXMLNode->get_attribute("label"));
		if (label.length() == 0)
			THROW(("Invalid menu specification, label is missing"));
	}

	MMenu *menu = new MMenu(label, inPopup);
	menu->mID = inXMLNode->get_attribute("id");

	int section = 0;

	for (auto &item : *inXMLNode)
	{
		if (item.name() == "separator")
		{
			++section;
			continue;
		}

		std::string section_id = std::to_string(section);
		
		if (item.name() == "item")
		{
			label = GetLocalisedStringForContext("menu", item.get_attribute("label"));
			menu->AppendItem(label, section_id,
				item.get_attribute("cmd"), item.get_attribute("check") == "check");
		}
		if (item.name() == "radio-item")
		{
			std::vector<std::string> channels;
			for (auto &channel : item)
				channels.emplace_back(channel.get_attribute("label"));
			menu->AppendRadioItems(channels, section_id, item.get_attribute("action"));
		}
		else if (item.name() == "menu")
			menu->AppendSubmenu(Create(&item, false), section_id);
	}

	return menu;
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
			sInstance->AppendSubmenu(MMenu::Create(&item, false), "");
	}
}

void MMenuBar::AddToWindow(MWindowImpl *inWindowImpl)
{
	mImpl->AddToWindow(inWindowImpl);
}

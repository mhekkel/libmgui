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

#pragma once

#include "MCommands.hpp"
#include "MP2PEvents.hpp"

#include <zeep/xml/node.hpp>

#include <list>

class MHandler;
class MWindow;
class MWindowImpl;
class MFile;
class MMenuImpl;
class MMenuBar;

class MMenu
{
  public:
	MMenu(const std::string &inLabel, bool inPopup);
	virtual ~MMenu();

	static MMenu *CreateFromResource(const char *inResourceName, bool inPopup);

	void AppendItem(const std::string &inLabel, uint32_t inCommand);
	void AppendRadioItem(const std::string &inLabel, uint32_t inCommand);
	void AppendCheckItem(const std::string &inLabel, uint32_t inCommand);
	void AppendSeparator();
	virtual void AppendMenu(MMenu *inMenu);
	uint32_t CountItems();
	void RemoveItems(uint32_t inFromIndex, uint32_t inCount);

	std::string GetItemLabel(uint32_t inIndex) const;

	void SetItemCommand(uint32_t inIndex, uint32_t inCommand);
	uint32_t GetItemCommand(uint32_t inIndex) const;

	void SetTarget(MHandler *inHandler);

	void UpdateCommandStatus();

	std::string GetLabel() { return mLabel; }
	MHandler *GetTarget() { return mTarget; }

	void Popup(MWindow *inTarget, int32_t inX, int32_t inY, bool inBottomMenu);

	static MMenu *Create(zeep::xml::element *inXMLNode, bool inPopup);

	MMenuImpl *impl() const { return mImpl; }

  protected:
	MMenu(MMenuImpl *inImpl);

	MMenuImpl *mImpl;
	std::string mLabel;
	std::string mSpecial;
	MHandler *mTarget;
};

class MMenuBar : public MMenu
{
  public:
	MMenuBar();
	void AddToWindow(MWindowImpl *inWindowImpl);
	static MMenuBar *Create(zeep::xml::element *inXMLNode);
};
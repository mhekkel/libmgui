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

#include "MP2PEvents.hpp"

#include <mxml/node.hpp>

#include <list>
#include <memory>

class MWindow;
class MWindowImpl;
class MMenuImpl;
class MMenuBar;

class MMenu
{
  public:
	class MMenuImpl
	{
	  public:
		MMenuImpl(MMenu *inMenu)
			: mMenu(inMenu)
		{
		}

		virtual ~MMenuImpl() = default;

		virtual void AppendItem(uint32_t inSection, const std::string &inLabel, const std::string &inAction, bool inStateful) = 0;
		virtual void AppendRadioItems(uint32_t inSection, const std::vector<std::string> &inLabels, const std::string &inAction) = 0;
		virtual void AppendSubmenu(uint32_t inSection, MMenu *inMenu) = 0;

		virtual void ReplaceItemsInSection(uint32_t inSection, const std::string &inAction,
			const std::vector<std::tuple<std::string,int>> &inItems) = 0;

		virtual MMenu *FindMenuByID(const std::string &inMenuID) = 0;

		virtual void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu) = 0;
		virtual void AddToWindow(MWindowImpl *inWindow) {}

		static MMenuImpl *Create(MMenu *inMenu, bool inPopup);
		static MMenuImpl *CreateBar(MMenu *inMenu);

	  protected:
		MMenu *mMenu;
	};

	MMenu(const std::string &inID, const std::string &inLabel, bool inPopup)
		: mImpl(MMenuImpl::Create(this, inPopup))
		, mID(inID)
		, mLabel(inLabel)
	{
	}

	virtual ~MMenu() = default;

	static MMenu *CreateFromResource(const char *inResourceName, bool inPopup);
	static MMenu *Create(const mxml::element &inXMLNode, bool inPopup);

	void AppendItem(uint32_t inSection, const std::string &inLabel, const std::string &inAction, bool inStateful = false)
	{
		mImpl->AppendItem(inSection, inLabel, inAction, inStateful);
	}

	void AppendRadioItems(uint32_t inSection, const std::vector<std::string> &inLabels, const std::string &inAction)
	{
		mImpl->AppendRadioItems(inSection, inLabels, inAction);
	}

	void AppendSubmenu(uint32_t inSection, MMenu *inMenu)
	{
		mImpl->AppendSubmenu(inSection, inMenu);
	}

	void ReplaceItemsInSection(uint32_t inSection, const std::string &inAction,
		const std::vector<std::tuple<std::string,int>> &inItems)
	{
		mImpl->ReplaceItemsInSection(inSection, inAction, inItems);
	}

	virtual MMenu *FindMenuByID(const std::string &inMenuID)
	{
		if (mID == inMenuID)
			return this;
		else
			return mImpl->FindMenuByID(inMenuID);
	}

	void Popup(MWindow *inTarget, int32_t inX, int32_t inY, bool inBottomMenu);

	MMenuImpl *impl() const { return mImpl.get(); }

	std::string GetLabel() const
	{
		return mLabel;
	}

  protected:
	MMenu(MMenuImpl *inImpl)
		: mImpl(inImpl)
	{
	}

	std::unique_ptr<MMenuImpl> mImpl;
	std::string mID;
	std::string mLabel;
};

// --------------------------------------------------------------------
// Menu bars are global objects, there is now only one for each application

class MMenuBar : public MMenu
{
  public:
	static void Init(const std::string &inMenuResourceName);

	static MMenuBar &Instance()
	{
		return *sInstance;
	}

	void AddToWindow(MWindowImpl *inWindowImpl);

  private:
	MMenuBar();

	static std::unique_ptr<MMenuBar> sInstance;
};

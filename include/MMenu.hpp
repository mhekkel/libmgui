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

#include <zeep/xml/node.hpp>

#include <list>
#include <memory>

class MWindow;
class MWindowImpl;
class MFile;
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

		virtual void AppendItem(const std::string &inLabel, const std::string &inSection, const std::string &inAction, bool inStateful) = 0;
		virtual void AppendRadioItems(const std::vector<std::string> &inLabels, const std::string &inSection, const std::string &inAction) = 0;
		virtual void AppendSubmenu(MMenu *inMenu, const std::string &inSection) = 0;

		virtual void RemoveItemsFromSection(const std::string &inSection) = 0;

		virtual MMenu *GetSubmenu(uint32_t inIndex) const = 0;

		virtual void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu) = 0;
		virtual void AddToWindow(MWindowImpl *inWindow) = 0;

		static MMenuImpl *Create(MMenu *inMenu, bool inPopup);
		static MMenuImpl *CreateBar(MMenu *inMenu);

	  protected:
		MMenu *mMenu;
	};

	MMenu(const std::string &inLabel, bool inPopup)
		: mImpl(MMenuImpl::Create(this, inPopup))
		, mLabel(inLabel)
	{
	}

	virtual ~MMenu() = default;

	static MMenu *CreateFromResource(const char *inResourceName, bool inPopup);
	static MMenu *Create(zeep::xml::element *inXMLNode, bool inPopup);

	void AppendItem(const std::string &inLabel, const std::string &inSection, const std::string &inAction, bool inStateful = false)
	{
		mImpl->AppendItem(inLabel, inSection, inAction, inStateful);
	}

	void AppendRadioItems(const std::vector<std::string> &inLabels, const std::string &inSection, const std::string &inAction)
	{
		mImpl->AppendRadioItems(inLabels, inSection, inAction);
	}

	void AppendSubmenu(MMenu *inMenu, const std::string &inSection)
	{
		mImpl->AppendSubmenu(inMenu, inSection);
	}

	void RemoveItemsFromSection(const std::string &inSection)
	{
		mImpl->RemoveItemsFromSection(inSection);
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

	MMenu *FindMenuByID(const std::string &inMenuID);

	static MMenuBar &instance()
	{
		return *sInstance;
	}

	void AddToWindow(MWindowImpl *inWindowImpl);

  private:
	MMenuBar();

	static std::unique_ptr<MMenuBar> sInstance;
};

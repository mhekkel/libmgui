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

#include "MMenu.hpp"

class MMenuImpl
{
  public:
	MMenuImpl(MMenu *inMenu)
		: mMenu(inMenu)
	{
	}

	virtual ~MMenuImpl() = default;

	virtual void SetItemState(uint32_t inItem, bool inEnabled, bool inChecked) = 0;

	virtual void AppendItem(const std::string &inLabel, const std::string &inAction) = 0;
	virtual void AppendSubmenu(MMenu *inSubmenu) = 0;
	virtual void AppendSeparator() = 0;
	virtual void AppendCheckbox(const std::string &inLabel, const std::string &inAction) = 0;
	virtual void AppendRadiobutton(const std::string &inLabel, const std::string &inAction) = 0;
	virtual uint32_t CountItems() const = 0;
	virtual void RemoveItems(uint32_t inFirstIndex, uint32_t inCount) = 0;

	virtual std::string GetItemLabel(uint32_t inIndex) const = 0;
	virtual void SetItemCommand(uint32_t inIndex, const std::string &inAction) = 0;
	virtual uint32_t GetItemCommand(uint32_t inIndex) const = 0;
	virtual MMenu *GetSubmenu(uint32_t inIndex) const = 0;

	virtual void Popup(MWindow *inHandler, int32_t inX, int32_t inY, bool inBottomMenu) = 0;
	virtual void AddToWindow(MWindowImpl *inWindow) = 0;

	virtual void MenuUpdated() = 0;

	static MMenuImpl *Create(MMenu *inMenu, bool inPopup);
	static MMenuImpl *CreateBar(MMenu *inMenu);

  protected:
	MMenu *mMenu;
};

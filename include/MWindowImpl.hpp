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

#include "MWindow.hpp"

#undef CreateDialog

class MMenuBar;

class MWindowImpl
{
  public:
	static MWindowImpl *Create(const std::string &inTitle, MRect inBounds,
		MWindowFlags inFlags, MWindow *inWindow);

	static MWindowImpl *CreateDialog(const std::string &inResource, MWindow *inWindow);

	virtual ~MWindowImpl() {}

	MWindowFlags GetFlags() const { return mFlags; }

	virtual void Finish() {}

	virtual void SetTitle(std::string inTitle) = 0;
	// virtual std::string	GetTitle() const = 0;

	virtual void Show() = 0;
	virtual void Hide() = 0;

	virtual bool ShowModal() { return false; }
	virtual void SetTransientFor(MWindow *inWindow) {}

	virtual bool Visible() const = 0;

	virtual void Select() = 0;
	virtual void Close() = 0;

	virtual void ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta) = 0;

	virtual void SetWindowPosition(MRect inBounds, bool inTransition) = 0;
	virtual void GetWindowPosition(MRect &outBounds) const = 0;

	virtual void UpdateNow() = 0;

	virtual void SetCursor(MCursor inCursor) = 0;
	virtual void ObscureCursor() = 0;

	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const = 0;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const = 0;

  protected:
	MWindowImpl(MWindowFlags inFlags, MWindow *inWindow)
		: mWindow(inWindow)
		, mFlags(inFlags)
	{
	}

	MMenuBar *CreateMenu(const std::string &inMenu);

	MWindow *mWindow;
	MWindowFlags mFlags;
};

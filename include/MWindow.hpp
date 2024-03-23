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

#include <list>

#include "MColor.hpp"
#include "MView.hpp"

#include "MP2PEvents.hpp"

// --------------------------------------------------------------------

class MMenuBar;

// --------------------------------------------------------------------

enum MWindowFlags
{
	kMFixedSize = (1 << 0),
	kMAcceptFileDrops = (1 << 1),
	kMPostionDefault = (1 << 2),
	kMDialogBackground = (1 << 3),
	kMNoEraseOnUpdate = (1 << 4),
	kMNoSizeBox = (1 << 5),
	kMAcceptDragAndDrop = (1 << 6),
	kMCustomNonClient = (1 << 7),
	kMShowMenubar = (1 << 8)
};

// clean way to work with bitfields
inline MWindowFlags operator|(MWindowFlags f1, MWindowFlags f2)
{
	return MWindowFlags(uint32_t(f1) | uint32_t(f2));
}

// --------------------------------------------------------------------

class MWindowImpl
{
  public:
	static MWindowImpl *Create(const std::string &inTitle, MRect inBounds,
		MWindowFlags inFlags, MWindow *inWindow);

	static MWindowImpl *CreateDialog(const std::string &inResource, MWindow *inWindow);

	virtual ~MWindowImpl() = default;

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

// --------------------------------------------------------------------

class MWindow : public MView
{
  public:
	MWindow(const std::string &inTitle,
		const MRect &inBounds, MWindowFlags inFlags);

	virtual ~MWindow();

	virtual MWindow *GetWindow() const;
	MWindowFlags GetFlags() const;

	// virtual void Activated();

	virtual void Show();
	virtual void Select();
	virtual void UpdateNow();

	virtual bool AllowClose(bool inQuit);
	virtual void Close();

	// void Beep();

	// --------------------------------------------------------------------
	// Window List

	static MWindow *GetFirstWindow();
	MWindow *GetNextWindow() const;
	static void RemoveWindowFromWindowList(MWindow *window);
	static bool WindowExists(MWindow *window);

	// --------------------------------------------------------------------

	virtual void SetTitle(const std::string &inTitle);
	virtual std::string GetTitle() const;

	void SetModifiedMarkInTitle(bool inModified);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
	virtual void ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta);

	void GetWindowPosition(MRect &outPosition);
	void SetWindowPosition(const MRect &outPosition, bool inTransition = false);

	MWindowImpl *GetImpl() const { return mImpl; }

	// --------------------------------------------------------------------
	// coordinate manipulations
	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const;

	// --------------------------------------------------------------------
	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();

	static void GetMainScreenBounds(MRect &outRect);

  protected:
	MWindow(MWindowImpl *inImpl);

	void SetImpl(MWindowImpl *inImpl);

  private:
	virtual void ShowSelf();
	virtual void HideSelf();

	MWindowImpl *mImpl;
	std::string mTitle;
	bool mModified;

	static std::list<MWindow *> sWindowList;
};

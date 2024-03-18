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
#include "MHandler.hpp"
#include "MView.hpp"

#include "MP2PEvents.hpp"

#undef GetNextWindow

class MWindowImpl;

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
};

class MWindow : public MView, public MHandler
{
  public:
	MWindow(const std::string &inTitle,
		const MRect &inBounds, MWindowFlags inFlags,
		const std::string &inMenu);

	virtual ~MWindow();

	virtual MWindow *GetWindow() const;
	MWindowFlags GetFlags() const;

	virtual void Mapped();
	virtual void Unmapped();

	virtual void Show();
	virtual void Select();
	virtual void Activate();
	virtual void UpdateNow();

	virtual bool AllowClose(bool inQuit);
	virtual void Close();

	void Beep();

	static MWindow *GetFirstWindow();
	MWindow *GetNextWindow() const;
	static void RemoveWindowFromWindowList(MWindow *window);
	static bool WindowExists(MWindow *window);

	virtual void SetTitle(const std::string &inTitle);
	virtual std::string GetTitle() const;

	void SetModifiedMarkInTitle(bool inModified);

	// 0.0 is fully transparent, 1.0 is fully opaque
	void SetTransparency(float inAlpha);

	// Latent focus is the control that should receive focus
	// when the window becomes active/receives focus
	virtual void SetLatentFocus(MHandler *inHandler);

	virtual void BeFocus();

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);

	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);

	virtual void ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta);

	void GetWindowPosition(MRect &outPosition);

	void SetWindowPosition(const MRect &outPosition, bool inTransition = false);

	void GetMaxPosition(MRect &outRect) const;

	MWindowImpl *GetImpl() const { return mImpl; }

	// coordinate manipulations
	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const;

	virtual void GetMouse(int32_t &outX, int32_t &outY, uint32_t &outModifiers) const;
	virtual uint32_t GetModifiers() const;

	virtual void Invalidate(MRect inRect);
	virtual void ScrollRect(MRect inRect, int32_t inX, int32_t inY);

	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();

	static void GetMainScreenBounds(MRect &outRect);

  protected:
	MWindow(MWindowImpl *inImpl);

	void SetImpl(MWindowImpl *inImpl);

	virtual void ActivateSelf();

  private:
	void TransitionTo(MRect inPosition);

	virtual void ShowSelf();
	virtual void HideSelf();

	MWindowImpl *mImpl;
	std::string mTitle;
	bool mModified;
	MHandler *mLatentFocus;

	static std::list<MWindow *>
		sWindowList;
};

// clean way to work with bitfields
inline MWindowFlags operator|(MWindowFlags f1, MWindowFlags f2)
{
	return MWindowFlags(uint32_t(f1) | uint32_t(f2));
}

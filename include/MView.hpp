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

/*	$Id$
    Copyright Maarten L. Hekkelman
    Created 28-09-07 11:15:18
*/

#pragma once

#include "MP2PEvents.hpp"
#include "MTypes.hpp"

#include <list>
#include <vector>

class MWindow;
class MDevice;
class MView;

typedef std::list<MView *> MViewList;

enum MCursor
{
	eNormalCursor,
	eIBeamCursor,
	eRightCursor,

	eBlankCursor,

	eCursorCount
};

/**
 * MView's have bounds and frames. The frame is the outer rectangle
 * in parent coordinates that encloses all of the MView, including
 * its margins. The bounds are in coordinates relative to the top/
 * left of the frame. When setting the frame, the bounds are
 * calculated based on the frame's dimensions and the margins.
 *
 * MViewScroller scrolls a MView by making the bounds's dimension
 * the same as the visible area inside the scroller. The bounds
 * are then moved inside the frame.
 */

class MView
{
  public:
	MView(const std::string &inID, MRect inBounds);
	virtual ~MView();

	std::string GetID() const { return mID; }

	virtual MView *GetParent() const;
	virtual const MViewList &GetChildren() const { return mChildren; }

	virtual void SetParent(MView *inParent);
	virtual void AddChild(MView *inChild);
	virtual void RemoveChild(MView *inChild);
	virtual void AddedToWindow();
	virtual MWindow *GetWindow() const;

	virtual MRect GetBounds() const;

	virtual MRect GetFrame() const;
	virtual void SetFrame(const MRect &inFrame);

	virtual void MoveFrame(int32_t inXDelta, int32_t inYDelta);
	virtual void ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta);
	virtual void GetBindings(bool &outFollowLeft, bool &outFollowTop, bool &outFollowRight, bool &outFollowBottom) const;
	virtual void SetBindings(bool inFollowLeft, bool inFollowTop, bool inFollowRight, bool inFollowBottom);
	bool WidthResizable() const { return mBindLeft and mBindRight; }
	bool HeightResizable() const { return mBindTop and mBindBottom; }

	virtual void GetMargins(int32_t &outLeftMargin, int32_t &outTopMargin, int32_t &outRightMargin, int32_t &outBottomMargin) const;
	virtual void SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin);

	// used in automatic layout
	virtual void RecalculateLayout();
	virtual void ChildResized();

	virtual void MiddleMouseButtonClick(int32_t inX, int32_t inY);
	virtual void SecondaryMouseButtonClick(int32_t inX, int32_t inY);

	virtual void Activate();
	virtual void Deactivate();
	bool IsActive() const;

	virtual void Enable();
	virtual void Disable();
	bool IsEnabled() const;

	virtual void Show();
	virtual void Hide();
	bool IsVisible() const;

	virtual void Invalidate();

	virtual void UpdateNow();
	virtual void AdjustCursor(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual void SetCursor(MCursor inCursor);
	virtual void ObscureCursor();

	MView *FindSubView(int32_t inX, int32_t inY) const;
	virtual MView *FindSubViewByID(const std::string &inID) const;

	virtual void ConvertToParent(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromParent(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertToWindow(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromWindow(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertToScreen(int32_t &ioX, int32_t &ioY) const;
	virtual void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const;

  protected:
	void SuperActivate();
	virtual void ActivateSelf();
	void SuperDeactivate();
	virtual void DeactivateSelf();

	void SuperEnable();
	virtual void EnableSelf();
	void SuperDisable();
	virtual void DisableSelf();

	void SuperShow();
	virtual void ShowSelf();
	void SuperHide();
	virtual void HideSelf();

	std::string mID;
	MRect mBounds;
	MRect mFrame;
	int32_t mLeftMargin, mTopMargin, mRightMargin, mBottomMargin;
	bool mBindLeft, mBindTop, mBindRight, mBindBottom;
	MView *mParent;
	MViewList mChildren;
	MTriState mActive;
	MTriState mVisible;
	MTriState mEnabled;
};

// --------------------------------------------------------------------

class MPager : public MView
{
  public:
	MPager(const std::string &inID, MRect inBounds);

	void AddPage(MView *inPage);
	void SelectPage(uint32_t inPage);
	virtual void RecalculateLayout();
};

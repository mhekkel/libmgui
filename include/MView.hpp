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

#include <filesystem>
#include <list>
#include <vector>

// --------------------------------------------------------------------

class MWindow;
class MDevice;
class MView;

// --------------------------------------------------------------------

typedef std::list<MView *> MViewList;

struct MMargins
{
	uint32_t left, top, right, bottom;
};

struct MViewLayout
{
	MViewLayout()
		: mHExpand(false)
		, mVExpand(false)
		, mMargin{ 0, 0, 0, 0 }
	{
	}

	MViewLayout(bool inExpand, uint32_t inMargin)
		: mHExpand(inExpand)
		, mVExpand(inExpand)
		, mMargin{ inMargin, inMargin, inMargin, inMargin }
	{
	}

	MViewLayout(bool inHExpand, bool inVExpand,
		uint32_t inMarginLeft, uint32_t inMarginTop, uint32_t inMarginRight, uint32_t inMarginBottom)
		: mHExpand(inHExpand)
		, mVExpand(inVExpand)
		, mMargin{ inMarginLeft, inMarginTop, inMarginRight, inMarginBottom }
	{
	}

	bool mHExpand, mVExpand;
	MMargins mMargin;
};

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

	virtual void RequestSize(int32_t inWidth, int32_t inHeight) {}

	virtual void SetLayout(MViewLayout inLayout);
	virtual MViewLayout GetLayout() const;

	bool WidthResizable() const { return mLayout.mHExpand; }
	bool HeightResizable() const { return mLayout.mVExpand; }

	// used in automatic layout
	virtual void RecalculateLayout();
	virtual void ChildResized();

	virtual void MiddleMouseButtonClick(int32_t inX, int32_t inY);
	virtual void SecondaryMouseButtonClick(int32_t inX, int32_t inY);

	virtual bool KeyPressed(uint32_t inKeyCode, char32_t inUnicode, uint32_t inModifiers, bool inAutoRepeat) { return false;}
	virtual void KeyReleased(uint32_t inKeyValue, uint32_t inModifiers) { }
	virtual void Modifiers(uint32_t inModifiers) { }
	virtual void EnterText(const std::string &inText) { }

	// Drag & Drop support
	/// \brief Return whether we will accept this drop, \a inMimeType is empty for file drops
	virtual bool DragAcceptsMimeType(const std::string &inMimeType) { return false; }
	virtual bool DragAcceptsFile() { return false; }
	virtual void DragEnter(int32_t inX, int32_t inY) { }
	virtual void DragMotion(int32_t inX, int32_t inY) { }
	virtual void DragLeave() { }
	virtual bool DragAcceptData(int32_t inX, int32_t inY, const std::string &inData) { return false; }
	virtual bool DragAcceptFile(int32_t inX, int32_t inY, const std::filesystem::path &inFile) { return false; }

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
	MViewLayout mLayout{};
	MView *mParent;
	MViewList mChildren;
	MTriState mActive;
	MTriState mVisible;
	MTriState mEnabled;
};

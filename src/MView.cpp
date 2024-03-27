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
    Copyright Drs M.L. Hekkelman
    Created 28-09-07 11:18:30
*/

#include "MControls.hpp"
#include "MDevice.hpp"
#include "MError.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include <cassert>
#include <iostream>

MView::MView(const std::string &inID, MRect inBounds)
	: mID(inID)
	, mBounds(0, 0, inBounds.width, inBounds.height)
	, mFrame(inBounds)
	, mLeftMargin(0)
	, mTopMargin(0)
	, mRightMargin(0)
	, mBottomMargin(0)
	, mBindLeft(true)
	, mBindTop(true)
	, mBindRight(false)
	, mBindBottom(false)
	, mParent(nullptr)
	, mActive(eTriStateLatent)
	, mVisible(eTriStateLatent)
	, mEnabled(eTriStateOn)
{
}

MView::~MView()
{
	while (mChildren.size() > 0)
	{
		MView *v = mChildren.back();
		v->mParent = nullptr;
		mChildren.pop_back();
		delete v;
	}

	if (mParent != nullptr)
		mParent->RemoveChild(this);
}

MView *MView::GetParent() const
{
	return mParent;
}

void MView::SetParent(MView *inParent)
{
	mParent = inParent;
}

void MView::AddChild(MView *inView)
{
	MRect frame(mFrame);

	mChildren.push_back(inView);
	inView->mParent = this;

	if (mEnabled == eTriStateOn)
		inView->SuperEnable();
	else
		inView->SuperDisable();

	if (mVisible == eTriStateOn)
		inView->SuperShow();
	else
		inView->SuperHide();

	if (GetWindow() != nullptr)
		inView->AddedToWindow();

	RecalculateLayout();

	if (frame != mFrame and mParent != nullptr)
		mParent->ChildResized();
}

void MView::AddedToWindow()
{
	if (mVisible == eTriStateOn and mParent->mVisible == eTriStateOff)
		mVisible = eTriStateLatent;

	for (MView *child : mChildren)
		child->AddedToWindow();
}

void MView::RemoveChild(MView *inView)
{
	MViewList::iterator i = std::find(mChildren.begin(), mChildren.end(), inView);
	if (i != mChildren.end())
	{
		MView *child = *i;
		mChildren.erase(i);
		child->mParent = nullptr;
	}
}

MWindow *MView::GetWindow() const
{
	MWindow *result = nullptr;
	if (mParent != nullptr)
		result = mParent->GetWindow();
	return result;
}

MRect MView::GetBounds() const
{
	return mBounds;
}

MRect MView::GetFrame() const
{
	return mFrame;
}

void MView::SetFrame(const MRect &inFrame)
{
	if (inFrame != mFrame)
	{
		mFrame = inFrame;

		mBounds.x = mLeftMargin;
		mBounds.y = mTopMargin;
		mBounds.width = mFrame.width - mLeftMargin - mRightMargin;
		mBounds.height = mFrame.height - mTopMargin - mBottomMargin;
	}
}

void MView::GetBindings(bool &outFollowLeft, bool &outFollowTop, bool &outFollowRight, bool &outFollowBottom) const
{
	outFollowLeft = mBindLeft;
	outFollowTop = mBindTop;
	outFollowRight = mBindRight;
	outFollowBottom = mBindBottom;
}

void MView::SetBindings(bool inFollowLeft, bool inFollowTop, bool inFollowRight, bool inFollowBottom)
{
	mBindLeft = inFollowLeft;
	mBindTop = inFollowTop;
	mBindRight = inFollowRight;
	mBindBottom = inFollowBottom;
}

void MView::MoveFrame(int32_t inXDelta, int32_t inYDelta)
{
	mFrame.x += inXDelta;
	mFrame.y += inYDelta;

	for (MView *child : mChildren)
		child->MoveFrame(0, 0);
}

void MView::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	mFrame.width += inWidthDelta;
	mFrame.height += inHeightDelta;

	mBounds.width += inWidthDelta;
	mBounds.height += inHeightDelta;

	for (MView *child : mChildren)
	{
		if (child->mVisible == eTriStateOff)
			continue;

		int32_t dx = 0, dy = 0, dw = 0, dh = 0;

		if (child->mBindRight)
		{
			if (child->mBindLeft)
				dw = inWidthDelta;
			else
				dx = inWidthDelta;
		}

		if (child->mBindBottom)
		{
			if (child->mBindTop)
				dh = inHeightDelta;
			else
				dy = inHeightDelta;
		}

		if (dx or dy)
			child->MoveFrame(dx, dy);
		child->ResizeFrame(dw, dh);
	}
}

void MView::GetMargins(int32_t &outLeftMargin, int32_t &outTopMargin, int32_t &outRightMargin, int32_t &outBottomMargin) const
{
	outLeftMargin = mLeftMargin;
	outTopMargin = mTopMargin;
	outRightMargin = mRightMargin;
	outBottomMargin = mBottomMargin;
}

void MView::SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin)
{
	int32_t dx = inLeftMargin - mLeftMargin;
	int32_t dy = inTopMargin - mTopMargin;

	mLeftMargin = inLeftMargin;
	mTopMargin = inTopMargin;
	mRightMargin = inRightMargin;
	mBottomMargin = inBottomMargin;

	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;

	mFrame.width = mBounds.width + mLeftMargin + mRightMargin;
	mFrame.height = mBounds.height + mTopMargin + mBottomMargin;

	for (MView *child : mChildren)
		child->MoveFrame(dx, dy);
}

void MView::RecalculateLayout()
{
	MRect b = mBounds;

	for (MView *child : mChildren)
	{
		if (child->mVisible == eTriStateOff)
			continue;

		b |= child->GetFrame();
	}

	mFrame.width = b.width + mLeftMargin + mRightMargin;
	mFrame.height = b.height + mTopMargin + mBottomMargin;
}

void MView::ChildResized()
{
	MRect frame(mFrame);
	RecalculateLayout();
	if (frame != mFrame and mParent != nullptr)
		mParent->ChildResized();
}

void MView::Invalidate()
{
}

void MView::UpdateNow()
{
	if (mParent != nullptr)
		mParent->UpdateNow();
}

void MView::SetCursor(MCursor inCursor)
{
	if (mParent != nullptr)
		mParent->SetCursor(inCursor);
}

void MView::AdjustCursor(int32_t inX, int32_t inY, uint32_t inModifiers)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->AdjustCursor(inX, inY, inModifiers);
	}
	else
		SetCursor(eNormalCursor);
}

void MView::ObscureCursor()
{
	if (mParent != nullptr)
		mParent->ObscureCursor();
}

void MView::MiddleMouseButtonClick(int32_t inX, int32_t inY)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->MiddleMouseButtonClick(inX, inY);
	}
}

void MView::SecondaryMouseButtonClick(int32_t inX, int32_t inY)
{
	if (mParent != nullptr)
	{
		ConvertToParent(inX, inY);
		mParent->SecondaryMouseButtonClick(inX, inY);
	}
}

void MView::Show()
{
	if (mVisible != eTriStateOn)
	{
		if (mParent and mParent->mVisible != eTriStateOff)
		{
			mVisible = eTriStateOn;
			Invalidate();
			ShowSelf();
		}
		else
			mVisible = eTriStateLatent;
	}

	if (mVisible == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperShow();

		if (mParent != nullptr)
			mParent->ChildResized();
	}
}

void MView::SuperShow()
{
	if (mVisible == eTriStateLatent)
	{
		mVisible = eTriStateOn;
		ShowSelf();
	}

	if (mVisible == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperShow();
	}
}

void MView::ShowSelf()
{
}

void MView::Hide()
{
	if (mVisible == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperHide();
	}

	if (mVisible != eTriStateOff)
	{
		Invalidate();

		bool wasVisible = (mVisible != eTriStateOff);
		mVisible = eTriStateOff;
		if (wasVisible)
		{
			HideSelf();

			if (mParent != nullptr)
				mParent->ChildResized();
		}
	}
}

void MView::SuperHide()
{
	if (mVisible == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperHide();

		mVisible = eTriStateLatent;
		HideSelf();
	}
}

void MView::HideSelf()
{
}

bool MView::IsVisible() const
{
	return mVisible == eTriStateOn;
}

void MView::Enable()
{
	if (mEnabled == eTriStateOff)
	{
		if (mParent != nullptr and mParent->mEnabled == eTriStateOn)
			mEnabled = eTriStateOn;
		else
			mEnabled = eTriStateLatent;
	}

	if (mEnabled == eTriStateOn)
	{
		EnableSelf();
		for (MView *child : mChildren)
			child->SuperEnable();
	}
}

void MView::SuperEnable()
{
	if (mEnabled == eTriStateLatent)
	{
		mEnabled = eTriStateOn;
		EnableSelf();
	}

	if (mEnabled == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperEnable();
	}
}

void MView::EnableSelf()
{
}

void MView::Disable()
{
	if (mEnabled == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperDisable();
	}

	// bool wasEnabled = (mEnabled != eTriStateOff);
	mEnabled = eTriStateOff;
	// if (wasEnabled)
	DisableSelf();
}

void MView::SuperDisable()
{
	if (mEnabled == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperDisable();

		mEnabled = eTriStateLatent;
		DisableSelf();
	}
}

void MView::DisableSelf()
{
}

bool MView::IsEnabled() const
{
	return (mEnabled == eTriStateOn) and IsVisible();
}

void MView::Activate()
{
	if (mActive == eTriStateOff)
	{
		if (mParent != nullptr and mParent->mActive == eTriStateOn)
		{
			mActive = eTriStateOn;
			ActivateSelf();
		}
		else
			mActive = eTriStateLatent;
	}

	if (mActive == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperActivate();
	}
}

void MView::SuperActivate()
{
	if (mActive == eTriStateLatent)
	{
		mActive = eTriStateOn;
		ActivateSelf();
	}

	if (mActive == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperActivate();
	}
}

void MView::ActivateSelf()
{
}

void MView::Deactivate()
{
	if (mActive == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperDeactivate();
	}

	bool wasActive = (mActive == eTriStateOn);
	mActive = eTriStateOff;
	if (wasActive)
		DeactivateSelf();
}

void MView::SuperDeactivate()
{
	if (mActive == eTriStateOn)
	{
		for (MView *child : mChildren)
			child->SuperDeactivate();

		mActive = eTriStateLatent;
		DeactivateSelf();
	}
}

void MView::DeactivateSelf()
{
}

bool MView::IsActive() const
{
	return (mActive == eTriStateOn) and IsVisible();
}

MView *MView::FindSubView(int32_t inX, int32_t inY) const
{
	const MView *result = this;

	ConvertFromParent(inX, inY);

	for (MView *view : mChildren)
	{
		if (view->IsVisible() and view->mFrame.ContainsPoint(inX, inY))
		{
			result = view->FindSubView(inX, inY);
			break;
		}
	}

	return const_cast<MView *>(result);
}

MView *MView::FindSubViewByID(const std::string &inID) const
{
	const MView *result = nullptr;

	if (mID == inID)
		result = this;
	else
	{
		for (MView *view : mChildren)
		{
			result = view->FindSubViewByID(inID);
			if (result != nullptr)
				break;
		}
	}

	return const_cast<MView *>(result);
}

void MView::ConvertToParent(int32_t &ioX, int32_t &ioY) const
{
	ioX += mFrame.x - mBounds.x + mLeftMargin;
	ioY += mFrame.y - mBounds.y + mTopMargin;
}

void MView::ConvertFromParent(int32_t &ioX, int32_t &ioY) const
{
	ioX -= mFrame.x - mBounds.x + mLeftMargin;
	ioY -= mFrame.y - mBounds.y + mTopMargin;
}

void MView::ConvertToWindow(int32_t &ioX, int32_t &ioY) const
{
	ConvertToParent(ioX, ioY);
	if (mParent != nullptr)
		mParent->ConvertToWindow(ioX, ioY);
}

void MView::ConvertFromWindow(int32_t &ioX, int32_t &ioY) const
{
	if (mParent != nullptr)
		mParent->ConvertFromWindow(ioX, ioY);
	ConvertFromParent(ioX, ioY);
}

void MView::ConvertToScreen(int32_t &ioX, int32_t &ioY) const
{
	ConvertToParent(ioX, ioY);
	if (mParent != nullptr)
		mParent->ConvertToScreen(ioX, ioY);
}

void MView::ConvertFromScreen(int32_t &ioX, int32_t &ioY) const
{
	if (mParent != nullptr)
		mParent->ConvertFromScreen(ioX, ioY);
	ConvertFromParent(ioX, ioY);
}

bool MView::PastePrimaryBuffer(const std::string &inText)
{
	return false;
}

// --------------------------------------------------------------------

MPager::MPager(const std::string &inID, MRect inBounds)
	: MView(inID, inBounds)
{
}

void MPager::AddPage(MView *inPage)
{
	AddChild(inPage);
}

void MPager::SelectPage(uint32_t inPage)
{
	if (inPage < mChildren.size())
	{
		for (MView *page : mChildren)
		{
			if (inPage-- == 0)
			{
				page->Show();
				page->Enable();
			}
			else
			{
				page->Hide();
				page->Disable();
			}
		}
	}
}

void MPager::RecalculateLayout()
{
	MRect b = mBounds;

	for (MView *child : mChildren)
		b |= child->GetFrame();

	mBounds.x = mLeftMargin;
	mBounds.y = mTopMargin;
	mBounds.width = b.width;
	mBounds.height = b.height;

	mFrame.width = b.width + mLeftMargin + mRightMargin;
	mFrame.height = b.height + mTopMargin + mBottomMargin;
}

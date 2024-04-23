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

		mBounds.x = mLayout.mMargin.left;
		mBounds.y = mLayout.mMargin.top;
		mBounds.width = mFrame.width - mLayout.mMargin.left - mLayout.mMargin.right;
		mBounds.height = mFrame.height - mLayout.mMargin.top - mLayout.mMargin.bottom;
	}
}

MViewLayout MView::GetLayout() const
{
	return mLayout;
}

void MView::SetLayout(MViewLayout inLayout)
{
	int32_t dx = inLayout.mMargin.left - mLayout.mMargin.left;
	int32_t dy = inLayout.mMargin.top - mLayout.mMargin.top;

	mLayout = inLayout;

	mBounds.x = mLayout.mMargin.left;
	mBounds.y = mLayout.mMargin.top;

	mFrame.width = mBounds.width + mLayout.mMargin.left + mLayout.mMargin.right;
	mFrame.height = mBounds.height + mLayout.mMargin.top + mLayout.mMargin.bottom;

	for (MView *child : mChildren)
		child->MoveFrame(dx, dy);
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
	mBounds.width += inWidthDelta;

	mFrame.height += inHeightDelta;
	mBounds.height += inHeightDelta;

	for (MView *child : mChildren)
	{
		if (child->mVisible == eTriStateOff)
			continue;

		child->ResizeFrame(inWidthDelta, inHeightDelta);
	}
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

	mFrame.width = b.width + mLayout.mMargin.left + mLayout.mMargin.right;
	mFrame.height = b.height + mLayout.mMargin.top + mLayout.mMargin.bottom;
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
	ioX += mFrame.x - mBounds.x + mLayout.mMargin.left;
	ioY += mFrame.y - mBounds.y + mLayout.mMargin.top;
}

void MView::ConvertFromParent(int32_t &ioX, int32_t &ioY) const
{
	ioX -= mFrame.x - mBounds.x + mLayout.mMargin.left;
	ioY -= mFrame.y - mBounds.y + mLayout.mMargin.top;
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

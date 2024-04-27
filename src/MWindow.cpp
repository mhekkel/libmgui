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

#include "MWindow.hpp"
#include "MApplication.hpp"
#include "MControls.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MUtils.hpp"

#include "mrsrc.hpp"

#include <iostream>

#undef GetNextWindow

// --------------------------------------------------------------------
//
//	MWindow
//

MWindow *MWindow::sFirst = nullptr;


MWindow::MWindow(const std::string &inTitle, const MRect &inBounds, MWindowFlags inFlags)
	: MWindow(MWindowImpl::Create(inTitle, inBounds, inFlags, this))
{
	mVisible = eTriStateLatent;

	mBounds.x = mBounds.y = 0;

	SetLayout({ true, 0 });
}

MWindow::MWindow(MWindowImpl *inImpl)
	: MView("window", MRect(0, 0, 100, 100))
	, mImpl(inImpl)
{
	SetLayout({ true, 0 });

	mNext = sFirst;
	sFirst = this;
}

MWindow::~MWindow()
{
	RemoveFromList(this);
	delete mImpl;
}

void MWindow::DoClose()
{
	if (AllowClose(false))
		Close();
}

void MWindow::RemoveFromList(MWindow *inWindow)
{
	if (inWindow == sFirst)
		sFirst = inWindow->mNext;
	else if (sFirst != nullptr)
	{
		MWindow* w = sFirst;
		while (w != nullptr)
		{
			MWindow* next = w->mNext;
			if (next == inWindow)
			{
				w->mNext = inWindow->mNext;
				break;
			}
			w = next;
		}
	}
}

void MWindow::SetImpl(MWindowImpl *inImpl)
{
	if (mImpl != nullptr)
		delete mImpl;
	mImpl = inImpl;
}

void MWindow::Activate()
{
	if (mActive != eTriStateOn and IsVisible())
	{
		mActive = eTriStateOn;
		ActivateSelf();
		MView::Activate();

		mLastActivate = std::chrono::steady_clock::now();

		if (mLatentFocus != nullptr)
			mLatentFocus->SetFocus();
	}
}

MWindowFlags MWindow::GetFlags() const
{
	return mImpl->GetFlags();
}

MWindow *MWindow::GetWindow() const
{
	return const_cast<MWindow *>(this);
}

void MWindow::Show()
{
	mVisible = eTriStateOn;
	ShowSelf();

	MView::Show();
}

void MWindow::ShowSelf()
{
	mImpl->Show();
}

void MWindow::HideSelf()
{
	mImpl->Hide();
}

void MWindow::Select()
{
	mImpl->Select();
}

bool MWindow::IgnoreSelectClick()
{
	return (std::chrono::steady_clock::now() - mLastActivate) < std::chrono::milliseconds(100);
}

void MWindow::UpdateNow()
{
	mImpl->UpdateNow();
}

bool MWindow::AllowClose(bool inQuit)
{
	return true;
}

void MWindow::Close()
{
	if (mImpl != nullptr)
		mImpl->Close();
}

void MWindow::SetTitle(const std::string &inTitle)
{
	mTitle = inTitle;

	if (mModified)
		mImpl->SetTitle(mTitle + "*");
	else
		mImpl->SetTitle(mTitle);
}

std::string MWindow::GetTitle() const
{
	return mTitle;
}

void MWindow::SetIconName(const std::string &inIconName)
{
	mImpl->SetIconName(inIconName);
}

void MWindow::SetModifiedMarkInTitle(bool inModified)
{
	if (mModified != inModified)
	{
		mModified = inModified;
		SetTitle(mTitle);
	}
}

void MWindow::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	ResizeWindow(inWidthDelta, inHeightDelta);
}

void MWindow::ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta)
{
	mImpl->ResizeWindow(inWidthDelta, inHeightDelta);
}

void MWindow::GetWindowPosition(MRect &outPosition)
{
	mImpl->GetWindowPosition(outPosition);
}

void MWindow::SetWindowPosition(const MRect &inPosition, bool inTransition)
{
	mImpl->SetWindowPosition(inPosition, inTransition);
}

void MWindow::ConvertToScreen(int32_t &ioX, int32_t &ioY) const
{
	mImpl->ConvertToScreen(ioX, ioY);
}

void MWindow::ConvertFromScreen(int32_t &ioX, int32_t &ioY) const
{
	mImpl->ConvertFromScreen(ioX, ioY);
}

void MWindow::SetCursor(MCursor inCursor)
{
	mImpl->SetCursor(inCursor);
}

void MWindow::ObscureCursor()
{
	mImpl->ObscureCursor();
}

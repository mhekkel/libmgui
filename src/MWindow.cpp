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
#include "MCommands.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MUtils.hpp"
#include "MWindowImpl.hpp"

#include "mrsrc.hpp"

#include "zeep/xml/document.hpp"

#include <iostream>

#undef GetNextWindow

using namespace std;
using namespace zeep;

// --------------------------------------------------------------------
//
//	MWindowImpl
//

MMenuBar *MWindowImpl::CreateMenu(const std::string &inMenu)
{
	using namespace std::literals;

	mrsrc::istream rsrc("Menus/"s + inMenu + ".xml");
	xml::document doc(rsrc);

	return MMenuBar::Create(&doc.front());
}

// --------------------------------------------------------------------
//
//	MWindow
//

list<MWindow *> MWindow::sWindowList;

MWindow::MWindow(const string &inTitle, const MRect &inBounds, MWindowFlags inFlags, const string &inMenu)
	: MView("window", inBounds)
	, MHandler(gApp)
	, mImpl(MWindowImpl::Create(inTitle, inBounds, inFlags, inMenu, this))
	, mModified(false)
{
	mVisible = eTriStateLatent;

	mBounds.x = mBounds.y = 0;

	SetBindings(true, true, true, true);

	sWindowList.push_back(this);
}

MWindow::MWindow(MWindowImpl *inImpl)
	: MView("window", MRect(0, 0, 100, 100))
	, MHandler(gApp)
	, mImpl(inImpl)
{
	SetBindings(true, true, true, true);

	sWindowList.push_back(this);
}

MWindow::~MWindow()
{
	delete mImpl;
	RemoveWindowFromWindowList(this);
}

void MWindow::SetImpl(MWindowImpl *inImpl)
{
	if (mImpl != nullptr)
		delete mImpl;
	mImpl = inImpl;
}

void MWindow::Mapped()
{
	SuperShow();
}

void MWindow::Unmapped()
{
	SuperHide();
}

MWindowFlags MWindow::GetFlags() const
{
	return mImpl->GetFlags();
}

MWindow *MWindow::GetFirstWindow()
{
	MWindow *result = nullptr;
	if (not sWindowList.empty())
		result = sWindowList.front();
	return result;
}

MWindow *MWindow::GetNextWindow() const
{
	MWindow *result = nullptr;

	list<MWindow *>::const_iterator w =
		find(sWindowList.begin(), sWindowList.end(), this);

	if (w != sWindowList.end())
	{
		++w;
		if (w != sWindowList.end())
			result = *w;
	}

	return result;
}

void MWindow::RemoveWindowFromWindowList(MWindow *window)
{
	sWindowList.erase(remove(sWindowList.begin(), sWindowList.end(), window), sWindowList.end());
}

bool MWindow::WindowExists(MWindow *window)
{
	return find(sWindowList.begin(), sWindowList.end(), window) != sWindowList.end();
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
	if (not mImpl->Visible())
		Show();
	mImpl->Select();
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

void MWindow::SetTitle(const string &inTitle)
{
	mTitle = inTitle;

	if (mModified)
		mImpl->SetTitle(mTitle + "*");
	else
		mImpl->SetTitle(mTitle);
}

string MWindow::GetTitle() const
{
	return mTitle;
}

void MWindow::SetModifiedMarkInTitle(bool inModified)
{
	if (mModified != inModified)
	{
		mModified = inModified;
		SetTitle(mTitle);
	}
}

void MWindow::SetTransparency(float inAlpha)
{
	mImpl->SetTransparency(inAlpha);
}

bool MWindow::UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Close:
			outEnabled = true;
			break;

		default:
			result = MHandler::UpdateCommandStatus(inCommand, inMenu, inItemIndex, outEnabled, outChecked);
	}

	return result;
}

bool MWindow::ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_Close:
			if (AllowClose(false))
				Close();
			break;

		default:
			result = MHandler::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}

	return result;
}

void MWindow::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	ResizeWindow(inWidthDelta, inHeightDelta);
	// MView::ResizeFrame(0, 0);

	// MRect frame;
	// CalculateFrame(frame);
	// if (frame != mFrame)
	// mImpl->ResizeWindow(frame.width - mFrame.width, frame.height - mFrame.height);
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

uint32_t MWindow::GetModifiers() const
{
	return mImpl->GetModifiers();
}

void MWindow::SetCursor(MCursor inCursor)
{
	mImpl->SetCursor(inCursor);
}

void MWindow::ObscureCursor()
{
	mImpl->ObscureCursor();
}

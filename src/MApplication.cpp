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

#include "MApplication.hpp"
#include "MCommands.hpp"
#include "MControls.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MPreferences.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include <filesystem>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

#if DEBUG
int VERBOSE, TRACE;
#endif

MApplication *gApp;
fs::path gExecutablePath, gPrefixPath;

// --------------------------------------------------------------------

void MAsyncHandlerBase::execute()
{
	try
	{
		execute_self();
	}
	catch (const std::exception &ex)
	{
		std::cerr << ex.what() << std::endl;
	}
}

// --------------------------------------------------------------------

MApplication::MApplication(MApplicationImpl *inImpl)
	: MHandler(nullptr)
	, mImpl(inImpl)
	, mQuit(false)
	, mQuitPending(false)
{
	// set the global pointing to us
	gApp = this;
}

MApplication::~MApplication()
{
	delete mImpl;
}

void MApplication::Initialise()
{
	mImpl->Initialise();
}

void MApplication::SaveGlobals()
{
	Preferences::SaveIfDirty();
}

void MApplication::DoNew()
{
}

void MApplication::DoOpen()
{
}

void MApplication::Open(const string &inURI)
{
}

bool MApplication::ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_New:
			DoNew();
			break;

		case cmd_Open:
			DoOpen();
			break;

		case cmd_SelectWindowFromMenu:
			DoSelectWindowFromWindowMenu(inItemIndex - 2);
			break;

		case cmd_Quit:
			if (AllowQuit(false))
				DoQuit();
			break;

		default:
			result = false;
			break;
	}

	return result;
}

bool MApplication::UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked)
{
	bool result = true;

	switch (inCommand)
	{
		case cmd_New:
		case cmd_Open:
		case cmd_SelectWindowFromMenu:
		case cmd_Quit:
			outEnabled = true;
			break;

		default:
			result = false;
			break;
	}

	return result;
}

void MApplication::UpdateSpecialMenu(const string &inName, MMenu *inMenu)
{
	if (inName == "window")
		UpdateWindowMenu(inMenu);
	else
		PRINT(("Unknown special menu %s", inName.c_str()));
}

void MApplication::UpdateWindowMenu(MMenu *inMenu)
{
}

void MApplication::DoSelectWindowFromWindowMenu(uint32_t inIndex)
{
}

int MApplication::RunEventLoop()
{
	return mImpl->RunEventLoop();
}

bool MApplication::AllowQuit(bool inLogOff)
{
	bool result = mQuitPending;

	if (result == false)
	{
		result = true;

		MWindow *window = MWindow::GetFirstWindow();
		while (window != nullptr)
		{
			if (not window->AllowClose(true))
			{
				result = false;
				break;
			}

			window = window->GetNextWindow();
		}

		mQuitPending = result;
	}

	return result;
}

void MApplication::DoQuit()
{
	mQuit = true;
	mQuitPending = true;

	SaveGlobals();

	mImpl->Quit();
}

void MApplication::Pulse()
{
	// if there are no visible windows left, we quit
	MWindow *front = MWindow::GetFirstWindow();
	while (front != nullptr and not front->IsVisible())
		front = front->GetNextWindow();

	if (front == nullptr)
		DoQuit();
	else
		eIdle(GetLocalTime());

	Preferences::SaveIfDirty();
}

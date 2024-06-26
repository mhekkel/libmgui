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
#include "MControls.hpp"
#include "MError.hpp"
#include "MMenu.hpp"
#include "MPreferences.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"
#include "MWindow.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>

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
		std::cerr << ex.what() << '\n';
	}
}

// --------------------------------------------------------------------

MApplication::MApplication(MApplicationImpl *inImpl)
	: mImpl(inImpl)

	, cQuit(this, "quit", &MApplication::DoQuit, 'Q', kControlKey)

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

void MApplication::SetIconName(const std::string &inIconName)
{
	mImpl->SetIconName(inIconName);
}

void MApplication::SaveGlobals()
{
	MPrefs::SaveIfDirty();
}

void MApplication::Execute(const std::string &inCommand,
		const std::vector<std::string> &inArguments)
{
}

bool MApplication::AllowQuit(bool inLogOff)
{
	return true;
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
	eIdle();
	MPrefs::SaveIfDirty();
}

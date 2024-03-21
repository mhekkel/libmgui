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

#include "MApplicationImpl.hpp"
#include "MHandler.hpp"
#include "MP2PEvents.hpp"
#include "MTypes.hpp"

#include <chrono>
#include <filesystem>
#include <list>
#include <vector>

extern const char kAppName[], kVersionString[];

extern std::filesystem::path gExecutablePath;

class MWindow;

// ===========================================================================

class MApplication : public MHandler
{
  public:
	static void Install(const std::string &inPrefix);
	static MApplication *Create(MApplicationImpl *inImpl);
	static int Main(const std::vector<std::string> &argv);

	~MApplication();
	virtual void Initialise();

	virtual void DoNew();
	virtual void DoOpen();
	// virtual void Open(const std::string &inURL);
	virtual void Execute(const std::string &inCommand,
		const std::vector<std::string> &inArguments);

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);

	virtual void UpdateSpecialMenu(const std::string &inMenuKind, MMenu *inMenu);
	virtual void UpdateWindowMenu(MMenu *inMenu);

	MEventOut<void()> eIdle;

	template <typename Handler>
	void ExecuteAsync(Handler &&handler)
	{
		mImpl->execute(std::move(handler));
	}

	virtual void Pulse();

	virtual bool AllowQuit(bool inLogOff);
	virtual void DoQuit();

	bool IsQuitting() const { return mQuitPending; }
	void CancelQuit() { mQuitPending = false; }

	MApplicationImpl *GetImpl() const { return mImpl; }

  protected:
	MApplication(MApplicationImpl *inImpl);

	typedef std::list<MWindow *> MWindowList;

	virtual void DoSelectWindowFromWindowMenu(uint32_t inIndex);

	virtual void SaveGlobals();

	MApplicationImpl *mImpl;

	bool mQuit;
	bool mQuitPending;
};

// --------------------------------------------------------------------

extern MApplication *gApp;

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

#include "MMenu.hpp"
#include "MP2PEvents.hpp"
#include "MTypes.hpp"
#include "MWindow.hpp"

#include <chrono>
#include <condition_variable>
#include <deque>
#include <filesystem>
#include <list>
#include <mutex>
#include <vector>

// --------------------------------------------------------------------

extern const char kAppName[], kVersionString[];

extern std::filesystem::path gExecutablePath;

// --------------------------------------------------------------------

class MAsyncHandlerBase
{
  public:
	MAsyncHandlerBase() {}
	MAsyncHandlerBase(const MAsyncHandlerBase &) = delete;
	MAsyncHandlerBase &operator=(const MAsyncHandlerBase &) = delete;

	virtual ~MAsyncHandlerBase() {}

	void execute();

  protected:
	virtual void execute_self() = 0;
};

template <typename Handler>
class MAsyncHandler : public MAsyncHandlerBase
{
  public:
	MAsyncHandler(Handler &&handler)
		: mHandler(std::move(handler))
	{
	}

	virtual void execute_self() override
	{
		mHandler();
	}

	Handler mHandler;
};

class MApplicationImpl
{
  public:
	MApplicationImpl() {}
	virtual ~MApplicationImpl() {}

	virtual void Initialise() = 0;
	virtual void SetIconName(const std::string &inIconName) = 0;
	virtual int RunEventLoop() = 0;
	virtual void Quit() = 0;

	virtual void InhibitQuit(bool inInhibit, const std::string &inReason, MWindowImpl *inImpl) = 0;

	virtual MWindow *GetActiveWindow() = 0;

	template <typename Handler>
	void execute(Handler &&h)
	{
		std::unique_lock lock(mMutex);

		mHandlerQueue.push_back(new MAsyncHandler{ std::move(h) });

		mCV.notify_one();
	}

	std::deque<MAsyncHandlerBase *> mHandlerQueue;
	std::mutex mMutex;
	std::condition_variable mCV;
};

// ===========================================================================

class MApplication
{
  public:
	static MApplication *Create(MApplicationImpl *inImpl);
	static int Main(const std::vector<std::string> &argv);

	~MApplication();
	virtual void Initialise();

	virtual void DoNew();
	virtual void DoOpen();
	// virtual void Open(const std::string &inURL);
	virtual void Execute(const std::string &inCommand, const std::vector<std::string> &inArguments);

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

	void InhibitQuit(bool inInhibit, const std::string &inReason, MWindow *inWindow = nullptr)
	{
		mImpl->InhibitQuit(inInhibit, inReason, inWindow ? inWindow->GetImpl() : nullptr);
	}

	MWindow *GetActiveWindow()
	{
		return mImpl->GetActiveWindow();
	}

	MApplicationImpl *GetImpl() const { return mImpl; }

  protected:
	MApplication(MApplicationImpl *inImpl);

	void SetIconName(const std::string &inIconName);

	virtual void SaveGlobals();

	MApplicationImpl *mImpl;

	bool mQuit;
	bool mQuitPending;
};

// --------------------------------------------------------------------

extern MApplication *gApp;

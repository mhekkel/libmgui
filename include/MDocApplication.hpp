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

#include "MApplication.hpp"
#include "MController.hpp"

// ===========================================================================

class MDocApplication : public MApplication
{
  public:
	~MDocApplication();

	virtual void Initialise();
	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex,
		bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex,
		uint32_t inModifiers);
	virtual void UpdateSpecialMenu(const std::string &inMenuKind, MMenu *inMenu);
	virtual void UpdateWindowMenu(MMenu *inMenu);
	virtual void UpdateRecentMenu(MMenu *inMenu);

	void AddToRecentMenu(std::string inURL);
	const std::string &GetRecent(uint32_t inIndex);

	virtual MWindow *DisplayDocument(MDocument *inDocument) = 0;
	virtual bool CloseAll(MCloseReason inReason);

	const std::string &GetCurrentFolder() const { return mCurrentFolder; }
	virtual void SetCurrentFolder(const std::string &inFolder) { mCurrentFolder = inFolder; }

  protected:
	MDocApplication(MApplicationImpl *inImpl);
	virtual bool IsCloseAllCandidate(MDocument *inDocument) { return true; }

	virtual void DoNew() = 0;
	virtual void DoOpen();
	virtual void DoQuit();
	virtual bool AllowQuit(bool inLogOff);
	virtual void DoSaveAll();
	virtual void DoSelectWindowFromWindowMenu(uint32_t inIndex);

	virtual void SaveGlobals();

	std::string mCurrentFolder;
	std::deque<std::string> mRecentFiles;
};

extern MDocApplication *gDocApp;

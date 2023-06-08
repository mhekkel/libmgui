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

#include "MController.hpp"
#include "MDocument.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

class MDocWindow : public MWindow
{
  public:
	MDocWindow(const std::string &inTitle, const MRect &inBounds, MWindowFlags inFlags, const std::string &inMenu);

	static MDocWindow *FindWindowForDocument(MDocument *inDocument);

	MEventIn<void(bool)> eModifiedChanged;
	MEventIn<void(MDocument *)> eFileSpecChanged;
	MEventIn<void(MDocument *)> eDocumentChanged;

	void SetDocument(MDocument *inDocument);

	MDocument *GetDocument();

	// document is about to be closed
	virtual void SaveState();

	virtual void AddRoutes(MDocument *inDocument);

	virtual void RemoveRoutes(MDocument *inDocument);

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);

	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);

  protected:
	static std::string GetUntitledTitle();

	virtual void DocumentChanged(MDocument *inDocument);

	virtual bool AllowClose(bool inQuit);

	virtual void ModifiedChanged(bool inModified);

	virtual void FileSpecChanged(MDocument *inDocument);

	virtual void ActivateSelf();

  protected:
	MController mController;

	virtual ~MDocWindow();
};

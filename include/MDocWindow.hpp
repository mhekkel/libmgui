/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Maarten L. Hekkelman
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
#include "MWindow.hpp"

class MDocWindow : public MWindow
{
  public:
	MDocWindow(const std::string &inTitle, const MRect &inBounds, MWindowFlags inFlags);

	static MDocWindow *FindWindowForDocument(MDocument *inDocument);

	MEventIn<void(bool)> eModifiedChanged;
	MEventIn<void(MDocument *, const std::filesystem::path &)> eFileSpecChanged;
	MEventIn<void(MDocument *)> eDocumentChanged;

	MDocument *GetDocument();

	// document is about to be closed
	virtual void SaveState();

	virtual void AddRoutes(MDocument *inDocument);

	virtual void RemoveRoutes(MDocument *inDocument);

	virtual void BeFocus();

  protected:
	static std::string GetUntitledTitle();

	virtual void Initialize(MDocument *inDocument);

	virtual void DocumentChanged(MDocument *inDocument);

	bool AllowClose(bool inQuit) override;

	virtual void ModifiedChanged(bool inModified);

	virtual void FileSpecChanged(MDocument *inDocument, const std::filesystem::path &inFile);

  protected:
	MController *mController;

	virtual ~MDocWindow();
};

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

/*
    Model = MDocument
    View = MTextView
    Controller = MController
*/

#include "MP2PEvents.hpp"
#include "MSaverMixin.hpp"

#include <filesystem>

class MDocument;
class MWindow;
class MDocWindow;

class MController : public MSaverMixin
{
  public:
	MController(MDocWindow *inWindow);
	~MController();

	void SetWindow(MDocWindow *inWindow);

	void SetDocument(MDocument *inDocument);

	MDocument *GetDocument() const { return mDocument; }

	MDocWindow *GetWindow() const { return mDocWindow; }

	bool SaveDocument();

	void RevertDocument();

	bool DoSaveAs(const std::filesystem::path &inURL);

	void CloseAfterNavigationDialog();

	void SaveDocumentAs();

	virtual bool TryCloseDocument(/* MCloseReason inAction */);
	virtual bool TryCloseController(/* MCloseReason inAction */);
	void TryDiscardChanges();

	MEventOut<void(MDocument *)> eDocumentChanged;

  protected:
	virtual void Print();

	MController(const MController &);
	MController &operator=(const MController &);

	MDocument *mDocument;
	MDocWindow *mDocWindow;
	bool mCloseOnNavTerminate;
};

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

#include "MHandler.hpp"
#include "MP2PEvents.hpp"

class MDocument;
class MWindow;

enum MCloseReason
{
	kSaveChangesClosingDocument,
	kSaveChangesClosingAllDocuments,
	kSaveChangesQuittingApplication
};

class MController : public MHandler
{
  public:
	MController(MWindow *inWindow);
	~MController();

	void SetDocument(MDocument *inDocument);

	MDocument *GetDocument() const { return mDocument; }
	MWindow *GetWindow() const { return mWindow; }

	bool SaveDocument();
	void RevertDocument();
	bool DoSaveAs(const std::string &inURL);
	void SaveDocumentAs();

	void CloseAfterNavigationDialog();

	virtual bool TryCloseDocument(MCloseReason inAction);
	virtual bool TryCloseController(MCloseReason inAction);
	void TryDiscardChanges();

	virtual bool UpdateCommandStatus(uint32_t inCommand, MMenu *inMenu, uint32_t inItemIndex, bool &outEnabled, bool &outChecked);
	virtual bool ProcessCommand(uint32_t inCommand, const MMenu *inMenu, uint32_t inItemIndex, uint32_t inModifiers);
	virtual bool HandleKeyDown(uint32_t inKeyCode, uint32_t inModifiers, bool inRepeat);
	virtual bool HandleCharacter(const std::string &inText, bool inRepeat);

	MEventOut<void(MDocument *)> eDocumentChanged;
	MEventOut<void(MDocument *)> eAboutToCloseDocument;

  protected:
	virtual void Print();

	MController(const MController &);
	MController &operator=(const MController &);

	MDocument *mDocument;
	MWindow *mWindow;
};

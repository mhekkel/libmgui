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

#include <filesystem>
#include <string>

class MWindow;

enum class MCloseReason
{
	ClosingDocument,
	ClosingAllDocuments,
	QuittingApplication
};

class MSaverMixin
{
  public:
	MSaverMixin();
	virtual ~MSaverMixin();

	static bool IsNavDialogVisible();

	virtual void TryCloseDocument(/* MCloseReason inAction,  */const std::string &inDocument, MWindow *inParentWindow);
	virtual void TryDiscardChanges(const std::string &inDocument, MWindow *inParentWindow);
	virtual void SaveDocumentAs(MWindow *inParentWindow, const std::filesystem::path &inSuggestedName);

  protected:
	virtual bool SaveDocument() = 0;
	virtual void RevertDocument() = 0;
	virtual bool DoSaveAs(const std::filesystem::path &inPath) = 0;
	virtual void CloseAfterNavigationDialog() = 0;

	static MSaverMixin *sFirst;

	MSaverMixin *mNext = nullptr;
	bool mCloseOnNavTerminate = true;
	bool mClosePending = false;
	bool mCloseAllPending = false;
	bool mQuitPending = false;
	bool mShowingDialog = false;
};

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

#include "MController.hpp"
#include "MAlerts.hpp"
#include "MApplication.hpp"
#include "MCommands.hpp"
#include "MDocument.hpp"
#include "MPreferences.hpp"
#include "MWindow.hpp"

#include <cassert>

namespace
{

const int32_t
	kAskSaveChanges_Save = 3,
	kAskSaveChanges_Cancel = 2,
	kAskSaveChanges_DontSave = 1,

	kDiscardChanges_Discard = 1;
}

MController::MController(MWindow *inWindow)
	: MHandler(inWindow)
	, mDocument(nullptr)
	, mWindow(inWindow)
{
}

MController::~MController()
{
	assert(mDocument == nullptr);
}

void MController::SetDocument(MDocument *inDocument)
{
	if (inDocument != mDocument)
	{
		if (mDocument != nullptr)
		{
			eAboutToCloseDocument(mDocument);
			mDocument->RemoveController(this);
		}

		mDocument = inDocument;

		if (mDocument != nullptr)
			mDocument->AddController(this);

		eDocumentChanged(mDocument);
	}
}

bool MController::ProcessCommand(
	uint32_t inCommand,
	const MMenu *inMenu,
	uint32_t inItemIndex,
	uint32_t inModifiers)
{
	bool handled = false;

	if (mDocument != nullptr)
		handled = mDocument->ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);

	if (not handled)
	{
		handled = true;
		switch (inCommand)
		{
				// case cmd_Close:
				//	TryCloseController(kSaveChangesClosingDocument);
				//	break;

			case cmd_Save:
				SaveDocument();
				break;

			case cmd_SaveAs:
				SaveDocumentAs();
				break;

			case cmd_Revert:
				TryDiscardChanges();
				break;

			case cmd_Print:
				Print();
				break;

			default:
				handled = MHandler::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
				break;
		}
	}

	return handled;
}

bool MController::UpdateCommandStatus(
	uint32_t inCommand,
	MMenu *inMenu,
	uint32_t inItemIndex,
	bool &outEnabled,
	bool &outChecked)
{
	bool handled = false;

	if (mDocument != nullptr)
		handled = mDocument->UpdateCommandStatus(inCommand, inMenu, inItemIndex,
			outEnabled, outChecked);

	if (not handled)
	{
		handled = true;
		switch (inCommand)
		{
			// always
			// case cmd_Close:
			case cmd_SaveAs:
			case cmd_Find:
			case cmd_Print:
				outEnabled = true;
				break;

			// dirty
			case cmd_Save:
				outEnabled =
					mDocument != nullptr and
					mDocument->IsModified() and
					(not mDocument->IsSpecified() or not mDocument->IsReadOnly());
				break;

			case cmd_Revert:
				outEnabled = mDocument != nullptr and mDocument->IsSpecified() and mDocument->IsModified();
				break;

			default:
				handled = MHandler::UpdateCommandStatus(
					inCommand, inMenu, inItemIndex, outEnabled, outChecked);
				break;
		}
	}

	return handled;
}

bool MController::HandleKeyDown(
	uint32_t inKeyCode,
	uint32_t inModifiers,
	bool inRepeat)
{
	bool handled = false;
	if (mDocument != nullptr)
		handled = mDocument->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
	if (not handled)
		handled = MHandler::HandleKeyDown(inKeyCode, inModifiers, inRepeat);
	return handled;
}

bool MController::HandleCharacter(const string &inText, bool inRepeat)
{
	bool handled = false;
	if (mDocument != nullptr)
		handled = mDocument->HandleCharacter(inText, inRepeat);
	if (not handled)
		handled = MHandler::HandleCharacter(inText, inRepeat);
	return handled;
}

bool MController::TryCloseDocument(MCloseReason inAction)
{
	bool result = true;

	if (mDocument != nullptr)
	{
		if (not mDocument->IsModified())
			SetDocument(nullptr);
		else
		{
			result = false;
			string name;

			// if (mDocument->IsSpecified())
			//	name = mDocument->GetFile().GetPath().filename();
			// else
			name = mWindow->GetTitle();

			// TODO: FOUT!!! save changes alert?
			switch (DisplayAlert(mWindow, "save-changes-alert", { name }))
			{
				case kAskSaveChanges_Save:
					if (SaveDocument())
					{
						CloseAfterNavigationDialog();
						result = true;
					}
					break;

				case kAskSaveChanges_Cancel:
					break;

				case kAskSaveChanges_DontSave:
					SetDocument(nullptr);
					result = true;
					break;
			}
		}
	}

	return result;
}

bool MController::TryCloseController(MCloseReason inAction)
{
	bool result = true;

	if (mDocument != nullptr)
	{
		if (mDocument->CountControllers() > 1 or not mDocument->IsModified())
			SetDocument(nullptr);
		else
			result = TryCloseDocument(inAction);
	}

	return result;
}

void MController::SaveDocumentAs()
{
	fs::path file;

	if (mDocument->IsSpecified())
		file = mDocument->GetFile().GetPath();
	else
		file = fs::path(mDocument->GetDocumentName());

	if (MFileDialogs::SaveFileAs(mWindow, file))
		mDocument->DoSaveAs(file);
}

void MController::TryDiscardChanges()
{
	if (mDocument == nullptr)
		return;

	if (DisplayAlert(mWindow, "discard-changes-alert", { mDocument->GetWindowTitle() }) == kDiscardChanges_Discard)
		RevertDocument();
}

bool MController::SaveDocument()
{
	bool result = true;

	try
	{
		if (mDocument != nullptr)
		{
			if (mDocument->IsSpecified() and not mDocument->IsReadOnly())
				result = mDocument->DoSave();
			else
			{
				result = false;
				SaveDocumentAs();
			}
		}
	}
	catch (std::exception &inErr)
	{
		DisplayError(inErr);
		result = false;
	}

	return result;
}

void MController::RevertDocument()
{
	mDocument->RevertDocument();
}

bool MController::DoSaveAs(const string &inURL)
{
	return mDocument->DoSaveAs(inURL);
}

void MController::CloseAfterNavigationDialog()
{
	SetDocument(nullptr);
}

void MController::Print()
{
}

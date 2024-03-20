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

#include "MDocWindow.hpp"
#include "MApplication.hpp"
#include "MDocWindow.hpp"
#include "MStrings.hpp"

#include <cassert>
#include <sstream>

#undef GetNextWindow

using namespace std;

// ------------------------------------------------------------------
//

MDocWindow::MDocWindow(const string &inTitle, const MRect &inBounds,
	MWindowFlags inFlags, const string &inMenu)
	: MWindow(inTitle, inBounds, inFlags, inMenu)
	, eModifiedChanged(this, &MDocWindow::ModifiedChanged)
	, eFileSpecChanged(this, &MDocWindow::FileSpecChanged)
	, eDocumentChanged(this, &MDocWindow::DocumentChanged)
	, mController(this)
{
	// SetFocus(&mController);
	AddRoute(mController.eDocumentChanged, eDocumentChanged);
}

MDocWindow::~MDocWindow()
{
}

void MDocWindow::SetDocument(
	MDocument *inDocument)
{
	mController.SetDocument(inDocument);
}

bool MDocWindow::AllowClose(bool inQuit)
{
	return mController.TryCloseController(
		inQuit ? kSaveChangesQuittingApplication : kSaveChangesClosingDocument);
}

MDocWindow *MDocWindow::FindWindowForDocument(MDocument *inDocument)
{
	MWindow *w = MWindow::GetFirstWindow();

	while (w != nullptr)
	{
		MDocWindow *d = dynamic_cast<MDocWindow *>(w);

		if (d != nullptr and d->GetDocument() == inDocument)
			break;

		w = w->GetNextWindow();
	}

	return static_cast<MDocWindow *>(w);
}

string MDocWindow::GetUntitledTitle()
{
	static int sDocNr = 0;
	stringstream result;

	result << _("Untitled");

	if (++sDocNr > 1)
		result << ' ' << sDocNr;

	return result.str();
}

bool MDocWindow::UpdateCommandStatus(
	uint32_t inCommand,
	MMenu *inMenu,
	uint32_t inItemIndex,
	bool &outEnabled,
	bool &outChecked)
{
	bool handled = true;

	switch (inCommand)
	{
		case cmd_Close:
			outEnabled = true;
			break;

		default:
			handled = MWindow::UpdateCommandStatus(
				inCommand, inMenu, inItemIndex, outEnabled, outChecked);
			break;
	}

	return handled;
}

bool MDocWindow::ProcessCommand(
	uint32_t inCommand,
	const MMenu *inMenu,
	uint32_t inItemIndex,
	uint32_t inModifiers)
{
	bool handled = true;

	switch (inCommand)
	{
		case cmd_Close:
			if (mController.TryCloseController(kSaveChangesClosingDocument))
				Close();
			break;

		default:
			handled = MWindow::ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);
			break;
	}

	return handled;
}

MDocument *MDocWindow::GetDocument()
{
	return mController.GetDocument();
}

void MDocWindow::SaveState()
{
}

void MDocWindow::DocumentChanged(
	MDocument *inDocument)
{
	if (inDocument != nullptr)
	{
		// set title

		if (inDocument->IsSpecified())
			FileSpecChanged(inDocument);
		else
			SetTitle(GetUntitledTitle());

		ModifiedChanged(inDocument->IsModified());
	}
}

void MDocWindow::ModifiedChanged(
	bool inModified)
{
	SetModifiedMarkInTitle(inModified);
}

void MDocWindow::FileSpecChanged(
	MDocument *inDocument)
{
	MDocument *doc = mController.GetDocument();

	if (doc != nullptr)
	{
		string title = doc->GetWindowTitle();

		if (doc->IsReadOnly())
			title += _(" [Read Only]");

		SetTitle(title);
	}
	else
		SetTitle(GetUntitledTitle());
}

void MDocWindow::AddRoutes(
	MDocument *inDocument)
{
	AddRoute(inDocument->eModifiedChanged, eModifiedChanged);
	AddRoute(inDocument->eFileSpecChanged, eFileSpecChanged);
}

void MDocWindow::RemoveRoutes(
	MDocument *inDocument)
{
	RemoveRoute(inDocument->eModifiedChanged, eModifiedChanged);
	RemoveRoute(inDocument->eFileSpecChanged, eFileSpecChanged);
}

// void MDocWindow::ActivateSelf()
// {
// 	static bool sShowingDialog = false;
// 	if (not sShowingDialog)
// 	{
// 		value_changer<bool> save(sShowingDialog, true);

// 		MDocument *doc = mController.GetDocument();
// 		if (doc != nullptr and doc->IsSpecified())
// 			doc->CheckIfModifiedOnDisk(this);
// 	}
// }

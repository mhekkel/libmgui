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

#include "MDocWindow.hpp"
#include "MStrings.hpp"

#include <cassert>
#include <sstream>

// ------------------------------------------------------------------
//

MDocWindow::MDocWindow(const std::string &inTitle, const MRect &inBounds, MWindowFlags inFlags)
	: MWindow(inTitle, inBounds, inFlags)
	, eModifiedChanged(this, &MDocWindow::ModifiedChanged)
	, eFileSpecChanged(this, &MDocWindow::FileSpecChanged)
	, eDocumentChanged(this, &MDocWindow::DocumentChanged)
	, mController(nullptr)
{
}

MDocWindow::~MDocWindow()
{
	delete mController;
}

void MDocWindow::Initialize(MDocument *inDocument)
{
	if (mController == nullptr)
	{
		mController = new MController(this);
		AddRoute(mController->eDocumentChanged, eDocumentChanged);
	}

	mController->SetDocument(inDocument);
}

bool MDocWindow::AllowClose(bool inQuit)
{
	return mController ? mController->TryCloseController(/* inQuit ? MCloseReason::QuittingApplication : MCloseReason::ClosingDocument */) : true;
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

std::string MDocWindow::GetUntitledTitle()
{
	static int sDocNr = 0;
	std::stringstream result;

	result << _("Untitled");

	if (++sDocNr > 1)
		result << ' ' << sDocNr;

	return result.str();
}

MDocument *MDocWindow::GetDocument()
{
	return mController->GetDocument();
}

void MDocWindow::SaveState()
{
}

void MDocWindow::DocumentChanged(MDocument *inDocument)
{
	if (inDocument != nullptr)
	{
		// set title

		if (inDocument->IsSpecified())
			FileSpecChanged(inDocument, inDocument->GetFile());
		else
			SetTitle(GetUntitledTitle());

		ModifiedChanged(inDocument->IsModified());
	}
	else
		Close();
}

void MDocWindow::ModifiedChanged(bool inModified)
{
	SetModifiedMarkInTitle(inModified);
}

void MDocWindow::FileSpecChanged(MDocument *inDocument,
	const std::filesystem::path &inFile)
{
	MDocument *doc = mController->GetDocument();

	if (doc != nullptr)
	{
		std::string title = doc->GetWindowTitle();

		if (doc->IsReadOnly())
			title += _(" [Read Only]");

		SetTitle(title);
	}
	else
		SetTitle(GetUntitledTitle());
}

void MDocWindow::AddRoutes(MDocument *inDocument)
{
	AddRoute(inDocument->eModifiedChanged, eModifiedChanged);
	AddRoute(inDocument->eFileSpecChanged, eFileSpecChanged);
}

void MDocWindow::RemoveRoutes(MDocument *inDocument)
{
	RemoveRoute(inDocument->eModifiedChanged, eModifiedChanged);
	RemoveRoute(inDocument->eFileSpecChanged, eFileSpecChanged);
}

void MDocWindow::BeFocus()
{
	// mController->TakeFocus();
}

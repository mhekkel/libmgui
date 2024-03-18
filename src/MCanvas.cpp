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

#include "MApplication.hpp"
#include "MCanvasImpl.hpp"
#include "MControls.inl"

MCanvas::MCanvas(const std::string &inID, MRect inBounds, bool inAcceptDropFiles, bool inAcceptDropText)
	: MControl<MCanvasImpl>(inID, inBounds, MCanvasImpl::Create(this, inBounds.width, inBounds.height))
	, mAcceptDropFiles(inAcceptDropFiles)
	, mAcceptDropText(inAcceptDropText)
{
	mWillDraw = true;
}

MCanvas::~MCanvas()
{
	delete mImpl;
}

// void MCanvas::MoveFrame(
//	int32_t			inXDelta,
//	int32_t			inYDelta)
//{
//	mImpl->MoveFrame(inXDelta, inYDelta);
// }
//
// void MCanvas::ResizeFrame(
//	int32_t			inWidthDelta,
//	int32_t			inHeightDelta)
//{
//	mImpl->ResizeFrame(inWidthDelta, inHeightDelta);
// }

void MCanvas::AddedToWindow()
{
	MControl::AddedToWindow();

	if (mAcceptDropFiles or mAcceptDropText)
		mImpl->AcceptDragAndDrop(mAcceptDropFiles, mAcceptDropText);
}

void MCanvas::Invalidate()
{
	mImpl->Invalidate();
}

void MCanvas::DragEnter()
{
}

bool MCanvas::DragWithin(int32_t inX, int32_t inY)
{
	return false;
}

void MCanvas::DragLeave()
{
}

bool MCanvas::Drop(bool inMove, int32_t inX, int32_t inY, const std::string &inText)
{
	return false;
}

bool MCanvas::Drop(int32_t inX, int32_t inY, const std::filesystem::path &inFile)
{
	try
	{
		// gApp->OpenOneDocument(inFile.string());
	}
	catch (...)
	{
	}
	return true;
}

void MCanvas::StartDrag()
{
	mImpl->StartDrag();
}

void MCanvas::DragSendData(std::string &outData)
{
}

void MCanvas::DragDeleteData()
{
}

// void MCanvas::SetFocus()
//{
//	mImpl->SetFocus();
// }
//
// void MCanvas::ReleaseFocus()
//{
//	mImpl->ReleaseFocus();
// }
//
// bool MCanvas::IsFocus() const
//{
//	return mImpl->IsFocus();
// }
//
// void MCanvas::TrackMouse(bool inTrackMove, bool inTrackExit)
//{
//	mImpl->TrackMouse(inTrackMove, inTrackExit);
// }

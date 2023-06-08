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

template <class IMPL>
MControl<IMPL>::MControl(const std::string &inID, MRect inBounds, IMPL *inImpl)
	: MControlBase(inID, inBounds)
	, mImpl(inImpl)
{
}

template <class IMPL>
MControl<IMPL>::~MControl()
{
	delete mImpl;
}

template <class IMPL>
bool MControl<IMPL>::IsFocus() const
{
	return mImpl->IsFocus();
}

template <class IMPL>
void MControl<IMPL>::SetFocus()
{
	mImpl->SetFocus();
}

template <class IMPL>
MControlImplBase *MControl<IMPL>::GetControlImplBase()
{
	return mImpl;
}

template <class IMPL>
void MControl<IMPL>::Draw()
{
	mImpl->Draw();
}

template <class IMPL>
void MControl<IMPL>::MoveFrame(int32_t inXDelta, int32_t inYDelta)
{
	MView::MoveFrame(inXDelta, inYDelta);
	mImpl->FrameMoved();
}

template <class IMPL>
void MControl<IMPL>::ResizeFrame(int32_t inWidthDelta, int32_t inHeightDelta)
{
	MView::ResizeFrame(inWidthDelta, inHeightDelta);
	mImpl->FrameResized();
}

template <class IMPL>
void MControl<IMPL>::SetMargins(int32_t inLeftMargin, int32_t inTopMargin, int32_t inRightMargin, int32_t inBottomMargin)

{
	MView::SetMargins(inLeftMargin, inTopMargin, inRightMargin, inBottomMargin);
	mImpl->MarginsChanged();
}

template <class IMPL>
void MControl<IMPL>::ActivateSelf()
{
	mImpl->ActivateSelf();
}

template <class IMPL>
void MControl<IMPL>::DeactivateSelf()
{
	mImpl->DeactivateSelf();
}

template <class IMPL>
void MControl<IMPL>::EnableSelf()
{
	mImpl->EnableSelf();
}

template <class IMPL>
void MControl<IMPL>::DisableSelf()
{
	mImpl->DisableSelf();
}

template <class IMPL>
void MControl<IMPL>::ShowSelf()
{
	mImpl->ShowSelf();
}

template <class IMPL>
void MControl<IMPL>::HideSelf()
{
	mImpl->HideSelf();
}

template <class IMPL>
void MControl<IMPL>::AddedToWindow()
{
	mImpl->AddedToWindow();

	MControlBase::AddedToWindow();
}

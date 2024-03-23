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

#include "MGtkControlsImpl.hpp"

#include "MCanvasImpl.hpp"

#include <cassert>


class MGtkCanvasImpl : public MGtkControlImpl<MCanvas>
{
  public:
	MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight);
	virtual ~MGtkCanvasImpl();

	cairo_t *GetCairo() const
	{
		assert(mCurrentCairo);
		return mCurrentCairo;
	}

	void CreateWidget() override;

	virtual bool OnMouseDown(int32_t inX, int32_t inY, uint32_t inButtonNr, uint32_t inClickCount, uint32_t inModifiers);
	virtual bool OnMouseMove(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseUp(int32_t inX, int32_t inY, uint32_t inModifiers);
	virtual bool OnMouseExit();

	void Invalidate();

	// // MCanvasImpl overrides
	// virtual void AcceptDragAndDrop(bool inFiles, bool inText);
	// virtual void StartDrag();

  protected:
	virtual bool OnDrawEvent(cairo_t *inCairo);
	virtual bool OnConfigureEvent(GdkEvent *inEvent);

	// virtual bool OnKeyPressEvent(GdkEvent *inEvent);
	void OnCommit(char *inText) override;

	virtual bool OnScrollEvent(GdkEvent *inEvent);

	cairo_t *mCurrentCairo = nullptr;
};

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

#include "MCanvas.hpp"
#include "MGtkControlsImpl.hpp"

#include <cassert>

// --------------------------------------------------------------------

class MGtkCanvasImpl : public MGtkControlImpl<MCanvas>
{
  public:
	MGtkCanvasImpl(MCanvas *inCanvas, uint32_t inWidth, uint32_t inHeight);
	~MGtkCanvasImpl();

	cairo_t *GetCairo() const
	{
		assert(mCurrentCairo);
		return mCurrentCairo;
	}

	void CreateWidget() override;

	void Invalidate() override;

  protected:

	void OnGestureClickPressed(double inX, double inY, gint inClickCount) override;
	void OnGestureClickReleased(double inX, double inY, gint inClickCount) override;
	void OnGestureClickStopped() override;

	void OnMiddleButtonClick(double inX, double inY, gint inClickCount) override;
	void OnSecondaryButtonClick(double inX, double inY, gint inClickCount) override;

	void OnPointerEnter(double inX, double inY) override;
	void OnPointerMotion(double inX, double inY) override;
	void OnPointerLeave() override;

	bool OnKeyPressed(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers) override;
	void OnKeyReleased(guint inKeyValue, guint inKeyCode, GdkModifierType inModifiers) override;
	void OnKeyModifiers(GdkModifierType inModifiers) override;

	void OnDecelerate(double inVelX, double inVelY) override;
	bool OnScroll(double inX, double inY) override;
	void OnScrollBegin() override;
	void OnScrollEnd() override;

	void OnCommit(char *inText) override;

	static void Draw(GtkDrawingArea *area, cairo_t *cr, int width, int height, gpointer data);

	MSlot<void(int, int)> mResize;
	void Resize(int width, int height);

	cairo_t *mCurrentCairo = nullptr;
};

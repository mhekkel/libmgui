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

#include "MDeviceImpl.hpp"

#include <cairo/cairo.h>
#include <pango/pango.h>

#include <stack>

// --------------------------------------------------------------------
// base class for MDeviceImp
// provides only the basic Pango functionality
// This is needed in measuring text metrics and such

class MGtkDeviceImpl : public MDeviceImpl
{
  public:
	MGtkDeviceImpl();

	MGtkDeviceImpl(PangoLayout *inLayout);

	~MGtkDeviceImpl() override;

	void Save() override;
	void Restore() override;

	void SetOrigin(int32_t inX, int32_t inY) override;

	void SetFont(const std::string &inFont) override;

	void SetForeColor(MColor inColor) override;

	MColor GetForeColor() const override;

	void SetBackColor(MColor inColor) override;

	MColor GetBackColor() const override;

	void ClipRect(MRect inRect) override;

	// virtual void			ClipRegion(MRegion inRegion) override;

	void EraseRect(MRect inRect) override;

	void FillRect(MRect inRect) override;

	void StrokeRect(MRect inRect, uint32_t inLineWidth = 1) override;

	void FillEllipse(MRect inRect) override;

	// void DrawImage(cairo_surface_t *inImage, float inX, float inY, float inShear);

	// void CreateAndUsePattern(MColor inColor1, MColor inColor2, uint32_t inWidth, float inRotation) override;

	PangoFontMetrics *GetMetrics();

	float GetAscent() override;

	float GetDescent() override;

	float GetLeading() override;

	float GetXWidth() override;

	void DrawString(const std::string &inText, float inX, float inY, uint32_t inTruncateWidth = 0, MAlignment inAlign = eAlignNone) override;

	uint32_t GetStringWidth(const std::string &inText) override;

	// Text Layout options

	void SetText(const std::string &inText) override;

	void SetTabStops(float inTabWidth) override;

	void SetTextColors(uint32_t inColorCount, uint32_t inColorIndices[], uint32_t inOffsets[], MColor inColors[]) override;
	void SetTextStyles(uint32_t inStyleCount, uint32_t inStyles[], uint32_t inOffsets[]) override;
	void RenderTextBackground(float inX, float inY, uint32_t inStart, uint32_t inLength, MColor inColor) override;

	void SetTextSelection(uint32_t inStart, uint32_t inLength, MColor inSelectionColor) override;

	void IndexToPosition(uint32_t inIndex, bool inTrailing, int32_t &outPosition) override;

	bool PositionToIndex(int32_t inPosition, uint32_t &outIndex) override;

	float GetTextWidth() override;

	void RenderText(float inX, float inY) override;

	void DrawCaret(float inX, float inY, uint32_t inOffset) override;

	void BreakLines(uint32_t inWidth, std::vector<uint32_t> &outBreaks) override;

	void MakeTransparent(float inOpacity) override
	{
	}

	// virtual GdkPixmap *GetPixmap() const override
	// {
	// 	return nullptr;
	// }

	void SetDrawWhiteSpace(bool inDrawWhiteSpace, MColor inWhiteSpaceColor) override
	{
	}

  protected:
	PangoItem *Itemize(const char *inText, PangoAttrList *inAttrs);

	void GetWhiteSpaceGlyphs(uint32_t &outSpace, uint32_t &outTab, uint32_t &outNL);

	PangoLayout *mPangoLayout;
	PangoFontDescription *mFont;
	PangoFontMetrics *mMetrics;
	bool mTextEndsWithNewLine;
	uint32_t mSpaceGlyph, mTabGlyph, mNewLineGlyph;
	uint32_t mPangoScale;
};

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

/*
   Created by: Maarten L. Hekkelman
   Date: woensdag 09 januari, 2019
*/

#pragma once

#include "MTypes.hpp"
#include "MColor.hpp"

#include <initializer_list>
#include <memory>
#include <tuple>

class MView;

// --------------------------------------------------------------------

struct FontExtents
{
	float ascent, descent, height, maxXAdvance, maxYAdvance;
};

struct TextExtents
{
	float xBearing, yBearing, width, height, xAdvance, yAdvance;
};

// --------------------------------------------------------------------

enum MThemeColor
{
	kThemeColorMenuBackground,
	kThemeColorMenuFrame,
	kThemeColorMenuText,

	kThemeColorMenubarBackground,
	kThemeColorMenubarText,

	kThemeColorButtonBackground,
	kThemeColorButtonFrame,
	kThemeColorButtonText,

	kThemeColorCount
};

// --------------------------------------------------------------------

class MGfxDeviceImpl
{
  public:
	MGfxDeviceImpl() {}
	virtual ~MGfxDeviceImpl() {}

	virtual void ClipRect(float x, float y, float width, float height) = 0;

	virtual void SetColorRGB(float r, float g, float b) = 0;

	virtual void SetLineWidth(float lineWidth) = 0;

	virtual void MoveTo(float x, float y) = 0;
	virtual void LineTo(float x, float y) = 0;
	virtual void Stroke() = 0;

	virtual void FillRect(float x, float y, float width, float height) = 0;
	virtual void StrokeRect(float x, float y, float width, float height) = 0;

	virtual void FillRoundedRect(float x, float y, float width, float height, float radius) = 0;
	virtual void StrokeRoundedRect(float x, float y, float width, float height, float radius) = 0;

	virtual void FillPoly(std::initializer_list<std::tuple<float, float>> pts) = 0;
	virtual void StrokePoly(std::initializer_list<std::tuple<float, float>> pts) = 0;

	virtual void SetFont(const char *inFontName, float inSize,
		bool inBold = false, bool inItalic = false) = 0;

	virtual FontExtents GetFontExtents() = 0;

	virtual TextExtents GetTextExtents(const char *inText) = 0;

	virtual void ShowText(const char *inText) = 0;
};

// --------------------------------------------------------------------

class MGfxDevice
{
  public:
	// constructor to create an offscreen device
	MGfxDevice(int width, int height);

	// constructor that uses the underlying view
	MGfxDevice(MView &view);

	MGfxDevice(MGfxDeviceImpl *inImpl)
		: mImpl(inImpl)
	{
	}
	~MGfxDevice() {}

	MGfxDevice(const MGfxDevice &) = delete;
	MGfxDevice &operator=(const MGfxDevice &) = delete;

	void ClipRect(MRect r)
	{
		mImpl->ClipRect(r.x, r.y, r.width, r.height);
	}

	void SetColorRGB(MColor color)
	{
		mImpl->SetColorRGB(color.red / 255.f, color.green / 255.f, color.blue / 255.f);
	}

	void SetThemeColor(MThemeColor inColor, bool inEnabled = true, bool inActive = false);

	void SetLineWidth(float lineWidth)
	{
		mImpl->SetLineWidth(lineWidth);
	}

	void MoveTo(float x, float y)
	{
		mImpl->MoveTo(x, y);
	}

	void LineTo(float x, float y)
	{
		mImpl->LineTo(x, y);
	}

	void Stroke() { mImpl->Stroke(); }

	void StrokeLine(float x1, float y1, float x2, float y2);

	void FillRect(float x, float y, float width, float height)
	{
		mImpl->FillRect(x, y, width, height);
	}

	void FillRect(MRect r) { mImpl->FillRect(r.x, r.y, r.width, r.height); }

	void StrokeRect(float x, float y, float width, float height)
	{
		mImpl->StrokeRect(x, y, width, height);
	}

	void StrokeRect(MRect r) { mImpl->StrokeRect(r.x, r.y, r.width, r.height); }

	void FillRoundedRect(float x, float y, float width, float height, float radius)
	{
		mImpl->FillRoundedRect(x, y, width, height, radius);
	}

	void StrokeRoundedRect(float x, float y, float width, float height, float radius)
	{
		mImpl->StrokeRoundedRect(x, y, width, height, radius);
	}

	void FillRoundedRect(MRect r, float radius)
	{
		FillRoundedRect(r.x, r.y, r.width, r.height, radius);
	}

	void StrokeRoundedRect(MRect r, float radius)
	{
		StrokeRoundedRect(r.x, r.y, r.width, r.height, radius);
	}

	void FillPoly(std::initializer_list<std::tuple<float, float>> pts)
	{
		mImpl->FillPoly(pts);
	}

	void StrokePoly(std::initializer_list<std::tuple<float, float>> pts)
	{
		mImpl->StrokePoly(pts);
	}

	void SetFont(const char *inFontName, float inSize,
		bool inBold = false, bool inItalic = false)
	{
		mImpl->SetFont(inFontName, inSize, inBold, inItalic);
	}

	FontExtents GetFontExtents()
	{
		return mImpl->GetFontExtents();
	}

	TextExtents GetTextExtents(const char *inText)
	{
		return mImpl->GetTextExtents(inText);
	}

	void ShowText(const char *inText)
	{
		mImpl->ShowText(inText);
	}

	void ShowTextInRect(MRect bounds, const char *inText);

  protected:
	std::unique_ptr<MGfxDeviceImpl> mImpl;
};

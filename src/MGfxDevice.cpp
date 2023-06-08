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

#include "MGfxDevice.hpp"
#include "MColor.hpp"

using namespace std;

// --------------------------------------------------------------------

struct MThemeColorRGB
{
	MColor normal, disabled, active;
} kThemeColors[kThemeColorCount] = {
	{ "#fff", "#fff", "#3f67a5" }, //	kThemeColorMenuBackground,
	{ "#888", "#888", "#888" },    //	kThemeColorMenuFrame,
	{ "#000", "#777", "#fff" },    //	kThemeColorMenuText,

	{ "#ddd", "#ddd", "#3f67a5" }, //	kThemeColorMenubarBackground,
	{ "#M000", "#777", "#fff" },   //	kThemeColorMenubarText,

	{ "#eee", "#fff", "#3f67a5" }, //	kThemeColorButtonBackground,
	{ "#bbb", "#888", "#888" },    //	kThemeColorButtonFrame,
	{ "#000", "#777", "#fff" },    //	kThemeColorButtonText,

	//	{ "#2b71df", "#666", "#fa4" },	//	kThemeColorButtonBackground,
	//	{ "#888", "#888", "#888" },		//	kThemeColorButtonFrame,
	//	{ "#fff", "#777", "#fff" },		//	kThemeColorButtonText,

};

// --------------------------------------------------------------------

// --------------------------------------------------------------------

void MGfxDevice::SetThemeColor(MThemeColor inThemeColor, bool inEnabled, bool inActive)
{
	auto &themeColor = kThemeColors[inThemeColor];
	if (inEnabled == false)
		SetColorRGB(themeColor.disabled);
	else if (inActive)
		SetColorRGB(themeColor.active);
	else
		SetColorRGB(themeColor.normal);
}

void MGfxDevice::StrokeLine(float x1, float y1, float x2, float y2)
{
	MoveTo(x1, y1);
	LineTo(x2, y2);
	Stroke();
}

void MGfxDevice::ShowTextInRect(MRect bounds, const char *inText)
{
	auto fontExtends = GetFontExtents();
	auto textExtends = GetTextExtents(inText);

	float x = bounds.x + (bounds.width - textExtends.width) / 2;
	float y = bounds.y + (bounds.height - fontExtends.height) / 2 + fontExtends.ascent;

	MoveTo(x, y);
	ShowText(inText);
}

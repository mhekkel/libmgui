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

#include <iomanip>
#include <iostream>

#include <cstdint>

struct MColor
{
  public:
	uint8_t red;
	uint8_t green;
	uint8_t blue;

	MColor();
	MColor(const MColor &inOther);
	// MColor(// const GdkColor& inColor);
	MColor(const char *inHex);
	MColor(const std::string &inHex);
	MColor(uint8_t inRed, uint8_t inGreen, uint8_t inBlue);
	MColor(float inRed, float inGreen, float inBlue);
	MColor &operator=(const MColor &inOther);

	MColor Disable(const MColor &inBackColor) const;
	MColor Disable(const MColor &inBackColor, float inScale) const;
	MColor Distinct(const MColor &inBackColor) const;

	// bleach out a color (toward white, 0 <= factor <= 1)
	MColor Bleach(float inBleachFactor) const;
	// operator GdkColor() const;

	bool operator==(const MColor &rhs) const
	{
		return red == rhs.red and green == rhs.green and blue == rhs.blue;
	}

	bool operator!=(const MColor &rhs) const
	{
		return red != rhs.red or green != rhs.green or blue != rhs.blue;
	}

	std::string hex() const;
	void hex(const std::string &inHex);

	std::string str() const;
};

extern const MColor
	kWhite,
	kBlack,
	kNoteColor,
	kWarningColor,
	kErrorColor,
	kSelectionColor,
	kDialogBackgroundColor;

std::ostream &operator<<(std::ostream &os, const MColor &inColor);

void rgb2hsv(float r, float g, float b, float &h, float &s, float &v);
void hsv2rgb(float h, float s, float v, float &r, float &g, float &b);

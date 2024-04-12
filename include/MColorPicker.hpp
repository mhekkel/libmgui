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
#include "MDialog.hpp"

// --------------------------------------------------------------------

class MColorSwatch : public MCanvas
{
  public:
	MColorSwatch(const std::string &inID, MRect inBounds, MColor inColor);

	MColor GetColor() const;
	void SetColor(MColor inColor);

	void SetPalette(const std::vector<MColor> &inPaletteColors);

	MEventOut<void(const std::string &, MColor)> eColorChanged;
	MEventOut<void(const std::string &, MColor)> eColorPreview;

  private:
	void Draw() override;

	void ClickPressed(int32_t inX, int32_t inY, int32_t inClickCount, uint32_t inModifiers) override;
	void ClickReleased(int32_t inX, int32_t inY, uint32_t inModifiers) override;

	// void PointerEnter(int32_t inX, int32_t inY, uint32_t inModifiers) override;
	void PointerMotion(int32_t inX, int32_t inY, uint32_t inModifiers) override;
	// void PointerLeave() override;

	MEventIn<void(MColor)> ePickedColor;
	MEventIn<void(MColor)> ePreviewColor;

	void OnPickedColor(MColor inColor);
	void OnPreviewColor(MColor inColor);

	MColor mColor;
	std::vector<MColor> mPalette;
};

// --------------------------------------------------------------------

enum MPickerMode
{
	ePickSVH,
	ePickHVS,
	ePickHSV,
	ePickBGR,
	ePickBRG,
	ePickRGB
};

class MColorPicker : public MDialog
{
  public:
	MColorPicker(MWindow *inWindow, MColor inColor = kBlack,
		std::vector<MColor> inPalette = {});

	virtual bool OKClicked();
	virtual bool CancelClicked();

	virtual void RadiobuttonChanged(const std::string &inID, bool inValue);
	virtual void TextChanged(const std::string &inID, const std::string &inText);

	MEventOut<void(MPickerMode)> eChangedMode;
	MEventOut<void(MColor)> eChangedColor;
	MEventOut<void(MColor)> eSelectedColor;

	void SetColor(MColor inColor);
	void SetRGB(float inRed, float inGreen, float inBlue);
	void SetHSV(float inHue, float inSaturation, float inValue);

	void GetColor(MColor &outColor) const;
	void GetRGB(float &outRed, float &outGreen, float &outBlue) const;
	void GetHSV(float &outHue, float &outSaturation, float &outValue) const;

	void SetMode(MPickerMode inMode);
	MPickerMode GetMode() const;

  private:
	void UpdateColor();

	MPickerMode mMode;
	bool mSettingText;
	float mRed, mGreen, mBlue, mHue, mSaturation, mValue;
	MColor mOriginal;
};

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

/*	$Id: MDialog.h 133 2007-05-01 08:34:48Z maarten $
    Copyright Maarten L. Hekkelman
    Created Sunday August 15 2004 13:51:09
*/

#pragma once

#include <vector>

#include "MWindow.hpp"

class MDialog : public MWindow
{
  public:
	~MDialog();

	virtual bool OKClicked();

	virtual bool CancelClicked();

	using MWindow::Show;
	void Show(MWindow *inParent);
	bool ShowModal(MWindow *inParent);

	void SavePosition(const char *inName);

	void RestorePosition(const char *inName);

	MWindow *GetParentWindow() const { return mParentWindow; }

	using MHandler::SetFocus;
	void SetFocus(const std::string &inID);

	std::string GetText(const std::string &inID) const;
	void SetText(const std::string &inID, const std::string &inText);

	int32_t GetValue(const std::string &inID) const;
	void SetValue(const std::string &inID, int32_t inValue);

	// checkboxes
	bool IsChecked(const std::string &inID) const;
	void SetChecked(const std::string &inID, bool inChecked);

	// popup
	void SetChoices(const std::string &inID, std::vector<std::string> &inChoices);

	// chevron
	bool IsOpen(const std::string &inID) const;
	void SetOpen(const std::string &inID, bool inOpen);

	// color swatch
	MColor GetColor(const std::string &inID) const;
	void SetColor(const std::string &inID, MColor inColor);

	void SetEnabled(const std::string &inID, bool inEnabled);
	void SetVisible(const std::string &inID, bool inVisible);

	void SetPasswordChar(const std::string &inID, uint32_t inUnicode = 0x2022);

	virtual void ButtonClicked(const std::string &inID);
	virtual void CheckboxChanged(const std::string &inID, bool inValue);
	virtual void RadiobuttonChanged(const std::string &inID, bool inValue);
	virtual void TextChanged(const std::string &inID, const std::string &inText);
	virtual void ValueChanged(const std::string &inID, int32_t inValue);
	virtual void ColorChanged(const std::string &inID, MColor inColor);

	MEventIn<void(const std::string &)> eButtonClicked;
	MEventIn<void(const std::string &, bool)> eCheckboxClicked;
	MEventIn<void(const std::string &, bool)> eRadiobuttonClicked;
	MEventIn<void(const std::string &, const std::string &)> eTextChanged;
	MEventIn<void(const std::string &, int32_t)> eValueChanged;
	MEventIn<void(const std::string &, MColor)> eColorChanged;

  protected:
	MDialog(const std::string &inDialogResource);

	virtual void ChildResized();
	virtual void RecalculateLayout();

  private:
	MWindow *mParentWindow;
	MDialog *mNext; // for the close all
	static MDialog *sFirst;
};

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

#include "MControls.hpp"
#include "MWindow.hpp"

#include <mxml/node.hpp>

#include <vector>

// --------------------------------------------------------------------

class MDialog : public MWindow
{
  public:
	~MDialog();

	virtual bool OKClicked();
	virtual bool CancelClicked();

	using MWindow::Show;
	void Show(MWindow *inParent);

	void SavePosition(const char *inName);
	void RestorePosition(const char *inName);

	void SetDefaultButton(MButton *inButton);

	MWindow *GetParentWindow() const { return mParentWindow; }

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

	virtual void Build();

	MMargins GetMargins(const mxml::element &inTemplate);

	MView *CreateControls(const mxml::element &inTemplate, int32_t inX, int32_t inY);

	MView *CreateButton(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateColorSwatch(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateCaption(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateCheckbox(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateRadiobutton(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateExpander(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateCombobox(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateEdittext(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateFiller(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreatePopup(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateScrollbar(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateSeparator(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateBox(const mxml::element &inTemplate, int32_t inX, int32_t inY, bool inHorizontal);
	// MView *CreateTable(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreatePager(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateListBox(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	// MView *CreateListView(const mxml::element &inTemplate, int32_t inX, int32_t inY);
	MView *CreateCanvas(const mxml::element &inTemplate, int32_t inX, int32_t inY);

	std::string l(const std::string &s);

	bool AllowClose(bool inQuitting) override;

	void ChildResized() override;
	void RecalculateLayout() override;

	bool KeyPressed(uint32_t inKeyCode, char32_t inUnicode, uint32_t inModifiers, bool inAutoRepeat) override;
	void KeyReleased(uint32_t inKeyValue, uint32_t inModifiers) override;
	void Modifiers(uint32_t inModifiers) override;
	void EnterText(const std::string &inText) override;

	std::string mRsrc;
	float mDLUX, mDLUY;
	std::vector<MRadiobutton *> mRadioGroup;
	MButton *mOKButton = nullptr, *mCancelButton = nullptr;

  private:

	std::pair<int, bool> GetAttributeSize(const mxml::element &e, const char *name, float inDLU);

	MWindow *mParentWindow;
};

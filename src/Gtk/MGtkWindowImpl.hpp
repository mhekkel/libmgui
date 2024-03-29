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

#include "MGtkCommandEmitter.hpp"
#include "MGtkWidgetMixin.hpp"
#include "MMenu.hpp"
#include "MWindow.hpp"

#include <list>

class MGtkWindowImpl : public MWindowImpl, public MGtkWidgetMixin
{
  public:
	MGtkWindowImpl(MWindowFlags inFlags, MWindow *inWindow);
	~MGtkWindowImpl();

	virtual void Create(MRect inBounds, const std::string &inTitle);

	// A window contains a VBox, and in this VBox you have to add the various elements.
	// (in the right order, please!)
	virtual void AddStatusbarWidget(MGtkWidgetMixin *inChild);
	void Append(MGtkWidgetMixin *inChild) override;

	void SetTitle(std::string inTitle) override;
	void SetIconName(const std::string &inIconName) override;

	void Show() override;
	void Hide() override;

	void SetTransientFor(MWindow *inWindow) override;

	void Select() override;
	void Close() override;

	void ResizeWindow(int32_t inWidthDelta, int32_t inHeightDelta) override;

	void SetWindowPosition(MRect inBounds, bool inTransition) override;
	void GetWindowPosition(MRect &outBounds) const override;

	void UpdateNow() override;

	void SetCursor(MCursor inCursor) override;
	void ObscureCursor() override;

	void ConvertToScreen(int32_t &ioX, int32_t &ioY) const override;
	void ConvertFromScreen(int32_t &ioX, int32_t &ioY) const override;

	MWindow *GetWindow() const { return mWindow; }

	static MGtkWindowImpl *GetWindowImpl(GtkWindow *inW);

  protected:
	// bool DispatchKeyDown(uint32_t inKeyCode, uint32_t inModifiers, const std::string &inText) override;

	void OnDestroy() override;
	void OnMap() override;
	void OnUnmap() override;

	MSlot<bool()> mCloseRequest;
	bool OnCloseRequest();

	// void OnIsSuspendedChanged(GParamSpec *inProperty);
	// MSlot<void(GParamSpec *)> mIsSuspendedChanged;

	void OnIsActiveChanged(GParamSpec *inProperty);
	MSlot<void(GParamSpec *)> mIsActiveChanged;

	void CreateMainVBox();

	GtkWidget *mMainVBox;
	MGtkWidgetMixin *mFocus;
	bool mConfigured;
};

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

#include "MControls.hpp"

#include <filesystem>

class MCanvasImpl;

class MCanvas : public MControl<MCanvasImpl>
{
  public:
	typedef MCanvasImpl MImpl;

	MCanvas(const std::string &inID, MRect inBounds/* , bool inAcceptDropFiles, bool inAcceptDropText */);
	~MCanvas();

	// void AddedToWindow() override;

	void Invalidate() override;

	// void DragEnter() override;
	// bool DragWithin(int32_t inX, int32_t inY) override;
	// void DragLeave() override;
	// bool Drop(bool inMove, int32_t inX, int32_t inY override,
	// 	const std::string &inText);
	// bool Drop(int32_t inX, int32_t inY override,
	// 	const std::filesystem::path &inFile);

	// void StartDrag() override;
	// void DragSendData(std::string &outData) override;
	// void DragDeleteData() override;

  protected:
	// bool mAcceptDropFiles;
	// bool mAcceptDropText;
};

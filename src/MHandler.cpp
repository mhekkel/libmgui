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

#include "MHandler.hpp"
#include "MLib.hpp"

using namespace std;

MHandler::MHandler(MHandler *inSuper)
	: mSuper(inSuper)
{
}

MHandler::~MHandler()
{
	SetSuper(nullptr);
}

bool MHandler::UpdateCommandStatus(
	uint32_t inCommand,
	MMenu *inMenu,
	uint32_t inItemIndex,
	bool &outEnabled,
	bool &outChecked)
{
	bool result = false;

	if (mSuper != nullptr)
	{
		result = mSuper->UpdateCommandStatus(
			inCommand, inMenu, inItemIndex, outEnabled, outChecked);
	}
	else
	{
		outEnabled = false;
		outChecked = false;
	}

	return result;
}

bool MHandler::ProcessCommand(
	uint32_t inCommand,
	const MMenu *inMenu,
	uint32_t inItemIndex,
	uint32_t inModifiers)
{
	bool result = false;

	if (mSuper != nullptr)
		result = mSuper->ProcessCommand(inCommand, inMenu, inItemIndex, inModifiers);

	return result;
}

bool MHandler::HandleKeyDown(
	uint32_t inKeyCode,
	uint32_t inModifiers,
	bool inRepeat)
{
	bool result = false;
	if (mSuper != nullptr)
		result = mSuper->HandleKeyDown(inKeyCode, inModifiers, inRepeat);
	return result;
}

bool MHandler::HandleCharacter(const string &inText, bool inRepeat)
{
	bool result = false;
	if (mSuper != nullptr)
		result = mSuper->HandleCharacter(inText, inRepeat);
	return result;
}

void MHandler::SetSuper(
	MHandler *inSuper)
{
	mSuper = inSuper;
}

bool MHandler::IsFocus() const
{
	return false;
}

void MHandler::BeFocus()
{
}

void MHandler::DontBeFocus()
{
}

void MHandler::SetFocus()
{
}

void MHandler::ReleaseFocus()
{
}

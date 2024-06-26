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

#include <concepts>
#include <exception>
#include <future>
#include <sstream>
#include <vector>

class MWindow;

void DisplayError(const std::exception &inException);
void DisplayError(const std::string &inError);
void DisplayError(const std::error_code &inError);

template <typename T>
concept ReplyHandlerCallbackType = std::is_invocable_v<T, int>;


struct AlertReplyHandlerBase
{
	virtual ~AlertReplyHandlerBase() = default;

	virtual void HandleReply(int inReplyBtn) = 0;
};

template <ReplyHandlerCallbackType Handler>
struct AlertReplyHandler : public AlertReplyHandlerBase
{
	AlertReplyHandler(Handler &&handler)
		: mHandler(std::move(handler))
	{
	}

	void HandleReply(int inReplyBtn) override
	{
		mHandler(inReplyBtn);
	}

	Handler mHandler;
};

void DisplayAlert(MWindow *inParent, const std::string &inResourceName,
	AlertReplyHandlerBase *inHandler, std::initializer_list<std::string> inArguments = {});

template <ReplyHandlerCallbackType Handler>
void DisplayAlert(MWindow *inParent, const std::string &inResourceName,
	Handler &&inHandler, std::initializer_list<std::string> inArguments = {})
{
	DisplayAlert(inParent, inResourceName, new AlertReplyHandler(std::move(inHandler)), inArguments);
}

inline void DisplayAlert(MWindow *inParent, const std::string &inResourceName,
	std::initializer_list<std::string> inArguments = {})
{
	DisplayAlert(inParent, inResourceName, nullptr, inArguments);
}


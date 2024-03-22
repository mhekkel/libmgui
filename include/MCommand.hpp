/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Maarten L. Hekkelman
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
#include <memory>
#include <string>
#include <type_traits>

// --------------------------------------------------------------------

class MWindow;
class MApplication;

template <typename>
class MCommand;

template <typename T>
concept CommandEmitter = (
	std::is_base_of_v<MWindow, T> or std::is_base_of_v<MApplication, T>
);

template <typename Sig>
concept ImplementedSignature = (
	std::is_same_v<Sig, void(void)> or
	std::is_same_v<Sig, void(int)>
);

template <typename R, typename... Args>
	requires ImplementedSignature<R(Args...)>
class MCommand<R(Args...)>
{
	struct MCommandCallbackBase;

  public:
	template <CommandEmitter C>
	MCommand(C *owner, const std::string &action, R (C::*callback)(Args...))
		: mHandler(new MCommandHandler<C>(owner, callback))
	{
		RegisterCommand(owner, action);
	}

	R Execute(Args... args)
	{
		return mHandler->execute(std::forward<Args>(args)...);
	}

  private:

	void RegisterCommand(MApplication *app, const std::string &action);
	void RegisterCommand(MWindow *win, const std::string &action);

	struct MCommandHandlerBase
	{
		virtual ~MCommandHandlerBase() = default;
		virtual R execute(Args... args) = 0;
	};

	template <CommandEmitter C>
	struct MCommandHandler : public MCommandHandlerBase
	{
		using F = R (C::*)(Args...);

		MCommandHandler(C *owner, F method)
			: mOwner(owner)
			, mMethod(method)
		{
		}

		R execute(Args... args) override
		{
			return (mOwner->*mMethod)(args...);
		}

		C *mOwner;
		F mMethod;
	};



	std::unique_ptr<MCommandHandlerBase> mHandler;
};

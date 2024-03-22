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

#include "MCommand.hpp"
#include "MGtkWidgetMixin.hpp"

#include <gtk/gtk.h>

#include <map>

// --------------------------------------------------------------------

class MGtkCommandEmitter
{
  private:
	struct MActionHandlerBase
	{
		virtual ~MActionHandlerBase() = default;
		virtual void ActionActivated(GVariant *param) = 0;
	};

	template <ImplementedSignature Sig>
	struct MActionHandler;

	template <>
	struct MActionHandler<void()>
	{
		MActionHandler(MCommand<void()> &inCommand)
			: mCommand(inCommand)
		{
		}

		void ActionActivated(GVariant *param)
		{
			mCommand.Execute();
		}

		MCommand<void()> &mCommand;
	};

  public:
	MGtkCommandEmitter()
	{
	}

	virtual ~MGtkCommandEmitter()
	{
	}

	virtual GObject *GetObject() = 0;

	template <ImplementedSignature Sig>
	void RegisterAction(const std::string &action, MCommand<Sig> &inCommand)
	{
		GSimpleAction *act = g_simple_action_new(action.c_str(), nullptr);
		g_action_map_add_action(G_ACTION_MAP(GetObject()), G_ACTION(act));
		g_signal_connect(act, "activate", G_CALLBACK(MGtkCommandEmitter::ActionActivated), this);
	}

	static void ActionActivated(GAction *action, GVariant *param, void *user_data)
	{
		auto self = static_cast<MGtkCommandEmitter *>(user_data);
		auto h = self->mActionHandlers[action];
		if (h)
			h->ActionActivated(param);
	}

  private:
	std::map<GAction *, MActionHandlerBase *> mActionHandlers;

	// std::map<GAction *,
};
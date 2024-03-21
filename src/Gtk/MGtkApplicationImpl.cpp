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

#include "MAcceleratorTable.hpp"
#include "MAlerts.hpp"
#include "MApplication.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MUtils.hpp"

#include "Gtk/MGtkApplicationImpl.hpp"
#include "Gtk/MGtkWindowImpl.hpp"

#include "mrsrc.hpp"

#include <cstring>

MGtkApplicationImpl::MGtkApplicationImpl()
{
	sInstance = this;
}

void MGtkApplicationImpl::Initialise()
{
	GList *iconList = nullptr;

	mrsrc::rsrc appIconResource("Icons/appicon.png");
	if (appIconResource)
	{
		GInputStream *s = g_memory_input_stream_new_from_data(appIconResource.data(), appIconResource.size(), nullptr);
		THROW_IF_NIL(s);

		GError *error = nullptr;
		GdkPixbuf *icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
		if (icon)
			iconList = g_list_append(iconList, icon);

		if (error)
			g_free(error);
	}

	mrsrc::rsrc smallAppIconResource("Icons/appicon.png");
	if (smallAppIconResource)
	{
		GInputStream *s = g_memory_input_stream_new_from_data(smallAppIconResource.data(), smallAppIconResource.size(), nullptr);
		THROW_IF_NIL(s);

		GError *error = nullptr;
		GdkPixbuf *icon = gdk_pixbuf_new_from_stream(s, nullptr, &error);
		if (icon)
			iconList = g_list_append(iconList, icon);

		if (error)
			g_free(error);
	}

#warning FIXME
	// if (iconList)
	// 	gtk_window_set_default_icon_list(iconList);

	// now start up the normal executable
	gtk_window_set_default_icon_name("salt-terminal");

	// gdk_notify_startup_complete();
}

MGtkApplicationImpl::~MGtkApplicationImpl()
{
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
	if (mAsyncTaskThread.joinable())
		mAsyncTaskThread.join();
}

int MGtkApplicationImpl::RunEventLoop()
{
	mPulseID = g_timeout_add(100, &MGtkApplicationImpl::Timeout, nullptr);

	// Start processing async tasks

	mAsyncTaskThread = std::thread([this, context = g_main_context_get_thread_default()]()
		{ ProcessAsyncTasks(context); });

#warning FIXME
	// gtk_main();


	auto loop = g_main_loop_new(nullptr, false);
	g_main_loop_run(loop);
	g_object_unref(loop);

	return 0;
}

void MGtkApplicationImpl::Quit()
{
	if (mPulseID)
		g_source_remove(mPulseID);

#warning FIXME
	// gtk_main_quit();

	std::unique_lock lock(mMutex);
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
}

void MGtkApplicationImpl::ProcessAsyncTasks(GMainContext *context)
{
	std::unique_lock lock(mMutex);

	bool done = false;

	while (not done)
	{
		mCV.wait(lock, [this]()
			{ return not mHandlerQueue.empty(); });

		while (not mHandlerQueue.empty())
		{
			auto ah = mHandlerQueue.front();
			mHandlerQueue.pop_front();

			if (not ah)
			{
				done = true;
				break;
			}

			g_main_context_invoke_full(context, G_PRIORITY_DEFAULT,
				&MGtkApplicationImpl::HandleAsyncCallback, ah,
				(GDestroyNotify)&MGtkApplicationImpl::DeleteAsyncHandler);
		}
	}
}

gboolean MGtkApplicationImpl::HandleAsyncCallback(gpointer inData)
{
	// PRINT(("Handle Async Task in Thread ID = %p", std::this_thread::get_id()));

	MAsyncHandlerBase *handler = reinterpret_cast<MAsyncHandlerBase *>(inData);
	handler->execute();
	return G_SOURCE_REMOVE;
}

void MGtkApplicationImpl::DeleteAsyncHandler(gpointer inData)
{
	MAsyncHandlerBase *handler = reinterpret_cast<MAsyncHandlerBase *>(inData);
	delete handler;
}

gboolean MGtkApplicationImpl::Timeout(gpointer inData)
{
	try
	{
		MGtkWindowImpl::RecycleWindows();

		gApp->Pulse();
	}
	catch (const std::exception &e)
	{
		DisplayError(e);
	}

	return true;
}

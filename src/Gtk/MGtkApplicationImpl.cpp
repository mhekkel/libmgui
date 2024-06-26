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

#include "MAlerts.hpp"
#include "MDocApplication.hpp"
#include "MClipboard.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"

#include "MGtkApplicationImpl.hpp"
#include "MGtkWindowImpl.hpp"

#include "mrsrc.hpp"

#include <cstring>

MGtkApplicationImpl *MGtkApplicationImpl::sInstance;

MGtkApplicationImpl::MGtkApplicationImpl(const std::string &inApplicationID)
	: mStartup(this, &MGtkApplicationImpl::Startup)
	, mActivate(this, &MGtkApplicationImpl::Activate)
	, mQueryEnd(this, &MGtkApplicationImpl::OnQueryEnd)
	, mCommandLine(this, &MGtkApplicationImpl::CommandLine)
	, mGtkApplication(gtk_application_new(inApplicationID.c_str(), G_APPLICATION_HANDLES_COMMAND_LINE))
{
	sInstance = this;

	mStartup.Connect(G_OBJECT(mGtkApplication), "startup");
	mActivate.Connect(G_OBJECT(mGtkApplication), "activate");
	mCommandLine.Connect(G_OBJECT(mGtkApplication), "command-line");
	mQueryEnd.Connect(G_OBJECT(mGtkApplication), "query-end");

	GValue gv{ G_TYPE_BOOLEAN };
	g_value_set_boolean(&gv, true);
	g_object_set_property(G_OBJECT(mGtkApplication), "register-session", &gv);
	g_value_unset(&gv);
}

void MGtkApplicationImpl::Startup()
{
	mPulseID = g_timeout_add(100, &MGtkApplicationImpl::Timeout, nullptr);

	// Start processing async tasks
	mAsyncTaskThread = std::thread([this, context = g_main_context_get_thread_default()]()
		{ ProcessAsyncTasks(context); });

	MClipboard::InitPrimary();

	gApp->Initialise();
}

void MGtkApplicationImpl::Activate()
{
	MDocApplication::Instance().DoNew();
}

void MGtkApplicationImpl::Initialise()
{
}

void MGtkApplicationImpl::SetIconName(const std::string &inIconName)
{
	gtk_window_set_default_icon_name(inIconName.c_str());
}

int MGtkApplicationImpl::CommandLine(GApplicationCommandLine *inCommandLine)
{
	gchar **argv;
	gint argc;

	argv = g_application_command_line_get_arguments(inCommandLine, &argc);

	int result = gApp->HandleCommandLine(argc, argv);

	g_strfreev(argv);

	return result;
}

MGtkApplicationImpl::~MGtkApplicationImpl()
{
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
	if (mAsyncTaskThread.joinable())
		mAsyncTaskThread.join();

	g_object_unref(mGtkApplication);
}

int MGtkApplicationImpl::RunEventLoop()
{
	return g_application_run(G_APPLICATION(mGtkApplication), 0, nullptr);
}

void MGtkApplicationImpl::OnQueryEnd()
{
	if (not gApp->AllowQuit(true))
		InhibitQuit(true, _("There are open windows"), nullptr);
}

void MGtkApplicationImpl::InhibitQuit(bool inInhibit, const std::string &inReason, MWindowImpl *inImpl)
{
	if (inInhibit)
	{
		if (not mInhibitCookie)
		{
			if (auto w = gApp->GetActiveWindow(); inImpl == nullptr and w != nullptr)
				inImpl = w->GetImpl();

			GtkWindow *gw = inImpl ? GTK_WINDOW(static_cast<MGtkWindowImpl *>(inImpl)->GetWidget()) : nullptr;
			mInhibitCookie = gtk_application_inhibit(mGtkApplication, gw, GTK_APPLICATION_INHIBIT_LOGOUT, inReason.c_str());
		}
	}
	else
	{
		if (mInhibitCookie)
			gtk_application_uninhibit(mGtkApplication, mInhibitCookie);
	}
}

void MGtkApplicationImpl::Quit()
{
	if (mPulseID)
		g_source_remove(mPulseID);

	g_application_quit(G_APPLICATION(mGtkApplication));

	std::unique_lock lock(mMutex);
	mHandlerQueue.push_front(nullptr);
	mCV.notify_one();
}

MWindow *MGtkApplicationImpl::GetActiveWindow()
{
	MWindow *result = nullptr;
	if (auto w = gtk_application_get_active_window(mGtkApplication); w != nullptr)
	{
		if (auto impl = MGtkWindowImpl::GetWindowImpl(w); impl != nullptr)
			result = impl->GetWindow();
	}
	
	return result;
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
		gApp->Pulse();
	}
	catch (const std::exception &e)
	{
		DisplayError(e);
	}

	return true;
}

// --------------------------------------------------------------------

int MApplication::Main(const std::string &inApplicationID, const std::vector<std::string> &argv)
{
	try
	{
		// First find out who we are. Uses proc filesystem to find out.
		char exePath[PATH_MAX + 1];

		int r = readlink("/proc/self/exe", exePath, PATH_MAX);
		if (r > 0)
		{
			exePath[r] = 0;
			gExecutablePath = std::filesystem::canonical(exePath);
			gPrefixPath = gExecutablePath.parent_path();
		}

		assert(g_application_id_is_valid(inApplicationID.c_str()));

		MApplication *app = MApplication::Create(new MGtkApplicationImpl(inApplicationID));
		GApplication *g_app = G_APPLICATION(static_cast<MGtkApplicationImpl *>(app->GetImpl())->GetGtkApp());

		std::vector<char *> args;
		for (auto &a : argv)
			args.emplace_back(const_cast<char *>(a.c_str()));
		args.emplace_back(nullptr);

		return g_application_run(g_app, argv.size(), args.data());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}
	catch (...)
	{
		std::cerr << "Exception caught\n";
	}

	return 0;
}

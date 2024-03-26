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
#include "MApplication.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"

#include "MGtkApplicationImpl.hpp"
#include "MGtkWindowImpl.hpp"

#include "mrsrc.hpp"

#include <cstring>

MGtkApplicationImpl *MGtkApplicationImpl::sInstance;

MGtkApplicationImpl::MGtkApplicationImpl(std::function<void()> inActivateCB)
	: mStartup(this, &MGtkApplicationImpl::Startup)
	, mActivate(this, &MGtkApplicationImpl::Activate)
	, mCommandLine(this, &MGtkApplicationImpl::CommandLine)
	, mGtkApplication(gtk_application_new("com.hekkelman.mgui-dummy-app-id", G_APPLICATION_HANDLES_COMMAND_LINE))
	, mActivateCB(inActivateCB)
	, mQueryEnd(this, &MGtkApplicationImpl::OnQueryEnd)
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

	gApp->Initialise();
}

void MGtkApplicationImpl::Activate()
{
	gApp->DoNew();
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

int MGtkApplicationImpl::CommandLine(GApplicationCommandLine *inCommandLine)
{
	gchar **argv;
	gint argc;
	gint i;

	argv = g_application_command_line_get_arguments(inCommandLine, &argc);

	// g_application_command_line_print(inCommandLine,
	// 	"This text is written back\n"
	// 	"to stdout of the caller\n");

	for (i = 0; i < argc; i++)
		g_print("argument %d: %s\n", i, argv[i]);

	g_strfreev(argv);

	if (argc <= 1)
		gApp->DoNew();

	return 0;
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

int MApplication::Main(const std::vector<std::string> &argv)
{
	try
	{
		// First find out who we are. Uses proc filesystem to find out.
		char exePath[PATH_MAX + 1];

		int r = readlink("/proc/self/exe", exePath, PATH_MAX);
		if (r > 0)
		{
			exePath[r] = 0;
			gExecutablePath = fs::canonical(exePath);
			gPrefixPath = gExecutablePath.parent_path();
		}

		MApplication *app = MApplication::Create(new MGtkApplicationImpl([]() {}));

		std::vector<char *> args;
		for (auto &a : argv)
			args.emplace_back(const_cast<char *>(a.c_str()));
		args.emplace_back(nullptr);

		return g_application_run(G_APPLICATION(static_cast<MGtkApplicationImpl *>(app->GetImpl())->GetGtkApp()),
			argv.size(), args.data());
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

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

#include "Gtk/MGtkApplicationImpl.hpp"

#include "MApplication.hpp"
#include "MError.hpp"

#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace std;
namespace fs = std::filesystem;

#define MGDBUS_SERVER_NAME "com.hekkelman.GDBus.SaltServer"
#define MGDBUS_SERVER_OBJECT_NAME "/com/hekkelman/GDBus/SaltObject"

// bool gQuit = false;

MGtkApplicationImpl *MGtkApplicationImpl::sInstance = nullptr;

class MGDbusServer
{
  public:
	MGDbusServer(std::vector<std::string> &&argv)
		: mIntrospectionData(nullptr)
		, mOwnerId(0)
		, mRegistrationID(0)
		, mWatcherID(0)
		, mArguments(argv)
	{
		static const char my_server_xml[] =
			"<node>"
			"  <interface name='" MGDBUS_SERVER_NAME "'>"
			"    <annotation name='org.gtk.GDBus.Annotation' value='OnInterface'/>"
			"    <annotation name='org.gtk.GDBus.Annotation' value='AlsoOnInterface'/>"
			"    <method name='New'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"    <method name='Open'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='url' direction='in'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"    <method name='Connect'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"    <method name='Exec'>"
			"      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>"
			"      <arg type='s' name='command' direction='in'/>"
			"      <arg type='s' name='result' direction='out'/>"
			"    </method>"
			"  </interface>"
			"</node>";

		mIntrospectionData = g_dbus_node_info_new_for_xml(my_server_xml, nullptr);
		if (mIntrospectionData == nullptr)
			throw runtime_error("failed to parse introspection data");

		mOwnerId = g_bus_own_name(G_BUS_TYPE_SESSION, MGDBUS_SERVER_NAME,
			G_BUS_NAME_OWNER_FLAGS_NONE, HandleBusAcquired, HandleNameAcquired, HandleNameLost,
			this, nullptr);
	}

	~MGDbusServer()
	{
		if (mWatcherID != 0)
			g_bus_unwatch_name(mWatcherID);

		if (mOwnerId != 0)
			g_bus_unown_name(mOwnerId);

		if (mIntrospectionData != nullptr)
			g_dbus_node_info_unref(mIntrospectionData);
	}

	bool IsServer() const
	{
		return mOwnerId != 0 and mRegistrationID != 0;
	}

  private:
	void BusAcquired(GDBusConnection *connection, const char *name)
	{
		mRegistrationID = g_dbus_connection_register_object(
			connection, MGDBUS_SERVER_OBJECT_NAME,
			mIntrospectionData->interfaces[0],
			&sInterfaceVTable, this, nullptr, nullptr);
		assert(mRegistrationID > 0);
	}

	void NameAcquired(GDBusConnection *connection, const char *name)
	{
		if (mArguments.empty() or mArguments.front() == "New")
			gApp->DoNew();
		else if (mArguments.front() == "Connect")
			gApp->ProcessCommand('Conn', nullptr, 0, 0);
		else if (mArguments.front() == "Exec")
			gApp->Open(mArguments.back());
		else
			gApp->Open(mArguments.back());
	}

	void NameLost(GDBusConnection *connection, const char *name)
	{
		mWatcherID = g_bus_watch_name(G_BUS_TYPE_SESSION, MGDBUS_SERVER_NAME,
			G_BUS_NAME_WATCHER_FLAGS_NONE, HandleNameAppeared, HandleNameVanished,
			this, nullptr);
	}

	void NameAppeared(GDBusConnection *connection, const gchar *name, const char *name_owner)
	{
		g_dbus_connection_call(
			connection,
			MGDBUS_SERVER_NAME, MGDBUS_SERVER_OBJECT_NAME, MGDBUS_SERVER_NAME,
			mArguments.front().c_str(),
			mArguments.size() > 1 ? g_variant_new("(s)", mArguments.back().c_str()) : nullptr,
			nullptr,
			G_DBUS_CALL_FLAGS_NONE,
			-1,
			nullptr,
			&HandleAsyncReady,
			nullptr);
	}

	void AsyncReady(GDBusConnection *connection, GVariant *result, GError *error)
	{
		//		PRINT(("Ready, result is %s", result ? "not null" : "null"));
		if (error)
		{
			cerr << error->message << endl;
			//			PRINT(("Error: '%s'", error->message));
			g_error_free(error);
		}
	}

	void NameVanished(GDBusConnection *connection, const gchar *name)
	{
		PRINT(("Name Vanished: %s", name));
		// something fishy... just open
		// if (mOpenParameter.empty())
		// 	gApp->DoNew();
		// else
		// 	gApp->Open(mOpenParameter);
	}

	void MethodCall(GDBusConnection *connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation)
	{
		PRINT(("MethodCall: %s", method_name));
		try
		{
			if (strcmp(method_name, "Open") == 0)
			{
				const gchar *gurl;
				g_variant_get(parameters, "(&s)", &gurl);
				string url(gurl ? gurl : "");

				gApp->Open(url);
			}
			else if (strcmp(method_name, "Connect") == 0)
				gApp->ProcessCommand('Conn', nullptr, 0, 0);
			else if (strcmp(method_name, "New") == 0)
				gApp->DoNew();
			else if (strcmp(method_name, "Exec") == 0)
			{
				const gchar *gcommand;
				g_variant_get(parameters, "(&s)", &gcommand);
				string command(gcommand ? gcommand : "");

				gApp->Open(command);
			}
			else
				throw runtime_error("unimplemented DBus Method");

			g_dbus_method_invocation_return_value(invocation,
				g_variant_new("(s)", "ok"));
		}
		catch (const exception &e)
		{
			g_dbus_method_invocation_return_value(invocation, g_variant_new("(s)", e.what()));
		}
	}

	static void HandleBusAcquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->BusAcquired(connection, name);
	}

	static void HandleNameAcquired(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameAcquired(connection, name);
	}

	static void HandleNameLost(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameLost(connection, name);
	}

	static void HandleMethodCall(GDBusConnection *connection, const gchar *sender,
		const gchar *object_path, const gchar *interface_name,
		const gchar *method_name, GVariant *parameters,
		GDBusMethodInvocation *invocation, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->MethodCall(connection, sender, object_path, interface_name, method_name, parameters, invocation);
	}

	static void HandleNameAppeared(GDBusConnection *connection, const gchar *name, const char *name_owner, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameAppeared(connection, name, name_owner);
	}

	static void HandleNameVanished(GDBusConnection *connection, const gchar *name, gpointer user_data)
	{
		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->NameVanished(connection, name);
	}

	static void HandleAsyncReady(GObject *source_object, GAsyncResult *result, gpointer user_data)
	{
		GError *error = nullptr;
		GVariant *var = g_dbus_connection_call_finish((GDBusConnection *)source_object, result, &error);

		MGDbusServer *server = reinterpret_cast<MGDbusServer *>(user_data);
		server->AsyncReady((GDBusConnection *)source_object, var, error);
	}

	static const GDBusInterfaceVTable sInterfaceVTable;

	GDBusNodeInfo *mIntrospectionData;
	uint32_t mOwnerId, mRegistrationID, mWatcherID;
	std::vector<std::string> mArguments;
};

const GDBusInterfaceVTable MGDbusServer::sInterfaceVTable = {
	MGDbusServer::HandleMethodCall, nullptr, nullptr, {}
};

// ----------------------------------------------------------------------------
//	Main routines

void my_signal_handler(int inSignal)
{
	switch (inSignal)
	{
		case SIGPIPE:
			break;

		case SIGUSR1:
			break;

		case SIGINT:
			//			gQuit = true;
			gApp->DoQuit();
			break;

		case SIGTERM:
			//			gQuit = true;
			gApp->DoQuit();
			break;
	}
}

int MApplication::Main(std::initializer_list<std::string> argv)
{
	setenv("UBUNTU_MENUPROXY", "0", true);

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

		gtk_init(0, nullptr);

		MGDbusServer dBusServer(argv);

		unique_ptr<MApplication> app(MApplication::Create(
			new MGtkApplicationImpl));

		app->Initialise();

		struct sigaction act, oact;
		act.sa_handler = my_signal_handler;
		sigemptyset(&act.sa_mask);
		act.sa_flags = 0;
		::sigaction(SIGTERM, &act, &oact);
		::sigaction(SIGUSR1, &act, &oact);
		::sigaction(SIGPIPE, &act, &oact);
		::sigaction(SIGINT, &act, &oact);

		app->RunEventLoop();
	}
	catch (exception &e)
	{
		cerr << e.what() << endl;
	}
	catch (...)
	{
		cerr << "Exception caught" << endl;
	}

	return 0;
}

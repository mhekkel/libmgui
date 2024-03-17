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

#include <zeep/xml/document.hpp>

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
	MGDbusServer(const std::string &method, const std::vector<std::string> &arg)
		: mIntrospectionData(nullptr)
		, mOwnerId(0)
		, mRegistrationID(0)
		, mWatcherID(0)
		, mMethod(method)
		, mArguments(arg)
	{
		using namespace zeep::xml::literals;

		auto my_server = R"(
<node>
  <interface>
    <annotation name='org.gtk.GDBus.Annotation' value='OnInterface'/>
    <annotation name='org.gtk.GDBus.Annotation' value='AlsoOnInterface'/>
    <method name='New'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
    </method>
    <method name='Open'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <arg type='s' name='url' direction='in'/>
    </method>
    <method name='Connect'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
    </method>
    <method name='Execute'>
      <annotation name='org.gtk.GDBus.Annotation' value='OnMethod'/>
      <arg type='as' name='argv' direction='in'/>
    </method>
  </interface>
</node>)"_xml;

		my_server.find_first("/node/interface")->set_attribute("name", MGDBUS_SERVER_NAME);
		auto my_server_xml = (std::ostringstream() << my_server).str();

		mIntrospectionData = g_dbus_node_info_new_for_xml(my_server_xml.c_str(), nullptr);
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
		gApp->Execute(mMethod, mArguments);
	}

	void NameLost(GDBusConnection *connection, const char *name)
	{
		mWatcherID = g_bus_watch_name(G_BUS_TYPE_SESSION, MGDBUS_SERVER_NAME,
			G_BUS_NAME_WATCHER_FLAGS_NONE, HandleNameAppeared, HandleNameVanished,
			this, nullptr);
	}

	void NameAppeared(GDBusConnection *connection, const gchar *name, const char *name_owner)
	{
		GVariant *params = nullptr;

		if (mMethod == "Open")
		{
			// GVariantDict d;
			// g_variant_dict_init(&d, nullptr);
			// g_variant_dict_insert(&d, "url", "(s)", mArguments.front().c_str());
			// params = g_variant_dict_end(&d);
			params = g_variant_new("(s)", mArguments.front().c_str());
		}
		else if (mMethod == "Execute")
		{
			GVariantBuilder builder;

			g_variant_builder_init(&builder, G_VARIANT_TYPE_STRING_ARRAY);
			for (auto &a : mArguments)
				g_variant_builder_add(&builder, "s", a.c_str());

			auto p = g_variant_builder_end(&builder);

			params = g_variant_new("(@as)", p);
		}

		g_dbus_connection_call(
			connection,
			MGDBUS_SERVER_NAME, MGDBUS_SERVER_OBJECT_NAME, MGDBUS_SERVER_NAME,
			mMethod.c_str(),
			params,
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
			cerr << error->message << '\n';
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
				std::string url(gurl ? gurl : "");

				gApp->Execute("Open", { url });
			}
			else if (strcmp(method_name, "Connect") == 0)
				gApp->Execute("Connect", {});
			else if (strcmp(method_name, "New") == 0)
				gApp->Execute("New", {});
			else if (strcmp(method_name, "Execute") == 0)
			{
				GVariantIter *iter;
				gchar *str;

				std::vector<std::string> args;

				g_variant_get(parameters, "(as)", &iter);
				while (g_variant_iter_loop(iter, "s", &str))
					args.emplace_back(str);
				g_variant_iter_free(iter);

				gApp->Execute("Execute", args);
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
	std::string mMethod;
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

int MApplication::Main(const std::string &cmd, const std::vector<std::string> &argv)
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

		MGDbusServer dBusServer(cmd, argv);

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
		cerr << e.what() << '\n';
	}
	catch (...)
	{
		cerr << "Exception caught\n";
	}

	return 0;
}
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

#include "Gtk/MGtkWindowImpl.hpp"

#include "MAlerts.hpp"
#include "MError.hpp"
#include "MStrings.hpp"

#include "mrsrc.hpp"

#include <zeep/xml/document.hpp>

namespace xml = zeep::xml;

GtkWidget *CreateAlertWithArgs(const char *inResourceName, std::initializer_list<std::string> inArgs)
{
	using namespace std::literals;

	GtkWidget *dlg = nullptr;

	mrsrc::istream rsrc("Alerts/"s + inResourceName + ".xml");
	if (not rsrc)
		THROW(("Could not load resource Alerts/%s.xml", inResourceName));

	// parse the XML data
	xml::document doc(rsrc);

	// build an alert
	xml::element *root = doc.find_first("/alert");

	if (root->name() != "alert")
		THROW(("Invalid resource for alert %s, first tag should be <alert>", inResourceName));

	std::string instruction, content;
	std::vector<std::pair<std::string, uint32_t>> btns;
	int32_t defaultButton = -1;
	GtkMessageType type = GTK_MESSAGE_INFO;

	if (root->get_attribute("type") == "warning")
		type = GTK_MESSAGE_WARNING;
	else if (root->get_attribute("type") == "error")
		type = GTK_MESSAGE_ERROR;

	for (auto item : *root)
	{
		if (item.name() == "content")
		{
			// replace parameters
			char s[] = "^0";
			std::string text = GetLocalisedStringForContext(inResourceName, item.get_content());

			for (auto a : inArgs)
			{
				std::string::size_type p = text.find(s);
				if (p != std::string::npos)
					text.replace(p, 2, a);
				++s[1];
			}

			content = text;
		}
		else if (item.name() == "instruction")
		{
			// replace parameters
			char s[] = "^0";
			std::string text = GetLocalisedStringForContext(inResourceName, item.get_content());

			for (auto a : inArgs)
			{
				zeep::replace_all(text, s, a);
				++s[1];
			}

			instruction = text;
		}
		else if (item.name() == "buttons")
		{
			for (auto button : item)
			{
				if (button.name() == "button")
				{
					std::string label = GetLocalisedStringForContext(inResourceName, button.get_attribute("title"));
					uint32_t cmd = stoul(button.get_attribute("cmd"));

					if (button.get_attribute("default") == "true")
						defaultButton = cmd;

					btns.push_back(std::make_pair(label, cmd));
				}
			}
		}
	}

	dlg = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
		type, GTK_BUTTONS_NONE, "%s", instruction.c_str());

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-security"
	THROW_IF_NIL(dlg);
	if (not content.empty())
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dlg), content.c_str());
#pragma GCC diagnostic pop

	for (auto b : btns)
		gtk_dialog_add_button(GTK_DIALOG(dlg), b.first.c_str(), b.second);

	if (defaultButton >= 0)
		gtk_dialog_set_default_response(GTK_DIALOG(dlg), defaultButton);

	return dlg;
}

int32_t DisplayAlert(MWindow *inParent, const std::string &inResourceName, std::initializer_list<std::string> inArgs)
{
	int32_t result = -1;

	try
	{
		GtkWidget *dlg = CreateAlertWithArgs(inResourceName.c_str(), inArgs);

		if (inParent != nullptr)
		{
			inParent->Select();

			MGtkWindowImpl *impl =
				static_cast<MGtkWindowImpl *>(inParent->GetImpl());

			gtk_window_set_transient_for(
				GTK_WINDOW(dlg),
				GTK_WINDOW(impl->GetWidget()));
		}
		else if (MWindow::GetFirstWindow() != nullptr)
		{
			MWindow::GetFirstWindow()->Select();

			MGtkWindowImpl *impl =
				static_cast<MGtkWindowImpl *>(MWindow::GetFirstWindow()->GetImpl());

			gtk_window_set_transient_for(
				GTK_WINDOW(dlg),
				GTK_WINDOW(impl->GetWidget()));
		}

#warning FIXME
		// result = gtk_dialog_run(GTK_DIALOG(dlg));

		// gtk_widget_destroy(dlg);
	}
	catch (const std::exception &e)
	{
		DisplayError(e);
	}

	return result;
}

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

#include "Gtk/MPrimary.hpp"
#include "Gtk/MGtkLib.hpp"
#include "Gtk/MGtkWidgetMixin.hpp"

#include "MError.hpp"
#include "MTypes.hpp"
#include "MUnicode.hpp"

// --------------------------------------------------------------------

// struct MPrimaryImpl
// {
// 	MPrimaryImpl();
// 	~MPrimaryImpl();

// 	bool HasText();
// 	void GetText(std::string &outText);
// 	void SetText(const std::string &inText);
// 	void SetText(std::function<void(std::string &)> provider);
// 	void LoadClipboardIfNeeded();

// 	static void GtkClipboardGet(GtkClipboard *inClipboard, GtkSelectionData *inSelectionData, guint inInfo, gpointer inUserDataOrOwner);
// 	static void GtkClipboardClear(GtkClipboard *inClipboard, gpointer inUserDataOrOwner);
// 	void OnOwnerChange(GdkEventOwnerChange *inEvent);

// 	std::string mText;
// 	std::function<void(std::string &)> mProvider;
// 	MSlot<void(GdkEventOwnerChange *)> mOwnerChange;
// 	GtkClipboard *mGtkClipboard;
// 	bool mClipboardIsMine;
// 	bool mOwnerChanged;
// 	bool mSettingOwner;
// };

// MPrimaryImpl::MPrimaryImpl()
// 	: mOwnerChange(this, &MPrimaryImpl::OnOwnerChange)
// 	, mGtkClipboard(gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_PRIMARY))
// 	, mClipboardIsMine(false)
// 	, mOwnerChanged(true)
// {
// 	mOwnerChange.Connect(G_OBJECT(mGtkClipboard), "owner-change");
// }

// MPrimaryImpl::~MPrimaryImpl()
// {
// 	if (mClipboardIsMine)
// 		gtk_clipboard_store(mGtkClipboard);
// }

// void MPrimaryImpl::GtkClipboardGet(
// 	GtkClipboard *inClipboard,
// 	GtkSelectionData *inSelectionData,
// 	guint inInfo,
// 	gpointer inUserDataOrOwner)
// {
// 	MPrimaryImpl *self = reinterpret_cast<MPrimaryImpl *>(inUserDataOrOwner);

// 	if (self->mText.empty() and self->mProvider)
// 	{
// 		self->mProvider(self->mText);
// 		self->mProvider = {};
// 	}

// 	gtk_selection_data_set_text(inSelectionData, self->mText.c_str(), self->mText.length());
// }

// void MPrimaryImpl::GtkClipboardClear(
// 	GtkClipboard *inClipboard,
// 	gpointer inUserDataOrOwner)
// {
// 	MPrimaryImpl *self = reinterpret_cast<MPrimaryImpl *>(inUserDataOrOwner);

// 	if (self->mClipboardIsMine and not self->mSettingOwner)
// 	{
// 		self->mOwnerChanged = true;
// 		self->mClipboardIsMine = false;
// 		self->mText.clear();
// 		self->mProvider = {};
// 	}
// }

// void MPrimaryImpl::OnOwnerChange(
// 	GdkEventOwnerChange *inEvent)
// {
// 	if (not mClipboardIsMine and not mSettingOwner)
// 	{
// 		mOwnerChanged = true;
// 		mText.clear();
// 		mProvider = {};
// 	}
// }

// void MPrimaryImpl::LoadClipboardIfNeeded()
// {
// 	if (not mClipboardIsMine and
// 		mOwnerChanged and
// 		gtk_clipboard_wait_is_text_available(mGtkClipboard))
// 	{
// 		gchar *text = gtk_clipboard_wait_for_text(mGtkClipboard);
// 		if (text != nullptr)
// 		{
// 			SetText(std::string(text));
// 			g_free(text);
// 		}
// 		mOwnerChanged = false;
// 	}
// }

// bool MPrimaryImpl::HasText()
// {
// 	LoadClipboardIfNeeded();
// 	return not(mText.empty() and not mProvider);
// }

// void MPrimaryImpl::GetText(std::string &outText)
// {
// 	if (not mText.empty())
// 		outText = mText;
// 	else if (mProvider)
// 		mProvider(outText);
// }

// void MPrimaryImpl::SetText(const std::string &inText)
// {
// 	mText = inText;
// 	mProvider = {};

// 	GtkTargetEntry targets[] = {
// 		{ const_cast<gchar *>("UTF8_STRING"), 0, 0 },
// 		{ const_cast<gchar *>("COMPOUND_TEXT"), 0, 0 },
// 		{ const_cast<gchar *>("TEXT"), 0, 0 },
// 		{ const_cast<gchar *>("STRING"), 0, 0 },
// 	};

// 	mSettingOwner = true;
// 	gtk_clipboard_set_with_data(mGtkClipboard,
// 		targets, sizeof(targets) / sizeof(GtkTargetEntry),
// 		&MPrimaryImpl::GtkClipboardGet, &MPrimaryImpl::GtkClipboardClear, this);

// 	mSettingOwner = false;
// 	mOwnerChanged = false;
// 	mClipboardIsMine = true;
// }

// void MPrimaryImpl::SetText(std::function<void(std::string &)> provider)
// {
// 	SetText(std::string(""));
// 	mProvider = provider;
// }

// --------------------------------------------------------------------

MPrimary &MPrimary::Instance()
{
	static MPrimary sInstance;
	return sInstance;
}

#warning FIXME
MPrimary::MPrimary()
	: mImpl(nullptr/* new MPrimaryImpl */)
{
}

MPrimary::~MPrimary()
{
	delete mImpl;
}

bool MPrimary::HasText()
{
	// return mImpl->HasText();
	return false;
}

void MPrimary::GetText(std::string &text)
{
	// mImpl->GetText(text);
}

void MPrimary::SetText(const std::string &text)
{
	// mImpl->SetText(text);
}

void MPrimary::SetText(std::function<void(std::string &)> provider)
{
	// mImpl->SetText(provider);
}

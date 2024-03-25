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

#include "MPrimary.hpp"
#include "MGtkLib.hpp"
#include "MGtkWidgetMixin.hpp"

#include "MError.hpp"
#include "MTypes.hpp"
#include "MUnicode.hpp"

// --------------------------------------------------------------------

struct MPrimaryImpl
{
	MPrimaryImpl();
	~MPrimaryImpl();

	bool HasText();
	std::string GetText();
	void SetText(const std::string &inText);
	void LoadClipboardIfNeeded();

	// static void GdkClipboardGet(GdkClipboard *inClipboard, GtkSelectionData *inSelectionData, guint inInfo, gpointer inUserDataOrOwner);
	// static void GdkClipboardClear(GdkClipboard *inClipboard, gpointer inUserDataOrOwner);
	void OnOwnerChange();

	std::string mText;
	MSlot<void()> mOwnerChange;
	GdkClipboard *mGdkClipboard;
	bool mClipboardIsMine;
	bool mOwnerChanged;
	bool mSettingOwner;
};

MPrimaryImpl::MPrimaryImpl()
	: mOwnerChange(this, &MPrimaryImpl::OnOwnerChange)
	, mGdkClipboard(gdk_display_get_primary_clipboard(gdk_display_get_default()))
	, mClipboardIsMine(false)
	, mOwnerChanged(true)
{
	mOwnerChange.Connect(G_OBJECT(mGdkClipboard), "owner-change");
}

MPrimaryImpl::~MPrimaryImpl()
{
	// if (mClipboardIsMine)
	// 	gtk_clipboard_store(mGdkClipboard);
}

// void MPrimaryImpl::GdkClipboardGet(GdkClipboard *inClipboard, GtkSelectionData *inSelectionData, guint inInfo, gpointer inUserDataOrOwner)
// {
// 	MPrimaryImpl *self = reinterpret_cast<MPrimaryImpl *>(inUserDataOrOwner);

// 	if (self->mText.empty() and self->mProvider)
// 	{
// 		self->mProvider(self->mText);
// 		self->mProvider = {};
// 	}

// 	gtk_selection_data_set_text(inSelectionData, self->mText.c_str(), self->mText.length());
// }

// void MPrimaryImpl::GdkClipboardClear(GdkClipboard *inClipboard, gpointer inUserDataOrOwner)
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

void MPrimaryImpl::OnOwnerChange()
{
	if (not mClipboardIsMine and not mSettingOwner)
	{
		mOwnerChanged = true;
		mText.clear();
	}
}

void MPrimaryImpl::LoadClipboardIfNeeded()
{
#warning FIXME
	// if (not mClipboardIsMine and
	// 	mOwnerChanged and
	// 	gtk_clipboard_wait_is_text_available(mGdkClipboard))
	// {
	// 	gchar *text = gtk_clipboard_wait_for_text(mGdkClipboard);
	// 	if (text != nullptr)
	// 	{
	// 		SetText(std::string(text));
	// 		g_free(text);
	// 	}
	// 	mOwnerChanged = false;
	// }
}

bool MPrimaryImpl::HasText()
{
	LoadClipboardIfNeeded();
	return not mText.empty();
}

std::string MPrimaryImpl::GetText()
{
	return mText;
}

void MPrimaryImpl::SetText(const std::string &inText)
{
	mText = inText;

	gdk_clipboard_set_text(mGdkClipboard, inText.c_str());

	// GtkTargetEntry targets[] = {
	// 	{ const_cast<gchar *>("UTF8_STRING"), 0, 0 }, 	{ const_cast<gchar *>("COMPOUND_TEXT"), 0, 0 }, 	{ const_cast<gchar *>("TEXT"), 0, 0 }, 	{ const_cast<gchar *>("STRING"), 0, 0 }, };

	// mSettingOwner = true;
	// gtk_clipboard_set_with_data(mGdkClipboard, 	targets, sizeof(targets) / sizeof(GtkTargetEntry), 	&MPrimaryImpl::GdkClipboardGet, &MPrimaryImpl::GdkClipboardClear, this);

	mSettingOwner = false;
	mOwnerChanged = false;
	mClipboardIsMine = true;
}

// --------------------------------------------------------------------

MPrimary &MPrimary::Instance()
{
	static MPrimary sInstance;
	return sInstance;
}

MPrimary::MPrimary()
	: mImpl(new MPrimaryImpl)
{
}

MPrimary::~MPrimary()
{
	delete mImpl;
}

bool MPrimary::HasText()
{
	return mImpl->HasText();
}

std::string MPrimary::GetText()
{
	return mImpl->GetText();
}

void MPrimary::SetText(const std::string &text)
{
	mImpl->SetText(text);
}

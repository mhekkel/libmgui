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

#include "MClipboardImpl.hpp"

#include "Gtk/MGtkWidgetMixin.hpp"

#include "MUtils.hpp"

using namespace std;

class MGtkClipboardImpl : public MClipboardImpl
{
  public:
	MGtkClipboardImpl(MClipboard *inClipboard);

	virtual void LoadClipboardIfNeeded();
	virtual void Reset();
	virtual void Commit();

	void OnOwnerChange(GdkEventOwnerChange *inEvent)
	{
		if (not mClipboardIsMine)
			mOwnerChanged = true;
	}

	static void GtkClipboardGet(GtkClipboard *inClipboard, GtkSelectionData *inSelectionData,
		guint inInfo, gpointer inUserDataOrOwner);
	static void GtkClipboardClear(GtkClipboard *inClipboard, gpointer inUserDataOrOwner);

	MSlot<void(GdkEventOwnerChange *)> mOwnerChange;
	GtkClipboard *mGtkClipboard;
	bool mClipboardIsMine;
	bool mOwnerChanged;
	bool mNoCommit;
};

MGtkClipboardImpl::MGtkClipboardImpl(MClipboard *inClipboard)
	: MClipboardImpl(inClipboard)
	, mOwnerChange(this, &MGtkClipboardImpl::OnOwnerChange)
	, mGtkClipboard(gtk_clipboard_get_for_display(gdk_display_get_default(), GDK_SELECTION_CLIPBOARD))
	, mClipboardIsMine(false)
	, mOwnerChanged(true)
	, mNoCommit(false)
{
	mOwnerChange.Connect(G_OBJECT(mGtkClipboard), "owner-change");
}

void MGtkClipboardImpl::LoadClipboardIfNeeded()
{
	if (not mClipboardIsMine and
		mOwnerChanged and
		gtk_clipboard_wait_is_text_available(mGtkClipboard))
	{
		mOwnerChanged = false;

		// cout << "Reloading clipboard\n";
		gchar *text = gtk_clipboard_wait_for_text(mGtkClipboard);
		if (text != nullptr)
		{
			mNoCommit = true;

			mClipboard->SetData(text, false);

			mNoCommit = false;

			g_free(text);
		}
	}
}

void MGtkClipboardImpl::Reset()
{
}

void MGtkClipboardImpl::Commit()
{
	if (not mNoCommit)
	{
		GtkTargetEntry targets[] = {
			{ const_cast<gchar *>("UTF8_STRING"), 0, 0 },
			{ const_cast<gchar *>("COMPOUND_TEXT"), 0, 0 },
			{ const_cast<gchar *>("TEXT"), 0, 0 },
			{ const_cast<gchar *>("STRING"), 0, 0 },
		};

		gtk_clipboard_set_with_data(mGtkClipboard,
			targets, sizeof(targets) / sizeof(GtkTargetEntry),
			&MGtkClipboardImpl::GtkClipboardGet, &MGtkClipboardImpl::GtkClipboardClear, this);

		//	gtk_clipboard_set_text(mGtkClipboard, inText.c_str(), inText.length());

		mOwnerChanged = false;
		mClipboardIsMine = true;
	}
}

void MGtkClipboardImpl::GtkClipboardGet(GtkClipboard *inClipboard, GtkSelectionData *inSelectionData,
	guint inInfo, gpointer inUserDataOrOwner)
{
	string text;
	bool block;

	MClipboard::Instance().GetData(text, block);

	gtk_selection_data_set_text(inSelectionData, text.c_str(), text.length());
}

void MGtkClipboardImpl::GtkClipboardClear(GtkClipboard *inClipboard, gpointer inUserDataOrOwner)
{
	MGtkClipboardImpl *self = reinterpret_cast<MGtkClipboardImpl *>(inUserDataOrOwner);

	self->mOwnerChanged = true;
	self->mClipboardIsMine = false;
}

MClipboardImpl *MClipboardImpl::Create(MClipboard *inClipboard)
{
	return new MGtkClipboardImpl(inClipboard);
}

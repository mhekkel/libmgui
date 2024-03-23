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

#include "Gtk/MGtkWidgetMixin.hpp"

#include <cassert>

using namespace std;

MGtkWidgetMixin::MGtkWidgetMixin()
	: mWidget(nullptr)
	, mDestroy(this, &MGtkWidgetMixin::OnDestroy)
	, mDirectionChanged(this, &MGtkWidgetMixin::OnDirectionChanged)
	, mHide(this, &MGtkWidgetMixin::OnHide)
	, mKeynavFailed(this, &MGtkWidgetMixin::OnKeynavFailed)
	, mMap(this, &MGtkWidgetMixin::OnMap)
	, mMnemonicActivate(this, &MGtkWidgetMixin::OnMnemonicActivate)
	, mMoveFocus(this, &MGtkWidgetMixin::OnMoveFocus)
	, mQueryTooltip(this, &MGtkWidgetMixin::OnQueryTooltip)
	, mRealize(this, &MGtkWidgetMixin::OnRealize)
	, mShow(this, &MGtkWidgetMixin::OnShow)
	, mStateFlagsChanged(this, &MGtkWidgetMixin::OnStateFlagsChanged)
	, mUnmap(this, &MGtkWidgetMixin::OnUnmap)
	, mUnrealize(this, &MGtkWidgetMixin::OnUnrealize)

	, mCommit(this, &MGtkWidgetMixin::OnCommit)
	, mDeleteSurrounding(this, &MGtkWidgetMixin::OnDeleteSurrounding)
	, mPreeditChanged(this, &MGtkWidgetMixin::OnPreeditChanged)
	, mPreeditEnd(this, &MGtkWidgetMixin::OnPreeditEnd)
	, mPreeditStart(this, &MGtkWidgetMixin::OnPreeditStart)
	, mRetrieveSurrounding(this, &MGtkWidgetMixin::OnRetrieveSurrounding)

	, mRequestedWidth(-1)
	, mRequestedHeight(-1)
	, mAutoRepeat(false)
	, mIMContext(nullptr)
	, mNextKeyPressIsAutoRepeat(false)
{
}

MGtkWidgetMixin::~MGtkWidgetMixin()
{
	// if (mWidget != nullptr)
	// 	PRINT(("mWidget != null!"));
}

void MGtkWidgetMixin::SetWidget(GtkWidget *inWidget)
{
	mWidget = inWidget;

	if (inWidget != nullptr)
	{
		mDestroy.Connect(inWidget, "destroy");
		mDirectionChanged.Connect(inWidget, "direction-changed");
		mHide.Connect(inWidget, "hide");
		mKeynavFailed.Connect(inWidget, "keynav-failed");
		mMap.Connect(inWidget, "map");
		mMnemonicActivate.Connect(inWidget, "mnemonic-activate");
		mMoveFocus.Connect(inWidget, "move-focus");
		mQueryTooltip.Connect(inWidget, "query-tooltip");
		mRealize.Connect(inWidget, "realize");
		mShow.Connect(inWidget, "show");
		mStateFlagsChanged.Connect(inWidget, "state-flags-changed");
		mUnmap.Connect(inWidget, "unmap");
		mUnrealize.Connect(inWidget, "unrealize");

		if (mRequestedWidth >= 0 or mRequestedHeight >= 0)
			gtk_widget_set_size_request(inWidget, mRequestedWidth, mRequestedHeight);
	}
}

void MGtkWidgetMixin::CreateIMContext()
{
	if (mIMContext == nullptr)
	{
		mIMContext = gtk_im_context_simple_new();

		mCommit.Connect(G_OBJECT(mIMContext), "commit");
		mDeleteSurrounding.Connect(G_OBJECT(mIMContext), "delete-surrounding");
		mPreeditChanged.Connect(G_OBJECT(mIMContext), "preedit-changed");
		mPreeditStart.Connect(G_OBJECT(mIMContext), "preedit-start");
		mPreeditEnd.Connect(G_OBJECT(mIMContext), "preedit-end");
		mRetrieveSurrounding.Connect(G_OBJECT(mIMContext), "retrieve-surrounding");
	}
}

void MGtkWidgetMixin::OnDestroy()
{
	SetWidget(nullptr);
}

void MGtkWidgetMixin::OnDirectionChanged(GtkTextDirection previous_direction)
{

}

void MGtkWidgetMixin::OnHide()
{

}

bool MGtkWidgetMixin::OnKeynavFailed(GtkDirectionType direction)
{
	return false;
}

void MGtkWidgetMixin::OnMap()
{

}

bool MGtkWidgetMixin::OnMnemonicActivate(gboolean group_cycling)
{
	return false;
}

void MGtkWidgetMixin::OnMoveFocus(GtkDirectionType direction)
{

}

bool MGtkWidgetMixin::OnQueryTooltip(gint x, gint y, gboolean keyboard_mode, GtkTooltip* tooltip)
{
	return false;
}

void MGtkWidgetMixin::OnRealize()
{
	if (mIMContext)
		gtk_im_context_set_client_widget(mIMContext, mWidget);
}

void MGtkWidgetMixin::OnShow()
{

}

void MGtkWidgetMixin::OnStateFlagsChanged(GtkStateFlags flags)
{
	if (flags & GTK_STATE_FLAG_FOCUSED)
	{
		if (not mGainedFocusAt.has_value())
		{
			mGainedFocusAt = std::chrono::steady_clock::now();

			if (mIMContext)
				gtk_im_context_focus_in(mIMContext);
		}
	}
	else if (mGainedFocusAt.has_value())
	{
		mGainedFocusAt.reset();

		if (mIMContext)
			gtk_im_context_focus_out(mIMContext);
	}
}

void MGtkWidgetMixin::OnUnmap()
{

}

void MGtkWidgetMixin::OnUnrealize()
{

}

void MGtkWidgetMixin::RequestSize(int32_t inWidth, int32_t inHeight)
{
	mRequestedWidth = inWidth;
	mRequestedHeight = inHeight;

	if (GTK_IS_WIDGET(mWidget) and (mRequestedWidth >= 0 or mRequestedHeight >= 0))
		gtk_widget_set_size_request(mWidget, mRequestedWidth, mRequestedHeight);
}

void MGtkWidgetMixin::Append(MGtkWidgetMixin *inChild, bool inExpand, MRect inMargins)
{
	assert(false);
}

void MGtkWidgetMixin::SetFocus()
{
	if (gtk_widget_get_can_focus(mWidget))
		gtk_widget_grab_focus(mWidget);
}

void MGtkWidgetMixin::ReleaseFocus()
{
	// PRINT(("MGtkWidgetMixin::ReleaseFocus"));
}

bool MGtkWidgetMixin::IsFocus() const
{
	return gtk_widget_has_focus(mWidget);
}

bool MGtkWidgetMixin::IsActive() const
{
	return gtk_widget_has_focus(mWidget);
}

void MGtkWidgetMixin::OnCommit(char *inText)
{
}

bool MGtkWidgetMixin::OnDeleteSurrounding(gint inStart, gint inLength)
{
	return false;
}

void MGtkWidgetMixin::OnPreeditChanged()
{
}

void MGtkWidgetMixin::OnPreeditEnd()
{
}

void MGtkWidgetMixin::OnPreeditStart()
{
}

bool MGtkWidgetMixin::OnRetrieveSurrounding()
{
	return false;
}
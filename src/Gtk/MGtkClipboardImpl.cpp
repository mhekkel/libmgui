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

#include "MClipboard.hpp"
#include "MGtkWidgetMixin.hpp"
#include "MUtils.hpp"

// --------------------------------------------------------------------

class MGdkClipboardImpl : public MClipboardImpl
{
  public:
	MGdkClipboardImpl(MClipboard *inClipboard, bool inPrimary);

	bool HasData() override;
	void GetData(MClipboardGetDataHandlerbase *inHandler) override;
	void SetData(const std::string &inData) override;

	static void GetDataResult(GdkClipboard *source_object, GAsyncResult *res,
		MClipboardGetDataHandlerbase *handler);

	GdkClipboard *mGdkClipboard;
	bool mIsPrimary;
};

MGdkClipboardImpl::MGdkClipboardImpl(MClipboard *inClipboard, bool inPrimary)
	: MClipboardImpl(inClipboard)
	, mIsPrimary(inPrimary)
{
	auto display = gdk_display_get_default();

	if (inPrimary)
		mGdkClipboard = gdk_display_get_primary_clipboard(display);
	else
		mGdkClipboard = gdk_display_get_clipboard(display);
}

bool MGdkClipboardImpl::HasData()
{
	bool result = false;

	if (auto content = gdk_clipboard_get_formats(mGdkClipboard); content)
		result = gdk_content_formats_contain_gtype(content, G_TYPE_STRING);

	return result;
}

void MGdkClipboardImpl::GetDataResult(GdkClipboard *source_object, GAsyncResult *res,
		MClipboardGetDataHandlerbase *handler)
{
	auto text = gdk_clipboard_read_text_finish(source_object, res, nullptr);

	if (text)
	{
		handler->HandleData({text});
		g_free(text);
	}

	delete handler;
}

void MGdkClipboardImpl::GetData(MClipboardGetDataHandlerbase *inHandler)
{
	gdk_clipboard_read_text_async(mGdkClipboard, nullptr,
		GAsyncReadyCallback(&MGdkClipboardImpl::GetDataResult), inHandler);
}

void MGdkClipboardImpl::SetData(const std::string &inData)
{
	gdk_clipboard_set_text(mGdkClipboard, inData.c_str());
}

// --------------------------------------------------------------------

MClipboardImpl *MClipboardImpl::Create(MClipboard *inClipboard, bool inPrimary)
{
	return new MGdkClipboardImpl(inClipboard, inPrimary);
}

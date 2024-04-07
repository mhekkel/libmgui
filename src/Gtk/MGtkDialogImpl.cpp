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

#include "MGtkApplicationImpl.hpp"
#include "MGtkControlsImpl.hpp"
#include "MGtkWindowImpl.hpp"

#include "MCanvas.hpp"
#include "MControls.hpp"
#include "MDevice.hpp"
#include "MDialog.hpp"
#include "MError.hpp"
#include "MStrings.hpp"
#include "MUtils.hpp"

#include "mrsrc.hpp"

#include <charconv>

#include <mxml/document.hpp>

class MGtkDialogImpl : public MGtkWindowImpl
{
  public:
	MGtkDialogImpl(MWindow *inParent)
		: MGtkWindowImpl(MWindowFlags(0), inParent)
		, mResponse(this, &MGtkDialogImpl::OnResponse)
		, mResultIsOK(false)
	{
	}

	void CreateWindow(MRect inBounds, const std::string &inTitle) override
	{
		SetEventMask(MEventMask::KeyCapture);

		MGtkWindowImpl::CreateWindow(inBounds, inTitle);

		// mKeyPressed.GetSourceGObject
	}

	void OnResponse(int32_t inResponseID)
	{
		// PRINT(("Response: %d", inResponseID));

		MDialog *dlog = static_cast<MDialog *>(mWindow);

		switch (inResponseID)
		{
			case GTK_RESPONSE_OK:
				mResultIsOK = true;
				if (dlog->OKClicked())
					dlog->Close();
				break;

			case GTK_RESPONSE_CANCEL:
				if (dlog->CancelClicked())
					dlog->Close();
				break;

			case GTK_RESPONSE_DELETE_EVENT:
				// ehm... now what?
				break;

			default:
				if (inResponseID > 0 and static_cast<uint32_t>(inResponseID) <= mResponseIDs.size())
					dlog->ButtonClicked(mResponseIDs[inResponseID - 1]);
				break;
		}
	}

	MSlot<void(int32_t)> mResponse;

	std::vector<std::string> mResponseIDs;
	bool mResultIsOK;
};

// bool MGtkDialogImpl::OnKeyPressEvent(GdkEvent *inEvent)
// {
// 	// PRINT(("MGtkDialogImpl::OnKeyPressEvent"));

// 	bool result = MGtkWidgetMixin::OnKeyPressEvent(inEvent);

// 	if (not result)
// 	{
// 		uint32_t keyCode = MapKeyCode(gdk_key_event_get_keyval(inEvent));
// 		uint32_t modifiers = MapModifier(gdk_event_get_modifier_state(inEvent));

// 		if ((keyCode == kEnterKeyCode or keyCode == kReturnKeyCode) and modifiers == 0)
// 		{
// 			OnResponse(mDefaultResponse);
// 			result = true;
// 		}
// 	}

// 	// PRINT(("MGtkDialogImpl::OnKeyPressEvent => %d", result));

// 	return result;
// }

// --------------------------------------------------------------------

MWindowImpl *MWindowImpl::CreateDialogImpl(MWindow *inWindow)
{
	return new MGtkDialogImpl(inWindow);
}

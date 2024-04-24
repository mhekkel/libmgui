/*-
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2024 Maarten L. Hekkelman
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

#include "MDocApplication.hpp"
#include "MAlerts.hpp"
#include "MController.hpp"
#include "MDocument.hpp"
#include "MFile.hpp"

// --------------------------------------------------------------------

MDocApplication::MDocApplication(MApplicationImpl *inImpl)
	: MApplication(inImpl)
{
}

void MDocApplication::DoNew()
{
}

void MDocApplication::DoOpen()
{
	MFileDialogs::ChooseFiles(gApp->GetActiveWindow(), [&](bool ok, std::vector<std::filesystem::path> files)
		{
			if (ok)
			{
				try
				{
					MDocument *doc = nullptr;

					for (auto &file : files)
						doc = OpenOneDocument(file);

					if (doc != nullptr)
						DisplayDocument(doc);
				}
				catch (const std::exception &e)
				{
					DisplayError(e);
				}
			}
			//
		});
}

void MDocApplication::DoCloseAll(/* MCloseReason inAction */)
{
	MDocument *doc = MDocument::GetFirstDocument();

	while (doc != nullptr)
	{
		auto next = doc->GetNextDocument();
		auto controller = doc->GetFirstController();
		if (controller)
			controller->TryCloseDocument();
		else
			assert(controller);
		doc = next;
	}
}

bool MDocApplication::AllowQuit(bool inLogOff)
{
	DoCloseAll();
	return MDocument::GetFirstDocument() == nullptr;
}

MDocument *MDocApplication::OpenOneDocument(const std::filesystem::path &inFileRef)
{
	return nullptr;
}

MDocWindow *MDocApplication::DisplayDocument(MDocument *inDocument)
{
	return nullptr;
}

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

#pragma once

#include "MApplication.hpp"
#include "MSaverMixin.hpp"

class MDocument;
class MDocWindow;

// --------------------------------------------------------------------

class MDocApplication : public MApplication
{
  public:
	MDocApplication(MApplicationImpl *inImpl);

	virtual void DoNew();
	virtual void DoOpen();
	virtual void DoCloseAll();

	virtual MDocument *OpenOneDocument(const std::filesystem::path &inFileRef);
	virtual MDocWindow *DisplayDocument(MDocument *inDocument);

	static MDocApplication &Instance()
	{
		return *static_cast<MDocApplication*>(gApp);
	}

  protected:

	MCommand<void()> cNew;
	MCommand<void()> cOpen;
	MCommand<void()> cCloseAll;
};
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

/*	$Id: MClipboard.h 158 2007-05-28 10:08:18Z maarten $
    Copyright Maarten L. Hekkelman
    Created Wednesday July 21 2004 18:18:19
*/

#pragma once

#include "MTypes.hpp"

#include <memory>
#include <string>

// --------------------------------------------------------------------

class MClipboard;

struct MClipboardGetDataHandlerbase
{
	virtual ~MClipboardGetDataHandlerbase() = default;
	virtual void HandleData(const std::string &data) = 0;
};

// --------------------------------------------------------------------

class MClipboardImpl
{
  public:
	MClipboardImpl(MClipboard *inClipboard)
		: mClipboard(inClipboard)
	{
	}

	virtual ~MClipboardImpl() = default;

	virtual bool HasData() = 0;
	virtual bool IsLocal() = 0;
	virtual void GetData(MClipboardGetDataHandlerbase *inHandler) = 0;
	virtual void SetData(const std::string &inData) = 0;

	static MClipboardImpl *Create(MClipboard *inClipboard, bool inPrimary);

  protected:
	MClipboard *mClipboard;
};

// --------------------------------------------------------------------

class MClipboard
{
  public:

	virtual ~MClipboard()
	{
		delete mImpl;
	}

	static void Init(MClipboardImpl *inImpl = nullptr)
	{
		if (inImpl)
			Init(new MClipboard(inImpl));
		else
			Init(new MClipboard(false));
	}

	static void Init(MClipboard *inClipboard)
	{
		return sInstance.reset(inClipboard);
	}

	static void InitPrimary(MClipboardImpl *inPrimaryImpl = nullptr)
	{
		if (inPrimaryImpl)
			InitPrimary(new MClipboard(inPrimaryImpl));
		else
			InitPrimary(new MClipboard(true));
	}

	static void InitPrimary(MClipboard *inPrimary)
	{
		return sPrimary.reset(inPrimary);
	}

	static MClipboard &Instance()
	{
		if (not sInstance)
			Init();

		return *sInstance.get();
	}

	static MClipboard &PrimaryInstance()
	{
		return sPrimary ? *sPrimary.get() : Instance();
	}

	bool HasData()
	{
		return mImpl->HasData();
	}

	bool IsLocal()
	{
		return mImpl->IsLocal();
	}

	template <typename Handler>
	struct MGetDataHandler : public MClipboardGetDataHandlerbase
	{
		MGetDataHandler(Handler &&handler)
			: mHandler(std::move(handler))
		{
		}

		void HandleData(const std::string &inData)
		{
			mHandler(inData);
		}

		Handler mHandler;
	};

	template <typename Handler>
	void GetData(Handler &&handler)
	{
		mImpl->GetData(new MGetDataHandler(std::move(handler)));
	}

	void SetData(const std::string &inText)
	{
		mImpl->SetData(inText);
	}

  protected:
	
	MClipboard(MClipboardImpl *inImpl)
		: mImpl(inImpl)
	{
	}

  private:
	MClipboard(bool inPrimary)
		: MClipboard(MClipboardImpl::Create(this, inPrimary))
	{
	}

	class MClipboardImpl *mImpl;
	static std::unique_ptr<MClipboard> sInstance, sPrimary;
};

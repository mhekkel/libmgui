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

#include <string>

const uint32_t
	kClipboardRingSize = 7;

class MClipboard
{
  public:
	static MClipboard &Instance();

	bool HasData();
	bool IsBlock();
	void NextInRing();
	void PreviousInRing();
	void GetData(std::string &outText, bool &outIsBlock);
	void SetData(const std::string &inText, bool inBlock);
	void AddData(const std::string &inText);

	struct Data
	{
		Data(const std::string &inText, bool inBlock);
		void SetData(const std::string &inText, bool inBlock);
		void AddData(const std::string &inText);

		std::string mText;
		bool mBlock;
	};

  private:
	MClipboard();
	virtual ~MClipboard();

	class MClipboardImpl *mImpl;

	Data *mRing[kClipboardRingSize];
	uint32_t mCount;
};

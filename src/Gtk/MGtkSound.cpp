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

#include "MGtkLib.hpp"
#include "MPreferences.hpp"
#include "MSound.hpp"

#include "mrsrc.hpp"

#include <canberra.h>

// void PlaySound(const std::string &inSoundName)
// {
// 	gdk_display_beep(gdk_display_get_default());
// }

namespace
{

class MAudioSocket
{
  public:
	static MAudioSocket &
	Instance();

	void Play(const std::string &inPath);

  private:
	MAudioSocket();
	~MAudioSocket();

	static void CAFinishCallback(ca_context *inContext, uint32_t inID, int inErrorCode, void *inUserData);

	ca_context *mCAContext;
};

MAudioSocket::MAudioSocket()
	: mCAContext(nullptr)
{
	int err = ca_context_create(&mCAContext);
	if (err == 0)
		err = ca_context_open(mCAContext);
}

MAudioSocket::~MAudioSocket()
{
	ca_context_destroy(mCAContext);
	mCAContext = nullptr;
}

MAudioSocket &MAudioSocket::Instance()
{
	static MAudioSocket sInstance;
	return sInstance;
}

void MAudioSocket::CAFinishCallback(ca_context *inContext, uint32_t inID, int inErrorCode, void *inUserData)
{
	if (inErrorCode != CA_SUCCESS)
	{
		gdk_display_beep(gdk_display_get_default());
		std::cerr << "Error playing sound using canberra: " << ca_strerror(inErrorCode) << std::endl;
	}
}

void MAudioSocket::Play(const std::string &inSoundName)
{
	if (mCAContext != nullptr)
	{
		ca_proplist *pl;
		ca_proplist_create(&pl);
		ca_proplist_sets(pl, CA_PROP_EVENT_ID, inSoundName.c_str());
		int err = ca_context_play_full(mCAContext, 0, pl, &MAudioSocket::CAFinishCallback, this);
		if (err != CA_SUCCESS)
			std::cerr << "Error calling ca_context_play_full: " << ca_strerror(err) << std::endl;
		ca_proplist_destroy(pl);
	}
	else
		gdk_display_beep(gdk_display_get_default());
}

} // namespace

void PlaySound(const std::string &inSoundName)
{
	MAudioSocket::Instance().Play(inSoundName);
}

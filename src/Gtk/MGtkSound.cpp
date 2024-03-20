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

#include "Gtk/MGtkLib.hpp"

#include "MError.hpp"
#include "MFile.hpp"
#include "MPreferences.hpp"
#include "MSound.hpp"
#include "MWindow.hpp"

#include <canberra.h>
#include <dlfcn.h>
#include <iostream>
#include <map>

using namespace std;
namespace fs = std::filesystem;

namespace
{

class MAudioSocket
{
  public:
	static MAudioSocket &Instance();

	void Play(const string &inPath);

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
		cerr << "Error playing sound using canberra: " << ca_strerror(inErrorCode) << '\n';
}

void MAudioSocket::Play(const string &inFile)
{
	if (mCAContext != nullptr)
	{
		ca_proplist *pl;

		ca_proplist_create(&pl);
		ca_proplist_sets(pl, CA_PROP_MEDIA_FILENAME, inFile.c_str());

		int pan = 2;
		//        gc = gconf_client_get_default();
		//        value = gconf_client_get(gc, ALARM_GCONF_PATH, NULL);
		//
		//        if (value && value->type == GCONF_VALUE_INT)
		//                pan = gconf_value_get_int(value);
		//        else
		//                pan = 2;
		float volume = (1.0f - float(pan) / 2.0f) * -6.0f;
		ca_proplist_setf(pl, CA_PROP_CANBERRA_VOLUME, "%f", volume);

		int err = ca_context_play_full(mCAContext, 0, pl, &MAudioSocket::CAFinishCallback, this);
		if (err != CA_SUCCESS)
			cerr << "Error calling ca_context_play_full: " << ca_strerror(err) << '\n';

		ca_proplist_destroy(pl);
	}
	//	else if (MWindow::GetFirstWindow() != nullptr)
	//		MWindow::GetFirstWindow()->Beep();
	else
		gdk_display_beep(gdk_display_get_default());
}

} // namespace

void PlaySound(const string &inSoundName)
{
	//	if (not gPlaySounds)
	//		return;

	try
	{
		StOKToThrow ok;
		string filename;

		if (inSoundName == "success")
			filename = Preferences::GetString("success sound", "info.wav");
		else if (inSoundName == "failure" or inSoundName == "error")
			filename = Preferences::GetString("failure sound", "error.wav");
		else if (inSoundName == "warning")
			filename = Preferences::GetString("warning sound", "warning.wav");
		else if (inSoundName == "question")
			filename = Preferences::GetString("question sound", "question.wav");
		else
		{
			filename = "warning.wav";
			cerr << "Unknown sound name " << inSoundName << '\n';
		}

		fs::path path = filename;

		const char *const *config_dirs = g_get_system_data_dirs();
		for (const char *const *dir = config_dirs; *dir != nullptr; ++dir)
		{
			path = fs::path(*dir) / "sounds" / filename;
			if (fs::exists(path))
				break;
		}

		if (fs::exists(path))
			MAudioSocket::Instance().Play(path.string());
		else
		{
			cerr << "Sound does not exist: " << path.string() << '\n';
			//			if (MWindow::GetFirstWindow() != nullptr)
			//				MWindow::GetFirstWindow()->Beep();
			//			else
			gdk_display_beep(gdk_display_get_default());
		}
	}
	catch (...)
	{
		gdk_display_beep(gdk_display_get_default());
	}
}

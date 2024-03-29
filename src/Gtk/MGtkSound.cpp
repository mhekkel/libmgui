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

// #include "MError.hpp"
// #include "MFile.hpp"
// #include "MPreferences.hpp"
#include "MSound.hpp"
// #include "MWindow.hpp"

#include "mrsrc.hpp"

// #include <canberra.h>
// #include <dlfcn.h>
// #include <iostream>
// #include <map>

static void
sound_ended(GObject *object)
{
	g_object_unref(object);
}

void PlaySound(const std::string &inSoundName)
{
	// mrsrc::rsrc data("Sounds/" + inSoundName + ".ogg");
	// if (data)
	// {
	// 	// GInputStream *s = g_memory_input_stream_new_from_data(data.data(), data.size(), nullptr);

	// 	// auto mf = gtk_media_file_new_for_input_stream(s);

	// 	auto mf = gtk_media_file_new_for_filename("/home/maarten/projects/salt/rsrc/Sounds/service-login.oga");

	// 	gtk_media_stream_set_volume(GTK_MEDIA_STREAM(mf), 1.0);
	// 	gtk_media_stream_play(GTK_MEDIA_STREAM(mf));
	// 	g_signal_connect(mf, "notify::ended", G_CALLBACK(sound_ended), NULL);

	// 	// g_object_unref(mf);

	// 	return;
	// }

	// ao_device *device;
	// 	ao_sample_format format;
	// 	int default_driver;
	// 	char *buffer;
	// 	int buf_size;
	// 	int sample;
	// 	float freq = 440.0;
	// 	int i;

	// 	/* -- Initialize -- */

	// 	fprintf(stderr, "libao example program\n");

	// 	ao_initialize();

	// 	/* -- Setup for default driver -- */

	// 	default_driver = ao_default_driver_id();

	//         memset(&format, 0, sizeof(format));
	// 	format.bits = 16;
	// 	format.channels = 2;
	// 	format.rate = 44100;
	// 	format.byte_format = AO_FMT_LITTLE;

	// 	/* -- Open driver -- */
	// 	device = ao_open_live(default_driver, &format, NULL /* no options */);
	// 	if (device == NULL) {
	// 		fprintf(stderr, "Error opening device.\n");
	// 		return 1;
	// 	}

	// 	/* -- Play some stuff -- */
	// 	buf_size = format.bits/8 * format.channels * format.rate;
	// 	buffer = calloc(buf_size,
	// 			sizeof(char));

	// 	for (i = 0; i < format.rate; i++) {
	// 		sample = (int)(0.75 * 32768.0 *
	// 			sin(2 * M_PI * freq * ((float) i/format.rate)));

	// 		/* Put the same stuff in left and right channel */
	// 		buffer[4*i] = buffer[4*i+2] = sample & 0xff;
	// 		buffer[4*i+1] = buffer[4*i+3] = (sample >> 8) & 0xff;
	// 	}
	// 	ao_play(device, buffer, buf_size);

	// 	/* -- Close and shutdown -- */
	// 	ao_close(device);

	// 	ao_shutdown();

	// 	free(buffer);

	//   return (0);

	// if (not gPlaySounds)
	// 	return;

	// try
	// {
	// 	StOKToThrow ok;
	// 	std::string filename;

	// 	if (inSoundName == "success")
	// 		filename = MPrefs::GetString("success sound", "info.wav");
	// 	else if (inSoundName == "failure" or inSoundName == "error")
	// 		filename = MPrefs::GetString("failure sound", "error.wav");
	// 	else if (inSoundName == "warning")
	// 		filename = MPrefs::GetString("warning sound", "warning.wav");
	// 	else if (inSoundName == "question")
	// 		filename = MPrefs::GetString("question sound", "question.wav");
	// 	else
	// 	{
	// 		filename = "warning.wav";
	// 		cerr << "Unknown sound name " << inSoundName << '\n';
	// 	}

	// 	fs::path path = filename;

	// 	const char *const *config_dirs = g_get_system_data_dirs();
	// 	for (const char *const *dir = config_dirs; *dir != nullptr; ++dir)
	// 	{
	// 		path = fs::path(*dir) / "sounds" / filename;
	// 		if (fs::exists(path))
	// 			break;
	// 	}

	// 	if (fs::exists(path))
	// 		MAudioSocket::Instance().Play(path.string());
	// 	else
	// 	{
	// 		cerr << "Sound does not exist: " << path.string() << '\n';
	// 		//			if (MWindow::GetFirstWindow() != nullptr)
	// 		//				MWindow::GetFirstWindow()->Beep();
	// 		//			else
	// 		gdk_display_beep(gdk_display_get_default());
	// 	}
	// }
	// catch (...)
	// {
	gdk_display_beep(gdk_display_get_default());
	// }
}

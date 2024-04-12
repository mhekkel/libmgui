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

#include "MError.hpp"
// #include "MFile.hpp"
// #include "MPreferences.hpp"
#include "MSound.hpp"
// #include "MWindow.hpp"

#include "mrsrc.hpp"

#include "ao/ao.h"

void PlaySound(const std::string &inSoundName)
{
	ao_initialize();
	auto default_driver_id = ao_default_driver_id();

	ao_device *device = nullptr;

	try
	{
		mrsrc::rsrc data("Sounds/" + inSoundName);
		if (not data)
			data = mrsrc::rsrc("warning.wav");

		if (data)
			std::cerr << "yeah\n";


		ao_sample_format format
		{
			.bits = 16,
			.rate = 24000,
			.channels = 2,
			.byte_format = AO_FMT_LITTLE
		};

		device = ao_open_live(default_driver_id, &format, nullptr);
		if (device)
			ao_play(device, const_cast<char *>(data.data()), data.size());
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
	}

	if (device)
		ao_close(device);

	ao_shutdown();
}

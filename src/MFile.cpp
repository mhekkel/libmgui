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

#include "MFile.hpp"

#include <gxrio.hpp>

// #include <unistd.h>

#include <cassert>

// --------------------------------------------------------------------
// MFileLoader, used to load the contents of a file.
// This works strictly asynchronous.

MFileLoader::MFileLoader(MDocument &inDocument, std::filesystem::path &inFile)
	: mFile(inFile)
	, mDocument(inDocument)
{
}

MFileLoader::~MFileLoader()
{
	eFileLoaderDeleted(this);
}

void MFileLoader::Cancel()
{
	delete this;
}

void MFileLoader::SetFileInfo(bool inReadOnly, std::chrono::system_clock::time_point inModDate)
{
	std::error_code ec;

	using std::filesystem::perms;

	auto cp = std::filesystem::status(mFile, ec).permissions();

	if (inReadOnly)
		std::filesystem::permissions(mFile, cp ^ (perms::owner_write | perms::group_write | perms::others_write), ec);
	else
		std::filesystem::permissions(mFile, cp | perms::owner_write, ec);

	if (ec)
		throw std::system_error(ec);

	std::filesystem::last_write_time(mFile, std::filesystem::file_time_type::clock::now() + (inModDate - std::chrono::system_clock::now()), ec);
	if (ec)
		throw std::system_error(ec);
}

// --------------------------------------------------------------------

class MLocalFileLoader : public MFileLoader
{
  public:
	MLocalFileLoader(MDocument &inDocument, std::filesystem::path &inFile);

	virtual void DoLoad();
};

// --------------------------------------------------------------------

MLocalFileLoader::MLocalFileLoader(MDocument &inDocument, std::filesystem::path &inFile)
	: MFileLoader(inDocument, inFile)
{
}

void MLocalFileLoader::DoLoad()
{
	try
	{
		if (not std::filesystem::exists(mFile))
			throw std::runtime_error("File does not exist, " + mFile.string());

		auto modTime = std::filesystem::last_write_time(mFile);
		bool readOnly = false;

		std::error_code ec;

		using std::filesystem::perms;

		if (auto st = std::filesystem::status(mFile, ec); ec == std::errc{})
			readOnly = (st.permissions() & perms::owner_write) != perms::owner_write;
		// {
		// 	// fetch user&group
		// 	unsigned int gid = getgid();
		// 	unsigned int uid = getuid();

		// 	readOnly = not((uid == st.permissions() and (S_IWUSR & st.st_mode)) or
		// 				   (gid == st.st_gid and (S_IWGRP & st.st_mode)) or
		// 				   (S_IWOTH & st.st_mode));

		// 	if (readOnly && S_IWGRP & st.st_mode)
		// 	{
		// 		int ngroups = getgroups(0, nullptr);
		// 		if (ngroups > 0)
		// 		{
		// 			vector<gid_t> groups(ngroups);
		// 			if (getgroups(ngroups, &groups[0]) == 0)
		// 				readOnly = find(groups.begin(), groups.end(), st.st_gid) == groups.end();
		// 		}
		// 	}
		// }

		gxrio::ifstream file(mFile, std::ios_base::in | std::ios::binary);
		// std::ifstream file(mFile, std::ios::binary);

		if (not file.is_open())
			throw std::system_error(std::make_error_code(std::errc(errno)), mFile.string());

		eReadFile(file);

		using namespace std::chrono;
		auto t = modTime - std::filesystem::file_time_type::clock::now();
		SetFileInfo(readOnly, t + system_clock::now());

		eFileLoaded();
	}
	catch (const std::exception &e)
	{
		eError(e.what());
		delete this;
		throw;
	}

	delete this;
}

MFileLoader *MFileLoader::Load(MDocument &inDocument, std::filesystem::path &inFile)
{
	return new MLocalFileLoader(inDocument, inFile);
}

// --------------------------------------------------------------------
// MFileSaver, used to save data to a file.

MFileSaver::MFileSaver(MDocument &inDocument, std::filesystem::path &inFile)
	: mFile(inFile)
	, mDocument(inDocument)
{
}

MFileSaver::~MFileSaver()
{
	eFileSaverDeleted(this);
}

void MFileSaver::Cancel()
{
	delete this;
}

void MFileSaver::SetFileInfo(bool inReadOnly, std::chrono::system_clock::time_point inModDate)
{
	std::error_code ec;

	using std::filesystem::perms;

	auto cp = std::filesystem::status(mFile, ec).permissions();

	if (inReadOnly)
		std::filesystem::permissions(mFile, cp ^ (perms::owner_write | perms::group_write | perms::others_write), ec);
	else
		std::filesystem::permissions(mFile, cp | perms::owner_write, ec);

	if (ec)
		throw std::system_error(ec);

	std::filesystem::last_write_time(mFile, std::filesystem::file_time_type::clock::now() + (inModDate - std::chrono::system_clock::now()), ec);
	if (ec)
		throw std::system_error(ec);
}

// --------------------------------------------------------------------

class MLocalFileSaver : public MFileSaver
{
  public:
	MLocalFileSaver(MDocument &inDocument, std::filesystem::path &inFile);

	virtual void DoSave();
};

// --------------------------------------------------------------------

MLocalFileSaver::MLocalFileSaver(MDocument &inDocument, std::filesystem::path &inFile)
	: MFileSaver(inDocument, inFile)
{
}

void MLocalFileSaver::DoSave()
{
	try
	{
		// if (not std::filesystem::exists(mFile) or
		// 	// std::filesystem::last_write_time(mFile) <= mFile.GetModDate() or
		// 	eAskOverwriteNewer())
		// {
			gxrio::ofstream file(mFile, std::ios_base::out | std::ios::trunc | std::ios::binary);

			if (not file.is_open())
				throw std::system_error(std::make_error_code(std::errc(errno)), mFile.string());

			eWriteFile(file);

			file.close();

			eFileWritten();

			// SetFileInfo(false, std::filesystem::last_write_time(mFile));
		// }
	}
	catch (const std::exception &e)
	{
		eError(e.what());
	}

	delete this;
}

MFileSaver *MFileSaver::Save(MDocument &inDocument, std::filesystem::path &inFile)
{
	return new MLocalFileSaver(inDocument, inFile);
}

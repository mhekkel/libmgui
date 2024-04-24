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

#pragma once

#include "MWindow.hpp"

#include <filesystem>
#include <functional>

class MDocument;

// --------------------------------------------------------------------
// MFileLoader, used to load the contents of a file.

class MFileLoader
{
  public:
	std::function<void(float, const std::string &)> eProgress;
	std::function<void(const std::string &)> eError;
	std::function<void(std::istream &)> eReadFile;
	std::function<void()> eFileLoaded;
	std::function<void(MFileLoader *)> eFileLoaderDeleted;

	virtual void DoLoad() = 0;

	virtual void Cancel();

	static MFileLoader *Load(MDocument &inDocument, std::filesystem::path &inFile);

  protected:
	MFileLoader(MDocument &inDocument, std::filesystem::path &inFile);

	virtual ~MFileLoader();

	std::filesystem::path &mFile;

	void SetFileInfo(bool inReadOnly, std::chrono::system_clock::time_point inModDate);

  private:
	MFileLoader(const MFileLoader &rhs) = delete;
	MFileLoader &operator=(const MFileLoader &rhs) = delete;

	MDocument &mDocument;
};

// --------------------------------------------------------------------
// MFileSaver, used to save data to a file.

class MFileSaver
{
  public:
	std::function<void(float, const std::string &)> eProgress;
	std::function<void(const std::string &)> eError;
	std::function<void(std::ostream &)> eWriteFile;
	std::function<bool()> eAskOverwriteNewer;
	std::function<void(void)> eFileWritten;
	std::function<void(MFileSaver *)> eFileSaverDeleted;

	virtual void DoSave() = 0;

	virtual void Cancel();

	static MFileSaver *Save(MDocument &inDocument, std::filesystem::path &inFile);

  protected:
	MFileSaver(MDocument &inDocument, std::filesystem::path &inFile);

	virtual ~MFileSaver();

	std::filesystem::path &mFile;

	void SetFileInfo(bool inReadOnly, std::chrono::system_clock::time_point inModDate);

  private:
	MFileSaver(const MFileSaver &rhs) = delete;
	MFileSaver &operator=(const MFileSaver &rhs) = delete;

	MDocument &mDocument;
};

// --------------------------------------------------------------------

namespace MFileDialogs
{
// All callbacks have as first parameter a boolean indicating a
// positive result (selected something) or a cancel.

void ChooseOneFile(MWindow *inParent, std::function<void(bool, std::filesystem::path)> &&inCallback);
void ChooseFiles(MWindow *inParent, std::function<void(bool, std::vector<std::filesystem::path>)> &&inCallback);
void ChooseDirectory(MWindow *inParent, std::function<void(bool, std::filesystem::path)> &&inCallback);
void SaveFileAs(MWindow *inParent, std::filesystem::path inFileName, std::function<void(bool, std::filesystem::path)> &&inCallback);

} // namespace MFileDialogs

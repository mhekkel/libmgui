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

#include "MDocument.hpp"
#include "MAlerts.hpp"
#include "MController.hpp"
#include "MDocApplication.hpp"
#include "MDocClosedNotifier.hpp"
#include "MDocWindow.hpp"
#include "MUtils.hpp"

#include <cassert>
#include <cstring>

namespace fs = std::filesystem;

MDocument *MDocument::sFirst;

// ---------------------------------------------------------------------------
//	MDocument

MDocument::MDocument(const std::filesystem::path &inFile)
	: mFile(inFile)
	, mWarnedReadOnly(false)
	, mDirty(false)
	, mFileLoader(nullptr)
	, mFileSaver(nullptr)
	, mNext(nullptr)
{
	mNext = sFirst;
	sFirst = this;
}

// ---------------------------------------------------------------------------
//	~MDocument

MDocument::~MDocument()
{
	if (mFileLoader != nullptr)
		mFileLoader->Cancel();

	if (mFileSaver != nullptr)
		mFileSaver->Cancel();

	if (sFirst == this)
		sFirst = mNext;
	else
	{
		MDocument *doc = sFirst;
		while (doc != nullptr)
		{
			MDocument *next = doc->mNext;
			if (next == this)
			{
				doc->mNext = mNext;
				break;
			}
			doc = next;
		}
	}

	eDocumentClosed(this);
}

// ---------------------------------------------------------------------------
//	SetFile

void MDocument::SetFile(const std::filesystem::path &inFile)
{
	mFile = inFile;
	mLastSaved = fs::last_write_time(mFile);
	eFileSpecChanged(this, mFile);
}

// ---------------------------------------------------------------------------
//	MDocument::DoLoad

void MDocument::DoLoad()
{
	// if (not mFile.IsValid())
	// 	THROW(("File is not specified"));

	if (mFileLoader != nullptr)
		throw std::runtime_error("File is already being loaded");

	// if (mFile.IsLocal() == false or mFile.Exists())
	// {
	mFileLoader = MFileLoader::Load(*this, mFile);

	using namespace std::placeholders;

	mFileLoader->eProgress = std::bind(&MDocument::IOProgress, this, _1, _2);
	mFileLoader->eError = std::bind(&MDocument::IOError, this, _1);
	mFileLoader->eReadFile = std::bind(&MDocument::ReadFile, this, _1);
	mFileLoader->eFileLoaded = std::bind(&MDocument::IOFileLoaded, this);
	mFileLoader->eFileLoaderDeleted = std::bind(&MDocument::FileLoaderDeleted, this, _1);

	mFileLoader->DoLoad();
	// }
}

// ---------------------------------------------------------------------------
//	DoSave

bool MDocument::DoSave()
{
	assert(IsSpecified());

	if (mFile.empty())
		throw std::runtime_error("File is not specified");

	if (mFileSaver != nullptr)
		return false;

	mFileSaver = MFileSaver::Save(*this, mFile);

	using namespace std::placeholders;

	mFileSaver->eProgress = std::bind(&MDocument::IOProgress, this, _1, _2);
	mFileSaver->eError = std::bind(&MDocument::IOError, this, _1);
	mFileSaver->eWriteFile = std::bind(&MDocument::WriteFile, this, _1);
	mFileSaver->eFileWritten = std::bind(&MDocument::IOFileWritten, this);
	mFileSaver->eFileSaverDeleted = std::bind(&MDocument::FileSaverDeleted, this, _1);

	if (fs::exists(mFile) and fs::last_write_time(mFile) > mLastSaved)
	{
		DisplayAlert(GetWindow(), "ask-overwrite-newer",
			[&](int reply)
			{
				if (reply == 2)
				{
					mFileSaver->DoSave();
					MDocApplication::Instance().AddToRecentMenu(mFile);
				}
				else
					mFileSaver->Cancel();
			},
			{ mFile.filename().string() });
		return false;
	}
	else
	{
		mFileSaver->DoSave();
		MDocApplication::Instance().AddToRecentMenu(mFile);
	}

	return true;
}

// ---------------------------------------------------------------------------
//	DoSaveAs

bool MDocument::DoSaveAs(const std::filesystem::path &inFile)
{
	bool result = false;

	std::filesystem::path savedFile = mFile;
	mFile = inFile;

	if (DoSave())
	{
		eFileSpecChanged(this, mFile);
		// mNotifiers.clear();
		result = true;
	}
	else
		mFile = savedFile;

	return result;
}

// ---------------------------------------------------------------------------
//	FileLoaderDeleted

void MDocument::FileLoaderDeleted(MFileLoader *inFileLoader)
{
	assert(inFileLoader == mFileLoader);
	if (inFileLoader == mFileLoader)
		mFileLoader = nullptr;
}

// ---------------------------------------------------------------------------
//	FileSaverDeleted

void MDocument::FileSaverDeleted(MFileSaver *inFileSaver)
{
	assert(inFileSaver == mFileSaver);
	if (inFileSaver == mFileSaver)
		mFileSaver = nullptr;
}

// ---------------------------------------------------------------------------
//	RevertDocument

void MDocument::RevertDocument()
{
	if (IsSpecified())
		DoLoad();
}

// ---------------------------------------------------------------------------
//	UsesFile

bool MDocument::UsesFile(const std::filesystem::path &inFile) const
{
	return /* mFile.IsValid() and  */ mFile == inFile;
}

MDocument *MDocument::GetDocumentForFile(const std::filesystem::path &inFile)
{
	MDocument *doc = sFirst;

	while (doc != nullptr and not doc->UsesFile(inFile))
		doc = doc->mNext;

	return doc;
}

// ---------------------------------------------------------------------------
//	AddNotifier

void MDocument::AddNotifier(MDocClosedNotifier &&inNotifier, bool inRead)
{
	mNotifiers.push_back(std::move(inNotifier));
}

// ---------------------------------------------------------------------------
//	AddController

void MDocument::AddController(MController *inController)
{
	if (find(mControllers.begin(), mControllers.end(), inController) == mControllers.end())
		mControllers.push_back(inController);
}

// ---------------------------------------------------------------------------
//	RemoveController

void MDocument::RemoveController(MController *inController)
{
	assert(find(mControllers.begin(), mControllers.end(), inController) != mControllers.end());

	mControllers.erase(remove(mControllers.begin(), mControllers.end(), inController),
		mControllers.end());

	if (mControllers.size() == 0)
		CloseDocument();
}

// ---------------------------------------------------------------------------
//	GetFirstController

MController *MDocument::GetFirstController() const
{
	MController *controller = nullptr;
	if (mControllers.size() > 0)
		controller = mControllers.front();
	return controller;
}

// ---------------------------------------------------------------------------
//	GetWindow

MDocWindow *MDocument::GetWindow() const
{
	MDocWindow *result = nullptr;
	MController *controller = GetFirstController();

	if (controller != nullptr)
		result = controller->GetWindow();

	return result;
}

// ---------------------------------------------------------------------------
//	MakeFirstDocument

void MDocument::MakeFirstDocument()
{
	MDocument *d = sFirst;

	if (d != this)
	{
		while (d != nullptr and d->mNext != this)
			d = d->mNext;

		assert(d->mNext == this);
		d->mNext = mNext;
		mNext = sFirst;
		sFirst = this;
	}
}

// ---------------------------------------------------------------------------
//	SetModified

void MDocument::SetModified(bool inModified)
{
	mDirty = inModified;
	eModifiedChanged(mDirty);
}

// ---------------------------------------------------------------------------
//	CloseDocument

void MDocument::CloseDocument()
{
	try
	{
		eDocumentClosed(this);
	}
	catch (...)
	{
	}

	delete this;
}

// // ---------------------------------------------------------------------------
// //	UpdateCommandStatus

// bool MDocument::UpdateCommandStatus(
// 	uint32_t			inCommand,
// 	MMenu*			inMenu,
// 	uint32_t			inItemIndex,
// 	bool&			outEnabled,
// 	bool&			outChecked)
// {
// 	return false;
// }

// // ---------------------------------------------------------------------------
// //	ProcessCommand

// bool MDocument::ProcessCommand(
// 	uint32_t			inCommand,
// 	const MMenu*	inMenu,
// 	uint32_t			inItemIndex,
// 	uint32_t			inModifiers)
// {
// 	return false;
// }

// ---------------------------------------------------------------------------
//	GetWindowTitle

std::string MDocument::GetWindowTitle() const
{
	std::string result;

	// if (mFile.IsLocal())
	// {
	auto file = std::filesystem::canonical(mFile);

	// NormalizePath(file);
	result = file.string();

	// strip off HOME, if any
	std::string home = GetHomeDirectory();
	if (not home.empty() and result.starts_with(home))
	{
		result.erase(0, home.length());
		result.insert(0, "~");
	}
	// }
	// else
	// 	result = mFile.GetURI();

	return result;
}

// ---------------------------------------------------------------------------
//	IOProgress

void MDocument::IOProgress(float inProgress, const std::string &inMessage)
{
}

// ---------------------------------------------------------------------------
//	IOError

void MDocument::IOError(const std::string &inError)
{
	DisplayError(inError);
}

// ---------------------------------------------------------------------------
//	IOFileLoaded

void MDocument::IOFileLoaded()
{
	mLastSaved = fs::last_write_time(mFile);
	SetModified(false);
}

// ---------------------------------------------------------------------------
//	IOFileWritten

void MDocument::IOFileWritten()
{
	mLastSaved = fs::last_write_time(mFile);
	SetModified(false);
}

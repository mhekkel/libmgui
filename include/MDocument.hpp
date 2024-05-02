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

/*

    MDocument is the model in the Model-View-Controller triad

*/

#include "MP2PEvents.hpp"
#include "MFile.hpp"

#include <list>

// class MDocClosedNotifier;
class MController;
class MDocWindow;
class MMenu;

class MDocument
{
  public:
	virtual ~MDocument();

	template <class D>
	static D *Create(const std::filesystem::path &inFile)
	{
		std::unique_ptr<D> doc(new D(inFile));
		if (std::filesystem::exists(inFile))
			doc->DoLoad();
		return doc.release();
	}

	virtual bool DoSave();

	virtual bool DoSaveAs(const std::filesystem::path &inFile);

	virtual void RevertDocument();

	virtual bool IsSpecified() const { return not mFile.empty(); }

	virtual void SetFile(const std::filesystem::path &inNewFile);
	virtual const std::filesystem::path &GetFile() const { return mFile; }

	std::filesystem::file_time_type GetFileSavedTime() const { return mLastSaved; }
	void SetFileSavedTime(std::filesystem::file_time_type inTime) { mLastSaved = inTime; }

	virtual bool UsesFile(const std::filesystem::path &inFile) const;

	static MDocument *GetDocumentForFile(const std::filesystem::path &inFile);

	virtual bool IsReadOnly() const {return false; };// { return std::filesystem::status(mFile).permissions().; }

	// virtual void AddNotifier(MDocClosedNotifier &inNotifier, bool inRead);

	// the MVC interface

	void AddController(MController *inController);

	void RemoveController(MController *inController);

	uint32_t CountControllers() const { return mControllers.size(); }

	MDocWindow *GetWindow() const;

	virtual MController *
	GetFirstController() const;

	static MDocument *GetFirstDocument() { return sFirst; }

	MDocument *GetNextDocument() { return mNext; }

	void MakeFirstDocument();

	bool IsModified() const { return mDirty; }

	virtual void SetModified(bool inModified);

	virtual std::string GetWindowTitle() const;

	MEventOut<void(bool)> eModifiedChanged;
	MEventOut<void(MDocument *)> eDocumentClosed;
	MEventOut<void(MDocument *, const std::filesystem::path &)> eFileSpecChanged;
	MEventOut<void(const std::filesystem::path &)> eBaseDirChanged;

	virtual void FileLoaderDeleted(MFileLoader *inFileLoader);
	virtual void FileSaverDeleted(MFileSaver *inFileSaver);

  protected:
	explicit MDocument(const std::filesystem::path &inFile);

	virtual void DoLoad();

	virtual void CloseDocument();

	// Asynchronous IO support
	virtual void ReadFile(std::istream &inFile) = 0;

	virtual void WriteFile(std::ostream &inFile) = 0;

	virtual void IOProgress(float inProgress, const std::string &);
	virtual void IOError(const std::string &inError);

	virtual void IOFileLoaded();
	virtual void IOFileWritten();

	typedef std::list<MController *> MControllerList;
	// typedef std::list<MDocClosedNotifier> MDocClosedNotifierList;

	MControllerList mControllers;
	// MDocClosedNotifierList mNotifiers;
	std::filesystem::path mFile;
	bool mWarnedReadOnly;
	bool mDirty;
	std::filesystem::file_time_type mLastSaved;

  private:
	MFileLoader *mFileLoader;
	MFileSaver *mFileSaver;

	MDocument *mNext;
	static MDocument *sFirst;
};

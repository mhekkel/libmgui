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

#include "MGtkWidgetMixin.hpp"
#include "MGtkWindowImpl.hpp"

#include "MApplication.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MStrings.hpp"

#include <cassert>
#include <filesystem>

namespace fs = std::filesystem;

// ------------------------------------------------------------------
//
//  Three different implementations of extended attributes...
//

// ------------------------------------------------------------------
//  FreeBSD

#if defined(__FreeBSD__) and (__FreeBSD__ > 0)

# include <sys/extattr.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, size_t inDataSize)
{
	std::string path = inPath.string();

	return extattr_get_file(path.c_str(), EXTATTR_NAMESPACE_USER,
		inName, outData, inDataSize);
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, size_t inDataSize)
{
	std::string path = inPath.string();

	time_t t = last_write_time(inPath);

	int r = extattr_set_file(path.c_str(), EXTATTR_NAMESPACE_USER,
		inName, inData, inDataSize);

	last_write_time(inPath, t);
}

#endif

// ------------------------------------------------------------------
//  Linux

#if defined(__linux__)

// #include <attr/attributes.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, std::size_t inDataSize)
{
	std::string path = inPath.string();

	int length = inDataSize;
	// int err = ::attr_get(path.c_str(), inName,
	// 	reinterpret_cast<char*>(outData), &length, 0);

	// if (err != 0)
	// 	length = 0;

	return length;
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, std::size_t inDataSize)
{
	std::string path = inPath.string();

	// (void)::attr_set(path.c_str(), inName,
	// 	reinterpret_cast<const char*>(inData), inDataSize, 0);

	return inDataSize;
}

#endif

// ------------------------------------------------------------------
//  MacOS X

#if defined(__APPLE__)

# include <sys/xattr.h>

int32_t read_attribute(const fs::path &inPath, const char *inName, void *outData, size_t inDataSize)
{
	std::string path = inPath.string();

	return ::getxattr(path.c_str(), inName, outData, inDataSize, 0, 0);
}

int32_t write_attribute(const fs::path &inPath, const char *inName, const void *inData, size_t inDataSize)
{
	std::string path = inPath.string();

	(void)::setxattr(path.c_str(), inName, inData, inDataSize, 0, 0);
}

#endif
namespace MFileDialogs
{

struct FileChooserResponder
{
  public:
	FileChooserResponder(GtkFileChooser *dialog, std::function<void(std::filesystem::path)> &&callback)
		: mOnResponse(this, &FileChooserResponder::OnResponse)
		, mCallback(std::move(callback))
		, mChooser(dialog)
	{
		g_object_set_data(G_OBJECT(dialog), "save_as_dialog_responder", this);
		mOnResponse.Connect(G_OBJECT(dialog), "response");
	}

  private:
	void OnResponse(int response);

	MSlot<void(int)> mOnResponse;
	std::function<void(std::filesystem::path)> mCallback;
	GtkFileChooser *mChooser;
};

void FileChooserResponder::OnResponse(int response)
{
	if (response == GTK_RESPONSE_ACCEPT)
	{
		GFile *file = gtk_file_chooser_get_file(mChooser);

		const char *path = g_file_get_path(file);
		if (path != nullptr)
		{
			mCallback(path);
			g_free(gpointer(path));
		}

		g_object_unref(file);
	}

	g_object_unref(mChooser);

	delete this;
}

void ChooseOneFile(MWindow *inParent, std::function<void(std::filesystem::path)> &&callback)
{
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_OPEN;

	GtkFileChooserNative *native = gtk_file_chooser_native_new(_("Select File"),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inParent->GetImpl())->GetWidget()),
		action,
		_("Open"),
		_("Cancel"));

	GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

	new FileChooserResponder(chooser, std::move(callback));

	gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

void ChooseDirectory(MWindow *inParent, std::function<void(std::filesystem::path)> &&callback)
{
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER;

	GtkFileChooserNative *native = gtk_file_chooser_native_new(_("Select Folder"),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inParent->GetImpl())->GetWidget()),
		action,
		_("Select"),
		_("Cancel"));

	GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

	new FileChooserResponder(chooser, std::move(callback));

	gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

void SaveFileAs(MWindow *inParent, std::filesystem::path filename, std::function<void(std::filesystem::path)> &&callback)
{
	GtkFileChooserAction action = GTK_FILE_CHOOSER_ACTION_SAVE;

	GtkFileChooserNative *native = gtk_file_chooser_native_new(_("Save File"),
		GTK_WINDOW(static_cast<MGtkWindowImpl *>(inParent->GetImpl())->GetWidget()),
		action,
		_("Save"),
		_("Cancel"));

	GtkFileChooser *chooser = GTK_FILE_CHOOSER(native);

	if (exists(filename))
	{
		GFile *file = g_file_new_for_path(filename.c_str());
		gtk_file_chooser_set_file(chooser, file, NULL);
		g_object_unref(file);
	}
	else if (not filename.empty())
		gtk_file_chooser_set_current_name(chooser, filename.filename().c_str());
	else
		gtk_file_chooser_set_current_name(chooser, _("Untitled document"));

	new FileChooserResponder(chooser, std::move(callback));

	gtk_native_dialog_show(GTK_NATIVE_DIALOG(native));
}

} // namespace MFileDialogs

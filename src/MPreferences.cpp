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

/*	$Id: MPreferences.cpp 85 2006-09-14 08:23:20Z maarten $
    Copyright Maarten L. Hekkelman
    Created Sunday August 01 2004 13:33:22
*/

#include "MPreferences.hpp"
#include "MApplication.hpp"
#include "MError.hpp"
#include "MFile.hpp"
#include "MTypes.hpp"
#include "MUtils.hpp"

#include <zeep/xml/document.hpp>
#include <zeep/xml/node.hpp>
#include <zeep/xml/serialize.hpp>

#include <cerrno>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

namespace xml = zeep::xml;
namespace fs = std::filesystem;

fs::path gPrefsDir;
std::string gPrefsFileName = std::string(kAppName) + ".cfg";

namespace Preferences
{

struct preference
{
	std::string name;
	std::vector<std::string> value;

	template <class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar &zeep::make_nvp("name", name) & zeep::make_nvp("value", value);
	}
};

struct preferences
{
	std::vector<preference> pref;

	template <class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar &zeep::xml::make_element_nvp("pref", pref);
	}
};

class IniFile
{
  public:
	static IniFile &Instance();

	void SetString(const char *inName, const std::string &inValue);
	std::string GetString(const char *inName, const std::string &inDefault);

	void SetStrings(const char *inName, const std::vector<std::string> &inValues);
	std::vector<std::string> GetStrings(const char *inName);

	std::filesystem::file_time_type GetCreationTime() const { return fs::last_write_time(mPrefsFile); }
	bool IsDirty() const { return mDirty; }

	void Save();

  private:
	IniFile();
	~IniFile();

	typedef std::map<std::string, std::vector<std::string>> PrefsMap;

	fs::path mPrefsFile;
	PrefsMap mPrefs;
	bool mDirty;
};

IniFile::IniFile()
	: mDirty(false)
{
	try
	{
		// we use the gPrefsFileName that is located in the directory containing the exe, if it exists...
		if (fs::exists(gExecutablePath))
			mPrefsFile = gExecutablePath.parent_path() / gPrefsFileName;

		if (fs::exists(mPrefsFile))
			gPrefsDir = gExecutablePath.parent_path();
		else
		{
			gPrefsDir = GetPrefsDirectory();

			if (not fs::exists(gPrefsDir))
				fs::create_directories(gPrefsDir);

			mPrefsFile = gPrefsDir / gPrefsFileName;
		}

		if (fs::exists(mPrefsFile))
		{
			std::ifstream data(mPrefsFile, std::ios::binary);

			if (data.is_open())
			{
				xml::document doc(data);

				preferences prefs;
				doc.deserialize("preferences", prefs);

				for (const preference &p : prefs.pref)
					mPrefs[p.name] = p.value;
			}
		}
	}
	catch (const std::exception &ex)
	{
		std::cerr << "Exception reading preferences: " << ex.what() << '\n';
	}
}

IniFile::~IniFile()
{
	if (mDirty)
		Save();
}

IniFile &IniFile::Instance()
{
	static IniFile sInstance;
	return sInstance;
}

void IniFile::Save()
{
	try
	{
		if (not fs::exists(mPrefsFile))
		{
			if (not fs::exists(gPrefsDir))
				fs::create_directories(gPrefsDir);

			mPrefsFile = gPrefsDir / gPrefsFileName;
		}

		fs::path tmpPrefs = mPrefsFile.parent_path() / (mPrefsFile.filename().string() + "-new");

		std::ofstream data(tmpPrefs);

		if (data.is_open())
		{
			preferences prefs;

			transform(mPrefs.begin(), mPrefs.end(), std::back_inserter(prefs.pref),
				[](const std::pair<std::string, std::vector<std::string>> &p) -> preference
				{
					preference pp = { p.first, p.second };
					return pp;
				});

			xml::document doc;
			doc.serialize("preferences", prefs);;

			doc.set_write_xml_decl(true);
			data << std::setw(2) << doc;

			data.close();

			fs::path oldPrefs = mPrefsFile.parent_path() / (mPrefsFile.filename().string() + "-old");

			if (fs::exists(mPrefsFile))
				fs::rename(mPrefsFile, oldPrefs);

			fs::rename(tmpPrefs, mPrefsFile);

			if (fs::exists(oldPrefs))
				fs::remove(oldPrefs);

			mDirty = false;
		}
	}
	catch (const std::exception &ex)
	{
		PRINT(("Exception writing prefs file: %s", ex.what()));
	}
	catch (...)
	{
	}
}

void IniFile::SetString(const char *inName, const std::string &inValue)
{
	std::vector<std::string> values = { inValue };
	if (mPrefs[inName] != values)
	{
		mDirty = true;
		mPrefs[inName] = values;
	}
}

std::string IniFile::GetString(const char *inName, const std::string &inDefault)
{
	if (mPrefs.find(inName) == mPrefs.end() or mPrefs[inName].size() == 0)
		SetString(inName, inDefault);
	std::vector<std::string> &values = mPrefs[inName];
	assert(values.size() == 1);
	if (values.size() != 1)
		std::cerr << "Inconsistent use of preference array/value\n";
	return mPrefs[inName].front();
}

void IniFile::SetStrings(const char *inName, const std::vector<std::string> &inValues)
{
	if (mPrefs[inName] != inValues)
	{
		mDirty = true;
		mPrefs[inName] = inValues;
	}
}

std::vector<std::string> IniFile::GetStrings(const char *inName)
{
	return mPrefs[inName];
}

bool GetBoolean(const char *inName, bool inDefaultValue)
{
	bool result = inDefaultValue;

	try
	{
		std::string s = GetString(inName, inDefaultValue ? "true" : "false");
		result = (s == "true" or s == "1");
	}
	catch (...)
	{
	}

	return result;
}

void SetBoolean(const char *inName, bool inValue)
{
	SetString(inName, inValue ? "true" : "false");
}

int32_t GetInteger(const char *inName, int32_t inDefaultValue)
{
	int32_t result = inDefaultValue;

	try
	{
		result = std::stoi(GetString(
			inName, std::to_string(inDefaultValue)));
	}
	catch (...)
	{
	}

	return result;
}

void SetInteger(const char *inName, int32_t inValue)
{
	SetString(inName, std::to_string(inValue));
}

std::string GetString(const char *inName, std::string inDefaultValue)
{
	return IniFile::Instance().GetString(inName, inDefaultValue);
}

void SetString(const char *inName, std::string inValue)
{
	IniFile::Instance().SetString(inName, inValue);
}

std::vector<std::string> GetArray(const char *inName)
{
	return IniFile::Instance().GetStrings(inName);
}

void SetArray(const char *inName, const std::vector<std::string> &inArray)
{
	IniFile::Instance().SetStrings(inName, inArray);
}

MColor GetColor(const char *inName, MColor inDefaultValue)
{
	inDefaultValue.hex(GetString(inName, inDefaultValue.hex()));
	return inDefaultValue;
}

void SetColor(const char *inName, MColor inValue)
{
	SetString(inName, inValue.hex());
}

MRect GetRect(const char *inName, MRect inDefault)
{
	std::stringstream s;
	s << inDefault.x << ' ' << inDefault.y << ' ' << inDefault.width << ' ' << inDefault.height;
	s.str(GetString(inName, s.str()));

	s >> inDefault.x >> inDefault.y >> inDefault.width >> inDefault.height;
	return inDefault;
}

void SetRect(const char *inName, MRect inValue)
{
	std::ostringstream s;
	s << inValue.x << ' ' << inValue.y << ' ' << inValue.width << ' ' << inValue.height;
	SetString(inName, s.str());
}

std::filesystem::file_time_type GetPrefsFileCreationTime()
{
	return IniFile::Instance().GetCreationTime();
}

void Save()
{
	IniFile::Instance().Save();
}

void SaveIfDirty()
{
	auto &instance = IniFile::Instance();
	if (instance.IsDirty())
		instance.Save();
}

} // namespace Preferences

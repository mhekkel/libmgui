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

#include "MStrings.hpp"
#include "MUtils.hpp"

#include <zeep/xml/document.hpp>
#include <zeep/xml/node.hpp>
#include <zeep/xml/serialize.hpp>

#include "mrsrc.hpp"

#include <map>

using namespace std;
namespace xml = zeep::xml;

struct ls
{
	string context;
	string key;
	string value;

	template <class Archive>
	void serialize(Archive &ar, const unsigned int version)
	{
		ar &zeep::make_nvp("key", key) & zeep::make_nvp("value", value) & zeep::xml::make_attribute_nvp("context", context);
	}
};

class MLocalisedStringTable
{
  public:
	static MLocalisedStringTable &Instance();

	string Map(const string &inString);
	string Map(const string &inContext, const string &inString);
	const char *Map(const char *inString);

	template <class Archive>
	void serialize(Archive &ar, unsigned long v)
	{
		ar &zeep::xml::make_element_nvp("localstring", mLocalStrings);
	}

	MLocalisedStringTable() {}
	MLocalisedStringTable(int);

	map<string, string> mMappedStrings;
	vector<ls> mLocalStrings;
};

MLocalisedStringTable::MLocalisedStringTable(int)
{
	try
	{
		std::string ls = std::locale("").name();

		auto s = ls.find('_');
		std::string lang = ls.substr(0, s);

		mrsrc::istream rsrc("strings.xml." + lang);
		if (rsrc)
		{
			// parse the XML data
			xml::document doc(rsrc);
			doc.deserialize("strings", *this);

			for (auto &s : mLocalStrings)
			{
				zeep::replace_all(s.key, "\\r", "\r");
				zeep::replace_all(s.key, "\\n", "\n");
				zeep::replace_all(s.key, "\\t", "\t");

				zeep::replace_all(s.value, "\\r", "\r");
				zeep::replace_all(s.value, "\\n", "\n");
				zeep::replace_all(s.value, "\\t", "\t");

				mMappedStrings[s.key] = s.value;
			}
		}
	}
	catch (...)
	{
	}
}

string MLocalisedStringTable::Map(const string &inString)
{
	string result = inString;
	map<string, string>::iterator m = mMappedStrings.find(inString);
	if (m != mMappedStrings.end())
		result = m->second;
	return result;
}

string MLocalisedStringTable::Map(const string &inContext, const string &inString)
{
	string result;
	bool found = false;

	for (ls &s : mLocalStrings)
	{
		if (s.context == inContext and s.key == inString)
		{
			found = true;
			result = s.value;
			break;
		}
	}

	if (not found)
		result = Map(inString);

	return result;
}

const char *MLocalisedStringTable::Map(const char *inString)
{
	const char *result = inString;
	map<string, string>::iterator m = mMappedStrings.find(inString);
	if (m != mMappedStrings.end())
		result = m->second.c_str();
	return result;
}

MLocalisedStringTable &MLocalisedStringTable::Instance()
{
	static MLocalisedStringTable sInstance(1);
	return sInstance;
}

const char *GetLocalisedString(const char *inString)
{
	return MLocalisedStringTable::Instance().Map(inString);
}

string GetLocalisedString(const string &inString)
{
	return MLocalisedStringTable::Instance().Map(inString);
}

string GetLocalisedStringForContext(const string &inContext, const string &inString)
{
	return MLocalisedStringTable::Instance().Map(inContext, inString);
}

string GetFormattedLocalisedStringWithArguments(
	const string &inString,
	const vector<string> &inArgs)
{
	string result = GetLocalisedString(inString.c_str());

	char s[] = "^0";

	for (vector<string>::const_iterator a = inArgs.begin(); a != inArgs.end(); ++a)
	{
		string::size_type p = result.find(s);
		if (p != string::npos)
			result.replace(p, 2, *a);
		++s[1];
	}

	return result;
}

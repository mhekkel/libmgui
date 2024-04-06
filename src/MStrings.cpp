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
#include "MUnicode.hpp"
#include "MUtils.hpp"

#include <zeep/xml/document.hpp>
#include <zeep/xml/serialize.hpp>

#include "mrsrc.hpp"

#include <map>

namespace xml = zeep::xml;

struct ls
{
	std::string context;
	std::string key;
	std::string value;

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

	std::string Map(const std::string &inString);
	std::string Map(const std::string &inContext, const std::string &inString);
	const char *Map(const char *inString);

	template <class Archive>
	void serialize(Archive &ar, unsigned long v)
	{
		ar &zeep::xml::make_element_nvp("localstring", mLocalStrings);
	}

	MLocalisedStringTable() {}
	MLocalisedStringTable(int);

	std::map<std::string, std::string> mMappedStrings;
	std::vector<ls> mLocalStrings;
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
				ReplaceAll(s.key, "\\r", "\r");
				ReplaceAll(s.key, "\\n", "\n");
				ReplaceAll(s.key, "\\t", "\t");

				ReplaceAll(s.value, "\\r", "\r");
				ReplaceAll(s.value, "\\n", "\n");
				ReplaceAll(s.value, "\\t", "\t");

				mMappedStrings[s.key] = s.value;
			}
		}
	}
	catch (...)
	{
	}
}

std::string MLocalisedStringTable::Map(const std::string &inString)
{
	std::string result = inString;
	std::map<std::string, std::string>::iterator m = mMappedStrings.find(inString);
	if (m != mMappedStrings.end())
		result = m->second;
	return result;
}

std::string MLocalisedStringTable::Map(const std::string &inContext, const std::string &inString)
{
	std::string result;
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
	std::map<std::string, std::string>::iterator m = mMappedStrings.find(inString);
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

std::string GetLocalisedString(const std::string &inString)
{
	return MLocalisedStringTable::Instance().Map(inString);
}

std::string GetLocalisedStringForContext(const std::string &inContext, const std::string &inString)
{
	return MLocalisedStringTable::Instance().Map(inContext, inString);
}

std::string GetFormattedLocalisedStringWithArguments(
	const std::string &inString,
	const std::vector<std::string> &inArgs)
{
	std::string result = GetLocalisedString(inString.c_str());

	char s[] = "^0";

	for (std::vector<std::string>::const_iterator a = inArgs.begin(); a != inArgs.end(); ++a)
	{
		std::string::size_type p = result.find(s);
		if (p != std::string::npos)
			result.replace(p, 2, *a);
		++s[1];
	}

	return result;
}

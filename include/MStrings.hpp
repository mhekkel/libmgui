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

#include <sstream>
#include <string>
#include <vector>

const char *GetLocalisedString(const char *inString);
std::string GetLocalisedString(const std::string &inString);
std::string GetFormattedLocalisedStringWithArguments(const std::string &inString, const std::vector<std::string> &inArgs);
std::string GetLocalisedStringForContext(const std::string &inContext, const std::string &inString);

template <typename T>
auto _(const T &s)
{
	return GetLocalisedString(s);
}

// --------------------------------------------------------------------

template <class T>
inline void PushArgument(std::vector<std::string> &inArgs, const T &inArg)
{
	std::stringstream s;
	s << inArg;
	inArgs.push_back(s.str());
}

inline void PushArgument(std::vector<std::string> &inArgs, const char *inArg)
{
	inArgs.push_back(GetLocalisedString(inArg));
}

template <>
inline void PushArgument(std::vector<std::string> &inArgs, const std::string &inArg)
{
	inArgs.push_back(GetLocalisedString(inArg.c_str()));
}

template <class ...T>
std::string FormatString(const char *inString, const T & ... inArg)
{
	return GetFormattedLocalisedStringWithArguments(inString, { inArg... });
}

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

#include <filesystem>
#include <string>
#include <vector>

#include "MColor.hpp"
#include "MTypes.hpp"

extern std::filesystem::path gPrefsDir;
extern std::string gPrefsFileName;

namespace Preferences
{

bool GetBoolean(const char *inName, bool inDefaultValue);
void SetBoolean(const char *inName, bool inValue);

int32_t GetInteger(const char *inName, int32_t inDefaultValue);
void SetInteger(const char *inName, int32_t inValue);

std::string GetString(const char *inName, std::string inDefaultValue);
void SetString(const char *inName, std::string inValue);

void GetArray(const char *inName, std::vector<std::string> &outArray);
void SetArray(const char *inName, const std::vector<std::string> &inArray);

MColor GetColor(const char *inName, MColor inDefaultValue);
void SetColor(const char *inName, MColor inValue);

MRect GetRect(const char *inName, MRect inDefault);
void SetRect(const char *inName, MRect inValue);

std::filesystem::file_time_type
GetPrefsFileCreationTime();

void Save();
void SaveIfDirty();

} // namespace Preferences

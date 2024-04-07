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

#include "MUtils.hpp"
#include "MError.hpp"

#include "revision.hpp"

#include <cmath>
#include <sstream>
#include <stack>
#include <string>

uint16_t CalculateCRC(const void *inData, uint32_t inLength, uint16_t inCRC)
{
	const uint8_t *p = reinterpret_cast<const uint8_t *>(inData);

	while (inLength-- > 0)
	{
		inCRC = static_cast<uint16_t>(inCRC ^ (static_cast<uint16_t>(*p++) << 8));
		for (uint16_t i = 0; i < 8; i++)
		{
			if (inCRC & 0x8000)
				inCRC = static_cast<uint16_t>((inCRC << 1) ^ 0x1021);
			else
				inCRC = static_cast<uint16_t>(inCRC << 1);
		}
	}

	return inCRC;
}

std::string Escape(std::string inString)
{
	std::string result;

	for (std::string::iterator c = inString.begin(); c != inString.end(); ++c)
	{
		if (*c == '\n')
		{
			result += '\\';
			result += 'n';
		}
		else if (*c == '\t')
		{
			result += '\\';
			result += 't';
		}
		else if (*c == '\\')
		{
			result += '\\';
			result += '\\';
		}
		else
			result += *c;
	}

	return result;
}

std::string Unescape(std::string inString)
{
	std::string result;

	for (std::string::iterator c = inString.begin(); c != inString.end(); ++c)
	{
		if (*c == '\\')
		{
			++c;
			if (c != inString.end())
			{
				switch (*c)
				{
					case 'n':
						result += '\n';
						break;

					case 't':
						result += '\t';
						break;

					default:
						result += *c;
						break;
				}
			}
			else
				result += '\\';
		}
		else
			result += *c;
	}

	return result;
}

void HexDump(
	const void *inBuffer,
	uint32_t inLength,
	std::ostream &outStream)
{
	const char kHex[] = "0123456789abcdef";
	char s[] = "xxxxxxxx  cccc cccc cccc cccc  cccc cccc cccc cccc  |................|";
	const int kHexOffset[] = { 10, 12, 15, 17, 20, 22, 25, 27, 31, 33, 36, 38, 41, 43, 46, 48 };
	const int kAsciiOffset = 53;

	const unsigned char *data = reinterpret_cast<const unsigned char *>(inBuffer);
	uint32_t offset = 0;

	while (offset < inLength)
	{
		int rr = inLength - offset;
		if (rr > 16)
			rr = 16;

		char *t = s + 7;
		long o = offset;

		while (t >= s)
		{
			*t-- = kHex[o % 16];
			o /= 16;
		}

		for (int i = 0; i < rr; ++i)
		{
			s[kHexOffset[i] + 0] = kHex[data[i] >> 4];
			s[kHexOffset[i] + 1] = kHex[data[i] & 0x0f];
			if (data[i] < 128 and not iscntrl(data[i]) and isprint(data[i]))
				s[kAsciiOffset + i] = data[i];
			else
				s[kAsciiOffset + i] = '.';
		}

		for (int i = rr; i < 16; ++i)
		{
			s[kHexOffset[i] + 0] = ' ';
			s[kHexOffset[i] + 1] = ' ';
			s[kAsciiOffset + i] = ' ';
		}

		outStream << s << '\n';

		offset += rr;
		data += rr;
	}
}
// --------------------------------------------------------------------

std::string GetMGuiVersionString()
{
	std::ostringstream os;
	write_version_string(os, false);
	return os.str();
}
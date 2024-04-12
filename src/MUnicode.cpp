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

/*	$Id: MUnicode.cpp 151 2007-05-21 15:59:05Z maarten $
    Copyright Maarten L. Hekkelman
    Created Monday July 12 2004 21:45:58
*/

#include "MUnicode.hpp"
#include "MError.hpp"
#include "MTypes.hpp"

#include <cassert>
#include <iterator>
#include <sstream>

enum WordBreakClass
{
	eWB_CR,
	eWB_LF,
	eWB_Sep,
	eWB_Tab,
	eWB_Let,
	eWB_Com,
	eWB_Hira,
	eWB_Kata,
	eWB_Han,
	eWB_Other,
	eWB_None
};

enum CharBreakClass
{
	kCBC_CR,
	kCBC_LF,
	kCBC_Control,
	kCBC_Extend,
	kCBC_L,
	kCBC_V,
	kCBC_T,
	kCBC_LV,
	kCBC_LVT,
	kCBC_Other,

	kCBC_Prepend,
	kCBC_SpacingMark
};

enum LineBreakClass
{
	kLBC_OpenPunctuation,
	kLBC_ClosePunctuation,
	kLBC_CloseParenthesis,
	kLBC_Quotation,
	kLBC_NonBreaking,
	kLBC_Nonstarter,
	kLBC_Exlamation,
	kLBC_SymbolAllowingBreakAfter,
	kLBC_InfixNumericSeparator,
	kLBC_PrefixNumeric,
	kLBC_PostfixNumeric,
	kLBC_Numeric,
	kLBC_Alphabetic,
	kLBC_Ideographic,
	kLBC_Inseperable,
	kLBC_Hyphen,
	kLBC_BreakAfter,
	kLBC_BreakBefor,
	kLBC_BreakOpportunityBeforeAndAfter,
	kLBC_ZeroWidthSpace,
	kLBC_CombiningMark,
	kLBC_WordJoiner,
	kLBC_HangulLVSyllable,
	kLBC_HangulLVTSyllable,
	kLBC_HangulLJamo,
	kLBC_HangulVJamo,
	kLBC_HangulTJamo,

	kLBC_MandatoryBreak,
	kLBC_CarriageReturn,
	kLBC_LineFeed,
	kLBC_NextLine,
	kLBC_Surrogate,
	kLBC_Space,
	kLBC_ContigentBreakOpportunity,
	kLBC_Ambiguous,
	kLBC_ComplexContext,
	kLBC_Unknown
};


#include "MUnicodeTables.hpp"

// clang-format off
const unicode kMacOSRomanChars[] = {
	0x0000,  0x0001,  0x0002,  0x0003,  0x0004,  0x0005,  0x0006,  0x0007,  
	0x0008,  0x0009,  0x000A,  0x000B,  0x000C,  0x000D,  0x000E,  0x000F,  
	0x0010,  0x0011,  0x0012,  0x0013,  0x0014,  0x0015,  0x0016,  0x0017,  
	0x0018,  0x0019,  0x001A,  0x001B,  0x001C,  0x001D,  0x001E,  0x001F,  
	0x0020,  0x0021,  0x0022,  0x0023,  0x0024,  0x0025,  0x0026,  0x0027,  
	0x0028,  0x0029,  0x002A,  0x002B,  0x002C,  0x002D,  0x002E,  0x002F,  
	0x0030,  0x0031,  0x0032,  0x0033,  0x0034,  0x0035,  0x0036,  0x0037,  
	0x0038,  0x0039,  0x003A,  0x003B,  0x003C,  0x003D,  0x003E,  0x003F,  
	0x0040,  0x0041,  0x0042,  0x0043,  0x0044,  0x0045,  0x0046,  0x0047,  
	0x0048,  0x0049,  0x004A,  0x004B,  0x004C,  0x004D,  0x004E,  0x004F,  
	0x0050,  0x0051,  0x0052,  0x0053,  0x0054,  0x0055,  0x0056,  0x0057,  
	0x0058,  0x0059,  0x005A,  0x005B,  0x005C,  0x005D,  0x005E,  0x005F,  
	0x0060,  0x0061,  0x0062,  0x0063,  0x0064,  0x0065,  0x0066,  0x0067,  
	0x0068,  0x0069,  0x006A,  0x006B,  0x006C,  0x006D,  0x006E,  0x006F,  
	0x0070,  0x0071,  0x0072,  0x0073,  0x0074,  0x0075,  0x0076,  0x0077,  
	0x0078,  0x0079,  0x007A,  0x007B,  0x007C,  0x007D,  0x007E,  0x007F,  
	0x00C4,  0x00C5,  0x00C7,  0x00C9,  0x00D1,  0x00D6,  0x00DC,  0x00E1,  
	0x00E0,  0x00E2,  0x00E4,  0x00E3,  0x00E5,  0x00E7,  0x00E9,  0x00E8,  
	0x00EA,  0x00EB,  0x00ED,  0x00EC,  0x00EE,  0x00EF,  0x00F1,  0x00F3,  
	0x00F2,  0x00F4,  0x00F6,  0x00F5,  0x00FA,  0x00F9,  0x00FB,  0x00FC,  
	0x2020,  0x00B0,  0x00A2,  0x00A3,  0x00A7,  0x2022,  0x00B6,  0x00DF,  
	0x00AE,  0x00A9,  0x2122,  0x00B4,  0x00A8,  0x2260,  0x00C6,  0x00D8,  
	0x221E,  0x00B1,  0x2264,  0x2265,  0x00A5,  0x00B5,  0x2202,  0x2211,  
	0x220F,  0x03C0,  0x222B,  0x00AA,  0x00BA,  0x03A9,  0x00E6,  0x00F8,  
	0x00BF,  0x00A1,  0x00AC,  0x221A,  0x0192,  0x2248,  0x2206,  0x00AB,  
	0x00BB,  0x2026,  0x00A0,  0x00C0,  0x00C3,  0x00D5,  0x0152,  0x0153,  
	0x2013,  0x2014,  0x201C,  0x201D,  0x2018,  0x2019,  0x00F7,  0x25CA,  
	0x00FF,  0x0178,  0x2044,  0x20AC,  0x2039,  0x203A,  0xFB01,  0xFB02,  
	0x2021,  0x00B7,  0x201A,  0x201E,  0x2030,  0x00C2,  0x00CA,  0x00C1,  
	0x00CB,  0x00C8,  0x00CD,  0x00CE,  0x00CF,  0x00CC,  0x00D3,  0x00D4,  
	0xF8FF,  0x00D2,  0x00DA,  0x00DB,  0x00D9,  0x0131,  0x02C6,  0x02DC,  
	0x00AF,  0x02D8,  0x02D9,  0x02DA,  0x00B8,  0x02DD,  0x02DB,  0x02C7,  
};
// clang-format on

namespace MUnicodeMapping
{

unicode GetUnicode(MEncoding inEncoding, char inByte)
{
	switch (inEncoding)
	{
		case kEncodingMacOSRoman: return kMacOSRomanChars[static_cast<uint8_t>(inByte)];
		case kEncodingISO88591: return static_cast<uint8_t>(inByte); // iso-8859-1 maps exactly on unicode
		default: throw std::runtime_error("Invalid encoding for GetUnicode");
	}
}

char GetChar(MEncoding inEncoding, unicode inChar)
{
	char result = 0;

	for (int i = 0; i < 256; ++i)
	{
		if (inChar == kMacOSRomanChars[i])
		{
			result = static_cast<char>(i);
			break;
		}
	}

	if (inChar != 0 and result == 0)
	{
		result = '?';
	}

	return result;
}

} // namespace MUnicodeMapping

UnicodeProperty GetProperty(unicode inUnicode)
{
	uint8_t result = 0;

	if (inUnicode < 0x110000)
	{
		uint32_t ix = inUnicode >> 8;
		uint32_t p_ix = inUnicode & 0x00FF;

		ix = kUnicodeInfo.page_index[ix];
		result = kUnicodeInfo.data[ix][p_ix].prop;
	}

	return UnicodeProperty(result);
}

unicode ToLower(unicode inUnicode)
{
	unicode result = inUnicode;

	if (inUnicode < 0x110000)
	{
		uint32_t ix = inUnicode >> 8;
		uint32_t p_ix = inUnicode & 0x00FF;

		ix = kUnicodeInfo.page_index[ix];
		if (kUnicodeInfo.data[ix][p_ix].lower != 0)
			result = kUnicodeInfo.data[ix][p_ix].lower;
	}

	return result;
}

unicode ToUpper(unicode inUnicode)
{
	unicode result = inUnicode;

	if (inUnicode < 0x110000)
	{
		uint32_t ix = inUnicode >> 8;
		uint32_t p_ix = inUnicode & 0x00FF;

		ix = kUnicodeInfo.page_index[ix];
		if (kUnicodeInfo.data[ix][p_ix].upper != 0)
			result = kUnicodeInfo.data[ix][p_ix].upper;
	}

	return result;
}

// --------------------------------------------------------------------

bool IEquals(std::string_view a, std::string_view b)
{
	if (a.size() != b.size())
		return false;

	auto ai = a.begin();
	auto bi = b.begin();

	using traits = MEncodingTraits<kEncodingUTF8>;

	while (ai != a.end() and bi != b.end())
	{
		uint32_t la, lb;
		char32_t ua, ub;

		traits::ReadUnicode(ai, la, ua);
		traits::ReadUnicode(bi, lb, ub);

		if (ua != ub)
			return false;

		ai += la;
		bi += lb;
	}

	return true;
}

void Trim(std::string &ioString)
{
	auto s = ioString.begin();
	auto e = ioString.end();

	while (s != e and std::isspace(*s))
		++s;

	if (s > ioString.begin())
	{
		ioString.erase(ioString.begin(), s);
		e = ioString.end();
	}

	while (e != s and std::isspace(e[-1]))
		--e;

	if (e < ioString.end())
		ioString.erase(e, ioString.end());
}

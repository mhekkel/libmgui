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

#include "MError.hpp"
#include "MAlerts.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTypes.hpp"
#include "MUtils.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

#ifndef NDEBUG
int StOKToThrow::sOkToThrow = 0;
#endif

// #ifndef _MSC_VER
// MException::MException(
//	int					inErr)
//{
//	snprintf(mMessage, sizeof(mMessage), "OS error %d", inErr);
// }
// #endif

MException::MException(
	const char *inMsg,
	...)
{
	inMsg = GetLocalisedString(inMsg);

	va_list vl;
	va_start(vl, inMsg);
	vsnprintf(mMessage, sizeof(mMessage), inMsg, vl);
	va_end(vl);

	PRINT(("Throwing with msg: %s", mMessage));
}

const char *MException::what() const throw()
{
	return mMessage;
}

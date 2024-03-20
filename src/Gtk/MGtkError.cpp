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

#include "MAlerts.hpp"
#include "MError.hpp"
#include "MSound.hpp"
#include "MStrings.hpp"
#include "MTypes.hpp"
#include "MUtils.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

#include <gtk/gtk.h>

using namespace std;

#ifndef NDEBUG

const char *__S_FILE;
int __S_LINE;

#ifndef _MSC_VER
void __debug_printf(const char *inStr, ...)
{
	char msg[1024];

	va_list vl;
	va_start(vl, inStr);
	vsnprintf(msg, sizeof(msg), inStr, vl);
	va_end(vl);

	cerr << msg << '\n';
}

void __signal_throw(
	const char *inCode,
	const char *inFunction,
	const char *inFile,
	int inLine)
{
	cerr << "Throwing in file " << inFile << " line " << inLine
		 << " \"" << inFunction << "\": \n"
		 << inCode << '\n';

	if (StOKToThrow::IsOK())
		return;

	GtkWidget *dlg = gtk_message_dialog_new(nullptr, GTK_DIALOG_MODAL,
		GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
		"Exception thrown in file '%s', line %d, function: '%s'\n\n"
		"code: %s",
		inFile, inLine, inFunction, inCode);

	PlaySound("error");
	(void)gtk_dialog_run(GTK_DIALOG(dlg));

	gtk_widget_destroy(dlg);
}
#endif

#endif

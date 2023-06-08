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

// stock commands

enum MStdCommand
{
	cmd_New = 'new ',
	cmd_Open = 'open',
	cmd_Quit = 'quit',
	cmd_Close = 'clos',
	cmd_Save = 'save',
	cmd_SaveAs = 'sava',
	cmd_Revert = 'reve',
	cmd_PageSetup = 'pgsu',
	cmd_Print = 'prnt',
	cmd_Undo = 'undo',
	cmd_Redo = 'redo',
	cmd_Cut = 'cut ',
	cmd_Copy = 'copy',
	cmd_Paste = 'past',
	cmd_Clear = 'clea',
	cmd_SelectAll = 'sall',

	cmd_About = 'abou',
	cmd_Help = 'help',

	cmd_Menu = 'menu', // Shift-F10 or Menu key

	cmd_Find = 'Find',
	cmd_FindNext = 'FndN',
	cmd_FindPrev = 'FndP',
	cmd_Replace = 'Repl',
	cmd_ReplaceAll = 'RplA',
	cmd_ReplaceFindNext = 'RpFN',
	cmd_ReplaceFindPrev = 'RpFP',
	cmd_Preferences = 'pref',
	cmd_CloseAll = 'Clos',
	cmd_SaveAll = 'Save',
	cmd_OpenRecent = 'Recd',
	cmd_ClearRecent = 'ClrR',
	cmd_SelectWindowFromMenu = 'WSel',
};

// ---------------------------------------------------------------------------
//
// edit key commands
//

enum MKeyCommand
{
	kcmd_None,

	kcmd_MoveCharacterLeft,
	kcmd_MoveCharacterRight,
	kcmd_MoveWordLeft,
	kcmd_MoveWordRight,
	kcmd_MoveToBeginningOfLine,
	kcmd_MoveToEndOfLine,
	kcmd_MoveToPreviousLine,
	kcmd_MoveToNextLine,
	kcmd_MoveToTopOfPage,
	kcmd_MoveToEndOfPage,
	kcmd_MoveToBeginningOfFile,
	kcmd_MoveToEndOfFile,

	kcmd_MoveLineUp,
	kcmd_MoveLineDown,

	kcmd_DeleteCharacterLeft,
	kcmd_DeleteCharacterRight,
	kcmd_DeleteWordLeft,
	kcmd_DeleteWordRight,
	kcmd_DeleteToEndOfLine,
	kcmd_DeleteToEndOfFile,

	kcmd_ExtendSelectionWithCharacterLeft,
	kcmd_ExtendSelectionWithCharacterRight,
	kcmd_ExtendSelectionWithPreviousWord,
	kcmd_ExtendSelectionWithNextWord,
	kcmd_ExtendSelectionToCurrentLine,
	kcmd_ExtendSelectionToPreviousLine,
	kcmd_ExtendSelectionToNextLine,
	kcmd_ExtendSelectionToBeginningOfLine,
	kcmd_ExtendSelectionToEndOfLine,
	kcmd_ExtendSelectionToBeginningOfPage,
	kcmd_ExtendSelectionToEndOfPage,
	kcmd_ExtendSelectionToBeginningOfFile,
	kcmd_ExtendSelectionToEndOfFile,

	kcmd_ScrollOneLineUp,
	kcmd_ScrollOneLineDown,
	kcmd_ScrollPageUp,
	kcmd_ScrollPageDown,
	kcmd_ScrollToStartOfFile,
	kcmd_ScrollToEndOfFile
};

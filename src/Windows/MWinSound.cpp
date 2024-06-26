//          Copyright Maarten L. Hekkelman 2006-2008
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "MWinLib.hpp"

#include <map>
#include <iostream>

#include "MFile.hpp"
#include "MSound.hpp"
#include "MPreferences.hpp"
#include "MWindow.hpp"
#include "MError.hpp"

using namespace std;

void PlaySound(
	const string&		inSoundName)
{
	if (MPrefs::GetBoolean("play sounds", true) == false)
		return;

	if (inSoundName == "success")
		::MessageBeep(MB_ICONINFORMATION);
	else if (inSoundName == "failure" or inSoundName == "error")
		::MessageBeep(MB_ICONERROR);
	else if (inSoundName == "warning")
		::MessageBeep(MB_ICONWARNING);
	else if (inSoundName == "question")
		::MessageBeep(MB_ICONQUESTION);
	else
		::MessageBeep(MB_ICONINFORMATION);
}


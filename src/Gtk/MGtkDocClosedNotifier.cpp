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

#include "MDocClosedNotifier.hpp"
#include "MError.hpp"

#include <cassert>
#include <iostream>

using namespace std;

// if (inRead)
//{
//	mPreparedForStdOut = true;
//	mDataFD = inNotifier.GetFD();

//	int flags = fcntl(mDataFD, F_GETFL, 0);
//	if (fcntl(mDataFD, F_SETFL, flags | O_NONBLOCK))
//		cerr << _("Failed to set fd non blocking: ") << strerror(errno) << '\n';
//}

// if (mDataFD >= 0)
//{
//	char buffer[10240];
//	int r = read(mDataFD, buffer, sizeof(buffer));
//	if (r == 0 or (r < 0 and errno != EAGAIN))
//		mDataFD = -1;
//	else if (r > 0)
//		StdOut(buffer, r);
// }

MDocClosedNotifierImpl::MDocClosedNotifierImpl()
	: mRefCount(1)
{
}

MDocClosedNotifierImpl::~MDocClosedNotifierImpl()
{
	assert(mRefCount == 0);
}

void MDocClosedNotifierImpl::AddRef()
{
	++mRefCount;
}

void MDocClosedNotifierImpl::Release()
{
	if (--mRefCount <= 0)
		delete this;
}

// --------------------------------------------------------------------

MDocClosedNotifier::MDocClosedNotifier(
	MDocClosedNotifierImpl *inImpl)
	: mImpl(inImpl)
{
}

MDocClosedNotifier::MDocClosedNotifier(
	const MDocClosedNotifier &inRHS)
{
	mImpl = inRHS.mImpl;
	mImpl->AddRef();
}

MDocClosedNotifier &MDocClosedNotifier::operator=(
	const MDocClosedNotifier &inRHS)
{
	if (mImpl != inRHS.mImpl)
	{
		mImpl->Release();
		mImpl = inRHS.mImpl;
		mImpl->AddRef();
	}

	return *this;
}

MDocClosedNotifier::~MDocClosedNotifier()
{
	mImpl->Release();
}

bool MDocClosedNotifier::ReadSome(
	string &outText)
{
	return mImpl->ReadSome(outText);
}

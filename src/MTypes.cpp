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

#include "MTypes.hpp"

#include <algorithm>
#include <cassert>
#include <vector>

using namespace std;

void MRect::InsetBy(int32_t inDeltaX, int32_t inDeltaY)
{
	if (inDeltaX < 0 or 2 * inDeltaX <= width)
	{
		x += inDeltaX;
		width -= inDeltaX * 2;
	}
	else
	{
		x += width / 2;
		width = 0;
	}

	if (inDeltaY < 0 or 2 * inDeltaY <= height)
	{
		y += inDeltaY;
		height -= inDeltaY * 2;
	}
	else
	{
		y += height / 2;
		height = 0;
	}
}

void MRect::PinPoint(int32_t &ioX, int32_t &ioY) const
{
	if (ioX < x)
		ioX = x;
	if (ioX > x + width)
		ioX = x + width;
	if (ioY < y)
		ioY = y;
	if (ioY > y + height)
		ioY = y + height;
}

MRect MRect::operator&(const MRect &inRhs)
{
	MRect result(*this);
	result &= inRhs;
	return result;
}

MRect &MRect::operator&=(const MRect &inRhs)
{
	int32_t nx = x;
	if (nx < inRhs.x)
		nx = inRhs.x;

	int32_t ny = y;
	if (ny < inRhs.y)
		ny = inRhs.y;

	int32_t nx2 = x + width;
	if (nx2 > inRhs.x + inRhs.width)
		nx2 = inRhs.x + inRhs.width;

	int32_t ny2 = y + height;
	if (ny2 > inRhs.y + inRhs.height)
		ny2 = inRhs.y + inRhs.height;

	x = nx;
	y = ny;

	width = nx2 - nx;
	if (width < 0)
		width = 0;

	height = ny2 - ny;
	if (height < 0)
		height = 0;

	return *this;
}

MRect &MRect::operator|=(const MRect &inRhs)
{
	int32_t nx = x;
	if (nx > inRhs.x)
		nx = inRhs.x;

	int32_t ny = y;
	if (ny > inRhs.y)
		ny = inRhs.y;

	int32_t nx2 = x + width;
	if (nx2 < inRhs.x + inRhs.width)
		nx2 = inRhs.x + inRhs.width;

	int32_t ny2 = y + height;
	if (ny2 < inRhs.y + inRhs.height)
		ny2 = inRhs.y + inRhs.height;

	x = nx;
	y = ny;

	width = nx2 - nx;
	if (width < 0)
		width = 0;

	height = ny2 - ny;
	if (height < 0)
		height = 0;

	return *this;
}

MRect MRect::operator|(const MRect &inRhs)
{
	MRect result(*this);
	result |= inRhs;
	return result;
}

bool MRect::empty() const
{
	return width <= 0 or height <= 0;
}

struct MRegionImpl : public vector<MRect>
{
};

MRegion::MRegion()
	: mImpl(new MRegionImpl())
{
}

MRegion::MRegion(const MRect &inRect)
	: mImpl(new MRegionImpl())
{
	mImpl->push_back(inRect);
}

MRegion::MRegion(const MRegion &inRegion)
	: mImpl(new MRegionImpl(*inRegion.mImpl))
{
}

MRegion::~MRegion()
{
	delete mImpl;
}

MRegion &MRegion::operator=(const MRegion &inRegion)
{
	if (this != &inRegion)
		*mImpl = *inRegion.mImpl;
	return *this;
}

// MRegion& MRegion::operator&(const MRegion& inRegion) const
//{
//	assert(false);
//	return *this;
// }
//
// MRegion& MRegion::operator&(const MRect& inRect) const
//{
//	assert(false);
//	return *this;
// }
//
// MRegion& MRegion::operator&=(const MRegion& inRegion) const
//{
//	assert(false);
//	return *this;
// }
//
// MRegion& MRegion::operator&=(const MRect& inRect)
//{
//	assert(false);
//	return *this;
// }

// MRegion& MRegion::operator|(const MRegion& inRegion) const
//{
//	assert(false);
//	return *this;
// }
//
// MRegion& MRegion::operator|(const MRect& inRect) const
//{
//	mImpl->push_back(inRect);
//	return *this;
// }
//
// MRegion& MRegion::operator|=(const MRegion& inRegion)
//{
//	assert(false);
//	return *this;
// }

MRegion &MRegion::operator|=(const MRect &inRect)
{
	mImpl->push_back(inRect);
	return *this;
}

MRegion::operator bool() const
{
	bool result = true;
	if (not mImpl->empty())
	{
		result = find_if(mImpl->begin(), mImpl->end(),
					 [](const MRect &r)
					 { return not r; }) == mImpl->end();
	}

	return result;
}

void MRegion::OffsetBy(int32_t inX, int32_t inY)
{
	assert(false);
}

bool MRegion::ContainsPoint(int32_t inX, int32_t inY) const
{
	return find_if(mImpl->begin(), mImpl->end(),
			   [&inX, &inY](const MRect &r)
			   { return r.ContainsPoint(inX, inY); }) != mImpl->end();
}

MRect MRegion::GetBounds() const
{
	MRect bounds;
	for_each(mImpl->begin(), mImpl->end(),
		[&bounds](const MRect &r)
		{ bounds &= r; });
	return bounds;
}
